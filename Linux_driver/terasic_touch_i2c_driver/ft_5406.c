/*********************************************************************************
 * Terasic MTL2 touch screen driver code
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *********************************************************************************/
 
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/i2c/ft5406_ts.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/input/mt.h>

#define ts_pio_phybase  0xff200020
#define ts_pio_span     0x20
#define MTL_TSC_MIN_X_VAL			0x0
#define MTL_TSC_MAX_X_VAL			0x356
#define MTL_TSC_MIN_Y_VAL			0x0
#define MTL_TSC_MAX_Y_VAL			0x1E0
#define MAX_SUPPORT_POINTS	        5
/***********************************************************************************************
struct definitions
***********************************************************************************************/
struct ts_event 
{
    u16    x1;
    u16    y1;
    u16    x2;
    u16    y2;
    u16    x3;
    u16    y3;
    u16    x4;
    u16    y4;
    u16    x5;
    u16    y5;
    u16    pressure;
    u8     touch_point;
};

int x,y;
/******************************************************
******************************************************/
struct ft5406_ts_data 
{
    struct input_dev        *input_dev;
    struct ts_event         event;
    struct work_struct      pen_event_work;
    struct workqueue_struct *ts_workqueue;
};

/***********************************************************************************************
global variables
***********************************************************************************************/
static  struct i2c_client   *this_client;
static  void __iomem *ts_pio_base;
static void touch_timer_fire(unsigned long data1);

static struct timer_list touch_timer =  
        TIMER_INITIALIZER(touch_timer_fire, 0, 0); 


static int ft5406_i2c_rxdata(char *rxdata, int length)
{
    int ret;
 
    struct i2c_msg msgs[] = 
    {        
        {
            .addr   = this_client->addr,
            .flags  = 0 ,
            .len    = 1,
            .buf    = rxdata,
        },
 
        {
            .addr   = this_client->addr,
            .flags  = I2C_M_RD,
            .len    = length,
            .buf    = rxdata,
        },
    };
 
    ret = i2c_transfer(this_client->adapter, msgs, 2);
    if (ret < 0)
        pr_err("msg %s i2c write internal address error: %d\n", __func__, ret);
    return ret;
}
 

static int ft5406_i2c_txdata(char *txdata, int length)
{
    int ret;
 
    struct i2c_msg msg[] = 
    {
        {
            .addr   = this_client->addr,
            .flags  = 0,
            .len    = length,
            .buf    = txdata,
        },
    };
    ret = i2c_transfer(this_client->adapter, msg, 1);
    if (ret < 0)
        pr_err("%s i2c write error: %d\n", __func__, ret);
 
    return ret;
}
 

static void ft5406_ts_release(void)
{
    struct ft5406_ts_data *data = i2c_get_clientdata(this_client);
    input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, 0);
    input_sync(data->input_dev);
}
 

static int ft5406_read_data(void)
{
    struct ft5406_ts_data *data = i2c_get_clientdata(this_client);
    struct ts_event *event = &data->event;
    u8 buf[32] = {0};
    int ret = -1;
    /*voodoos say we read 26 bytes of five-point touch data from register 0x00*/
    buf[0]=0x00;
    ret = ft5406_i2c_rxdata(buf, 31);
    if (ret < 0) 
    {
        ft5406_ts_release();
        return ret;
    }
 
    memset(event, 0, sizeof(struct ts_event));
    /*read byte02 & count the number of touch points ctd in this msg*/
    event->touch_point = buf[2] & 0xF;
    if (event->touch_point == 0) 
    {
        ft5406_ts_release();
        return 2;
    }
 
    /*read upto five points of touch data*/
    switch (event->touch_point) 
    {
        case 5:
            event->x5 = (buf[27] & 0x0F)<<8 | buf[28];
            event->y5 = (s16)(buf[29] & 0x0F)<<8 | (s16)buf[30];
        case 4:
            event->x4  =(buf[21] & 0x0F)<<8 | buf[22];
            event->y4 = (s16)(buf[23] & 0x0F)<<8 | (s16)buf[24];
        case 3:
            event->x3= (buf[15] & 0x0F)<<8 | buf[16];
            event->y3 = (s16)(buf[17] & 0x0F)<<8 | (s16)buf[18];
        case 2:
            event->x2 = (buf[9] & 0x0F)<<8 | buf[10];
            event->y2 = (s16)(buf[11] & 0x0F)<<8 | (s16)buf[12];
        case 1:
            event->x1= (buf[3] & 0x0F)<<8 | buf[4];
            event->y1 = (s16)(buf[5] & 0x0F)<<8 | (s16)buf[6];
            break;
        default:
            return -1;
    }
    event->pressure = 50; //this parameter decide the size erea of  touch point
    dev_dbg(&this_client->dev, "%s: 1:%d %d 2:%d %d \n", __func__, event->x1, event->y1, event->x2, event->y2);
 
    return 0;
}
 

static void touch_timer_fire(unsigned long data1)
{
	struct ft5406_ts_data *data = i2c_get_clientdata(this_client);
	input_report_abs(data->input_dev, ABS_X, x);
	input_report_abs(data->input_dev, ABS_Y, y);
        input_report_key(data->input_dev, BTN_TOUCH, 0);  
        input_report_abs(data->input_dev, ABS_PRESSURE, 0);  
        input_sync(data->input_dev);
	return;
 
} 


static void ft5406_report_value(void)
{
    struct ft5406_ts_data *data = i2c_get_clientdata(this_client);
    struct ts_event *event = &data->event;
    switch(event->touch_point) 
    {
        case 5:
            input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, event->pressure);
            input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->x5);
            input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->y5);
            input_report_abs(data->input_dev, ABS_MT_WIDTH_MAJOR, 1);
            input_mt_sync(data->input_dev);
        case 4:
            input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, event->pressure);
            input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->x4);
            input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->y4);
            input_report_abs(data->input_dev, ABS_MT_WIDTH_MAJOR, 1);
            input_mt_sync(data->input_dev);
        case 3:
            input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, event->pressure);
            input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->x3);
            input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->y3);
            input_report_abs(data->input_dev, ABS_MT_WIDTH_MAJOR, 1);
            input_mt_sync(data->input_dev);
        case 2:
            input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, event->pressure);
            input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->x2);
            input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->y2);
            input_report_abs(data->input_dev, ABS_MT_WIDTH_MAJOR, 1);
            input_mt_sync(data->input_dev);
        case 1:
            input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, event->pressure);
            input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->x1);
            input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->y1);
            input_report_abs(data->input_dev, ABS_MT_WIDTH_MAJOR, 1);
	    input_mt_sync(data->input_dev);
	    input_report_abs(data->input_dev, ABS_X, event->x1);
	    x=event->x1;
	    input_report_abs(data->input_dev, ABS_Y, event->y1);
	    y=event->y1;
	    //input_report_abs(data->input_dev, ABS_PRESSURE, 1);
	    input_report_key(data->input_dev, BTN_TOUCH, 1);

	    //input_sync(data->input_dev);
        default:
            break;
    }
 
    input_sync(data->input_dev);
    mod_timer(&touch_timer, jiffies+5); 
    
}   


static void ft5406_ts_pen_irq_work(struct work_struct *work)
{
    int ret = -1;
 
    ret = ft5406_read_data();
    if(ret==0)	
       ft5406_report_value();
    return;
}


static irqreturn_t ft5406_ts_interrupt(int irq, void *dev_id)
{
    struct ft5406_ts_data *ft5406_ts = dev_id;
    __raw_writel(0x00,ts_pio_base+0X03*4);
    if (!work_pending(&ft5406_ts->pen_event_work)) 
    {
        queue_work(ft5406_ts->ts_workqueue, &ft5406_ts->pen_event_work);
    } 
    return IRQ_HANDLED;
}
 
 
/******************************************************
******************************************************/
static const struct i2c_device_id ft5406_ts_id[] = 
{
    {FT5406_NAME, 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c,ft5406_ts_id);


static int ft5406_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct ft5406_ts_data *ft5406_ts;
    struct input_dev *input_dev;
    int err = 0;

    printk("==ft5406_ts_probe=\n");
    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) 
    {
        err = -ENODEV;
        goto exit_check_functionality_failed;
    }
 
    ft5406_ts = kzalloc(sizeof(*ft5406_ts), GFP_KERNEL);
    if (!ft5406_ts)    
    {
        err = -ENOMEM;
        goto exit_alloc_data_failed;
    }
    
    this_client = client;
    i2c_set_clientdata(client, ft5406_ts);
 


   INIT_WORK(&ft5406_ts->pen_event_work, ft5406_ts_pen_irq_work);
 
    ft5406_ts->ts_workqueue = create_singlethread_workqueue(dev_name(&client->dev));
    if (!ft5406_ts->ts_workqueue) 
    {
        err = -ESRCH;
        dev_err(&this_client->dev,"==singlethread error==\n");
        goto exit_create_singlethread;
    }

    err = request_irq(this_client->irq,ft5406_ts_interrupt,0, FT5406_NAME, ft5406_ts);
    if (err ) 
    {
        dev_err(&client->dev, "ft5406_probe: request irq failed\n");
        goto exit_irq_request_failed;
    }
    disable_irq(this_client->irq);
 
 
    input_dev = input_allocate_device();
    if (!input_dev) 
    {
        err = -ENOMEM;
        dev_err(&client->dev, "failed to allocate input device\n");
        goto exit_input_dev_alloc_failed;
    }    
 
    ft5406_ts->input_dev = input_dev;
    input_dev->name = FT5406_NAME;
 
 
    set_bit(ABS_MT_TOUCH_MAJOR, input_dev->absbit);
    set_bit(ABS_MT_POSITION_X, input_dev->absbit);
    set_bit(ABS_MT_POSITION_Y, input_dev->absbit);
    set_bit(ABS_MT_WIDTH_MAJOR, input_dev->absbit);
 
    input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0, SCREEN_MAX_X, 0, 0);
    input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0, SCREEN_MAX_Y, 0, 0);
    input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, 800, 0, 0);
    input_set_abs_params(input_dev, ABS_MT_TOUCH_MINOR, 0, 480, 0, 0);
    input_set_abs_params(input_dev, ABS_MT_WIDTH_MAJOR, 0, 200, 0, 0);


    input_set_abs_params(input_dev, ABS_X, MTL_TSC_MIN_X_VAL,
 		     MTL_TSC_MAX_X_VAL, 0, 0);
    input_set_abs_params(input_dev, ABS_Y, MTL_TSC_MIN_Y_VAL,
			     MTL_TSC_MAX_Y_VAL, 0, 0);
    set_bit(EV_ABS, input_dev->evbit);
    set_bit(EV_KEY, input_dev->evbit);
    set_bit(EV_SYN, input_dev->evbit);

    input_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);

    err = input_register_device(input_dev);
    if (err) 
    {
        dev_err(&client->dev,"ft5406_ts_probe: failed to register input device: %s\n", dev_name(&client->dev));
        goto exit_input_register_device_failed;
    }
//#define ts_pio_phybase  0xff200200
//#define      0x20
    ts_pio_base = ioremap(ts_pio_phybase,ts_pio_span );
     if (!ts_pio_base) {
 	 err = -ENOMEM;
 	 goto err_release_mem;
 	}
     __raw_writel(0x01,ts_pio_base+0X02*4);     
    enable_irq(this_client->irq);

    printk("==ft5406_ts_probe complete==\n\n");
    return 0;
 
err_release_mem: 
exit_input_register_device_failed:
    input_free_device(input_dev);
 
exit_input_dev_alloc_failed:
    free_irq(this_client->irq, ft5406_ts);
 
exit_irq_request_failed:
    cancel_work_sync(&ft5406_ts->pen_event_work);
    destroy_workqueue(ft5406_ts->ts_workqueue);
 
exit_create_singlethread:
    i2c_set_clientdata(client, NULL);
    kfree(ft5406_ts);
 
exit_alloc_data_failed:
exit_check_functionality_failed:
 
    printk("==ft5406_ts_probe failed==\n\n");
    return err;
}
 

static int ft5406_ts_remove(struct i2c_client *client)
{
    struct ft5406_ts_data *ft5406_ts=0;
 
    printk("==ft5406_ts_remove=\n");
    
    ft5406_ts = i2c_get_clientdata(client);
    free_irq(this_client->irq, ft5406_ts);
 
    input_unregister_device(ft5406_ts->input_dev);
    kfree(ft5406_ts);
    cancel_work_sync(&ft5406_ts->pen_event_work);
    destroy_workqueue(ft5406_ts->ts_workqueue);
    i2c_set_clientdata(client, NULL);
    return 0;
}

/******************************************************
******************************************************/
static struct i2c_driver ft5406_ts_driver = 
{
    .driver= 
    {
        .name=  FT5406_NAME,
        .owner= THIS_MODULE,
    },
    .probe=ft5406_ts_probe,
    .remove= ft5406_ts_remove,
    .id_table= ft5406_ts_id,   
};
 
module_i2c_driver(ft5406_ts_driver);

MODULE_DESCRIPTION("mtl touch screen test driver");
MODULE_AUTHOR("matthew wang--terasic");
MODULE_LICENSE("GPL");
 
/*end of file*/

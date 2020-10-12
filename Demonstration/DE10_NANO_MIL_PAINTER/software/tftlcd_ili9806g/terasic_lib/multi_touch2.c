#include "terasic_includes.h"
#include "multi_touch2.h"
#include "I2C_core.h"
#include "I2C.h"
#include "alt_types.h"

#define TRUE 1
#define FALSE 0

static void mtc2_QueryData(MTC2_INFO *p){
    MTC2_EVENT *pEvent, *pOldEvent;
    unsigned char reg_data[31];
    unsigned long x1,y1,x2,y2,x3,y3,x4,y4,x5,y5;
    if(OC_I2C_Read(p->TOUCH_I2C_BASE,I2C_FT5316_ADDR,0x00,reg_data,31))
        {
 //   			if(reg_data[1]>0)
//    				printf("reg_data[1]=%x\n",reg_data[1]);
    			 pEvent = (MTC2_EVENT *)malloc(sizeof(MTC2_EVENT));
    			 pEvent->Event=reg_data[1];
        		 pEvent->TouchNum = reg_data[2];
        		 x1 = ((reg_data[3]&0x0f)<<8)|reg_data[4];
        		 y1 = ((reg_data[5]&0x0f)<<8)|reg_data[6];
        		 x2 = ((reg_data[9]&0x0f)<<8)|reg_data[10];
        		 y2 = ((reg_data[11]&0x0f)<<8)|reg_data[12];

        		 x3 = ((reg_data[15]&0x0f)<<8)|reg_data[16];
				 y3 = ((reg_data[17]&0x0f)<<8)|reg_data[18];

				 x4 = ((reg_data[21]&0x0f)<<8)|reg_data[22];
				 y4 = ((reg_data[23]&0x0f)<<8)|reg_data[24];

				 x5 = ((reg_data[27]&0x0f)<<8)|reg_data[28];
				 y5 = ((reg_data[29]&0x0f)<<8)|reg_data[30];
				 //the register value (1024,600)
				 //change the value to (800,480)
//        		 pEvent->x1=(x1*800)>>10;
//        		 pEvent->y1=(y1/10)<<3;
//        		 pEvent->x2=(x2*800)>>10;
//        		 pEvent->y2=(y2/10)<<3;
//        		 pEvent->x3=(x3*800)>>10;
//				 pEvent->y3=(y3/10)<<3;
//				 pEvent->x4=(x4*800)>>10;
//				 pEvent->y4=(y4/10)<<3;
//				 pEvent->x5=(x5*800)>>10;
//				 pEvent->y5=(y5/10)<<3;

//        		 pEvent->x1=480-x1;
//        		 pEvent->y1=854-y1;
//        		 pEvent->x2=480-x2;
//        		 pEvent->y2=854-y2;
//        		 pEvent->x3=480-x3;
//				 pEvent->y3=854-y3;
//				 pEvent->x4=480-x4;
//				 pEvent->y4=854-y4;
//				 pEvent->x5=480-x5;
//				 pEvent->y5=854-y5;

        		 pEvent->x1=x1;
        		 pEvent->y1=y1;
        		 pEvent->x2=x2;
        		 pEvent->y2=y2;
        		 pEvent->x3=x3;
				 pEvent->y3=y3;
				 pEvent->x4=x4;
				 pEvent->y4=y4;
				 pEvent->x5=x5;
				 pEvent->y5=y5;

        }
    if((pEvent->TouchNum>0)&&(pEvent->TouchNum<=5))
    {
		if (QUEUE_IsFull(p->pQueue)){
					  // remove the old one
		  pOldEvent = (MTC2_EVENT *)QUEUE_Pop(p->pQueue);
		  free(pOldEvent);
		 }
		 QUEUE_Push(p->pQueue, (alt_u32)pEvent);
    }
    else
    	free(pEvent);
}


#ifdef ALT_ENHANCED_INTERRUPT_API_PRESENT
static void mtc2_ISR(void* context){
#else
static void mtc2_ISR(void* context, alt_u32 id){
#endif
   MTC2_INFO *p = (MTC2_INFO *)context;
#ifdef ALT_ENHANCED_INTERRUPT_API_PRESENT
    alt_ic_irq_disable(LCD_TOUCH_INT_IRQ_INTERRUPT_CONTROLLER_ID,p->INT_IRQ_NUM);
#else
    alt_irq_disable(id);
#endif
    mtc2_QueryData(p);

    // Reset the edge capture register
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(p->TOUCH_INT_BASE,0);
#ifdef ALT_ENHANCED_INTERRUPT_API_PRESENT
    alt_ic_irq_enable(LCD_TOUCH_INT_IRQ_INTERRUPT_CONTROLLER_ID,p->INT_IRQ_NUM);
#else
    alt_irq_enable(id);
#endif
 }

MTC2_INFO* MTC2_Init(alt_u32 TOUCH_I2C_BASE,alt_u32 TOUCH_INT_BASE, alt_u32 INT_IRQ_NUM)
{
    MTC2_INFO *p;

    p = (MTC2_INFO *)malloc(sizeof(MTC2_INFO));
    p->TOUCH_I2C_BASE=TOUCH_I2C_BASE;
    p->TOUCH_INT_BASE=TOUCH_INT_BASE;

    p->INT_IRQ_NUM = INT_IRQ_NUM;
    p->pQueue = QUEUE_New(TOUCH_QUEUE_SIZE);


//    // enable interrupt
    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(p->TOUCH_INT_BASE, 0x00);
//    // clear capture flag
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(p->TOUCH_INT_BASE, 0x00);
 // register callback function
 //   error = alt_irq_register (p->INT_IRQ_NUM, p, mtc2_ISR);
#ifdef ALT_ENHANCED_INTERRUPT_API_PRESENT
  if ((alt_ic_isr_register(LCD_TOUCH_INT_IRQ_INTERRUPT_CONTROLLER_ID,
		                   p->INT_IRQ_NUM,
		                   mtc2_ISR,
		                   (void *)p,
		                   NULL
		                   ) != 0)){
 #else
  if ((alt_irq_register(p->INT_IRQ_NUM, (void *)p, mtc2_ISR) != 0)){
 #endif

	  printf(("[TOUCH]register IRQ fail\n"));
		  }else{
			  printf(("[TOUCH]register IRQ success\n"));
		  }
    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(p->TOUCH_INT_BASE, 0x01);
    return p;
}

void MTC2_UnInit(MTC2_INFO *p){
    if (p){
        QUEUE_Delete(p->pQueue);
        free(p);
    }
}

//, int X3,int Y3,int X4,int Y4,int X5,int Y5
bool MTC2_GetStatus(MTC2_INFO *p, alt_u8 *Event, alt_u8 *TouchNum, int *X1, int *Y1, int *X2, int *Y2,int *X3, int *Y3, int *X4, int *Y4,int *X5,int *Y5)
{
    bool bFind;
    MTC2_EVENT *pEvent;
    bFind = QUEUE_IsEmpty(p->pQueue)?FALSE:TRUE;
    if (bFind){
        pEvent = (MTC2_EVENT *)QUEUE_Pop(p->pQueue);
        *Event=pEvent->Event;;
        *TouchNum = pEvent->TouchNum;
        *X1 = pEvent->x1;
        *Y1 = pEvent->y1;
        *X2 = pEvent->x2;
        *Y2 = pEvent->y2;
        *X3 = pEvent->x3;
		*Y3 = pEvent->y3;
		*X4 = pEvent->x4;
		*Y4 = pEvent->y4;
		*X5 = pEvent->x5;
		*Y5 = pEvent->y5;
        free(pEvent);
    }
    return bFind;
}


void MTC2_ClearEvent(MTC2_INFO *p){
    QUEUE_Empty(p->pQueue);
}




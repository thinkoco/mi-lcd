#include "lcd.h"
#include "unistd.h"
#include "system.h"
#include "altera_avalon_spi.h"
#include "altera_avalon_spi_regs.h"
void delay_ms (u32 tm)
{
	u32 i;
	for(i=0;i<tm;i++)
		usleep(1011);
}

static void LCD_Rst(void)
{
	delay_ms(500);
	//SPI select
	IOWR_ALTERA_AVALON_SPI_SLAVE_SEL(LCD_SPI_BASE, 1);
	//IOWR_ALTERA_AVALON_SPI_CONTROL(LCD_SPI_BASE,0x400);
}

static void WriteComm(u8 cmd)
{
	u8 status;
	do{
         status = IORD_ALTERA_AVALON_SPI_STATUS(LCD_SPI_BASE);//get status
		}while ((status & ALTERA_AVALON_SPI_STATUS_TRDY_MSK) == 0);

	//TRDY = 0, not empty;  RRDY = 0,not full
	IOWR_ALTERA_AVALON_SPI_TXDATA(LCD_SPI_BASE,  (cmd & 0x0FF));
}
static void WriteData(u8 tem_data)
{
	u8 status;
	do{
         status = IORD_ALTERA_AVALON_SPI_STATUS(LCD_SPI_BASE);//get status
		}while ((status & ALTERA_AVALON_SPI_STATUS_TRDY_MSK) == 0 );
/* wait till transmit and ready ok */
	IOWR_ALTERA_AVALON_SPI_TXDATA(LCD_SPI_BASE, (tem_data | 0x100));
}

static void WriteColor(u8 tem_data)
{
	IOWR_ALTERA_AVALON_SPI_TXDATA(LCD_SPI_BASE, (tem_data | 0x100));
}
/**********************************************
Lcd初始化函数
***********************************************/
void Lcd_Initialize(void)
{	
LCD_Rst();
   //************* Start Initial Sequence **********//
WriteComm(0xFF); // EXTC Command Set enable register
WriteData(0xFF);
WriteData(0x98);
WriteData(0x06);
WriteComm(0xBA); // SPI Interface Setting
WriteData(0xE0);
WriteComm(0xBC); // GIP 1
WriteData(0x03);
WriteData(0x0F);
WriteData(0x63);
WriteData(0x69);
WriteData(0x01);
WriteData(0x01);
WriteData(0x1B);
WriteData(0x11);
WriteData(0x70);
WriteData(0x73);
WriteData(0xFF);
WriteData(0xFF);
WriteData(0x08);
WriteData(0x09);
WriteData(0x05);
WriteData(0x00);
WriteData(0xEE);
WriteData(0xE2);
WriteData(0x01);
WriteData(0x00);
WriteData(0xC1);
WriteComm(0xBD); // GIP 2
WriteData(0x01);
WriteData(0x23);
WriteData(0x45);
WriteData(0x67);
WriteData(0x01);
WriteData(0x23);
WriteData(0x45);
WriteData(0x67);
WriteComm(0xBE); // GIP 3
WriteData(0x00);
WriteData(0x22);
WriteData(0x27);
WriteData(0x6A);
WriteData(0xBC);
WriteData(0xD8);
WriteData(0x92);
WriteData(0x22);
WriteData(0x22);
WriteComm(0xC7); // Vcom
WriteData(0x1E);
WriteComm(0xED); // EN_volt_reg
WriteData(0x7F);
WriteData(0x0F);
WriteData(0x00);
WriteComm(0xC0); // Power Control 1
WriteData(0xE3);
WriteData(0x0B);
WriteData(0x00);
WriteComm(0xFC);
WriteData(0x08);
WriteComm(0xDF); // Engineering Setting
WriteData(0x00);
WriteData(0x00);
WriteData(0x00);
WriteData(0x00);
WriteData(0x00);
WriteData(0x02);
WriteComm(0xF3); // DVDD Voltage Setting
WriteData(0x74);
WriteComm(0xB4); // Display Inversion Control
WriteData(0x00);
WriteData(0x00);
WriteData(0x00);
WriteComm(0xF7); // 480x854
WriteData(0x81);
WriteComm(0xB0); // Hsync Vsync low Active
WriteData(0x0D);
WriteComm(0xB1); // Frame Rate
WriteData(0x00);
WriteData(0x10);
WriteData(0x14);
WriteComm(0xF1); // Panel Timing Control
WriteData(0x29);
WriteData(0x8A);
WriteData(0x07);
WriteComm(0xF2); //Panel Timing Control
WriteData(0x40);
WriteData(0xD2);
WriteData(0x50);
WriteData(0x28);
WriteComm(0xC1); // Power Control 2
WriteData(0x17);
WriteData(0X85);
WriteData(0x85);
WriteData(0x20);
WriteComm(0xE0);
WriteData(0x00); //P1
WriteData(0x0C); //P2
WriteData(0x15); //P3
WriteData(0x0D); //P4
WriteData(0x0F); //P5
WriteData(0x0C); //P6
WriteData(0x07); //P7
WriteData(0x05); //P8
WriteData(0x07); //P9
WriteData(0x0B); //P10
WriteData(0x10); //P11
WriteData(0x10); //P12
WriteData(0x0D); //P13
WriteData(0x17); //P14
WriteData(0x0F); //P15
WriteData(0x00); //P16
WriteComm(0xE1);
WriteData(0x00); //P1
WriteData(0x0D); //P2
WriteData(0x15); //P3
WriteData(0x0E); //P4
WriteData(0x10); //P5
WriteData(0x0D); //P6
WriteData(0x08); //P7
WriteData(0x06); //P8
WriteData(0x07); //P9
WriteData(0x0C); //P10
WriteData(0x11); //P11
WriteData(0x11); //P12
WriteData(0x0E); //P13
WriteData(0x17); //P14
WriteData(0x0F); //P15
WriteData(0x00); //P16
WriteComm(0x35); //Tearing Effect ON
WriteData(0x00);
WriteComm(0x36);
WriteData(0x22);//60
WriteComm(0x2A);
WriteData(0x00);
WriteData(0x00);
WriteData(0x03);
WriteData(0x55);
WriteComm(0xB5); //HBP
WriteData(0x14);
WriteData(0x14);
WriteData(0x2e);
WriteData(0x00);
WriteComm(0x2B);
WriteData(0x00);
WriteData(0x00);
WriteData(0x01);
WriteData(0xDF);
WriteComm(0x3A);
WriteData(0x77);
WriteComm(0xB6); // Interface for RAM access form System interface
WriteData(0xA2);// 0xa2 RGB sync mode; 0x22 RGB DE mode ; 0x02 system interface mode
WriteComm(0x11); //Exit Sleep
delay_ms(50);
WriteComm(0x29); // Display On
delay_ms(50);
//	WriteComm(0x3A); WriteData(0x77);
}
/******************************************
函数名：Lcd写命令函数
功能：向Lcd指定位置写入应有命令或数据
入口参数：Index 要寻址的寄存器地址
          ConfigTemp 写入的数据或命令值
******************************************/
void LCD_WR_REG(u16 Index,u16 CongfigTemp)
{
	WriteComm(Index);
	WriteData(CongfigTemp);
}

/**********************************************
函数名：Lcd块选函数
功能：选定Lcd上指定的矩形区域

注意：xStart、yStart、Xend、Yend随着屏幕的旋转而改变，位置是矩形框的四个角

入口参数：xStart x方向的起始点
          ySrart y方向的起始点
          Xend   y方向的终止点
          Yend   y方向的终止点
返回值：无
***********************************************/
void BlockWrite(unsigned int Xstart,unsigned int Xend,unsigned int Ystart,unsigned int Yend)
{

	WriteComm(0x2a);
	WriteData(Xstart>>8);
	WriteData(Xstart&0xff);
	WriteData(Xend>>8);
	WriteData(Xend&0xff);

	WriteComm(0x2b);
	WriteData(Ystart>>8);
	WriteData(Ystart&0xff);
	WriteData(Yend>>8);
	WriteData(Yend&0xff);

	WriteComm(0x2c);
}
/**********************************************
函数名：Lcd块选函数
功能：选定Lcd上指定的矩形区域

注意：xStart和 yStart随着屏幕的旋转而改变，位置是矩形框的四个角

入口参数：xStart x方向的起始点
          ySrart y方向的终止点
          xLong 要选定矩形的x方向长度
          yLong  要选定矩形的y方向长度
返回值：无
***********************************************/
void Lcd_ColorBox(u16 xStart,u16 yStart,u16 xLong,u16 yLong,u16 Color)
{
	u32 temp;

	BlockWrite(xStart,xStart+xLong-1,yStart,yStart+yLong-1);
	//IOWR_ALTERA_AVALON_SPI_CONTROL(LCD_SPI_BASE,0x400);
	for (temp=0; temp<xLong*yLong; temp++)
	{
		WriteData(Color>>8);
		WriteData(Color);
	}
	//IOWR_ALTERA_AVALON_SPI_CONTROL(LCD_SPI_BASE,0x000);
}

/******************************************
函数名：Lcd图像填充100*100
功能：向Lcd指定位置填充图像
入口参数：Index 要寻址的寄存器地址
          ConfigTemp 写入的数据或命令值
******************************************/
void LCD_Fill_Pic(u16 x, u16 y,u16 pic_H, u16 pic_V, const unsigned char* pic)
{
  unsigned long i;
// 	WriteComm(0x36); //Set_address_mode
// 	WriteData(0x0a); //横屏，从左下角开始，从左到右，从下到上
	BlockWrite(x,x+pic_H-1,y,y+pic_V-1);
	for (i = 0; i < pic_H*pic_V*2; i+=2)
	{
		WriteData(pic[i+1]);
		WriteData(pic[i]);

	}
// 	WriteComm(0x36); //Set_address_mode
// 	WriteData(0xaa);
}

void DrawPixel(u16 x, u16 y, u16 Color)
{
	BlockWrite(x,x+1,y,y+1);
	WriteData(Color>>8);
	WriteData(Color);
}

u16 ReadPixel(u16 x,u8 y)
{
	u16 dat;
	WriteComm(0x2a);
	WriteData(x>>8);
	WriteData(x&0xff);
	WriteComm(0x2b);
	WriteData(y>>8);
	WriteData(y&0xff);
	WriteComm(0x2e);

	//
	return dat;
}




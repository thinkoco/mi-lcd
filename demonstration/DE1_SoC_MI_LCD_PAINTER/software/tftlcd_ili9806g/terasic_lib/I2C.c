// --------------------------------------------------------------------
// Copyright (c) 2007 by Terasic Technologies Inc. 
// --------------------------------------------------------------------
//
// Permission:
//
//   Terasic grants permission to use and modify this code for use
//   in synthesis for all Terasic Development Boards and Altera Development 
//   Kits made by Terasic.  Other use of this code, including the selling 
//   ,duplication, or modification of any portion is strictly prohibited.
//
// Disclaimer:
//
//   This VHDL/Verilog or C/C++ source code is intended as a design reference
//   which illustrates how these types of functions can be implemented.
//   It is the user's responsibility to verify their design for
//   consistency and functionality through the use of formal
//   verification methods.  Terasic provides no warranty regarding the use 
//   or functionality of this code.
//
// --------------------------------------------------------------------
//           
//                     Terasic Technologies Inc
//                     356 Fu-Shin E. Rd Sec. 1. JhuBei City,
//                     HsinChu County, Taiwan
//                     302
//
//                     web: http://www.terasic.com/
//                     email: support@terasic.com
//
// --------------------------------------------------------------------
#include "terasic_includes.h"
#include "I2C.h"

// Note. Remember to reset device befroe acceess I2C interface
#ifdef DEBUG_I2C
    #define I2C_DEBUG(x)    DEBUG(x)  
#else
    #define I2C_DEBUG(x)
#endif

void my_wait(int nWait){
    int i;
    i = 0;
    while (i < nWait){
        i++;
     //   IOWR(LED_BASE, 0, i);
    }    
}


#define SDA_DIR_IN(data_base)   IOWR_ALTERA_AVALON_PIO_DIRECTION(data_base,0)
#define SDA_DIR_OUT(data_base)  IOWR_ALTERA_AVALON_PIO_DIRECTION(data_base,1) 
#define SDA_HIGH(data_base)     IOWR_ALTERA_AVALON_PIO_DATA(data_base, 1)
#define SDA_LOW(data_base)      IOWR_ALTERA_AVALON_PIO_DATA(data_base, 0)
#define SDA_READ(data_base)     IORD_ALTERA_AVALON_PIO_DATA(data_base)
#define SCL_HIGH(clk_base)     IOWR_ALTERA_AVALON_PIO_DATA(clk_base, 1)
#define SCL_LOW(clk_base)      IOWR_ALTERA_AVALON_PIO_DATA(clk_base, 0)

//#define SCL_DELAY    usleep(2)
//#define SCL_DELAY   my_wait(22) // 20~25 ok at 120 MHZ,  15~20 ok relative to cpu clock 100MHZ
//#define SCL_DELAY   my_wait(30) // 20~25 ok at 120 MHZ,  15~20 ok relative to cpu clock 100MHZ

#define SCL_DELAY usleep(30)

void i2c_start(alt_u32 clk_base, alt_u32 data_base);
void i2c_stop(alt_u32 clk_base, alt_u32 data_base);
bool i2c_write(alt_u32 clk_base, alt_u32 data_base, alt_u8 Data);
void i2c_read(alt_u32 clk_base, alt_u32 data_base, alt_u8 *pData, bool bAck);



bool I2C_Write(alt_u32 clk_base, alt_u32 data_base, alt_8 DeviceAddr, alt_u8 ControlAddr, alt_u8 ControlData){
    bool bSuccess = TRUE;
    //alt_u8 DeviceAddr;
    
    // device id
    //DeviceAddr = HMB_E2_I2C_ID;
    i2c_start(clk_base, data_base);
    if (!i2c_write(clk_base, data_base, DeviceAddr)){  // send ID
        bSuccess = FALSE;
        I2C_DEBUG(("I2C HMB_E2 Fail: Address NACK!\n"));
    }
    if (bSuccess && !i2c_write(clk_base, data_base, ControlAddr)){ // send sub-address
        bSuccess = FALSE;
        I2C_DEBUG(("I2C HMB_E2 Fail: SubAddress NACK!\n"));
    }            
    if (bSuccess && !i2c_write(clk_base, data_base, ControlData)){  
        bSuccess = FALSE;
        I2C_DEBUG(("I2C HMB_E2 Fail: write NACK!\n"));
    }
    i2c_stop(clk_base, data_base);
    
    usleep(7*100); // delay to wait EE2 ready (at least 5 ms delay is required)
    
    return bSuccess;

    
}

bool I2C_Write16(alt_u32 clk_base, alt_u32 data_base, alt_8 DeviceAddr, alt_u8 ControlAddr, alt_u16 ControlData){
    bool bSuccess = TRUE;
    alt_u8 HighByte, LowByte;
    //alt_u8 DeviceAddr;
    HighByte = (ControlData >> 8) & 0xFF;
    LowByte = ControlData & 0xFF; 
    
    // device id
    //DeviceAddr = HMB_E2_I2C_ID;

    i2c_start(clk_base, data_base);
    if (!i2c_write(clk_base, data_base, DeviceAddr)){  // send ID
        bSuccess = FALSE;
        I2C_DEBUG(("I2C HMB_E2 Fail: Address NACK!\n"));
    }
    if (bSuccess && !i2c_write(clk_base, data_base, ControlAddr)){ // send sub-address
        bSuccess = FALSE;
        I2C_DEBUG(("I2C HMB_E2 Fail: SubAddress NACK!\n"));
    }            
    if (bSuccess && !i2c_write(clk_base, data_base, HighByte)){  
        bSuccess = FALSE;
        I2C_DEBUG(("I2C HMB_E2 Fail: write NACK!\n"));
    }
    if (bSuccess && !i2c_write(clk_base, data_base, LowByte)){  
        bSuccess = FALSE;
        I2C_DEBUG(("I2C HMB_E2 Fail: write NACK!\n"));
    }    
    i2c_stop(clk_base, data_base);
    
    usleep(7*1000); // delay to wait EE2 ready (at least 5 ms delay is required)
    
    return bSuccess;
}

bool I2C_Read(alt_u32 clk_base, alt_u32 data_base, alt_8 DeviceAddr, alt_u8 ControlAddr, alt_u8 *pControlData){
    bool bSuccess = TRUE;
    //alt_u8 DeviceAddr;
   
    // device id
    //DeviceAddr = HMB_E2_I2C_ID;
    if (!i2c_write(clk_base, data_base, DeviceAddr)){  // send ID
        bSuccess = FALSE;
        I2C_DEBUG(("I2C_Read(1) Fail: Address NACK!\n"));
    }
    if (bSuccess && !i2c_write(clk_base, data_base, ControlAddr)){ // send sub-address
        bSuccess = FALSE;
        I2C_DEBUG(("I2C_Read(2) Fail: SubAddress NACK!\n"));
    }            
    i2c_start(clk_base, data_base);  // restart
    DeviceAddr |= 1; // Read
    if (bSuccess && !i2c_write(clk_base, data_base, DeviceAddr)){  // send id
        bSuccess = FALSE;
        I2C_DEBUG(("I2C_Read(3) Fail: Address+1 NACK!\n"));
    }
    
    if (bSuccess){
        i2c_read(clk_base, data_base, pControlData, FALSE);  // read
    }        
    i2c_stop(clk_base, data_base);
    
    return bSuccess;
}

bool I2C_Read16(alt_u32 clk_base, alt_u32 data_base, alt_8 DeviceAddr, alt_u8 ControlAddr, alt_u16 *pControlData){
    bool bSuccess = TRUE;
    alt_u8 LowByte, HighByte;
    //alt_u8 DeviceAddr;
   
    // device id
    //DeviceAddr = HMB_E2_I2C_ID;
    i2c_start(clk_base, data_base);
    if (!i2c_write(clk_base, data_base, DeviceAddr)){  // send ID
        bSuccess = FALSE;
        I2C_DEBUG(("I2C HMB_E2 Fail: Address NACK!\n"));
    }
    if (bSuccess && !i2c_write(clk_base, data_base, ControlAddr)){ // send sub-address
        bSuccess = FALSE;
        I2C_DEBUG(("I2C HMB_E2 Fail: SubAddress NACK!\n"));
    }            
    i2c_start(clk_base, data_base);  // restart
    DeviceAddr |= 1; // Read
    if (bSuccess && !i2c_write(clk_base, data_base, DeviceAddr)){  // send id
        bSuccess = FALSE;
        I2C_DEBUG(("I2C HMB_E2 Fail: Address+1 NACK!\n"));
    }
    
    if (bSuccess){
        i2c_read(clk_base, data_base, &HighByte, TRUE);  // read
    }        
    if (bSuccess){
        i2c_read(clk_base, data_base, &LowByte, FALSE);  // read
    }        
    if (bSuccess)
        *pControlData = (HighByte << 8) | LowByte;
    i2c_stop(clk_base, data_base);
    return bSuccess;
}


bool I2C_MultipleRead(alt_u32 clk_base, alt_u32 data_base, alt_8 DeviceAddr, alt_u8 ControlAddr, alt_u8 *pBuf, alt_u16 len){
    int i;
    bool bSuccess = TRUE;
    // device id
    //DeviceAddr = HMB_E2_I2C_ID;
    i2c_start(clk_base, data_base);
    if (!i2c_write(clk_base, data_base, DeviceAddr)){  // send ID
        bSuccess = FALSE;
        I2C_DEBUG(("I2C_MultipleRead(1) Fail: Address NACK!\n"));
    }    
    if (bSuccess && !i2c_write(clk_base, data_base, ControlAddr)){ // send sub-address
        bSuccess = FALSE;
        I2C_DEBUG(("I2C_MultipleRead(2) Fail: SubAddress NACK!\n"));
    }    
    if (bSuccess)        
        i2c_start(clk_base, data_base);  // restart
    DeviceAddr |= 1; // Read
    if (bSuccess && !i2c_write(clk_base, data_base, DeviceAddr)){  // send id
        bSuccess = FALSE;
        I2C_DEBUG(("I2C_MultipleRead(3) Fail: Address+1 NACK!\n"));
    }
    
    if (bSuccess){
        for(i=0;i<len && bSuccess;i++){
            i2c_read(clk_base, data_base, pBuf, (i==(len-1))?FALSE:TRUE);  // read
            pBuf++;
        }            
    }        
    i2c_stop(clk_base, data_base);
    
    return bSuccess;    
    
}




/////////////////////////////////////////////////////////////////////////////////////////////
///////////// Interncal function (i2cXXX) body //////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////


//SDA 1->0 while SCL=1
void i2c_start(alt_u32 clk_base, alt_u32 data_base){
    
    // make sure it is in normal state
    SDA_DIR_OUT(data_base);  // data output enabled
    // start condition
    SDA_HIGH(data_base); // data high
    SCL_HIGH(clk_base);
    SCL_DELAY;
    SDA_LOW(data_base); // data low
    SCL_DELAY; 
    SCL_LOW(clk_base); // clock low
    SCL_DELAY;
}

// SDA 0->1 while SCL=1
void i2c_stop(alt_u32 clk_base, alt_u32 data_base){
    // assume SCL = 0
    
    SDA_DIR_OUT(data_base);  // data output enabled
    SDA_LOW(data_base); // Data Low
    SCL_DELAY;
    SCL_HIGH(clk_base);  // clock high
    SCL_DELAY; // clock high long delay
    SDA_HIGH(data_base); // data high
    SCL_DELAY; // data high delay
 //   SDA_DIR_IN(data_base);  // data read modes
}

bool i2c_write(alt_u32 clk_base, alt_u32 data_base, alt_u8 Data){ // return true if device response ack
    alt_u8 Mask = 0x80;
    bool bAck;
    int i;
    int w;
    
    // assume, SCL = 0
    
    SDA_DIR_OUT(data_base);  // data write mode
    
    for(i=0;i<8;i++){
        SCL_LOW(clk_base);  // new, make sure data change at clk low
        // output data on bus
        if (Data & Mask){ // there is a delay in this command
            SDA_HIGH(data_base);
        }else{    
            SDA_LOW(data_base);
        }
        Mask >>= 1; // there is a delay in this command
        // clock high
        SCL_DELAY;
        SCL_HIGH(clk_base);
        SCL_DELAY;
        SCL_LOW(clk_base);
        SCL_DELAY;
    }
    
    //===== get ack
    SDA_DIR_IN(data_base);  // data read mode
    //SCL_DELAY;
//------------------ wait ack for muti-touch panel
    SCL_DELAY;

//	w = 0;
//	while(SDA_READ(data_base) && w++ < 1000000){
//	}


    // clock high
    SCL_HIGH(clk_base);  // clock high
    SCL_DELAY;  // clock high delay
    SCL_DELAY;  // clock high delay
    bAck = SDA_READ(data_base)?FALSE:TRUE;  // get ack
    //SCL_DELAY;
    //SDA_DIR_OUT;
    SCL_LOW(clk_base); // clock low         
    SCL_DELAY; // clock low delay
    return bAck;
}    

void i2c_read(alt_u32 clk_base, alt_u32 data_base, alt_u8 *pData, bool bAck){ // return true if device response ack
    alt_u8 Data=0;
    int i;
    
    // assume SCL = low
    SDA_DIR_IN(data_base);  // set data read mode
    SCL_LOW(clk_base); // clock low
    SCL_DELAY; // clock low delay

    for(i=0;i<8;i++){
        Data <<= 1;
        SCL_HIGH(clk_base);  // clock high
        SCL_DELAY;
        if (SDA_READ(data_base))  // read data   
            Data |= 0x01;
        SCL_LOW(clk_base);  // clock log  
        SCL_DELAY;
    }
    // send ACK
    SCL_LOW(clk_base);  // new, make sure data change at clk low
    SDA_DIR_OUT(data_base);  // set data write mode
    if (bAck)
        SDA_LOW(data_base);
    else
        SDA_HIGH(data_base);
    SCL_HIGH(clk_base); // clock high
    SCL_DELAY; // clock high  delay
    SCL_LOW(clk_base); // clock low
    SCL_DELAY; // clock low delay
    SDA_LOW(data_base);  // data low
    SCL_DELAY; // data low delay
//    SDA_DIR_IN;  // set data read mode
    *pData = Data;
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
// size > 2Kbits
bool I2CL_Write(alt_u32 clk_base, alt_u32 data_base, alt_8 DeviceAddr, alt_u16 ControlAddr, alt_u8 ControlData){
    bool bSuccess = TRUE;
    alt_u8 AddrHigh, AddrLow;
    
    AddrHigh = (ControlAddr >> 8) & 0xFF; 
    AddrLow  = ControlAddr & 0xFF; 

    i2c_start(clk_base, data_base);
    if (!i2c_write(clk_base, data_base, DeviceAddr)){  // send ID
        bSuccess = FALSE;
        I2C_DEBUG(("I2C HMB_E2 Fail: Address NACK!\n"));
    }
    if (bSuccess && !i2c_write(clk_base, data_base, AddrHigh)){ // send sub-address
        bSuccess = FALSE;
        I2C_DEBUG(("I2C HMB_E2 Fail: SubAddress-high NACK!\n"));
    }            
    if (bSuccess && !i2c_write(clk_base, data_base, AddrLow)){ // send sub-address
        bSuccess = FALSE;
        I2C_DEBUG(("I2C HMB_E2 Fail: SubAddress-low NACK!\n"));
    }            
    if (bSuccess && !i2c_write(clk_base, data_base, ControlData)){  
        bSuccess = FALSE;
        I2C_DEBUG(("I2C HMB_E2 Fail: write NACK!\n"));
    }
    i2c_stop(clk_base, data_base);
    
    usleep(7*1000); // delay to wait EE2 ready (at least 5 ms delay is required)
    
    return bSuccess;
    
}

bool I2CL_Read(alt_u32 clk_base, alt_u32 data_base, alt_8 DeviceAddr, alt_u16 ControlAddr, alt_u8 *pControlData){
    bool bSuccess = TRUE;
    alt_u8 AddrHigh, AddrLow;
    
    AddrHigh = (ControlAddr >> 8) & 0xFF; 
    AddrLow  = ControlAddr & 0xFF; 

    i2c_start(clk_base, data_base);
    if (!i2c_write(clk_base, data_base, DeviceAddr)){  // send ID
        bSuccess = FALSE;
        I2C_DEBUG(("I2C HMB_E2 Fail: Address NACK!\n"));
    }
    if (bSuccess && !i2c_write(clk_base, data_base, AddrHigh)){ // send sub-address
        bSuccess = FALSE;
        I2C_DEBUG(("I2C HMB_E2 Fail: SubAddress-high NACK!\n"));
    }            
    if (bSuccess && !i2c_write(clk_base, data_base, AddrLow)){ // send sub-address
        bSuccess = FALSE;
        I2C_DEBUG(("I2C HMB_E2 Fail: SubAddress-low NACK!\n"));
    }            
    i2c_start(clk_base, data_base);  // restart
    DeviceAddr |= 1; // Read
    if (bSuccess && !i2c_write(clk_base, data_base, DeviceAddr)){  // send id
        bSuccess = FALSE;
        I2C_DEBUG(("I2C HMB_E2 Fail: Address+1 NACK!\n"));
    }
    
    if (bSuccess){
        i2c_read(clk_base, data_base, pControlData, FALSE);  // read
    }        
    i2c_stop(clk_base, data_base);
    
    return bSuccess;    
}

// read from address 0
bool I2CL_MultipleRead(alt_u32 clk_base, alt_u32 data_base, alt_8 DeviceAddr, alt_u16 ControlAddr, alt_u8 szData[], alt_u16 len){
    int i;
    bool bSuccess = TRUE;
  // alt_u8 ControlAddr = 0;
    alt_u8 AddrHigh, AddrLow;
    
    AddrHigh = (ControlAddr >> 8) & 0xFF; 
    AddrLow  = ControlAddr & 0xFF;     
    
   
    // device id
    //DeviceAddr = HMB_E2_I2C_ID;

    i2c_start(clk_base, data_base);
    if (!i2c_write(clk_base, data_base, DeviceAddr)){  // send ID
        bSuccess = FALSE;
        I2C_DEBUG(("I2C HMB_E2 Fail: Address NACK!\n"));
    }
    if (bSuccess && !i2c_write(clk_base, data_base, AddrHigh)){ // send sub-address
        bSuccess = FALSE;
        I2C_DEBUG(("I2C HMB_E2 Fail: SubAddress-high NACK!\n"));
    }    
    if (bSuccess && !i2c_write(clk_base, data_base, AddrLow)){ // send sub-address
        bSuccess = FALSE;
        I2C_DEBUG(("I2C HMB_E2 Fail: SubAddress-low NACK!\n"));
    }    
    if (bSuccess)        
        i2c_start(clk_base, data_base);  // restart
    DeviceAddr |= 1; // Read
    if (bSuccess && !i2c_write(clk_base, data_base, DeviceAddr)){  // send id
        bSuccess = FALSE;
        I2C_DEBUG(("I2CL_MultipleRead Fail: Address+1 NACK!\n"));
    }
    
    if (bSuccess){
        for(i=0;i<len && bSuccess;i++){
            i2c_read(clk_base, data_base, &szData[i], (i==(len-1))?FALSE:TRUE);  // read
        }            
    }        
    i2c_stop(clk_base, data_base);
    
    return bSuccess;    
    
} 



bool I2CL_MultipleWrite(alt_u32 clk_base, alt_u32 data_base, alt_8 DeviceAddr, alt_u16 ControlAddr, alt_u8 szData[], alt_u16 len){
    bool bSuccess = TRUE;
    alt_u8 AddrHigh, AddrLow;
    int i;
    
    AddrHigh = (ControlAddr >> 8) & 0xFF; 
    AddrLow  = ControlAddr & 0xFF; 
    

    i2c_start(clk_base, data_base);
    if (!i2c_write(clk_base, data_base, DeviceAddr)){  // send ID
        bSuccess = FALSE;
        I2C_DEBUG(("I2C HMB_E2 Fail: Address NACK!\n"));
    }
    if (bSuccess && !i2c_write(clk_base, data_base, AddrHigh)){ // send sub-address
        bSuccess = FALSE;
        I2C_DEBUG(("I2C HMB_E2 Fail: SubAddress-high NACK!\n"));
    }            
    if (bSuccess && !i2c_write(clk_base, data_base, AddrLow)){ // send sub-address
        bSuccess = FALSE;
        I2C_DEBUG(("I2C HMB_E2 Fail: SubAddress-low NACK!\n"));
    }            
    if (bSuccess){// && !i2c_write(clk_base, data_base, ControlData)){
        for(i=0;i<len && bSuccess;i++){
            bSuccess = i2c_write(clk_base, data_base, szData[i]);
        }  
        //bSuccess = FALSE;
        if (!bSuccess)
            I2C_DEBUG(("I2C HMB_E2 Fail: write NACK!\n"));
    }
    i2c_stop(clk_base, data_base);
    
    usleep(7*1000); // delay to wait EE2 ready (at least 5 ms delay is required)
    
    return bSuccess;
    
}
 


bool I2CL_Read16(alt_u32 clk_base, alt_u32 data_base, alt_8 DeviceAddr, alt_u16 ControlAddr, alt_u16 *pControlData){
    bool bSuccess;
    alt_u8 szData[2];
    bSuccess = I2CL_MultipleRead(clk_base, data_base, DeviceAddr, ControlAddr, szData, sizeof(szData));
    if (bSuccess){
        *pControlData = (szData[0] << 8 ) | szData[1];
    }
    return bSuccess;    
}

bool I2CL_Write16(alt_u32 clk_base, alt_u32 data_base, alt_8 DeviceAddr, alt_u16 ControlAddr, alt_u16 ControlData){
    bool bSuccess;
    alt_u8 szData[2];
    szData[0] = (ControlData >> 8) & 0xFF;
    szData[1] = ControlData & 0xFF;   
    bSuccess = I2CL_MultipleWrite(clk_base, data_base, DeviceAddr, ControlAddr, szData, sizeof(szData));
    return bSuccess;    
}


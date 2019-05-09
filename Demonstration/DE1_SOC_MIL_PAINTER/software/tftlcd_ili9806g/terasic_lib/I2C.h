// --------------------------------------------------------------------
// Copyright (c) 2008 by Terasic Technologies Inc. 
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
#ifndef I2C_H_
#define I2C_H_

// address device for DE2-115 board
#define I2C_TV_ADDR        0x40
#define I2C_AUDIO_ADDR     0x34
#define I2C_EEPROM_ADDR    0xA0
#define I2C_FT5316_ADDR    0x70

// size <= 2Kbits (256 Bytes)
bool I2C_Write(alt_u32 clk_base, alt_u32 data_base, alt_8 DeviceAddr, alt_u8 ControlAddr, alt_u8 ControlData);
bool I2C_Read(alt_u32 clk_base, alt_u32 data_base, alt_8 DeviceAddr, alt_u8 ControlAddr, alt_u8 *pControlData);
//bool I2C_MultipleRead(alt_u32 clk_base, alt_u32 data_base, alt_8 DeviceAddr, alt_u8 szData[], alt_u16 len); // read from address 0
bool I2C_MultipleRead(alt_u32 clk_base, alt_u32 data_base, alt_8 DeviceAddr, alt_u8 ControlAddr, alt_u8 *pBuf, alt_u16 len);

bool I2C_Write16(alt_u32 clk_base, alt_u32 data_base, alt_8 DeviceAddr, alt_u8 ControlAddr, alt_u16 ControlData);
bool I2C_Read16(alt_u32 clk_base, alt_u32 data_base, alt_8 DeviceAddr, alt_u8 ControlAddr, alt_u16 *pControlData);


// size > 2Kbits
bool I2CL_Write(alt_u32 clk_base, alt_u32 data_base, alt_8 DeviceAddr, alt_u16 ControlAddr, alt_u8 ControlData);
bool I2CL_Read(alt_u32 clk_base, alt_u32 data_base, alt_8 DeviceAddr, alt_u16 ControlAddr, alt_u8 *pControlData);
bool I2CL_MultipleRead(alt_u32 clk_base, alt_u32 data_base, alt_8 DeviceAddr, alt_u16 ControlAddr, alt_u8 szData[], alt_u16 len);
bool I2CL_MultipleWrite(alt_u32 clk_base, alt_u32 data_base, alt_8 DeviceAddr, alt_u16 ControlAddr, alt_u8 szData[], alt_u16 len); 

bool I2CL_Read16(alt_u32 clk_base, alt_u32 data_base, alt_8 DeviceAddr, alt_u16 ControlAddr, alt_u16 *pControlData);
bool I2CL_Write16(alt_u32 clk_base, alt_u32 data_base, alt_8 DeviceAddr, alt_u16 ControlAddr, alt_u16 ControlData);



#endif /*I2C_H_*/

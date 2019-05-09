/*
 * I2C_ocre.h
 *
 *  Created on: 2014-6-3
 *      Author: niubility
 */

#ifndef I2C_OCRE_H_
#define I2C_OCRE_H_
#include "alt_types.h"  // alt_u32
#include "terasic_includes.h"


bool  oc_i2c_init(alt_32 i2c_base);
bool  ACK_single_check(alt_32 i2c_base);
bool  ACK_check(alt_32 i2c_base);
bool  OC_I2C_Write(alt_32 i2c_base,alt_u8 device_address,alt_u8 sub_address, alt_u8 *pData, int nWriteLength);
bool  ACK_judge_for_read(alt_32 i2c_base);
bool  OC_I2C_Read(alt_32 i2c_base,alt_u8 device_address,alt_u8 sub_address, alt_u8 *pData8, int nReadLength);
bool  OC_I2CL_Write(alt_32 i2c_base,alt_u8 device_address,int sub_address,alt_u8 *pData, int nWriteLength);
bool  OC_I2CL_Read(alt_32 i2c_base,alt_u8 device_address,int sub_address, alt_u8 *pData8);

#endif /* I2C_OCRE_H_ */

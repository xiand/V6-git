/******************************************************************************

  Copyright (C), 2008-2018, YXROBOT Co., Ltd.

 ******************************************************************************
  File Name     : I2C_module_simulate.h
  Version       : Initial Draft
  Author        : STaoJ
  Created       : 2018/7/6
  Last Modified : 2018/7/6
  Description   : I2C_module_simulate.c header file
  				  1.使用本函数，需要对I2C_PORT_STRU	I2C_Port_Use进行配置
  				  2.如果只有一个I2C器件端口，使能Define USE_ONLY_ONE_I2C_PORT
  				  3.如果有多个I2C器件端口，则可以直接定义多个PORT，并在调用函数前将器
                件PORT设置为对应端口
                  4.
  Function List :
  History       :
  1.Date        : 2018/7/6
    Author      : STaoJ
    Modification: Created file

******************************************************************************/
#ifndef __I2C_MODULE_SIMULATE_H__
#define __I2C_MODULE_SIMULATE_H__
		 
		 
#ifdef __cplusplus
#if __cplusplus
		 extern "C"{
#endif
#endif /* __cplusplus */

#include "stdint.h"
#include "stm32f4xx.h"


/*----------------------------------------------*
 * user manual                   				*
 *----------------------------------------------*/
//#define USE_MORE_THAN_ONE_I2C_PORT			// 使用多个I2C端口
#ifndef USE_MORE_THAN_ONE_I2C_PORT
	#define USE_ONLY_ONE_I2C_PORT				// 使用一个I2C端口
#endif


typedef enum I2C_DEVICE
{
	Use_I2c_None = 0,
	Use_I2c_1 = 1,
	Use_I2c_2,
	Use_I2c_3,
	Use_I2c_4
}I2C_DEVICE_ENUM;			// I2C_器件端口的个数

typedef enum I2C_ADDR_TYPE
{
	I2C_ADDR_Byte,
	I2C_ADDR_Word
}I2C_ADDR_TYPE_ENUM;

#define USE_I2C_MPU6050					Use_I2c_1
#define USE_I2C_MAG						Use_I2c_2		// 使用多个I2C模拟端口时，在这里配置，并在"板载多个I2C端口配置"中设置对应端口
//#define USE_I2C_xxxxxx				Use_I2c_3
//#define USE_I2C_xxxxxx				Use_I2c_4		// 最多4个，否则需要更改.c文件I2C_Port_Set()



// 如果使用多个端口，则定义USE_MORE_THAN_ONE_I2C_PORT，
// 然后在下面defined (USE_MORE_THAN_ONE_I2C_PORT)中配置好对应的端口
//#if defined (USE_ONLY_ONE_I2C_PORT)				// 板载1个I2C端口
// I2C 端口1配置
#define I2C_1_SCL_PORT			MPU6050_I2C_SCL_PORT
#define I2C_1_SCL_PIN			MPU6050_I2C_SCL_PIN		// SCL需要配置成PP输出
#define I2C_1_SDA_PORT			MPU6050_I2C_SDA_PORT
#define I2C_1_SDA_PIN			MPU6050_I2C_SDA_PIN		// SDA需要配置成OD输出
#define I2C_1_ADDR_TYPE			MPU6050_SLAVE_ADDRESS

#ifdef USE_MORE_THAN_ONE_I2C_PORT		// 板载多个I2C端口配置
// I2C 端口2配置
#define I2C_2_SCL_PORT			PORT_SCL_MAG
#define I2C_2_SCL_PIN			PIN_SCL_MAG			// SCL需要配置成PP输出
#define I2C_2_SDA_PORT			PORT_SDA_MAG
#define I2C_2_SDA_PIN			PIN_SDA_MAG			// SDA需要配置成OD输出
#define I2C_2_ADDR_TYPE			I2C_ADDR_Byte

// I2C 端口3配置
#define I2C_3_SCL_PORT			I2C_2_SCL_PORT
#define I2C_3_SCL_PIN			I2C_2_SCL_PIN		// SCL需要配置成PP输出
#define I2C_3_SDA_PORT			I2C_2_SDA_PORT
#define I2C_3_SDA_PIN			I2C_2_SDA_PIN		// SDA需要配置成OD输出
#define I2C_3_ADDR_TYPE			I2C_ADDR_Byte

// I2C 端口4配置
#define I2C_4_SCL_PORT			I2C_3_SCL_PORT
#define I2C_4_SCL_PIN			I2C_3_SCL_PIN		// SCL需要配置成PP输出
#define I2C_4_SDA_PORT			I2C_3_SDA_PORT
#define I2C_4_SDA_PIN			I2C_3_SDA_PIN		// SDA需要配置成OD输出
#define I2C_4_ADDR_TYPE			I2C_ADDR_Byte

// 如果有更多端口，请在这里增加，并更改I2C_Port_Set()的配置
#endif


/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/

/*----------------------------------------------*
 * external routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * internal routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * project-wide global variables                *
 *----------------------------------------------*/

/*----------------------------------------------*
 * module-wide global variables                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * constants                                    *
 *----------------------------------------------*/

/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/
#define I2C_OK					0x00			// I2C执行成功
#define I2C_ERR					0x80			// I2C执行失败

void I2C_Gpio_Init(void);


/*****************************************************************************
 Prototype    : I2C_Module_Write
 Description  : write byte to the specific register
 Input        : I2C_DEVICE_ENUM I2C_Port 端口定义的端口，1-4
 				uint8_t bySlave  器件从地址，为器件写地址，读时会自动或上0x01
                uint8_t byAddr   芯片内部读取的起始地址
                uint8_t *pDate   读取数据的存放位置
                uint8_t byNum    读取数据的数量
 Output       : None
 Return Value : uint8_t I2C_OK:执行成功， I2C_ERR:执行失败， 
 Calls        : 
 Called By    : 需要读I2C器件的函数
*****************************************************************************/
extern uint8_t I2C_Module_Write(I2C_DEVICE_ENUM I2C_Port, uint8_t bySlave, 
										uint16_t wAddr, uint8_t *pBuf, uint8_t byNum);

/*****************************************************************************
 Prototype    : I2C_Module_Read
 Description  : write byte to the specific register
 Input        : I2C_DEVICE_ENUM I2C_Port 端口定义的端口，1-4
 				uint8_t bySlave  器件从地址，为器件写地址，读时会自动或上0x01
                uint8_t byAddr   芯片内部写入的起始地址
                uint8_t *pBuf    写入数据的存放位置
                uint8_t byNum    写入数据的数量
 Output       : None
 Return Value : uint8_t I2C_OK:执行成功， I2C_ERR:执行失败， 
 Calls        : 
 Called By    : 需要写I2C器件的函数
*****************************************************************************/
extern uint8_t I2C_Module_Read(I2C_DEVICE_ENUM I2C_Port, uint8_t bySlave, 
										uint16_t wAddr, uint8_t *pData, uint8_t byNum);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __I2C_MODULE_SIMULATE_H__ */


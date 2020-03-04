/******************************************************************************

  Copyright (C), 2008-2018, YXROBOT Co., Ltd.

 ******************************************************************************
  File Name     : I2C_module_simulate.h
  Version       : Initial Draft
  Author        : STaoJ
  Created       : 2018/7/6
  Last Modified : 2018/7/6
  Description   : I2C_module_simulate.c header file
  				  1.ʹ�ñ���������Ҫ��I2C_PORT_STRU	I2C_Port_Use��������
  				  2.���ֻ��һ��I2C�����˿ڣ�ʹ��Define USE_ONLY_ONE_I2C_PORT
  				  3.����ж��I2C�����˿ڣ������ֱ�Ӷ�����PORT�����ڵ��ú���ǰ����
                ��PORT����Ϊ��Ӧ�˿�
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
//#define USE_MORE_THAN_ONE_I2C_PORT			// ʹ�ö��I2C�˿�
#ifndef USE_MORE_THAN_ONE_I2C_PORT
	#define USE_ONLY_ONE_I2C_PORT				// ʹ��һ��I2C�˿�
#endif


typedef enum I2C_DEVICE
{
	Use_I2c_None = 0,
	Use_I2c_1 = 1,
	Use_I2c_2,
	Use_I2c_3,
	Use_I2c_4
}I2C_DEVICE_ENUM;			// I2C_�����˿ڵĸ���

typedef enum I2C_ADDR_TYPE
{
	I2C_ADDR_Byte,
	I2C_ADDR_Word
}I2C_ADDR_TYPE_ENUM;

#define USE_I2C_MPU6050					Use_I2c_1
#define USE_I2C_MAG						Use_I2c_2		// ʹ�ö��I2Cģ��˿�ʱ�����������ã�����"���ض��I2C�˿�����"�����ö�Ӧ�˿�
//#define USE_I2C_xxxxxx				Use_I2c_3
//#define USE_I2C_xxxxxx				Use_I2c_4		// ���4����������Ҫ����.c�ļ�I2C_Port_Set()



// ���ʹ�ö���˿ڣ�����USE_MORE_THAN_ONE_I2C_PORT��
// Ȼ��������defined (USE_MORE_THAN_ONE_I2C_PORT)�����úö�Ӧ�Ķ˿�
//#if defined (USE_ONLY_ONE_I2C_PORT)				// ����1��I2C�˿�
// I2C �˿�1����
#define I2C_1_SCL_PORT			MPU6050_I2C_SCL_PORT
#define I2C_1_SCL_PIN			MPU6050_I2C_SCL_PIN		// SCL��Ҫ���ó�PP���
#define I2C_1_SDA_PORT			MPU6050_I2C_SDA_PORT
#define I2C_1_SDA_PIN			MPU6050_I2C_SDA_PIN		// SDA��Ҫ���ó�OD���
#define I2C_1_ADDR_TYPE			MPU6050_SLAVE_ADDRESS

#ifdef USE_MORE_THAN_ONE_I2C_PORT		// ���ض��I2C�˿�����
// I2C �˿�2����
#define I2C_2_SCL_PORT			PORT_SCL_MAG
#define I2C_2_SCL_PIN			PIN_SCL_MAG			// SCL��Ҫ���ó�PP���
#define I2C_2_SDA_PORT			PORT_SDA_MAG
#define I2C_2_SDA_PIN			PIN_SDA_MAG			// SDA��Ҫ���ó�OD���
#define I2C_2_ADDR_TYPE			I2C_ADDR_Byte

// I2C �˿�3����
#define I2C_3_SCL_PORT			I2C_2_SCL_PORT
#define I2C_3_SCL_PIN			I2C_2_SCL_PIN		// SCL��Ҫ���ó�PP���
#define I2C_3_SDA_PORT			I2C_2_SDA_PORT
#define I2C_3_SDA_PIN			I2C_2_SDA_PIN		// SDA��Ҫ���ó�OD���
#define I2C_3_ADDR_TYPE			I2C_ADDR_Byte

// I2C �˿�4����
#define I2C_4_SCL_PORT			I2C_3_SCL_PORT
#define I2C_4_SCL_PIN			I2C_3_SCL_PIN		// SCL��Ҫ���ó�PP���
#define I2C_4_SDA_PORT			I2C_3_SDA_PORT
#define I2C_4_SDA_PIN			I2C_3_SDA_PIN		// SDA��Ҫ���ó�OD���
#define I2C_4_ADDR_TYPE			I2C_ADDR_Byte

// ����и���˿ڣ������������ӣ�������I2C_Port_Set()������
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
#define I2C_OK					0x00			// I2Cִ�гɹ�
#define I2C_ERR					0x80			// I2Cִ��ʧ��

void I2C_Gpio_Init(void);


/*****************************************************************************
 Prototype    : I2C_Module_Write
 Description  : write byte to the specific register
 Input        : I2C_DEVICE_ENUM I2C_Port �˿ڶ���Ķ˿ڣ�1-4
 				uint8_t bySlave  �����ӵ�ַ��Ϊ����д��ַ����ʱ���Զ�����0x01
                uint8_t byAddr   оƬ�ڲ���ȡ����ʼ��ַ
                uint8_t *pDate   ��ȡ���ݵĴ��λ��
                uint8_t byNum    ��ȡ���ݵ�����
 Output       : None
 Return Value : uint8_t I2C_OK:ִ�гɹ��� I2C_ERR:ִ��ʧ�ܣ� 
 Calls        : 
 Called By    : ��Ҫ��I2C�����ĺ���
*****************************************************************************/
extern uint8_t I2C_Module_Write(I2C_DEVICE_ENUM I2C_Port, uint8_t bySlave, 
										uint16_t wAddr, uint8_t *pBuf, uint8_t byNum);

/*****************************************************************************
 Prototype    : I2C_Module_Read
 Description  : write byte to the specific register
 Input        : I2C_DEVICE_ENUM I2C_Port �˿ڶ���Ķ˿ڣ�1-4
 				uint8_t bySlave  �����ӵ�ַ��Ϊ����д��ַ����ʱ���Զ�����0x01
                uint8_t byAddr   оƬ�ڲ�д�����ʼ��ַ
                uint8_t *pBuf    д�����ݵĴ��λ��
                uint8_t byNum    д�����ݵ�����
 Output       : None
 Return Value : uint8_t I2C_OK:ִ�гɹ��� I2C_ERR:ִ��ʧ�ܣ� 
 Calls        : 
 Called By    : ��ҪдI2C�����ĺ���
*****************************************************************************/
extern uint8_t I2C_Module_Read(I2C_DEVICE_ENUM I2C_Port, uint8_t bySlave, 
										uint16_t wAddr, uint8_t *pData, uint8_t byNum);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __I2C_MODULE_SIMULATE_H__ */


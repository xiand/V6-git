
/******************************************************************************

  Copyright (C), 2008-2018, YXROBOT Co., Ltd.

 ******************************************************************************
  File Name     : I2C_module_simulate.c
  Version       : Initial Draft
  Author        : STaoJ
  Created       : 2018/7/6
  Last Modified :
  Description   : I2Cģ��ģ����򣬿�����ΧI2C����
  Function List :
              I2C_Delay
              I2C_Master_Ack
              I2C_No_Ack
              I2C_Read_Byte
              I2C_Slave_Ack
              I2C_Start
              I2C_Stop
              I2C_Write_Byte
			  I2C_Module_Readbyte
			  I2C_Module_Writebyte
  History       :
  1.Date        : 2018/7/6
    Author      : STaoJ
    Modification: Created file
    
  2.Date		: 2018/11/20
    Author	    : suntaojun
    Modification: ȡ���������д��ӵ�ַ��Ĵ�����ַ���ֹͣ��������һ����ʼ������

  3.Date        : 2018/12/13
    Author      : suntaojun
    Modification: �Ż�I2C_Delay(),���������ȴ�ʱ��ӵ�10�룬�����ЩIST8305�޷�
    	��ȡ���ݵ����⣬���ǻᵼ����������Ҫ��ʱ��ô��ʱ�����������ʱ�䡣

******************************************************************************/
//#include "bsp_port_define.h"
#include "I2C_module_simulate.h"
#include "soft.h"
#include "core_cmInstr.h"

/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/

/*----------------------------------------------*
 * external routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * internal routine prototypes                  *
 *----------------------------------------------*/

typedef struct I2C_PORT
{
	GPIO_TypeDef*	I2C_SCL_Port;
	uint16_t		I2C_SCL_Pin;
	GPIO_TypeDef*	I2C_SDA_Port;
	uint16_t		I2C_SDA_Pin;
	I2C_ADDR_TYPE_ENUM I2C_ADDR_Type;
}I2C_PORT_STRU;

/*----------------------------------------------*
 * project-wide global variables                *
 *----------------------------------------------*/

/*----------------------------------------------*
 * module-wide global variables                 *
 *----------------------------------------------*/

static I2C_PORT_STRU	s_I2C_Port_Use;			// �洢Ŀǰ����ʹ�õ�I2C�˿�

/*----------------------------------------------*
 * constants                                    *
 *----------------------------------------------*/

/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/
#define I2C_SCL_PORT			s_I2C_Port_Use.I2C_SCL_Port,s_I2C_Port_Use.I2C_SCL_Pin
#define I2C_SDA_PORT			s_I2C_Port_Use.I2C_SDA_Port,s_I2C_Port_Use.I2C_SDA_Pin


#define I2C_SCL_HIGH()			HAL_GPIO_WritePin(I2C_SCL_PORT,GPIO_PIN_SET)			// ����SCL�ߵ�ƽ
#define I2C_SCL_LOW()			HAL_GPIO_WritePin(I2C_SCL_PORT,GPIO_PIN_RESET)		// ����SCL�͵�ƽ

#define	I2C_SDA_HIGH()			HAL_GPIO_WritePin(I2C_SDA_PORT,GPIO_PIN_SET)			// ����SDA�ߵ�ƽ
#define I2C_SDA_LOW()			HAL_GPIO_WritePin(I2C_SDA_PORT,GPIO_PIN_RESET)		// ����SDA�͵�ƽ
#define I2C_SDA_READ()			HAL_GPIO_ReadPin(I2C_SDA_PORT)	// ��ȡSDA��ƽ

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

void I2C_Gpio_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;			//�ܽŽṹ
	__HAL_RCC_GPIOB_CLK_ENABLE();

	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP; 
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FAST;

	GPIO_InitStructure.Pin = I2C_1_SCL_PIN;
	HAL_GPIO_Init(I2C_1_SCL_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD; 
	GPIO_InitStructure.Pin = I2C_1_SDA_PIN;
	HAL_GPIO_Init(I2C_1_SDA_PORT, &GPIO_InitStructure);
}

/*****************************************************************************
 Prototype    : I2C_Port_Set
 Description  : I2Cģ��˿ں���ʹ��ǰ����
 Input        : i2cPort ����ΪI2C_DEVICE_ENUM��
 Output       : None
 Return Value : None
 Calls        : 
 Called By    : ʹ�ö��I2Cģ��ʱ���л�ʱ��Ҫʹ��
 
  History        :
  1.Date         : 2018/7/6
    Author       : STaoJ
    Modification : Created function

*****************************************************************************/
static void I2C_Port_Set(I2C_DEVICE_ENUM i2cPort)
{
	static I2C_DEVICE_ENUM port = Use_I2c_None;
	
	if(port == i2cPort)			// ����Ѿ���ͬ���Ͳ�������
	{
		return;
	}
	
	switch(i2cPort)
	{
	default:
	case Use_I2c_1:
		s_I2C_Port_Use.I2C_SCL_Port = I2C_1_SCL_PORT;
		s_I2C_Port_Use.I2C_SCL_Pin = I2C_1_SCL_PIN;
		s_I2C_Port_Use.I2C_SDA_Port = I2C_1_SDA_PORT;
		s_I2C_Port_Use.I2C_SDA_Pin = I2C_1_SDA_PIN;
		s_I2C_Port_Use.I2C_ADDR_Type = I2C_1_ADDR_TYPE;
		break;
		
#ifdef USE_MORE_THAN_ONE_I2C_PORT
	case Use_I2c_2:
		s_I2C_Port_Use.I2C_SCL_Port = I2C_2_SCL_PORT;
		s_I2C_Port_Use.I2C_SCL_Pin = I2C_2_SCL_PIN;
		s_I2C_Port_Use.I2C_SDA_Port = I2C_2_SDA_PORT;
		s_I2C_Port_Use.I2C_SDA_Pin = I2C_2_SDA_PIN;
		s_I2C_Port_Use.I2C_ADDR_Type = I2C_2_ADDR_TYPE;
		break;
		
	case Use_I2c_3:
		s_I2C_Port_Use.I2C_SCL_Port = I2C_3_SCL_PORT;
		s_I2C_Port_Use.I2C_SCL_Pin = I2C_3_SCL_PIN;
		s_I2C_Port_Use.I2C_SDA_Port = I2C_3_SDA_PORT;
		s_I2C_Port_Use.I2C_SDA_Pin = I2C_3_SDA_PIN;
		s_I2C_Port_Use.I2C_ADDR_Type = I2C_3_ADDR_TYPE;
		break;
		
	case Use_I2c_4:
		s_I2C_Port_Use.I2C_SCL_Port = I2C_4_SCL_PORT;
		s_I2C_Port_Use.I2C_SCL_Pin = I2C_4_SCL_PIN;
		s_I2C_Port_Use.I2C_SDA_Port = I2C_4_SDA_PORT;
		s_I2C_Port_Use.I2C_SDA_Pin = I2C_4_SDA_PIN;
		s_I2C_Port_Use.I2C_ADDR_Type = I2C_4_ADDR_TYPE;
		break;
#endif
	}
}

/*****************************************************************************
 Prototype    : I2C_SDA_Set
 Description  : �л�SDA�ţ�Out_OD��Inģʽ��
 Input        : GPIOMode_TypeDef GpioMode ȡֵ��GPIO_Mode_IN / GPIO_Mode_OUT
 				s_I2C_Port_Use�Ѿ������ã�������ʹ�ú���ǰ����Ҫ�Ѿ����ù�I2C_Port_Set();
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2018/9/5
    Author       : STaoJ
    Modification : Created function

*****************************************************************************/
static void I2C_SDA_Set(uint32_t GpioMode)
{	
	GPIO_InitTypeDef GPIO_InitStructure;			//�ܽŽṹ
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	GPIO_InitStructure.Mode = GpioMode; 
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
	
//	GPIO_InitStructure.GPIO_Mode = GpioMode;
//	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

	//////////////////////////PB�ڿ�©�����///////////////////
//	GPIO_InitStructure.GPIO_Pin = s_I2C_Port_Use.I2C_SDA_Pin;
	GPIO_InitStructure.Pin = s_I2C_Port_Use.I2C_SDA_Pin;
	HAL_GPIO_Init(s_I2C_Port_Use.I2C_SDA_Port, &GPIO_InitStructure);
}


/*****************************************************************************
 Prototype    : I2C_Delay
 Description  : I2C���ƹ����еĵȴ�
 Input        : uint32 nCount ��ʱ���� 
 Output       : None
 Return Value : none
 Calls        : none
 Called By    : I2C_module_xxx();
 
  History        :
  1.Date         : 2018/7/6
    Author       : STaoJ
    Modification : Created function

*****************************************************************************/
static void I2C_Delay(uint32_t nCount)
{
	for(; nCount != 0; nCount--)
	{
		__NOP();
		__NOP();
		__NOP();
		__NOP();
	}
}

/*****************************************************************************
 Prototype    : I2C_Start
 Description  : Simulate I2C start
 Input        : void  
 Output       : None
 Return Value : None
 
  History        :
  1.Date         : 2018/7/6
    Author       : STaoJ
    Modification : Created function

*****************************************************************************/
static uint8_t I2C_Start(void)
{
	I2C_SDA_HIGH();
	if(!I2C_SDA_READ())
	{
		return I2C_ERR;
	}
    I2C_Delay(10);
    I2C_SCL_HIGH();
    I2C_Delay(10);//delay for starting ,always >4.7us
    I2C_SDA_LOW();
    I2C_Delay(10);//delay for starting ,always >4.7us
    I2C_SCL_LOW();
	
	return I2C_OK;
}

/*****************************************************************************
 Prototype    : I2C_Stop
 Description  : simulate I2C stop
 Input        : void  
 Output       : None
 Return Value : None
 
  History        :
  1.Date         : 2018/7/6
    Author       : STaoJ
    Modification : Created function

*****************************************************************************/
static void I2C_Stop(void)
{
	I2C_SDA_LOW();
	I2C_Delay(10);//delay for stop ,always >4.7us
	I2C_SCL_HIGH();
	I2C_Delay(10);
	I2C_SDA_HIGH();
}

/*****************************************************************************
 Prototype    : I2C_Master_Ack
 Description  : iic slave transmit data and master response with ACK
 Input        : void  
 Output       : None
 Return Value : None
 
  History        :
  1.Date         : 2018/7/6
    Author       : STaoJ
    Modification : Created function

*****************************************************************************/
static void I2C_Master_Ack(void)
{
	I2C_SDA_LOW();
	I2C_Delay(10);
	I2C_SCL_HIGH();
	I2C_Delay(10);		// 20181126 STaoJ ���˴���8��Ϊ6
	I2C_SCL_LOW();
	I2C_Delay(10);
	I2C_SDA_HIGH();
}

/*****************************************************************************
 Prototype    : I2C_Slave_Ack
 Description  : master transmit data and slave response ack
 Input        : void  
 Output       : None
 Return Value : uint8_t I2C_OK:ִ�гɹ��� I2C_ERR:ִ��ʧ�ܣ� 
 
  History        :
  1.Date         : 2018/7/6
    Author       : STaoJ
    Modification : Created function

*****************************************************************************/
static uint8_t I2C_Slave_Ack(void)
{
	uint8_t byErrTime=50;

	I2C_SDA_HIGH();
	I2C_SDA_Set(GPIO_MODE_INPUT);
	I2C_Delay(10);
	I2C_SCL_HIGH();
	I2C_Delay(10);		// 20181126 STaoJ ���˴���8��Ϊ6
	while(I2C_SDA_READ())
	{
		byErrTime--;
		if(!byErrTime)
		{
			I2C_SDA_Set(GPIO_MODE_OUTPUT_OD);
			I2C_Stop();
			return I2C_ERR;
		}
	}
	I2C_SCL_LOW();
	I2C_SDA_Set(GPIO_MODE_OUTPUT_OD);		// ��SCL_low ��תΪ���ģʽ
	return I2C_OK;
}

/*****************************************************************************
 Prototype    : I2C_No_Ack
 Description  : slave transmit data and master response I2C_No_Ack
 Input        : void  
 Output       : None
 Return Value : None
 
  History        :
  1.Date         : 2018/7/6
    Author       : STaoJ
    Modification : Created function

*****************************************************************************/
static void I2C_No_Ack(void)
{
	I2C_SDA_HIGH();
	I2C_Delay(10);
	I2C_SCL_HIGH();
	I2C_Delay(10);
	I2C_SCL_LOW();
	I2C_Delay(10);
}

/*****************************************************************************
 Prototype    : I2C_Write_Byte
 Description  : i2c write data to slave
 Input        : uint8_t byData  
 Output       : None
 Return Value : byData: Data write to 
 
  History        :
  1.Date         : 2018/7/6
    Author       : STaoJ
    Modification : Created function

*****************************************************************************/
static void I2C_Write_Byte(uint8_t byData)
{
	uint8_t i;
	for (i=0; i<8; i++)
	{
		I2C_SCL_LOW();
		I2C_Delay(10);		// 20181126 STaoJ ���˴���8��Ϊ1
		if(byData&0x80)
		{
			I2C_SDA_HIGH();
		}
		else
		{
			I2C_SDA_LOW();
		}
		byData <<= 1;
		I2C_Delay(10);		// 20181126 STaoJ ���˴���8��Ϊ6
		I2C_SCL_HIGH();
		I2C_Delay(10);		// 20181126 STaoJ ���˴���8��Ϊ1
	}
	I2C_SCL_LOW();
	I2C_SDA_HIGH();
}

/*****************************************************************************
 Prototype    : I2C_Read_Byte
 Description  : i2c read data from slave
 Input        : void  
 Output       : None
 Return Value : uint8_t I2C_OK:ִ�гɹ��� I2C_ERR:ִ��ʧ�ܣ�
 
  History        :
  1.Date         : 2018/7/6
    Author       : STaoJ
    Modification : Created function

*****************************************************************************/
static uint8_t I2C_Read_Byte(void)
{
	uint8_t i,byData;
	
	I2C_Delay(10);
	byData=0;
	for (i=0; i<8; i++)
	{
		I2C_SCL_LOW();
		I2C_Delay(10);
		I2C_SCL_HIGH();
		I2C_Delay(10);
		byData <<= 1;
		if(I2C_SDA_READ())
		{
			byData |= 0x01;
		}
	}
	I2C_Delay(4);
	I2C_SCL_LOW();
	I2C_Delay(4);
	
	return (byData);
}

/*****************************************************************************
 Prototype    : I2C_Module_Write
 Description  : write byte to the specific register
 Input        : I2C_DEVICE_ENUM I2C_Port �˿ڶ���Ķ˿ڣ�1-4
 		 		uint8_t bySlave  �����ӵ�ַ��Ϊ����д��ַ����ʱ���Զ�����0x01
			    uint8_t byAddr   оƬ�ڲ�д�����ʼ��ַ
			    uint8_t *pBuf	  д�����ݵĴ��λ��
			    uint8_t byNum	  д�����ݵ�����
 Output       : None
 Return Value : uint8_t I2C_OK:ִ�гɹ��� I2C_ERR:ִ��ʧ�ܣ� 
 Calls        : 
 Called By    : ��ҪдI2C�����ĺ���
 
  History        :
  1.Date         : 2018/7/6
    Author       : STaoJ
    Modification : Created function

*****************************************************************************/
uint8_t I2C_Module_Write(I2C_DEVICE_ENUM I2C_Port, uint8_t bySlave, 
									uint16_t wAddr, uint8_t *pBuf, uint8_t byNum)
{
	uint8_t i, ret;

	// ѡ��I2C�˿�
	I2C_Port_Set(I2C_Port);
	
	I2C_SDA_Set(GPIO_MODE_OUTPUT_OD);
	// ��д���������ַ
	ret = I2C_Start();
	if(ret != I2C_OK)
	{
		I2C_Stop();
		return ret;
	}
	
	I2C_Write_Byte(bySlave);
	ret = I2C_Slave_Ack();
	if(ret != I2C_OK)
	{
		I2C_Stop();
		return ret;
	}
	if(s_I2C_Port_Use.I2C_ADDR_Type == I2C_ADDR_Word)
	{
		I2C_Write_Byte((uint8_t)(wAddr>>8));
		ret = I2C_Slave_Ack();
		if(ret != I2C_OK)
		{
			I2C_Stop();
			return ret;
		}
	}
	I2C_Write_Byte((uint8_t)(wAddr&0x00ff));
	ret = I2C_Slave_Ack();
	if(ret != I2C_OK)
	{
		I2C_Stop();
		return ret;
	}
	
	// ��ʼд����
	for(i=0; i<byNum; i++)
	{
		I2C_Write_Byte(pBuf[i]);
		// ��д���������ַ
		ret = I2C_Slave_Ack();
		if(ret != I2C_OK)
		{
			I2C_Stop();
			return ret;
		}
	}
	I2C_Stop();
	
	return I2C_OK;
}

/*****************************************************************************
 Prototype	  : I2C_Module_Read
 Description  : read byte to the specific register
 Input        : I2C_DEVICE_ENUM I2C_Port �˿ڶ���Ķ˿ڣ�1-4
 		 		uint8_t bySlave  �����ӵ�ַ��Ϊ����д��ַ����ʱ���Զ�����0x01
				uint8_t byAddr   оƬ�ڲ���ȡ����ʼ��ַ
				uint8_t *pBuf    ��ȡ���ݵĴ��λ��
				uint8_t byNum    ��ȡ���ݵ�����
 Output 	  : None
 Return Value : uint8_t I2C_OK:ִ�гɹ��� I2C_ERR:ִ��ʧ�ܣ� 
 Calls		  : 
 Called By	  : ��Ҫ��I2C�������ݵĺ���
 
  History		 :
  1.Date		 : 2018/7/6
	Author		 : STaoJ
	Modification : Created function

*****************************************************************************/
uint8_t I2C_Module_Read(I2C_DEVICE_ENUM I2C_Port, uint8_t bySlave, 
								uint16_t wAddr, uint8_t *pData, uint8_t byNum)
{
	uint8_t i, ret;

	// ѡ��I2C�˿�
	I2C_Port_Set(I2C_Port);
	
	I2C_SDA_Set(GPIO_MODE_OUTPUT_OD);
	
	// ��д���������ַ
	ret = I2C_Start();
	if(ret != I2C_OK)
	{
		I2C_Stop();
		return ret;
	}
	
	I2C_Write_Byte(bySlave);
	ret = I2C_Slave_Ack();
	if(ret != I2C_OK)
	{
		I2C_Stop();
		return ret;
	}
	if(s_I2C_Port_Use.I2C_ADDR_Type == I2C_ADDR_Word)
	{
		I2C_Write_Byte((uint8_t)(wAddr>>8));
		ret = I2C_Slave_Ack();
		if(ret != I2C_OK)
		{
			I2C_Stop();
			return ret;
		}
	}
	I2C_Write_Byte((uint8_t)(wAddr&0x00ff));
	ret = I2C_Slave_Ack();
	//I2C_Stop();			// STaoJ �Ȳ���ֹͣ//20181120 STaoJ ȡ��ֹͣ����

	// ��ʼ������
	ret = I2C_Start();	// STaoJ ���ﲻ�ж�
	//if(ret != I2C_OK)	
	//{
	//	I2C_Stop();
	//	return ret;
	//}
	I2C_Write_Byte(bySlave|0x01);
	I2C_Slave_Ack();
	for(i=0; i<byNum; i++)
	{
		// �л��������
		I2C_SDA_Set(GPIO_Mode_IN);
		pData[i] = I2C_Read_Byte();
		
		// �л��������
		I2C_SDA_Set(GPIO_Mode_OUT);
		if(i < (byNum - 1))
		{
			I2C_Master_Ack();
		}
		else
		{
			I2C_No_Ack();
		}
	}
	I2C_Stop();
	
	return I2C_OK;
}


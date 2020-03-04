
/******************************************************************************

  Copyright (C), 2008-2018, YXROBOT Co., Ltd.

 ******************************************************************************
  File Name     : I2C_module_simulate.c
  Version       : Initial Draft
  Author        : STaoJ
  Created       : 2018/7/6
  Last Modified :
  Description   : I2C模块模拟程序，控制外围I2C器件
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
    Modification: 取消读命令的写完从地址与寄存器地址后的停止，仅产生一个起始条件。

  3.Date        : 2018/12/13
    Author      : suntaojun
    Modification: 优化I2C_Delay(),并将各处等待时间加到10秒，解决有些IST8305无法
    	读取数据的问题，但是会导致其它不需要延时这么长时间的器件耽误时间。

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

static I2C_PORT_STRU	s_I2C_Port_Use;			// 存储目前正在使用的I2C端口

/*----------------------------------------------*
 * constants                                    *
 *----------------------------------------------*/

/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/
#define I2C_SCL_PORT			s_I2C_Port_Use.I2C_SCL_Port,s_I2C_Port_Use.I2C_SCL_Pin
#define I2C_SDA_PORT			s_I2C_Port_Use.I2C_SDA_Port,s_I2C_Port_Use.I2C_SDA_Pin


#define I2C_SCL_HIGH()			HAL_GPIO_WritePin(I2C_SCL_PORT,GPIO_PIN_SET)			// 配置SCL高电平
#define I2C_SCL_LOW()			HAL_GPIO_WritePin(I2C_SCL_PORT,GPIO_PIN_RESET)		// 配置SCL低电平

#define	I2C_SDA_HIGH()			HAL_GPIO_WritePin(I2C_SDA_PORT,GPIO_PIN_SET)			// 配置SDA高电平
#define I2C_SDA_LOW()			HAL_GPIO_WritePin(I2C_SDA_PORT,GPIO_PIN_RESET)		// 配置SDA低电平
#define I2C_SDA_READ()			HAL_GPIO_ReadPin(I2C_SDA_PORT)	// 读取SDA电平

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

void I2C_Gpio_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;			//管脚结构
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
 Description  : I2C模拟端口函数使用前配置
 Input        : i2cPort 类型为I2C_DEVICE_ENUM，
 Output       : None
 Return Value : None
 Calls        : 
 Called By    : 使用多个I2C模块时，切换时需要使用
 
  History        :
  1.Date         : 2018/7/6
    Author       : STaoJ
    Modification : Created function

*****************************************************************************/
static void I2C_Port_Set(I2C_DEVICE_ENUM i2cPort)
{
	static I2C_DEVICE_ENUM port = Use_I2c_None;
	
	if(port == i2cPort)			// 如果已经相同，就不配置了
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
 Description  : 切换SDA脚，Out_OD与In模式，
 Input        : GPIOMode_TypeDef GpioMode 取值：GPIO_Mode_IN / GPIO_Mode_OUT
 				s_I2C_Port_Use已经被引用，所以在使用函数前，需要已经调用过I2C_Port_Set();
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
	GPIO_InitTypeDef GPIO_InitStructure;			//管脚结构
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	GPIO_InitStructure.Mode = GpioMode; 
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
	
//	GPIO_InitStructure.GPIO_Mode = GpioMode;
//	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

	//////////////////////////PB口开漏输出：///////////////////
//	GPIO_InitStructure.GPIO_Pin = s_I2C_Port_Use.I2C_SDA_Pin;
	GPIO_InitStructure.Pin = s_I2C_Port_Use.I2C_SDA_Pin;
	HAL_GPIO_Init(s_I2C_Port_Use.I2C_SDA_Port, &GPIO_InitStructure);
}


/*****************************************************************************
 Prototype    : I2C_Delay
 Description  : I2C控制过程中的等待
 Input        : uint32 nCount 延时次数 
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
	I2C_Delay(10);		// 20181126 STaoJ 将此处从8改为6
	I2C_SCL_LOW();
	I2C_Delay(10);
	I2C_SDA_HIGH();
}

/*****************************************************************************
 Prototype    : I2C_Slave_Ack
 Description  : master transmit data and slave response ack
 Input        : void  
 Output       : None
 Return Value : uint8_t I2C_OK:执行成功， I2C_ERR:执行失败， 
 
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
	I2C_Delay(10);		// 20181126 STaoJ 将此处从8改为6
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
	I2C_SDA_Set(GPIO_MODE_OUTPUT_OD);		// 先SCL_low 再转为输出模式
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
		I2C_Delay(10);		// 20181126 STaoJ 将此处从8改为1
		if(byData&0x80)
		{
			I2C_SDA_HIGH();
		}
		else
		{
			I2C_SDA_LOW();
		}
		byData <<= 1;
		I2C_Delay(10);		// 20181126 STaoJ 将此处从8改为6
		I2C_SCL_HIGH();
		I2C_Delay(10);		// 20181126 STaoJ 将此处从8改为1
	}
	I2C_SCL_LOW();
	I2C_SDA_HIGH();
}

/*****************************************************************************
 Prototype    : I2C_Read_Byte
 Description  : i2c read data from slave
 Input        : void  
 Output       : None
 Return Value : uint8_t I2C_OK:执行成功， I2C_ERR:执行失败，
 
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
 Input        : I2C_DEVICE_ENUM I2C_Port 端口定义的端口，1-4
 		 		uint8_t bySlave  器件从地址，为器件写地址，读时会自动或上0x01
			    uint8_t byAddr   芯片内部写入的起始地址
			    uint8_t *pBuf	  写入数据的存放位置
			    uint8_t byNum	  写入数据的数量
 Output       : None
 Return Value : uint8_t I2C_OK:执行成功， I2C_ERR:执行失败， 
 Calls        : 
 Called By    : 需要写I2C器件的函数
 
  History        :
  1.Date         : 2018/7/6
    Author       : STaoJ
    Modification : Created function

*****************************************************************************/
uint8_t I2C_Module_Write(I2C_DEVICE_ENUM I2C_Port, uint8_t bySlave, 
									uint16_t wAddr, uint8_t *pBuf, uint8_t byNum)
{
	uint8_t i, ret;

	// 选择I2C端口
	I2C_Port_Set(I2C_Port);
	
	I2C_SDA_Set(GPIO_MODE_OUTPUT_OD);
	// 先写入器件与地址
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
	
	// 开始写数据
	for(i=0; i<byNum; i++)
	{
		I2C_Write_Byte(pBuf[i]);
		// 先写入器件与地址
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
 Input        : I2C_DEVICE_ENUM I2C_Port 端口定义的端口，1-4
 		 		uint8_t bySlave  器件从地址，为器件写地址，读时会自动或上0x01
				uint8_t byAddr   芯片内部读取的起始地址
				uint8_t *pBuf    读取数据的存放位置
				uint8_t byNum    读取数据的数量
 Output 	  : None
 Return Value : uint8_t I2C_OK:执行成功， I2C_ERR:执行失败， 
 Calls		  : 
 Called By	  : 需要读I2C器件数据的函数
 
  History		 :
  1.Date		 : 2018/7/6
	Author		 : STaoJ
	Modification : Created function

*****************************************************************************/
uint8_t I2C_Module_Read(I2C_DEVICE_ENUM I2C_Port, uint8_t bySlave, 
								uint16_t wAddr, uint8_t *pData, uint8_t byNum)
{
	uint8_t i, ret;

	// 选择I2C端口
	I2C_Port_Set(I2C_Port);
	
	I2C_SDA_Set(GPIO_MODE_OUTPUT_OD);
	
	// 先写入器件与地址
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
	//I2C_Stop();			// STaoJ 先产生停止//20181120 STaoJ 取消停止动作

	// 开始读数据
	ret = I2C_Start();	// STaoJ 这里不判断
	//if(ret != I2C_OK)	
	//{
	//	I2C_Stop();
	//	return ret;
	//}
	I2C_Write_Byte(bySlave|0x01);
	I2C_Slave_Ack();
	for(i=0; i<byNum; i++)
	{
		// 切换成输入口
		I2C_SDA_Set(GPIO_Mode_IN);
		pData[i] = I2C_Read_Byte();
		
		// 切换成输出口
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


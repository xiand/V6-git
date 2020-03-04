/*
*********************************************************************************************************
*
*	模块名称 : 三轴陀螺仪MPU-6050驱动模块
*	文件名称 : bsp_mpu6050.c
*	版    本 : V1.0
*	说    明 : 实现MPU-6050的读写操作。
*
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2013-02-01 armfly  正式发布
*
*	Copyright (C), 2013-2014, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

/*
	应用说明：访问MPU-6050前，请先调用一次 bsp_InitI2C()函数配置好I2C相关的GPIO.
*/

#include "bsp.h"

MPU6050_T g_tMPU6050;		/* 定义一个全局变量，保存实时数据 */

/*
*********************************************************************************************************
*	函 数 名: bsp_InitMPU6050
*	功能说明: 初始化MPU-6050
*	形    参:  无
*	返 回 值: 1 表示正常， 0 表示不正常
*********************************************************************************************************
*/
void bsp_InitMPU6050(void)
{
	uint8_t i = 0;
	g_tMPU6050.who_am_i = MPU6050_ReadByte(WHO_AM_I);
	bsp_DelayMS(10);
	MPU6050_WriteByte(PWR_MGMT_1, 0x80);	//解除休眠状态
	bsp_DelayMS(10);
	MPU6050_WriteByte(PWR_MGMT_1, 0x00);	//解除休眠状态
	MPU6050_WriteByte(SMPLRT_DIV, 0x07);
	MPU6050_WriteByte(CONFIG, 0x06);
	MPU6050_WriteByte(GYRO_CONFIG, 0x18);
	MPU6050_WriteByte(ACCEL_CONFIG, 0x01);
	g_tMPU6050.bias_count = 0;
}

/*
*********************************************************************************************************
*	函 数 名: MPU6050_WriteByte
*	功能说明: 向 MPU-6050 寄存器写入一个数据
*	形    参: _ucRegAddr : 寄存器地址
*			  _ucRegData : 寄存器数据
*	返 回 值: 无
*********************************************************************************************************
*/
void MPU6050_WriteByte(uint8_t _ucRegAddr, uint8_t _ucRegData)
{
    i2c_Start();							/* 总线开始信号 */

    i2c_SendByte(MPU6050_SLAVE_ADDRESS);	/* 发送设备地址+写信号 */
	i2c_WaitAck();

    i2c_SendByte(_ucRegAddr);				/* 内部寄存器地址 */
	i2c_WaitAck();

    i2c_SendByte(_ucRegData);				/* 内部寄存器数据 */
	i2c_WaitAck();

    i2c_Stop();                   			/* 总线停止信号 */
}

/*
*********************************************************************************************************
*	函 数 名: MPU6050_ReadByte
*	功能说明: 读取 MPU-6050 寄存器的数据
*	形    参: _ucRegAddr : 寄存器地址
*	返 回 值: 无
*********************************************************************************************************
*/
uint8_t MPU6050_ReadByte(uint8_t _ucRegAddr)
{
	uint8_t ucData;

	i2c_Start();                  			/* 总线开始信号 */
	i2c_SendByte(MPU6050_SLAVE_ADDRESS);	/* 发送设备地址+写信号 */
	i2c_WaitAck();
	i2c_SendByte(_ucRegAddr);     			/* 发送存储单元地址 */
	i2c_WaitAck();

	i2c_Start();                  			/* 总线开始信号 */

	i2c_SendByte(MPU6050_SLAVE_ADDRESS+1); 	/* 发送设备地址+读信号 */
	i2c_WaitAck();

	ucData = i2c_ReadByte();       			/* 读出寄存器数据 */
	i2c_NAck();
	i2c_Stop();                  			/* 总线停止信号 */
	return ucData;
}


/*
*********************************************************************************************************
*	函 数 名: MPU6050_ReadData
*	功能说明: 读取 MPU-6050 数据寄存器， 结果保存在全局变量 g_tMPU6050.  主程序可以定时调用该程序刷新数据
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void MPU6050_ReadData(void)
{
	uint8_t ucReadBuf[14];
	uint8_t i;
	uint8_t ack;

#if 1 /* 连续读 */
	i2c_Start();                  			/* 总线开始信号 */
	i2c_SendByte(MPU6050_SLAVE_ADDRESS);	/* 发送设备地址+写信号 */
	ack = i2c_WaitAck();
	if (ack != 0)
	{
		i2c_Stop(); 
		return;
	}
	i2c_SendByte(ACCEL_XOUT_H);     		/* 发送存储单元地址  */
	ack = i2c_WaitAck();
	if (ack != 0)
	{
		i2c_Stop(); 
		return;
	}

	i2c_Start();                  			/* 总线开始信号 */

	i2c_SendByte(MPU6050_SLAVE_ADDRESS + 1); /* 发送设备地址+读信号 */
	ack = i2c_WaitAck();
	if (ack != 0)
	{
		i2c_Stop(); 
		return;
	}

	for (i = 0; i < 13; i++)
	{
		ucReadBuf[i] = i2c_ReadByte();       			/* 读出寄存器数据 */
		i2c_Ack();
	}

	/* 读最后一个字节，时给 NAck */
	ucReadBuf[13] = i2c_ReadByte();
	i2c_NAck();

	i2c_Stop();                  			/* 总线停止信号 */

#else	/* 单字节读 */
	for (i = 0 ; i < 14; i++)
	{
		ucReadBuf[i] = MPU6050_ReadByte(ACCEL_XOUT_H + i);
	}
#endif

	/* 将读出的数据保存到全局结构体变量 */
	g_tMPU6050.Accel_X = (ucReadBuf[0] << 8) + ucReadBuf[1];
	g_tMPU6050.Accel_Y = (ucReadBuf[2] << 8) + ucReadBuf[3];
	g_tMPU6050.Accel_Z = (ucReadBuf[4] << 8) + ucReadBuf[5];

	g_tMPU6050.Temp = (int16_t)((ucReadBuf[6] << 8) + ucReadBuf[7]);

	g_tMPU6050.GYRO_X = (ucReadBuf[8] << 8) + ucReadBuf[9];
	g_tMPU6050.GYRO_Y = (ucReadBuf[10] << 8) + ucReadBuf[11];
	g_tMPU6050.GYRO_Z = (ucReadBuf[12] << 8) + ucReadBuf[13];
}


uint8_t MPU6050_DataUpdate(uint8_t parm)
{
	uint8_t i = 0;
	i = MPU6050_ReadByte(INTERRUPT_STATUS);
	if((i & 0x01) != 0)
	{
		MPU6050_ReadData();
		if(parm == 0)
		{
			g_tMPU6050.Accel_X = g_tMPU6050.Accel_X - g_tMPU6050.accl_x_bias;
			g_tMPU6050.Accel_Y = g_tMPU6050.Accel_Y - g_tMPU6050.accl_y_bias;
			g_tMPU6050.Accel_Z = g_tMPU6050.Accel_Z - g_tMPU6050.accl_z_bias;

			g_tMPU6050.GYRO_X = g_tMPU6050.GYRO_X - g_tMPU6050.gyro_x_bias;
			g_tMPU6050.GYRO_Y = g_tMPU6050.GYRO_Y - g_tMPU6050.gyro_y_bias;
			g_tMPU6050.GYRO_Z = g_tMPU6050.GYRO_Z - g_tMPU6050.gyro_z_bias;
		}				
		return 1;
	}
	else
	{
		return 0;
	}
}

void MPU6050_BiasUpDate(void)
{
	uint16_t i = 0;
	uint8_t ret = 0;
	int16_t gyro_x = 0;
	int16_t gyro_y = 0;
	int16_t gyro_z = 0;
	int16_t accl_x = 0;
	int16_t accl_y = 0;
	int16_t accl_z = 0;
	
	while(1)
	{
		if(MPU6050_DataUpdate(1))
		{
			i++;
			if(i > 50)
			{
				gyro_x += g_tMPU6050.GYRO_X;
				gyro_y += g_tMPU6050.GYRO_Y;
				gyro_z += g_tMPU6050.GYRO_Z;
				accl_x += g_tMPU6050.Accel_X;
				accl_y += g_tMPU6050.Accel_Y;
				accl_z += g_tMPU6050.Accel_Z;
			}
			if(i == 300)
			{
				g_tMPU6050.gyro_x_bias = gyro_x/250;
				g_tMPU6050.gyro_y_bias = gyro_y/250;
				g_tMPU6050.gyro_z_bias = gyro_z/250;
				g_tMPU6050.accl_x_bias = accl_x/250;
				g_tMPU6050.accl_y_bias = accl_y/250;
				g_tMPU6050.accl_z_bias = accl_z/250;
				g_tMPU6050.bias_count = 1;
				break;
			}
		}
	}
	return ;
}

extern void xrobot_eulerAngle_estimation();
extern  uint32_t giv_sys_time;
void MPU6050_get_oula_angle(void)
{
//	if((giv_sys_time > 50000)&&(g_tMPU6050.bias_count == 0))
//	{
//		MPU6050_BiasUpDate();
//		g_tMPU6050.bias_count = 1;
//		return;
//	}

	if(MPU6050_DataUpdate(0))
	{
		xrobot_eulerAngle_estimation();
	}
}


/*
*********************************************************************************************************
*	函 数 名: DemoEEPROM
*	功能说明: 串行EEPROM读写例程
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void DemoMPU6050(void)
{
	if (i2c_CheckDevice(MPU6050_SLAVE_ADDRESS) == 0)
	{
		printf("MPU-6050 Ok (0x%02X)\r\n", MPU6050_SLAVE_ADDRESS);
	}
	else
	{
		printf("MPU-6050 Err (0x%02X)\r\n", MPU6050_SLAVE_ADDRESS);
	}

	//DispMenu();		/* 打印命令提示 */

	bsp_InitMPU6050();

	bsp_StartAutoTimer(0, 10);	/* 启动1个自动定时器，周期300ms */
	while(1)
	{
		bsp_Idle();		/* 这个函数在bsp.c文件。用户可以修改这个函数实现CPU休眠和喂狗 */

		if (bsp_CheckTimer(0))		/* 检查定时周期到否 */
		{
			MPU6050_ReadData();		/* 读取 MPU-6050的数据到全局变量 g_tMPU6050 */

			/* 显示 g_tMPU6050 结构体成员中的数据 */
			{
				printf("AX=%6d,AY=%6d,AZ=%6d,",
					g_tMPU6050.Accel_X,
					g_tMPU6050.Accel_Y,
					g_tMPU6050.Accel_Z);

				printf("GX=%6d,GY=%6d,GZ=%6d,T=%f\r\n",					
					g_tMPU6050.GYRO_X,
					g_tMPU6050.GYRO_Y,
					g_tMPU6050.GYRO_Z,
					((float)g_tMPU6050.Temp/ 340.0) + 36.53);			
			}
		}
	}
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/

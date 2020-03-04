/*
*********************************************************************************************************
*
*	ģ������ : ����������MPU-6050����ģ��
*	�ļ����� : bsp_mpu6050.c
*	��    �� : V1.0
*	˵    �� : ʵ��MPU-6050�Ķ�д������
*
*	�޸ļ�¼ :
*		�汾��  ����        ����     ˵��
*		V1.0    2013-02-01 armfly  ��ʽ����
*
*	Copyright (C), 2013-2014, ���������� www.armfly.com
*
*********************************************************************************************************
*/

/*
	Ӧ��˵��������MPU-6050ǰ�����ȵ���һ�� bsp_InitI2C()�������ú�I2C��ص�GPIO.
*/

#include "bsp.h"

MPU6050_T g_tMPU6050;		/* ����һ��ȫ�ֱ���������ʵʱ���� */

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitMPU6050
*	����˵��: ��ʼ��MPU-6050
*	��    ��:  ��
*	�� �� ֵ: 1 ��ʾ������ 0 ��ʾ������
*********************************************************************************************************
*/
void bsp_InitMPU6050(void)
{
	uint8_t i = 0;
	g_tMPU6050.who_am_i = MPU6050_ReadByte(WHO_AM_I);
	bsp_DelayMS(10);
	MPU6050_WriteByte(PWR_MGMT_1, 0x80);	//�������״̬
	bsp_DelayMS(10);
	MPU6050_WriteByte(PWR_MGMT_1, 0x00);	//�������״̬
	MPU6050_WriteByte(SMPLRT_DIV, 0x07);
	MPU6050_WriteByte(CONFIG, 0x06);
	MPU6050_WriteByte(GYRO_CONFIG, 0x18);
	MPU6050_WriteByte(ACCEL_CONFIG, 0x01);
	g_tMPU6050.bias_count = 0;
}

/*
*********************************************************************************************************
*	�� �� ��: MPU6050_WriteByte
*	����˵��: �� MPU-6050 �Ĵ���д��һ������
*	��    ��: _ucRegAddr : �Ĵ�����ַ
*			  _ucRegData : �Ĵ�������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void MPU6050_WriteByte(uint8_t _ucRegAddr, uint8_t _ucRegData)
{
    i2c_Start();							/* ���߿�ʼ�ź� */

    i2c_SendByte(MPU6050_SLAVE_ADDRESS);	/* �����豸��ַ+д�ź� */
	i2c_WaitAck();

    i2c_SendByte(_ucRegAddr);				/* �ڲ��Ĵ�����ַ */
	i2c_WaitAck();

    i2c_SendByte(_ucRegData);				/* �ڲ��Ĵ������� */
	i2c_WaitAck();

    i2c_Stop();                   			/* ����ֹͣ�ź� */
}

/*
*********************************************************************************************************
*	�� �� ��: MPU6050_ReadByte
*	����˵��: ��ȡ MPU-6050 �Ĵ���������
*	��    ��: _ucRegAddr : �Ĵ�����ַ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
uint8_t MPU6050_ReadByte(uint8_t _ucRegAddr)
{
	uint8_t ucData;

	i2c_Start();                  			/* ���߿�ʼ�ź� */
	i2c_SendByte(MPU6050_SLAVE_ADDRESS);	/* �����豸��ַ+д�ź� */
	i2c_WaitAck();
	i2c_SendByte(_ucRegAddr);     			/* ���ʹ洢��Ԫ��ַ */
	i2c_WaitAck();

	i2c_Start();                  			/* ���߿�ʼ�ź� */

	i2c_SendByte(MPU6050_SLAVE_ADDRESS+1); 	/* �����豸��ַ+���ź� */
	i2c_WaitAck();

	ucData = i2c_ReadByte();       			/* �����Ĵ������� */
	i2c_NAck();
	i2c_Stop();                  			/* ����ֹͣ�ź� */
	return ucData;
}


/*
*********************************************************************************************************
*	�� �� ��: MPU6050_ReadData
*	����˵��: ��ȡ MPU-6050 ���ݼĴ����� ���������ȫ�ֱ��� g_tMPU6050.  ��������Զ�ʱ���øó���ˢ������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void MPU6050_ReadData(void)
{
	uint8_t ucReadBuf[14];
	uint8_t i;
	uint8_t ack;

#if 1 /* ������ */
	i2c_Start();                  			/* ���߿�ʼ�ź� */
	i2c_SendByte(MPU6050_SLAVE_ADDRESS);	/* �����豸��ַ+д�ź� */
	ack = i2c_WaitAck();
	if (ack != 0)
	{
		i2c_Stop(); 
		return;
	}
	i2c_SendByte(ACCEL_XOUT_H);     		/* ���ʹ洢��Ԫ��ַ  */
	ack = i2c_WaitAck();
	if (ack != 0)
	{
		i2c_Stop(); 
		return;
	}

	i2c_Start();                  			/* ���߿�ʼ�ź� */

	i2c_SendByte(MPU6050_SLAVE_ADDRESS + 1); /* �����豸��ַ+���ź� */
	ack = i2c_WaitAck();
	if (ack != 0)
	{
		i2c_Stop(); 
		return;
	}

	for (i = 0; i < 13; i++)
	{
		ucReadBuf[i] = i2c_ReadByte();       			/* �����Ĵ������� */
		i2c_Ack();
	}

	/* �����һ���ֽڣ�ʱ�� NAck */
	ucReadBuf[13] = i2c_ReadByte();
	i2c_NAck();

	i2c_Stop();                  			/* ����ֹͣ�ź� */

#else	/* ���ֽڶ� */
	for (i = 0 ; i < 14; i++)
	{
		ucReadBuf[i] = MPU6050_ReadByte(ACCEL_XOUT_H + i);
	}
#endif

	/* �����������ݱ��浽ȫ�ֽṹ����� */
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
*	�� �� ��: DemoEEPROM
*	����˵��: ����EEPROM��д����
*	��    �Σ���
*	�� �� ֵ: ��
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

	//DispMenu();		/* ��ӡ������ʾ */

	bsp_InitMPU6050();

	bsp_StartAutoTimer(0, 10);	/* ����1���Զ���ʱ��������300ms */
	while(1)
	{
		bsp_Idle();		/* ���������bsp.c�ļ����û������޸��������ʵ��CPU���ߺ�ι�� */

		if (bsp_CheckTimer(0))		/* ��鶨ʱ���ڵ��� */
		{
			MPU6050_ReadData();		/* ��ȡ MPU-6050�����ݵ�ȫ�ֱ��� g_tMPU6050 */

			/* ��ʾ g_tMPU6050 �ṹ���Ա�е����� */
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

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/

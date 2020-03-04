#ifndef _MY_MPU6050_h_
#define _MY_MPU6050_h_

//****************************************
// ����MPU6050�ڲ���ַ
//****************************************
#define	SMPLRT_DIV		0x19	//�����ǲ����ʣ�����ֵ��0x07(125Hz)
#define	CONFIG			0x1A	//��ͨ�˲�Ƶ�ʣ�����ֵ��0x06(5Hz)
#define	GYRO_CONFIG		0x1B	//�������Լ켰������Χ������ֵ��0x18(���Լ죬2000deg/s)

#define	ACCEL_CONFIG	0x1C	//���ټ��Լ졢������Χ����ͨ�˲�Ƶ�ʣ�����ֵ��0x01(���Լ죬2G��5Hz)

#define INTERRUPT_STATUS 0x3A  //�ж�״̬�Ĵ���

#define	ACCEL_XOUT_H	0x3B
#define	ACCEL_XOUT_L	0x3C
#define	ACCEL_YOUT_H	0x3D
#define	ACCEL_YOUT_L	0x3E
#define	ACCEL_ZOUT_H	0x3F
#define	ACCEL_ZOUT_L	0x40

#define	TEMP_OUT_H		0x41
#define	TEMP_OUT_L		0x42

#define	GYRO_XOUT_H		0x43
#define	GYRO_XOUT_L		0x44
#define	GYRO_YOUT_H		0x45
#define	GYRO_YOUT_L		0x46
#define	GYRO_ZOUT_H		0x47
#define	GYRO_ZOUT_L		0x48

#define	PWR_MGMT_1		0x6B	//��Դ��������ֵ��0x00(��������)
#define	WHO_AM_I		0x75	//IIC��ַ�Ĵ���(Ĭ����ֵ0x68��ֻ��)


#define MPU6050_SLAVE_ADDRESS    0xD0		/* I2C�ӻ���ַ */
#define MPU6050_I2C_SCL_PORT	GPIOB			/* SCL GPIO�˿� */
//#define RCC_I2C_PORT 	RCC_AHB1Periph_GPIOB		/* GPIO�˿�ʱ�� */
#define MPU6050_I2C_SCL_PIN		GPIO_PIN_6			/* ���ӵ�SCLʱ���ߵ�GPIO */
#define MPU6050_I2C_SDA_PORT	GPIOB			/* SDA GPIO�˿� */
#define MPU6050_I2C_SDA_PIN		GPIO_PIN_9			/* ���ӵ�SDA�����ߵ�GPIO */


typedef struct
{
	int16_t Accel_X;
	int16_t Accel_Y;
	int16_t Accel_Z;

	int16_t Temp;

	int16_t GYRO_X;
	int16_t GYRO_Y;
	int16_t GYRO_Z;

	int16_t accl_x_bias;
	int16_t accl_y_bias;
	int16_t accl_z_bias;
	int16_t gyro_x_bias;
	int16_t gyro_y_bias;
	int16_t gyro_z_bias;

	uint8_t bias_count;
	float roll;  //������ y
	float pitch; //������ x
	float yaw;  //ƫ���� z
}MPU6050_T;


extern MPU6050_T g_tMPU6050;

#endif


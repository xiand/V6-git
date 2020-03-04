#ifndef _MY_MPU6050_h_
#define _MY_MPU6050_h_

//****************************************
// 定义MPU6050内部地址
//****************************************
#define	SMPLRT_DIV		0x19	//陀螺仪采样率，典型值：0x07(125Hz)
#define	CONFIG			0x1A	//低通滤波频率，典型值：0x06(5Hz)
#define	GYRO_CONFIG		0x1B	//陀螺仪自检及测量范围，典型值：0x18(不自检，2000deg/s)

#define	ACCEL_CONFIG	0x1C	//加速计自检、测量范围及高通滤波频率，典型值：0x01(不自检，2G，5Hz)

#define INTERRUPT_STATUS 0x3A  //中断状态寄存器

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

#define	PWR_MGMT_1		0x6B	//电源管理，典型值：0x00(正常启用)
#define	WHO_AM_I		0x75	//IIC地址寄存器(默认数值0x68，只读)


#define MPU6050_SLAVE_ADDRESS    0xD0		/* I2C从机地址 */
#define MPU6050_I2C_SCL_PORT	GPIOB			/* SCL GPIO端口 */
//#define RCC_I2C_PORT 	RCC_AHB1Periph_GPIOB		/* GPIO端口时钟 */
#define MPU6050_I2C_SCL_PIN		GPIO_PIN_6			/* 连接到SCL时钟线的GPIO */
#define MPU6050_I2C_SDA_PORT	GPIOB			/* SDA GPIO端口 */
#define MPU6050_I2C_SDA_PIN		GPIO_PIN_9			/* 连接到SDA数据线的GPIO */


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
	float roll;  //翻滚角 y
	float pitch; //俯仰角 x
	float yaw;  //偏航角 z
}MPU6050_T;


extern MPU6050_T g_tMPU6050;

#endif


#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "I2C_module_simulate.h"
uint8_t mpu_data = 0;

void MPU6050_Init(void)
{
	uint8_t write_data = 0;
	
	I2C_Module_Read(USE_I2C_MPU6050,MPU6050_SLAVE_ADDRESS,WHO_AM_I,&mpu_data,1);
	if(mpu_data == 0x63)
	{
		write_data = 0x80;
		I2C_Module_Write(USE_I2C_MPU6050,MPU6050_SLAVE_ADDRESS,PWR_MGMT_1,&write_data,1);
        delay_Ms(5);
		write_data = 0x00;
		I2C_Module_Write(USE_I2C_MPU6050,MPU6050_SLAVE_ADDRESS,PWR_MGMT_1,&write_data,1);
        write_data = 0x07;
		I2C_Module_Write(USE_I2C_MPU6050,MPU6050_SLAVE_ADDRESS,SMPLRT_DIV,&write_data,1);   
        write_data = 0x06;
		I2C_Module_Write(USE_I2C_MPU6050,MPU6050_SLAVE_ADDRESS,CONFIG,&write_data,1);        
        write_data = 0x18;
		I2C_Module_Write(USE_I2C_MPU6050,MPU6050_SLAVE_ADDRESS,GYRO_CONFIG,&write_data,1);                  
        write_data = 0x01;
		I2C_Module_Write(USE_I2C_MPU6050,MPU6050_SLAVE_ADDRESS,ACCEL_CONFIG,&write_data,1);  
	}
}


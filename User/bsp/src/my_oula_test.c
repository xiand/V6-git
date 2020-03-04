
#include "bsp.h"
#include "my_math.h"

//define OFFSET_CALIB_DYN //动态标定

#ifdef OFFSET_CALIB_DYN
#include "icm206_gyro.h"
#endif

extern float	roll, pitch, yaw;


uint32_t		time_current = 0;
uint32_t		time_previous = 0;

extern float	xytheta_start[3];

#define GRAVITY_EARTH			9.81f
#define SCALE_GYRO				(2.66462e-4)		// 0.0001331581f		// 1*pi/65.5/180/65.5	 // (1.331581*1e-4f)			// (1000.0f*3.14f/(180.0f*65536.0f))		// 1000dps/65536=1000*3.14/180/65536=
#define SCALE_ACCEL 			0.0011971008f		// 9.81f/8192.0f				// 0.0011971008f			// (8.0f*GRAVITY_EARTH/65536.0f)

signed short int 			test_gyro[3] =
{
	0
};


signed short int test_acc[3] =
{
	0
};


//static float	gyro_raw_first[3] =
//{
//	0.0f
//};


//static float	accel_raw_first[3] =
//{
//	0.0f
//};


//static float	bias_gyro[3] =
//{
//	0.0f
//};


//static float	bias_accel[3] =
//{
//	0.0f
//};


//bool			start_first = true;
//bool			calib_before_start = false;
//extern bool 	bFactoryImuDemarcate;

#ifdef OFFSET_CALIB_DYN
s16 			gyro_offset_dyn[3] =
{
	0
};


#endif


void gyro_value_cal(float data[3])
{
	signed short int 			gyro[3] =
	{
		0
	};

	//	gyro[0] = GetDate(GYRO_XOUT_H);
	//	gyro[1] = GetDate(GYRO_YOUT_H);
	//	gyro[2] = GetDate(GYRO_ZOUT_H);
	gyro[0] 			= g_tMPU6050.GYRO_X;
	gyro[1] 			= g_tMPU6050.GYRO_Y;
	gyro[2] 			= g_tMPU6050.GYRO_Z;

	//	memcpy(test_gyro,gyro,sizeof(gyro));
	for (int i = 0; i < 3; i++)
	{
		test_gyro[i]		= gyro[i];
	}

	// //printf("gyro_0_int:%d	",gyro[0]);
#if 1

	if (gyro[0] > -20 && gyro[0] < 20)
		gyro[0] = 0;

	if (gyro[1] > -20 && gyro[1] < 20)
		gyro[1] = 0;

	if (gyro[2] > -20 && gyro[2] < 20)
		gyro[2] = 0;

#endif

	data[0] 			= SCALE_GYRO * gyro[0];
	data[1] 			= SCALE_GYRO * gyro[1];
	data[2] 			= SCALE_GYRO * gyro[2];

	// orientation match
#if OLD_MPU_VERSION

	// orientation match
	data[0] 			= -data[0];
	data[2] 			= -data[2];

#else

	float			tep = data[0];

	data[0] 			= data[1];
	data[1] 			= tep;
	data[2] 			= -data[2];

	// //printf("gyro_0:%d	%6.3f",gyro[0],data[0]);
#endif
}


void accel_value_cal(float data[3])
{
	signed short int 			accel[3] =
	{
		0
	};

	//	accel[0] = GetDate(ACCEL_XOUT_H);
	//	accel[1] = GetDate(ACCEL_YOUT_H);
	//	accel[2] = GetDate(ACCEL_ZOUT_H);
	accel[0]			= g_tMPU6050.Accel_X;
	accel[1]			= g_tMPU6050.Accel_Y;
	accel[2]			= g_tMPU6050.Accel_Z;

	//	memcpy(test_acc,accel,sizeof(accel));	
	for (int i = 0; i < 3; i++)
	{
		test_acc[i] 		= accel[i];
	}

#if 1

	if (accel[0] > -20 && accel[0] < 20)
		accel[0] = 0;

	if (accel[1] > -20 && accel[1] < 20)
		accel[1] = 0;

	if (accel[2] > -20 && accel[2] < 20)
		accel[2] = 0;

#endif

	data[0] 			= SCALE_ACCEL * accel[0];
	data[1] 			= SCALE_ACCEL * accel[1];
	data[2] 			= SCALE_ACCEL * accel[2];

#if OLD_MPU_VERSION

	// orientation match
	data[0] 			= -data[0];
	data[2] 			= -data[2];

#else

	float			tep = data[0];

	data[0] 			= data[1];
	data[1] 			= tep;
	data[2] 			= -data[2];

#endif
}


#if 0
typedef enum 
{
calibrate_no = 0, 
calibrating, 
calibrateFactoryMode, 
calibrated, 
} calibrate_status_e;


extern MachineRunMode e_MachineRunMode;
static calibrate_status_e gyro_calib_status = calibrate_no;
static calibrate_status_e accel_calib_status = calibrate_no;
static calibrate_status_e gyro_calib_status_first = calibrate_no;
static calibrate_status_e accel_calib_status_first = calibrate_no;


void xrobot_gyro_calib_update(float gyro_raw[3], float gyro_calibrated[3])
{
	u16 			bias_temp = 0;

	// //printf("mark0\r\n");
	static uint32_t counter_gyro = 0;

	counter_gyro++;

	//	 //printf("counter_gyro:%d\r\n",counter_gyro);
	switch (gyro_calib_status)
	{
		case calibrate_no:
			{
				//			 //printf("g_mark1 = %d\r\n",counter_gyro);
				gyro_calibrated[0]	= 0.0f;
				gyro_calibrated[1]	= 0.0f;
				gyro_calibrated[2]	= 0.0f;

				if (counter_gyro >= 20)
				{
					gyro_calib_status	= calibrating;
				}

				break;
			}

		case calibrating:
			{
				//			 //printf("g_mark2 = %d\r\n",counter_gyro);
				static float	gyro_sum[3] =
				{
					0.0f
				};
				gyro_sum[0] 		+= gyro_raw[0];
				gyro_sum[1] 		+= gyro_raw[1];
				gyro_sum[2] 		+= gyro_raw[2];

				// 21 - 119
				if (counter_gyro < 120)
				{
					// //printf("mark3\r\n");
					gyro_calibrated[0]	= 0.0f;
					gyro_calibrated[1]	= 0.0f;
					gyro_calibrated[2]	= 0.0f;
				}
				else 
				{
					// //printf("mark4\r\n");
					// 120
					bias_gyro[0]		= gyro_sum[0] / 100.0f;
					bias_gyro[1]		= gyro_sum[1] / 100.0f;
					bias_gyro[2]		= gyro_sum[2] / 100.0f;


					gyro_sum[0] 		= 0;
					gyro_sum[1] 		= 0;
					gyro_sum[2] 		= 0;
					gyro_calibrated[0]	= 0.0f;
					gyro_calibrated[1]	= 0.0f;
					gyro_calibrated[2]	= 0.0f;


					if (bFactoryImuDemarcate)
					{
						if (bias_gyro[0] >= 0)
							bias_temp = (uint16_t) (bias_gyro[0] *100000);
						else 
							bias_temp = 65536 - ABS(bias_gyro[0] *100000);

						//printf("test_bias_gyrox = %d\r\n",bias_temp);
						STMFLASH_Write(gyroBiosAddrX, &bias_temp, 1); //???????flash

						if (bias_gyro[1] >= 0)
							bias_temp = (uint16_t) (bias_gyro[1] *100000);
						else 
							bias_temp = 65536 - ABS(bias_gyro[1] *100000);

						//printf("test_bias_gyroy = %d\r\n",bias_temp);				
						STMFLASH_Write(gyroBiosAddrY, &bias_temp, 1);

						if (bias_gyro[2] >= 0)
							bias_temp = (uint16_t) (bias_gyro[2] *100000);
						else 
							bias_temp = 65536 - ABS(bias_gyro[2] *100000);

						//printf("test_bias_gyroz = %d\r\n",bias_temp);				
						STMFLASH_Write(gyroBiosAddrZ, &bias_temp, 1);
						printf("fac_bias_gyro:%6.3f,%6.3f,%6.3f,%d\r\n", bias_gyro[0], bias_gyro[1], bias_gyro[2],
							 sys_time / 1000);
						gyro_calib_status	= calibrateFactoryMode;
						break;
					}
					else 
					{
						bias_temp			= STMFLASH_ReadHalfWord(gyroBiosAddrX);

						//					//printf("read_gyrox = %d\r\n",bias_temp);
						if (bias_temp <= 32768)
							bias_gyro[0] = (float)
							bias_temp / 100000;
						else 
							bias_gyro[0] = (float) ((-1) * (65536 - bias_temp)) / 100000;

						bias_temp			= STMFLASH_ReadHalfWord(gyroBiosAddrY);

						//					//printf("read_gyroy = %d\r\n",bias_temp);
						if (bias_temp <= 32768)
							bias_gyro[1] = (float)
							bias_temp / 100000;
						else 
							bias_gyro[1] = (float) ((-1) * (65536 - bias_temp)) / 100000;

						bias_temp			= STMFLASH_ReadHalfWord(gyroBiosAddrZ);

						//					//printf("read_gyroz = %d\r\n",bias_temp);
						if (bias_temp <= 32768)
							bias_gyro[2] = (float)
							bias_temp / 100000;
						else 
							bias_gyro[2] = (float) ((-1) * (65536 - bias_temp)) / 100000;

#ifdef LOG_DEBUG_TANG
						printf("bias_gyro:%6.3f,%6.3f,%6.3f,%d\r\n", bias_gyro[0], bias_gyro[1], bias_gyro[2],
							 sys_time / 1000);
#endif

						gyro_calib_status	= calibrated;
					}

				}

				break;
			}

		case calibrateFactoryMode:
			if (e_MachineRunMode == eMachineNormolMode)
			{
				//printf("gyro_factory - normal\r\n");	
				bFactoryImuDemarcate = false;
				gyro_calib_status	= calibrate_no;
				counter_gyro		= 0;
				break;
			}

			gyro_calibrated[0] = gyro_raw[0] -bias_gyro[0];
			gyro_calibrated[1] = gyro_raw[1] -bias_gyro[1];
			gyro_calibrated[2] = gyro_raw[2] -bias_gyro[2];
			break;

		case calibrated:
			{
				if (bFactoryImuDemarcate) //开始IMU/gyro标定	
				{
					//					printf("gyro_normal - factory\r\n");
					gyro_calib_status	= calibrate_no;
					counter_gyro		= 0;
					break;
				}

				//				memcpy(gyro_calibrated,bias_gyro,sizeof(bias_gyro));		
				gyro_calibrated[0]	= gyro_raw[0] -bias_gyro[0];
				gyro_calibrated[1]	= gyro_raw[1] -bias_gyro[1];
				gyro_calibrated[2]	= gyro_raw[2] -bias_gyro[2];
				break;
			}

		default:
			// //printf("mark6\r\n");
			break;
	}
}


uint32_t		counter_gyro_first = 0;
float			gyro_sum_first[3] =
{
	0.0f
};


void xrobot_gyro_calib_update_first()
{
	u16 			bias_temp = 0;

	// //printf("mark0\r\n");
	counter_gyro_first++;

	//	 //printf("counter_gyro:%d\r\n",counter_gyro);
	switch (gyro_calib_status_first)
	{
		case calibrate_no:
			{
				//			 //printf("g_mark1_first\r\n");
				if (counter_gyro_first >= 20)
				{
					gyro_sum_first[0]	= 0.0f;
					gyro_sum_first[1]	= 0.0f;
					gyro_sum_first[2]	= 0.0f;
					gyro_calib_status_first = calibrating;
				}

				break;
			}

		case calibrating:
			{
				//			 //printf("g_mark2\r\n");
				// //printf("calibrating:%d\r\n",counter_gyro_first);
				gyro_sum_first[0]	+= gyro_raw_first[0]; // gyro_raw
				gyro_sum_first[1]	+= gyro_raw_first[1];
				gyro_sum_first[2]	+= gyro_raw_first[2];

				// 21 - 119
				if (counter_gyro_first < 120)
				{

				}
				else 
				{
					// //printf("mark4\r\n");
					// 120
					bias_gyro[0]		= gyro_sum_first[0] / 100.0f;
					bias_gyro[1]		= gyro_sum_first[1] / 100.0f;
					bias_gyro[2]		= gyro_sum_first[2] / 100.0f;


					if (bias_gyro[0] >= 0)
						bias_temp = (uint16_t) (bias_gyro[0] *100000);
					else 
						bias_temp = 65536 - ABS(bias_gyro[0] *100000);

					//printf("first_bias_gyrox = %d\r\n",bias_temp);
					STMFLASH_Write(gyroBiosAddrX, &bias_temp, 1); //讲标定参数写入flash

					if (bias_gyro[1] >= 0)
						bias_temp = (uint16_t) (bias_gyro[1] *100000);
					else 
						bias_temp = 65536 - ABS(bias_gyro[1] *100000);

					//printf("first_bias_gyroy = %d\r\n",bias_temp);				
					STMFLASH_Write(gyroBiosAddrY, &bias_temp, 1);

					if (bias_gyro[2] >= 0)
						bias_temp = (uint16_t) (bias_gyro[2] *100000);
					else 
						bias_temp = 65536 - ABS(bias_gyro[2] *100000);

					//printf("first_bias_gyroz = %d\r\n",bias_temp);				
					STMFLASH_Write(gyroBiosAddrZ, &bias_temp, 1);

					start_first 		= false;	//标定完成标志

					counter_gyro_first	= 0;
					memset(gyro_sum_first, 0x00, sizeof(gyro_sum_first));
				}

				break;
			}

		case calibrated:
			{
				break;
			}

		default:
			// //printf("mark6\r\n");
			break;
	}
}


extern egetSensor e_egetSensor;


void xrobot_accel_calib_update(float accel_raw[3], float accel_calibrated[3])
{
	u16 			bias_temp = 0;

	static uint32_t counter_accel = 0;

	counter_accel++;

	switch (accel_calib_status)
	{
		case calibrate_no:
			{
				//			 //printf("a_mark1 = %d\r\n",counter_accel);
				accel_calibrated[0] = 0.0f;
				accel_calibrated[1] = 0.0f;
				accel_calibrated[2] = 0.0f;

				if (counter_accel >= 20)
				{
					accel_calib_status	= calibrating;
				}

				break;
			}

		case calibrating:
			{
				static float	accel_sum[3] =
				{
					0.0f
				};
				accel_sum[0]		+= (accel_raw[0]);
				accel_sum[1]		+= (accel_raw[1]);
				accel_sum[2]		+= (accel_raw[2] +GRAVITY_EARTH);

				//			//printf("a_mark2 = %d\r\n",counter_accel);
				// 21 - 119
				if (counter_accel < 120)
				{
					accel_calibrated[0] = 0.0f;
					accel_calibrated[1] = 0.0f;
					accel_calibrated[2] = 0.0f;
				}
				else 
				{
					// 120
					counter_accel		= 0;
					bias_accel[0]		= accel_sum[0] / 100.0f;
					bias_accel[1]		= accel_sum[1] / 100.0f;
					bias_accel[2]		= accel_sum[2] / 100.0f;

					accel_sum[0]		= 0;
					accel_sum[1]		= 0;
					accel_sum[2]		= 0;
					accel_calibrated[0] = 0.0f;
					accel_calibrated[1] = 0.0f;
					accel_calibrated[2] = 0.0f;

					if (bFactoryImuDemarcate)
					{
						if (bias_accel[0] >= 0)
							bias_temp = (uint16_t) (bias_accel[0] *10000);
						else 
							bias_temp = 65536 - ABS(bias_accel[0] *10000);

						//printf("test_bias_accx = %d\r\n",bias_temp);
						STMFLASH_Write(accBiosAddrX, &bias_temp, 1); //???????flash

						if (bias_accel[1] >= 0)
							bias_temp = (uint16_t) (bias_accel[1] *10000);
						else 
							bias_temp = 65536 - ABS(bias_accel[1] *10000);

						//printf("test_bias_accy = %d\r\n",bias_temp);				
						STMFLASH_Write(accBiosAddrY, &bias_temp, 1);

						if (bias_accel[2] >= 0)
							bias_temp = (uint16_t) (bias_accel[2] *10000);
						else 
							bias_temp = 65536 - ABS(bias_accel[2] *10000);

						//printf("test_bias_accz = %d\r\n",bias_temp);				
						STMFLASH_Write(accBiosAddrZ, &bias_temp, 1);

						printf("fac_bias_accel:%6.3f,%6.3f,%6.3f,%d\r\n", bias_accel[0], bias_accel[1], bias_accel[2],
							 sys_time / 1000);
						accel_calib_status	= calibrateFactoryMode;
						break;
					}
					else 
					{
						bias_temp			= STMFLASH_ReadHalfWord(accBiosAddrX);

						//					//printf("read_accx = %d\r\n",bias_temp);
						if (bias_temp <= 32768)
							bias_accel[0] = (float)
							bias_temp / 10000;
						else 
							bias_accel[0] = (float) ((-1) * (65536 - bias_temp)) / 10000;

						bias_temp			= STMFLASH_ReadHalfWord(accBiosAddrY);

						//					//printf("read_accy = %d\r\n",bias_temp);
						if (bias_temp <= 32768)
							bias_accel[1] = (float)
							bias_temp / 10000;
						else 
							bias_accel[1] = (float) ((-1) * (65536 - bias_temp)) / 10000;

						bias_temp			= STMFLASH_ReadHalfWord(accBiosAddrZ);

						//					//printf("read_accz = %d\r\n",bias_temp);
						if (bias_temp <= 32768)
							bias_accel[2] = (float)
							bias_temp / 10000;
						else 
							bias_accel[2] = (float) ((-1) * (65536 - bias_temp)) / 10000;

#ifdef LOG_DEBUG_TANG
						printf("bias_accel:%6.3f,%6.3f,%6.3f,%d\r\n", bias_accel[0], bias_accel[1], bias_accel[2],
							 sys_time / 1000);
#endif

						accel_calib_status	= calibrated;
					}

				}

				break;
			}

		case calibrateFactoryMode:
			if (e_MachineRunMode == eMachineNormolMode)
			{
				//printf("acc_factory - normal\r\n");	
				bFactoryImuDemarcate = false;
				accel_calib_status	= calibrate_no;
				counter_accel		= 0;
				break;
			}
			else 
			{
				accel_calibrated[0] = accel_raw[0] -bias_accel[0];
				accel_calibrated[1] = accel_raw[1] -bias_accel[1];
				accel_calibrated[2] = accel_raw[2] -bias_accel[2];
			}

			break;

		case calibrated:
			{
				if (bFactoryImuDemarcate) //开始IMU/acc标定	
				{
					//printf("acc_normal - factory\r\n");
					accel_calib_status	= calibrate_no;
					counter_accel		= 0;
					break;
				}

				memcpy(accel_calibrated, bias_accel, sizeof(bias_accel));
				accel_calibrated[0] = accel_raw[0] -bias_accel[0];
				accel_calibrated[1] = accel_raw[1] -bias_accel[1];
				accel_calibrated[2] = accel_raw[2] -bias_accel[2];


				break;
			}

		default:
			break;
	}
}


uint32_t		counter_accel_first = 0;
float			accel_sum_first[3] =
{
	0.0f
};


void xrobot_accel_calib_update_first()
{
	u16 			bias_temp = 0;

	// //printf("mark0\r\n");
	counter_accel_first++;

	//	 //printf("counter_gyro:%d\r\n",counter_gyro);
	switch (accel_calib_status_first)
	{
		case calibrate_no:
			{
				//			 //printf("a_mark1_first\r\n");
				if (counter_accel_first >= 20)
				{
					accel_sum_first[0]	= 0.0f;
					accel_sum_first[1]	= 0.0f;
					accel_sum_first[2]	= 0.0f;
					accel_calib_status_first = calibrating;
				}

				break;
			}

		case calibrating:
			{
				//			 //printf("a_mark2_first\r\n");
				accel_sum_first[0]	+= accel_raw_first[0]; // gyro_raw
				accel_sum_first[1]	+= accel_raw_first[1];

				// accel_sum[2] += accel_raw_first[2];
				accel_sum_first[2]	+= (accel_raw_first[2] +GRAVITY_EARTH);

				// 21 - 119
				if (counter_accel_first < 120)
				{

				}
				else 
				{
					// //printf("mark4\r\n");
					// 120
					bias_accel[0]		= accel_sum_first[0] / 100.0f;
					bias_accel[1]		= accel_sum_first[1] / 100.0f;
					bias_accel[2]		= accel_sum_first[2] / 100.0f;


					if (bias_accel[0] >= 0)
						bias_temp = (uint16_t) (bias_accel[0] *10000);
					else 
						bias_temp = 65536 - ABS(bias_accel[0] *10000);

					//printf("first_bias_accx = %d\r\n",bias_temp);
					STMFLASH_Write(accBiosAddrX, &bias_temp, 1); //讲标定参数写入flash

					if (bias_accel[1] >= 0)
						bias_temp = (uint16_t) (bias_accel[1] *10000);
					else 
						bias_temp = 65536 - ABS(bias_accel[1] *10000);

					//printf("first_bias_accy = %d\r\n",bias_temp);				
					STMFLASH_Write(accBiosAddrY, &bias_temp, 1);

					if (bias_accel[2] >= 0)
						bias_temp = (uint16_t) (bias_accel[2] *10000);
					else 
						bias_temp = 65536 - ABS(bias_accel[2] *10000);

					//printf("first_bias_accz = %d\r\n",bias_temp);				
					STMFLASH_Write(accBiosAddrZ, &bias_temp, 1);

					start_first 		= false;

					counter_accel_first = 0;
					memset(accel_sum_first, 0x00, sizeof(accel_sum_first));
				}

				break;
			}

		case calibrated:
			{
				break;
			}

		default:
			// //printf("mark6\r\n");
			break;
	}
}


#endif


#define TWO_KP_DEF				0.8f
#define TWO_KI_DEF				0.002f		// 0.1f		// 0.002f 

static float	q0 = 1.0f;
static float	q1 = 0.0f;
static float	q2 = 0.0f;
static float	q3 = 0.0f;

float			twoKp = TWO_KP_DEF;
float			twoKi = TWO_KI_DEF;
float			integralFBx = 0.0f;
float			integralFBy = 0.0f;
float			integralFBz = 0.0f;

float invSqrt(float x);


#if 0
#define IMU_SAMPLE_FREQ 		400
#define IMU_CUTOFF_FREQ 		30

lpf2p_param 	gyro_filter_param[3];
lpf2p_param 	accel_filter_param[3];


void xrobot_attitude_estimation_init()
{
	memset(gyro_filter_param, 0x00, sizeof(gyro_filter_param));
	memset(accel_filter_param, 0x00, sizeof(accel_filter_param));
	set_cutoff_param(&gyro_filter_param[0], IMU_SAMPLE_FREQ, IMU_CUTOFF_FREQ);
	set_cutoff_param(&gyro_filter_param[1], IMU_SAMPLE_FREQ, IMU_CUTOFF_FREQ);
	set_cutoff_param(&gyro_filter_param[2], IMU_SAMPLE_FREQ, IMU_CUTOFF_FREQ);

	set_cutoff_param(&accel_filter_param[0], IMU_SAMPLE_FREQ, IMU_CUTOFF_FREQ);
	set_cutoff_param(&accel_filter_param[1], IMU_SAMPLE_FREQ, IMU_CUTOFF_FREQ);
	set_cutoff_param(&accel_filter_param[2], IMU_SAMPLE_FREQ, IMU_CUTOFF_FREQ);


	q0					= 1.0f;
	q1					= 0.0f;
	q2					= 0.0f;
	q3					= 0.0f;

	integralFBx 		= 0.0f;
	integralFBy 		= 0.0f;
	integralFBz 		= 0.0f;

#ifdef OFFSET_CALIB_DYN
	icm206xx_gyro_INIT();
	gyro_offset_dyn[0]	= 0;
	gyro_offset_dyn[1]	= 0;
	gyro_offset_dyn[2]	= 0;
#endif
}
#endif

void xrobot_attitude_estimation_update(float gyro[3], float accel[3], float RPY[3], float dtt)
{
	float			accel_sqr_sum = accel[0] *accel[0] +accel[1] *accel[1] +accel[2] *accel[2];
//	float			accel_length = sqrtf(accel_sqr_sum); //  1.0f / invSqrt(accel_sqr_sum);		// 
	float			accel_length = yx_sqrt(accel_sqr_sum); //  1.0f / invSqrt(accel_sqr_sum);		// 
	if (accel_length > 0.00001f)
	{
		float			recipNorm_accel = invSqrt(accel_sqr_sum);

		accel[0]			*= recipNorm_accel;
		accel[1]			*= recipNorm_accel;
		accel[2]			*= recipNorm_accel;

		float			halfvx = q1 * q3 - q0 * q2;
		float			halfvy = q0 * q1 + q2 * q3;
		float			halfvz = q0 * q0 - 0.5f + q3 * q3;

		// float halfvz = 0.5f - q1*q1 - q2*q2;
		float			halfex = - (accel[1] *halfvz - accel[2] *halfvy);
		float			halfey = - (accel[2] *halfvx - accel[0] *halfvz);
		float			halfez = - (accel[0] *halfvy - accel[1] *halfvx);

		if (twoKi > 0.0f)
		{
			integralFBx 		+= halfex * dtt;
			integralFBy 		+= halfey * dtt;
			integralFBz 		+= halfez * dtt;
		}
		else 
		{
			integralFBx 		= 0.0f;
			integralFBy 		= 0.0f;
			integralFBz 		= 0.0f;
		}

		gyro[0] 			+= twoKi * integralFBx;
		gyro[1] 			+= twoKi * integralFBy;
		gyro[2] 			+= twoKi * integralFBz;

		gyro[0] 			+= twoKp * halfex;
		gyro[1] 			+= twoKp * halfey;
		gyro[2] 			+= twoKp * halfez;
	}

	gyro[0] 			*= (0.5f * dtt);
	gyro[1] 			*= (0.5f * dtt);
	gyro[2] 			*= (0.5f * dtt);

	float			qa	= q0;
	float			qb	= q1;
	float			qc	= q2;

	q0					+= (-qb * gyro[0] -qc * gyro[1] -q3 * gyro[2]);
	q1					+= (qa * gyro[0] +qc * gyro[2] -q3 * gyro[1]);
	q2					+= (qa * gyro[1] -qb * gyro[2] +q3 * gyro[0]);
	q3					+= (qa * gyro[2] +qb * gyro[1] -qc * gyro[0]);

	float			recipNorm_q = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);

	q0					*= recipNorm_q;
	q1					*= recipNorm_q;
	q2					*= recipNorm_q;
	q3					*= recipNorm_q;

	// quartion to roll/pitch/yaw
//	RPY[0]				= atan2f(2.0f * (q0 * q1 + q2 * q3), q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3);
//	RPY[1]				= asinf(-2.0f * (q1 * q3 - q0 * q2));
//	RPY[2]				= atan2f(2.0f * q1 * q2 + 2.0f * q0 * q3, q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3);

	RPY[0]				= yx_atan2_f32(2.0f * (q0 * q1 + q2 * q3), q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3);
	RPY[1]				= yx_sin_f32(-2.0f * (q1 * q3 - q0 * q2));
	RPY[2]				= yx_atan2_f32(2.0f * q1 * q2 + 2.0f * q0 * q3, q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3);
}



float invSqrt(float x)
{
	float			halfx = 0.5f * x;
	float			y	= x;
	long			i	= * (long *) &y;

	i					= 0x5f3759df - (i >> 1);
	y					= * (float *) &i;
	y					= y * (1.5f - (halfx * y * y));
	return y;
}


extern float	roll, pitch, yaw;
float			attitude[3] =
{
	0.0f
};


float			accSum[3] =
{
	0.0f
};


float			gyroSum[3] =
{
	0.0f
};


//uint32_t		imuSumCount = 0;
//extern RobotChargeStatus e_RobotChargeStatus;
//extern RobotCleanStatus e_RobotCleanStatus;
float			gyro_out[3] =
{
	0.0f
};


float			gyro_pkg[3] =
{
	0.0f
};


float			accel_pkg[3] =
{
	0.0f
};

extern uint32_t giv_sys_time;
float			dtt = 0.002f;

void xrobot_eulerAngle_estimation()
{
	float			gyro_raw[3] =
	{
		0.0f
	};
	float			accel_raw[3] =
	{
		0.0f
	};
	float			gyro[3] =
	{
		0.0f
	};
	float			accel[3] =
	{
		0.0f
	};

//	if ((RobotModeType == eRobotModeSleep) || (RobotModeType == eRobotModeUpdata) //		||(e_KeyPowerStatus != eKeyR16PowerOn)
//) //升级和没开始清扫的时候、不检测碰撞传感器
//	{
//		return;
//	}

	gyro_value_cal(gyro_raw);
	accel_value_cal(accel_raw);
//	xrobot_gyro_calib_update(gyro_raw, gyro);
//	xrobot_accel_calib_update(accel_raw, accel);

	gyro[0] = gyro_raw[0];
	gyro[1] = gyro_raw[1];
	gyro[2] = gyro_raw[2];

	accel[0] = accel_raw[0];
	accel[1] = accel_raw[1];
	accel[2] = accel_raw[2];

	// filter
	//	gyro[0] = lowpassfilter2p(&gyro_filter_param[0], gyro[0]);
	//	gyro[1] = lowpassfilter2p(&gyro_filter_param[1], gyro[1]);
	//	gyro[2] = lowpassfilter2p(&gyro_filter_param[2], gyro[2]);
	//	
	//	accel[0] = lowpassfilter2p(&accel_filter_param[0], accel[0]);
	//	accel[1] = lowpassfilter2p(&accel_filter_param[1], accel[1]);
	//	accel[2] = lowpassfilter2p(&accel_filter_param[2], accel[2]);
	accSum[0]			+= accel[0];
	accSum[1]			+= accel[1];
	accSum[2]			+= accel[2];

	gyroSum[0]			+= gyro[0];
	gyroSum[1]			+= gyro[1];
	gyroSum[2]			+= gyro[2];
//	imuSumCount++;


#ifdef OFFSET_CALIB_DYN
	s16 			gyro_int16[3];

	gyro_int16[0]		= gyro[0] / SCALE_GYRO;
	gyro_int16[1]		= gyro[1] / SCALE_GYRO;
	gyro_int16[2]		= gyro[2] / SCALE_GYRO;

	icm206xx_gyro_ZeroRateOffset_Calibration(gyro_int16[0], gyro_int16[1], gyro_int16[2], &gyro_offset_dyn[0],
		 &gyro_offset_dyn[1], &gyro_offset_dyn[2]);
	gyro_int16[0]		-= gyro_offset_dyn[0];
	gyro_int16[1]		-= gyro_offset_dyn[1];
	gyro_int16[2]		-= gyro_offset_dyn[2];

	gyro[0] 			= SCALE_GYRO * gyro_int16[0];
	gyro[1] 			= SCALE_GYRO * gyro_int16[1];
	gyro[2] 			= SCALE_GYRO * gyro_int16[2];
#endif

	//	if(sys_time - tctc < 200)
	//		return;
	//	tctc = sys_time;
	time_current		= giv_sys_time;
	int32_t 		delta_t_int = (time_current - time_previous);
	

	if (delta_t_int > 0 && delta_t_int < 500)
	{
		dtt 				= (time_current - time_previous) * 0.0001f;
	}

	memcpy(gyro_out, gyro, sizeof(gyro_out));

	//	printf("acc = %f,%f,%f,%f,%f,%f\r\n",gyro[0],gyro[1],gyro[2],accel[0],accel[1],accel[2]);
	xrobot_attitude_estimation_update(gyro, accel, attitude, dtt);
	time_previous		= time_current;
	g_tMPU6050.roll				= attitude[0] *57.3f;
	g_tMPU6050.pitch				= attitude[1] *57.3f;
	g_tMPU6050.yaw 				= attitude[2] *57.3f;

#ifdef FUSION_SENSOR
	updatePoseEstimate();

	// //printf("xy:%6.3f,%6.3f\r\n",X_xy_ekf[0],X_xy_ekf[2]);
#endif

	
	//	
}

#if 0
void xrobot_attitude_get(float attitude[3])
{
	attitude[0] 		= roll;
	attitude[1] 		= pitch;
	attitude[2] 		= yaw;
}


extern u8		l_enco_send, r_enco_send;
extern bool 	bdata_report;
extern uint32_t imu_data_cnt, encoder_report_cnt;
uint32_t		counter_imu = 0;


void xrobot_reportToR16_imuData()
{
	uint16_t		eulerSendPitch = 0, eulerSendRoll = 0, eulerSendYaw = 0;
	u8				report[ReportLenMax] =
	{
		0
	};
	int16_t 		accAverage[3] =
	{
		0
	},
	gyroAverage[3]		=
	{
		0
	};

	if ((RobotModeType == eRobotModeFast) || (RobotModeType == eRobotModeUpdata)) //(e_RobotCleanStatus != eRobotStartClean)升级和没开始清扫的时候、不检测碰撞传感器
		return;

	if (e_KeyPowerStatus != eKeyR16PowerOn)
		return;

	if ((encoder_report_cnt >= 2) || (bdata_report))
	{
		encoder_report_cnt	= 0;
		bdata_report		= false;

		if (imuSumCount > 0)
		{
			accSum[0]			/= imuSumCount;
			accSum[1]			/= imuSumCount;
			accSum[2]			/= imuSumCount;

			gyroSum[0]			/= imuSumCount;
			gyroSum[1]			/= imuSumCount;
			gyroSum[2]			/= imuSumCount;
			imuSumCount 		= 0;
		}

		gyroAverage[0]		= gyroSum[0] *1000;
		gyroAverage[1]		= gyroSum[1] *1000;
		gyroAverage[2]		= gyroSum[2] *1000;

		accAverage[0]		= accSum[0] *1000;
		accAverage[1]		= accSum[1] *1000;
		accAverage[2]		= accSum[2] *1000;
		float			eulerRoll = attitude[0] *57.3f;
		float			eulerPitch = attitude[1] *57.3f;
		float			eulerYaw = attitude[2] *57.3f;



		if (eulerPitch < 0)
		{
			eulerSendPitch		= 65536 - (uint16_t) (ABS(eulerPitch) * 100);
		}
		else 
			eulerSendPitch = eulerPitch * 100;

		if (eulerRoll < 0)
		{
			eulerSendRoll		= 65536 - (uint16_t) (ABS(eulerRoll) * 100);
		}
		else 
			eulerSendRoll = eulerRoll * 100;

		if (eulerYaw < 0)
		{
			eulerSendYaw		= 65536 - (uint16_t) (ABS(eulerYaw) * 100);
		}
		else 
			eulerSendYaw = eulerYaw * 100;



		report[0]			= (accAverage[0] >> 8) & 0xff;
		report[1]			= accAverage[0] &0x00ff;

		report[2]			= (accAverage[1] >> 8) & 0xff;
		report[3]			= accAverage[1] &0x00ff;

		report[4]			= (accAverage[2] >> 8) & 0xff;
		report[5]			= accAverage[2] &0x00ff;

		report[6]			= (gyroAverage[0] >> 8) & 0xff;
		report[7]			= gyroAverage[0] &0x00ff;

		report[8]			= (gyroAverage[1] >> 8) & 0xff;
		report[9]			= gyroAverage[1] &0x00ff;

		report[10]			= (gyroAverage[2] >> 8) & 0xff;
		report[11]			= gyroAverage[2] &0x00ff;

		report[12]			= (eulerSendPitch >> 8) & 0xff;
		report[13]			= eulerSendPitch & 0x00ff;

		report[14]			= (eulerSendRoll >> 8) & 0xff;
		report[15]			= eulerSendRoll & 0x00ff;

		report[16]			= (eulerSendYaw >> 8) & 0xff;
		report[17]			= eulerSendYaw & 0x00ff;

		xrobot_reported_packageProc(eSerialGyroData, report, 18);

		//			xrobot_mpu_reported_package(eSerialGyroData,accAverage[0],accAverage[1],accAverage[2],gyroAverage[0],gyroAverage[1],gyroAverage[2],eulerPitch,eulerRoll,eulerYaw);								
		xrobot_encoder_sensor_package(eSerialEncodeData, l_enco_send, r_enco_send);
		memset(gyroSum, 0x00, sizeof(gyroSum));
		memset(accSum, 0x00, sizeof(accSum));
	}

}



void xrobot_accel_get(float accel[3])
{
	memcpy(accel, accel_pkg, sizeof(accel_pkg));
}


void xrobot_gyro_get(float gyro[3])
{
	memcpy(gyro, gyro_pkg, sizeof(gyro_pkg));
}



void xrobot_SlamEkfAttitudeInit()
{

	xrobot_attitude_estimation_init();
	slam_acc_ekf_init();
}
#endif


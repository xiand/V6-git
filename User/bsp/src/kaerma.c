#include "math.h"
#include "bsp_mpu6050.h"
 
//MPU6050 accelgyro;
 
unsigned long now, lastTime = 0;
float dt;                                   //微分时间
 
int16_t ax, ay, az, gx, gy, gz;             //加速度计陀螺仪原始数据
float aax=0, aay=0,aaz=0, agx=0, agy=0, agz=0;    //角度变量
long axo = 0, ayo = 0, azo = 0;             //加速度计偏移量
long gxo = 0, gyo = 0, gzo = 0;             //陀螺仪偏移量
 
float pi = 3.1415926;
float AcceRatio = 16384.0;                  //加速度计比例系数
//float GyroRatio = 131.0;                    //陀螺仪比例系数
float GyroRatio = 16.375; 
uint8_t n_sample = 8;                       //加速度计滤波算法采样个数
float aaxs[8] = {0}, aays[8] = {0}, aazs[8] = {0};         //x,y轴采样队列
long aax_sum, aay_sum,aaz_sum;                      //x,y轴采样和
 
float a_x[10]={0}, a_y[10]={0},a_z[10]={0} ,g_x[10]={0} ,g_y[10]={0},g_z[10]={0}; //加速度计协方差计算队列
float Px=1, Rx, Kx, Sx, Vx, Qx;             //x轴卡尔曼变量
float Py=1, Ry, Ky, Sy, Vy, Qy;             //y轴卡尔曼变量
float Pz=1, Rz, Kz, Sz, Vz, Qz;             //z轴卡尔曼变量
 
void setup_init(void)
{
    unsigned short times = 200;             //采样次数
    for(int i=0;i<times;i++)
    {
        //accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz); //读取六轴原始数值
        MPU6050_ReadData();
		ax = g_tMPU6050.Accel_X;
		ay = g_tMPU6050.Accel_Y;
		az = g_tMPU6050.Accel_Z;

		gx = g_tMPU6050.GYRO_X;
		gy = g_tMPU6050.GYRO_Y;
		gz = g_tMPU6050.GYRO_Z;
		
        axo += ax; ayo += ay; azo += az;      //采样和
        gxo += gx; gyo += gy; gzo += gz;
    
    }
    
    axo /= times; ayo /= times; azo /= times; //计算加速度计偏移
    gxo /= times; gyo /= times; gzo /= times; //计算陀螺仪偏移
}


extern int32_t g_iRunTime;

void kaerma_updata(void)
{
    unsigned long now = g_iRunTime;             //当前时间(ms)
    dt = (now - lastTime) / 1000.0;           //微分时间(s)
    lastTime = now;                           //上一次采样时间(ms)
 
    //accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz); //读取六轴原始数值
	MPU6050_ReadData();
	ax = g_tMPU6050.Accel_X;
	ay = g_tMPU6050.Accel_Y;
	az = g_tMPU6050.Accel_Z;

	gx = g_tMPU6050.GYRO_X;
	gy = g_tMPU6050.GYRO_Y;
	gz = g_tMPU6050.GYRO_Z;
	
    float accx = ax / AcceRatio;              //x轴加速度
    float accy = ay / AcceRatio;              //y轴加速度
    float accz = az / AcceRatio;              //z轴加速度
 
    aax = atan(accy / accz) * (-180) / pi;    //y轴对于z轴的夹角
    aay = atan(accx / accz) * 180 / pi;       //x轴对于z轴的夹角
    aaz = atan(accz / accy) * 180 / pi;       //z轴对于y轴的夹角
 
    aax_sum = 0;                              // 对于加速度计原始数据的滑动加权滤波算法
    aay_sum = 0;
    aaz_sum = 0;
  
    for(int i=1;i<n_sample;i++)
    {
        aaxs[i-1] = aaxs[i];
        aax_sum += aaxs[i] * i;
        aays[i-1] = aays[i];
        aay_sum += aays[i] * i;
        aazs[i-1] = aazs[i];
        aaz_sum += aazs[i] * i;
    
    }
    
    aaxs[n_sample-1] = aax;
    aax_sum += aax * n_sample;
    aax = (aax_sum / (11*n_sample/2.0)) * 9 / 7.0; //角度调幅至0-90°
    aays[n_sample-1] = aay;                        //此处应用实验法取得合适的系数
    aay_sum += aay * n_sample;                     //本例系数为9/7
    aay = (aay_sum / (11*n_sample/2.0)) * 9 / 7.0;
    aazs[n_sample-1] = aaz; 
    aaz_sum += aaz * n_sample;
    aaz = (aaz_sum / (11*n_sample/2.0)) * 9 / 7.0;
 
    float gyrox = - (gx-gxo) / GyroRatio * dt; //x轴角速度
    float gyroy = - (gy-gyo) / GyroRatio * dt; //y轴角速度
    float gyroz = - (gz-gzo) / GyroRatio * dt; //z轴角速度
    agx += gyrox;                             //x轴角速度积分
    agy += gyroy;                             //x轴角速度积分
    agz += gyroz;
    
    /* kalman start */
    Sx = 0; Rx = 0;
    Sy = 0; Ry = 0;
    Sz = 0; Rz = 0;
    
    for(int i=1;i<10;i++)
    {                 //测量值平均值运算
        a_x[i-1] = a_x[i];                      //即加速度平均值
        Sx += a_x[i];
        a_y[i-1] = a_y[i];
        Sy += a_y[i];
        a_z[i-1] = a_z[i];
        Sz += a_z[i];
    
    }
    
    a_x[9] = aax;
    Sx += aax;
    Sx /= 10;                                 //x轴加速度平均值
    a_y[9] = aay;
    Sy += aay;
    Sy /= 10;                                 //y轴加速度平均值
    a_z[9] = aaz;
    Sz += aaz;
    Sz /= 10;
 
    for(int i=0;i<10;i++)
    {
        Rx += sqrt(a_x[i] - Sx);
        Ry += sqrt(a_y[i] - Sy);
        Rz += sqrt(a_z[i] - Sz);
    
    }
    
    Rx = Rx / 9;                              //得到方差
    Ry = Ry / 9;                        
    Rz = Rz / 9;
  
    Px = Px + 0.0025;                         // 0.0025在下面有说明...
    Kx = Px / (Px + Rx);                      //计算卡尔曼增益
    agx = agx + Kx * (aax - agx);             //陀螺仪角度与加速度计速度叠加
    Px = (1 - Kx) * Px;                       //更新p值
 
    Py = Py + 0.0025;
    Ky = Py / (Py + Ry);
    agy = agy + Ky * (aay - agy); 
    Py = (1 - Ky) * Py;
  
    Pz = Pz + 0.0025;
    Kz = Pz / (Pz + Rz);
    agz = agz + Kz * (aaz - agz); 
    Pz = (1 - Kz) * Pz;

  	g_tMPU6050.pitch = agx;
	g_tMPU6050.roll = agy;
	g_tMPU6050.yaw = agz;
    /* kalman end */
}

typedef enum
{
	PITCH = 0,
	ROLL,
	YAW,
}EULER_A;


#define PI  3.1415926
#define ACC_G  16384.0                 //加速度计比例系数
//float GyroRatio = 131.0;                    //陀螺仪比例系数
#define GCC_G  (16.4*57.3)


double one_filter_angle[3] = {0.0};
float  kalman_filter_angle[3] = {0};
float kalman_filter_angle_dot[3] = {0};

//----------------
//在进行滤波之前，需要获取原始数据 
float one_filter(float angle_m,float gyro_m,EULER_A euler)
{

    float  K1 =0.1; // 对加速度计取值的权重
    float  dt=0.005;//注意：dt的取值为滤波器采样时间

    one_filter_angle[euler] = K1 * angle_m+ (1-K1) * (one_filter_angle[euler] + gyro_m * dt);
    return one_filter_angle[euler];
}


//------------------
float kalman_filter(float angle_m,float gyro_m,EULER_A euler )
{

    //滤波参数
    float  dt = 0.005;   //卡尔曼采样时间
    float  P[2][2]    = {{1,0},{0,1}};
    float  Pdot[4]    = {0,0,0,0};
    float  Q_angle = 0.001;//角度数据置信度,陀螺仪协方差
    float  Q_gyro = 0.005;     //角速度数据置信度，陀螺仪飘移噪声协方差
    float  R_angle = 0.5;    //加速度计协方差
    char     C_0 = 1;
    float  q_bias = 0,angle_err = 0; //q_bias为陀螺仪飘移
    float  PCt_0 = 0,PCt_1 = 0,E = 0;
    float  K_0 = 0,  K_1 = 0,  t_0 = 0,  t_1 = 0;


     kalman_filter_angle[euler] += (gyro_m - q_bias) * dt;    //卡尔曼预测方程，认为每次飘移相同，


    Pdot[0]=Q_angle - P[0][1] - P[1][0];
    Pdot[1]=- P[1][1];
    Pdot[2]=- P[1][1];
    Pdot[3]=Q_gyro;

    P[0][0] += Pdot[0] * dt;
    P[0][1] += Pdot[1] * dt;
    P[1][0] += Pdot[2] * dt;
    P[1][1] += Pdot[3] * dt;

    PCt_0 = C_0 * P[0][0];     //矩阵乘法中间变量
    PCt_1 = C_0 * P[1][0];

    E = R_angle + C_0 * PCt_0;     //分母

    K_0 = PCt_0 / E;   //增益值
    K_1 = PCt_1 / E;

    angle_err = angle_m - kalman_filter_angle[euler];    
    kalman_filter_angle[euler] += K_0 * angle_err; //对状态的卡尔曼估计，最优角度
    q_bias += K_1 * angle_err;
    kalman_filter_angle_dot[euler] = gyro_m-q_bias;//最优角速度

    t_0 = PCt_0;     //矩阵计算中间变量
    t_1 = C_0 * P[0][1];

    P[0][0] -= K_0 * t_0;
    P[0][1] -= K_0 * t_1;
    P[1][0] -= K_1 * t_0;
    P[1][1] -= K_1 * t_1;

    return kalman_filter_angle[euler];
}

#define Kp 2.0f                        // 比例增益支配率收敛到加速度计/磁强计
#define Ki 0.001f                // 积分增益支配率的陀螺仪偏见的衔接
#define halfT 0.001f                // 采样周期的一半

float q0 = 1, q1 = 0, q2 = 0, q3 = 0;          // 四元数的元素，代表估计方向
float exInt = 0, eyInt = 0, ezInt = 0;        // 按比例缩小积分误差

float Yaw,Pitch,Roll;  //偏航角，俯仰角，翻滚角


void IMUupdate(float gx, float gy, float gz, float ax, float ay, float az)
{
        float norm;
        float vx, vy, vz;
        float ex, ey, ez;  

        // 测量正常化
        norm = sqrt(ax*ax + ay*ay + az*az);      
        ax = ax / norm;                   //单位化
        ay = ay / norm;
        az = az / norm;      

        // 估计方向的重力
        vx = 2*(q1*q3 - q0*q2);
        vy = 2*(q0*q1 + q2*q3);
        vz = q0*q0 - q1*q1 - q2*q2 + q3*q3;

        // 错误的领域和方向传感器测量参考方向之间的交叉乘积的总和
        ex = (ay*vz - az*vy);
        ey = (az*vx - ax*vz);
        ez = (ax*vy - ay*vx);

        // 积分误差比例积分增益
        exInt = exInt + ex*Ki;
        eyInt = eyInt + ey*Ki;
        ezInt = ezInt + ez*Ki;

        // 调整后的陀螺仪测量
        gx = gx + Kp*ex + exInt;
        gy = gy + Kp*ey + eyInt;
        gz = gz + Kp*ez + ezInt;

        // 整合四元数率和正常化
        q0 = q0 + (-q1*gx - q2*gy - q3*gz)*halfT;
        q1 = q1 + (q0*gx + q2*gz - q3*gy)*halfT;
        q2 = q2 + (q0*gy - q1*gz + q3*gx)*halfT;
        q3 = q3 + (q0*gz + q1*gy - q2*gx)*halfT;  

        // 正常化四元
        norm = sqrt(q0*q0 + q1*q1 + q2*q2 + q3*q3);
        q0 = q0 / norm;
        q1 = q1 / norm;
        q2 = q2 / norm;
        q3 = q3 / norm;

        g_tMPU6050.pitch  = asin(-2 * q1 * q3 + 2 * q0* q2)* 57.3; // pitch ,转换为度数
        g_tMPU6050.roll = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2* q2 + 1)* 57.3; // rollv
        g_tMPU6050.yaw = atan2(2*(q1*q2 + q0*q3),q0*q0+q1*q1-q2*q2-q3*q3) * 57.3;               
}



float  acc_angle_x = 0,gyro_angle_x = 0;    //一阶互补的参数传递
float  acc_angle_y = 0,gyro_angle_y = 0; 
float  acc_angle_z = 0,gyro_angle_z = 0; 

void kalman_filter_deal(void)
{
	static uint32_t last_time = 0;
	
	float  init_ax = 0,init_ay = 0,init_az = 0;
	
	
	MPU6050_ReadData();
	init_ax =  (float)g_tMPU6050.Accel_X / ACC_G;		 //计算单位为G的各轴重力加速度分量
	init_ay =  (float)g_tMPU6050.Accel_Y / ACC_G;
	init_az =  (float)g_tMPU6050.Accel_Z / ACC_G;

	if(last_time == 0)
	{
		last_time = g_iRunTime;
	}
//	aax = atan(accy / accz) * (-180) / pi;	  //y轴对于z轴的夹角
//	  aay = atan(accx / accz) * 180 / pi;		//x轴对于z轴的夹角
//	  aaz = atan(accz / accy) * 180 / pi;		//z轴对于y轴的夹角

	acc_angle_x = atan(init_ax/init_az) * 180 / PI ;    //加速度计x轴角度-x轴相对于z轴
	acc_angle_y = atan(init_ay/init_az) * 180 / PI;     //y轴相对于z轴
	acc_angle_z = atan(init_az/init_ay) * 180 / PI;
	
	gyro_angle_x = -(float)(g_tMPU6050.GYRO_Y-gyo)/ GCC_G;	 //陀螺仪计算x轴角度??
	gyro_angle_y = -(float)(g_tMPU6050.GYRO_X-gxo)/ GCC_G;
	gyro_angle_z = -(float)(g_tMPU6050.GYRO_Z-gzo)/ GCC_G;

//	g_tMPU6050.pitch = one_filter(acc_angle_x,gyro_angle_x,PITCH);		 //一阶互补滤波，获取偏x轴角度
//	g_tMPU6050.roll = one_filter(acc_angle_y,gyro_angle_y,ROLL);
//	g_tMPU6050.yaw = one_filter(acc_angle_z,gyro_angle_z,YAW);
	
	g_tMPU6050.pitch = kalman_filter(acc_angle_x,gyro_angle_x,PITCH); 	//获取卡尔曼滤波
	g_tMPU6050.roll = kalman_filter(acc_angle_y,gyro_angle_y,ROLL); 	
	g_tMPU6050.yaw = kalman_filter(acc_angle_z,gyro_angle_z,YAW); 	


//	gyro_angle_x += ((float)(g_tMPU6050.GYRO_X-gxo)/ GCC_G)*(g_iRunTime - last_time)/1000;
//	gyro_angle_y += ((float)(g_tMPU6050.GYRO_Y-gyo)/ GCC_G)*(g_iRunTime - last_time)/1000;
//	gyro_angle_z += ((float)(g_tMPU6050.GYRO_Z-gzo)/ GCC_G)*(g_iRunTime - last_time)/1000;
//
//
//	IMUupdate(gyro_angle_x, gyro_angle_y, gyro_angle_z, acc_angle_x, acc_angle_y, acc_angle_z);

}





#include "math.h"
#include "bsp_mpu6050.h"
 
//MPU6050 accelgyro;
 
unsigned long now, lastTime = 0;
float dt;                                   //΢��ʱ��
 
int16_t ax, ay, az, gx, gy, gz;             //���ٶȼ�������ԭʼ����
float aax=0, aay=0,aaz=0, agx=0, agy=0, agz=0;    //�Ƕȱ���
long axo = 0, ayo = 0, azo = 0;             //���ٶȼ�ƫ����
long gxo = 0, gyo = 0, gzo = 0;             //������ƫ����
 
float pi = 3.1415926;
float AcceRatio = 16384.0;                  //���ٶȼƱ���ϵ��
//float GyroRatio = 131.0;                    //�����Ǳ���ϵ��
float GyroRatio = 16.375; 
uint8_t n_sample = 8;                       //���ٶȼ��˲��㷨��������
float aaxs[8] = {0}, aays[8] = {0}, aazs[8] = {0};         //x,y���������
long aax_sum, aay_sum,aaz_sum;                      //x,y�������
 
float a_x[10]={0}, a_y[10]={0},a_z[10]={0} ,g_x[10]={0} ,g_y[10]={0},g_z[10]={0}; //���ٶȼ�Э����������
float Px=1, Rx, Kx, Sx, Vx, Qx;             //x�Ῠ��������
float Py=1, Ry, Ky, Sy, Vy, Qy;             //y�Ῠ��������
float Pz=1, Rz, Kz, Sz, Vz, Qz;             //z�Ῠ��������
 
void setup_init(void)
{
    unsigned short times = 200;             //��������
    for(int i=0;i<times;i++)
    {
        //accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz); //��ȡ����ԭʼ��ֵ
        MPU6050_ReadData();
		ax = g_tMPU6050.Accel_X;
		ay = g_tMPU6050.Accel_Y;
		az = g_tMPU6050.Accel_Z;

		gx = g_tMPU6050.GYRO_X;
		gy = g_tMPU6050.GYRO_Y;
		gz = g_tMPU6050.GYRO_Z;
		
        axo += ax; ayo += ay; azo += az;      //������
        gxo += gx; gyo += gy; gzo += gz;
    
    }
    
    axo /= times; ayo /= times; azo /= times; //������ٶȼ�ƫ��
    gxo /= times; gyo /= times; gzo /= times; //����������ƫ��
}


extern int32_t g_iRunTime;

void kaerma_updata(void)
{
    unsigned long now = g_iRunTime;             //��ǰʱ��(ms)
    dt = (now - lastTime) / 1000.0;           //΢��ʱ��(s)
    lastTime = now;                           //��һ�β���ʱ��(ms)
 
    //accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz); //��ȡ����ԭʼ��ֵ
	MPU6050_ReadData();
	ax = g_tMPU6050.Accel_X;
	ay = g_tMPU6050.Accel_Y;
	az = g_tMPU6050.Accel_Z;

	gx = g_tMPU6050.GYRO_X;
	gy = g_tMPU6050.GYRO_Y;
	gz = g_tMPU6050.GYRO_Z;
	
    float accx = ax / AcceRatio;              //x����ٶ�
    float accy = ay / AcceRatio;              //y����ٶ�
    float accz = az / AcceRatio;              //z����ٶ�
 
    aax = atan(accy / accz) * (-180) / pi;    //y�����z��ļн�
    aay = atan(accx / accz) * 180 / pi;       //x�����z��ļн�
    aaz = atan(accz / accy) * 180 / pi;       //z�����y��ļн�
 
    aax_sum = 0;                              // ���ڼ��ٶȼ�ԭʼ���ݵĻ�����Ȩ�˲��㷨
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
    aax = (aax_sum / (11*n_sample/2.0)) * 9 / 7.0; //�Ƕȵ�����0-90��
    aays[n_sample-1] = aay;                        //�˴�Ӧ��ʵ�鷨ȡ�ú��ʵ�ϵ��
    aay_sum += aay * n_sample;                     //����ϵ��Ϊ9/7
    aay = (aay_sum / (11*n_sample/2.0)) * 9 / 7.0;
    aazs[n_sample-1] = aaz; 
    aaz_sum += aaz * n_sample;
    aaz = (aaz_sum / (11*n_sample/2.0)) * 9 / 7.0;
 
    float gyrox = - (gx-gxo) / GyroRatio * dt; //x����ٶ�
    float gyroy = - (gy-gyo) / GyroRatio * dt; //y����ٶ�
    float gyroz = - (gz-gzo) / GyroRatio * dt; //z����ٶ�
    agx += gyrox;                             //x����ٶȻ���
    agy += gyroy;                             //x����ٶȻ���
    agz += gyroz;
    
    /* kalman start */
    Sx = 0; Rx = 0;
    Sy = 0; Ry = 0;
    Sz = 0; Rz = 0;
    
    for(int i=1;i<10;i++)
    {                 //����ֵƽ��ֵ����
        a_x[i-1] = a_x[i];                      //�����ٶ�ƽ��ֵ
        Sx += a_x[i];
        a_y[i-1] = a_y[i];
        Sy += a_y[i];
        a_z[i-1] = a_z[i];
        Sz += a_z[i];
    
    }
    
    a_x[9] = aax;
    Sx += aax;
    Sx /= 10;                                 //x����ٶ�ƽ��ֵ
    a_y[9] = aay;
    Sy += aay;
    Sy /= 10;                                 //y����ٶ�ƽ��ֵ
    a_z[9] = aaz;
    Sz += aaz;
    Sz /= 10;
 
    for(int i=0;i<10;i++)
    {
        Rx += sqrt(a_x[i] - Sx);
        Ry += sqrt(a_y[i] - Sy);
        Rz += sqrt(a_z[i] - Sz);
    
    }
    
    Rx = Rx / 9;                              //�õ�����
    Ry = Ry / 9;                        
    Rz = Rz / 9;
  
    Px = Px + 0.0025;                         // 0.0025��������˵��...
    Kx = Px / (Px + Rx);                      //���㿨��������
    agx = agx + Kx * (aax - agx);             //�����ǽǶ�����ٶȼ��ٶȵ���
    Px = (1 - Kx) * Px;                       //����pֵ
 
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
#define ACC_G  16384.0                 //���ٶȼƱ���ϵ��
//float GyroRatio = 131.0;                    //�����Ǳ���ϵ��
#define GCC_G  (16.4*57.3)


double one_filter_angle[3] = {0.0};
float  kalman_filter_angle[3] = {0};
float kalman_filter_angle_dot[3] = {0};

//----------------
//�ڽ����˲�֮ǰ����Ҫ��ȡԭʼ���� 
float one_filter(float angle_m,float gyro_m,EULER_A euler)
{

    float  K1 =0.1; // �Լ��ٶȼ�ȡֵ��Ȩ��
    float  dt=0.005;//ע�⣺dt��ȡֵΪ�˲�������ʱ��

    one_filter_angle[euler] = K1 * angle_m+ (1-K1) * (one_filter_angle[euler] + gyro_m * dt);
    return one_filter_angle[euler];
}


//------------------
float kalman_filter(float angle_m,float gyro_m,EULER_A euler )
{

    //�˲�����
    float  dt = 0.005;   //����������ʱ��
    float  P[2][2]    = {{1,0},{0,1}};
    float  Pdot[4]    = {0,0,0,0};
    float  Q_angle = 0.001;//�Ƕ��������Ŷ�,������Э����
    float  Q_gyro = 0.005;     //���ٶ��������Ŷȣ�������Ʈ������Э����
    float  R_angle = 0.5;    //���ٶȼ�Э����
    char     C_0 = 1;
    float  q_bias = 0,angle_err = 0; //q_biasΪ������Ʈ��
    float  PCt_0 = 0,PCt_1 = 0,E = 0;
    float  K_0 = 0,  K_1 = 0,  t_0 = 0,  t_1 = 0;


     kalman_filter_angle[euler] += (gyro_m - q_bias) * dt;    //������Ԥ�ⷽ�̣���Ϊÿ��Ʈ����ͬ��


    Pdot[0]=Q_angle - P[0][1] - P[1][0];
    Pdot[1]=- P[1][1];
    Pdot[2]=- P[1][1];
    Pdot[3]=Q_gyro;

    P[0][0] += Pdot[0] * dt;
    P[0][1] += Pdot[1] * dt;
    P[1][0] += Pdot[2] * dt;
    P[1][1] += Pdot[3] * dt;

    PCt_0 = C_0 * P[0][0];     //����˷��м����
    PCt_1 = C_0 * P[1][0];

    E = R_angle + C_0 * PCt_0;     //��ĸ

    K_0 = PCt_0 / E;   //����ֵ
    K_1 = PCt_1 / E;

    angle_err = angle_m - kalman_filter_angle[euler];    
    kalman_filter_angle[euler] += K_0 * angle_err; //��״̬�Ŀ��������ƣ����ŽǶ�
    q_bias += K_1 * angle_err;
    kalman_filter_angle_dot[euler] = gyro_m-q_bias;//���Ž��ٶ�

    t_0 = PCt_0;     //��������м����
    t_1 = C_0 * P[0][1];

    P[0][0] -= K_0 * t_0;
    P[0][1] -= K_0 * t_1;
    P[1][0] -= K_1 * t_0;
    P[1][1] -= K_1 * t_1;

    return kalman_filter_angle[euler];
}

#define Kp 2.0f                        // ��������֧�������������ٶȼ�/��ǿ��
#define Ki 0.001f                // ��������֧���ʵ�������ƫ�����ν�
#define halfT 0.001f                // �������ڵ�һ��

float q0 = 1, q1 = 0, q2 = 0, q3 = 0;          // ��Ԫ����Ԫ�أ�������Ʒ���
float exInt = 0, eyInt = 0, ezInt = 0;        // ��������С�������

float Yaw,Pitch,Roll;  //ƫ���ǣ������ǣ�������


void IMUupdate(float gx, float gy, float gz, float ax, float ay, float az)
{
        float norm;
        float vx, vy, vz;
        float ex, ey, ez;  

        // ����������
        norm = sqrt(ax*ax + ay*ay + az*az);      
        ax = ax / norm;                   //��λ��
        ay = ay / norm;
        az = az / norm;      

        // ���Ʒ��������
        vx = 2*(q1*q3 - q0*q2);
        vy = 2*(q0*q1 + q2*q3);
        vz = q0*q0 - q1*q1 - q2*q2 + q3*q3;

        // ���������ͷ��򴫸��������ο�����֮��Ľ���˻����ܺ�
        ex = (ay*vz - az*vy);
        ey = (az*vx - ax*vz);
        ez = (ax*vy - ay*vx);

        // ������������������
        exInt = exInt + ex*Ki;
        eyInt = eyInt + ey*Ki;
        ezInt = ezInt + ez*Ki;

        // ������������ǲ���
        gx = gx + Kp*ex + exInt;
        gy = gy + Kp*ey + eyInt;
        gz = gz + Kp*ez + ezInt;

        // ������Ԫ���ʺ�������
        q0 = q0 + (-q1*gx - q2*gy - q3*gz)*halfT;
        q1 = q1 + (q0*gx + q2*gz - q3*gy)*halfT;
        q2 = q2 + (q0*gy - q1*gz + q3*gx)*halfT;
        q3 = q3 + (q0*gz + q1*gy - q2*gx)*halfT;  

        // ��������Ԫ
        norm = sqrt(q0*q0 + q1*q1 + q2*q2 + q3*q3);
        q0 = q0 / norm;
        q1 = q1 / norm;
        q2 = q2 / norm;
        q3 = q3 / norm;

        g_tMPU6050.pitch  = asin(-2 * q1 * q3 + 2 * q0* q2)* 57.3; // pitch ,ת��Ϊ����
        g_tMPU6050.roll = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2* q2 + 1)* 57.3; // rollv
        g_tMPU6050.yaw = atan2(2*(q1*q2 + q0*q3),q0*q0+q1*q1-q2*q2-q3*q3) * 57.3;               
}



float  acc_angle_x = 0,gyro_angle_x = 0;    //һ�׻����Ĳ�������
float  acc_angle_y = 0,gyro_angle_y = 0; 
float  acc_angle_z = 0,gyro_angle_z = 0; 

void kalman_filter_deal(void)
{
	static uint32_t last_time = 0;
	
	float  init_ax = 0,init_ay = 0,init_az = 0;
	
	
	MPU6050_ReadData();
	init_ax =  (float)g_tMPU6050.Accel_X / ACC_G;		 //���㵥λΪG�ĸ����������ٶȷ���
	init_ay =  (float)g_tMPU6050.Accel_Y / ACC_G;
	init_az =  (float)g_tMPU6050.Accel_Z / ACC_G;

	if(last_time == 0)
	{
		last_time = g_iRunTime;
	}
//	aax = atan(accy / accz) * (-180) / pi;	  //y�����z��ļн�
//	  aay = atan(accx / accz) * 180 / pi;		//x�����z��ļн�
//	  aaz = atan(accz / accy) * 180 / pi;		//z�����y��ļн�

	acc_angle_x = atan(init_ax/init_az) * 180 / PI ;    //���ٶȼ�x��Ƕ�-x�������z��
	acc_angle_y = atan(init_ay/init_az) * 180 / PI;     //y�������z��
	acc_angle_z = atan(init_az/init_ay) * 180 / PI;
	
	gyro_angle_x = -(float)(g_tMPU6050.GYRO_Y-gyo)/ GCC_G;	 //�����Ǽ���x��Ƕ�??
	gyro_angle_y = -(float)(g_tMPU6050.GYRO_X-gxo)/ GCC_G;
	gyro_angle_z = -(float)(g_tMPU6050.GYRO_Z-gzo)/ GCC_G;

//	g_tMPU6050.pitch = one_filter(acc_angle_x,gyro_angle_x,PITCH);		 //һ�׻����˲�����ȡƫx��Ƕ�
//	g_tMPU6050.roll = one_filter(acc_angle_y,gyro_angle_y,ROLL);
//	g_tMPU6050.yaw = one_filter(acc_angle_z,gyro_angle_z,YAW);
	
	g_tMPU6050.pitch = kalman_filter(acc_angle_x,gyro_angle_x,PITCH); 	//��ȡ�������˲�
	g_tMPU6050.roll = kalman_filter(acc_angle_y,gyro_angle_y,ROLL); 	
	g_tMPU6050.yaw = kalman_filter(acc_angle_z,gyro_angle_z,YAW); 	


//	gyro_angle_x += ((float)(g_tMPU6050.GYRO_X-gxo)/ GCC_G)*(g_iRunTime - last_time)/1000;
//	gyro_angle_y += ((float)(g_tMPU6050.GYRO_Y-gyo)/ GCC_G)*(g_iRunTime - last_time)/1000;
//	gyro_angle_z += ((float)(g_tMPU6050.GYRO_Z-gzo)/ GCC_G)*(g_iRunTime - last_time)/1000;
//
//
//	IMUupdate(gyro_angle_x, gyro_angle_y, gyro_angle_z, acc_angle_x, acc_angle_y, acc_angle_z);

}





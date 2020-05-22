/*
*********************************************************************************************************
*
*	ģ������ : ���ݴ���оƬGT811��������
*	�ļ����� : bsp_gt811.c
*	��    �� : V1.0
*	˵    �� : GT811����оƬ��������
*	�޸ļ�¼ :
*		�汾��   ����        ����     ˵��
*		V1.0    2017-12-02  armfly   ��ʽ����
*
*	Copyright (C), 2017-2025, ���������� www.armfly.com
*********************************************************************************************************
*/
#include "bsp.h"
#include "GUI.h"


#if 0
	#define printf_gt811dbg printf
#else
	#define printf_gt811dbg(...)
#endif

#define GT811_READ_XY_REG    0x814E /* ����Ĵ��� */ 

#define GT811_CLEARBUF_REG   0x814E /* �������Ĵ��� */ 

#define GT811_CONFIG_REG     0x8047 /* ���ò����Ĵ��� */ 

#define GT811_COMMAND_REG    0x8040 /* ʵʱ���� */ 

#define GT811_PRODUCT_ID_REG 0x8140 /* оƬID */ 

#define GT811_VENDOR_ID_REG  0x814A /* ��ǰģ��ѡ����Ϣ */ 

#define GT811_CONFIG_VERSION_REG   0x8047 /* �����ļ��汾�� */ 

#define GT811_CONFIG_CHECKSUM_REG  0x80FF /* �����ļ�У���� */ 

#define GT811_FIRMWARE_VERSION_REG 0x8144 /* �̼��汾�� */ 

/* GT811�����������ò�����һ����д�� */ 
const uint8_t s_GT811_CfgParams[]= 
{ 

#if 1	/* 1024 * 600 */
	0x00,0x00,0x04,0x58,0x02,0x0A,0x0D,0x00,
	0x01,0x08,0x28,0x05,0x50,0x32,0x03,0x05,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x8C,0x2E,0x0E,0x27,0x24,
	0xD0,0x07,0x00,0x00,0x01,0x99,0x04,0x1D,
	0x00,0x00,0x00,0x00,0x00,0x03,0x64,0x32,
	0x00,0x00,0x00,0x0F,0x23,0x94,0xC5,0x02,
	0x07,0x00,0x00,0x04,0xA2,0x10,0x00,0x8C,
	0x13,0x00,0x7C,0x16,0x00,0x68,0x1B,0x00,
	0x5C,0x20,0x00,0x5C,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x01,0x25,0x14,0x04,0x14,
	0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x02,0x04,0x06,0x08,0x0A,0x0C,0x0E,0x10,
	0x12,0x14,0x16,0x18,0x1A,0x1C,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,
	0x04,0x06,0x08,0x0A,0x0C,0x0F,0x10,0x12,
	0x13,0x14,0x16,0x18,0x1C,0x1D,0x1E,0x1F,
	0x20,0x21,0x22,0x24,0x26,0x28,0x29,0x2A,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x43,0x01	
#else

	0x00, //0x8047 �汾�� 

	0xE0,0x01, //0x8048/8049 X����������ֵ480 

	0x56,0x03, //0x804a/804b Y����������ֵ854 

	0x01, //0x804c �������������� 

	0x35, //0x804d �������룬�½��ش��� 

	0x00, //0x804e reserved 

	0x02, //0x804f ��ָ����ȥ�������� 

	0x08, //0x8050 ԭʼ���괰���˲�ֵ 

	0x28, //0x8051 ������������ 

	0x0A, //0x8052 ��������ֵ 

	0x5A, //0x8053 ���ϴ�������޵��е���ֵ 

	0x46, //0x8054 ���ϴ�������е��޵���ֵ 

	0x03, //0x8055 ���͹���ʱ�� s 

	0x05, //0x8056 �����ϱ��� 

	0x00, //0x8057 X������������� 

	0x00, //0x8058 Y������������� 

	0x00,0X00, //0x8059-0x805a reserved 

	0x00, //0x805b reserved 

	0x00, //0x805c reserved 

	0x00, //0x805d ���߹�����Сfilter���� 

	0x18, //0x805e �������� 1 ϵ�� 

	0x1A, //0x805f �������� 2 ϵ�� 

	0x1E, //0x8060 �������� 3 ϵ�� 

	0x14, //0x8061 ������������� 

	0x8C, //0x8062 ���� 

	0x28, //0x8063 ���� 

	0x0C, //0x8064 ���� 

	0x71, //0x8065 ������A������Ƶ�ʱ�Ƶϵ�� 

	0x73, //0x8066 ������B������Ƶ�ʱ�Ƶϵ�� 

	0xB2, //0x8067 ������A��B�Ļ�Ƶ 

	0x04, //0x8068 

	0x00, //0x8069 �������������ź����ʱ���� 

	0x00, //0x806a 

	0x00, //0x806b ���� 

	0x02, //0x806c ���� 

	0x03, //0x806d ԭʼֵ�Ŵ�ϵ�� 

	0x1D, //0x806e ���� 

	0x00, //0x806f reserved 

	0x01, //0x8070 ���� 

	0x00,0x00, //reserved 

	0x00, //0x8073 ���� 

	0x00,0x00,0x00,0x00,0x00,0x00, //0x8071 - 0x8079 reserved 

	0x50, //0x807a ��Ƶ��Χ�����Ƶ�� 

	0xA0, //0x807b ��Ƶ��Χ���յ�Ƶ�� 

	0x94, //0x807c �����������ȷ����������1-63��Ч 

	0xD5, //0x807d ������ⳬʱʱ�� 

	0x02, //0x807e ���� 

	0x07, //0x807f �б��и��ŵ����� 

	0x00,0x00, //0x8081 reserved 

	0x04, //0x8082 ��Ƶ�������Ƶ��1���ĵ��Ƶ������������A��B�� 

	0xA4, //0x8083 

	0x55, //0x8084 ��Ƶ�������Ƶ��1���ĵ㱶Ƶϵ�� 

	0x00, //0x8085 ��Ƶ�������Ƶ��2���ĵ��Ƶ(����A��B�ڴ˻����ϻ���) 

	0x91, //0x8086 

	0x62, //0x8087 ��Ƶ�������Ƶ��2���ĵ㱶Ƶϵ�� 

	0x00, //0x8088 ��Ƶ�������Ƶ��3���ĵ��Ƶ������������A��B�� 

	0x80, //0x8089 

	0x71, //0x808a ��Ƶ�������Ƶ��3���ĵ㱶Ƶϵ�� 

	0x00, //0x808b ��Ƶ�������Ƶ��4���ĵ��Ƶ������������A��B�� 

	0x71, //0x808c 

	0x82, //0x808d ��Ƶ�������Ƶ��4���ĵ㱶Ƶϵ�� 

	0x00, //0x808e ��Ƶ�������Ƶ��5���ĵ��Ƶ������������A��B�� 

	0x65, //0x808f 

	0x95, //0x8090 ��Ƶ�������Ƶ��5���ĵ㱶Ƶϵ�� 

	0x00, 0x65, //reserved 

	0x00, //0x8093 key1λ�� 0���ް��� 

	0x00, //0x8094 key2λ�� 0���ް��� 

	0x00, //0x8095 key3λ�� 0���ް��� 

	0x00, //0x8096 key4λ�� 0���ް��� 

	0x00, //0x8097 reserved 

	0x00, //0x8098 reserved 

	0x00, //0x8099 reserved 

	0x00, //0x809a reserved 

	0x00, //0x809b reserved 

	0x00, //0x809c reserved 

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //0x809d-0x80b2 reserved 

	0x00, //0x80b3 �Ͽ���� 

	0x00, //0x80b4 

	0x00,0x00, //0x80b6 reserved 

	0x06, //0x80b7 

	0x08, //0x80b8 

	0x0A, //0x80b9 

	0x0C, //0x80ba 

	0x0E, //0x80bb 

	0x10, //0x80bc 

	0x12, //0x80bd 

	0x14, //0x80be 

	0x16, //0x80bf 
+	
	0x18, //0x80c0 

	0x1A, //0x80c1 

	0x1C, //0x80c2 

	0xFF, //0x80c3 

	0xFF, //0x80c4 

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 

	0x00,0x00,0x00 



	,0x00, //0x80c5-0x80d4 reserved 

	0x00, //0x80d5 

	0x02, //0x80d6 

	0x04, //0x80d7 

	0x06, //0x80d8 

	0x08, //0x80d9 

	0x0A, //0x80da 

	0x0C, //0x80db 

	0x0F, //0x80dc 

	0x10, //0x80dd 

	0x12, //0x80de 

	0x13, //0x80df 

	0x14, //0x80e0 

	0x16, //0x80e1 

	0x18, //0x80e2 

	0x1C, //0x80e3 

	0x1D, //0x80e4 

	0x1E, //0x80e5 

	0x1F, //0x80e6 

	0x20, //0x80e7 

	0x21, //0x80e8 

	0xFF, //0x80e9 

	0xFF, //0x80ea 

	0xFF, //0x80eb 

	0xFF, //0x80ec 

	0xFF, //0x80ed 

	0xFF, //0x80ee 

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 

	0x00,0x00,0x00,0x00, //0x80ef-0x80fe reserved 

	0x0B, //0x80ff ������ϢУ�� 

	0x01 //0x8100 �����Ը��±�� 
#endif
}; 

GT811_T g_GT811;

static void GT811_WriteReg(uint16_t _usRegAddr, uint8_t *_pRegBuf, uint8_t _ucLen);
static uint8_t GT811_ReadReg(uint16_t _usRegAddr, uint8_t *_pRegBuf, uint8_t _ucLen);


/*
*********************************************************************************************************
*	�� �� ��: GT811_InitHard
*	����˵��: ���ô���оƬ.  �ڵ��øú���ǰ������ִ�� bsp_touch.c �еĺ��� bsp_DetectLcdType() ʶ��id
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void GT811_InitHard(void)
{
#if 0
	uint16_t ver;
	ver = GT811_ReadVersion();
	
	printf("GT811 ID :%08X, Version : %04X\r\n", id, ver);
#endif

	g_GT811.TimerCount = 0;
	
	bsp_DelayMS(50);
	
	/* I2C���߳�ʼ���� bsp.c ��ִ�� */
	
	/*
		���贫�����ò���
		GT811_WriteReg(GT811_CONFIG_REG, (uint8_t *)s_GT811_CfgParams, sizeof(s_GT811_CfgParams));
	*/
	
	g_GT811.TimerCount = 0;
	g_GT811.Enable = 1;
}

/*
*********************************************************************************************************
*	�� �� ��: GT811_ReadVersion
*	����˵��: ���GT811��оƬ�汾
*	��    ��: ��
*	�� �� ֵ: 16λ�汾
*********************************************************************************************************
*/
uint16_t GT811_ReadVersion(void)
{
	uint8_t buf[2]; 

	GT811_ReadReg(GT811_FIRMWARE_VERSION_REG, buf, 2); 

	return ((uint16_t)buf[1] << 8) + buf[0]; 
}

/*
*********************************************************************************************************
*	�� �� ��: GT811_WriteReg
*	����˵��: д1���������Ķ���Ĵ���
*	��    ��: _usRegAddr : �Ĵ�����ַ
*			  _pRegBuf : �Ĵ������ݻ�����
*			 _ucLen : ���ݳ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void GT811_WriteReg(uint16_t _usRegAddr, uint8_t *_pRegBuf, uint8_t _ucLen)
{
	uint8_t i;

    i2c_Start();					/* ���߿�ʼ�ź� */

    i2c_SendByte(g_GT811.i2c_addr);	/* �����豸��ַ+д�ź� */
	i2c_WaitAck();

    i2c_SendByte(_usRegAddr >> 8);	/* ��ַ��8λ */
	i2c_WaitAck();

    i2c_SendByte(_usRegAddr);		/* ��ַ��8λ */
	i2c_WaitAck();

	for (i = 0; i < _ucLen; i++)
	{
	    i2c_SendByte(_pRegBuf[i]);  /* �Ĵ������� */
		i2c_WaitAck();
	}

    i2c_Stop();                   	/* ����ֹͣ�ź� */
}

/*
*********************************************************************************************************
*	�� �� ��: GT811_ReadReg
*	����˵��: ��1���������Ķ���Ĵ���
*	��    ��: _usRegAddr : �Ĵ�����ַ
*			  _pRegBuf : �Ĵ������ݻ�����
*			 _ucLen : ���ݳ���
*	�� �� ֵ: 0 ��ʾʧ�ܣ�1��ʾ�ɹ�
*********************************************************************************************************
*/
static uint8_t GT811_ReadReg(uint16_t _usRegAddr, uint8_t *_pRegBuf, uint8_t _ucLen)
{
	int16_t i;

    i2c_Start();					/* ���߿�ʼ�ź� */

    i2c_SendByte(g_GT811.i2c_addr);	/* �����豸��ַ+д�ź� */
	if (i2c_WaitAck() != 0)
	{
		goto cmd_fail;	
	}

    i2c_SendByte(_usRegAddr >> 8);	/* ��ַ��8λ */
	if (i2c_WaitAck() != 0)
	{
		goto cmd_fail;	
	}

    i2c_SendByte(_usRegAddr);		/* ��ַ��8λ */
	if (i2c_WaitAck() != 0)
	{
		goto cmd_fail;	
	}

	i2c_Start();
    i2c_SendByte(g_GT811.i2c_addr + 0x01);	/* �����豸��ַ+���ź� */
	if (i2c_WaitAck() != 0)
	{
		goto cmd_fail;	
	}

	for (i = 0; i < _ucLen - 1; i++)
	{
	    _pRegBuf[i] = i2c_ReadByte();	/* ���Ĵ������� */
		i2c_Ack();
	}

	/* ���һ������ */
	 _pRegBuf[i] = i2c_ReadByte();		/* ���Ĵ������� */

	i2c_NAck();

    i2c_Stop();							/* ����ֹͣ�ź� */
	
	return 1;
	
cmd_fail: /* ����ִ��ʧ�ܺ��мǷ���ֹͣ�źţ�����Ӱ��I2C�����������豸 */
	/* ����I2C����ֹͣ�ź� */
	i2c_Stop();
	

	return 0;
}

/*
*********************************************************************************************************
*	�� �� ��: GT811_Timer1ms
*	����˵��: ÿ��1ms����1��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void GT811_Timer1ms(void)
{
	g_GT811.TimerCount++;
}

/*
*********************************************************************************************************
*	�� �� ��: GT811_OnePiontScan
*	����˵��: ��ȡGT811�������ݣ��������ȡһ�������㡣
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
#ifdef USE_EMWIN
extern GUI_PID_STATE State;
void GT811_OnePiontScan(void)
{
	uint8_t buf[10];
	static uint8_t s_tp_down = 0;
	uint16_t x, y;
	uint8_t clear_flag = 0;
	uint8_t res;
	static uint32_t count = 0;
	
	
	/* ��ȡ�Ĵ�����0x721  R  TouchpointFlag  Sensor_ID  key  tp4  tp3  tp2  tp1  tp0 */
	res = GT811_ReadReg(GT811_READ_XY_REG, buf, 1);
	
	if(res == 0)
	{
		printf_gt811dbg("1.failed\r\n");
		return;
	}	
	
	/* �ж��Ƿ��£�û�а��£�ֱ���˳� */
	if ((buf[0] & 0x0f) == 0)
	//if (buf[0] == 0)
	{
		GT811_WriteReg(GT811_READ_XY_REG, &clear_flag,	1);
		
		if (s_tp_down == 1)
		{
			count++;
			/* State.x��State.y����ֵ������£�State��ȫ�ֱ���������ľ������һ�ε���ֵ */
			/* �����⵽2�����ϲſ������� */
			if(count > 2)
			{   
				s_tp_down = 0;
				count = 0;
				State.Pressed = 0;
				GUI_PID_StoreState(&State);
			}
		}
		return;
	}
	
	/* ��ȡ��һ��������0 */
	res = GT811_ReadReg(GT811_READ_XY_REG + 1, &buf[1], 5);
	GT811_WriteReg(GT811_READ_XY_REG, &clear_flag,	1);	
	
	if(res == 0)
	{
		printf_gt811dbg("2.failed\r\n");
		return;
	}
	
	/*
	0x721  R  TouchpointFlag  Sensor_ID  key  tp4  tp3  tp2  tp1  tp0
	0x722  R  Touchkeystate     0  0  0  0  key4  key3  key2  key1

	0x723  R  Point0Xh  ������ 0��X ����� 8 λ
	0x724  R  Point0Xl  ������ 0��X ����� 8 λ
	0x725  R  Point0Yh  ������ 0��Y ����� 8 λ
	0x726  R  Point0Yl  ������ 0��Y ����� 8 λ
	0x727  R  Point0Pressure  ������ 0������ѹ��
	*/
	g_GT811.TouchpointFlag = buf[0];
	g_GT811.Touchkeystate = buf[1];

	g_GT811.X0 = ((uint16_t)buf[3] << 8) + buf[2];
	g_GT811.Y0 = ((uint16_t)buf[5] << 8) + buf[4];
	
#if 0  /* ���������ʱ�ò��� ���Ҫʹ�ã���Ҫ���ȡ�����ֽ����� */
	g_GT811.P0 = ((uint16_t)buf[7] << 8) + buf[6];
	
	if(g_GT811.P0 == 0)
	{
		printf_gt811dbg("%5d,%5d,%3d\r\n",  g_GT811.X0, g_GT811.Y0, g_GT811.P0);
		return;
	}
#endif
	
	/* ��ⰴ�� */
	/* ����ת�� :
		���ݴ��������½��� (0��0);  ���Ͻ��� (479��799)
		��Ҫת��LCD���������� (���Ͻ��� (0��0), ���½��� (799��479)
	*/
	x = g_GT811.X0;
	y = g_GT811.Y0;
	
	if (x > 799)
	{
		x = 799;
	}
	
	if (y > 479)
	{
		y = 479;
	}
	
	
	if (s_tp_down == 0)
	{
		s_tp_down = 1;
		State.x = x;
		State.y = y;
		State.Pressed = 1;
		GUI_PID_StoreState(&State);
	}
	else
	{
		State.x = x;
		State.y = y;
		State.Pressed = 1;
		GUI_PID_StoreState(&State);
	}
	
	/* ʵʱ���� */
	count = 0;

#if 0
	printf("%5d,%5d,%5d\r\n",  g_GT811.X0, g_GT811.Y0, g_GT811.P0);
#endif	
}
#endif

/*
*********************************************************************************************************
*	�� �� ��: GT811_Scan
*	����˵��: ��ȡGT811�������ݡ���ȡȫ�������ݣ���Ҫ 720us���ҡ����� bsp_Idle()��ִ��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void GT811_Scan(void)
{
	uint8_t buf[48];
	//uint8_t i;
	static uint8_t s_tp_down = 0;
	uint16_t x, y;
	static uint16_t x_save, y_save;
	uint8_t clear_flag = 0;

	if (g_GT811.Enable == 0)
	{
		return;
	}
#ifndef USE_FreeRTOS
	/* 20ms ִ��һ�� */
	if (g_GT811.TimerCount < 20)
	{
		return;
	}
#endif

	g_GT811.TimerCount = 0;

	GT811_ReadReg(GT811_READ_XY_REG, buf, 1);
	//if ((buf[0] & 0x0F) == 0)
	if (buf[0] == 0)
	{
		//GT811_WriteReg(GT811_READ_XY_REG, &clear_flag,	1);		/* ������������д0��� */
		
		if (s_tp_down == 1)
		{
			s_tp_down = 0;
			TOUCH_PutKey(TOUCH_RELEASE, x_save, y_save);
		}
		return;
	}
					
	GT811_ReadReg(GT811_READ_XY_REG + 1, &buf[1], 39);
	GT811_WriteReg(GT811_READ_XY_REG, &clear_flag,	1);		/* ������������д0��� */
	
	/*
		0x814E R/W Bufferstatus Large_Detect number of touch points 
			Bit7: Buffer status��1��ʾ���꣨�򰴼����Ѿ�׼���ã����ؿ��Զ�ȡ��0��ʾδ������������Ч�������ض�ȡ������󣬱���ͨ��I2C���˱�־���������ֽڣ�дΪ0��
			Bit4: HaveKey, 1��ʾ�а�����0��ʾ�ް������Ѿ��ɼ�����
			Bit3~0: Number of touch points, ���ϵ���������
	
		0x814F R Point1 track id 
		0x8150 R Point1Xl ������ 1��X ����� 8 λ 
		0x8151 R Point1Xh ������ 1��X ����� 8 λ 
		0x8152 R Point1Yl ������ 1��Y ����� 8 λ 
		0x8153 R Point1Yh ������ 1��Y ����� 8 λ 
		0x8154 R Point1 ������ 1����������� 8 λ 
		0x8155 R Point1 ������ 1����������� 8 λ 
		0x8156 ----
	
		0x8157 R Point2 track id 
		0x8158 R Point2Xl ������ 2��X ����� 8 λ 
		0x8159 R Point2Xh ������ 2��X ����� 8 λ 
		0x815A R Point2Yl ������ 2��Y ����� 8 λ 
		0x815B R Point2Yh ������ 2��Y ����� 8 λ 
		0x815C R Point2 ������ 2����������� 8 λ 
		0x815D R Point2 ������ 2����������� 8 λ 
		0x815E ----

		0x815F R Point3 track id 
		0x8160 R Point3Xl ������ 3��X ����� 8 λ 
		0x8161 R Point3Xh ������ 3��X ����� 8 λ 
		0x8162 R Point3Yl ������ 3��Y ����� 8 λ 
		0x8163 R Point3Yh ������ 3��Y ����� 8 λ 
		0x8164 R Point3 ������ 3����������� 8 λ 
		0x8165 R Point3 ������ 3����������� 8 λ 
		0x8166 ----

		0x8167 R Point4 track id 
		0x8168 R Point4Xl ������ 4��X ����� 8 λ 
		0x8169 R Point4Xh ������ 4��X ����� 8 λ 
		0x816A R Point4Yl ������ 4��Y ����� 8 λ 
		0x816B R Point4Yh ������ 4��Y ����� 8 λ 
		0x816C R Point4 ������ 4����������� 8 λ 
		0x816D R Point4 ������ 4����������� 8 λ 
		0x816E ----

		0x816F R Point5 track id 
		0x8170 R Point5Xl ������ 5��X ����� 8 λ 
		0x8171 R Point5Xh ������ 5��X ����� 8 λ 
		0x8172 R Point5Yl ������ 5��Y ����� 8 λ 
		0x8173 R Point5Yh ������ 5��Y ����� 8 λ 
		0x8174 R Point5 ������ 5����������� 8 λ 
		0x8175 R Point5 ������ 5����������� 8 λ 
		0x8176 --
		
	*/
	g_GT811.TouchpointFlag = buf[0];
	g_GT811.Touchkeystate = buf[1];

	g_GT811.X0 = ((uint16_t)buf[3] << 8) + buf[2];
	g_GT811.Y0 = ((uint16_t)buf[5] << 8) + buf[4];
	g_GT811.P0 = ((uint16_t)buf[7] << 8) + buf[6];

	g_GT811.X1 = ((uint16_t)buf[9] << 8) + buf[10];
	g_GT811.Y1 = ((uint16_t)buf[11] << 8) + buf[12];
	g_GT811.P1 = ((uint16_t)buf[13] << 8) + buf[14];

	g_GT811.X2 = ((uint16_t)buf[17] << 8) + buf[16];
	g_GT811.Y2 = ((uint16_t)buf[19] << 8) + buf[18];
	g_GT811.P2 = ((uint16_t)buf[21] << 8) + buf[20];

	g_GT811.X3 = ((uint16_t)buf[24] << 8) + buf[23];
	g_GT811.Y3 = ((uint16_t)buf[26] << 8) + buf[25];
	g_GT811.P3 = ((uint16_t)buf[28] << 8) + buf[27];

	g_GT811.X4 = ((uint16_t)buf[31] << 8) + buf[30];
	g_GT811.Y4 = ((uint16_t)buf[33] << 8) + buf[32];
	g_GT811.P4 = ((uint16_t)buf[35] << 8) + buf[34];

	/* ��ⰴ�� */
	{
		/* ����ת�� :
			���ݴ��������½��� (0��0);  ���Ͻ��� (479��799)
			��Ҫת��LCD���������� (���Ͻ��� (0��0), ���½��� (799��479)
		*/

		/* �����ж�ֵ�� */
//		if (g_GT811.i2c_addr == GT811_I2C_ADDR1)
		{
			x = g_GT811.X0;
			y = g_GT811.Y0;
			
			if (x > 799)
			{
				x = 799;
			}
			
			if (y > 479)
			{
				y = 479;
			}
		}
//		else
//		{
//			x = g_GT811.Y0;
//			y = 599 - g_GT811.X0;
//		
//			if (x > 1024)
//			{
//				x = 1024;
//			}
//			
//			if (y > 600)
//			{
//				y = 600;
//			}
//		}
	}
	
	if (s_tp_down == 0)
	{
		s_tp_down = 1;
		
		TOUCH_PutKey(TOUCH_DOWN, x, y);
	}
	else
	{
		TOUCH_PutKey(TOUCH_MOVE, x, y);
	}
	x_save = x;	/* �������꣬�����ͷ��¼� */
	y_save = y;

#if 0
	{
		uint8_t i;
		
		for (i = 0; i < 34; i++)
		{
			printf("%02X ", buf[i]);
		}
		printf("\r\n");

		printf("(%5d,%5d,%3d) ",  g_GT811.X0, g_GT811.Y0, g_GT811.P0);
		printf("(%5d,%5d,%3d) ",  g_GT811.X1, g_GT811.Y1, g_GT811.P1);
		printf("(%5d,%5d,%3d) ",  g_GT811.X2, g_GT811.Y2, g_GT811.P2);
		printf("(%5d,%5d,%3d) ",  g_GT811.X3, g_GT811.Y3, g_GT811.P3);
		printf("(%5d,%5d,%3d) ",  x, y, g_GT811.P4);
		printf("\r\n");
	}
#endif	
}

/*
*********************************************************************************************************
*	�� �� ��: GT811_ReadSensorID
*	����˵��: ʶ����ʾģ����𡣶�ȡGT811 SensorID����״̬����3��״̬�����գ��ӵ�Դ���ӵء�
*	��    ��: ��
*	�� �� ֵ: ��ʾģ�����, 0, 1, 2
*********************************************************************************************************
*/
uint8_t GT811_ReadSensorID(void)
{
	uint8_t value;
	
	/* 	0x721  R  TouchpointFlag      Sensor_ID  key  tp4  tp3  tp2  tp1  tp0 */
	GT811_ReadReg(0x721, &value, 1);
	
	return (value >> 6);
}


/***************************** ���������� www.armfly.com (END OF FILE) *********************************/

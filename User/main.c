
#include "includes.h"
#include "bsp.h"
#include "MainTask.h"


//��������
static void vTaskGUI(void *pvParameters);
static void vTaskTaskUserIF(void *pvParameters);
static void vTaskLED(void *pvParameters);
static void vTaskMsgPro(void *pvParameters);
static void vTaskStart(void *pvParameters);
static void AppTaskCreate(void);
static void AppObjCreate(void);
static void App_Printf(char *format,...);


//��������
static TaskHandle_t xHandleTaskUserIF = NULL;
static TaskHandle_t xHandleTaskLED = NULL;
static TaskHandle_t xHandleTaskMsgPro = NULL;
static TaskHandle_t xHandleTaskStart = NULL;
static SemaphoreHandle_t xMutex = NULL;


int main(void)
{
	__set_PRIMASK(1);

	bsp_Init();

	vSetupSysInfoTest();

	//��������
	AppTaskCreate();

	//��������ͨ�Ż���
	AppObjCreate();

	//�������ȣ���ʼִ������
	vTaskStartScheduler();

	while(1);
//	MainTask();
}


#if 1
static void vTaskGUI(void *pvParameters)
{
#if 0
	FONT_T tFont12;			/* ����һ������ṹ���������������������� */
	FONT_T tFont16;			/* ����һ������ṹ���������������������� */
	uint8_t buf[100], count = 0;

	/* ����������� */
	{
		tFont12.FontCode = FC_ST_12;	    /* ������� 12���� */
		tFont12.FrontColor = CL_WHITE;		/* ������ɫ */
		tFont12.BackColor = CL_BLUE;	    /* ���ֱ�����ɫ */
		tFont12.Space = 0;					/* ���ּ�࣬��λ = ���� */
	}
	
	/* ����������� */
	{
		tFont16.FontCode = FC_ST_16;	    /* ������� 16���� */
		tFont16.FrontColor = CL_WHITE;		/* ������ɫ */
		tFont16.BackColor = CL_BLUE;	    /* ���ֱ�����ɫ */
		tFont16.Space = 0;					/* ���ּ�࣬��λ = ���� */
	}

	/* �ӳ�200ms�ٵ������⣬����˲����� */
	bsp_DelayMS(200); 
	
	/* ���� */
	LCD_ClrScr(CL_BLUE);

	/* ��ʾ���� */
	LCD_DispStr(5, 3, "�������ǻƺ�¥���̻����������ݡ�", &tFont12); 
	LCD_DispStr(5, 18, "�·�ԶӰ�̿վ���Ψ�������������", &tFont12); 
	LCD_DispStr(5, 38, "�������ǻƺ�¥���̻����������ݡ�", &tFont16); 
	LCD_DispStr(5, 58, "�·�ԶӰ�̿վ���Ψ�������������", &tFont16); 
	
	/* ����2Dͼ�� */
	LCD_DrawLine(5, 120, 100, 220, CL_RED);
	LCD_DrawRect(120, 120, 100, 100, CL_RED);
	LCD_DrawCircle(280, 170, 50, CL_RED);
	LCD_Fill_Rect (340, 120, 100, 100, CL_BUTTON_GREY);
	
	/* ����������ʾ��Ϻ��ٴ򿪱��⣬����Ϊȱʡ���� */
	bsp_DelayMS(100); 
	LCD_SetBackLight(BRIGHT_DEFAULT);	
	while (1)
	{
		/* �ж������ʱ��0�Ƿ�ʱ */
//		if(bsp_CheckTimer(0))
		{
			/* ÿ��200ms ����һ�� */  
			bsp_LedToggle(2);
			
			sprintf((char *)buf, "count = %03d", count++);
			LCD_DispStr(5, 90, (char *)buf, &tFont16); 
			vTaskDelay(200);
		}
	}
#endif


#if 1
	snake_game_hmi_show();
	snake_game_move();

//	while (1)
//	{
//		/* �ж������ʱ��0�Ƿ�ʱ */
////		if(bsp_CheckTimer(0))
//		{
//			/* ÿ��200ms ����һ�� */  
//			bsp_LedToggle(2);					
//			vTaskDelay(200);
//		}
//	}
#endif
	
#if 0
	while(1)
	{
		MainTask();

	}
#endif

}

static void vTaskTaskUserIF(void *pvParameters)
{
	uint8_t ucKeyCode = 0;
	uint8_t pcWriteBuffer[500] = {0};

	while(1)
	{
		ucKeyCode = bsp_GetKey();

		if(ucKeyCode != KEY_NONE)
		{
			switch(ucKeyCode)
			{
				case KEY_DOWN_K1:
					App_Printf("=================================================\r\n");
					App_Printf("������      ����״̬ ���ȼ�   ʣ��ջ �������\r\n");
					vTaskList((char *)&pcWriteBuffer);
					App_Printf("%s\r\n",pcWriteBuffer);

					App_Printf("\r\n������       ���м���         ʹ����\r\n");
					vTaskGetRunTimeStats((char *)&pcWriteBuffer);
					App_Printf("%s\r\n",pcWriteBuffer);
					printf("��ǰ��̬�ڴ�ʣ���С = %d�ֽ�\r\n",xPortGetFreeHeapSize());
					break;

				default:
					break;
					
			}
		}
		vTaskDelay(20);
	}
}


static void vTaskLED(void *pvParameters)
{
	TickType_t xLastWakeTime = 0;
	const TickType_t xFrequency = 200;

	//��ȡ��ǰ��ϵͳʱ��
	xLastWakeTime = xTaskGetTickCount();

	while(1)
	{
		vTaskDelayUntil(&xLastWakeTime,xFrequency);
	}
}

static void vTaskStart(void *pvParameters)
{
	uint8_t ucCount1 = 0;

	while(1)
	{
		if(g_GT811.Enable == 1)
		{
			bsp_KeyScan10ms();
			ucCount1++;
			if(ucCount1 == 2)
			{
				ucCount1 = 0;
				GT811_Scan();
			}
			vTaskDelay(10);
		}

	}
}

static void AppTaskCreate(void)
{
	xTaskCreate(vTaskGUI,
				"vTaskGUI",
				1024,
				NULL,
				4,
				NULL);

	xTaskCreate(vTaskTaskUserIF,
				"vTaskUserIF",
				512,
				NULL,
				2,
				&xHandleTaskUserIF);

//	xTaskCreate(vTaskLED,
//				"vTaskLED",
//				512,
//				NULL,
//				3,
//				&xHandleTaskLED);

	xTaskCreate(vTaskStart,
				"vTaskStart",
				512,
				NULL,
				3,
				&xHandleTaskStart);
}

static void AppObjCreate(void)
{
	xMutex = xSemaphoreCreateMutex();

	if(NULL == xMutex)
	{
		
	}
}

static void App_Printf(char *format,...)
{
	char buf_str[200 + 1];
	va_list v_args;

	 va_start(v_args, format);
   (void)vsnprintf((char       *)&buf_str[0],
                   (size_t      ) sizeof(buf_str),
                   (char const *) format,
                                  v_args);
    va_end(v_args);

	xSemaphoreTake(xMutex,portMAX_DELAY);

	printf("%s",buf_str);

	xSemaphoreGive(xMutex);
	
}
#endif
#if 0   //������򣬲��������ǵ�ŷ���Ǽ���ʾ��
#include "bsp.h"			/* �ײ�Ӳ������ */


/*
*********************************************************************************************************
*	�� �� ��: main
*	����˵��: c�������
*	��    ��: ��
*	�� �� ֵ: �������(���账��)
*********************************************************************************************************
*/
extern  uint32_t giv_sys_time;
extern void kaerma_updata(void);
extern void setup_init(void);
extern void kalman_filter_deal(void);

int main(void)
{
	bsp_Init();
	LCD429_test();
#if 0
	uint32_t ret = 0;
	static uint32_t test_time = 0;
	bsp_Init();		/* Ӳ����ʼ�� */
	setup_init();	
	bsp_StartAutoTimer(0, 5);	/* ����1���Զ���ʱ��������300ms */
	bsp_StartAutoTimer(1, 200);	/* ����1���Զ���ʱ��������300ms */

	ret = bsp_TestExtSDRAM1();
	
	while(1)
	{
		if (bsp_CheckTimer(0))		/* ��鶨ʱ���ڵ��� */
		{
			//kaerma_updata();
			kalman_filter_deal();
		}
		if(bsp_CheckTimer(1))
		{
			printf("%f  %f  %f %d\r\n",g_tMPU6050.pitch,g_tMPU6050.roll,g_tMPU6050.yaw,ret);
		}
	}
//	DemoMPU6050();	/* ��ʾ���� */
//	while(1)
//	{
//		MPU6050_get_oula_angle();
//		if(giv_sys_time - test_time > 1000)
//		{
//			test_time = giv_sys_time;
//			printf("oula angle %f %f %f \r\n",g_tMPU6050.roll, g_tMPU6050.pitch, g_tMPU6050.yaw);
//		}
//	}

#endif
#if 0
	PrintfLogo();	/* ��ӡ�������ƺͰ汾����Ϣ */
	PrintfHelp();	/* ��ӡ������ʾ */

	/* ������LED1��������ʾ */
	bsp_LedOn(1);
	bsp_DelayMS(100);
	bsp_LedOff(1);
	bsp_DelayMS(100);
	
	bsp_StartAutoTimer(0, 100); /* ����1��100ms���Զ���װ�Ķ�ʱ�� */
	bsp_StartAutoTimer(1, 500);	/* ����1��500ms���Զ���װ�Ķ�ʱ�� */
	
	/* ����������ѭ���� */
	while (1)
	{
		bsp_Idle();		/* ���������bsp.c�ļ����û������޸��������ʵ��CPU���ߺ�ι�� */

		/* �ж϶�ʱ����ʱʱ�� */
		if (bsp_CheckTimer(0))	
		{
			/* ÿ��100ms ����һ�� */  
			bsp_LedToggle(1);			
		}
		
		/* �ж϶�ʱ����ʱʱ�� */
		if (bsp_CheckTimer(1))	
		{
			/* ÿ��500ms ����һ�� */ 
			bsp_LedToggle(2);			
			bsp_LedToggle(3);			
			bsp_LedToggle(4);
		}
	}
#endif
}
#endif


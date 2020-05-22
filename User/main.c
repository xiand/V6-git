
#include "includes.h"
#include "bsp.h"
#include "MainTask.h"


//函数声明
static void vTaskGUI(void *pvParameters);
static void vTaskTaskUserIF(void *pvParameters);
static void vTaskLED(void *pvParameters);
static void vTaskMsgPro(void *pvParameters);
static void vTaskStart(void *pvParameters);
static void AppTaskCreate(void);
static void AppObjCreate(void);
static void App_Printf(char *format,...);


//变量声明
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

	//创建任务
	AppTaskCreate();

	//创建任务通信机制
	AppObjCreate();

	//启动调度，开始执行任务
	vTaskStartScheduler();

	while(1);
//	MainTask();
}


#if 1
static void vTaskGUI(void *pvParameters)
{
#if 0
	FONT_T tFont12;			/* 定义一个字体结构体变量，用于设置字体参数 */
	FONT_T tFont16;			/* 定义一个字体结构体变量，用于设置字体参数 */
	uint8_t buf[100], count = 0;

	/* 设置字体参数 */
	{
		tFont12.FontCode = FC_ST_12;	    /* 字体代码 12点阵 */
		tFont12.FrontColor = CL_WHITE;		/* 字体颜色 */
		tFont12.BackColor = CL_BLUE;	    /* 文字背景颜色 */
		tFont12.Space = 0;					/* 文字间距，单位 = 像素 */
	}
	
	/* 设置字体参数 */
	{
		tFont16.FontCode = FC_ST_16;	    /* 字体代码 16点阵 */
		tFont16.FrontColor = CL_WHITE;		/* 字体颜色 */
		tFont16.BackColor = CL_BLUE;	    /* 文字背景颜色 */
		tFont16.Space = 0;					/* 文字间距，单位 = 像素 */
	}

	/* 延迟200ms再点亮背光，避免瞬间高亮 */
	bsp_DelayMS(200); 
	
	/* 清屏 */
	LCD_ClrScr(CL_BLUE);

	/* 显示汉字 */
	LCD_DispStr(5, 3, "故人西辞黄鹤楼，烟花三月下扬州。", &tFont12); 
	LCD_DispStr(5, 18, "孤帆远影碧空尽，唯见长江天际流。", &tFont12); 
	LCD_DispStr(5, 38, "故人西辞黄鹤楼，烟花三月下扬州。", &tFont16); 
	LCD_DispStr(5, 58, "孤帆远影碧空尽，唯见长江天际流。", &tFont16); 
	
	/* 绘制2D图形 */
	LCD_DrawLine(5, 120, 100, 220, CL_RED);
	LCD_DrawRect(120, 120, 100, 100, CL_RED);
	LCD_DrawCircle(280, 170, 50, CL_RED);
	LCD_Fill_Rect (340, 120, 100, 100, CL_BUTTON_GREY);
	
	/* 界面整体显示完毕后，再打开背光，设置为缺省亮度 */
	bsp_DelayMS(100); 
	LCD_SetBackLight(BRIGHT_DEFAULT);	
	while (1)
	{
		/* 判断软件定时器0是否超时 */
//		if(bsp_CheckTimer(0))
		{
			/* 每隔200ms 进来一次 */  
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
//		/* 判断软件定时器0是否超时 */
////		if(bsp_CheckTimer(0))
//		{
//			/* 每隔200ms 进来一次 */  
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
					App_Printf("任务名      任务状态 优先级   剩余栈 任务序号\r\n");
					vTaskList((char *)&pcWriteBuffer);
					App_Printf("%s\r\n",pcWriteBuffer);

					App_Printf("\r\n任务名       运行计数         使用率\r\n");
					vTaskGetRunTimeStats((char *)&pcWriteBuffer);
					App_Printf("%s\r\n",pcWriteBuffer);
					printf("当前动态内存剩余大小 = %d字节\r\n",xPortGetFreeHeapSize());
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

	//获取当前的系统时间
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
#if 0   //裸机程序，测试陀螺仪的欧拉角及显示屏
#include "bsp.h"			/* 底层硬件驱动 */


/*
*********************************************************************************************************
*	函 数 名: main
*	功能说明: c程序入口
*	形    参: 无
*	返 回 值: 错误代码(无需处理)
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
	bsp_Init();		/* 硬件初始化 */
	setup_init();	
	bsp_StartAutoTimer(0, 5);	/* 启动1个自动定时器，周期300ms */
	bsp_StartAutoTimer(1, 200);	/* 启动1个自动定时器，周期300ms */

	ret = bsp_TestExtSDRAM1();
	
	while(1)
	{
		if (bsp_CheckTimer(0))		/* 检查定时周期到否 */
		{
			//kaerma_updata();
			kalman_filter_deal();
		}
		if(bsp_CheckTimer(1))
		{
			printf("%f  %f  %f %d\r\n",g_tMPU6050.pitch,g_tMPU6050.roll,g_tMPU6050.yaw,ret);
		}
	}
//	DemoMPU6050();	/* 演示例程 */
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
	PrintfLogo();	/* 打印例程名称和版本等信息 */
	PrintfHelp();	/* 打印操作提示 */

	/* 先做个LED1的亮灭显示 */
	bsp_LedOn(1);
	bsp_DelayMS(100);
	bsp_LedOff(1);
	bsp_DelayMS(100);
	
	bsp_StartAutoTimer(0, 100); /* 启动1个100ms的自动重装的定时器 */
	bsp_StartAutoTimer(1, 500);	/* 启动1个500ms的自动重装的定时器 */
	
	/* 进入主程序循环体 */
	while (1)
	{
		bsp_Idle();		/* 这个函数在bsp.c文件。用户可以修改这个函数实现CPU休眠和喂狗 */

		/* 判断定时器超时时间 */
		if (bsp_CheckTimer(0))	
		{
			/* 每隔100ms 进来一次 */  
			bsp_LedToggle(1);			
		}
		
		/* 判断定时器超时时间 */
		if (bsp_CheckTimer(1))	
		{
			/* 每隔500ms 进来一次 */ 
			bsp_LedToggle(2);			
			bsp_LedToggle(3);			
			bsp_LedToggle(4);
		}
	}
#endif
}
#endif


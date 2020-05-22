#include "bsp.h"
#include "key_image.h"

static void snake_control_info(uint8_t *dir);


void snake_game_hmi_show(void)
{
	/* 清屏 */
	LCD_ClrScr(CL_BLACK);

	//显示四个按键
	FONT_T my_font_up = {
		.FontCode = FC_ST_12,
		.FrontColor = CL_WHITE,
		.BackColor = CL_BLUE,
		.Space = 0,
	};
	ICON_T my_icon_up = {
		.id = 0,
		.Left = 30,
		.Top = 30,
		.Height = 80,
		.Width = 80,
		.pBmp = (uint16_t *)Up,
		.Text = "up",
	};
	LCD_DrawIcon(&my_icon_up, &my_font_up, 0);


	FONT_T my_font_down = {
		.FontCode = FC_ST_12,
		.FrontColor = CL_WHITE,
		.BackColor = CL_BLUE,
		.Space = 0,
	};
	ICON_T my_icon_down = {
		.id = 0,
		.Left = 30,
		.Top = 140,
		.Height = 80,
		.Width = 80,
		.pBmp = (uint16_t *)Down,
		.Text = "down",
	};
	LCD_DrawIcon(&my_icon_down, &my_font_down, 0);

	FONT_T my_font_left = {
		.FontCode = FC_ST_12,
		.FrontColor = CL_WHITE,
		.BackColor = CL_BLUE,
		.Space = 0,
	};
	ICON_T my_icon_left = {
		.id = 0,
		.Left = 30,
		.Top = 250,
		.Height = 80,
		.Width = 80,
		.pBmp = (uint16_t *)Left,
		.Text = "left",
	};
	LCD_DrawIcon(&my_icon_left, &my_font_left, 0);

	FONT_T my_font_right = {
		.FontCode = FC_ST_12,
		.FrontColor = CL_WHITE,
		.BackColor = CL_BLUE,
		.Space = 0,
	};
	ICON_T my_icon_right = {
		.id = 0,
		.Left = 30,
		.Top = 360,
		.Height = 80,
		.Width = 80,
		.pBmp = (uint16_t *)Right,
		.Text = "right",
	};
	LCD_DrawIcon(&my_icon_right, &my_font_right, 0);

	

	//显示一个游戏框，snake只能在这个框里面运行
	
	//LCD429_DrawRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor)

	LCD429_DrawRect(140,30,420,630,CL_RED);

	/* 界面整体显示完毕后，再打开背光，设置为缺省亮度 */
	bsp_DelayMS(100); 
	LCD_SetBackLight(BRIGHT_DEFAULT);	
}


uint16_t snake_body_x[40* 60];
uint16_t snake_body_y[40* 60];

uint8_t snake_run_dir = 0;
uint16_t snake_length = 5;

void snake_game_move(void)
{
	
 	uint16_t i = 0;
	uint8_t food_flag = 0;
	uint16_t food_x = 0;
	uint16_t food_y = 0;
	
	uint8_t game_over_flag = 0;
	TickType_t xLastWakeTime = 0;
	const TickType_t xFrequency = 100;

	//获取当前的系统时间
	xLastWakeTime = xTaskGetTickCount();

	FONT_T tFont16;			/* 定义一个字体结构体变量，用于设置字体参数 */

	/* 设置字体参数 */
	{
		tFont16.FontCode = FC_ST_16;	    /* 字体代码 16点阵 */
		tFont16.FrontColor = CL_WHITE;		/* 字体颜色 */
		tFont16.BackColor = CL_BLUE;	    /* 文字背景颜色 */
		tFont16.Space = 0;					/* 文字间距，单位 = 像素 */
	}
	
	for(i = 0; i < snake_length; i++)
	{
		snake_body_y[i] = 240;
		snake_body_x[i] = 400 - 10*i;
	}

	while(game_over_flag == 0)
	{

		switch(snake_run_dir)
		{
			case 1:  //向上1格
			{
				snake_body_y[0] = snake_body_y[0] - 10;
				break;
			}
			case 2: //向下一格
			{
				snake_body_y[0] = snake_body_y[0] + 10;
				break;
			}
			case 3: //向左一格
			{
				snake_body_x[0] = snake_body_x[0] - 10;
				break;
			}
			case 4://向右一格
			{
				snake_body_x[0] = snake_body_x[0] + 10;
				break;
			}
			default:
				snake_run_dir = 0;
		}

		if(snake_run_dir != 0)
		{
				for(i = snake_length; i > 1; i--)
				{
					snake_body_x[i - 1] = snake_body_x[i - 2];
					snake_body_y[i - 1] = snake_body_y[i - 2];
				}
		}
	
		
		snake_control_info(&snake_run_dir);

		if(food_flag == 0)  //需要生成food
		{
			srand(xTaskGetTickCount());

			food_x = (rand()% 590) + 160;

			srand(xTaskGetTickCount() + snake_length);
			food_y = (rand()% 380) + 50;

			food_flag = 1;

			for (i=1; i<snake_length; i++)
			{
				if (
					 ((snake_body_x[i] >= food_x) && (snake_body_x[i] <= (food_x+10)) && (snake_body_y[i] >= food_y) && (snake_body_y[i] <= (food_y+10)))   			           //判断第i节蛇身小方块的左上角是否与食物交叉
				 ||(((snake_body_x[i]+10) >= food_x) && ((snake_body_x[i]+10) <= (food_x+10)) && (snake_body_y[i] >= food_y) && (snake_body_y[i] <= (food_y+10)))          //判断第i节蛇身小方块的右上角是否与食物交叉
				 ||((snake_body_x[i] >= food_x) && (snake_body_x[i] <= (food_x+10)) && ((snake_body_y[i]+10) >= food_y) && ((snake_body_y[i]+10) <= (food_y+10)))          //判断第i节蛇身小方块的左下角是否与食物交叉
				 ||(((snake_body_x[i]+10) >= food_x) && ((snake_body_x[i]+10) <= (food_x+10)) && ((snake_body_y[i]+10) >= food_y) && ((snake_body_y[i]+10) <= (food_y+10)))//判断第i节蛇身小方块的右下角是否与食物交叉
					 )
				{
					food_flag = 0;
				}
			}
		}

		/*
			判断蛇是否吃了食物
			如果蛇头小方块的四个角中，
			任意一个角的X、Y坐标同时进入了食物小方块的坐标范围内，则说明蛇吃了食物
		*/
		if (
			 ((snake_body_x[0] >= food_x) && (snake_body_x[0] <= (food_x+10)) && (snake_body_y[0] >= food_y) && (snake_body_y[0] <= (food_y+10)))   			           //蛇头左上角
		 ||(((snake_body_x[0]+10) >= food_x) && ((snake_body_x[0]+10) <= (food_x+10)) && (snake_body_y[0] >= food_y) && (snake_body_y[0] <= (food_y+10)))          //蛇头右上角
		 ||((snake_body_x[0] >= food_x) && (snake_body_x[0] <= (food_x+10)) && ((snake_body_y[0]+10) >= food_y) && ((snake_body_y[0]+10) <= (food_y+10)))          //蛇头左下角
		 ||(((snake_body_x[0]+10) >= food_x) && ((snake_body_x[0]+10) <= (food_x+10)) && ((snake_body_y[0]+10) >= food_y) && ((snake_body_y[0]+10) <= (food_y+10)))//蛇头右下角
			 )
		{
			snake_length +=1;   //吃食物后蛇长度+1
			
			/*最后一节(增加的一节)坐标暂时设为跟倒数第二节一样(在"蛇身前进一格"的地方会自动和倒数第二节分开)*/
			snake_body_x[snake_length-1] = (snake_body_x[snake_length-2]);
			snake_body_y[snake_length-1] = (snake_body_y[snake_length-2]);
			
			food_flag = 0;
		}
		
		
		
		/*如果蛇头部超出了活动范围(撞墙)，则游戏结束*/
		if ((snake_body_x[0] < 150) || (snake_body_x[0] > 770) || (snake_body_y[0] < 35) || (snake_body_y[0] > 450))
		{
			game_over_flag = 1;
		}
		
		/*
			把蛇头部的坐标和身体的坐标一一进行比对
			如果蛇头部的坐标和身体任意一个部位的坐标重合(撞到自身)，则游戏结束
		*/
//		for (i=1; i<snake_length; i++)
//		{
//			if ((snake_body_x[0] == snake_body_x[i]) && (snake_body_y[0] == snake_body_y[i]))
//			{
//				game_over_flag = 1;
//			}
//		}

		
		//显示snake到屏幕
		for(i = 0; i < snake_length; i++)
		{
			LCD429_FillRect(snake_body_x[i], snake_body_y[i], 10, 10, CL_BLUE);
		}
		//显示食物
		LCD429_FillRect(food_x,food_y, 10,10,CL_BLUE);
		
		bsp_LedToggle(2);					
		vTaskDelayUntil(&xLastWakeTime,xFrequency);
		//清除snake到屏幕
		
		for(i = 0; i < snake_length; i++)
		{
			LCD429_FillRect(snake_body_x[i], snake_body_y[i], 10, 10, CL_BLACK);
		}
		LCD429_FillRect(food_x,food_y, 10,10,CL_BLACK);


		while(game_over_flag == 1)
		{
			LCD_DispStr(400, 200, "按任意键继续。.....", &tFont16); 
			snake_control_info(&snake_run_dir);
			if(snake_run_dir != 0)
			{
				game_over_flag = 0;
				snake_run_dir = 0;
				snake_game_hmi_show();
				snake_length = 5;
				for(i = 0; i < snake_length; i++)
				{
					snake_body_y[i] = 240;
					snake_body_x[i] = 400 - 10*i;
				}
			}
			vTaskDelayUntil(&xLastWakeTime,xFrequency);
		}
	}
	
}


static void snake_control_info(uint8_t *dir)
{
	uint8_t ret = 0;
	int16_t px = 0;
	int16_t py = 0;
		
	ret = TOUCH_GetKey(&px,&py);

	if((px < 120) && (py > 30) && (py < 110))
	{
		*dir = 1;
	}
	else if((px < 120) && (py > 140) && (py < 220))
	{
		*dir = 2;
	}
	else if((px < 120) && (py > 250) && (py < 330))
	{
		*dir = 3;
	}
	else if((px < 120) && (py > 360) && (py < 420))
	{
		*dir = 4;
	}
	else
	{
		*dir = 0;
	}
	if(*dir != 0)
	{
		TOUCH_CelarFIFO();
	}

}



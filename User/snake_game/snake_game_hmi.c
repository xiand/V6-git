#include "bsp.h"
#include "key_image.h"

static void snake_control_info(uint8_t *dir);


void snake_game_hmi_show(void)
{
	/* ���� */
	LCD_ClrScr(CL_BLACK);

	//��ʾ�ĸ�����
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

	

	//��ʾһ����Ϸ��snakeֻ�����������������
	
	//LCD429_DrawRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor)

	LCD429_DrawRect(140,30,420,630,CL_RED);

	/* ����������ʾ��Ϻ��ٴ򿪱��⣬����Ϊȱʡ���� */
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

	//��ȡ��ǰ��ϵͳʱ��
	xLastWakeTime = xTaskGetTickCount();

	FONT_T tFont16;			/* ����һ������ṹ���������������������� */

	/* ����������� */
	{
		tFont16.FontCode = FC_ST_16;	    /* ������� 16���� */
		tFont16.FrontColor = CL_WHITE;		/* ������ɫ */
		tFont16.BackColor = CL_BLUE;	    /* ���ֱ�����ɫ */
		tFont16.Space = 0;					/* ���ּ�࣬��λ = ���� */
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
			case 1:  //����1��
			{
				snake_body_y[0] = snake_body_y[0] - 10;
				break;
			}
			case 2: //����һ��
			{
				snake_body_y[0] = snake_body_y[0] + 10;
				break;
			}
			case 3: //����һ��
			{
				snake_body_x[0] = snake_body_x[0] - 10;
				break;
			}
			case 4://����һ��
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

		if(food_flag == 0)  //��Ҫ����food
		{
			srand(xTaskGetTickCount());

			food_x = (rand()% 590) + 160;

			srand(xTaskGetTickCount() + snake_length);
			food_y = (rand()% 380) + 50;

			food_flag = 1;

			for (i=1; i<snake_length; i++)
			{
				if (
					 ((snake_body_x[i] >= food_x) && (snake_body_x[i] <= (food_x+10)) && (snake_body_y[i] >= food_y) && (snake_body_y[i] <= (food_y+10)))   			           //�жϵ�i������С��������Ͻ��Ƿ���ʳ�ｻ��
				 ||(((snake_body_x[i]+10) >= food_x) && ((snake_body_x[i]+10) <= (food_x+10)) && (snake_body_y[i] >= food_y) && (snake_body_y[i] <= (food_y+10)))          //�жϵ�i������С��������Ͻ��Ƿ���ʳ�ｻ��
				 ||((snake_body_x[i] >= food_x) && (snake_body_x[i] <= (food_x+10)) && ((snake_body_y[i]+10) >= food_y) && ((snake_body_y[i]+10) <= (food_y+10)))          //�жϵ�i������С��������½��Ƿ���ʳ�ｻ��
				 ||(((snake_body_x[i]+10) >= food_x) && ((snake_body_x[i]+10) <= (food_x+10)) && ((snake_body_y[i]+10) >= food_y) && ((snake_body_y[i]+10) <= (food_y+10)))//�жϵ�i������С��������½��Ƿ���ʳ�ｻ��
					 )
				{
					food_flag = 0;
				}
			}
		}

		/*
			�ж����Ƿ����ʳ��
			�����ͷС������ĸ����У�
			����һ���ǵ�X��Y����ͬʱ������ʳ��С��������귶Χ�ڣ���˵���߳���ʳ��
		*/
		if (
			 ((snake_body_x[0] >= food_x) && (snake_body_x[0] <= (food_x+10)) && (snake_body_y[0] >= food_y) && (snake_body_y[0] <= (food_y+10)))   			           //��ͷ���Ͻ�
		 ||(((snake_body_x[0]+10) >= food_x) && ((snake_body_x[0]+10) <= (food_x+10)) && (snake_body_y[0] >= food_y) && (snake_body_y[0] <= (food_y+10)))          //��ͷ���Ͻ�
		 ||((snake_body_x[0] >= food_x) && (snake_body_x[0] <= (food_x+10)) && ((snake_body_y[0]+10) >= food_y) && ((snake_body_y[0]+10) <= (food_y+10)))          //��ͷ���½�
		 ||(((snake_body_x[0]+10) >= food_x) && ((snake_body_x[0]+10) <= (food_x+10)) && ((snake_body_y[0]+10) >= food_y) && ((snake_body_y[0]+10) <= (food_y+10)))//��ͷ���½�
			 )
		{
			snake_length +=1;   //��ʳ����߳���+1
			
			/*���һ��(���ӵ�һ��)������ʱ��Ϊ�������ڶ���һ��(��"����ǰ��һ��"�ĵط����Զ��͵����ڶ��ڷֿ�)*/
			snake_body_x[snake_length-1] = (snake_body_x[snake_length-2]);
			snake_body_y[snake_length-1] = (snake_body_y[snake_length-2]);
			
			food_flag = 0;
		}
		
		
		
		/*�����ͷ�������˻��Χ(ײǽ)������Ϸ����*/
		if ((snake_body_x[0] < 150) || (snake_body_x[0] > 770) || (snake_body_y[0] < 35) || (snake_body_y[0] > 450))
		{
			game_over_flag = 1;
		}
		
		/*
			����ͷ������������������һһ���бȶ�
			�����ͷ�����������������һ����λ�������غ�(ײ������)������Ϸ����
		*/
//		for (i=1; i<snake_length; i++)
//		{
//			if ((snake_body_x[0] == snake_body_x[i]) && (snake_body_y[0] == snake_body_y[i]))
//			{
//				game_over_flag = 1;
//			}
//		}

		
		//��ʾsnake����Ļ
		for(i = 0; i < snake_length; i++)
		{
			LCD429_FillRect(snake_body_x[i], snake_body_y[i], 10, 10, CL_BLUE);
		}
		//��ʾʳ��
		LCD429_FillRect(food_x,food_y, 10,10,CL_BLUE);
		
		bsp_LedToggle(2);					
		vTaskDelayUntil(&xLastWakeTime,xFrequency);
		//���snake����Ļ
		
		for(i = 0; i < snake_length; i++)
		{
			LCD429_FillRect(snake_body_x[i], snake_body_y[i], 10, 10, CL_BLACK);
		}
		LCD429_FillRect(food_x,food_y, 10,10,CL_BLACK);


		while(game_over_flag == 1)
		{
			LCD_DispStr(400, 200, "�������������.....", &tFont16); 
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



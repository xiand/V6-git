/*
*********************************************************************************************************
*	                                  
*	模块名称 : 洗衣机简易操作界面
*	文件名称 : MainTask.c
*	版    本 : V1.0
*	说    明 : 实验内容如下
*              1. 本实例有三个值得大家学习的地方:
*                 （1）自定义按钮的实现，主要是通过自定义按钮的回调函数实现按钮的不同显示效果。
*                 （2）自定义菜单的实现，显示效果更好。
*                 （3）内存设备相关函数GUI_MEMDEV_Create，GUI_MEMDEV_Select，GUI_MEMDEV_Write的使用。
*                      通过函数数GUI_MEMDEV_Create申请所需的内存设备，然后通过函数GUI_MEMDEV_Select
*                      选择要使用的函数，这样绘图操作实现的界面就可以直接写到内存设备中，需要显示的时候
*                      直接调用函数GUI_MEMDEV_Write就能进行显示。
*              2. 本设计界面支持按键操作：
*                 （1）按键K3实现ESCAPE功能，也就是退出显示界面，在本工程中主要是关闭子菜单。
*                 （2）摇杆的上下左右键实现菜单选项的选项和子菜单的显示，按下摇杆的下键就能显示子菜单
*                 （3）摇杆的OK可以触发洗衣机界面中的start启动按钮。
*              
*	修改记录 :
*		版本号   日期         作者          说明
*		V1.0    2016-11-26   Eric2013  	    首版    
*                                     
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "MainTask.h"
#include "includes.h"




/*
*********************************************************************************************************
*                                       引用外部定义
*********************************************************************************************************
*/ 
extern GUI_CONST_STORAGE GUI_FONT GUI_FontYahei;
extern GUI_CONST_STORAGE GUI_BITMAP bmLogo_armfly;
extern GUI_CONST_STORAGE GUI_FONT GUI_Font18B;
extern GUI_CONST_STORAGE GUI_FONT GUI_Font60BD;
extern GUI_CONST_STORAGE GUI_FONT GUI_Font18;
extern GUI_CONST_STORAGE GUI_FONT GUI_Font60min;
extern GUI_CONST_STORAGE GUI_BITMAP _bmSubTop;
extern GUI_CONST_STORAGE GUI_BITMAP _bmSubBottom;
extern GUI_CONST_STORAGE GUI_BITMAP _bmBlueCircle;  
extern GUI_CONST_STORAGE GUI_BITMAP _bmCheckMark;

/*
*********************************************************************************************************
*                                       类型定义
*********************************************************************************************************
*/ 
typedef struct {
  int           NumItems;
  const char ** pData;
} SUB_ITEMS;

typedef struct {
  int Delay;
  int Cmd;
  int Param;
} ACTION_ITEM;

typedef struct {
  int State;
} EXEC_MACHINE_CONTEXT;

/*
*********************************************************************************************************
*                                       宏定义
*********************************************************************************************************
*/ 
#define STATUS_CHECK_CHECKED 1
#define STATUS_CHECK_DRAWONE 2
#define STATUS_CHECK_DRAWTWO 3
#define MENU_Y0              41
#define MENU_Y1              75
#define SUB_X0               4
#define SUB_Y0               14
#define SUB_X1               65
#define SUB_YD               25
#define CMD_KEY              1
#define CMD_EXEC             2
#define EXEC_START           1
#define EXEC_WAIT            2
#define STATE_READY          1
#define STATE_DELAY          2
#define STATE_WASHING        3
#define INIT_BUTTON          (WM_USER + 0)
#define MAIN_OPEN_SUB        (WM_USER + 1)
#define MAIN_CLOSE_SUB       (WM_USER + 2)

/*
*********************************************************************************************************
*                                       文本
*********************************************************************************************************
*/ 
static const char * _pText[] = {
  "Pre", 
  "Wash", 
  "Rinse", 
  "Spin",
  "Soil" 
};

static const char * _pTextPre[] = {
  "Off",
  "On"
};

static const char * _pTextWash[] = {
  "Warm",
  "Cold"
};

static const char * _pTextRinse[] = {
  "Warm",
  "Cold"
};

static const char * _pTextSpin[] = {
  "High",
  "Low"
};

static const char * _pTextSoil[] = {
  "Heavy",
  "Normal",
  "Light"
};

/*
*********************************************************************************************************
*                             洗衣机的几个工作状态的显示位置
*********************************************************************************************************
*/ 
static int _aStatusSep[5] = {3, 30, 130, 184, 217};

/*
*********************************************************************************************************
*                                       静态变量
*********************************************************************************************************
*/ 
static const int _aSeparator[] = { 20, 55, 110, 170, 220, 280 };
static const int _axPos[]      = { 20, 58, 118, 180, 235      };
static const int _aSubPosX[]   = {  5, 55, 115, 165, 225      };
static SUB_ITEMS _SubItems;
static unsigned  _Selection    = 2;
static WM_HWIN   _hStatus;
static int       _aSelection[] = { 1, 0, 1, 1, 0 };
static int       _Time;
static int       _State;
static int       _NoMemory;

static WM_HWIN   _hButton;

/*
*********************************************************************************************************
*	函 数 名: _DrawGradientRoundBar
*	功能说明: 绘制填充了垂直梯度色的圆角图形。
*	形    参: xPos0     起始位置X
*             yPos0     起始位置Y
*             xPos1     结束位置X
*             yPos1     结束位置Y
*             Color0    梯度起始色
*             Color1    梯度结束色
*	返 回 值: 无
*********************************************************************************************************
*/
static void _DrawGradientRoundBar(int xPos0, int yPos0, int xPos1, int yPos1, GUI_COLOR Color0, GUI_COLOR Color1) 
{
	GUI_COLOR Color;
	unsigned  r;
	unsigned  g;
	U32       b;
	double    rd;
	double    rr;
	double    x;
	double    y;
	int       Add;
	int       r0;
	int       g0;
	int       b0;
	int       r1;
	int       g1;
	int       b1;
	int       d;
	int       i;

	r0  = (Color0 >>  0) & 0x000000ff;
	g0  = (Color0 >>  8) & 0x000000ff;
	b0  = (Color0 >> 16) & 0x000000ff;
	r1  = (Color1 >>  0) & 0x000000ff;
	g1  = (Color1 >>  8) & 0x000000ff;
	b1  = (Color1 >> 16) & 0x000000ff;
	Add = -1;
	d   = yPos1 - yPos0 + 1;
	rd  = (yPos1 - yPos0) / 2.0;
	rr  = rd * rd;
	y   = rd;
	
	for (i = yPos0; i <= yPos1; i++) 
	{
		x = sqrt(rr - y * y);
		r = r0 + (r1 - r0) * (i - yPos0) / d;
		g = g0 + (g1 - g0) * (i - yPos0) / d;
		b = b0 + (b1 - b0) * (i - yPos0) / d;
		Color = r | (g << 8) | (b << 16);
		GUI_SetColor(Color);
		
		GUI_DrawHLine(i, (int)(xPos0 + rd - x), (int)(xPos0 + rd));
		GUI_DrawHLine(i, (int)(xPos0 + rd),     (int)(xPos1 - rd));
		GUI_DrawHLine(i, (int)(xPos1 - rd),     (int)(xPos1 - rd + x));
		
		y += Add;
		if (y < 0) 
		{
			Add = -Add;
			y = -y;
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: _DrawCheckbox
*	功能说明: 绘制自定义checkbox
*	形    参: x       起始位置X
*             y       起始位置Y
*             Status  状态
*	返 回 值: 无
*********************************************************************************************************
*/
static void _DrawCheckbox(int x, int y, int Status) 
{
	GUI_COLOR ColorOld;
	U16       c;

	c = 0;
	ColorOld = GUI_GetColor();
	
	/* 绘制蓝色圆圈位图 */
	GUI_DrawBitmap(&_bmBlueCircle, x, y);
	
	switch (Status) 
	{
		/* 自定义checkbox支持以下三种自定义状态 */
		case STATUS_CHECK_CHECKED:
			GUI_DrawBitmap(&_bmCheckMark, x + 4, y - 2);
			break;
		
		case STATUS_CHECK_DRAWONE:
			c = '1';
		
		//lint -fallthrough
		case STATUS_CHECK_DRAWTWO:
			if (c == 0) 
			{
				c = '2';
			}
			GUI_SetColor(0xEBAD00);
			GUI_SetFont(&GUI_Font18B);
			GUI_SetTextMode(GUI_TM_TRANS);
			GUI_DispCharAt(c, x + 5, y + 1);
			break;
	}
	
	GUI_SetColor(ColorOld);
}

/*
*********************************************************************************************************
*	函 数 名: _OnKeySub
*	功能说明: 子菜单回调函数的重绘函数
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void _OnPaintSub(WM_HWIN hWin) 
{
	int ySize;
	int i;

	ySize = WM_GetWindowSizeY(hWin);
	
	/* 绘制菜单控件的顶端部分 */
	GUI_DrawBitmap(&_bmSubTop,    0, 0);
	
	/* 绘制菜单控件的底端部分 */
	GUI_DrawBitmap(&_bmSubBottom, 0, ySize - _bmSubBottom.YSize - 1);
	
	/* 绘制菜单控件的中间部分 */
	GUI_SetColor(0xff0000);
	GUI_FillRect(0, _bmSubTop.YSize, 69, ySize - _bmSubBottom.YSize);
	
	/* 绘制菜单上要显示的文本 */
	GUI_SetTextMode(GUI_TM_TRANS);
	GUI_SetFont(&GUI_Font18B);
	
	/* 遍历菜单的所有项进行显示 */
	for (i = 0; i < _SubItems.NumItems; i++) 
	{
		/* 选中的话白底蓝字 */
		if (_aSelection[_Selection] == i) 
		{
			GUI_SetColor(GUI_WHITE);
			GUI_FillRect(SUB_X0, SUB_Y0 + i * SUB_YD, SUB_X1, SUB_Y0 + i * SUB_YD + 19);
			GUI_SetColor(0xff0000);
		}
		
		/* 没有选中的话蓝底白字 */
		else 
		{
			GUI_SetColor(GUI_WHITE);
		}
		
		GUI_DispStringHCenterAt(_SubItems.pData[i], 35, SUB_Y0 + 1 + i * SUB_YD);
	}
}

/*
*********************************************************************************************************
*	函 数 名: _OnKeySub
*	功能说明: 子菜单的按键消息处理
*	形    参: hWin   指针句柄
*             Key    按键值
*	返 回 值: 无
*********************************************************************************************************
*/
static int _OnKeySub(WM_HWIN hWin, int Key) 
{
	WM_HWIN hParent;

	hParent = WM_GetParent(hWin);
	
	switch (Key) 
	{
		/* 按下向下的按键，调整选择的子菜单选项下移 */
		case GUI_KEY_DOWN:
			if (_aSelection[_Selection] < (_SubItems.NumItems - 1)) 
			{
				_aSelection[_Selection]++;
				WM_InvalidateWindow(hWin);
				WM_InvalidateWindow(hParent);
			}
			break;
		
		/* 按下向上的按键，调整选择的子菜单选项上移 */
		case GUI_KEY_UP:
			if (_aSelection[_Selection] > 0) 
			{
				_aSelection[_Selection]--;
				WM_InvalidateWindow(hWin);
				WM_InvalidateWindow(hParent);
			}
			break;
		
		/* 收到GUI_KEY_ESCAPE消息，重新聚焦到父窗口并关闭打开的子窗口 */
		case GUI_KEY_ESCAPE:
			WM_SetFocus(hParent);
			WM_SendMessageNoPara(hParent, MAIN_CLOSE_SUB);
			return 0;
		
		/* 收到下面的两种消息，重新聚焦到父窗口并关闭打开的子窗口 */
		case GUI_KEY_LEFT:
		case GUI_KEY_RIGHT:
			WM_SetFocus(hParent);
			WM_SendMessageNoPara(hParent, MAIN_CLOSE_SUB);
			return 1;
	}
	
	return 0;
}

/*
*********************************************************************************************************
*	函 数 名: _OnPIDStateChangeSub
*	功能说明: 子菜单的PID事件处理
*	形    参: hWin   指针句柄
*             pInfo  WM_PID_STATE_CHANGED_INFO类型指针
*	返 回 值: 无
*********************************************************************************************************
*/
static void _OnPIDStateChangeSub(WM_HWIN hWin, const WM_PID_STATE_CHANGED_INFO * pInfo) 
{
	WM_HWIN hParent;

	/* 如果不在点击的范围内，退出 */
	if ((pInfo->State == 0) || (pInfo->y < SUB_Y0) || (pInfo->y > (SUB_Y0 + SUB_YD * _SubItems.NumItems))) 
	{
		return;
	}
	if ((pInfo->x < SUB_X0) || (pInfo->x > SUB_X1)) 
	{
		return;
	}
	
	/* 记录当前选择的子菜单 */
	_aSelection[_Selection] = (pInfo->y - SUB_Y0) / SUB_YD;
	
	/* 获取父窗口 */
	hParent = WM_GetParent(hWin);
	WM_InvalidateWindow(hParent);
	
	/* 设置聚焦 */
	WM_SetFocus(hParent);
	
	/* 删除此子菜单 */
	WM_DeleteWindow(hWin);
}

/*
*********************************************************************************************************
*	函 数 名: _cbSubMenu
*	功能说明: 窗口hSub的回调函数
*	形    参: pMsg  指针地址
*	返 回 值: 无
*********************************************************************************************************
*/
static void _cbSubMenu(WM_MESSAGE * pMsg) 
{
  WM_HWIN hWin;

	hWin = pMsg->hWin;
	switch (pMsg->MsgId) 
	{
		/* PID状态发生变化消息处理 */
		case WM_PID_STATE_CHANGED:
			_OnPIDStateChangeSub(pMsg->hWin, (const WM_PID_STATE_CHANGED_INFO *)pMsg->Data.p);
			return;
		
		/* 按键消息处理 */
		case WM_KEY:
			if (((const WM_KEY_INFO*)(pMsg->Data.p))->PressedCnt > 0) 
			{
				WM_HWIN hParent;
				int Key = ((const WM_KEY_INFO*)(pMsg->Data.p))->Key;
				
				hParent = WM_GetParent(hWin);
				if (_OnKeySub(hWin, Key)) 
				{
					WM_SendMessage(hParent, pMsg);
				}
			}
			return;
		
		/* 子菜单的重绘函数 */
		case WM_PAINT:
			_OnPaintSub(pMsg->hWin);
			break;
	}
	
	WM_DefaultProc(pMsg);
}

/*
*********************************************************************************************************
*	函 数 名: _ToggleSubMenu
*	功能说明: 打开或者关闭垂直的子菜单
*             此函数被回调函数_cbMain中的消息MAIN_OPEN_SUB和MAIN_CLOSE_SUB所调用。
*	形    参: hParent  子菜单的父窗口
*             OnOff    1  打开子菜单
*                      0  关闭子菜单
*	返 回 值: 无
*********************************************************************************************************
*/
static void _ToggleSubMenu(WM_HWIN hParent, U8 OnOff) 
{
	static WM_HWIN hSub = 0;
	
	/* 如果创建了，将其删除 */
	if (hSub) 
	{
		WM_DeleteWindow(hSub);
		hSub = 0;
	}
	
	/* 如果是关闭子菜单，退出即可，因为前面已经将其删除了 */
	if (OnOff == 0) 
	{
		return;
	}
	
	/* 根据选择的_Selection，设置要显示的子菜单内容 */
	switch (_Selection) 
	{
		case 0:
			_SubItems.pData    = _pTextPre;
			_SubItems.NumItems = GUI_COUNTOF(_pTextPre);
			break;
		
		case 1:
			_SubItems.pData    = _pTextWash;
			_SubItems.NumItems = GUI_COUNTOF(_pTextWash);
			break;
		case 2:
			_SubItems.pData    = _pTextRinse;
			_SubItems.NumItems = GUI_COUNTOF(_pTextRinse);
			break;
		
		case 3:
			_SubItems.pData    = _pTextSpin;
			_SubItems.NumItems = GUI_COUNTOF(_pTextSpin);
			break;
		
		case 4:
			_SubItems.pData    = _pTextSoil;
			_SubItems.NumItems = GUI_COUNTOF(_pTextSoil);
			break;
	}
	
	/* 创建子菜单窗口 */
	hSub = WM_CreateWindowAsChild(_aSubPosX[_Selection], 80, 70, _SubItems.NumItems * 25 + 20, hParent, WM_CF_SHOW | WM_CF_HASTRANS, _cbSubMenu, 0);
	
	/* 设置聚焦 */
	WM_SetFocus(hSub);
}

/*
*********************************************************************************************************
*	函 数 名: _CreateBitmap
*	功能说明: 绘制填充了垂直梯度色的圆角矩形，带边框。         
*	形    参: hWin    窗口句柄
*             xPos0   起始X位置  
*			  yPos0   起始Y位置
*             xPos1   结束X位置
*             yPos1   结束Y位置
*	返 回 值: 无
*********************************************************************************************************
*/
static GUI_MEMDEV_Handle _CreateBitmap(WM_HWIN hWin, int xPos0, int yPos0, int xPos1, int yPos1) 
{
	GUI_MEMDEV_Handle hMem;
	GUI_MEMDEV_Handle hOld;
	int               xPos;
	int               yPos;
	int               r;

	r    = (yPos1 - yPos0 + 1) / 2;
	xPos = WM_GetWindowOrgX(hWin);                /* 分别返回指定窗口的原点在桌面坐标中的X或Y位置 */
	yPos = WM_GetWindowOrgY(hWin);
	
	/* 注意这里是相对于桌面窗口的位置 */
	hMem = GUI_MEMDEV_Create(xPos + xPos0,        /* 存储设备的X位置。*/
	                         yPos + yPos0,        /* 存储设备的Y位置。*/
	                         xPos1 - xPos0 + 1,   /* 存储设备的X尺寸。*/
	                         yPos1 - yPos0 + 1);  /* 存储设备的Y尺寸。*/
	
	if (hMem) 
	{
		WM_SelectWindow(hWin);
		hOld = GUI_MEMDEV_Select(hMem);
		GUI_DrawGradientV(5, 40, 314, 76, 0xffffff, 0xffebeb);
		_DrawGradientRoundBar(xPos0 + 2, yPos0 + 2, xPos1 - 2, yPos1 - 2, 0xEEEEEE, 0x737373);
		GUI_SetColor(0xff0000);
		GUI_SetPenSize(3);
		GUI_AA_DrawArc(xPos0 + r, yPos0 + r, r - 2, r - 2,  90, 270);
		GUI_AA_DrawArc(xPos1 - r, yPos0 + r, r - 2, r - 2, 270, 450);
		GUI_AA_DrawLine(xPos0 + r, yPos0 + 2, xPos1 - r, yPos0 + 2);
		GUI_AA_DrawLine(xPos0 + r, yPos1 - 2, xPos1 - r, yPos1 - 2);
		GUI_MEMDEV_Select(hOld);
	} 
	else 
	{
		_NoMemory = 1;
	}
	
	return hMem;
}

/*
*********************************************************************************************************
*	函 数 名: _OnKeyMain
*	功能说明: 窗口hWinMain回调函数的重绘函数  
*	形    参: hWin        窗口句柄
*             phBitmap    GUI_MEMDEV_Handle类型指针
*	返 回 值: 无
*********************************************************************************************************
*/
static void _OnPaintMain(WM_HWIN hWin, GUI_MEMDEV_Handle * phBitmap) 
{
	static int xSize;
	unsigned   i;
	
	/* 获取窗口长度 */
	if (xSize == 0) 
	{
		xSize = WM_GetWindowSizeX(hWin);
	}
	
	/* 第一步：绘制背景 ***************************************/
	GUI_MEMDEV_Write(*phBitmap);

	/* 第二步：绘制菜单 ***************************************/
	GUI_SetTextMode(GUI_TM_TRANS);
	for (i = 0; i < GUI_COUNTOF(_pText); i++) 
	{
		/* 第一个checkbox选项 */
		if (i == 0) 
		{
			if (_aSelection[0]) 
			{
				GUI_SetFont(&GUI_Font18B);
			} 
			else 
			{
				GUI_SetFont(&GUI_Font18);
			}
		} 
		/* 其余菜单 */
		else 
		{
			GUI_SetFont(&GUI_Font18);
		}
		
		if (_Selection == i) 
		{
			GUI_SetColor(GUI_BLACK);
		} 
		else 
		{
			GUI_SetColor(0x9D9FA1);
		}
		
		GUI_DispStringAt(_pText[i], _axPos[i], 10);
	}
	
	/* 第三步：绘制checkbox ***************************************/
	_DrawCheckbox(19, 48, _aSelection[0]);
	
	/* 第四步：绘制其余菜单选项 ***********************************/
	if (_Selection) 
	{
		GUI_SetColor(0xff0000);
		GUI_FillRect(_aSeparator[_Selection], MENU_Y0, _aSeparator[_Selection + 1], MENU_Y1);
	}
	
	GUI_SetColor(GUI_WHITE);
	GUI_SetFont(&GUI_Font18B);
	GUI_DispStringHCenterAt(_pTextWash [_aSelection[1]], (_aSeparator[1] + _aSeparator[2]) / 2, 48);
	GUI_DispStringHCenterAt(_pTextRinse[_aSelection[2]], (_aSeparator[2] + _aSeparator[3]) / 2, 48);
	GUI_DispStringHCenterAt(_pTextSpin [_aSelection[3]], (_aSeparator[3] + _aSeparator[4]) / 2, 48);
	GUI_DispStringHCenterAt(_pTextSoil [_aSelection[4]], (_aSeparator[4] + _aSeparator[5]) / 2, 48);
	
	/* 分别设置洗衣机运行中和非运行中的显示效果 */
	if (_State == STATE_WASHING) 
	{
		//
		// Show remaining time it is washing
		//
		GUI_SetColor(0xff0000);
		GUI_SetFont(&GUI_Font60BD);
		GUI_DispDecAt(_Time, 115, 140, 2);
		GUI_SetFont(&GUI_Font60min);
		GUI_DispString("min");
	} 
	else 
	{
		//
		// 如果洗衣机不工作的话，显示logo
		//
		GUI_DrawBitmap(&bmLogo_armfly, 90, 90);
	}
}

/*
*********************************************************************************************************
*	函 数 名: _OnKeyMain
*	功能说明: 水平菜单栏的按键消息处理
*	形    参：hWin   窗口句柄
*             Key    按下的按键值
*	返 回 值: 无
*********************************************************************************************************
*/
static void _OnKeyMain(WM_HWIN hWin, int Key) 
{
	/* 如果洗衣机在工作状态，对按键消息不做处理 */
	if (_State == STATE_WASHING) 
	{
		return;
	}
	
	switch (Key) 
	{
		/* 左键消息，调整_Selection，从而实现水平菜单选择项的调整 */
		case GUI_KEY_LEFT:
			if (_Selection) 
			{
				_Selection--;
				WM_InvalidateWindow(hWin);
			}
			break;
		
        /* 右键消息，调整_Selection，从而实现水平菜单选择项的调整 */			
		case GUI_KEY_RIGHT:
			if (_Selection < (GUI_COUNTOF(_pText) - 1)) 
			{
				_Selection++;
				WM_InvalidateWindow(hWin);
			}
			break;
		
		/* 打开子菜单 */
		case GUI_KEY_DOWN:
			WM_SendMessageNoPara(hWin, MAIN_OPEN_SUB);
			break;

		case GUI_KEY_ENTER:
			WM_SetFocus(_hButton);
			GUI_SendKeyMsg(GUI_KEY_ENTER, 1);
			break;
	}
}

/*
*********************************************************************************************************
*	函 数 名: _OnPIDStateChangeMain
*	功能说明: 水平菜单栏的PID状态改变后的事件处理
*	形    参: hWin   窗口句柄
*             pInfo  WM_PID_STATE_CHANGED_INFO类型指针
*	返 回 值: 无
*********************************************************************************************************
*/
static void _OnPIDStateChangeMain(WM_HWIN hWin, const WM_PID_STATE_CHANGED_INFO * pInfo) 
{
	unsigned i;
	int      x;

	/* 点击了statrt按钮后，水平菜单栏的PID状态改变将不起作用 */
	if (_State == STATE_WASHING) 
	{
		return;
	}
	
	/* 保证点击在了相应的范围内才起作用 */
	if ((pInfo->State == 0) || (pInfo->y < MENU_Y0) || (pInfo->y > MENU_Y1)) 
	{
		return;
	}
	
	if ((pInfo->x < _aSeparator[0]) || (pInfo->x > _aSeparator[5])) 
	{
		return;
	}
	
	/* 根据点击的具体选项，向窗口发送相应消息 */
	for (i = 0; i < GUI_COUNTOF(_aSeparator) - 1; i++) 
	{
		x = pInfo->x;
		if ((x >= _aSeparator[i]) && (x <= _aSeparator[i + 1])) 
		{
			/* 菜单的某个选项被点击后，发送消息 */
			WM_SendMessageNoPara(hWin, MAIN_CLOSE_SUB);
			_Selection = i;
			WM_InvalidateWindow(hWin);
			
			/* 其余选项被点击 */
			if (i) 
			{
				WM_SendMessageNoPara(hWin, MAIN_OPEN_SUB);
			} 
			/* 第一个选项被点击 */
			else 
			{
				_aSelection[_Selection] ^= 1;
			}
			
			return;
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: _cbStatus
*	功能说明: 窗_hStatus回调函数  
*	形    参：pMsg  参数指针
*	返 回 值: 无
*********************************************************************************************************
*/
static void _cbStatus(WM_MESSAGE * pMsg) 
{
	static GUI_MEMDEV_Handle   hBitmap;
	static int                 xSize;
	static int                 ySize;
	const GUI_PID_STATE      * pState;
	GUI_MEMDEV_Handle          hOld;
	WM_HWIN                    hWin;
	int                        NewTime;
	int                        xPos;
	int                        yPos;
	int                        i;

	hWin = pMsg->hWin;
	
	switch (pMsg->MsgId) 
	{
		/* 指针输入设备接触到处于按下状态的窗口轮廓时发送到窗口 */
		case WM_TOUCH:
			pState = (const GUI_PID_STATE *)pMsg->Data.p;
			if (pState) 
			{
				if (pState->Pressed) 
				{
					NewTime = 90 - ((pState->x - 3) * 90) / (xSize - 6);
					if (NewTime < 0) 
					{
						NewTime = 0;
					}
					if (NewTime < _Time) 
					{
						_Time = NewTime;
					}
				}
			}
			break;
		
        /* 重绘消息 */			
		case WM_PAINT:
			GUI_MEMDEV_Write(hBitmap);
			xPos = 3 + ((90 - _Time) * (xSize - 6)) / 90;
			GUI_SetAlpha(0xA0);
			GUI_SetColor(GUI_RED);
			GUI_FillRect(3, 3, xPos, ySize - 4);
			GUI_SetAlpha(0);
			break;
		
		/* 使用内存设备绘制好要显示的内存 */
		case WM_CREATE:
			WM_SelectWindow(hWin);
			xSize = WM_GetWindowSizeX(hWin);
			ySize = WM_GetWindowSizeY(hWin);
			xPos  = WM_GetWindowOrgX(hWin);
			yPos  = WM_GetWindowOrgY(hWin);
		
		    /*  GUI_MEMDEV_NOTRANS
				创建存储设备，无透明性。用户必须确保正确绘制背景。
				这样可将存储设备用于非矩形区域。另一优势是速度较高：使
				用此标记可加速存储设备约30 - 50%。
			*/
			hBitmap = GUI_MEMDEV_CreateEx(xPos, yPos, xSize, ySize, GUI_MEMDEV_NOTRANS);
			if (hBitmap) 
			{
				hOld = GUI_MEMDEV_Select(hBitmap);
				GUI_DrawGradientV(3, 3, xSize - 4, ySize - 4, 0xEEEEEE, 0x737373);
				GUI_SetColor(GUI_BLACK);
				for (i = 1; i < 4; i++) 
				{
					GUI_DrawVLine(_aStatusSep[i], 0, 24);
				}
				GUI_SetColor(0xff0000);
				GUI_DrawRect(0, 0, xSize - 1, ySize - 1);
				GUI_DrawRect(1, 1, xSize - 2, ySize - 2);
				GUI_DrawRect(2, 2, xSize - 3, ySize - 3);
				GUI_SetTextMode(GUI_TM_TRANS);
				GUI_SetFont(&GUI_Font18);
				GUI_SetColor(GUI_WHITE);
				for (i = 0; i < 4; i++) 
				{
					GUI_DispStringHCenterAt(_pText[i], (_aStatusSep[i + 1] + _aStatusSep[i]) / 2, 3);
				}
				GUI_MEMDEV_Select(hOld);
			} 
			else 
			{
				_NoMemory = 1;
			}
			break;
			
		/* 窗口被删除后，记得删除申请的内存设备 */
		case WM_DELETE:
			GUI_MEMDEV_Delete(hBitmap);
			hBitmap = 0;
			break;
		
		default:
			WM_DefaultProc(pMsg);
	}
}

/*
*********************************************************************************************************
*	函 数 名: _cbMain
*	功能说明: 窗口hWinMain的回调函数  
*	形    参: pMsg  参数指针
*	返 回 值: 无
*********************************************************************************************************
*/
static void _cbMain(WM_MESSAGE * pMsg) 
{
	static GUI_MEMDEV_Handle hBitmap;
	unsigned                 i;
	WM_HWIN                  hWin;
	int                      xSizeStatus;
	int                      NCode;
	int                      Key;
	int                      Pos;

	hWin = pMsg->hWin;
	
	switch (pMsg->MsgId) 
	{
		/* 创建窗口上要显示的内容 */
		case WM_CREATE:
			hBitmap = _CreateBitmap(hWin, 10, 40, WM_GetWindowSizeX(hWin) - 10, 76);
			break;
		
		/* 按下状态已更改时，发送到指针输入设备指向的窗口 */
		case WM_PID_STATE_CHANGED:
			_OnPIDStateChangeMain(pMsg->hWin, (const WM_PID_STATE_CHANGED_INFO *)pMsg->Data.p);
			break;
		
		/* 外部按键消息的处理 */
		case WM_KEY:
			if (((const WM_KEY_INFO *)(pMsg->Data.p))->PressedCnt > 0) 
			{
				Key = ((const WM_KEY_INFO *)(pMsg->Data.p))->Key;
				_OnKeyMain(pMsg->hWin, Key);
			}
			break;
		
		/* 重绘函数 */
		case WM_PAINT:
			_OnPaintMain(pMsg->hWin, &hBitmap);
			break;
		
		/* 窗口被删除后，同时要删除内存设备和窗口_hStatus */
		case WM_DELETE:
			GUI_MEMDEV_Delete(hBitmap);
			hBitmap = 0;
			if (_hStatus) 
			{
				WM_DeleteWindow(_hStatus);
				_hStatus = 0;
			}
			/* 触发关闭子菜单 */
			_ToggleSubMenu(hWin, 0);
			break;
			
		case WM_NOTIFY_PARENT:
			NCode = pMsg->Data.v;
			switch (NCode) 
			{
				/* 点击start按钮释放后 */
				case WM_NOTIFICATION_RELEASED:
					WM_InvalidateWindow(hWin);
				
					/* 再次点击后进入到这个函数*/
					if (_State == STATE_WASHING) 
					{
						xSizeStatus = WM_GetWindowSizeX(_hStatus) - 6;
						for (i = 0; i < GUI_COUNTOF(_aStatusSep); i++) 
						{
							Pos = 3 + ((90 - _Time) * xSizeStatus) / 90;
							if (_aStatusSep[i] > Pos) 
							{
								_Time = 90 - ((_aStatusSep[i] - 3) * 90) / xSizeStatus;
								break;
							}
						}
					}
					
					/* 第一次点击后是进入到这个函数里面，并设置_State = STATE_WASHING */
					else 
					{
						WM_SendMessageNoPara(hWin, MAIN_CLOSE_SUB);
						_State = STATE_WASHING;
						/* 创建窗口_hStatus */
						if (_hStatus == 0) 
						{
							_hStatus = WM_CreateWindowAsChild(80, 108, 220, 24, hWin, WM_CF_SHOW, _cbStatus, 0);
						}
					}
					break;
			}
			break;
		
        /* 打开菜单消息 */			
		case MAIN_OPEN_SUB:
			_ToggleSubMenu(hWin, 1);
			break;
		
		/* 关闭菜单消息 */		
		case MAIN_CLOSE_SUB:
			_ToggleSubMenu(hWin, 0);
			break;
		
		default:
			WM_DefaultProc(pMsg);
	}
}

/*
*********************************************************************************************************
*	函 数 名: _DrawBackground
*	功能说明: 桌面窗口的回调函数的WM_PAINT消息
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void _DrawBackground(void) 
{
	GUI_DrawGradientV(  0,  0, LCD_GetXSize(),  LCD_GetYSize(), 0xff0000, 0xff3131);
	GUI_SetColor(GUI_WHITE);
	GUI_FillRect(  5,  40, 314,  79);
	GUI_DrawGradientV(5, 80, 314, 239, 0xffffff, 0xffa0a0);

	GUI_UC_SetEncodeUTF8();
	GUI_SetFont(&GUI_FontYahei);
	GUI_SetTextMode(GUI_TM_TRANS);
	GUI_SetColor(GUI_WHITE);
	GUI_DispStringHCenterAt("洗衣机简易操作界面", 160, 5);
	GUI_UC_SetEncodeUTF8();
}

/*
*********************************************************************************************************
*	函 数 名: _cbBkWin
*	功能说明: 桌面窗口的回调函数
*	形    参: pMsg 指针参数
*	返 回 值: 无
*********************************************************************************************************
*/
static void _cbBkWin(WM_MESSAGE * pMsg) 
{
	switch (pMsg->MsgId) 
	{
		case WM_PAINT:
			_DrawBackground();
			break;
		
		default:
			WM_DefaultProc(pMsg);
	}
}

/*
*********************************************************************************************************
*	函 数 名: _CreateButton
*	功能说明: 创建自定义按钮到内存设备中
*	形    参: hWin      窗口句柄
*             xPos      起始X轴位置
*             yPos      起始Y轴位置
*             xSize     长度
*             ySize     高度
*             Color0    绘制RoundBar的梯度色
*             Color1    绘制RoundBar的梯度色
*             Gradient0 梯度色
*             Gradient1 梯度色
*	返 回 值: hMem  内存设备创建后的句柄
*********************************************************************************************************
*/
static GUI_MEMDEV_Handle _CreateButton(WM_HWIN hWin, int xPos, int yPos, int xSize, int ySize, GUI_COLOR Color0, GUI_COLOR Color1, GUI_COLOR Gradient0, GUI_COLOR Gradient1) 
{
	GUI_MEMDEV_Handle hMem;
	GUI_MEMDEV_Handle hOld;
	int               xPosWin;
	int               yPosWin;
	int               r;

	r = xSize / 2;
	
	/* 首先要选择操作的窗口 */
	WM_SelectWindow(hWin);
	xPosWin = WM_GetWindowOrgX(hWin);
	yPosWin = WM_GetWindowOrgY(hWin);
	
	/* 创建内存设备 */
	hMem = GUI_MEMDEV_Create(xPosWin + xPos, yPosWin + yPos, xSize, ySize);
	
	/* 创建成功后，在内存设备上绘制按钮控件 */
	if (hMem) 
	{
		hOld = GUI_MEMDEV_Select(hMem);
		GUI_DrawGradientV(xPos, yPos, xPos + xSize, yPos + ySize, Gradient0, Gradient1);
		_DrawGradientRoundBar(xPos + 2, yPos + 2, xPos + xSize - 2, yPos + ySize - 2, Color0, Color1);
		GUI_SetColor(0xff0000);
		GUI_SetPenSize(3);
		GUI_AA_DrawArc(xPos + r, yPos + r, r - 2, r - 2, 0, 360);
		GUI_MEMDEV_Select(hOld);
	} 
	else 
	{
		_NoMemory = 1;
	}
	
	return hMem;
}

/*
*********************************************************************************************************
*	函 数 名: _cbButton
*	功能说明: 此函数主要用于得到按键的键值。
*	形    参：pMsg 指针参数
*	返 回 值: 无
*********************************************************************************************************
*/
static void _cbButton(WM_MESSAGE * pMsg) 
{
	static GUI_MEMDEV_Handle   hButton00;
	static GUI_MEMDEV_Handle   hButton01;
	static GUI_MEMDEV_Handle   hButton10;
	static GUI_MEMDEV_Handle   hButton11;
	GUI_MEMDEV_Handle          hShow;
	WM_HWIN                    hWin;
	char                     * pText;
	int                        IsPressed;

	hWin = pMsg->hWin;
	switch (pMsg->MsgId) 
	{
		case WM_PAINT:
			/* 获取要显示的文本和按钮控件 */
		    GUI_SetColor(0xFFFFFF);
			GUI_SetFont(&GUI_Font18B);
			GUI_SetTextMode(GUI_TM_TRANS);
			IsPressed = BUTTON_IsPressed(hWin);
			hShow     = (_State == STATE_WASHING) ? (IsPressed ? hButton11 : hButton10) : (IsPressed ? hButton01 : hButton00);
			pText     = (_State == STATE_WASHING) ? "Skip" : "Start";
			
		    /* 要显示的按钮内容已经放到了内存设备中，这里直接调用下面的函数就能显示出来 */
		    GUI_MEMDEV_Write(hShow);
		
		    /* 显示文本 */
			GUI_DispStringHCenterAt(pText, 30, 21);
			break;
		
		case INIT_BUTTON:
			/* 创建四种按钮所需要的内存设备 */
			hButton00 = _CreateButton(hWin, 0, 0, 60, 60, 0x00aa00, 0xEEEEEE, 0xffe0e0, 0xffc0c0);
			hButton01 = _CreateButton(hWin, 0, 0, 60, 60, 0xEEEEEE, 0x00aa00, 0xffe0e0, 0xffc0c0);
			hButton10 = _CreateButton(hWin, 0, 0, 60, 60, 0x4040ff, 0xEEEEEE, 0xffe0e0, 0xffc0c0);
			hButton11 = _CreateButton(hWin, 0, 0, 60, 60, 0xEEEEEE, 0x4040ff, 0xffe0e0, 0xffc0c0);
			break;
		
		case WM_DELETE:
			/* 按钮被删除后，要删除前面申请的内存设备 */
			GUI_MEMDEV_Delete(hButton00);
			GUI_MEMDEV_Delete(hButton01);
			GUI_MEMDEV_Delete(hButton10);
			GUI_MEMDEV_Delete(hButton11);
			hButton00 = 0;
			hButton01 = 0;
			hButton10 = 0;
			hButton11 = 0;
			break;
		
		default:
			BUTTON_Callback(pMsg);
	}
}

/*
*********************************************************************************************************
*	函 数 名: _DemoWashingMachine
*	功能说明: 洗衣机演示程序，这个函数是可以被循环调用的，也就是说，这个函数不是死循环，如果按下了洗衣机
*             界面上的start按钮，执行完演示后就会退出这个函数，然后再重新的进入此函数。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void _DemoWashingMachine(void) 
{
	GUI_TIMER_TIME         TimeDiff;
	GUI_TIMER_TIME         TimeEnd;
	GUI_TIMER_TIME         TimeNow;
	WM_CALLBACK          * pcbPrev;
	WM_HWIN                hWinMain;

	(void) _NoMemory; /* 防止警告 */
	
	/* 初始化相关变量 */
	_NoMemory      = 0;
	_Time          = 90;
	_aSelection[0] = 1;
	_aSelection[1] = 0;
	_aSelection[2] = 1;
	_aSelection[3] = 1;
	_aSelection[4] = 0;
	_Selection     = 2;
	_State         = 0;
	
	
	/* 设置桌面窗口的回调函数，并返回源桌面窗口的回调函数 */
	pcbPrev  = WM_SetCallback(WM_HBKWIN, _cbBkWin);
	
	/* 创建桌面窗口的子窗口，注意这里设置了透明标志 */
	hWinMain = WM_CreateWindowAsChild(5, 40, 310, 200, WM_HBKWIN, WM_CF_SHOW | WM_CF_HASTRANS, _cbMain, 0);
	
	/* 设置聚焦 */
	WM_SetFocus(hWinMain);
	
	/* 在窗口hWinMain上面创建按钮 */
	_hButton = BUTTON_CreateEx(10, 90, 60, 60, hWinMain, WM_CF_SHOW, 0, GUI_ID_BUTTON0);
	
	/* 设置按钮的回调函数，本身按钮的回调函数是自动执行的，这里属于自定义回调函数 */
	WM_SetCallback(_hButton, _cbButton);
	
	/* 发送消息，对按钮就行初始化 */
	WM_SendMessageNoPara(_hButton, INIT_BUTTON);
	
	/* 设置聚焦 */
	WM_SetFocus(_hButton);
	
	/* 如果创建了状态窗口_hStatus，在这里要将其删除，防止再次进入这个函数时重复的创建，从而造成
	   内存溢出。
	*/
	if (_hStatus) 
	{
		WM_DeleteWindow(_hStatus);
		WM_InvalidateWindow(hWinMain);
		_hStatus = 0;
	}
	
	while (1) 
	{
		/* 获取TimeNow和TimeEnd的数值*/
		TimeNow = GUI_GetTime();
		TimeEnd = TimeNow + 9000;
		
		/* 在9000ms的时间内做一个死循环 */
		while (TimeNow < TimeEnd) 
		{
			GUI_Delay(10);
			
			/* 如果在9000ms的时间内用户点击了start按钮，就开始动态更新相应界面 */
			if (_State == STATE_WASHING) 
			{
				TimeDiff = TimeEnd - TimeNow;
				_Time    = (TimeDiff * 90) / 9000;
				
				/* 如果界面窗口创建成功了，就进行更新 */
				if (_hStatus) 
				{
					WM_InvalidateWindow(hWinMain);
					WM_InvalidateWindow(_hStatus);
				}
			}		
			
			TimeNow = GUI_GetTime();
		}
		
		/* 演示完后删除按钮hButton，窗口hWinMain，并重还原为默认的桌面窗口回调函数 
		   然后退出本函数_DemoWashingMachine，再重新进入。
		*/
		if (_State == STATE_WASHING)
		{
			WM_SetCallback(WM_HBKWIN, pcbPrev);
			WM_DeleteWindow(_hButton);
			WM_DeleteWindow(hWinMain);
			break;	
		}		
	}
}

/*
*********************************************************************************************************
*	函 数 名: MainTask
*	功能说明: GUI主函数
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void MainTask(void) 
{
	/* 初始化 */
	GUI_Init();
	 
	/*
	 关于多缓冲和窗口内存设备的设置说明
	   1. 使能多缓冲是调用的如下函数，用户要在LCDConf_Lin_Template.c文件中配置了多缓冲，调用此函数才有效：
		  WM_MULTIBUF_Enable(1);
	   2. 窗口使能使用内存设备是调用函数：WM_SetCreateFlags(WM_CF_MEMDEV);
	   3. 如果emWin的配置多缓冲和窗口内存设备都支持，二选一即可，且务必优先选择使用多缓冲，实际使用
		  STM32F429BIT6 + 32位SDRAM + RGB565/RGB888平台测试，多缓冲可以有效的降低窗口移动或者滑动时的撕裂
		  感，并有效的提高流畅性，通过使能窗口使用内存设备是做不到的。
	   4. 所有emWin例子默认是开启三缓冲。
	*/
	WM_MULTIBUF_Enable(1);
	
	/*
       触摸校准函数默认是注释掉的，电阻屏需要校准，电容屏无需校准。如果用户需要校准电阻屏的话，执行
	   此函数即可，会将触摸校准参数保存到EEPROM里面，以后系统上电会自动从EEPROM里面加载。
	*/
    //TOUCH_Calibration();
	
	while (1) 
	{
		_DemoWashingMachine();
	}
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/

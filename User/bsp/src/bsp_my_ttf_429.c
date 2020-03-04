#include "bsp.h"


typedef uint32_t LCD_COLOR;

#define LCD429_FRAME_BUFFER EXT_SDRAM_ADDR

/* 偏移地址计算公式::
   Maximum width x Maximum Length x Maximum Pixel size (ARGB8888) in bytes
   => 640 x 480 x 4 =  1228800 or 0x12C000 */
 #define BUFFER_OFFSET (uint32_t)(g_LcdHeight * g_LcdWidth * 2)

uint32_t s_CurrentFrameBuffer;
uint8_t s_CurrentLayer;

static LTDC_HandleTypeDef Ltdc_Handler = {0};
static DMA2D_HandleTypeDef  hdma2d = {0};

void LCD429_SetPixelFormat(uint32_t PixelFormat);

static void LCD429_InitDMA2D(void);
static inline uint32_t LCD_LL_GetPixelformat(uint32_t LayerIndex);
void DMA2D_CopyBuffer(uint32_t LayerIndex, void * pSrc, void * pDst, uint32_t xSize, uint32_t ySize, uint32_t OffLineSrc, uint32_t OffLineDst);
static void DMA2D_FillBuffer(uint32_t LayerIndex, void * pDst, uint32_t xSize, uint32_t ySize, uint32_t OffLine, uint32_t ColorIndex);


/*
*********************************************************************************************************
*	函 数 名: LCD429_GetChipDescribe
*	功能说明: 读取LCD驱动芯片的描述符号，用于显示
*	形    参: char *_str : 描述符字符串填入此缓冲区
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_GetChipDescribe(char *_str)
{
	strcpy(_str, "STM32F429");
}

/*
*********************************************************************************************************
*	函 数 名: LCD429_SetLayer
*	功能说明: 切换层。只是更改程序变量，以便于后面的代码更改相关寄存器。硬件支持2层。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_SetLayer(uint8_t _ucLayer)
{
	if (_ucLayer == LCD_LAYER_1)
	{
		s_CurrentFrameBuffer = LCD429_FRAME_BUFFER;
		s_CurrentLayer = LCD_LAYER_1;
	}
	else if (_ucLayer == LCD_LAYER_2)
	{
		s_CurrentFrameBuffer = LCD429_FRAME_BUFFER + BUFFER_OFFSET;
		s_CurrentLayer = LCD_LAYER_2;
	}
}


/*
*********************************************************************************************************
*	函 数 名: LCD429_SetTransparency
*	功能说明: 配置当前层的透明属性
*	形    参: 透明度， 值域： 0x00 - 0xFF
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_SetTransparency(uint8_t transparency)
{
	HAL_LTDC_SetAlpha(&Ltdc_Handler, transparency, s_CurrentLayer);	/* 立即刷新 */
}


/*
*********************************************************************************************************
*	函 数 名: LCD429_SetPixelFormat
*	功能说明: 配置当前层的像素格式
*	形    参: 像素格式:
*                      LTDC_PIXEL_FORMAT_ARGB8888
*                      LTDC_PIXEL_FORMAT_ARGB8888_RGB888
*                      LTDC_PIXEL_FORMAT_ARGB8888_RGB565
*                      LTDC_PIXEL_FORMAT_ARGB8888_ARGB1555
*                      LTDC_PIXEL_FORMAT_ARGB8888_ARGB4444
*                      LTDC_PIXEL_FORMAT_ARGB8888_L8
*                      LTDC_PIXEL_FORMAT_ARGB8888_AL44
*                      LTDC_PIXEL_FORMAT_ARGB8888_AL88
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_SetPixelFormat(uint32_t PixelFormat)
{
	HAL_LTDC_SetPixelFormat(&Ltdc_Handler, PixelFormat, s_CurrentLayer);
}


/*
*********************************************************************************************************
*	函 数 名: LCD429_SetDispWin
*	功能说明: 设置显示窗口，进入窗口显示模式。
*	形    参:  
*		_usX : 水平坐标
*		_usY : 垂直坐标
*		_usHeight: 窗口高度
*		_usWidth : 窗口宽度
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_SetDispWin(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth)
{
	HAL_LTDC_SetWindowSize_NoReload(&Ltdc_Handler, _usWidth, _usHeight, s_CurrentLayer);	/* 不立即更新 */
	HAL_LTDC_SetWindowPosition(&Ltdc_Handler, _usX, _usY, s_CurrentLayer);		/* 立即更新 */
}


/*
*********************************************************************************************************
*	函 数 名: LCD429_QuitWinMode
*	功能说明: 退出窗口显示模式，变为全屏显示模式
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_QuitWinMode(void)
{
	HAL_LTDC_SetWindowSize_NoReload(&Ltdc_Handler, g_LcdWidth, g_LcdHeight, s_CurrentLayer);	/* 不立即更新 */
	HAL_LTDC_SetWindowPosition(&Ltdc_Handler, 0, 0, s_CurrentLayer);		/* 立即更新 */	
}

/*
*********************************************************************************************************
*	函 数 名: LCD429_DispOn
*	功能说明: 打开显示
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_DispOn(void)
{

}

/*
*********************************************************************************************************
*	函 数 名: LCD429_DispOff
*	功能说明: 关闭显示
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_DispOff(void)
{

}


/*
*********************************************************************************************************
*	函 数 名: LCD429_ClrScr
*	功能说明: 根据输入的颜色值清屏
*	形    参: _usColor : 背景色
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_ClrScr(uint16_t _usColor)
{
#if 1
	LCD429_FillRect(0, 0, g_LcdHeight, g_LcdWidth, _usColor);
#else
	uint16_t *index ;
	uint32_t i;

	index = (uint16_t *)s_CurrentFrameBuffer;

	for (i = 0; i < g_LcdHeight * g_LcdWidth; i++)
	{
		*index++ = _usColor;
	}
#endif	
}


/*
*********************************************************************************************************
*	函 数 名: LCD429_PutPixel
*	功能说明: 画1个像素
*	形    参:
*			_usX,_usY : 像素坐标
*			_usColor  : 像素颜色 ( RGB = 565 格式)
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_PutPixel(uint16_t _usX, uint16_t _usY, uint16_t _usColor)
{
	uint16_t *p;
	uint32_t index = 0;
	
	p = (uint16_t *)s_CurrentFrameBuffer;
		
	if (g_LcdDirection == 0)		/* 横屏 */
	{
		index = (uint32_t)_usY * g_LcdWidth + _usX;
	}
	else if (g_LcdDirection == 1)	/* 横屏180°*/
	{
		index = (uint32_t)(g_LcdHeight - _usY - 1) * g_LcdWidth + (g_LcdWidth - _usX - 1);
	}
	else if (g_LcdDirection == 2)	/* 竖屏 */
	{
		index = (uint32_t)(g_LcdWidth - _usX - 1) * g_LcdHeight + _usY;
	}
	else if (g_LcdDirection == 3)	/* 竖屏180° */
	{
		index = (uint32_t)_usX * g_LcdHeight + g_LcdHeight - _usY - 1;
	}	
	
	if (index < g_LcdHeight * g_LcdWidth)
	{
		p[index] = _usColor;
	}
}



/*
*********************************************************************************************************
*	函 数 名: LCD429_GetPixel
*	功能说明: 读取1个像素
*	形    参:
*			_usX,_usY : 像素坐标
*			_usColor  : 像素颜色
*	返 回 值: RGB颜色值
*********************************************************************************************************
*/
uint16_t LCD429_GetPixel(uint16_t _usX, uint16_t _usY)
{
	uint16_t usRGB;
	uint16_t *p;
	uint32_t index = 0;
	
	p = (uint16_t *)s_CurrentFrameBuffer;

	if (g_LcdDirection == 0)		/* 横屏 */
	{
		index = (uint32_t)_usY * g_LcdWidth + _usX;
	}
	else if (g_LcdDirection == 1)	/* 横屏180°*/
	{
		index = (uint32_t)(g_LcdHeight - _usY - 1) * g_LcdWidth + (g_LcdWidth - _usX - 1);
	}
	else if (g_LcdDirection == 2)	/* 竖屏 */
	{
		index = (uint32_t)(g_LcdWidth - _usX - 1) * g_LcdHeight + _usY;
	}
	else if (g_LcdDirection == 3)	/* 竖屏180° */
	{
		index = (uint32_t)_usX * g_LcdHeight + g_LcdHeight - _usY - 1;
	}
	
	usRGB = p[index];

	return usRGB;
}



/*
*********************************************************************************************************
*	函 数 名: LCD429_DrawLine
*	功能说明: 采用 Bresenham 算法，在2点间画一条直线。
*	形    参:
*			_usX1, _usY1 : 起始点坐标
*			_usX2, _usY2 : 终止点Y坐标
*			_usColor     : 颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_DrawLine(uint16_t _usX1 , uint16_t _usY1 , uint16_t _usX2 , uint16_t _usY2 , uint16_t _usColor)
{
	int32_t dx , dy ;
	int32_t tx , ty ;
	int32_t inc1 , inc2 ;
	int32_t d , iTag ;
	int32_t x , y ;

	/* 采用 Bresenham 算法，在2点间画一条直线 */

	LCD429_PutPixel(_usX1 , _usY1 , _usColor);

	/* 如果两点重合，结束后面的动作。*/
	if ( _usX1 == _usX2 && _usY1 == _usY2 )
	{
		return;
	}

	iTag = 0 ;
	/* dx = abs ( _usX2 - _usX1 ); */
	if (_usX2 >= _usX1)
	{
		dx = _usX2 - _usX1;
	}
	else
	{
		dx = _usX1 - _usX2;
	}

	/* dy = abs ( _usY2 - _usY1 ); */
	if (_usY2 >= _usY1)
	{
		dy = _usY2 - _usY1;
	}
	else
	{
		dy = _usY1 - _usY2;
	}

	if ( dx < dy )   /*如果dy为计长方向，则交换纵横坐标。*/
	{
		uint16_t temp;

		iTag = 1 ;
		temp = _usX1; _usX1 = _usY1; _usY1 = temp;
		temp = _usX2; _usX2 = _usY2; _usY2 = temp;
		temp = dx; dx = dy; dy = temp;
	}
	tx = _usX2 > _usX1 ? 1 : -1 ;    /* 确定是增1还是减1 */
	ty = _usY2 > _usY1 ? 1 : -1 ;
	x = _usX1 ;
	y = _usY1 ;
	inc1 = 2 * dy ;
	inc2 = 2 * ( dy - dx );
	d = inc1 - dx ;
	while ( x != _usX2 )     /* 循环画点 */
	{
		if ( d < 0 )
		{
			d += inc1 ;
		}
		else
		{
			y += ty ;
			d += inc2 ;
		}
		if ( iTag )
		{
			LCD429_PutPixel ( y , x , _usColor) ;
		}
		else
		{
			LCD429_PutPixel ( x , y , _usColor) ;
		}
		x += tx ;
	}	
}


/*
*********************************************************************************************************
*	函 数 名: LCD429_DrawHLine
*	功能说明: 绘制一条水平线. 使用STM32F429 DMA2D硬件绘制.
*	形    参:
*			_usX1, _usY1 : 起始点坐标
*			_usLen       : 线的长度
*			_usColor     : 颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_DrawHLine(uint16_t _usX, uint16_t _usY, uint16_t _usLen , uint16_t _usColor)
{
#if 0
	LCD429_FillRect(_usX, _usY, 1, _usLen, _usColor);
#else	
	uint16_t i;
	
	for (i = 0; i < _usLen; i++)
	{	
		LCD429_PutPixel(_usX + i , _usY , _usColor);
	}
#endif	
}


/*
*********************************************************************************************************
*	函 数 名: LCD429_DrawVLine
*	功能说明: 绘制一条垂直线。 使用STM32F429 DMA2D硬件绘制.
*	形    参:
*			_usX, _usY : 起始点坐标
*			_usLen       : 线的长度
*			_usColor     : 颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_DrawVLine(uint16_t _usX , uint16_t _usY , uint16_t _usLen , uint16_t _usColor)
{
#if 0
	LCD429_FillRect(_usX, _usY, _usLen, 1, _usColor);
#else	
	uint16_t i;
	
	for (i = 0; i < _usLen; i++)
	{	
		LCD429_PutPixel(_usX, _usY + i, _usColor);
	}
#endif	
}


/*
*********************************************************************************************************
*	函 数 名: LCD429_DrawPoints
*	功能说明: 采用 Bresenham 算法，绘制一组点，并将这些点连接起来。可用于波形显示。
*	形    参:
*			x, y     : 坐标数组
*			_usColor : 颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_DrawPoints(uint16_t *x, uint16_t *y, uint16_t _usSize, uint16_t _usColor)
{
	uint16_t i;

	for (i = 0 ; i < _usSize - 1; i++)
	{
		LCD429_DrawLine(x[i], y[i], x[i + 1], y[i + 1], _usColor);
	}
}


/*
*********************************************************************************************************
*	函 数 名: LCD429_DrawRect
*	功能说明: 绘制水平放置的矩形。
*	形    参:
*			_usX,_usY: 矩形左上角的坐标
*			_usHeight : 矩形的高度
*			_usWidth  : 矩形的宽度
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_DrawRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor)
{
	/*
	 ---------------->---
	|(_usX，_usY)        |
	V                    V  _usHeight
	|                    |
	 ---------------->---
		  _usWidth
	*/
	LCD429_DrawHLine(_usX, _usY, _usWidth, _usColor);
	LCD429_DrawVLine(_usX +_usWidth - 1, _usY, _usHeight, _usColor);
	LCD429_DrawHLine(_usX, _usY + _usHeight - 1, _usWidth, _usColor);
	LCD429_DrawVLine(_usX, _usY, _usHeight, _usColor);
}


/*
*********************************************************************************************************
*	函 数 名: LCD429_FillRect
*	功能说明: 用一个颜色值填充一个矩形。使用STM32F429内部DMA2D硬件绘制。
*	形    参:
*			_usX,_usY: 矩形左上角的坐标
*			_usHeight : 矩形的高度
*			_usWidth  : 矩形的宽度
*			_usColor  : 颜色代码
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_FillRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor)
{
#if 1	/* 将库函数展开，提高执行效率 */
	uint32_t  Xaddress = 0;
	uint16_t  OutputOffset = 0;
	uint16_t  NumberOfLine = 0;
	uint16_t  PixelPerLine = 0;	

	/* 根据显示方向设置不同的参数 */
	if (g_LcdDirection == 0)		/* 横屏 */
	{
		Xaddress = s_CurrentFrameBuffer + 2 * (g_LcdWidth * _usY + _usX);	
		OutputOffset = g_LcdWidth - _usWidth;
		NumberOfLine = _usHeight;
		PixelPerLine = _usWidth;
	}
	else if (g_LcdDirection == 1)	/* 横屏180°*/
	{
		Xaddress = s_CurrentFrameBuffer + 2 * (g_LcdWidth * (g_LcdHeight - _usHeight - _usY) + g_LcdWidth - _usX - _usWidth);	
		OutputOffset = g_LcdWidth - _usWidth;
		NumberOfLine = _usHeight;
		PixelPerLine = _usWidth;
	}
	else if (g_LcdDirection == 2)	/* 竖屏 */
	{
		Xaddress = s_CurrentFrameBuffer + 2 * (g_LcdHeight * (g_LcdWidth - _usX -  _usWidth) + _usY);	
		OutputOffset = g_LcdHeight - _usHeight;
		NumberOfLine = _usWidth;
		PixelPerLine = _usHeight;
	}
	else if (g_LcdDirection == 3)	/* 竖屏180° */
	{
		Xaddress = s_CurrentFrameBuffer + 2 * (g_LcdHeight * _usX + g_LcdHeight - _usHeight - _usY);	
		OutputOffset = g_LcdHeight - _usHeight;
		NumberOfLine = _usWidth;
		PixelPerLine = _usHeight;		
	}	

	DMA2D_FillBuffer(s_CurrentLayer, (void *)Xaddress, PixelPerLine, NumberOfLine, OutputOffset, _usColor);
#endif

#if 0	/* 使用库函数 -- 容易理解 */
	/* 使用DMA2D硬件填充矩形 */
	DMA2D_InitTypeDef      DMA2D_InitStruct;
	uint32_t  Xaddress = 0;
	uint16_t  OutputOffset;
	uint16_t  NumberOfLine;
	uint16_t  PixelPerLine;	
		
	if (g_LcdDirection == 0)		/* 横屏 */
	{
		Xaddress = s_CurrentFrameBuffer + 2 * (g_LcdWidth * _usY + _usX);	
		OutputOffset = g_LcdWidth - _usWidth;
		NumberOfLine = _usHeight;
		PixelPerLine = _usWidth;
	}
	else if (g_LcdDirection == 1)	/* 横屏180°*/
	{
		Xaddress = s_CurrentFrameBuffer + 2 * (g_LcdWidth * (g_LcdHeight - _usHeight - _usY) + g_LcdWidth - _usX - _usWidth);	
		OutputOffset = g_LcdWidth - _usWidth;
		NumberOfLine = _usHeight;
		PixelPerLine = _usWidth;
	}
	else if (g_LcdDirection == 2)	/* 竖屏 */
	{
		Xaddress = s_CurrentFrameBuffer + 2 * (g_LcdHeight * (g_LcdWidth - _usX -  _usWidth) + _usY);	
		OutputOffset = g_LcdHeight - _usHeight;
		NumberOfLine = _usWidth;
		PixelPerLine = _usHeight;
	}
	else if (g_LcdDirection == 3)	/* 竖屏180° */
	{
		Xaddress = s_CurrentFrameBuffer + 2 * (g_LcdHeight * _usX + g_LcdHeight - _usHeight - _usY);	
		OutputOffset = g_LcdHeight - _usHeight;
		NumberOfLine = _usWidth;
		PixelPerLine = _usHeight;		
	}	

	/* 配置 DMA2D */
	DMA2D_DeInit();		/* 复位 DMA2D */
	DMA2D_InitStruct.DMA2D_Mode = DMA2D_R2M;       /* 传输模式: 寄存器到存储器 */
	DMA2D_InitStruct.DMA2D_CMode = DMA2D_RGB565;   /* 颜色模式， RGB565 */   
	
	DMA2D_InitStruct.DMA2D_OutputRed = RGB565_R2(_usColor);		/* 红色分量，5bit，高位为0 */           
	DMA2D_InitStruct.DMA2D_OutputGreen = RGB565_G2(_usColor);	/* 绿色分量，6bit，高位为0 */
	DMA2D_InitStruct.DMA2D_OutputBlue = RGB565_B2(_usColor); 	/* 蓝色分量，5bit，高位为0 */    
	
	DMA2D_InitStruct.DMA2D_OutputAlpha = 0x0F;    				/* 透明参数， 对于565格式，无意义 */              
	DMA2D_InitStruct.DMA2D_OutputMemoryAdd = Xaddress;      	/* 目标内存地址 */          
	DMA2D_InitStruct.DMA2D_OutputOffset = OutputOffset; 	/* 每行间的显存像素地址差值  */               
	DMA2D_InitStruct.DMA2D_NumberOfLine = NumberOfLine;     /* 一共有几行 */        
	DMA2D_InitStruct.DMA2D_PixelPerLine = PixelPerLine;		/* 每行几个像素 */
	DMA2D_Init(&DMA2D_InitStruct); 

	/* Start Transfer */ 
	DMA2D_StartTransfer();

	/* Wait for CTC Flag activation */
	while(DMA2D_GetFlagStatus(DMA2D_FLAG_TC) == RESET)
	{
	}
#endif	
}


/*
*********************************************************************************************************
*	函 数 名: LCD429_DrawCircle
*	功能说明: 绘制一个圆，笔宽为1个像素
*	形    参:
*			_usX,_usY  : 圆心的坐标
*			_usRadius  : 圆的半径
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_DrawCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor)
{
	int32_t  D;			/* Decision Variable */
	uint32_t  CurX;		/* 当前 X 值 */
	uint32_t  CurY;		/* 当前 Y 值 */

	D = 3 - (_usRadius << 1);
	CurX = 0;
	CurY = _usRadius;

	while (CurX <= CurY)
	{
		LCD429_PutPixel(_usX + CurX, _usY + CurY, _usColor);
		LCD429_PutPixel(_usX + CurX, _usY - CurY, _usColor);
		LCD429_PutPixel(_usX - CurX, _usY + CurY, _usColor);
		LCD429_PutPixel(_usX - CurX, _usY - CurY, _usColor);
		LCD429_PutPixel(_usX + CurY, _usY + CurX, _usColor);
		LCD429_PutPixel(_usX + CurY, _usY - CurX, _usColor);
		LCD429_PutPixel(_usX - CurY, _usY + CurX, _usColor);
		LCD429_PutPixel(_usX - CurY, _usY - CurX, _usColor);

		if (D < 0)
		{
			D += (CurX << 2) + 6;
		}
		else
		{
			D += ((CurX - CurY) << 2) + 10;
			CurY--;
		}
		CurX++;
	}
}


/*
*********************************************************************************************************
*	函 数 名: LCD429_DrawBMP
*	功能说明: 在LCD上显示一个BMP位图，位图点阵扫描次序：从左到右，从上到下
*	形    参:  
*			_usX, _usY : 图片的坐标
*			_usHeight  ：图片高度
*			_usWidth   ：图片宽度
*			_ptr       ：图片点阵指针
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_DrawBMP(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t *_ptr)
{
	uint16_t i, k, y;
	const uint16_t *p;

	p = _ptr;
	y = _usY;
	for (i = 0; i < _usHeight; i++)
	{
		for (k = 0; k < _usWidth; k++)
		{
			LCD429_PutPixel(_usX + k, y, *p++);
		}
		
		y++;
	}
}


/*
*********************************************************************************************************
*	函 数 名: LCD429_SetDirection
*	功能说明: 设置显示屏显示方向（横屏 竖屏）
*	形    参: 显示方向代码 0 横屏正常, 1=横屏180度翻转, 2=竖屏, 3=竖屏180度翻转
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_SetDirection(uint8_t _dir)
{
	uint16_t temp;
	
	if (_dir == 0 || _dir == 1)		/* 横屏， 横屏180度 */
	{
		if (g_LcdWidth < g_LcdHeight)
		{
			temp = g_LcdWidth;
			g_LcdWidth = g_LcdHeight;
			g_LcdHeight = temp;			
		}
	}
	else if (_dir == 2 || _dir == 3)	/* 竖屏, 竖屏180°*/
	{
		if (g_LcdWidth > g_LcdHeight)
		{
			temp = g_LcdWidth;
			g_LcdWidth = g_LcdHeight;
			g_LcdHeight = temp;			
		}
	}
}


/*
*********************************************************************************************************
*	函 数 名: LCD429_BeginDraw
*	功能说明: 双缓冲区工作模式。开始绘图。将当前显示缓冲区的数据完整复制到另外一个缓冲区。
*			 必须和 LCD429_EndDraw函数成对使用。 实际效果并不好。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_BeginDraw(void)
{
#if 0
	uint16_t *src = NULL;
	uint16_t *dst = NULL;
		
	if (s_CurrentFrameBuffer == LCD429_FRAME_BUFFER)
	{
		src = (uint16_t *)LCD429_FRAME_BUFFER;
		dst =  (uint16_t *)(LCD429_FRAME_BUFFER + BUFFER_OFFSET);
		
		s_CurrentFrameBuffer = LCD429_FRAME_BUFFER + BUFFER_OFFSET;
	}
	else
	{
		src = (uint16_t *)(LCD429_FRAME_BUFFER + BUFFER_OFFSET);
		dst = (uint16_t *)LCD429_FRAME_BUFFER;
		
		s_CurrentFrameBuffer = LCD429_FRAME_BUFFER;
	}
	
	_DMA_Copy(src, dst, g_LcdHeight, g_LcdWidth, 0, 0);
#endif
}


/*
*********************************************************************************************************
*	函 数 名: LCD429_EndDraw
*	功能说明: APP结束了缓冲区绘图工作，切换硬件显示。
*			 必须和 LCD429_BeginDraw函数成对使用。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_EndDraw(void)
{
	//HAL_LTDC_SetAddress(Ltdc_Handler, s_CurrentFrameBuffer, LTDC_Layer1);
}

/*
*********************************************************************************************************
*	函 数 名: _GetBufferSize
*	功能说明: 无
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
uint32_t _GetBufferSize(uint8_t LayerIndex)
{
	return g_LcdWidth * g_LcdHeight;
}



/*
*********************************************************************************************************
*	函 数 名: _DMA_Copy
*	功能说明: 复制图块
*	形    参: 
*			 pSrc : 源内存地址
*			 pDst : 目标内存地址
*			 xSize : 矩形x尺寸
*			 ySize : 矩形y尺寸
*			 OffLineSrc : 源图块行偏移
*			 OffLineDst : 目标图块行偏移
*	返 回 值: 无
*********************************************************************************************************
*/
static void _DMA_Copy(void * pSrc, void * pDst, int xSize, int ySize, int OffLineSrc, int OffLineDst) 
{
	DMA2D->CR      = 0x00000000UL | (1 << 9);  	// Control Register (Memory to memory and TCIE)
	DMA2D->FGMAR   = (uint32_t)pSrc;            // Foreground Memory Address Register (Source address)
	DMA2D->OMAR    = (uint32_t)pDst;        	// Output Memory Address Register (Destination address)
	DMA2D->FGOR    = OffLineSrc;           		// Foreground Offset Register (Source line offset)
	DMA2D->OOR     = OffLineDst;            	// Output Offset Register (Destination line offset)
	DMA2D->FGPFCCR = LTDC_PIXEL_FORMAT_RGB565;           	// Foreground PFC Control Register (Defines the input pixel format)
	DMA2D->NLR     = (uint32_t)(xSize << 16) | (uint16_t)ySize; // Number of Line Register (Size configuration of area to be transfered)
	DMA2D->CR     |= 1;                               // Start operation
	//
	// Wait until transfer is done
	//
	while (DMA2D->CR & DMA2D_CR_START)
	{
		//__WFI();                                        // Sleep until next interrupt
	}
}


static void LCD429_AF_GPIOConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure = {0};

	__HAL_RCC_GPIOI_CLK_ENABLE();
	__HAL_RCC_GPIOJ_CLK_ENABLE();
	__HAL_RCC_GPIOK_CLK_ENABLE();

	/* GPIOs Configuration */
	/*
	+------------------------+-----------------------+----------------------------+
	+                       LCD pins assignment                                   +
	+------------------------+-----------------------+----------------------------+
	|  LCD429_TFT R0 <-> PI.15  |  LCD429_TFT G0 <-> PJ.07 |  LCD429_TFT B0 <-> PJ.12      |
	|  LCD429_TFT R1 <-> PJ.00  |  LCD429_TFT G1 <-> PJ.08 |  LCD429_TFT B1 <-> PJ.13      |
	|  LCD429_TFT R2 <-> PJ.01  |  LCD429_TFT G2 <-> PJ.09 |  LCD429_TFT B2 <-> PJ.14      |
	|  LCD429_TFT R3 <-> PJ.02  |  LCD429_TFT G3 <-> PJ.10 |  LCD429_TFT B3 <-> PJ.15      |
	|  LCD429_TFT R4 <-> PJ.03  |  LCD429_TFT G4 <-> PJ.11 |  LCD429_TFT B4 <-> PK.03      |
	|  LCD429_TFT R5 <-> PJ.04  |  LCD429_TFT G5 <-> PK.00 |  LCD429_TFT B5 <-> PK.04      |
	|  LCD429_TFT R6 <-> PJ.05  |  LCD429_TFT G6 <-> PK.01 |  LCD429_TFT B6 <-> PK.05      |
	|  LCD429_TFT R7 <-> PJ.06  |  LCD429_TFT G7 <-> PK.02 |  LCD429_TFT B7 <-> PK.06      |
	-------------------------------------------------------------------------------
	|  LCD429_TFT HSYNC <-> PI.12  | LCDTFT VSYNC <->  PI.13 |
	|  LCD429_TFT CLK   <-> PI.14  | LCD429_TFT DE   <->  PK.07 |
	-----------------------------------------------------
	*/
	GPIO_InitStructure.Pin = GPIO_PIN_All;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Alternate = GPIO_AF14_LTDC;
	HAL_GPIO_Init(GPIOJ, &GPIO_InitStructure);

	GPIO_InitStructure.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Alternate = GPIO_AF14_LTDC;
	HAL_GPIO_Init(GPIOI, &GPIO_InitStructure);

	GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | \
		GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Alternate = GPIO_AF14_LTDC;
	HAL_GPIO_Init(GPIOK, &GPIO_InitStructure);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
*********************************************************************************************************
*	                                 下面的函数被emWin所调用
*********************************************************************************************************
*/
/*
*********************************************************************************************************
*	函 数 名: LCD_ConfigLTDC
*	功能说明: 配置LTDC
*	形    参: 无
*	返 回 值: 无
*   笔    记:
*       LCD_TFT 同步时序配置（整理自官方做的一个截图，言简意赅）：
*       ----------------------------------------------------------------------------
*    
*                                                 Total Width
*                             <--------------------------------------------------->
*                       Hsync width HBP             Active Width                HFP
*                             <---><--><--------------------------------------><-->
*                         ____    ____|_______________________________________|____ 
*                             |___|   |                                       |    |
*                                     |                                       |    |
*                         __|         |                                       |    |
*            /|\    /|\  |            |                                       |    |
*             | VSYNC|   |            |                                       |    |
*             |Width\|/  |__          |                                       |    |
*             |     /|\     |         |                                       |    |
*             |  VBP |      |         |                                       |    |
*             |     \|/_____|_________|_______________________________________|    |
*             |     /|\     |         | / / / / / / / / / / / / / / / / / / / |    |
*             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*    Total    |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*    Heigh    |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*             |Active|      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*             |Heigh |      |         |/ / / / / / Active Display Area / / / /|    |
*             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*             |     \|/_____|_________|_______________________________________|    |
*             |     /|\     |                                                      |
*             |  VFP |      |                                                      |
*            \|/    \|/_____|______________________________________________________|
*            
*     
*     每个LCD设备都有自己的同步时序值：
*     Horizontal Synchronization (Hsync) 
*     Horizontal Back Porch (HBP)       
*     Active Width                      
*     Horizontal Front Porch (HFP)     
*   
*     Vertical Synchronization (Vsync)  
*     Vertical Back Porch (VBP)         
*     Active Heigh                       
*     Vertical Front Porch (VFP)         
*     
*     LCD_TFT 窗口水平和垂直的起始以及结束位置 :
*     ----------------------------------------------------------------
*   
*     HorizontalStart = (Offset_X + Hsync + HBP);
*     HorizontalStop  = (Offset_X + Hsync + HBP + Window_Width - 1); 
*     VarticalStart   = (Offset_Y + Vsync + VBP);
*     VerticalStop    = (Offset_Y + Vsync + VBP + Window_Heigh - 1);

*     上述的参数值，可以参考野火的历程说明
*
*********************************************************************************************************
*/

__IO uint16_t Width,Height, HSYNC_W, VSYNC_W, HBP, HFP, VBP, VFP;
void LCD_ConfigLTDC(void)
{
	
	RCC_PeriphCLKInitTypeDef Periph_ClkStructure = {0};
	/*使能时钟*/
	__HAL_RCC_LTDC_CLK_ENABLE();

	LCD429_AF_GPIOConfig();

	Ltdc_Handler.Instance = LTDC;
	/*配置信号极性*/
	Ltdc_Handler.Init.HSPolarity = LTDC_HSPOLARITY_AL;
	Ltdc_Handler.Init.VSPolarity = LTDC_VSPOLARITY_AL;
	Ltdc_Handler.Init.DEPolarity = LTDC_DEPOLARITY_AL;
	Ltdc_Handler.Init.PCPolarity = LTDC_PCPOLARITY_IPC;

	/*背景色配置*/
	Ltdc_Handler.Init.Backcolor.Blue = 0;
	Ltdc_Handler.Init.Backcolor.Green = 0;
	Ltdc_Handler.Init.Backcolor.Red = 0;

	/* 
	   LTDC时钟配置说明：(看数据手册)
	     函数RCC_PLLSAIConfig的第一个参数是PLLSAI_N，第三个参数数PLLSAI_R。
	     函数RCC_LTDCCLKDivConfig的参数是RCC_PLLSAIDivR。
	   
	   下面举一个例子：PLLSAI_N = 400， PLLSAI_R = 4  RCC_PLLSAIDivR = 2:
	     首先，输入时钟 PLLSAI_VCO Input = HSE_VALUE / PLL_M = 8M / 8 = 1MHz 
	       输出时钟 PLLSAI_VCO Output  = PLLSAI_VCO Input * PLLSAI_N = 1 * 400 = 400 1MHz 
	       PLLLCDCLK = PLLSAI_VCO Output / PLLSAI_R = 400 / 4 = 100 1MHz 
	     最好，LTDC 时钟 = PLLLCDCLK / RCC_PLLSAIDivR = 100 / 2 = 50 1MHz 
	 */

	/* 配置输出时钟为 15Mhz*/
	Periph_ClkStructure.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
	Periph_ClkStructure.PLLSAI.PLLSAIN = 420;
	Periph_ClkStructure.PLLSAI.PLLSAIR = 7;
	Periph_ClkStructure.PLLSAIDivR = RCC_PLLSAIDIVR_4;
	HAL_RCCEx_PeriphCLKConfig(&Periph_ClkStructure);

	Width = 800;
	Height = 480;

	HSYNC_W = 48;
	HBP = 88;
	HFP = 40;

	VSYNC_W = 3;
	VBP = 32;
	VFP = 13;
	
	g_LcdWidth = Width;
	g_LcdHeight = Height;

	Ltdc_Handler.LayerCfg->ImageWidth = Width;
	Ltdc_Handler.LayerCfg->ImageHeight = Height;
	
	/*配置LTDC 的同步时序*/
	Ltdc_Handler.Init.HorizontalSync = (HSYNC_W - 1);
	Ltdc_Handler.Init.VerticalSync = (VSYNC_W - 1);
	Ltdc_Handler.Init.AccumulatedHBP = (HSYNC_W + HBP - 1);
	Ltdc_Handler.Init.AccumulatedVBP = (VSYNC_W + VBP - 1);
	Ltdc_Handler.Init.AccumulatedActiveW = (HSYNC_W + HBP + Width - 1);
	Ltdc_Handler.Init.AccumulatedActiveH = (VSYNC_W + VBP + Height - 1);
	Ltdc_Handler.Init.TotalWidth = (Width + HSYNC_W + HBP + HFP - 1); ;
	Ltdc_Handler.Init.TotalHeigh = (Height + VSYNC_W + VBP + VFP - 1);

	HAL_LTDC_Init(&Ltdc_Handler);
}



/*
*********************************************************************************************************
*	函 数 名: LCD429_ConfigLTDC
*	功能说明: 配置LTDC
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void LCD429_ConfigLTDC(void)
{
	RCC_PeriphCLKInitTypeDef Periph_ClkStructure = {0};
	/*使能时钟*/
	__HAL_RCC_LTDC_CLK_ENABLE();
	__HAL_RCC_DMA2D_CLK_ENABLE();

	LCD429_AF_GPIOConfig();

	Ltdc_Handler.Instance = LTDC;
	/*配置信号极性*/
	Ltdc_Handler.Init.HSPolarity = LTDC_HSPOLARITY_AL;
	Ltdc_Handler.Init.VSPolarity = LTDC_VSPOLARITY_AL;
	Ltdc_Handler.Init.DEPolarity = LTDC_DEPOLARITY_AL;
	Ltdc_Handler.Init.PCPolarity = LTDC_PCPOLARITY_IPC;

	/*背景色配置*/
	Ltdc_Handler.Init.Backcolor.Blue = 0;
	Ltdc_Handler.Init.Backcolor.Green = 0;
	Ltdc_Handler.Init.Backcolor.Red = 0;

	/* 
	   LTDC时钟配置说明：(看数据手册)
	     函数RCC_PLLSAIConfig的第一个参数是PLLSAI_N，第三个参数数PLLSAI_R。
	     函数RCC_LTDCCLKDivConfig的参数是RCC_PLLSAIDivR。
	   
	   下面举一个例子：PLLSAI_N = 400， PLLSAI_R = 4  RCC_PLLSAIDivR = 2:
	     首先，输入时钟 PLLSAI_VCO Input = HSE_VALUE / PLL_M = 8M / 8 = 1MHz 
	       输出时钟 PLLSAI_VCO Output  = PLLSAI_VCO Input * PLLSAI_N = 1 * 400 = 400 1MHz 
	       PLLLCDCLK = PLLSAI_VCO Output / PLLSAI_R = 400 / 4 = 100 1MHz 
	     最好，LTDC 时钟 = PLLLCDCLK / RCC_PLLSAIDivR = 100 / 2 = 50 1MHz 
	 */

	/* 配置输出时钟为 15Mhz*/
	Periph_ClkStructure.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
	Periph_ClkStructure.PLLSAI.PLLSAIN = 420;
	Periph_ClkStructure.PLLSAI.PLLSAIR = 7;
	Periph_ClkStructure.PLLSAIDivR = RCC_PLLSAIDIVR_4;
	HAL_RCCEx_PeriphCLKConfig(&Periph_ClkStructure);

	Width = 800;
	Height = 480;

	HSYNC_W = 48;
	HBP = 88;
	HFP = 40;

	VSYNC_W = 3;
	VBP = 32;
	VFP = 13;
	
	g_LcdWidth = Width;
	g_LcdHeight = Height;

	Ltdc_Handler.LayerCfg->ImageWidth = Width;
	Ltdc_Handler.LayerCfg->ImageHeight = Height;
	
	/*配置LTDC 的同步时序*/
	Ltdc_Handler.Init.HorizontalSync = (HSYNC_W - 1);
	Ltdc_Handler.Init.VerticalSync = (VSYNC_W - 1);
	Ltdc_Handler.Init.AccumulatedHBP = (HSYNC_W + HBP - 1);
	Ltdc_Handler.Init.AccumulatedVBP = (VSYNC_W + VBP - 1);
	Ltdc_Handler.Init.AccumulatedActiveW = (HSYNC_W + HBP + Width - 1);
	Ltdc_Handler.Init.AccumulatedActiveH = (VSYNC_W + VBP + Height - 1);
	Ltdc_Handler.Init.TotalWidth = (Width + HSYNC_W + HBP + HFP - 1); ;
	Ltdc_Handler.Init.TotalHeigh = (Height + VSYNC_W + VBP + VFP - 1);

	HAL_LTDC_Init(&Ltdc_Handler);


	//LCD429_LayerInit();  展开此函数
	{


		LTDC_LayerCfgTypeDef LTDC_Layer_InitStruct = {0};

		LTDC_Layer_InitStruct.WindowX0 = 0;
		LTDC_Layer_InitStruct.WindowX1 = Width;
		LTDC_Layer_InitStruct.WindowY0 = 0; 
		LTDC_Layer_InitStruct.WindowY1 = Height;

		/* Pixel Format configuration*/
		LTDC_Layer_InitStruct.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
		/* Alpha constant (255 totally opaque) */
		LTDC_Layer_InitStruct.Alpha = 255;
		//默认的透明度常量
		LTDC_Layer_InitStruct.Alpha0 = 0;
		/* Default Color configuration (configure A,R,G,B component values) */
		LTDC_Layer_InitStruct.Backcolor.Blue = 0;
		LTDC_Layer_InitStruct.Backcolor.Green = 0;
		LTDC_Layer_InitStruct.Backcolor.Red = 0;
		
		/* Configure blending factors */
		LTDC_Layer_InitStruct.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
		LTDC_Layer_InitStruct.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;

		LTDC_Layer_InitStruct.ImageWidth = Width;
		LTDC_Layer_InitStruct.ImageHeight = Height;
		/* the length of one line of pixels in bytes + 3 then :
		Line Lenth = Active high width x number of bytes per pixel + 3
		Active high width         = LCD429_PIXEL_WIDTH
		number of bytes per pixel = 2    (pixel_format : RGB565)
		*/
		//LTDC_Layer_InitStruct.LTDC_CFBLineLength = ((Width * 2) + 3);
		/* the pitch is the increment from the start of one line of pixels to the
		start of the next line in bytes, then :
		Pitch = Active high width x number of bytes per pixel
		*/
		//LTDC_Layer_InitStruct.LTDC_CFBPitch = (Height * 2);

		/* Configure the number of lines */
		//LTDC_Layer_InitStruct.LTDC_CFBLineNumber = 	Width;	/*　此处需要填写宽度值?  */
		
		/* Start Address configuration : the LCD Frame buffer is defined on SDRAM */
		LTDC_Layer_InitStruct.FBStartAdress = LCD429_FRAME_BUFFER;

		HAL_LTDC_ConfigLayer(&Ltdc_Handler, &LTDC_Layer_InitStruct,LTDC_LAYER_1);

#if 0
		/* Configure Layer2 */
		/* Start Address configuration : the LCD Frame buffer is defined on SDRAM w/ Offset */
		LTDC_Layer_InitStruct.FBStartAdress = LCD429_FRAME_BUFFER + BUFFER_OFFSET;

		/* Configure blending factors */
		LTDC_Layer_InitStruct.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
		LTDC_Layer_InitStruct.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;

		HAL_LTDC_ConfigLayer(&Ltdc_Handler, &LTDC_Layer_InitStruct,LTDC_Layer2);

		__HAL_LTDC_RELOAD_CONFIG(&Ltdc_Handler);   //
//		LTDC_ReloadConfig(LTDC_IMReload);
//
//		/* Enable foreground & background Layers */
//		LTDC_LayerCmd(LTDC_Layer1, ENABLE);
//		LTDC_LayerCmd(LTDC_Layer2, ENABLE);
//		LTDC_ReloadConfig(LTDC_IMReload);
#endif		
	}
}


/*
*********************************************************************************************************
*	函 数 名: LCD429_InitHard
*	功能说明: 初始化LCD
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_InitHard(void)
{
	LCD429_ConfigLTDC();			/* 配置429 CPU内部LTDC */

	LCD429_InitDMA2D();             /* 使能DMA2D*/

	LCD429_SetLayer(LCD_LAYER_1);

	LCD429_QuitWinMode();
}



/*
*********************************************************************************************************
*	函 数 名: LCD429_InitDMA2D
*	功能说明: 配置DMA2D
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void LCD429_InitDMA2D(void)
{
	/* 使能DMA2D时钟 */
	__HAL_RCC_DMA2D_CLK_ENABLE();   
	
	/* 配置默认模式 */ 
	hdma2d.Init.Mode         = DMA2D_R2M;
	hdma2d.Init.ColorMode    = DMA2D_INPUT_RGB565;
	hdma2d.Init.OutputOffset = 0x0;     

	hdma2d.Instance          = DMA2D; 

	if (HAL_DMA2D_Init(&hdma2d) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}
 }

/*
*********************************************************************************************************
*	函 数 名: LCD_LL_GetPixelformat
*	功能说明: 获取图层的颜色格式
*	形    参: 无
*	返 回 值: 返回相应图层的颜色格式
*********************************************************************************************************
*/
static inline uint32_t LCD_LL_GetPixelformat(uint32_t LayerIndex)
{
	if (LayerIndex == 0)
	{
		return LTDC_PIXEL_FORMAT_RGB565;
	} 
	else
	{
		return LTDC_PIXEL_FORMAT_RGB565;  // LTDC_PIXEL_FORMAT_ARGB1555;
	} 
}

/*
*********************************************************************************************************
*	函 数 名: DMA2D_CopyBuffer
*	功能说明: 通过DMA2D从前景层复制指定区域的颜色数据到目标区域
*	形    参: LayerIndex    图层
*             pSrc          颜色数据源地址
*             pDst          颜色数据目的地址
*             xSize         要复制区域的X轴大小，即每行像素数
*             ySize         要复制区域的Y轴大小，即行数
*             OffLineSrc    前景层图像的行偏移
*             OffLineDst    输出的行偏移
*	返 回 值: 无
*********************************************************************************************************
*/
void DMA2D_CopyBuffer(uint32_t LayerIndex, void * pSrc, void * pDst, uint32_t xSize, uint32_t ySize, uint32_t OffLineSrc, uint32_t OffLineDst)
{
	uint32_t PixelFormat;

	PixelFormat = LCD_LL_GetPixelformat(LayerIndex);
	DMA2D->CR      = 0x00000000UL | (1 << 9);  

	/* 设置基本参数 */
	DMA2D->FGMAR   = (uint32_t)pSrc;                       
	DMA2D->OMAR    = (uint32_t)pDst;                       
	DMA2D->FGOR    = OffLineSrc;                      
	DMA2D->OOR     = OffLineDst; 

	/* 设置颜色格式 */  
	DMA2D->FGPFCCR = PixelFormat;  

	/*  设置传输大小 */
	DMA2D->NLR     = (uint32_t)(xSize << 16) | (uint16_t)ySize; 

	DMA2D->CR     |= DMA2D_CR_START;   

	/* 等待传输完成 */
	while (DMA2D->CR & DMA2D_CR_START) 
	{
	}
}

/*
*********************************************************************************************************
*	函 数 名: _LCD_DrawBitmap16bpp
*	功能说明: 16bpp位图绘制，专用于摄像头
*	形    参: --
*	返 回 值: 无
*********************************************************************************************************
*/
void _LCD_DrawCamera16bpp(int x, int y, uint16_t * p, int xSize, int ySize, int SrcOffLine) 
{
	uint32_t  AddrDst;
	int OffLineSrc, OffLineDst;

	AddrDst =s_CurrentFrameBuffer + (y * g_LcdWidth + x) * 2;
	OffLineSrc = SrcOffLine; 		/* 源图形的偏移，这里是全部都是用了 */
	OffLineDst = g_LcdWidth - xSize;/* 目标图形的便宜 */
	DMA2D_CopyBuffer(LCD_LAYER_1, (void *)p, (void *)AddrDst, xSize, ySize, OffLineSrc, OffLineDst);	
}

/*
*********************************************************************************************************
*	函 数 名: _DMA_Fill
*	功能说明: 通过DMA2D对于指定区域进行颜色填充
*	形    参: LayerIndex    图层
*             pDst          颜色数据目的地址
*             xSize         要复制区域的X轴大小，即每行像素数
*             ySize         要复制区域的Y轴大小，即行数
*             OffLine       前景层图像的行偏移
*             ColorIndex    要填充的颜色值
*	返 回 值: 无
*********************************************************************************************************
*/
static void DMA2D_FillBuffer(uint32_t LayerIndex, void * pDst, uint32_t xSize, uint32_t ySize, uint32_t OffLine, uint32_t ColorIndex) 
{
	uint32_t PixelFormat;

	PixelFormat = LCD_LL_GetPixelformat(LayerIndex);

	/* 颜色填充 */
	DMA2D->CR      = 0x00030000UL | (1 << 9);        
	DMA2D->OCOLR   = ColorIndex;                     

	/* 设置填充的颜色目的地址 */
	DMA2D->OMAR    = (uint32_t)pDst;                      

	/* 目的行偏移地址 */
	DMA2D->OOR     = OffLine;                        

	/* 设置颜色格式 */
	DMA2D->OPFCCR  = PixelFormat;                    

	/* 设置填充大小 */
	DMA2D->NLR     = (uint32_t)(xSize << 16) | (uint16_t)ySize;

	DMA2D->CR     |= DMA2D_CR_START; 

	/* 等待DMA2D传输完成 */
	while (DMA2D->CR & DMA2D_CR_START) 
	{
	}
}

#if 0
void LCD429_test(void)
{

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

	bsp_StartAutoTimer(0, 200); /* 启动1个200ms的自动重装的定时器，软件定时器0 */
	
	while (1)
	{
		/* 判断软件定时器0是否超时 */
		if(bsp_CheckTimer(0))
		{
			/* 每隔200ms 进来一次 */  
			bsp_LedToggle(2);
			
			sprintf((char *)buf, "count = %03d", count++);
			LCD_DispStr(5, 90, (char *)buf, &tFont16); 
			vTaskDelay(200);
		}
	}

}
#endif

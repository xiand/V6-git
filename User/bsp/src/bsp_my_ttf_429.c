#include "bsp.h"


typedef uint32_t LCD_COLOR;

#define LCD429_FRAME_BUFFER EXT_SDRAM_ADDR

/* ƫ�Ƶ�ַ���㹫ʽ::
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
*	�� �� ��: LCD429_GetChipDescribe
*	����˵��: ��ȡLCD����оƬ���������ţ�������ʾ
*	��    ��: char *_str : �������ַ�������˻�����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD429_GetChipDescribe(char *_str)
{
	strcpy(_str, "STM32F429");
}

/*
*********************************************************************************************************
*	�� �� ��: LCD429_SetLayer
*	����˵��: �л��㡣ֻ�Ǹ��ĳ���������Ա��ں���Ĵ��������ؼĴ�����Ӳ��֧��2�㡣
*	��    ��: ��
*	�� �� ֵ: ��
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
*	�� �� ��: LCD429_SetTransparency
*	����˵��: ���õ�ǰ���͸������
*	��    ��: ͸���ȣ� ֵ�� 0x00 - 0xFF
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD429_SetTransparency(uint8_t transparency)
{
	HAL_LTDC_SetAlpha(&Ltdc_Handler, transparency, s_CurrentLayer);	/* ����ˢ�� */
}


/*
*********************************************************************************************************
*	�� �� ��: LCD429_SetPixelFormat
*	����˵��: ���õ�ǰ������ظ�ʽ
*	��    ��: ���ظ�ʽ:
*                      LTDC_PIXEL_FORMAT_ARGB8888
*                      LTDC_PIXEL_FORMAT_ARGB8888_RGB888
*                      LTDC_PIXEL_FORMAT_ARGB8888_RGB565
*                      LTDC_PIXEL_FORMAT_ARGB8888_ARGB1555
*                      LTDC_PIXEL_FORMAT_ARGB8888_ARGB4444
*                      LTDC_PIXEL_FORMAT_ARGB8888_L8
*                      LTDC_PIXEL_FORMAT_ARGB8888_AL44
*                      LTDC_PIXEL_FORMAT_ARGB8888_AL88
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD429_SetPixelFormat(uint32_t PixelFormat)
{
	HAL_LTDC_SetPixelFormat(&Ltdc_Handler, PixelFormat, s_CurrentLayer);
}


/*
*********************************************************************************************************
*	�� �� ��: LCD429_SetDispWin
*	����˵��: ������ʾ���ڣ����봰����ʾģʽ��
*	��    ��:  
*		_usX : ˮƽ����
*		_usY : ��ֱ����
*		_usHeight: ���ڸ߶�
*		_usWidth : ���ڿ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD429_SetDispWin(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth)
{
	HAL_LTDC_SetWindowSize_NoReload(&Ltdc_Handler, _usWidth, _usHeight, s_CurrentLayer);	/* ���������� */
	HAL_LTDC_SetWindowPosition(&Ltdc_Handler, _usX, _usY, s_CurrentLayer);		/* �������� */
}


/*
*********************************************************************************************************
*	�� �� ��: LCD429_QuitWinMode
*	����˵��: �˳�������ʾģʽ����Ϊȫ����ʾģʽ
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD429_QuitWinMode(void)
{
	HAL_LTDC_SetWindowSize_NoReload(&Ltdc_Handler, g_LcdWidth, g_LcdHeight, s_CurrentLayer);	/* ���������� */
	HAL_LTDC_SetWindowPosition(&Ltdc_Handler, 0, 0, s_CurrentLayer);		/* �������� */	
}

/*
*********************************************************************************************************
*	�� �� ��: LCD429_DispOn
*	����˵��: ����ʾ
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD429_DispOn(void)
{

}

/*
*********************************************************************************************************
*	�� �� ��: LCD429_DispOff
*	����˵��: �ر���ʾ
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD429_DispOff(void)
{

}


/*
*********************************************************************************************************
*	�� �� ��: LCD429_ClrScr
*	����˵��: �����������ɫֵ����
*	��    ��: _usColor : ����ɫ
*	�� �� ֵ: ��
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
*	�� �� ��: LCD429_PutPixel
*	����˵��: ��1������
*	��    ��:
*			_usX,_usY : ��������
*			_usColor  : ������ɫ ( RGB = 565 ��ʽ)
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD429_PutPixel(uint16_t _usX, uint16_t _usY, uint16_t _usColor)
{
	uint16_t *p;
	uint32_t index = 0;
	
	p = (uint16_t *)s_CurrentFrameBuffer;
		
	if (g_LcdDirection == 0)		/* ���� */
	{
		index = (uint32_t)_usY * g_LcdWidth + _usX;
	}
	else if (g_LcdDirection == 1)	/* ����180��*/
	{
		index = (uint32_t)(g_LcdHeight - _usY - 1) * g_LcdWidth + (g_LcdWidth - _usX - 1);
	}
	else if (g_LcdDirection == 2)	/* ���� */
	{
		index = (uint32_t)(g_LcdWidth - _usX - 1) * g_LcdHeight + _usY;
	}
	else if (g_LcdDirection == 3)	/* ����180�� */
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
*	�� �� ��: LCD429_GetPixel
*	����˵��: ��ȡ1������
*	��    ��:
*			_usX,_usY : ��������
*			_usColor  : ������ɫ
*	�� �� ֵ: RGB��ɫֵ
*********************************************************************************************************
*/
uint16_t LCD429_GetPixel(uint16_t _usX, uint16_t _usY)
{
	uint16_t usRGB;
	uint16_t *p;
	uint32_t index = 0;
	
	p = (uint16_t *)s_CurrentFrameBuffer;

	if (g_LcdDirection == 0)		/* ���� */
	{
		index = (uint32_t)_usY * g_LcdWidth + _usX;
	}
	else if (g_LcdDirection == 1)	/* ����180��*/
	{
		index = (uint32_t)(g_LcdHeight - _usY - 1) * g_LcdWidth + (g_LcdWidth - _usX - 1);
	}
	else if (g_LcdDirection == 2)	/* ���� */
	{
		index = (uint32_t)(g_LcdWidth - _usX - 1) * g_LcdHeight + _usY;
	}
	else if (g_LcdDirection == 3)	/* ����180�� */
	{
		index = (uint32_t)_usX * g_LcdHeight + g_LcdHeight - _usY - 1;
	}
	
	usRGB = p[index];

	return usRGB;
}



/*
*********************************************************************************************************
*	�� �� ��: LCD429_DrawLine
*	����˵��: ���� Bresenham �㷨����2��仭һ��ֱ�ߡ�
*	��    ��:
*			_usX1, _usY1 : ��ʼ������
*			_usX2, _usY2 : ��ֹ��Y����
*			_usColor     : ��ɫ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD429_DrawLine(uint16_t _usX1 , uint16_t _usY1 , uint16_t _usX2 , uint16_t _usY2 , uint16_t _usColor)
{
	int32_t dx , dy ;
	int32_t tx , ty ;
	int32_t inc1 , inc2 ;
	int32_t d , iTag ;
	int32_t x , y ;

	/* ���� Bresenham �㷨����2��仭һ��ֱ�� */

	LCD429_PutPixel(_usX1 , _usY1 , _usColor);

	/* ��������غϣ���������Ķ�����*/
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

	if ( dx < dy )   /*���dyΪ�Ƴ������򽻻��ݺ����ꡣ*/
	{
		uint16_t temp;

		iTag = 1 ;
		temp = _usX1; _usX1 = _usY1; _usY1 = temp;
		temp = _usX2; _usX2 = _usY2; _usY2 = temp;
		temp = dx; dx = dy; dy = temp;
	}
	tx = _usX2 > _usX1 ? 1 : -1 ;    /* ȷ������1���Ǽ�1 */
	ty = _usY2 > _usY1 ? 1 : -1 ;
	x = _usX1 ;
	y = _usY1 ;
	inc1 = 2 * dy ;
	inc2 = 2 * ( dy - dx );
	d = inc1 - dx ;
	while ( x != _usX2 )     /* ѭ������ */
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
*	�� �� ��: LCD429_DrawHLine
*	����˵��: ����һ��ˮƽ��. ʹ��STM32F429 DMA2DӲ������.
*	��    ��:
*			_usX1, _usY1 : ��ʼ������
*			_usLen       : �ߵĳ���
*			_usColor     : ��ɫ
*	�� �� ֵ: ��
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
*	�� �� ��: LCD429_DrawVLine
*	����˵��: ����һ����ֱ�ߡ� ʹ��STM32F429 DMA2DӲ������.
*	��    ��:
*			_usX, _usY : ��ʼ������
*			_usLen       : �ߵĳ���
*			_usColor     : ��ɫ
*	�� �� ֵ: ��
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
*	�� �� ��: LCD429_DrawPoints
*	����˵��: ���� Bresenham �㷨������һ��㣬������Щ�����������������ڲ�����ʾ��
*	��    ��:
*			x, y     : ��������
*			_usColor : ��ɫ
*	�� �� ֵ: ��
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
*	�� �� ��: LCD429_DrawRect
*	����˵��: ����ˮƽ���õľ��Ρ�
*	��    ��:
*			_usX,_usY: �������Ͻǵ�����
*			_usHeight : ���εĸ߶�
*			_usWidth  : ���εĿ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD429_DrawRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor)
{
	/*
	 ---------------->---
	|(_usX��_usY)        |
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
*	�� �� ��: LCD429_FillRect
*	����˵��: ��һ����ɫֵ���һ�����Ρ�ʹ��STM32F429�ڲ�DMA2DӲ�����ơ�
*	��    ��:
*			_usX,_usY: �������Ͻǵ�����
*			_usHeight : ���εĸ߶�
*			_usWidth  : ���εĿ��
*			_usColor  : ��ɫ����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD429_FillRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor)
{
#if 1	/* ���⺯��չ�������ִ��Ч�� */
	uint32_t  Xaddress = 0;
	uint16_t  OutputOffset = 0;
	uint16_t  NumberOfLine = 0;
	uint16_t  PixelPerLine = 0;	

	/* ������ʾ�������ò�ͬ�Ĳ��� */
	if (g_LcdDirection == 0)		/* ���� */
	{
		Xaddress = s_CurrentFrameBuffer + 2 * (g_LcdWidth * _usY + _usX);	
		OutputOffset = g_LcdWidth - _usWidth;
		NumberOfLine = _usHeight;
		PixelPerLine = _usWidth;
	}
	else if (g_LcdDirection == 1)	/* ����180��*/
	{
		Xaddress = s_CurrentFrameBuffer + 2 * (g_LcdWidth * (g_LcdHeight - _usHeight - _usY) + g_LcdWidth - _usX - _usWidth);	
		OutputOffset = g_LcdWidth - _usWidth;
		NumberOfLine = _usHeight;
		PixelPerLine = _usWidth;
	}
	else if (g_LcdDirection == 2)	/* ���� */
	{
		Xaddress = s_CurrentFrameBuffer + 2 * (g_LcdHeight * (g_LcdWidth - _usX -  _usWidth) + _usY);	
		OutputOffset = g_LcdHeight - _usHeight;
		NumberOfLine = _usWidth;
		PixelPerLine = _usHeight;
	}
	else if (g_LcdDirection == 3)	/* ����180�� */
	{
		Xaddress = s_CurrentFrameBuffer + 2 * (g_LcdHeight * _usX + g_LcdHeight - _usHeight - _usY);	
		OutputOffset = g_LcdHeight - _usHeight;
		NumberOfLine = _usWidth;
		PixelPerLine = _usHeight;		
	}	

	DMA2D_FillBuffer(s_CurrentLayer, (void *)Xaddress, PixelPerLine, NumberOfLine, OutputOffset, _usColor);
#endif

#if 0	/* ʹ�ÿ⺯�� -- ������� */
	/* ʹ��DMA2DӲ�������� */
	DMA2D_InitTypeDef      DMA2D_InitStruct;
	uint32_t  Xaddress = 0;
	uint16_t  OutputOffset;
	uint16_t  NumberOfLine;
	uint16_t  PixelPerLine;	
		
	if (g_LcdDirection == 0)		/* ���� */
	{
		Xaddress = s_CurrentFrameBuffer + 2 * (g_LcdWidth * _usY + _usX);	
		OutputOffset = g_LcdWidth - _usWidth;
		NumberOfLine = _usHeight;
		PixelPerLine = _usWidth;
	}
	else if (g_LcdDirection == 1)	/* ����180��*/
	{
		Xaddress = s_CurrentFrameBuffer + 2 * (g_LcdWidth * (g_LcdHeight - _usHeight - _usY) + g_LcdWidth - _usX - _usWidth);	
		OutputOffset = g_LcdWidth - _usWidth;
		NumberOfLine = _usHeight;
		PixelPerLine = _usWidth;
	}
	else if (g_LcdDirection == 2)	/* ���� */
	{
		Xaddress = s_CurrentFrameBuffer + 2 * (g_LcdHeight * (g_LcdWidth - _usX -  _usWidth) + _usY);	
		OutputOffset = g_LcdHeight - _usHeight;
		NumberOfLine = _usWidth;
		PixelPerLine = _usHeight;
	}
	else if (g_LcdDirection == 3)	/* ����180�� */
	{
		Xaddress = s_CurrentFrameBuffer + 2 * (g_LcdHeight * _usX + g_LcdHeight - _usHeight - _usY);	
		OutputOffset = g_LcdHeight - _usHeight;
		NumberOfLine = _usWidth;
		PixelPerLine = _usHeight;		
	}	

	/* ���� DMA2D */
	DMA2D_DeInit();		/* ��λ DMA2D */
	DMA2D_InitStruct.DMA2D_Mode = DMA2D_R2M;       /* ����ģʽ: �Ĵ������洢�� */
	DMA2D_InitStruct.DMA2D_CMode = DMA2D_RGB565;   /* ��ɫģʽ�� RGB565 */   
	
	DMA2D_InitStruct.DMA2D_OutputRed = RGB565_R2(_usColor);		/* ��ɫ������5bit����λΪ0 */           
	DMA2D_InitStruct.DMA2D_OutputGreen = RGB565_G2(_usColor);	/* ��ɫ������6bit����λΪ0 */
	DMA2D_InitStruct.DMA2D_OutputBlue = RGB565_B2(_usColor); 	/* ��ɫ������5bit����λΪ0 */    
	
	DMA2D_InitStruct.DMA2D_OutputAlpha = 0x0F;    				/* ͸�������� ����565��ʽ�������� */              
	DMA2D_InitStruct.DMA2D_OutputMemoryAdd = Xaddress;      	/* Ŀ���ڴ��ַ */          
	DMA2D_InitStruct.DMA2D_OutputOffset = OutputOffset; 	/* ÿ�м���Դ����ص�ַ��ֵ  */               
	DMA2D_InitStruct.DMA2D_NumberOfLine = NumberOfLine;     /* һ���м��� */        
	DMA2D_InitStruct.DMA2D_PixelPerLine = PixelPerLine;		/* ÿ�м������� */
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
*	�� �� ��: LCD429_DrawCircle
*	����˵��: ����һ��Բ���ʿ�Ϊ1������
*	��    ��:
*			_usX,_usY  : Բ�ĵ�����
*			_usRadius  : Բ�İ뾶
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD429_DrawCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor)
{
	int32_t  D;			/* Decision Variable */
	uint32_t  CurX;		/* ��ǰ X ֵ */
	uint32_t  CurY;		/* ��ǰ Y ֵ */

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
*	�� �� ��: LCD429_DrawBMP
*	����˵��: ��LCD����ʾһ��BMPλͼ��λͼ����ɨ����򣺴����ң����ϵ���
*	��    ��:  
*			_usX, _usY : ͼƬ������
*			_usHeight  ��ͼƬ�߶�
*			_usWidth   ��ͼƬ���
*			_ptr       ��ͼƬ����ָ��
*	�� �� ֵ: ��
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
*	�� �� ��: LCD429_SetDirection
*	����˵��: ������ʾ����ʾ���򣨺��� ������
*	��    ��: ��ʾ������� 0 ��������, 1=����180�ȷ�ת, 2=����, 3=����180�ȷ�ת
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD429_SetDirection(uint8_t _dir)
{
	uint16_t temp;
	
	if (_dir == 0 || _dir == 1)		/* ������ ����180�� */
	{
		if (g_LcdWidth < g_LcdHeight)
		{
			temp = g_LcdWidth;
			g_LcdWidth = g_LcdHeight;
			g_LcdHeight = temp;			
		}
	}
	else if (_dir == 2 || _dir == 3)	/* ����, ����180��*/
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
*	�� �� ��: LCD429_BeginDraw
*	����˵��: ˫����������ģʽ����ʼ��ͼ������ǰ��ʾ�������������������Ƶ�����һ����������
*			 ����� LCD429_EndDraw�����ɶ�ʹ�á� ʵ��Ч�������á�
*	��    ��: ��
*	�� �� ֵ: ��
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
*	�� �� ��: LCD429_EndDraw
*	����˵��: APP�����˻�������ͼ�������л�Ӳ����ʾ��
*			 ����� LCD429_BeginDraw�����ɶ�ʹ�á�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD429_EndDraw(void)
{
	//HAL_LTDC_SetAddress(Ltdc_Handler, s_CurrentFrameBuffer, LTDC_Layer1);
}

/*
*********************************************************************************************************
*	�� �� ��: _GetBufferSize
*	����˵��: ��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
uint32_t _GetBufferSize(uint8_t LayerIndex)
{
	return g_LcdWidth * g_LcdHeight;
}



/*
*********************************************************************************************************
*	�� �� ��: _DMA_Copy
*	����˵��: ����ͼ��
*	��    ��: 
*			 pSrc : Դ�ڴ��ַ
*			 pDst : Ŀ���ڴ��ַ
*			 xSize : ����x�ߴ�
*			 ySize : ����y�ߴ�
*			 OffLineSrc : Դͼ����ƫ��
*			 OffLineDst : Ŀ��ͼ����ƫ��
*	�� �� ֵ: ��
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
*	                                 ����ĺ�����emWin������
*********************************************************************************************************
*/
/*
*********************************************************************************************************
*	�� �� ��: LCD_ConfigLTDC
*	����˵��: ����LTDC
*	��    ��: ��
*	�� �� ֵ: ��
*   ��    ��:
*       LCD_TFT ͬ��ʱ�����ã������Թٷ�����һ����ͼ���Լ����ࣩ��
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
*     ÿ��LCD�豸�����Լ���ͬ��ʱ��ֵ��
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
*     LCD_TFT ����ˮƽ�ʹ�ֱ����ʼ�Լ�����λ�� :
*     ----------------------------------------------------------------
*   
*     HorizontalStart = (Offset_X + Hsync + HBP);
*     HorizontalStop  = (Offset_X + Hsync + HBP + Window_Width - 1); 
*     VarticalStart   = (Offset_Y + Vsync + VBP);
*     VerticalStop    = (Offset_Y + Vsync + VBP + Window_Heigh - 1);

*     �����Ĳ���ֵ�����Բο�Ұ�������˵��
*
*********************************************************************************************************
*/

__IO uint16_t Width,Height, HSYNC_W, VSYNC_W, HBP, HFP, VBP, VFP;
void LCD_ConfigLTDC(void)
{
	
	RCC_PeriphCLKInitTypeDef Periph_ClkStructure = {0};
	/*ʹ��ʱ��*/
	__HAL_RCC_LTDC_CLK_ENABLE();

	LCD429_AF_GPIOConfig();

	Ltdc_Handler.Instance = LTDC;
	/*�����źż���*/
	Ltdc_Handler.Init.HSPolarity = LTDC_HSPOLARITY_AL;
	Ltdc_Handler.Init.VSPolarity = LTDC_VSPOLARITY_AL;
	Ltdc_Handler.Init.DEPolarity = LTDC_DEPOLARITY_AL;
	Ltdc_Handler.Init.PCPolarity = LTDC_PCPOLARITY_IPC;

	/*����ɫ����*/
	Ltdc_Handler.Init.Backcolor.Blue = 0;
	Ltdc_Handler.Init.Backcolor.Green = 0;
	Ltdc_Handler.Init.Backcolor.Red = 0;

	/* 
	   LTDCʱ������˵����(�������ֲ�)
	     ����RCC_PLLSAIConfig�ĵ�һ��������PLLSAI_N��������������PLLSAI_R��
	     ����RCC_LTDCCLKDivConfig�Ĳ�����RCC_PLLSAIDivR��
	   
	   �����һ�����ӣ�PLLSAI_N = 400�� PLLSAI_R = 4  RCC_PLLSAIDivR = 2:
	     ���ȣ�����ʱ�� PLLSAI_VCO Input = HSE_VALUE / PLL_M = 8M / 8 = 1MHz 
	       ���ʱ�� PLLSAI_VCO Output  = PLLSAI_VCO Input * PLLSAI_N = 1 * 400 = 400 1MHz 
	       PLLLCDCLK = PLLSAI_VCO Output / PLLSAI_R = 400 / 4 = 100 1MHz 
	     ��ã�LTDC ʱ�� = PLLLCDCLK / RCC_PLLSAIDivR = 100 / 2 = 50 1MHz 
	 */

	/* �������ʱ��Ϊ 15Mhz*/
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
	
	/*����LTDC ��ͬ��ʱ��*/
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
*	�� �� ��: LCD429_ConfigLTDC
*	����˵��: ����LTDC
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void LCD429_ConfigLTDC(void)
{
	RCC_PeriphCLKInitTypeDef Periph_ClkStructure = {0};
	/*ʹ��ʱ��*/
	__HAL_RCC_LTDC_CLK_ENABLE();
	__HAL_RCC_DMA2D_CLK_ENABLE();

	LCD429_AF_GPIOConfig();

	Ltdc_Handler.Instance = LTDC;
	/*�����źż���*/
	Ltdc_Handler.Init.HSPolarity = LTDC_HSPOLARITY_AL;
	Ltdc_Handler.Init.VSPolarity = LTDC_VSPOLARITY_AL;
	Ltdc_Handler.Init.DEPolarity = LTDC_DEPOLARITY_AL;
	Ltdc_Handler.Init.PCPolarity = LTDC_PCPOLARITY_IPC;

	/*����ɫ����*/
	Ltdc_Handler.Init.Backcolor.Blue = 0;
	Ltdc_Handler.Init.Backcolor.Green = 0;
	Ltdc_Handler.Init.Backcolor.Red = 0;

	/* 
	   LTDCʱ������˵����(�������ֲ�)
	     ����RCC_PLLSAIConfig�ĵ�һ��������PLLSAI_N��������������PLLSAI_R��
	     ����RCC_LTDCCLKDivConfig�Ĳ�����RCC_PLLSAIDivR��
	   
	   �����һ�����ӣ�PLLSAI_N = 400�� PLLSAI_R = 4  RCC_PLLSAIDivR = 2:
	     ���ȣ�����ʱ�� PLLSAI_VCO Input = HSE_VALUE / PLL_M = 8M / 8 = 1MHz 
	       ���ʱ�� PLLSAI_VCO Output  = PLLSAI_VCO Input * PLLSAI_N = 1 * 400 = 400 1MHz 
	       PLLLCDCLK = PLLSAI_VCO Output / PLLSAI_R = 400 / 4 = 100 1MHz 
	     ��ã�LTDC ʱ�� = PLLLCDCLK / RCC_PLLSAIDivR = 100 / 2 = 50 1MHz 
	 */

	/* �������ʱ��Ϊ 15Mhz*/
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
	
	/*����LTDC ��ͬ��ʱ��*/
	Ltdc_Handler.Init.HorizontalSync = (HSYNC_W - 1);
	Ltdc_Handler.Init.VerticalSync = (VSYNC_W - 1);
	Ltdc_Handler.Init.AccumulatedHBP = (HSYNC_W + HBP - 1);
	Ltdc_Handler.Init.AccumulatedVBP = (VSYNC_W + VBP - 1);
	Ltdc_Handler.Init.AccumulatedActiveW = (HSYNC_W + HBP + Width - 1);
	Ltdc_Handler.Init.AccumulatedActiveH = (VSYNC_W + VBP + Height - 1);
	Ltdc_Handler.Init.TotalWidth = (Width + HSYNC_W + HBP + HFP - 1); ;
	Ltdc_Handler.Init.TotalHeigh = (Height + VSYNC_W + VBP + VFP - 1);

	HAL_LTDC_Init(&Ltdc_Handler);


	//LCD429_LayerInit();  չ���˺���
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
		//Ĭ�ϵ�͸���ȳ���
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
		//LTDC_Layer_InitStruct.LTDC_CFBLineNumber = 	Width;	/*���˴���Ҫ��д���ֵ?  */
		
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
*	�� �� ��: LCD429_InitHard
*	����˵��: ��ʼ��LCD
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD429_InitHard(void)
{
	LCD429_ConfigLTDC();			/* ����429 CPU�ڲ�LTDC */

	LCD429_InitDMA2D();             /* ʹ��DMA2D*/

	LCD429_SetLayer(LCD_LAYER_1);

	LCD429_QuitWinMode();
}



/*
*********************************************************************************************************
*	�� �� ��: LCD429_InitDMA2D
*	����˵��: ����DMA2D
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void LCD429_InitDMA2D(void)
{
	/* ʹ��DMA2Dʱ�� */
	__HAL_RCC_DMA2D_CLK_ENABLE();   
	
	/* ����Ĭ��ģʽ */ 
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
*	�� �� ��: LCD_LL_GetPixelformat
*	����˵��: ��ȡͼ�����ɫ��ʽ
*	��    ��: ��
*	�� �� ֵ: ������Ӧͼ�����ɫ��ʽ
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
*	�� �� ��: DMA2D_CopyBuffer
*	����˵��: ͨ��DMA2D��ǰ���㸴��ָ���������ɫ���ݵ�Ŀ������
*	��    ��: LayerIndex    ͼ��
*             pSrc          ��ɫ����Դ��ַ
*             pDst          ��ɫ����Ŀ�ĵ�ַ
*             xSize         Ҫ���������X���С����ÿ��������
*             ySize         Ҫ���������Y���С��������
*             OffLineSrc    ǰ����ͼ�����ƫ��
*             OffLineDst    �������ƫ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DMA2D_CopyBuffer(uint32_t LayerIndex, void * pSrc, void * pDst, uint32_t xSize, uint32_t ySize, uint32_t OffLineSrc, uint32_t OffLineDst)
{
	uint32_t PixelFormat;

	PixelFormat = LCD_LL_GetPixelformat(LayerIndex);
	DMA2D->CR      = 0x00000000UL | (1 << 9);  

	/* ���û������� */
	DMA2D->FGMAR   = (uint32_t)pSrc;                       
	DMA2D->OMAR    = (uint32_t)pDst;                       
	DMA2D->FGOR    = OffLineSrc;                      
	DMA2D->OOR     = OffLineDst; 

	/* ������ɫ��ʽ */  
	DMA2D->FGPFCCR = PixelFormat;  

	/*  ���ô����С */
	DMA2D->NLR     = (uint32_t)(xSize << 16) | (uint16_t)ySize; 

	DMA2D->CR     |= DMA2D_CR_START;   

	/* �ȴ�������� */
	while (DMA2D->CR & DMA2D_CR_START) 
	{
	}
}

/*
*********************************************************************************************************
*	�� �� ��: _LCD_DrawBitmap16bpp
*	����˵��: 16bppλͼ���ƣ�ר��������ͷ
*	��    ��: --
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void _LCD_DrawCamera16bpp(int x, int y, uint16_t * p, int xSize, int ySize, int SrcOffLine) 
{
	uint32_t  AddrDst;
	int OffLineSrc, OffLineDst;

	AddrDst =s_CurrentFrameBuffer + (y * g_LcdWidth + x) * 2;
	OffLineSrc = SrcOffLine; 		/* Դͼ�ε�ƫ�ƣ�������ȫ���������� */
	OffLineDst = g_LcdWidth - xSize;/* Ŀ��ͼ�εı��� */
	DMA2D_CopyBuffer(LCD_LAYER_1, (void *)p, (void *)AddrDst, xSize, ySize, OffLineSrc, OffLineDst);	
}

/*
*********************************************************************************************************
*	�� �� ��: _DMA_Fill
*	����˵��: ͨ��DMA2D����ָ�����������ɫ���
*	��    ��: LayerIndex    ͼ��
*             pDst          ��ɫ����Ŀ�ĵ�ַ
*             xSize         Ҫ���������X���С����ÿ��������
*             ySize         Ҫ���������Y���С��������
*             OffLine       ǰ����ͼ�����ƫ��
*             ColorIndex    Ҫ������ɫֵ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DMA2D_FillBuffer(uint32_t LayerIndex, void * pDst, uint32_t xSize, uint32_t ySize, uint32_t OffLine, uint32_t ColorIndex) 
{
	uint32_t PixelFormat;

	PixelFormat = LCD_LL_GetPixelformat(LayerIndex);

	/* ��ɫ��� */
	DMA2D->CR      = 0x00030000UL | (1 << 9);        
	DMA2D->OCOLR   = ColorIndex;                     

	/* ����������ɫĿ�ĵ�ַ */
	DMA2D->OMAR    = (uint32_t)pDst;                      

	/* Ŀ����ƫ�Ƶ�ַ */
	DMA2D->OOR     = OffLine;                        

	/* ������ɫ��ʽ */
	DMA2D->OPFCCR  = PixelFormat;                    

	/* ��������С */
	DMA2D->NLR     = (uint32_t)(xSize << 16) | (uint16_t)ySize;

	DMA2D->CR     |= DMA2D_CR_START; 

	/* �ȴ�DMA2D������� */
	while (DMA2D->CR & DMA2D_CR_START) 
	{
	}
}

#if 0
void LCD429_test(void)
{

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

	bsp_StartAutoTimer(0, 200); /* ����1��200ms���Զ���װ�Ķ�ʱ���������ʱ��0 */
	
	while (1)
	{
		/* �ж������ʱ��0�Ƿ�ʱ */
		if(bsp_CheckTimer(0))
		{
			/* ÿ��200ms ����һ�� */  
			bsp_LedToggle(2);
			
			sprintf((char *)buf, "count = %03d", count++);
			LCD_DispStr(5, 90, (char *)buf, &tFont16); 
			vTaskDelay(200);
		}
	}

}
#endif

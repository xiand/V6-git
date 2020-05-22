/*
*********************************************************************************************************
*
*	ģ������ : ��ʱ��ʱ��
*	�ļ����� : SysInfoTest.c
*	��    �� : V1.0
*	˵    �� : Ϊ�˻�ȡFreeRTOS��������Ϣ����Ҫ����һ����ʱ���������ʱ����ʱ���׼����Ҫ����
*              ϵͳʱ�ӽ��ġ������õ���������Ϣ��׼ȷ��
*              ���ļ��ṩ�ĺ��������ڲ���Ŀ�ģ��в��ɽ�������ʵ����Ŀ��ԭ�������㣺
*               1. FreeRTOS��ϵͳ�ں�û�ж��ܵļ���ʱ�������������
*               2. ��ʱ���ж���50us����һ�Σ��Ƚ�Ӱ��ϵͳ���ܡ�
*              --------------------------------------------------------------------------------------
*              ���ļ�ʹ�õ���32λ����������50usһ�εļ���ֵ�����֧�ּ���ʱ�䣺
*              2^32 * 50us / 3600s = 59.6���ӡ�ʹ���в��Ե��������м���������ռ���ʳ�����59.6���ӽ���׼ȷ��
*
*	�޸ļ�¼ :
*		�汾��    ����        ����     ˵��
*		V1.0    2015-08-19  Eric2013   �׷�
*
*	Copyright (C), 2015-2020, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"


/* ��ʱ��Ƶ�ʣ�50usһ���ж� */
#define  timerINTERRUPT_FREQUENCY	20000

/* �ж����ȼ� */
#define  timerHIGHEST_PRIORITY		2

/* ��ϵͳ���� */
volatile uint32_t ulHighFrequencyTimerTicks = 0UL;

/*
*********************************************************************************************************
*	�� �� ��: vSetupTimerTest
*	����˵��: ������ʱ��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void vSetupSysInfoTest(void)
{
	bsp_SetTIMforInt(TIM6, timerINTERRUPT_FREQUENCY, timerHIGHEST_PRIORITY, 0);
}

/*
*********************************************************************************************************
*	�� �� ��: TIM6_DAC_IRQHandler
*	����˵��: TIM6�жϷ������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void TIM6_DAC_IRQHandler(void)
{
	if((TIM6->SR & TIM_FLAG_UPDATE) != RESET)
	{
		TIM6->SR = ~ TIM_FLAG_UPDATE;
		ulHighFrequencyTimerTicks++;
	}
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
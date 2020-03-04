

#include "bsp.h"


/* SDRAM��ʱ */
#define SDRAM_TIMEOUT     ((uint32_t)0xFFFF)

/* FMC SDRAM Mode definition register defines */
#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_1              ((uint16_t)0x0010)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)

static void SDRAM_GPIO_Config(void);
static void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram);


void bsp_InitExitSDRAM(void)
{
	SDRAM_HandleTypeDef hsdram1 = {0};
	FMC_SDRAM_TimingTypeDef FMC_SDRAMTimingInitStructure = {0};	

	/*GPIO configure for FMC SDRAM bank*/
	SDRAM_GPIO_Config();

	/*Enable FMC clock*/
	__HAL_RCC_FMC_CLK_ENABLE();

	/*����Ա���� TMRD �ӳ�(Load Mode Register to Active)�������ͼ���ģʽ�Ĵ��������Ҫ�ȴ���ʱ�䣬�������ʱ��ſ��Է�������Ч��ˢ�����*/
	FMC_SDRAMTimingInitStructure.LoadToActiveDelay = 2; //??

	/*����Ա�����˳� TXSR �ӳ�(Exit Self-refresh delay)�����˳�����ˢ�������Ҫ�ȴ���ʱ�䣬�������ʱ��ſ��Է�������Ч���*/
	FMC_SDRAMTimingInitStructure.ExitSelfRefreshDelay = 7; //

	/*tRAS��Active to Precharge Command������Ч��Ԥ������������ڣ�*/
	/*����Ա��������ˢ��ʱ�� TRAS������������Ч�����Ҫ�ȴ���ʱ�䣬�������ʱ���ִ��Ԥ�������*/
	FMC_SDRAMTimingInitStructure.SelfRefreshTime = 4;

	/*����Ա���� TRC �ӳ�(Row cycle delay)������������Ч����֮����ӳ٣��Լ���������ˢ������֮����ӳ١�*/
	FMC_SDRAMTimingInitStructure.RowCycleDelay = 7;

	/*Ϊ�˱�֤���ݵĿɿ�д�룬���������㹻��д��/У��ʱ��----��tWR��Write Recovery Time�����������Ҳ������д�أ�Write Back����*/
	FMC_SDRAMTimingInitStructure.WriteRecoveryTime = 2;
	/*Ҫ����һ��ʱ�����������RAS����Ч������µĹ����У�����������ΪtRP��Precharge command Period��Ԥ�����Ч���ڣ�*/
	/*����Ա���� TRP �ӳ�(Row precharge delay)����Ԥ�����������������֮����ӳ١�*/
	FMC_SDRAMTimingInitStructure.RPDelay = 2;
	/*�ڷ����ж�д����ʱ����Ҫ������Ч������һ�������������������Ϊ---tRCD����RAS to CAS Delay��RAS��CAS�ӳ٣�*/
	FMC_SDRAMTimingInitStructure.RCDDelay = 2;

	hsdram1.Instance = FMC_SDRAM_DEVICE;
	hsdram1.Init.SDBank = FMC_SDRAM_BANK1;
	hsdram1.Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_8;
	hsdram1.Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_12;
	hsdram1.Init.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_32;
	hsdram1.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
	hsdram1.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_3;
	hsdram1.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
	hsdram1.Init.SDClockPeriod = FMC_SDRAM_CLOCK_PERIOD_2;
	hsdram1.Init.ReadBurst = FMC_SDRAM_RBURST_ENABLE;

	hsdram1.Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_1;

	//FMC SDRAM Bank Init
	HAL_SDRAM_Init(&hsdram1, &FMC_SDRAMTimingInitStructure);

	//FMC SDRAM device initialization sequence
	SDRAM_Initialization_Sequence(&hsdram1);
	
}


static void SDRAM_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	__HAL_RCC_GPIOI_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();


	GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_0|GPIO_PIN_1
	      |GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5
	      |GPIO_PIN_6|GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
	HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
	          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_11|GPIO_PIN_12
	          |GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_5|GPIO_PIN_8
	          |GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12
	          |GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
	HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5
	          |GPIO_PIN_8|GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10
	          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
	          |GPIO_PIN_15|GPIO_PIN_0|GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_14
	          |GPIO_PIN_15|GPIO_PIN_0|GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}


//�ⲿSDRAM�豸 ��ʼ������
/*�� SDRAM �ϵ磬���ṩ�ȶ���ʱ�ӣ����� 100us��
���͡��ղ�����(NOP) ���
���͡�Ԥ��硱(PRECHARGE) ����������� Bank ����Ԥ��磬���ȴ� tRP ʱ�䣬tRP ��ʾԤ�������������֮����ӳ٣�
�������� 2 �����Զ�ˢ�¡�(AUTO REFRESH) ���ÿ���������ȴ� tRFC ʱ�䣬tRFC ��ʾ�Զ�ˢ��ʱ�䣻
���͡�����ģʽ�Ĵ�����(LOAD MODE REGISTER) ������� SDRAM �Ĺ������������ȴ� tMRD ʱ�䣬tMRD ��ʾ����ģʽ�Ĵ��������������л�ˢ������֮����ӳ٣�
��ʼ��������ϣ����Կ�ʼ��д���ݡ�*/
static void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram)
{
	FMC_SDRAM_CommandTypeDef command = {0};
	uint32_t tmpmrd = 0;
	uint32_t timeout = SDRAM_TIMEOUT;
	
	/*step 3---Configure a clock configuration enable command*/
	command.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
	command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
	command.AutoRefreshNumber = 1;
	command.ModeRegisterDefinition = 0;

	/*Wait until the SDRAM controller is ready*/
	while((HAL_SDRAM_GetState(hsdram) == HAL_SDRAM_STATE_BUSY) && (timeout > 0)) 
	{
		timeout--;
	}
	HAL_SDRAM_SendCommand(hsdram, &command, 0x1000);

	/*step 4  Insert 100ms delay*/
	bsp_DelayMS(100);

	/*step 5 Configure a PALL(precharge all) Command*/
	command.CommandMode = FMC_SDRAM_CMD_PALL;
	command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
	command.AutoRefreshNumber = 1;
	command.ModeRegisterDefinition = 0;

	/*Wait until the SDRAM controller is ready*/
	timeout = SDRAM_TIMEOUT;
	while((HAL_SDRAM_GetState(hsdram) == HAL_SDRAM_STATE_BUSY) && (timeout > 0))
	{
		timeout--;
	}
	HAL_SDRAM_SendCommand(hsdram, &command, 0x1000);
	
	/*step 6 Configure a Auto-Refresh command*/
	command.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
	command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
	command.AutoRefreshNumber = 8;
	command.ModeRegisterDefinition = 0;

	/*Wait until the SDRAM controller is ready*/
	timeout = SDRAM_TIMEOUT;
	while((HAL_SDRAM_GetState(hsdram) == HAL_SDRAM_STATE_BUSY) && (timeout > 0))
	{
		timeout--;
	}
	HAL_SDRAM_SendCommand(hsdram, &command, 0x1000);

	/*step 7 Program the external memory mode register*/
	tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1          |
					   SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |
					   SDRAM_MODEREG_CAS_LATENCY_3           |
					   SDRAM_MODEREG_OPERATING_MODE_STANDARD |
					   SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

	command.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
	command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
	command.AutoRefreshNumber = 1;
	command.ModeRegisterDefinition = tmpmrd;

	/*Wait until the SDRAM controller is ready*/
	timeout = SDRAM_TIMEOUT;
	while((HAL_SDRAM_GetState(hsdram) == HAL_SDRAM_STATE_BUSY) && (timeout > 0))
	{
		timeout--;
	}
	HAL_SDRAM_SendCommand(hsdram, &command, 0x1000);


	/******step 8 Set the refresh rate counter */
	/******(15.62 us��Freq) - 20 ***************/
	/*******Set the device refresh counter*/
	HAL_SDRAM_ProgramRefreshRate(hsdram,1386);

	/*Wait until the SDRAM controller is ready*/
	timeout = SDRAM_TIMEOUT;
	while((HAL_SDRAM_GetState(hsdram) == HAL_SDRAM_STATE_BUSY) && (timeout > 0))
	{
		timeout--;
	}
}


uint32_t bsp_TestExtSDRAM1(void)
{
	uint32_t i = 0;
	uint32_t *pSRAM = NULL;
	uint8_t *pBytes = NULL;
	uint32_t err;

	const uint8_t byteBuf[4] = {0x55, 0xA5, 0x5A, 0xAA};

	/*write SDRAM*/
	pSRAM = (uint32_t *)EXT_SDRAM_ADDR;
	for(i = 0; i < EXT_SDRAM_SIZE / 4; i++)
	{
		*pSRAM++ = i;
	}

	/*read SDRAM*/
	err = 0;
	pSRAM = (uint32_t *)EXT_SDRAM_ADDR;
	for(i = 0; i < EXT_SDRAM_SIZE / 4; i++)
	{
		if(*pSRAM++ != i)
		{
			err++;
		}
	}

	if(err > 0)
	{
		return (4*err);
	}

	/*���԰��ֽڷ�ʽ���ʣ�Ŀ����֤ FSMC_NBL0��FSMC_NBL1�ӿ�*/
	pBytes = (uint8_t *)EXT_SDRAM_ADDR;
	for(i = 0; i < sizeof(byteBuf); i++)
	{
		*pBytes++ = byteBuf[i];
	}

	err = 0;
	pBytes = (uint8_t *)EXT_SDRAM_ADDR;
	for(i = 0; i < sizeof(byteBuf); i++)
	{
		if(*pBytes++ != byteBuf[i])
		{
			err++;
		}
	}
	if(err > 0)
	{
		return err;
	}
	return 0;
	
}

#if 0
static void my_fmc_gpio_deinit(void)
{
	__HAL_RCC_GPIOI_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();

	__HAL_RCC_FMC_CLK_DISABLE();

	HAL_GPIO_DeInit(GPIOI, GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_0|GPIO_PIN_1
                          |GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_6|GPIO_PIN_7);

  	HAL_GPIO_DeInit(GPIOF, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_11|GPIO_PIN_12
                          |GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15);

  	HAL_GPIO_DeInit(GPIOH, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_5|GPIO_PIN_8
                          |GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12
                          |GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15);

  	HAL_GPIO_DeInit(GPIOG, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_8|GPIO_PIN_15);

  	HAL_GPIO_DeInit(GPIOE, GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_15|GPIO_PIN_0|GPIO_PIN_1);

  	HAL_GPIO_DeInit(GPIOD, GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_14
                          |GPIO_PIN_15|GPIO_PIN_0|GPIO_PIN_1);
}
#endif



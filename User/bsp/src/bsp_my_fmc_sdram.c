

#include "bsp.h"


/* SDRAM超时 */
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

	/*本成员设置 TMRD 延迟(Load Mode Register to Active)，即发送加载模式寄存器命令后要等待的时间，过了这段时间才可以发送行有效或刷新命令。*/
	FMC_SDRAMTimingInitStructure.LoadToActiveDelay = 2; //??

	/*本成员设置退出 TXSR 延迟(Exit Self-refresh delay)，即退出自我刷新命令后要等待的时间，过了这段时间才可以发送行有效命令。*/
	FMC_SDRAMTimingInitStructure.ExitSelfRefreshDelay = 7; //

	/*tRAS（Active to Precharge Command，行有效至预充电命令间隔周期）*/
	/*本成员设置自我刷新时间 TRAS，即发送行有效命令后要等待的时间，过了这段时间才执行预充电命令*/
	FMC_SDRAMTimingInitStructure.SelfRefreshTime = 4;

	/*本成员设置 TRC 延迟(Row cycle delay)，即两个行有效命令之间的延迟，以及两个相邻刷新命令之间的延迟。*/
	FMC_SDRAMTimingInitStructure.RowCycleDelay = 7;

	/*为了保证数据的可靠写入，都会留出足够的写入/校正时间----（tWR，Write Recovery Time），这个操作也被称作写回（Write Back）。*/
	FMC_SDRAMTimingInitStructure.WriteRecoveryTime = 2;
	/*要经过一段时间才能允许发送RAS行有效命令打开新的工作行，这个间隔被称为tRP（Precharge command Period，预充电有效周期）*/
	/*本成员设置 TRP 延迟(Row precharge delay)，即预充电命令与其它命令之间的延迟。*/
	FMC_SDRAMTimingInitStructure.RPDelay = 2;
	/*在发送列读写命令时必须要与行有效命令有一个间隔，这个间隔被定义为---tRCD，即RAS to CAS Delay（RAS至CAS延迟）*/
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


//外部SDRAM设备 初始化流程
/*给 SDRAM 上电，并提供稳定的时钟，至少 100us；
发送“空操作”(NOP) 命令；
发送“预充电”(PRECHARGE) 命令，控制所有 Bank 进行预充电，并等待 tRP 时间，tRP 表示预充电与其它命令之间的延迟；
发送至少 2 个“自动刷新”(AUTO REFRESH) 命令，每个命令后需等待 tRFC 时间，tRFC 表示自动刷新时间；
发送“加载模式寄存器”(LOAD MODE REGISTER) 命令，配置 SDRAM 的工作参数，并等待 tMRD 时间，tMRD 表示加载模式寄存器命令与行有行或刷新命令之间的延迟；
初始化流程完毕，可以开始读写数据。*/
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
	/******(15.62 us×Freq) - 20 ***************/
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

	/*测试按字节方式访问，目的验证 FSMC_NBL0、FSMC_NBL1接口*/
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



#ifndef _BSP_MY_FMC_SDRAM_H_
#define _BSP_MY_FMC_SDRAM_H_


#define EXT_SDRAM_ADDR  	((uint32_t)0xC0000000)
#define EXT_SDRAM_SIZE		(16 * 1024 * 1024)


/*
**********************************************************************************************************
						   LCD显存使用，共使用8MB，SDRAM容量16MB
**********************************************************************************************************
*/
/* LCD显存, 图层1, 分配4M字节 */
#define SDRAM_LCD_BUF1 		EXT_SDRAM_ADDR

/* LCD显存, 图层2, 分配4M字节 */
#define SDRAM_LCD_BUF2	   (EXT_SDRAM_ADDR + 4 * 1024 * 1024)

/* 仅SDRAM驱动里面的测试代码调用了 */
#define SDRAM_APP_SIZE	   (8 * 1024 * 1024)

/*
**********************************************************************************************************
					emWin动态内存使用，除了显存使用的8MB，后8MB给动态内存使用
**********************************************************************************************************
*/
/* emWin动态内存首地址 */
#define SDRAM_APP_BUF     (EXT_SDRAM_ADDR + 8 * 1024 * 1024)


void bsp_InitExitSDRAM(void);
uint32_t bsp_TestExtSDRAM1(void);


#endif

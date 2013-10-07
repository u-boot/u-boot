/*
 *  Copyright (C) 2004 PaulReynolds@lhsolutions.com
 *
 * (C) Copyright 2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/processor.h>
#include <spd_sdram.h>
#include <asm/ppc4xx-emac.h>
#include <netdev.h>

#ifdef CONFIG_SYS_INIT_SHOW_RESET_REG
void show_reset_reg(void);
#endif

DECLARE_GLOBAL_DATA_PTR;

int lcd_init(void);

int board_early_init_f (void)
{
	unsigned long reg;
	volatile unsigned int *GpioOdr;
	volatile unsigned int *GpioTcr;
	volatile unsigned int *GpioOr;

	/*-------------------------------------------------------------------------+
	  | Initialize EBC CONFIG
	  +-------------------------------------------------------------------------*/
	mtebc(EBC0_CFG, EBC_CFG_LE_UNLOCK |
	      EBC_CFG_PTD_ENABLE | EBC_CFG_RTC_64PERCLK |
	      EBC_CFG_ATC_PREVIOUS | EBC_CFG_DTC_PREVIOUS |
	      EBC_CFG_CTC_PREVIOUS | EBC_CFG_EMC_DEFAULT |
	      EBC_CFG_PME_DISABLE | EBC_CFG_PR_32);

	/*-------------------------------------------------------------------------+
	  | 64MB FLASH. Initialize bank 0 with default values.
	  +-------------------------------------------------------------------------*/
	mtebc(PB0AP, EBC_BXAP_BME_DISABLED|EBC_BXAP_TWT_ENCODE(15) |
	      EBC_BXAP_BCE_DISABLE |
	      EBC_BXAP_CSN_ENCODE(1) | EBC_BXAP_OEN_ENCODE(1) |
	      EBC_BXAP_WBN_ENCODE(1) | EBC_BXAP_WBF_ENCODE(1) |
	      EBC_BXAP_TH_ENCODE(3) | EBC_BXAP_RE_DISABLED |
	      EBC_BXAP_BEM_WRITEONLY |
	      EBC_BXAP_PEN_DISABLED);
	mtebc(PB0CR, EBC_BXCR_BAS_ENCODE(CONFIG_SYS_FLASH_BASE) |
	      EBC_BXCR_BS_64MB | EBC_BXCR_BU_RW|EBC_BXCR_BW_32BIT);

	/*-------------------------------------------------------------------------+
	  | FPGA. Initialize bank 1 with default values.
	  +-------------------------------------------------------------------------*/
	mtebc(PB1AP, EBC_BXAP_BME_DISABLED|EBC_BXAP_TWT_ENCODE(5) |
	      EBC_BXAP_BCE_DISABLE |
	      EBC_BXAP_CSN_ENCODE(1) | EBC_BXAP_OEN_ENCODE(1) |
	      EBC_BXAP_WBN_ENCODE(1) | EBC_BXAP_WBF_ENCODE(1) |
	      EBC_BXAP_TH_ENCODE(3) | EBC_BXAP_RE_DISABLED |
	      EBC_BXAP_BEM_WRITEONLY |
	      EBC_BXAP_PEN_DISABLED);
	mtebc(PB1CR, EBC_BXCR_BAS_ENCODE(0x41000000) |
	      EBC_BXCR_BS_1MB | EBC_BXCR_BU_RW | EBC_BXCR_BW_8BIT);

	/*-------------------------------------------------------------------------+
	  | LCM. Initialize bank 2 with default values.
	  +-------------------------------------------------------------------------*/
	mtebc(PB2AP, EBC_BXAP_BME_DISABLED | EBC_BXAP_TWT_ENCODE(64) |
	      EBC_BXAP_BCE_DISABLE |
	      EBC_BXAP_CSN_ENCODE(3) | EBC_BXAP_OEN_ENCODE(3) |
	      EBC_BXAP_WBN_ENCODE(3) | EBC_BXAP_WBF_ENCODE(3) |
	      EBC_BXAP_TH_ENCODE(7) | EBC_BXAP_RE_DISABLED |
	      EBC_BXAP_BEM_WRITEONLY |
	      EBC_BXAP_PEN_DISABLED);
	mtebc(PB2CR, EBC_BXCR_BAS_ENCODE(0x42000000) |
	      EBC_BXCR_BS_1MB | EBC_BXCR_BU_RW|EBC_BXCR_BW_8BIT);

	/*-------------------------------------------------------------------------+
	  | TMP. Initialize bank 3 with default values.
	  +-------------------------------------------------------------------------*/
	mtebc(PB3AP, EBC_BXAP_BME_DISABLED | EBC_BXAP_TWT_ENCODE(128) |
	      EBC_BXAP_BCE_DISABLE |
	      EBC_BXAP_CSN_ENCODE(3) | EBC_BXAP_OEN_ENCODE(3) |
	      EBC_BXAP_WBN_ENCODE(3) | EBC_BXAP_WBF_ENCODE(3) |
	      EBC_BXAP_TH_ENCODE(7) | EBC_BXAP_RE_DISABLED |
	      EBC_BXAP_BEM_WRITEONLY |
	      EBC_BXAP_PEN_DISABLED);
	mtebc(PB3CR, EBC_BXCR_BAS_ENCODE(0x48000000) |
	      EBC_BXCR_BS_64MB | EBC_BXCR_BU_RW | EBC_BXCR_BW_32BIT);

	/*-------------------------------------------------------------------------+
	  | Connector 4~7. Initialize bank 3~ 7 with default values.
	  +-------------------------------------------------------------------------*/
	mtebc(PB4AP,0);
	mtebc(PB4CR,0);
	mtebc(PB5AP,0);
	mtebc(PB5CR,0);
	mtebc(PB6AP,0);
	mtebc(PB6CR,0);
	mtebc(PB7AP,0);
	mtebc(PB7CR,0);

	/*--------------------------------------------------------------------
	 * Setup the interrupt controller polarities, triggers, etc.
	 *-------------------------------------------------------------------*/
	/*
	 * Because of the interrupt handling rework to handle 440GX interrupts
	 * with the common code, we needed to change names of the UIC registers.
	 * Here the new relationship:
	 *
	 * U-Boot name	440GX name
	 * -----------------------
	 * UIC0		UICB0
	 * UIC1		UIC0
	 * UIC2		UIC1
	 * UIC3		UIC2
	 */
	mtdcr (UIC1SR, 0xffffffff);	/* clear all */
	mtdcr (UIC1ER, 0x00000000);	/* disable all */
	mtdcr (UIC1CR, 0x00000009);	/* SMI & UIC1 crit are critical */
	mtdcr (UIC1PR, 0xfffffe13);	/* per ref-board manual */
	mtdcr (UIC1TR, 0x01c00008);	/* per ref-board manual */
	mtdcr (UIC1VR, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr (UIC1SR, 0xffffffff);	/* clear all */

	mtdcr (UIC2SR, 0xffffffff);	/* clear all */
	mtdcr (UIC2ER, 0x00000000);	/* disable all */
	mtdcr (UIC2CR, 0x00000000);	/* all non-critical */
	mtdcr (UIC2PR, 0xffffe0ff);	/* per ref-board manual */
	mtdcr (UIC2TR, 0x00ffc000);	/* per ref-board manual */
	mtdcr (UIC2VR, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr (UIC2SR, 0xffffffff);	/* clear all */

	mtdcr (UIC3SR, 0xffffffff);	/* clear all */
	mtdcr (UIC3ER, 0x00000000);	/* disable all */
	mtdcr (UIC3CR, 0x00000000);	/* all non-critical */
	mtdcr (UIC3PR, 0xffffffff);	/* per ref-board manual */
	mtdcr (UIC3TR, 0x00ff8c0f);	/* per ref-board manual */
	mtdcr (UIC3VR, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr (UIC3SR, 0xffffffff);	/* clear all */

	mtdcr (UIC0SR, 0xfc000000);	/* clear all */
	mtdcr (UIC0ER, 0x00000000);	/* disable all */
	mtdcr (UIC0CR, 0x00000000);	/* all non-critical */
	mtdcr (UIC0PR, 0xfc000000);	/* */
	mtdcr (UIC0TR, 0x00000000);	/* */
	mtdcr (UIC0VR, 0x00000001);	/* */

	/* Enable two GPIO 10~11 and TraceA signal */
	mfsdr(SDR0_PFC0,reg);
	reg |= 0x00300000;
	mtsdr(SDR0_PFC0,reg);

	mfsdr(SDR0_PFC1,reg);
	reg |= 0x00100000;
	mtsdr(SDR0_PFC1,reg);

	/* Set GPIO 10 and 11 as output */
	GpioOdr	= (volatile unsigned int*)(CONFIG_SYS_PERIPHERAL_BASE+0x718);
	GpioTcr = (volatile unsigned int*)(CONFIG_SYS_PERIPHERAL_BASE+0x704);
	GpioOr  = (volatile unsigned int*)(CONFIG_SYS_PERIPHERAL_BASE+0x700);

	*GpioOdr &= ~(0x00300000);
	*GpioTcr |= 0x00300000;
	*GpioOr  |= 0x00300000;

	return 0;
}

int misc_init_r(void)
{
	lcd_init();

	return 0;
}

int checkboard (void)
{
	char buf[64];
	int i = getenv_f("serial#", buf, sizeof(buf));

	printf ("Board: Taishan - AMCC PPC440GX Evaluation Board");
	if (i > 0) {
		puts(", serial# ");
		puts(buf);
	}
	putc ('\n');

#ifdef CONFIG_SYS_INIT_SHOW_RESET_REG
	show_reset_reg();
#endif

	return (0);
}

int board_eth_init(bd_t *bis)
{
	cpu_eth_init(bis);
	return pci_eth_init(bis);
}

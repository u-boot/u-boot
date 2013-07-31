/*
 * This is the main board level file for the Redwood AMCC board.
 *
 * (C) Copyright 2008
 * Feng Kan, Applied Micro Circuits Corp., fkan@amcc.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include "redwood.h"
#include <asm/ppc4xx.h>
#include <asm/processor.h>
#include <i2c.h>
#include <asm/io.h>

int compare_to_true(char *str);
char *remove_l_w_space(char *in_str);
char *remove_t_w_space(char *in_str);
int get_console_port(void);

static void early_init_EBC(void);
static int bootdevice_selected(void);
static void early_reinit_EBC(int);
static void early_init_UIC(void);

/*
 * Define Boot devices
 */
#define BOOT_FROM_8BIT_SRAM			0x00
#define BOOT_FROM_16BIT_SRAM			0x01
#define BOOT_FROM_32BIT_SRAM			0x02
#define BOOT_FROM_8BIT_NAND			0x03
#define BOOT_FROM_16BIT_NOR			0x04
#define BOOT_DEVICE_UNKNOWN			0xff

/*
 * EBC Devices Characteristics
 *   Peripheral Bank Access Parameters       -   EBC_BxAP
 *   Peripheral Bank Configuration Register  -   EBC_BxCR
 */

/*
 * 8 bit width SRAM
 * BU Value
 * BxAP : 0x03800000  - 0 00000111 0 00 00 00 00 00 000 0 0 0 0 00000
 * B0CR : 0xff098000  - BAS = ff0 - 100 11 00 0000000000000
 * B2CR : 0xe7098000  - BAS = e70 - 100 11 00 0000000000000
 */
#define EBC_BXAP_8BIT_SRAM					\
	EBC_BXAP_BME_DISABLED   | EBC_BXAP_TWT_ENCODE(7)  |	\
	EBC_BXAP_BCE_DISABLE    | EBC_BXAP_BCT_2TRANS     |	\
	EBC_BXAP_CSN_ENCODE(0)  | EBC_BXAP_OEN_ENCODE(0)  |	\
	EBC_BXAP_WBN_ENCODE(0)  | EBC_BXAP_WBF_ENCODE(0)  |	\
	EBC_BXAP_TH_ENCODE(0)   | EBC_BXAP_RE_DISABLED    |	\
	EBC_BXAP_SOR_DELAYED    | EBC_BXAP_BEM_WRITEONLY  |	\
	EBC_BXAP_PEN_DISABLED

#define EBC_BXAP_16BIT_SRAM	EBC_BXAP_8BIT_SRAM
#define EBC_BXAP_32BIT_SRAM	EBC_BXAP_8BIT_SRAM

/*
 * NAND flash
 * BU Value
 * BxAP : 0x048ff240  - 0 00000111 0 00 00 00 00 00 000 0 0 0 0 00000
 * B0CR : 0xff09a000  - BAS = ff0 - 100 11 01 0000000000000
 * B2CR : 0xe709a000  - BAS = e70 - 100 11 01 0000000000000
*/
#define EBC_BXAP_NAND						\
	EBC_BXAP_BME_DISABLED   | EBC_BXAP_TWT_ENCODE(7)  |	\
	EBC_BXAP_BCE_DISABLE    | EBC_BXAP_BCT_2TRANS     |	\
	EBC_BXAP_CSN_ENCODE(0)  | EBC_BXAP_OEN_ENCODE(0)  |	\
	EBC_BXAP_WBN_ENCODE(0)  | EBC_BXAP_WBF_ENCODE(0)  |	\
	EBC_BXAP_TH_ENCODE(0)   | EBC_BXAP_RE_DISABLED    |	\
	EBC_BXAP_SOR_DELAYED    | EBC_BXAP_BEM_WRITEONLY  |	\
	EBC_BXAP_PEN_DISABLED

/*
 * NOR flash
 * BU Value
 * BxAP : 0x048ff240  - 0 00000111 0 00 00 00 00 00 000 0 0 0 0 00000
 * B0CR : 0xff09a000  - BAS = ff0 - 100 11 01 0000000000000
 * B2CR : 0xe709a000  - BAS = e70 - 100 11 01 0000000000000
*/
#define EBC_BXAP_NOR						\
	EBC_BXAP_BME_DISABLED   | EBC_BXAP_TWT_ENCODE(7)  |	\
	EBC_BXAP_BCE_DISABLE    | EBC_BXAP_BCT_2TRANS     |	\
	EBC_BXAP_CSN_ENCODE(0)  | EBC_BXAP_OEN_ENCODE(0)  |	\
	EBC_BXAP_WBN_ENCODE(0)  | EBC_BXAP_WBF_ENCODE(0)  |	\
	EBC_BXAP_TH_ENCODE(0)   | EBC_BXAP_RE_DISABLED    |	\
	EBC_BXAP_SOR_DELAYED    | EBC_BXAP_BEM_WRITEONLY  |	\
	EBC_BXAP_PEN_DISABLED

/*
 * FPGA
 * BU value :
 * B1AP = 0x05895240  - 0 00001011 0 00 10 01 01 01 001 0 0 1 0 00000
 * B1CR = 0xe201a000  - BAS = e20 - 000 11 01 00000000000000
 */
#define EBC_BXAP_FPGA						\
	EBC_BXAP_BME_DISABLED   | EBC_BXAP_TWT_ENCODE(11) |	\
	EBC_BXAP_BCE_DISABLE    | EBC_BXAP_BCT_2TRANS     |	\
	EBC_BXAP_CSN_ENCODE(10) | EBC_BXAP_OEN_ENCODE(1)  |	\
	EBC_BXAP_WBN_ENCODE(1)  | EBC_BXAP_WBF_ENCODE(1)  |	\
	EBC_BXAP_TH_ENCODE(1)   | EBC_BXAP_RE_DISABLED    |	\
	EBC_BXAP_SOR_DELAYED    | EBC_BXAP_BEM_RW         |	\
	EBC_BXAP_PEN_DISABLED

#define EBC_BXCR_8BIT_SRAM_CS0						\
	EBC_BXCR_BAS_ENCODE(0xFFE00000) | EBC_BXCR_BS_1MB           |	\
	EBC_BXCR_BU_RW                  | EBC_BXCR_BW_8BIT

#define EBC_BXCR_32BIT_SRAM_CS0						\
	EBC_BXCR_BAS_ENCODE(0xFFC00000) | EBC_BXCR_BS_1MB           |	\
	EBC_BXCR_BU_RW                  | EBC_BXCR_BW_32BIT

#define EBC_BXCR_NAND_CS0						\
	EBC_BXCR_BAS_ENCODE(0xFF000000) | EBC_BXCR_BS_16MB          |	\
	EBC_BXCR_BU_RW                  | EBC_BXCR_BW_8BIT

#define EBC_BXCR_16BIT_SRAM_CS0						\
	EBC_BXCR_BAS_ENCODE(0xFFE00000) | EBC_BXCR_BS_2MB           |	\
	EBC_BXCR_BU_RW                  | EBC_BXCR_BW_16BIT

#define EBC_BXCR_NOR_CS0						\
	EBC_BXCR_BAS_ENCODE(0xFF000000) | EBC_BXCR_BS_16MB          |	\
	EBC_BXCR_BU_RW                  | EBC_BXCR_BW_16BIT

#define EBC_BXCR_NOR_CS1						\
	EBC_BXCR_BAS_ENCODE(0xE0000000) | EBC_BXCR_BS_128MB         |	\
	EBC_BXCR_BU_RW                  | EBC_BXCR_BW_16BIT

#define EBC_BXCR_NAND_CS1						\
	EBC_BXCR_BAS_ENCODE(0xE0000000) | EBC_BXCR_BS_128MB         |	\
	EBC_BXCR_BU_RW                  | EBC_BXCR_BW_8BIT

#define EBC_BXCR_NAND_CS2						\
	EBC_BXCR_BAS_ENCODE(0xC0000000) | EBC_BXCR_BS_128MB         |	\
	EBC_BXCR_BU_RW                  | EBC_BXCR_BW_8BIT

#define EBC_BXCR_SRAM_CS2						\
	EBC_BXCR_BAS_ENCODE(0xC0000000) | EBC_BXCR_BS_4MB           |	\
	EBC_BXCR_BU_RW                  | EBC_BXCR_BW_32BIT

#define EBC_BXCR_LARGE_FLASH_CS2					\
	EBC_BXCR_BAS_ENCODE(0xE7000000) | EBC_BXCR_BS_16MB          |	\
	EBC_BXCR_BU_RW                  | EBC_BXCR_BW_16BIT

#define EBC_BXCR_FPGA_CS3						\
	EBC_BXCR_BAS_ENCODE(0xE2000000) | EBC_BXCR_BS_1MB           |	\
	EBC_BXCR_BU_RW                  | EBC_BXCR_BW_16BIT

/*****************************************************************************
 * UBOOT initiated board specific function calls
 ****************************************************************************/

int board_early_init_f(void)
{
	int computed_boot_device = BOOT_DEVICE_UNKNOWN;

	/*
	 * Initialise EBC
	 */
	early_init_EBC();

	/*
	 * Determine which boot device was selected
	 */
	computed_boot_device = bootdevice_selected();

	/*
	 * Reinit EBC based on selected boot device
	 */
	early_reinit_EBC(computed_boot_device);

	/*
	 * Setup for UIC on 460SX redwood board
	 */
	early_init_UIC();

	return 0;
}

int checkboard(void)
{
	char buf[64];
	int i = getenv_f("serial#", buf, sizeof(buf));

	printf("Board: Redwood - AMCC 460SX Reference Board");
	if (i > 0) {
		puts(", serial# ");
		puts(buf);
	}
	putc('\n');

	return 0;
}

static void early_init_EBC(void)
{
	/*
	 * Initialize EBC CONFIG -
	 * Keep the Default value, but the bit PDT which has to be set to 1 ?TBC
	 * default value :
	 *      0x07C00000 - 0 0 000 1 1 1 1 1 0000 0 00000 000000000000
	 */
	mtebc(EBC0_CFG, EBC_CFG_LE_UNLOCK |
	      EBC_CFG_PTD_ENABLE |
	      EBC_CFG_RTC_16PERCLK |
	      EBC_CFG_ATC_PREVIOUS |
	      EBC_CFG_DTC_PREVIOUS |
	      EBC_CFG_CTC_PREVIOUS |
	      EBC_CFG_OEO_PREVIOUS |
	      EBC_CFG_EMC_DEFAULT | EBC_CFG_PME_DISABLE | EBC_CFG_PR_16);

	/*
	 * PART 1 : Initialize EBC Bank 3
	 * ==============================
	 * Bank1 is always associated to the EPLD.
	 * It has to be initialized prior to other banks settings computation
	 * since some board registers values may be needed to determine the
	 * boot type
	 */
	mtebc(PB1AP, EBC_BXAP_FPGA);
	mtebc(PB1CR, EBC_BXCR_FPGA_CS3);

}

static int bootdevice_selected(void)
{
	unsigned long sdr0_pinstp;
	unsigned long bootstrap_settings;
	int computed_boot_device = BOOT_DEVICE_UNKNOWN;

	/*
	 *  Determine which boot device was selected
	 *  =================================================
	 *
	 *  Read Pin Strap Register in PPC460SX
	 *  Result can either be :
	 *   - Boot strap = boot from EBC 8bits     => Small Flash
	 *   - Boot strap = boot from PCI
	 *   - Boot strap = IIC
	 *  In case of boot from IIC, read Serial Device Strap Register1
	 *
	 *  Result can either be :
	 *   - Boot from EBC  - EBC Bus Width = 8bits    => Small Flash
	 *   - Boot from EBC  - EBC Bus Width = 16bits   => Large Flash or SRAM
	 *   - Boot from PCI
	 */

	/* Read Pin Strap Register in PPC460SX */
	mfsdr(SDR0_PINSTP, sdr0_pinstp);
	bootstrap_settings = sdr0_pinstp & SDR0_PSTRP0_BOOTSTRAP_MASK;

	switch (bootstrap_settings) {
	case SDR0_PSTRP0_BOOTSTRAP_SETTINGS0:
		/*
		 * Boot from SRAM, 8bit width
		 */
		computed_boot_device = BOOT_FROM_8BIT_SRAM;
		break;
	case SDR0_PSTRP0_BOOTSTRAP_SETTINGS1:
		/*
		 * Boot from SRAM, 32bit width
		 */
		computed_boot_device = BOOT_FROM_32BIT_SRAM;
		break;
	case SDR0_PSTRP0_BOOTSTRAP_SETTINGS2:
		/*
		 * Boot from NAND, 8bit width
		 */
		computed_boot_device = BOOT_FROM_8BIT_NAND;
		break;
	case SDR0_PSTRP0_BOOTSTRAP_SETTINGS4:
		/*
		 * Boot from SRAM, 16bit width
		 * Boot setting in IIC EEPROM 0x50
		 */
		computed_boot_device = BOOT_FROM_16BIT_SRAM;
		break;
	case SDR0_PSTRP0_BOOTSTRAP_SETTINGS5:
		/*
		 * Boot from NOR, 16bit width
		 * Boot setting in IIC EEPROM 0x54
		 */
		computed_boot_device = BOOT_FROM_16BIT_NOR;
		break;
	default:
		/* should not be */
		computed_boot_device = BOOT_DEVICE_UNKNOWN;
		break;
	}

	return computed_boot_device;
}

static void early_reinit_EBC(int computed_boot_device)
{
	/*
	 *  Compute EBC settings depending on selected boot device
	 *  ======================================================
	 *
	 * Resulting EBC init will be among following configurations :
	 *
	 *  - Boot from EBC 8bits => boot from Small Flash selected
	 *            EBC-CS0     = Small Flash
	 *            EBC-CS2     = Large Flash and SRAM
	 *
	 *  - Boot from EBC 16bits => boot from Large Flash or SRAM
	 *            EBC-CS0     = Large Flash or SRAM
	 *            EBC-CS2     = Small Flash
	 *
	 *  - Boot from PCI
	 *            EBC-CS0     = not initialized to avoid address contention
	 *            EBC-CS2     = same as boot from Small Flash selected
	 */

	unsigned long ebc0_cs0_bxap_value = 0, ebc0_cs0_bxcr_value = 0;
	unsigned long ebc0_cs1_bxap_value = 0, ebc0_cs1_bxcr_value = 0;
	unsigned long ebc0_cs2_bxap_value = 0, ebc0_cs2_bxcr_value = 0;

	switch (computed_boot_device) {
		/*-------------------------------------------------------------------*/
	case BOOT_FROM_8BIT_SRAM:
		/*-------------------------------------------------------------------*/
		ebc0_cs0_bxap_value = EBC_BXAP_8BIT_SRAM;
		ebc0_cs0_bxcr_value = EBC_BXCR_8BIT_SRAM_CS0;
		ebc0_cs1_bxap_value = EBC_BXAP_NOR;
		ebc0_cs1_bxcr_value = EBC_BXCR_NOR_CS1;
		ebc0_cs2_bxap_value = EBC_BXAP_NAND;
		ebc0_cs2_bxcr_value = EBC_BXCR_NAND_CS2;
		break;

		/*-------------------------------------------------------------------*/
	case BOOT_FROM_16BIT_SRAM:
		/*-------------------------------------------------------------------*/
		ebc0_cs0_bxap_value = EBC_BXAP_16BIT_SRAM;
		ebc0_cs0_bxcr_value = EBC_BXCR_16BIT_SRAM_CS0;
		ebc0_cs1_bxap_value = EBC_BXAP_NOR;
		ebc0_cs1_bxcr_value = EBC_BXCR_NOR_CS1;
		ebc0_cs2_bxap_value = EBC_BXAP_NAND;
		ebc0_cs2_bxcr_value = EBC_BXCR_NAND_CS2;
		break;

		/*-------------------------------------------------------------------*/
	case BOOT_FROM_32BIT_SRAM:
		/*-------------------------------------------------------------------*/
		ebc0_cs0_bxap_value = EBC_BXAP_32BIT_SRAM;
		ebc0_cs0_bxcr_value = EBC_BXCR_32BIT_SRAM_CS0;
		ebc0_cs1_bxap_value = EBC_BXAP_NOR;
		ebc0_cs1_bxcr_value = EBC_BXCR_NOR_CS1;
		ebc0_cs2_bxap_value = EBC_BXAP_NAND;
		ebc0_cs2_bxcr_value = EBC_BXCR_NAND_CS2;
		break;

		/*-------------------------------------------------------------------*/
	case BOOT_FROM_16BIT_NOR:
		/*-------------------------------------------------------------------*/
		ebc0_cs0_bxap_value = EBC_BXAP_NOR;
		ebc0_cs0_bxcr_value = EBC_BXCR_NOR_CS0;
		ebc0_cs1_bxap_value = EBC_BXAP_NAND;
		ebc0_cs1_bxcr_value = EBC_BXCR_NAND_CS1;
		ebc0_cs2_bxap_value = EBC_BXAP_32BIT_SRAM;
		ebc0_cs2_bxcr_value = EBC_BXCR_SRAM_CS2;
		break;

		/*-------------------------------------------------------------------*/
	case BOOT_FROM_8BIT_NAND:
		/*-------------------------------------------------------------------*/
		ebc0_cs0_bxap_value = EBC_BXAP_NAND;
		ebc0_cs0_bxcr_value = EBC_BXCR_NAND_CS0;
		ebc0_cs1_bxap_value = EBC_BXAP_NOR;
		ebc0_cs1_bxcr_value = EBC_BXCR_NOR_CS1;
		ebc0_cs2_bxap_value = EBC_BXAP_32BIT_SRAM;
		ebc0_cs2_bxcr_value = EBC_BXCR_SRAM_CS2;
		break;

		/*-------------------------------------------------------------------*/
	default:
		/*-------------------------------------------------------------------*/
		/* BOOT_DEVICE_UNKNOWN */
		break;
	}

	mtebc(PB0AP, ebc0_cs0_bxap_value);
	mtebc(PB0CR, ebc0_cs0_bxcr_value);
	mtebc(PB1AP, ebc0_cs1_bxap_value);
	mtebc(PB1CR, ebc0_cs1_bxcr_value);
	mtebc(PB2AP, ebc0_cs2_bxap_value);
	mtebc(PB2CR, ebc0_cs2_bxcr_value);
}

static void early_init_UIC(void)
{
	/*
	 * Initialise UIC registers.  Clear all interrupts.  Disable all
	 * interrupts.
	 * Set critical interrupt values.  Set interrupt polarities.  Set
	 * interrupt trigger levels.  Make bit 0 High  priority.  Clear all
	 * interrupts again.
	 */
	mtdcr(UIC3SR, 0xffffffff);	/* Clear all interrupts */
	mtdcr(UIC3ER, 0x00000000);	/* disable all interrupts */
	mtdcr(UIC3CR, 0x00000000);	/* Set Critical / Non Critical
					 * interrupts */
	mtdcr(UIC3PR, 0xffffffff);	/* Set Interrupt Polarities */
	mtdcr(UIC3TR, 0x001fffff);	/* Set Interrupt Trigger Levels */
	mtdcr(UIC3VR, 0x00000000);	/* int31 highest, base=0x000 */
	mtdcr(UIC3SR, 0xffffffff);	/* clear all  interrupts */

	mtdcr(UIC2SR, 0xffffffff);	/* Clear all interrupts */
	mtdcr(UIC2ER, 0x00000000);	/* disable all interrupts */
	mtdcr(UIC2CR, 0x00000000);	/* Set Critical / Non Critical
					 * interrupts */
	mtdcr(UIC2PR, 0xebebebff);	/* Set Interrupt Polarities */
	mtdcr(UIC2TR, 0x74747400);	/* Set Interrupt Trigger Levels */
	mtdcr(UIC2VR, 0x00000000);	/* int31 highest, base=0x000 */
	mtdcr(UIC2SR, 0xffffffff);	/* clear all interrupts */

	mtdcr(UIC1SR, 0xffffffff);	/* Clear all interrupts */
	mtdcr(UIC1ER, 0x00000000);	/* disable all interrupts */
	mtdcr(UIC1CR, 0x00000000);	/* Set Critical / Non Critical
					 * interrupts */
	mtdcr(UIC1PR, 0xffffffff);	/* Set Interrupt Polarities */
	mtdcr(UIC1TR, 0x001fc0ff);	/* Set Interrupt Trigger Levels */
	mtdcr(UIC1VR, 0x00000000);	/* int31 highest, base=0x000 */
	mtdcr(UIC1SR, 0xffffffff);	/* clear all interrupts */

	mtdcr(UIC0SR, 0xffffffff);	/* Clear all interrupts */
	mtdcr(UIC0ER, 0x00000000);	/* disable all interrupts excepted
					 * cascade to be checked */
	mtdcr(UIC0CR, 0x00104001);	/* Set Critical / Non Critical
					 * interrupts */
	mtdcr(UIC0PR, 0xffffffff);	/* Set Interrupt Polarities */
	mtdcr(UIC0TR, 0x000f003c);	/* Set Interrupt Trigger Levels */
	mtdcr(UIC0VR, 0x00000000);	/* int31 highest, base=0x000 */
	mtdcr(UIC0SR, 0xffffffff);	/* clear all interrupts */

}

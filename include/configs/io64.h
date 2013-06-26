/*
 * (C) Copyright 2011
 * Dirk Eibach,  Guntermann & Drunck GmbH, eibach@gdsys.de
 *
 * based on kilauea.h
 * by Stefan Roese, DENX Software Engineering, sr@denx.de.
 * and Grant Erickson <gerickson@nuovations.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
 */

/************************************************************************
 * io64.h - configuration for Guntermann & Drunck Io64 (405EX)
 ***********************************************************************/

#ifndef __CONFIG_H
#define __CONFIG_H

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
#define CONFIG_IO64		1		/* Board is Io64 */
#define CONFIG_4xx		1		/* ... PPC4xx family */
#define CONFIG_405EX		1		/* Specifc 405EX support*/
#define CONFIG_SYS_CLK_FREQ	33333333	/* ext frequency to pll */

#ifndef CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_TEXT_BASE	0xFFFA0000
#endif

/*
 * CHIP_21 errata
 */
#define CONFIG_SYS_4xx_CHIP_21_405EX_SECURITY

/*
 * Include common defines/options for all AMCC eval boards
 */
#define CONFIG_HOSTNAME		io64
#define CONFIG_IDENT_STRING	" io64 0.02"
#include "amcc-common.h"

#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_BOARD_EARLY_INIT_R
#define CONFIG_MISC_INIT_R
#define CONFIG_LAST_STAGE_INIT

#undef CONFIG_ZERO_BOOTDELAY_CHECK	/* ignore keypress on bootdelay==0 */
#define CONFIG_AUTOBOOT_KEYED		/* use key strings to stop autoboot */
#define CONFIG_AUTOBOOT_STOP_STR " "

/* new uImage format support */
#define CONFIG_FIT
#define CONFIG_FIT_VERBOSE

/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_FLASH_BASE		0xFC000000
#define CONFIG_SYS_NVRAM_BASE		0xF0000000
#define CONFIG_SYS_FPGA0_BASE		0xF0100000
#define CONFIG_SYS_FPGA1_BASE		0xF0108000
#define CONFIG_SYS_LATCH_BASE		0xF0200000

/*-----------------------------------------------------------------------
 * Initial RAM & Stack Pointer Configuration Options
 *
 *   There are traditionally three options for the primordial
 *   (i.e. initial) stack usage on the 405-series:
 *
 *      1) On-chip Memory (OCM) (i.e. SRAM)
 *      2) Data cache
 *      3) SDRAM
 *
 *   For the 405EX(r), there is no OCM, so we are left with (2) or (3)
 *   the latter of which is less than desireable since it requires
 *   setting up the SDRAM and ECC in assembly code.
 *
 *   To use (2), define 'CONFIG_SYS_INIT_DCACHE_CS' to be an unused chip
 *   select on the External Bus Controller (EBC) and then select a
 *   value for 'CONFIG_SYS_INIT_RAM_ADDR' outside of the range of valid,
 *   physical SDRAM. Otherwise, undefine 'CONFIG_SYS_INIT_DCACHE_CS' and
 *   select a value for 'CONFIG_SYS_INIT_RAM_ADDR' within the range of valid,
 *   physical SDRAM to use (3).
 *-----------------------------------------------------------------------*/

#define CONFIG_SYS_INIT_DCACHE_CS	4

#if defined(CONFIG_SYS_INIT_DCACHE_CS)
#define CONFIG_SYS_INIT_RAM_ADDR \
	(CONFIG_SYS_SDRAM_BASE + (1 << 30))	/*  1 GiB */
#else
#define CONFIG_SYS_INIT_RAM_ADDR \
	(CONFIG_SYS_SDRAM_BASE + (32 << 20))	/* 32 MiB */
#endif /* defined(CONFIG_SYS_INIT_DCACHE_CS) */

#define CONFIG_SYS_INIT_RAM_SIZE \
	(4 << 10)				/*  4 KiB */
#define CONFIG_SYS_GBL_DATA_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)

/*
 * If the data cache is being used for the primordial stack and global
 * data area, the POST word must be placed somewhere else. The General
 * Purpose Timer (GPT) is unused by u-boot and the kernel and preserves
 * its compare and mask register contents across reset, so it is used
 * for the POST word.
 */

#if defined(CONFIG_SYS_INIT_DCACHE_CS)
# define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET
# define CONFIG_SYS_POST_WORD_ADDR \
	(CONFIG_SYS_PERIPHERAL_BASE + GPT0_COMP6)
#else
# define CONFIG_SYS_INIT_EXTRA_SIZE	16
# define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_GBL_DATA_OFFSET - CONFIG_SYS_INIT_EXTRA_SIZE)
# define CONFIG_SYS_OCM_DATA_ADDR	CONFIG_SYS_INIT_RAM_ADDR
#endif /* defined(CONFIG_SYS_INIT_DCACHE_CS) */

/*-----------------------------------------------------------------------
 * Serial Port
 *----------------------------------------------------------------------*/
#define CONFIG_CONS_INDEX	1	/* Use UART0 */
#define CONFIG_SYS_BASE_BAUD	691200

/*-----------------------------------------------------------------------
 * Environment
 *----------------------------------------------------------------------*/
#define CONFIG_ENV_IS_IN_FLASH	1	/* use FLASH for environment vars */

/*-----------------------------------------------------------------------
 * FLASH related
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_FLASH_CFI		/* The flash is CFI compatible */
#define CONFIG_FLASH_CFI_DRIVER		/* Use common CFI driver */

#define CONFIG_SYS_FLASH_BANKS_LIST    {CONFIG_SYS_FLASH_BASE}
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	512

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000
#define CONFIG_SYS_FLASH_WRITE_TOUT	500

#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE
#define CONFIG_SYS_FLASH_EMPTY_INFO

#ifdef CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_SECT_SIZE	0x20000	/* size of one complete sector */
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE-CONFIG_ENV_SECT_SIZE)
#define	CONFIG_ENV_SIZE		0x4000	/* Total Size of Environment Sector */

/* Address and size of Redundant Environment Sector */
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR-CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)
#endif /* CONFIG_ENV_IS_IN_FLASH */

/* Gbit PHYs */
#define CONFIG_BITBANGMII		/* bit-bang MII PHY management */
#define CONFIG_BITBANGMII_MULTI

#define CONFIG_SYS_MDIO_PIN  (0x80000000 >> 12)	/* MDIO is GPIO12 */
#define CONFIG_SYS_MDC_PIN   (0x80000000 >> 13)	/* MDC  is GPIO13 */

#define CONFIG_SYS_GBIT_MII_BUSNAME	"io_miiphy0"

#define CONFIG_SYS_MDIO1_PIN  (0x80000000 >> 2)	/* MDIO is GPIO2 */
#define CONFIG_SYS_MDC1_PIN   (0x80000000 >> 3)	/* MDC  is GPIO3 */

#define CONFIG_SYS_GBIT_MII1_BUSNAME	"io_miiphy1"

/*-----------------------------------------------------------------------
 * DDR SDRAM
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_MBYTES_SDRAM        (128)	/* 128MB */

/*
 * CONFIG_PPC4xx_DDR_AUTOCALIBRATION
 *
 * Note: DDR Autocalibration Method_A scans the full range of possible PPC4xx
 *       SDRAM Controller DDR autocalibration values and takes a lot longer
 *       to run than Method_B.
 * (See the Method_A and Method_B algorithm discription in the file:
 *	arch/powerpc/cpu/ppc4xx/4xx_ibm_ddr2_autocalib.c)
 * Define CONFIG_PPC4xx_DDR_METHOD_A to use DDR autocalibration Method_A
 *
 * DDR Autocalibration Method_B is the default.
 */
#define	CONFIG_PPC4xx_DDR_AUTOCALIBRATION
#define	DEBUG_PPC4xx_DDR_AUTOCALIBRATION
#undef	CONFIG_PPC4xx_DDR_METHOD_A

#define	CONFIG_SYS_SDRAM0_MB0CF_BASE	((0 << 20) + CONFIG_SYS_SDRAM_BASE)

/* DDR1/2 SDRAM Device Control Register Data Values */
#define CONFIG_SYS_SDRAM0_MB0CF	((CONFIG_SYS_SDRAM0_MB0CF_BASE >> 3) | \
				 SDRAM_RXBAS_SDSZ_128MB | \
				 SDRAM_RXBAS_SDAM_MODE2 | \
				 SDRAM_RXBAS_SDBE_ENABLE)
#define CONFIG_SYS_SDRAM0_MB1CF	SDRAM_RXBAS_SDBE_DISABLE
#define CONFIG_SYS_SDRAM0_MB2CF	SDRAM_RXBAS_SDBE_DISABLE
#define CONFIG_SYS_SDRAM0_MB3CF	SDRAM_RXBAS_SDBE_DISABLE
#define CONFIG_SYS_SDRAM0_MCOPT1	(SDRAM_MCOPT1_PMU_OPEN | \
				 SDRAM_MCOPT1_4_BANKS | \
				 SDRAM_MCOPT1_DDR2_TYPE | \
				 SDRAM_MCOPT1_QDEP | \
				 SDRAM_MCOPT1_DCOO_DISABLED)
#define CONFIG_SYS_SDRAM0_MCOPT2	0x00000000
#define CONFIG_SYS_SDRAM0_MODT0	(SDRAM_MODT_EB0W_ENABLE | \
				 SDRAM_MODT_EB0R_ENABLE)
#define CONFIG_SYS_SDRAM0_MODT1	0x00000000
#define CONFIG_SYS_SDRAM0_CODT		(SDRAM_CODT_RK0R_ON | \
				 SDRAM_CODT_CKLZ_36OHM | \
				 SDRAM_CODT_DQS_1_8_V_DDR2 | \
				 SDRAM_CODT_IO_NMODE)
#define CONFIG_SYS_SDRAM0_RTR		SDRAM_RTR_RINT_ENCODE(1560)
#define CONFIG_SYS_SDRAM0_INITPLR0	(SDRAM_INITPLR_ENABLE | \
		SDRAM_INITPLR_IMWT_ENCODE(80) | \
		SDRAM_INITPLR_ICMD_ENCODE(JEDEC_CMD_NOP))
#define CONFIG_SYS_SDRAM0_INITPLR1	(SDRAM_INITPLR_ENABLE | \
		SDRAM_INITPLR_IMWT_ENCODE(3) | \
		SDRAM_INITPLR_ICMD_ENCODE(JEDEC_CMD_PRECHARGE) | \
		SDRAM_INITPLR_IBA_ENCODE(JEDEC_BA_MR) | \
		SDRAM_INITPLR_IMA_ENCODE(JEDEC_MA_PRECHARGE_ALL))
#define CONFIG_SYS_SDRAM0_INITPLR2	(SDRAM_INITPLR_ENABLE | \
		SDRAM_INITPLR_IMWT_ENCODE(2) | \
		SDRAM_INITPLR_ICMD_ENCODE(JEDEC_CMD_EMR) | \
		SDRAM_INITPLR_IBA_ENCODE(JEDEC_BA_EMR2) | \
		SDRAM_INITPLR_IMA_ENCODE(JEDEC_MA_EMR2_TEMP_COMMERCIAL))
#define CONFIG_SYS_SDRAM0_INITPLR3	(SDRAM_INITPLR_ENABLE | \
		SDRAM_INITPLR_IMWT_ENCODE(2) | \
		SDRAM_INITPLR_ICMD_ENCODE(JEDEC_CMD_EMR) | \
		SDRAM_INITPLR_IBA_ENCODE(JEDEC_BA_EMR3) | \
		SDRAM_INITPLR_IMA_ENCODE(0))
#define CONFIG_SYS_SDRAM0_INITPLR4	(SDRAM_INITPLR_ENABLE | \
		SDRAM_INITPLR_IMWT_ENCODE(2) | \
		SDRAM_INITPLR_ICMD_ENCODE(JEDEC_CMD_EMR) | \
		SDRAM_INITPLR_IBA_ENCODE(JEDEC_BA_EMR) | \
		SDRAM_INITPLR_IMA_ENCODE(JEDEC_MA_EMR_DQS_DISABLE | \
					 JEDEC_MA_EMR_RTT_75OHM))
#define CONFIG_SYS_SDRAM0_INITPLR5	(SDRAM_INITPLR_ENABLE | \
		SDRAM_INITPLR_IMWT_ENCODE(2) | \
		SDRAM_INITPLR_ICMD_ENCODE(JEDEC_CMD_EMR) | \
		SDRAM_INITPLR_IBA_ENCODE(JEDEC_BA_MR) | \
		SDRAM_INITPLR_IMA_ENCODE(JEDEC_MA_MR_WR_DDR2_3_CYC | \
					 JEDEC_MA_MR_CL_DDR2_5_0_CLK | \
					 JEDEC_MA_MR_BLEN_4 | \
					 JEDEC_MA_MR_DLL_RESET))
#define CONFIG_SYS_SDRAM0_INITPLR6	(SDRAM_INITPLR_ENABLE | \
		SDRAM_INITPLR_IMWT_ENCODE(3) | \
		SDRAM_INITPLR_ICMD_ENCODE(JEDEC_CMD_PRECHARGE) | \
		SDRAM_INITPLR_IBA_ENCODE(0x0) | \
		SDRAM_INITPLR_IMA_ENCODE(JEDEC_MA_PRECHARGE_ALL))
#define CONFIG_SYS_SDRAM0_INITPLR7	(SDRAM_INITPLR_ENABLE | \
		SDRAM_INITPLR_IMWT_ENCODE(26) | \
		SDRAM_INITPLR_ICMD_ENCODE(JEDEC_CMD_REFRESH))
#define CONFIG_SYS_SDRAM0_INITPLR8	(SDRAM_INITPLR_ENABLE | \
		SDRAM_INITPLR_IMWT_ENCODE(26) | \
		SDRAM_INITPLR_ICMD_ENCODE(JEDEC_CMD_REFRESH))
#define CONFIG_SYS_SDRAM0_INITPLR9	(SDRAM_INITPLR_ENABLE | \
		SDRAM_INITPLR_IMWT_ENCODE(26) | \
		SDRAM_INITPLR_ICMD_ENCODE(JEDEC_CMD_REFRESH))
#define CONFIG_SYS_SDRAM0_INITPLR10	(SDRAM_INITPLR_ENABLE | \
		SDRAM_INITPLR_IMWT_ENCODE(26) | \
		SDRAM_INITPLR_ICMD_ENCODE(JEDEC_CMD_REFRESH))
#define CONFIG_SYS_SDRAM0_INITPLR11	(SDRAM_INITPLR_ENABLE | \
		SDRAM_INITPLR_IMWT_ENCODE(2) | \
		SDRAM_INITPLR_ICMD_ENCODE(JEDEC_CMD_EMR) | \
		SDRAM_INITPLR_IBA_ENCODE(JEDEC_BA_MR) | \
		SDRAM_INITPLR_IMA_ENCODE(JEDEC_MA_MR_WR_DDR2_3_CYC | \
					 JEDEC_MA_MR_CL_DDR2_5_0_CLK | \
					 JEDEC_MA_MR_BLEN_4))
#define CONFIG_SYS_SDRAM0_INITPLR12	(SDRAM_INITPLR_ENABLE | \
		SDRAM_INITPLR_IMWT_ENCODE(2) | \
		SDRAM_INITPLR_ICMD_ENCODE(JEDEC_CMD_EMR) | \
		SDRAM_INITPLR_IBA_ENCODE(JEDEC_BA_EMR) | \
		SDRAM_INITPLR_IMA_ENCODE(JEDEC_MA_EMR_OCD_ENTER | \
					 JEDEC_MA_EMR_RDQS_DISABLE | \
					 JEDEC_MA_EMR_DQS_DISABLE | \
					 JEDEC_MA_EMR_RTT_DISABLED | \
					 JEDEC_MA_EMR_ODS_NORMAL))
#define CONFIG_SYS_SDRAM0_INITPLR13	(SDRAM_INITPLR_ENABLE | \
		SDRAM_INITPLR_IMWT_ENCODE(2) | \
		SDRAM_INITPLR_ICMD_ENCODE(JEDEC_CMD_EMR) | \
		SDRAM_INITPLR_IBA_ENCODE(JEDEC_BA_EMR) | \
		SDRAM_INITPLR_IMA_ENCODE(JEDEC_MA_EMR_OCD_EXIT | \
					 JEDEC_MA_EMR_RDQS_DISABLE | \
					 JEDEC_MA_EMR_DQS_DISABLE | \
					 JEDEC_MA_EMR_RTT_DISABLED | \
					 JEDEC_MA_EMR_ODS_NORMAL))
#define CONFIG_SYS_SDRAM0_INITPLR14	(SDRAM_INITPLR_DISABLE)
#define CONFIG_SYS_SDRAM0_INITPLR15	(SDRAM_INITPLR_DISABLE)
#define CONFIG_SYS_SDRAM0_RQDC		(SDRAM_RQDC_RQDE_ENABLE | \
				 SDRAM_RQDC_RQFD_ENCODE(56))
#define CONFIG_SYS_SDRAM0_RFDC		SDRAM_RFDC_RFFD_ENCODE(521)
#define CONFIG_SYS_SDRAM0_RDCC		(SDRAM_RDCC_RDSS_T2)
#define CONFIG_SYS_SDRAM0_DLCR		(SDRAM_DLCR_DCLM_AUTO | \
				 SDRAM_DLCR_DLCS_CONT_DONE | \
				 SDRAM_DLCR_DLCV_ENCODE(165))
#define CONFIG_SYS_SDRAM0_CLKTR	(SDRAM_CLKTR_CLKP_180_DEG_ADV)
#define CONFIG_SYS_SDRAM0_WRDTR	0x00000000
#define CONFIG_SYS_SDRAM0_SDTR1	(SDRAM_SDTR1_LDOF_2_CLK | \
				 SDRAM_SDTR1_RTW_2_CLK | \
				 SDRAM_SDTR1_WTWO_1_CLK | \
				 SDRAM_SDTR1_RTRO_1_CLK)
#define CONFIG_SYS_SDRAM0_SDTR2	(SDRAM_SDTR2_RCD_3_CLK | \
				 SDRAM_SDTR2_WTR_2_CLK | \
				 SDRAM_SDTR2_XSNR_32_CLK | \
				 SDRAM_SDTR2_WPC_4_CLK | \
				 SDRAM_SDTR2_RPC_2_CLK | \
				 SDRAM_SDTR2_RP_3_CLK | \
				 SDRAM_SDTR2_RRD_2_CLK)
#define CONFIG_SYS_SDRAM0_SDTR3	(SDRAM_SDTR3_RAS_ENCODE(9) | \
				 SDRAM_SDTR3_RC_ENCODE(12) | \
				 SDRAM_SDTR3_XCS | \
				 SDRAM_SDTR3_RFC_ENCODE(21))
#define CONFIG_SYS_SDRAM0_MMODE	(SDRAM_MMODE_WR_DDR2_3_CYC | \
				 SDRAM_MMODE_DCL_DDR2_5_0_CLK | \
				 SDRAM_MMODE_BLEN_4)
#define CONFIG_SYS_SDRAM0_MEMODE	(SDRAM_MEMODE_DQS_DISABLE | \
				 SDRAM_MEMODE_RTT_75OHM)

/*-----------------------------------------------------------------------
 * I2C
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_I2C_PPC4XX_SPEED_0	400000

#define CONFIG_PCA9698		1	/* NXP PCA9698 */

#define CONFIG_SYS_I2C_EEPROM_ADDR	0x52	/* I2C boot EEPROM (24C02BN) */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1	/* Bytes of address */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10

/* I2C bootstrap EEPROM */
#define CONFIG_4xx_CONFIG_I2C_EEPROM_ADDR	0x54
#define CONFIG_4xx_CONFIG_I2C_EEPROM_OFFSET	0
#define CONFIG_4xx_CONFIG_BLOCKSIZE		16

/* Temp sensor/hwmon/dtt */
#define CONFIG_DTT_LM63		1	/* National LM63 */
#define CONFIG_DTT_SENSORS	{ 0x18, 0x4c, 0x4e }	/* Sensor addresses */
#define CONFIG_DTT_PWM_LOOKUPTABLE	\
		{ { 40, 10 }, { 43, 13 }, { 46, 16 },  \
		  { 50, 20 }, { 53, 27 }, { 56, 34 }, { 60, 40 } }
#define CONFIG_DTT_TACH_LIMIT	0xa10

/*-----------------------------------------------------------------------
 * Ethernet
 *----------------------------------------------------------------------*/
#define CONFIG_M88E1111_PHY	1
#define CONFIG_IBM_EMAC4_V4	1
#define CONFIG_EMAC_PHY_MODE	EMAC_PHY_MODE_RGMII_RGMII
#define CONFIG_PHY_ADDR		0x12	/* PHY address, See schematics */

#define CONFIG_PHY_RESET	1	/* reset phy upon startup */
#define CONFIG_PHY_GIGE		1	/* Include GbE speed/duplex detection */

#define CONFIG_HAS_ETH0		1

#define CONFIG_HAS_ETH1		1	/* add support for "eth1addr"   */
#define CONFIG_PHY1_ADDR	0x13

/* Debug messages for the DDR autocalibration */
#define CONFIG_AUTOCALIB		"silent\0"

/*
 * Default environment variables
 */
#define	CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_AMCC_DEF_ENV						\
	CONFIG_AMCC_DEF_ENV_POWERPC					\
	CONFIG_AMCC_DEF_ENV_PPC_OLD					\
	CONFIG_AMCC_DEF_ENV_NOR_UPD					\
	"logversion=2\0"						\
	"kernel_addr=fc000000\0"					\
	"fdt_addr=fc1e0000\0"						\
	"ramdisk_addr=fc200000\0"					\
	"pciconfighost=1\0"						\
	"pcie_mode=RP:RP\0"						\
	""

/*
 * Commands additional to the ones defined in amcc-common.h
 */
#define CONFIG_CMD_CHIP_CONFIG
#define CONFIG_CMD_DTT

#define CONFIG_SYS_POST_MEMORY_ON	CONFIG_SYS_POST_MEMORY

/* POST support */
#define CONFIG_POST		(CONFIG_SYS_POST_CACHE		| \
				 CONFIG_SYS_POST_CPU		| \
				 CONFIG_SYS_POST_ETHER		| \
				 CONFIG_SYS_POST_I2C		| \
				 CONFIG_SYS_POST_MEMORY_ON	| \
				 CONFIG_SYS_POST_UART)

/* Define here the base-addresses of the UARTs to test in POST */
#define CONFIG_SYS_POST_UART_TABLE	{ CONFIG_SYS_NS16550_COM1, \
			CONFIG_SYS_NS16550_COM2 }

#define CONFIG_LOGBUFFER
#define CONFIG_SYS_POST_CACHE_ADDR	0x00800000 /* free virtual address */

#define CONFIG_SYS_CONSOLE_IS_IN_ENV

/*-----------------------------------------------------------------------
 * External Bus Controller (EBC) Setup
 *----------------------------------------------------------------------*/

/* Memory Bank 0 (NOR-flash) */
#define CONFIG_SYS_EBC_PB0AP	(EBC_BXAP_BME_DISABLED		|	\
				 EBC_BXAP_TWT_ENCODE(11)	|	\
				 EBC_BXAP_BCE_DISABLE		|	\
				 EBC_BXAP_BCT_2TRANS		|	\
				 EBC_BXAP_CSN_ENCODE(0)		|	\
				 EBC_BXAP_OEN_ENCODE(0)		|	\
				 EBC_BXAP_WBN_ENCODE(1)		|	\
				 EBC_BXAP_WBF_ENCODE(2)		|	\
				 EBC_BXAP_TH_ENCODE(2)		|	\
				 EBC_BXAP_RE_DISABLED		|	\
				 EBC_BXAP_SOR_NONDELAYED	|	\
				 EBC_BXAP_BEM_WRITEONLY		|	\
				 EBC_BXAP_PEN_DISABLED)
#define CONFIG_SYS_EBC_PB0CR	(EBC_BXCR_BAS_ENCODE(CONFIG_SYS_FLASH_BASE) | \
				 EBC_BXCR_BS_64MB		|	\
				 EBC_BXCR_BU_RW			|	\
				 EBC_BXCR_BW_16BIT)

/* Memory Bank 1 (NVRAM/Uart) */
#define CONFIG_SYS_EBC_PB1AP	(EBC_BXAP_BME_ENABLED		|	\
				 EBC_BXAP_FWT_ENCODE(8)		|	\
				 EBC_BXAP_BWT_ENCODE(4)		|	\
				 EBC_BXAP_BCE_DISABLE		|	\
				 EBC_BXAP_BCT_2TRANS		|	\
				 EBC_BXAP_CSN_ENCODE(0)		|	\
				 EBC_BXAP_OEN_ENCODE(1)		|	\
				 EBC_BXAP_WBN_ENCODE(1)		|	\
				 EBC_BXAP_WBF_ENCODE(1)		|	\
				 EBC_BXAP_TH_ENCODE(2)		|	\
				 EBC_BXAP_RE_DISABLED		|	\
				 EBC_BXAP_SOR_NONDELAYED	|	\
				 EBC_BXAP_BEM_WRITEONLY		|	\
				 EBC_BXAP_PEN_DISABLED)
#define CONFIG_SYS_EBC_PB1CR	(EBC_BXCR_BAS_ENCODE(CONFIG_SYS_NVRAM_BASE) | \
				 EBC_BXCR_BS_1MB		|	\
				 EBC_BXCR_BU_RW			|	\
				 EBC_BXCR_BW_8BIT)

/* Memory Bank 2 (FPGA) */
#define CONFIG_SYS_EBC_PB2AP	(EBC_BXAP_BME_DISABLED		|	\
				 EBC_BXAP_TWT_ENCODE(5)		|	\
				 EBC_BXAP_BCE_DISABLE		|	\
				 EBC_BXAP_BCT_2TRANS		|	\
				 EBC_BXAP_CSN_ENCODE(0)		|	\
				 EBC_BXAP_OEN_ENCODE(2)		|	\
				 EBC_BXAP_WBN_ENCODE(1)		|	\
				 EBC_BXAP_WBF_ENCODE(1)		|	\
				 EBC_BXAP_TH_ENCODE(0)		|	\
				 EBC_BXAP_RE_DISABLED		|	\
				 EBC_BXAP_SOR_NONDELAYED	|	\
				 EBC_BXAP_BEM_WRITEONLY		|	\
				 EBC_BXAP_PEN_DISABLED)
#define CONFIG_SYS_EBC_PB2CR	(EBC_BXCR_BAS_ENCODE(CONFIG_SYS_FPGA0_BASE) | \
				 EBC_BXCR_BS_1MB		|	\
				 EBC_BXCR_BU_RW			|	\
				 EBC_BXCR_BW_16BIT)

/* Memory Bank 3 (Latches) */
#define CONFIG_SYS_EBC_PB3AP	(EBC_BXAP_BME_ENABLED		|	\
				 EBC_BXAP_FWT_ENCODE(8)		|	\
				 EBC_BXAP_BWT_ENCODE(4)		|	\
				 EBC_BXAP_BCE_DISABLE		|	\
				 EBC_BXAP_BCT_2TRANS		|	\
				 EBC_BXAP_CSN_ENCODE(0)		|	\
				 EBC_BXAP_OEN_ENCODE(1)		|	\
				 EBC_BXAP_WBN_ENCODE(1)		|	\
				 EBC_BXAP_WBF_ENCODE(1)		|	\
				 EBC_BXAP_TH_ENCODE(2)		|	\
				 EBC_BXAP_RE_DISABLED		|	\
				 EBC_BXAP_SOR_NONDELAYED	|	\
				 EBC_BXAP_BEM_WRITEONLY		|	\
				 EBC_BXAP_PEN_DISABLED)
#define CONFIG_SYS_EBC_PB3CR	(EBC_BXCR_BAS_ENCODE(CONFIG_SYS_LATCH_BASE) | \
				 EBC_BXCR_BS_1MB		|	\
				 EBC_BXCR_BU_RW			|	\
				 EBC_BXCR_BW_16BIT)

/* EBC peripherals */

#define CONFIG_SYS_FPGA_BASE(k) \
	(k ? CONFIG_SYS_FPGA1_BASE : CONFIG_SYS_FPGA0_BASE)

#define CONFIG_SYS_FPGA_DONE(k) \
	(k ? 0x0040 : 0x0080)

#define CONFIG_SYS_FPGA_COUNT		2

#define CONFIG_SYS_FPGA_PTR { \
	(struct ihs_fpga *)CONFIG_SYS_FPGA0_BASE, \
	(struct ihs_fpga *)CONFIG_SYS_FPGA1_BASE }

#define CONFIG_SYS_FPGA_COMMON

#define CONFIG_SYS_LATCH0_RESET		0xffff
#define CONFIG_SYS_LATCH0_BOOT		0xffff
#define CONFIG_SYS_LATCH1_RESET		0xffbf
#define CONFIG_SYS_LATCH1_BOOT		0xffff

/*-----------------------------------------------------------------------
 * GPIO Setup
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_4xx_GPIO_TABLE { /*	  Out		GPIO */ \
{ \
/* GPIO Core 0 */ \
{GPIO0_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_1     }, /* GPIO0 */ \
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_NO_CHG}, /* GPIO1 */ \
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_NO_CHG}, /* GPIO2 */ \
{GPIO0_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_1     }, /* GPIO3 */ \
{GPIO0_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_NO_CHG}, /* GPIO4 */ \
{GPIO0_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_NO_CHG}, /* GPIO5 */ \
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_NO_CHG}, /* GPIO6 */ \
{GPIO0_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_1     }, /* GPIO7 */ \
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0     }, /* GPIO8 */ \
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0     }, /* GPIO9 */ \
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0     }, /* GPIO10 */ \
{GPIO0_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_1     }, /* GPIO11 */ \
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_NO_CHG}, /* GPIO12 */ \
{GPIO0_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_1     }, /* GPIO13 */ \
{GPIO0_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_NO_CHG}, /* GPIO14 */ \
{GPIO0_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_NO_CHG}, /* GPIO15 */ \
{GPIO0_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_0     }, /* GPIO16 */ \
{GPIO0_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_0     }, /* GPIO17 */ \
{GPIO0_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_0     }, /* GPIO18 */ \
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0     }, /* GPIO19 */ \
{GPIO0_BASE, GPIO_OUT, GPIO_ALT2, GPIO_OUT_0     }, /* GPIO20 */ \
{GPIO0_BASE, GPIO_IN,  GPIO_ALT2, GPIO_OUT_0     }, /* GPIO21 */ \
{GPIO0_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_NO_CHG}, /* GPIO22 */ \
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_NO_CHG}, /* GPIO23 */ \
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0     }, /* GPIO24 */ \
{GPIO0_BASE, GPIO_IN,  GPIO_ALT3, GPIO_OUT_0     }, /* GPIO25 */ \
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0     }, /* GPIO26 */ \
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0     }, /* GPIO27 */ \
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0     }, /* GPIO28 */ \
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0     }, /* GPIO29 */ \
{GPIO0_BASE, GPIO_IN,  GPIO_ALT2, GPIO_OUT_0     }, /* GPIO30 */ \
{GPIO0_BASE, GPIO_IN,  GPIO_ALT2, GPIO_OUT_0     }, /* GPIO31 */ \
} \
}

#define CONFIG_SYS_GPIO_STARTUP_FINISHED	15
#define CONFIG_SYS_GPIO_STARTUP_FINISHED_N	14

#endif	/* __CONFIG_H */

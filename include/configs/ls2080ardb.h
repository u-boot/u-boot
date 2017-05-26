/*
 * Copyright 2017 NXP
 * Copyright 2015 Freescale Semiconductor
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __LS2_RDB_H
#define __LS2_RDB_H

#include "ls2080a_common.h"

#undef CONFIG_CONS_INDEX
#define CONFIG_CONS_INDEX       2

#ifdef CONFIG_FSL_QSPI
#ifdef CONFIG_TARGET_LS2081ARDB
#define CONFIG_QIXIS_I2C_ACCESS
#endif
#define CONFIG_SYS_I2C_EARLY_INIT
#define CONFIG_DISPLAY_BOARDINFO_LATE
#endif

#define I2C_MUX_CH_VOL_MONITOR		0xa
#define I2C_VOL_MONITOR_ADDR		0x38
#define CONFIG_VOL_MONITOR_IR36021_READ
#define CONFIG_VOL_MONITOR_IR36021_SET

#define CONFIG_VID_FLS_ENV		"ls2080ardb_vdd_mv"
#ifndef CONFIG_SPL_BUILD
#define CONFIG_VID
#endif
/* step the IR regulator in 5mV increments */
#define IR_VDD_STEP_DOWN		5
#define IR_VDD_STEP_UP			5
/* The lowest and highest voltage allowed for LS2080ARDB */
#define VDD_MV_MIN			819
#define VDD_MV_MAX			1212

#ifndef __ASSEMBLY__
unsigned long get_board_sys_clk(void);
#endif

#define CONFIG_SYS_CLK_FREQ		get_board_sys_clk()
#define CONFIG_DDR_CLK_FREQ		133333333
#define COUNTER_FREQUENCY_REAL		(CONFIG_SYS_CLK_FREQ/4)

#define CONFIG_DDR_SPD
#define CONFIG_DDR_ECC
#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER
#define CONFIG_MEM_INIT_VALUE		0xdeadbeef
#define SPD_EEPROM_ADDRESS1	0x51
#define SPD_EEPROM_ADDRESS2	0x52
#define SPD_EEPROM_ADDRESS3	0x53
#define SPD_EEPROM_ADDRESS4	0x54
#define SPD_EEPROM_ADDRESS5	0x55
#define SPD_EEPROM_ADDRESS6	0x56	/* dummy address */
#define SPD_EEPROM_ADDRESS	SPD_EEPROM_ADDRESS1
#define CONFIG_SYS_SPD_BUS_NUM	0	/* SPD on I2C bus 0 */
#define CONFIG_DIMM_SLOTS_PER_CTLR		2
#define CONFIG_CHIP_SELECTS_PER_CTRL		4
#ifdef CONFIG_SYS_FSL_HAS_DP_DDR
#define CONFIG_DP_DDR_DIMM_SLOTS_PER_CTLR	1
#endif
#define CONFIG_FSL_DDR_BIST	/* enable built-in memory test */

/* SATA */
#define CONFIG_LIBATA
#define CONFIG_SCSI_AHCI
#define CONFIG_SCSI_AHCI_PLAT
#define CONFIG_SCSI

#define CONFIG_SYS_SATA1			AHCI_BASE_ADDR1
#define CONFIG_SYS_SATA2			AHCI_BASE_ADDR2

#define CONFIG_SYS_SCSI_MAX_SCSI_ID		1
#define CONFIG_SYS_SCSI_MAX_LUN			1
#define CONFIG_SYS_SCSI_MAX_DEVICE		(CONFIG_SYS_SCSI_MAX_SCSI_ID * \
						CONFIG_SYS_SCSI_MAX_LUN)

#ifndef CONFIG_FSL_QSPI
/* undefined CONFIG_FSL_DDR_SYNC_REFRESH for simulator */

#define CONFIG_SYS_NOR0_CSPR_EXT	(0x0)
#define CONFIG_SYS_NOR_AMASK		IFC_AMASK(128*1024*1024)
#define CONFIG_SYS_NOR_AMASK_EARLY	IFC_AMASK(64*1024*1024)

#define CONFIG_SYS_NOR0_CSPR					\
	(CSPR_PHYS_ADDR(CONFIG_SYS_FLASH_BASE_PHYS)		| \
	CSPR_PORT_SIZE_16					| \
	CSPR_MSEL_NOR						| \
	CSPR_V)
#define CONFIG_SYS_NOR0_CSPR_EARLY				\
	(CSPR_PHYS_ADDR(CONFIG_SYS_FLASH_BASE_PHYS_EARLY)	| \
	CSPR_PORT_SIZE_16					| \
	CSPR_MSEL_NOR						| \
	CSPR_V)
#define CONFIG_SYS_NOR_CSOR	CSOR_NOR_ADM_SHIFT(12)
#define CONFIG_SYS_NOR_FTIM0	(FTIM0_NOR_TACSE(0x4) | \
				FTIM0_NOR_TEADC(0x5) | \
				FTIM0_NOR_TEAHC(0x5))
#define CONFIG_SYS_NOR_FTIM1	(FTIM1_NOR_TACO(0x35) | \
				FTIM1_NOR_TRAD_NOR(0x1a) |\
				FTIM1_NOR_TSEQRAD_NOR(0x13))
#define CONFIG_SYS_NOR_FTIM2	(FTIM2_NOR_TCS(0x4) | \
				FTIM2_NOR_TCH(0x4) | \
				FTIM2_NOR_TWPH(0x0E) | \
				FTIM2_NOR_TWP(0x1c))
#define CONFIG_SYS_NOR_FTIM3	0x04000000
#define CONFIG_SYS_IFC_CCR	0x01000000

#ifdef CONFIG_MTD_NOR_FLASH
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE
#define CONFIG_SYS_FLASH_QUIET_TEST
#define CONFIG_FLASH_SHOW_PROGRESS	45 /* count down from 45/5: 9..1 */

#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* number of banks */
#define CONFIG_SYS_MAX_FLASH_SECT	1024	/* sectors per device */
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */

#define CONFIG_SYS_FLASH_EMPTY_INFO
#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE,\
					 CONFIG_SYS_FLASH_BASE + 0x40000000}
#endif

#define CONFIG_NAND_FSL_IFC
#define CONFIG_SYS_NAND_MAX_ECCPOS	256
#define CONFIG_SYS_NAND_MAX_OOBFREE	2

#define CONFIG_SYS_NAND_CSPR_EXT	(0x0)
#define CONFIG_SYS_NAND_CSPR	(CSPR_PHYS_ADDR(CONFIG_SYS_NAND_BASE_PHYS) \
				| CSPR_PORT_SIZE_8 /* Port Size = 8 bit */ \
				| CSPR_MSEL_NAND	/* MSEL = NAND */ \
				| CSPR_V)
#define CONFIG_SYS_NAND_AMASK	IFC_AMASK(64 * 1024)

#define CONFIG_SYS_NAND_CSOR    (CSOR_NAND_ECC_ENC_EN   /* ECC on encode */ \
				| CSOR_NAND_ECC_DEC_EN  /* ECC on decode */ \
				| CSOR_NAND_ECC_MODE_4  /* 4-bit ECC */ \
				| CSOR_NAND_RAL_3	/* RAL = 3Byes */ \
				| CSOR_NAND_PGS_4K	/* Page Size = 4K */ \
				| CSOR_NAND_SPRZ_224	/* Spare size = 224 */ \
				| CSOR_NAND_PB(128))	/* Pages Per Block 128*/

#define CONFIG_SYS_NAND_ONFI_DETECTION

/* ONFI NAND Flash mode0 Timing Params */
#define CONFIG_SYS_NAND_FTIM0		(FTIM0_NAND_TCCST(0x0e) | \
					FTIM0_NAND_TWP(0x30)   | \
					FTIM0_NAND_TWCHT(0x0e) | \
					FTIM0_NAND_TWH(0x14))
#define CONFIG_SYS_NAND_FTIM1		(FTIM1_NAND_TADLE(0x64) | \
					FTIM1_NAND_TWBE(0xab)  | \
					FTIM1_NAND_TRR(0x1c)   | \
					FTIM1_NAND_TRP(0x30))
#define CONFIG_SYS_NAND_FTIM2		(FTIM2_NAND_TRAD(0x1e) | \
					FTIM2_NAND_TREH(0x14) | \
					FTIM2_NAND_TWHRE(0x3c))
#define CONFIG_SYS_NAND_FTIM3		0x0

#define CONFIG_SYS_NAND_BASE_LIST	{ CONFIG_SYS_NAND_BASE }
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_MTD_NAND_VERIFY_WRITE
#define CONFIG_CMD_NAND

#define CONFIG_SYS_NAND_BLOCK_SIZE	(512 * 1024)
#define CONFIG_FSL_QIXIS	/* use common QIXIS code */
#define QIXIS_LBMAP_SWITCH		0x06
#define QIXIS_LBMAP_MASK		0x0f
#define QIXIS_LBMAP_SHIFT		0
#define QIXIS_LBMAP_DFLTBANK		0x00
#define QIXIS_LBMAP_ALTBANK		0x04
#define QIXIS_LBMAP_NAND		0x09
#define QIXIS_RST_CTL_RESET		0x31
#define QIXIS_RST_CTL_RESET_EN		0x30
#define QIXIS_RCFG_CTL_RECONFIG_IDLE	0x20
#define QIXIS_RCFG_CTL_RECONFIG_START	0x21
#define QIXIS_RCFG_CTL_WATCHDOG_ENBLE	0x08
#define QIXIS_RCW_SRC_NAND		0x119
#define	QIXIS_RST_FORCE_MEM		0x01

#define CONFIG_SYS_CSPR3_EXT	(0x0)
#define CONFIG_SYS_CSPR3	(CSPR_PHYS_ADDR(QIXIS_BASE_PHYS_EARLY) \
				| CSPR_PORT_SIZE_8 \
				| CSPR_MSEL_GPCM \
				| CSPR_V)
#define CONFIG_SYS_CSPR3_FINAL	(CSPR_PHYS_ADDR(QIXIS_BASE_PHYS) \
				| CSPR_PORT_SIZE_8 \
				| CSPR_MSEL_GPCM \
				| CSPR_V)

#define CONFIG_SYS_AMASK3	IFC_AMASK(64*1024)
#define CONFIG_SYS_CSOR3	CSOR_GPCM_ADM_SHIFT(12)
/* QIXIS Timing parameters for IFC CS3 */
#define CONFIG_SYS_CS3_FTIM0		(FTIM0_GPCM_TACSE(0x0e) | \
					FTIM0_GPCM_TEADC(0x0e) | \
					FTIM0_GPCM_TEAHC(0x0e))
#define CONFIG_SYS_CS3_FTIM1		(FTIM1_GPCM_TACO(0xff) | \
					FTIM1_GPCM_TRAD(0x3f))
#define CONFIG_SYS_CS3_FTIM2		(FTIM2_GPCM_TCS(0xf) | \
					FTIM2_GPCM_TCH(0xf) | \
					FTIM2_GPCM_TWP(0x3E))
#define CONFIG_SYS_CS3_FTIM3		0x0

#if defined(CONFIG_SPL) && defined(CONFIG_NAND)
#define CONFIG_SYS_CSPR2_EXT		CONFIG_SYS_NOR0_CSPR_EXT
#define CONFIG_SYS_CSPR2		CONFIG_SYS_NOR0_CSPR_EARLY
#define CONFIG_SYS_CSPR2_FINAL		CONFIG_SYS_NOR0_CSPR
#define CONFIG_SYS_AMASK2		CONFIG_SYS_NOR_AMASK
#define CONFIG_SYS_CSOR2		CONFIG_SYS_NOR_CSOR
#define CONFIG_SYS_CS2_FTIM0		CONFIG_SYS_NOR_FTIM0
#define CONFIG_SYS_CS2_FTIM1		CONFIG_SYS_NOR_FTIM1
#define CONFIG_SYS_CS2_FTIM2		CONFIG_SYS_NOR_FTIM2
#define CONFIG_SYS_CS2_FTIM3		CONFIG_SYS_NOR_FTIM3
#define CONFIG_SYS_CSPR0_EXT		CONFIG_SYS_NAND_CSPR_EXT
#define CONFIG_SYS_CSPR0		CONFIG_SYS_NAND_CSPR
#define CONFIG_SYS_AMASK0		CONFIG_SYS_NAND_AMASK
#define CONFIG_SYS_CSOR0		CONFIG_SYS_NAND_CSOR
#define CONFIG_SYS_CS0_FTIM0		CONFIG_SYS_NAND_FTIM0
#define CONFIG_SYS_CS0_FTIM1		CONFIG_SYS_NAND_FTIM1
#define CONFIG_SYS_CS0_FTIM2		CONFIG_SYS_NAND_FTIM2
#define CONFIG_SYS_CS0_FTIM3		CONFIG_SYS_NAND_FTIM3

#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET		(2048 * 1024)
#define CONFIG_ENV_SECT_SIZE		0x20000
#define CONFIG_ENV_SIZE			0x2000
#define CONFIG_SPL_PAD_TO		0x80000
#define CONFIG_SYS_NAND_U_BOOT_OFFS	(1024 * 1024)
#define CONFIG_SYS_NAND_U_BOOT_SIZE	(512 * 1024)
#else
#define CONFIG_SYS_CSPR0_EXT		CONFIG_SYS_NOR0_CSPR_EXT
#define CONFIG_SYS_CSPR0		CONFIG_SYS_NOR0_CSPR_EARLY
#define CONFIG_SYS_CSPR0_FINAL		CONFIG_SYS_NOR0_CSPR
#define CONFIG_SYS_AMASK0		CONFIG_SYS_NOR_AMASK
#define CONFIG_SYS_CSOR0		CONFIG_SYS_NOR_CSOR
#define CONFIG_SYS_CS0_FTIM0		CONFIG_SYS_NOR_FTIM0
#define CONFIG_SYS_CS0_FTIM1		CONFIG_SYS_NOR_FTIM1
#define CONFIG_SYS_CS0_FTIM2		CONFIG_SYS_NOR_FTIM2
#define CONFIG_SYS_CS0_FTIM3		CONFIG_SYS_NOR_FTIM3
#define CONFIG_SYS_CSPR2_EXT		CONFIG_SYS_NAND_CSPR_EXT
#define CONFIG_SYS_CSPR2		CONFIG_SYS_NAND_CSPR
#define CONFIG_SYS_AMASK2		CONFIG_SYS_NAND_AMASK
#define CONFIG_SYS_CSOR2		CONFIG_SYS_NAND_CSOR
#define CONFIG_SYS_CS2_FTIM0		CONFIG_SYS_NAND_FTIM0
#define CONFIG_SYS_CS2_FTIM1		CONFIG_SYS_NAND_FTIM1
#define CONFIG_SYS_CS2_FTIM2		CONFIG_SYS_NAND_FTIM2
#define CONFIG_SYS_CS2_FTIM3		CONFIG_SYS_NAND_FTIM3

#define CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_ADDR			(CONFIG_SYS_FLASH_BASE + 0x300000)
#define CONFIG_ENV_SECT_SIZE		0x20000
#define CONFIG_ENV_SIZE			0x2000
#endif

/* Debug Server firmware */
#define CONFIG_SYS_DEBUG_SERVER_FW_IN_NOR
#define CONFIG_SYS_DEBUG_SERVER_FW_ADDR	0x580D00000ULL
#endif
#define CONFIG_SYS_LS_MC_BOOT_TIMEOUT_MS 5000

#ifdef CONFIG_TARGET_LS2081ARDB
#define CONFIG_FSL_QIXIS	/* use common QIXIS code */
#define QIXIS_QMAP_MASK			0x07
#define QIXIS_QMAP_SHIFT		5
#define QIXIS_LBMAP_DFLTBANK		0x00
#define QIXIS_LBMAP_QSPI		0x00
#define QIXIS_RCW_SRC_QSPI		0x62
#define QIXIS_LBMAP_ALTBANK		0x20
#define QIXIS_RST_CTL_RESET		0x31
#define QIXIS_RCFG_CTL_RECONFIG_IDLE	0x20
#define QIXIS_RCFG_CTL_RECONFIG_START	0x21
#define QIXIS_RCFG_CTL_WATCHDOG_ENBLE	0x08
#define QIXIS_LBMAP_MASK		0x0f
#define QIXIS_RST_CTL_RESET_EN		0x30
#endif

/*
 * I2C
 */
#ifdef CONFIG_TARGET_LS2081ARDB
#define CONFIG_SYS_I2C_FPGA_ADDR	0x66
#endif
#define I2C_MUX_PCA_ADDR		0x75
#define I2C_MUX_PCA_ADDR_PRI		0x75 /* Primary Mux*/

/* I2C bus multiplexer */
#define I2C_MUX_CH_DEFAULT      0x8

/* SPI */
#if defined(CONFIG_FSL_QSPI) || defined(CONFIG_FSL_DSPI)
#define CONFIG_SPI_FLASH
#ifdef CONFIG_FSL_QSPI
#define CONFIG_SPI_FLASH_STMICRO
#endif
#ifdef CONFIG_FSL_QSPI
#ifdef CONFIG_TARGET_LS2081ARDB
#define CONFIG_SPI_FLASH_STMICRO
#else
#define CONFIG_SPI_FLASH_SPANSION
#endif
#define FSL_QSPI_FLASH_SIZE		SZ_64M	/* 64MB */
#define FSL_QSPI_FLASH_NUM		2
#endif
#endif

/*
 * RTC configuration
 */
#define RTC
#ifdef CONFIG_TARGET_LS2081ARDB
#define CONFIG_RTC_PCF8563		1
#define CONFIG_SYS_I2C_RTC_ADDR         0x51
#else
#define CONFIG_RTC_DS3231               1
#define CONFIG_SYS_I2C_RTC_ADDR         0x68
#endif

/* EEPROM */
#define CONFIG_ID_EEPROM
#define CONFIG_SYS_I2C_EEPROM_NXID
#define CONFIG_SYS_EEPROM_BUS_NUM	0
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x57
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS 5

#define CONFIG_FSL_MEMAC

#ifdef CONFIG_PCI
#define CONFIG_PCI_SCAN_SHOW
#define CONFIG_CMD_PCI
#endif

/*  MMC  */
#ifdef CONFIG_MMC
#define CONFIG_FSL_ESDHC
#define CONFIG_SYS_FSL_MMC_HAS_CAPBLT_VS33
#endif

#define CONFIG_MISC_INIT_R

/*
 * USB
 */
#define CONFIG_HAS_FSL_XHCI_USB
#define CONFIG_USB_XHCI_FSL
#define CONFIG_USB_MAX_CONTROLLER_COUNT         2
#define CONFIG_SYS_USB_XHCI_MAX_ROOT_PORTS      2

#undef CONFIG_CMDLINE_EDITING
#include <config_distro_defaults.h>

#define BOOT_TARGET_DEVICES(func) \
	func(USB, usb, 0) \
	func(MMC, mmc, 0) \
	func(SCSI, scsi, 0) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>

/* Initial environment variables */
#undef CONFIG_EXTRA_ENV_SETTINGS
#ifdef CONFIG_SECURE_BOOT
#define CONFIG_EXTRA_ENV_SETTINGS		\
	"hwconfig=fsl_ddr:bank_intlv=auto\0"	\
	"scriptaddr=0x80800000\0"		\
	"kernel_addr_r=0x81000000\0"		\
	"pxefile_addr_r=0x81000000\0"		\
	"fdt_addr_r=0x88000000\0"		\
	"ramdisk_addr_r=0x89000000\0"		\
	"loadaddr=0x80100000\0"			\
	"kernel_addr=0x100000\0"		\
	"ramdisk_addr=0x800000\0"		\
	"ramdisk_size=0x2000000\0"		\
	"fdt_high=0xa0000000\0"			\
	"initrd_high=0xffffffffffffffff\0"	\
	"kernel_start=0x581000000\0"		\
	"kernel_load=0xa0000000\0"		\
	"kernel_size=0x2800000\0"		\
	"mcmemsize=0x40000000\0"		\
	"fdtfile=fsl-ls2080a-rdb.dtb\0"		\
	"mcinitcmd=esbc_validate 0x580700000;"  \
	"esbc_validate 0x580740000;"            \
	"fsl_mc start mc 0x580a00000"           \
	" 0x580e00000 \0"                       \
	BOOTENV
#else
#ifdef CONFIG_QSPI_BOOT
#define CONFIG_EXTRA_ENV_SETTINGS		\
	"hwconfig=fsl_ddr:bank_intlv=auto\0"	\
	"scriptaddr=0x80800000\0"		\
	"kernel_addr_r=0x81000000\0"		\
	"pxefile_addr_r=0x81000000\0"		\
	"fdt_addr_r=0x88000000\0"		\
	"ramdisk_addr_r=0x89000000\0"		\
	"loadaddr=0x80100000\0"			\
	"kernel_addr=0x100000\0"		\
	"ramdisk_size=0x2000000\0"		\
	"fdt_high=0xa0000000\0"			\
	"initrd_high=0xffffffffffffffff\0"	\
	"kernel_start=0x21000000\0"		\
	"mcmemsize=0x40000000\0"		\
	"mcinitcmd=fsl_mc start mc 0x20a00000" \
	" 0x20e00000 \0"                       \
	BOOTENV
#else
#define CONFIG_EXTRA_ENV_SETTINGS		\
	"hwconfig=fsl_ddr:bank_intlv=auto\0"	\
	"scriptaddr=0x80800000\0"		\
	"kernel_addr_r=0x81000000\0"		\
	"pxefile_addr_r=0x81000000\0"		\
	"fdt_addr_r=0x88000000\0"		\
	"ramdisk_addr_r=0x89000000\0"		\
	"loadaddr=0x80100000\0"			\
	"kernel_addr=0x100000\0"		\
	"ramdisk_addr=0x800000\0"		\
	"ramdisk_size=0x2000000\0"		\
	"fdt_high=0xa0000000\0"			\
	"initrd_high=0xffffffffffffffff\0"	\
	"kernel_start=0x581000000\0"		\
	"kernel_load=0xa0000000\0"		\
	"kernel_size=0x2800000\0"		\
	"mcmemsize=0x40000000\0"		\
	"fdtfile=fsl-ls2080a-rdb.dtb\0"		\
	"mcinitcmd=fsl_mc start mc 0x580a00000" \
	" 0x580e00000 \0"                       \
	BOOTENV
#endif
#endif


#undef CONFIG_BOOTARGS
#define CONFIG_BOOTARGS		"console=ttyS1,115200 root=/dev/ram0 " \
				"earlycon=uart8250,mmio,0x21c0600 " \
				"ramdisk_size=0x2000000 default_hugepagesz=2m" \
				" hugepagesz=2m hugepages=256"

#undef CONFIG_BOOTCOMMAND
#ifdef CONFIG_QSPI_BOOT
/* Try to boot an on-QSPI kernel first, then do normal distro boot */
#define CONFIG_BOOTCOMMAND "run mcinitcmd && fsl_mc lazyapply dpl 0x20d00000" \
			   " && bootm $kernel_start" \
			   " || run distro_bootcmd"
#else
/* Try to boot an on-NOR kernel first, then do normal distro boot */
#define CONFIG_BOOTCOMMAND "run mcinitcmd && fsl_mc lazyapply dpl 0x580d00000" \
			   " && cp.b $kernel_start $kernel_load $kernel_size" \
			   " && bootm $kernel_load" \
			   " || run distro_bootcmd"
#endif

/* MAC/PHY configuration */
#ifdef CONFIG_FSL_MC_ENET
#define CONFIG_PHYLIB_10G
#define CONFIG_PHY_AQUANTIA
#define CONFIG_PHY_CORTINA
#define CONFIG_PHYLIB
#define	CONFIG_SYS_CORTINA_FW_IN_NOR
#ifdef CONFIG_QSPI_BOOT
#define CONFIG_CORTINA_FW_ADDR		0x20980000
#else
#define CONFIG_CORTINA_FW_ADDR		0x580980000
#endif
#define CONFIG_CORTINA_FW_LENGTH	0x40000

#define CORTINA_PHY_ADDR1	0x10
#define CORTINA_PHY_ADDR2	0x11
#define CORTINA_PHY_ADDR3	0x12
#define CORTINA_PHY_ADDR4	0x13
#define AQ_PHY_ADDR1		0x00
#define AQ_PHY_ADDR2		0x01
#define AQ_PHY_ADDR3		0x02
#define AQ_PHY_ADDR4		0x03
#define AQR405_IRQ_MASK		0x36

#define CONFIG_MII
#define CONFIG_ETHPRIME		"DPMAC1@xgmii"
#define CONFIG_PHY_GIGE
#define CONFIG_PHY_AQUANTIA
#endif

#include <asm/fsl_secure_boot.h>

#endif /* __LS2_RDB_H */

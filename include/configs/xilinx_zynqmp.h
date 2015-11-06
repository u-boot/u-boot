/*
 * Configuration for Xilinx ZynqMP
 * (C) Copyright 2014 - 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 *
 * Based on Configuration for Versatile Express
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __XILINX_ZYNQMP_H
#define __XILINX_ZYNQMP_H

#define CONFIG_REMAKE_ELF

/* #define CONFIG_ARMV8_SWITCH_TO_EL1 */

#define CONFIG_SYS_NO_FLASH

#define CONFIG_SYS_GENERIC_BOARD

/* Generic Interrupt Controller Definitions */
#define CONFIG_GICV2
#define GICD_BASE	0xF9010000
#define GICC_BASE	0xF9020000

#define CONFIG_SYS_ALT_MEMTEST
#define CONFIG_SYS_MEMTEST_SCRATCH	0xfffc0000

#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		CONFIG_SYS_SDRAM_SIZE

/* Have release address at the end of 256MB for now */
#define CPU_RELEASE_ADDR	0xFFFFFF0

/* Cache Definitions */
#define CONFIG_SYS_CACHELINE_SIZE	64

#ifndef CONFIG_IDENT_STRING
#define CONFIG_IDENT_STRING		" Xilinx ZynqMP"
#endif
#define CONFIG_BOOTP_VCI_STRING		"U-boot.armv8.Xilinx_ZynqMP"

/* Text base on 16MB for now - 0 doesn't work */
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x7fff0)

/* Flat Device Tree Definitions */
#define CONFIG_OF_LIBFDT

/* Generic Timer Definitions - setup in EL3. Setup by ATF for other cases */
#ifndef COUNTER_FREQUENCY
# define COUNTER_FREQUENCY		100000000
#endif

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 0x2000000)

/* Serial setup */
#if defined(CONFIG_ZYNQ_DCC)
# define CONFIG_ARM_DCC
# define CONFIG_CPU_ARMV8
# define CONFIG_BOOTARGS	"setenv bootargs console=hvc0 " \
				"earlycon=dcc; run nosmp;"
#else
# if defined(CONFIG_ZYNQ_SERIAL_UART0) || defined(CONFIG_ZYNQ_SERIAL_UART1)
#  define CONFIG_ZYNQ_SERIAL
#  define CONFIG_BOOTARGS	"setenv bootargs console=ttyPS0,${baudrate} " \
				"earlycon=cdns,mmio,0xff000000,${baudrate}n8"
# endif
#endif

#define CONFIG_CONS_INDEX		0
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE \
	{ 4800, 9600, 19200, 38400, 57600, 115200 }

/* Command line configuration */
#define CONFIG_CMD_ENV
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_EXT4
#define CONFIG_CMD_FAT
#define CONFIG_CMD_FS_GENERIC
#define CONFIG_DOS_PARTITION
#define CONFIG_CMD_ELF
#define CONFIG_MP

#define CONFIG_CMD_MII

/* BOOTP options */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_MAY_FAIL
#define CONFIG_BOOTP_SERVERIP

/* SPI */
#ifdef CONFIG_ZYNQ_SPI
# define CONFIG_SPI_FLASH_SST
# define CONFIG_CMD_SPI
# define CONFIG_CMD_SF
#endif

#ifdef CONFIG_ZYNQMP_QSPI
# define CONFIG_SPI_GENERIC
# define CONFIG_SF_DEFAULT_SPEED	30000000
# define CONFIG_SF_DUAL_FLASH
# define CONFIG_SPI_FLASH_SPANSION
# define CONFIG_SPI_FLASH_STMICRO
# define CONFIG_SPI_FLASH_WINBOND
# define CONFIG_CMD_SPI
# define CONFIG_CMD_SF
#endif

/* NAND */
#ifdef CONFIG_NAND_ARASAN
# define CONFIG_CMD_NAND
# define CONFIG_CMD_NAND_LOCK_UNLOCK
# define CONFIG_SYS_MAX_NAND_DEVICE	1
# define CONFIG_SYS_NAND_SELF_INIT
# define CONFIG_SYS_NAND_ONFI_DETECTION
# define CONFIG_MTD_DEVICE
#endif

#if defined(CONFIG_ZYNQ_SDHCI0) || defined(CONFIG_ZYNQ_SDHCI1)
# define CONFIG_MMC
# define CONFIG_GENERIC_MMC
# define CONFIG_SDHCI
# define CONFIG_ZYNQ_SDHCI
# define CONFIG_CMD_MMC
# ifndef CONFIG_ZYNQ_SDHCI_MAX_FREQ
#  define CONFIG_ZYNQ_SDHCI_MAX_FREQ	200000000
# endif
#endif

#if defined(CONFIG_ZYNQ_SDHCI) || defined(CONFIG_ZYNQ_USB)
# define CONFIG_FAT_WRITE
# define CONFIG_CMD_EXT4_WRITE
#endif

/* Miscellaneous configurable options */
#define CONFIG_SYS_LOAD_ADDR		0x8000000

#if defined(CONFIG_ZYNQMP_USB)
# define CONFIG_USB_XHCI_DWC3
# define CONFIG_USB_XHCI
# define CONFIG_USB_MAX_CONTROLLER_COUNT	1
# define CONFIG_SYS_USB_XHCI_MAX_ROOT_PORTS	2
# define CONFIG_CMD_USB
# define CONFIG_USB_STORAGE
# define CONFIG_USB_XHCI_ZYNQMP

# define CONFIG_USB_DWC3
# define CONFIG_USB_DWC3_GADGET

# define CONFIG_USB_GADGET
# define CONFIG_USB_GADGET_DUALSPEED
# define CONFIG_USB_GADGET_VBUS_DRAW	2
# define CONFIG_USBDOWNLOAD_GADGET
# define CONFIG_SYS_DFU_DATA_BUF_SIZE	0x1800000
# define DFU_DEFAULT_POLL_TIMEOUT	300
# define CONFIG_DFU_FUNCTION
# define CONFIG_DFU_RAM
# define CONFIG_G_DNL_VENDOR_NUM	0x03FD
# define CONFIG_G_DNL_PRODUCT_NUM	0x0300
# define CONFIG_G_DNL_MANUFACTURER	"Xilinx"
# define CONFIG_USB_CABLE_CHECK
# define CONFIG_CMD_DFU
# define CONFIG_CMD_THOR_DOWNLOAD
# define CONFIG_THOR_FUNCTION
# define CONFIG_THOR_RESET_OFF
# define DFU_ALT_INFO_RAM \
	"dfu_ram_info=" \
	"setenv dfu_alt_info " \
	"Image ram $kernel_addr $kernel_size\\\\;" \
	"system.dtb ram $fdt_addr $fdt_size\0" \
	"dfu_ram=run dfu_ram_info && dfu 0 ram 0\0" \
	"thor_ram=run dfu_ram_info && thordown 0 ram 0\0"

# define DFU_ALT_INFO  \
		DFU_ALT_INFO_RAM
#endif

#if !defined(DFU_ALT_INFO)
# define DFU_ALT_INFO
#endif

/* Initial environment variables */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"ethaddr=00:0a:35:00:01:22\0" \
	"kernel_addr=0x200000\0" \
	"initrd_addr=0xa00000\0" \
	"initrd_size=0x2000000\0" \
	"fdt_addr=0x7000000\0" \
	"fdt_high=0x10000000\0" \
	"sdbootdev=0\0"\
	CONFIG_KERNEL_FDT_OFST_SIZE \
	"sata_root=if test $scsidevs -gt 0; then setenv bootargs $bootargs root=/dev/sda rw rootfstype=ext4; fi\0" \
	"veloce=fdt addr f000000 && " \
		"fdt set /amba/misc_clk clock-frequency <48000> && "\
		"fdt set /timer clock-frequency <240000> && " \
		"fdt set /amba/i2c_clk clock-frequency <240000> && " \
		"booti 80000 - f000000\0" \
	"netboot=tftpboot 80000 Image && tftpboot $fdt_addr system.dtb && " \
		 "booti 80000 - $fdt_addr\0" \
	"qspiboot=sf probe 0 0 0 && sf read $fdt_addr $fdt_offset $fdt_size && " \
		  "sf read $kernel_addr $kernel_offset $kernel_size && " \
		  "booti $kernel_addr - $fdt_addr\0" \
	"sdboot=mmc dev $sdbootdev && mmcinfo && load mmc $sdbootdev:$partid $fdt_addr system.dtb && " \
		"load mmc $sdbootdev:$partid $kernel_addr Image && " \
		"booti $kernel_addr - $fdt_addr\0" \
	"nandboot=nand info && nand read $fdt_addr $fdt_offset $fdt_size && " \
		  "nand read $kernel_addr $kernel_offset $kernel_size && " \
		  "booti $kernel_addr - $fdt_addr\0" \
	"xen=tftpb $fdt_addr system.dtb && fdt addr $fdt_addr && fdt resize && " \
		"tftpb 0x80000 Image && " \
		"fdt set /chosen/dom0 reg <0x80000 0x$filesize> && "\
		"tftpb 6000000 xen.ub && bootm 6000000 - $fdt_addr\0" \
	"jtagboot=tftpboot 10000000 image.ub && bootm\0" \
	"nosmp=setenv bootargs $bootargs maxcpus=1\0" \
	"nfsroot=setenv bootargs $bootargs root=/dev/nfs nfsroot=$serverip:/mnt/sata,tcp ip=$ipaddr:$serverip:$serverip:255.255.255.0:zynqmp:eth0:off rw\0" \
	"sdroot=setenv bootargs $bootargs root=/dev/mmcblk0p2 rw rootwait\0" \
	"sdroot1=setenv bootargs $bootargs root=/dev/mmcblk1p2 rw rootwait\0" \
	"usbhostboot=usb start && load usb 0 $fdt_addr system.dtb && " \
		     "load usb 0 $kernel_addr Image && " \
		     "booti $kernel_addr - $fdt_addr\0" \
	DFU_ALT_INFO

#define CONFIG_PREBOOT		"run bootargs; run sata_root; run setup"
#define CONFIG_BOOTCOMMAND	"run $modeboot"
#define CONFIG_BOOTDELAY	5

#define CONFIG_BOARD_LATE_INIT

/* Do not preserve environment */
#define CONFIG_ENV_IS_NOWHERE		1
#define CONFIG_ENV_SIZE			0x1000

/* Monitor Command Prompt */
/* Console I/O Buffer Size */
#define CONFIG_SYS_CBSIZE		2048
#define CONFIG_SYS_PROMPT		"ZynqMP> "
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_LONGHELP
#define CONFIG_CMDLINE_EDITING
/* max command args */
#define CONFIG_SYS_MAXARGS		64

/* Ethernet driver */
#if defined(CONFIG_ZYNQ_GEM0) || defined(CONFIG_ZYNQ_GEM1) || \
	defined(CONFIG_ZYNQ_GEM2) || defined(CONFIG_ZYNQ_GEM3)
# define CONFIG_NET_MULTI
# define CONFIG_ZYNQ_GEM
# define CONFIG_MII
# define CONFIG_SYS_FAULT_ECHO_LINK_DOWN
# define CONFIG_PHYLIB
# define CONFIG_PHY_MARVELL
# define CONFIG_PHY_NATSEMI
# define CONFIG_PHY_TI
# define CONFIG_PHY_GIGE
#endif

/* I2C */
#if defined(CONFIG_SYS_I2C_ZYNQ)
# define CONFIG_CMD_I2C
# define CONFIG_SYS_I2C
# define CONFIG_SYS_I2C_ZYNQ_SPEED		100000
# define CONFIG_SYS_I2C_ZYNQ_SLAVE		0
#endif

/* EEPROM */
#ifdef CONFIG_ZYNQMP_EEPROM
# define CONFIG_CMD_EEPROM
# define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		2
# define CONFIG_SYS_I2C_EEPROM_ADDR		0x54
# define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	4
# define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	5
# define CONFIG_SYS_EEPROM_SIZE			(64 * 1024)
#endif

#ifdef CONFIG_AHCI
#define CONFIG_LIBATA
#define CONFIG_SCSI_AHCI
#define CONFIG_SCSI_AHCI_PLAT
#define CONFIG_SYS_SCSI_MAX_SCSI_ID	1
#define CONFIG_SYS_SCSI_MAX_LUN		1
#define CONFIG_SYS_SCSI_MAX_DEVICE	(CONFIG_SYS_SCSI_MAX_SCSI_ID * \
					 CONFIG_SYS_SCSI_MAX_LUN)
#define CONFIG_CMD_SCSI
#endif

#define CONFIG_SYS_BOOTM_LEN	(60 * 1024 * 1024)

#define CONFIG_CMD_BOOTI
#define CONFIG_CMD_UNZIP

#define CONFIG_BOARD_EARLY_INIT_R
#define CONFIG_CLOCKS

#define CONFIG_CMD_CACHE

#endif /* __XILINX_ZYNQMP_H */

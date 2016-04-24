/*
 * (C) Copyright 2003-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2005-2007
 * Modified for InterControl digsyMTC MPC5200 board by
 * Frank Bodammer, GCD Hard- & Software GmbH,
 *                 frank.bodammer@gcd-solutions.de
 *
 * (C) Copyright 2009 Semihalf
 * Optimized for digsyMTC by: Grzegorz Bernacki <gjb@semihalf.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */

#define CONFIG_MPC5200		1	/* This is an MPC5200 CPU */
#define CONFIG_DIGSY_MTC	1	/* ... on InterControl digsyMTC board */
#define CONFIG_DISPLAY_BOARDINFO

/*
 * Valid values for CONFIG_SYS_TEXT_BASE are:
 * 0xFFF00000	boot high (standard configuration)
 * 0xFE000000	boot low
 * 0x00100000	boot from RAM (for testing only)
 */
#ifndef CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_TEXT_BASE	0xFFF00000	/* Standard: boot high */
#endif

#define CONFIG_SYS_MPC5XXX_CLKIN	33000000

#define CONFIG_SYS_CACHELINE_SIZE	32

/*
 * Serial console configuration
 */
#define CONFIG_PSC_CONSOLE	4	/* console is on PSC4  */
#define CONFIG_BAUDRATE		115200	/* ... at 115200  bps  */
#define CONFIG_SYS_BAUDRATE_TABLE	\
	{ 9600, 19200, 38400, 57600, 115200, 230400 }

/*
 * PCI Mapping:
 * 0x40000000 - 0x4fffffff - PCI Memory
 * 0x50000000 - 0x50ffffff - PCI IO Space
 */
#define CONFIG_PCI		1
#define CONFIG_PCI_PNP		1
#define CONFIG_PCI_SCAN_SHOW	1
#define CONFIG_PCI_BOOTDELAY	250

#define CONFIG_PCI_MEM_BUS	0x40000000
#define CONFIG_PCI_MEM_PHYS	CONFIG_PCI_MEM_BUS
#define CONFIG_PCI_MEM_SIZE	0x10000000

#define CONFIG_PCI_IO_BUS	0x50000000
#define CONFIG_PCI_IO_PHYS	CONFIG_PCI_IO_BUS
#define CONFIG_PCI_IO_SIZE	0x01000000

/*
 *  Partitions
 */
#define CONFIG_DOS_PARTITION
#define CONFIG_BZIP2

/*
 * Video
 */
#define CONFIG_VIDEO

#ifdef CONFIG_VIDEO
#define CONFIG_VIDEO_MB862xx
#define CONFIG_VIDEO_MB862xx_ACCEL
#define CONFIG_VIDEO_CORALP
#define CONFIG_CFB_CONSOLE
#define CONFIG_VIDEO_LOGO
#define CONFIG_VIDEO_BMP_LOGO
#define CONFIG_VIDEO_SW_CURSOR
#define CONFIG_VGA_AS_SINGLE_DEVICE
#define CONFIG_SYS_CONSOLE_IS_IN_ENV
#define CONFIG_SPLASH_SCREEN
#define CONFIG_VIDEO_BMP_GZIP
#define CONFIG_SYS_VIDEO_LOGO_MAX_SIZE	(2 << 20)	/* decompressed img */

/* Coral-PA clock frequency, geo and other both 133MHz */
#define CONFIG_SYS_MB862xx_CCF	0x00050000
/* Video SDRAM parameters */
#define CONFIG_SYS_MB862xx_MMR	0x11d7fa72
#endif

/*
 * Command line configuration.
 */
#ifdef CONFIG_VIDEO
#define CONFIG_CMD_BMP
#endif
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_IDE
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_PCI
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_SAVES

#if (CONFIG_SYS_TEXT_BASE == 0xFF000000)
#define CONFIG_SYS_LOWBOOT	1
#endif

/*
 * Autobooting
 */
#define CONFIG_BOOTDELAY	1

#undef	CONFIG_BOOTARGS

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"fw_image=digsyMPC.img\0"					\
	"mtcb_start=mtc led diag orange; run mtcb_1\0"			\
	"mtcb_clearled=for x in user1 user2 usbpwr usbbusy; "		\
		"do mtc led $x; done\0"					\
	"mtcb_1=if mtc key; then run mtcb_clearled mtcb_update; "	\
		"else run mtcb_fw; fi\0"				\
	"mtcb_fw=if bootm ff000000; then echo FIRMWARE OK!; "		\
		"else echo BAD FIRMWARE CRC!; mtc led diag red; fi\0"	\
	"mtcb_update=mtc led user1 orange;"				\
		"while mtc key; do ; done; run mtcb_2;\0"		\
	"mtcb_2=mtc led user1 green 2; usb reset; run mtcb_usb1;\0"	\
	"mtcb_usb1=if fatload usb 0 400000 script.img; "		\
		"then run mtcb_doscript; else run mtcb_usb2; fi\0"	\
	"mtcb_usb2=if fatload usb 0 400000 $fw_image; "			\
		"then run mtcb_dousb; else run mtcb_ide; fi\0"		\
	"mtcb_doscript=run mtcb_usbleds; mtc led user2 orange 2; "	\
		"run mtcb_wait_flickr mtcb_ds_1;\0"			\
	"mtcb_ds_1=if imi 400000; then mtc led usbbusy; "		\
		"source 400000; else run mtcb_error; fi\0"		\
	"mtcb_dousb=run mtcb_usbleds mtcb_wait_flickr mtcb_du_1;\0"	\
	"mtcb_du_1=if imi 400000; then run mtcb_du_2; "			\
		"else run mtcb_error; fi\0"				\
	"mtcb_du_2=run mtcb_clear mtcb_prog; mtc led usbbusy; "		\
		"run mtcb_checkfw\0"					\
	"mtcb_checkfw=if imi ff000000; then run mtcb_success; "		\
		"else run mtcb_error; fi\0"				\
	"mtcb_waitkey=mtc key; until test $? -eq 0; do mtc key; done\0"	\
	"mtcb_wait_flickr=run mtcb_waitkey mtcb_uledflckr\0"		\
	"mtcb_usbleds=mtc led usbpwr green; mtc led usbbusy orange 1;\0"\
	"mtcb_uledflckr=mtc led user1 orange 11\0"			\
	"mtcb_error=mtc led user1 red\0"				\
	"mtcb_clear=erase ff000000 ff0fffff\0"				\
	"mtcb_prog=cp.b 400000 ff000000 ${filesize}\0"			\
	"mtcb_success=mtc led user1 green\0"				\
	"mtcb_ide=if fatload ide 0 400000 $fw_image;"			\
		"then run mtcb_doide; else run mtcb_error; fi\0"	\
	"mtcb_doide=mtc led user2 green 1;"				\
		"run mtcb_wait_flickr mtcb_di_1;\0"			\
	"mtcb_di_1=if imi 400000; then run mtcb_di_2;"			\
		"else run mtcb_error; fi\0"				\
	"mtcb_di_2=run mtcb_clear; run mtcb_prog mtcb_checkfw\0"	\
	"ramdisk_num_sector=16\0"					\
	"flash_base=ff000000\0"						\
	"flashdisk_size=e00000\0"					\
	"env_sector=fff60000\0"						\
	"flashdisk_start=ff100000\0"					\
	"load_cmd=tftp 400000 digsyMPC.img\0"				\
	"clear_cmd=erase ff000000 ff0fffff\0"				\
	"flash_cmd=cp.b 400000 ff000000 ${filesize}\0"			\
	"update_cmd=run load_cmd; "					\
	"iminfo 400000; "						\
	"run clear_cmd flash_cmd; "					\
	"iminfo ff000000\0"						\
	"spi_driver=yes\0"						\
	"spi_watchdog=no\0"						\
	"ftps_start=yes\0"						\
	"ftps_user1=admin\0"						\
	"ftps_pass1=admin\0"						\
	"ftps_base1=/\0"						\
	"ftps_home1=/\0"						\
	"plc_sio_srv=no\0"						\
	"plc_sio_baud=57600\0"						\
	"plc_sio_parity=no\0"						\
	"plc_sio_stop=1\0"						\
	"plc_sio_com=2\0"						\
	"plc_eth_srv=yes\0"						\
	"plc_eth_port=1200\0"						\
	"plc_root=/ide/\0"						\
	"diag_level=0\0"						\
	"webvisu=no\0"							\
	"plc_can1_routing=no\0"						\
	"plc_can1_baudrate=250\0"					\
	"plc_can2_routing=no\0"						\
	"plc_can2_baudrate=250\0"					\
	"plc_can3_routing=no\0"						\
	"plc_can3_baudrate=250\0"					\
	"plc_can4_routing=no\0"						\
	"plc_can4_baudrate=250\0"					\
	"netdev=eth0\0"							\
	"console=ttyPSC0\0"						\
	"kernel_addr_r=400000\0"					\
	"fdt_addr_r=600000\0"						\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
	"nfsroot=${serverip}:${rootpath}\0"				\
	"addip=setenv bootargs ${bootargs} "				\
	"ip=${ipaddr}:${serverip}:${gatewayip}:"			\
	"${netmask}:${hostname}:${netdev}:off panic=1\0"		\
	"addcons=setenv bootargs ${bootargs} console=${console},${baudrate}\0"\
	"rootpath=/opt/eldk/ppc_6xx\0"					\
	"net_nfs=tftp ${kernel_addr_r} ${bootfile};"			\
		"tftp ${fdt_addr_r} ${fdt_file};"			\
		"run nfsargs addip addcons;"				\
		"bootm ${kernel_addr_r} - ${fdt_addr_r}\0"		\
	"load=tftp 200000 ${u-boot}\0"					\
	"update=protect off FFF00000 +${filesize};"			\
		"erase FFF00000 +${filesize};"				\
		"cp.b 200000 FFF00000 ${filesize};"			\
		"protect on FFF00000 +${filesize}\0"			\
	""

#define CONFIG_BOOTCOMMAND	"run mtcb_start"

/*
 * SPI configuration
 */
#define CONFIG_HARD_SPI		1
#define CONFIG_MPC52XX_SPI	1

/*
 * I2C configuration
 */
#define CONFIG_HARD_I2C		1
#define CONFIG_SYS_I2C_MODULE	1
#define CONFIG_SYS_I2C_SPEED	100000
#define CONFIG_SYS_I2C_SLAVE	0x7F

/*
 * EEPROM configuration
 */
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x50	/* 1010000x */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 	1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	70

/*
 * RTC configuration
 */
#if defined(CONFIG_DIGSY_REV5)
#define CONFIG_SYS_I2C_RTC_ADDR	0x56
#define CONFIG_RTC_RV3029
/* Enable 5k Ohm trickle charge resistor */
#define CONFIG_SYS_RV3029_TCR	0x20
#else
#define CONFIG_RTC_DS1337
#define CONFIG_SYS_I2C_RTC_ADDR	0x68
#define CONFIG_SYS_DS1339_TCR_VAL	0xAB	/* diode + 4k resistor */
#endif

/*
 * Flash configuration
 */
#define	CONFIG_SYS_FLASH_CFI		1
#define	CONFIG_FLASH_CFI_DRIVER	1

#if defined(CONFIG_DIGSY_REV5)
#define CONFIG_SYS_FLASH_BASE		0xFE000000
#define CONFIG_SYS_FLASH_BASE_CS1	0xFC000000
#define CONFIG_SYS_MAX_FLASH_BANKS	2
#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE, \
					CONFIG_SYS_FLASH_BASE_CS1}
#define CONFIG_SYS_UPDATE_FLASH_SIZE
#define CONFIG_FDT_FIXUP_NOR_FLASH_SIZE
#else
#define CONFIG_SYS_FLASH_BASE		0xFF000000
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE }
#endif

#define CONFIG_SYS_MAX_FLASH_SECT	256
#define CONFIG_FLASH_16BIT
#define CONFIG_SYS_FLASH_CFI_WIDTH FLASH_CFI_16BIT
#define CONFIG_SYS_FLASH_SIZE	0x01000000
#define CONFIG_SYS_FLASH_ERASE_TOUT	240000
#define CONFIG_SYS_FLASH_WRITE_TOUT	500

#define OF_CPU			"PowerPC,5200@0"
#define OF_SOC			"soc5200@f0000000"
#define OF_TBCLK		(bd->bi_busfreq / 4)

#define CONFIG_BOARD_EARLY_INIT_R
#define CONFIG_MISC_INIT_R

/*
 * Environment settings
 */
#define CONFIG_ENV_IS_IN_FLASH	1
#if defined(CONFIG_LOWBOOT)
#define CONFIG_ENV_ADDR		0xFF060000
#else	/* CONFIG_LOWBOOT */
#define CONFIG_ENV_ADDR		0xFFF60000
#endif	/* CONFIG_LOWBOOT */
#define CONFIG_ENV_SIZE		0x10000
#define CONFIG_ENV_SECT_SIZE	0x20000
#define CONFIG_ENV_OVERWRITE	1

/*
 * Memory map
 */
#define CONFIG_SYS_MBAR		0xF0000000
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#if !defined(CONFIG_SYS_LOWBOOT)
#define CONFIG_SYS_DEFAULT_MBAR	0x80000000
#else
#define CONFIG_SYS_DEFAULT_MBAR	0xF0000000
#endif

/*
 *  Use SRAM until RAM will be available
 */
#define CONFIG_SYS_INIT_RAM_ADDR	MPC5XXX_SRAM
#define CONFIG_SYS_INIT_RAM_SIZE		MPC5XXX_SRAM_SIZE

#define CONFIG_SYS_GBL_DATA_OFFSET	\
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE
#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#define CONFIG_SYS_RAMBOOT		1
#endif

#define CONFIG_SYS_MONITOR_LEN	(256 << 10)
#define CONFIG_SYS_MALLOC_LEN	(4096 << 10)
#define CONFIG_SYS_BOOTMAPSZ	(8 << 20)

/*
 * Ethernet configuration
 */
#define CONFIG_MPC5xxx_FEC	1
#define CONFIG_MPC5xxx_FEC_MII100
#if defined(CONFIG_DIGSY_REV5)
#define CONFIG_PHY_ADDR		0x01
#else
#define CONFIG_PHY_ADDR		0x00
#endif
#define CONFIG_PHY_RESET_DELAY	1000

#define CONFIG_NETCONSOLE		/* include NetConsole support	*/

/*
 * GPIO configuration
 * use pin gpio_wkup_6 as second SDRAM chip select (mem_cs1)
 *  Bit 0   (mask 0x80000000) : 0x1
 * SPI on Tmr2/3/4/5 pins
 *  Bit 2:3 (mask 0x30000000) : 0x2
 * ATA cs0/1 on csb_4/5
 *  Bit 6:7 (mask 0x03000000) : 0x2
 * Ethernet 100Mbit with MD
 *  Bits 12:15 (mask 0x000f0000): 0x5
 * USB - Two UARTs
 *  Bits 18:19 (mask 0x00003000) : 0x2
 * PSC3 - USB2 on PSC3
 *  Bits 20:23 (mask 0x00000f00) : 0x1
 * PSC2 - CAN1&2 on PSC2 pins
 *  Bits 25:27 (mask 0x00000070) : 0x1
 * PSC1 - AC97 functionality
 *  Bits 29:31 (mask 0x00000007) : 0x2
 */
#define CONFIG_SYS_GPS_PORT_CONFIG	0xA2552112

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP
#define CONFIG_AUTO_COMPLETE	1
#define CONFIG_CMDLINE_EDITING	1

#define CONFIG_MX_CYCLIC	1
#define CONFIG_ZERO_BOOTDELAY_CHECK

#define CONFIG_SYS_CBSIZE		1024
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS		32
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

#define CONFIG_SYS_ALT_MEMTEST
#define CONFIG_SYS_MEMTEST_SCRATCH	0x00001000
#define CONFIG_SYS_MEMTEST_START	0x00010000
#define CONFIG_SYS_MEMTEST_END		0x019fffff

#define CONFIG_SYS_LOAD_ADDR		0x00100000

/*
 * Various low-level settings
 */
#define CONFIG_SYS_SDRAM_CS1		1
#define CONFIG_SYS_XLB_PIPELINING	1

#define CONFIG_SYS_HID0_INIT		HID0_ICE | HID0_ICFI
#define CONFIG_SYS_HID0_FINAL		HID0_ICE

#if defined(CONFIG_SYS_LOWBOOT)
#define CONFIG_SYS_BOOTCS_START	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_BOOTCS_SIZE		CONFIG_SYS_FLASH_SIZE
#define CONFIG_SYS_BOOTCS_CFG		0x0002DD00
#endif

#define CONFIG_SYS_CS4_START		0x60000000
#define CONFIG_SYS_CS4_SIZE		0x1000
#define CONFIG_SYS_CS4_CFG		0x0008FC00

#define CONFIG_SYS_CS0_START		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_CS0_SIZE		CONFIG_SYS_FLASH_SIZE
#define CONFIG_SYS_CS0_CFG		0x0002DD00

#if defined(CONFIG_DIGSY_REV5)
#define CONFIG_SYS_CS1_START		CONFIG_SYS_FLASH_BASE_CS1
#define CONFIG_SYS_CS1_SIZE		CONFIG_SYS_FLASH_SIZE
#define CONFIG_SYS_CS1_CFG		0x0002DD00
#endif

#define CONFIG_SYS_CS_BURST		0x00000000
#define CONFIG_SYS_CS_DEADCYCLE	0x11111111

#if !defined(CONFIG_SYS_LOWBOOT)
#define CONFIG_SYS_RESET_ADDRESS	0xfff00100
#else
#define CONFIG_SYS_RESET_ADDRESS	0xff000100
#endif

/*
 * USB
 */
#define CONFIG_USB_OHCI_NEW
#define CONFIG_SYS_OHCI_BE_CONTROLLER
#define CONFIG_USB_STORAGE

#define CONFIG_USB_CLOCK	0x00013333
#define CONFIG_USB_CONFIG	0x00002000

#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	15
#define CONFIG_SYS_USB_OHCI_REGS_BASE	MPC5XXX_USB
#define CONFIG_SYS_USB_OHCI_SLOT_NAME	"mpc5200"
#define CONFIG_SYS_USB_OHCI_CPU_INIT

/*
 * IDE/ATA
 */
#define CONFIG_IDE_RESET
#define CONFIG_IDE_PREINIT

#define CONFIG_SYS_ATA_CS_ON_I2C2
#define CONFIG_SYS_IDE_MAXBUS		1
#define CONFIG_SYS_IDE_MAXDEVICE	1

#define CONFIG_SYS_ATA_IDE0_OFFSET	0x0000
#define CONFIG_SYS_ATA_BASE_ADDR	MPC5XXX_ATA
#define CONFIG_SYS_ATA_DATA_OFFSET	(0x0060)
#define CONFIG_SYS_ATA_REG_OFFSET	(CONFIG_SYS_ATA_DATA_OFFSET)
#define CONFIG_SYS_ATA_ALT_OFFSET	(0x005C)
#define CONFIG_SYS_ATA_STRIDE		4

#define CONFIG_ATAPI		1
#define CONFIG_LBA48		1

#endif /* __CONFIG_H */

/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2008
 * Sergei Poselenov, Emcraft Systems, sposelenov@emcraft.com.
 *
 * Wolfgang Denk <wd@denx.de>
 * Copyright 2004 Freescale Semiconductor.
 * (C) Copyright 2002,2003 Motorola,Inc.
 * Xianghua Xiao <X.Xiao@motorola.com>
 */

/*
 * Socrates
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* High Level Configuration Options */
#define CONFIG_SOCRATES		1

/*
 * Only possible on E500 Version 2 or newer cores.
 */
#define CONFIG_ENABLE_36BIT_PHYS	1

/*
 * sysclk for MPC85xx
 *
 * Two valid values are:
 *    33000000
 *    66000000
 *
 * Most PCI cards are still 33Mhz, so in the presence of PCI, 33MHz
 * is likely the desired value here, so that is now the default.
 * The board, however, can run at 66MHz.  In any event, this value
 * must match the settings of some switches.  Details can be found
 * in the README.mpc85xxads.
 */

/*
 * These can be toggled for performance analysis, otherwise use default.
 */
#define CONFIG_L2_CACHE			/* toggle L2 cache		*/

#define CONFIG_SYS_INIT_DBCR DBCR_IDM		/* Enable Debug Exceptions	*/

#undef	CONFIG_SYS_DRAM_TEST			/* memory test, takes time	*/

#define CONFIG_SYS_CCSRBAR		0xE0000000
#define CONFIG_SYS_CCSRBAR_PHYS_LOW	CONFIG_SYS_CCSRBAR

/* DDR Setup */
#define CONFIG_SPD_EEPROM		/* Use SPD EEPROM for DDR setup */

#define CONFIG_MEM_INIT_VALUE	0xDeadBeef

#define CONFIG_SYS_DDR_SDRAM_BASE	0x00000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE
#define CONFIG_VERY_BIG_RAM

/* I2C addresses of SPD EEPROMs */
#define SPD_EEPROM_ADDRESS	0x50	/* CTLR 0 DIMM 0 */


/* Hardcoded values, to use instead of SPD */
#define CONFIG_SYS_DDR_CS0_BNDS		0x0000000f
#define CONFIG_SYS_DDR_CS0_CONFIG		0x80010102
#define CONFIG_SYS_DDR_TIMING_0		0x00260802
#define CONFIG_SYS_DDR_TIMING_1		0x3935D322
#define CONFIG_SYS_DDR_TIMING_2		0x14904CC8
#define CONFIG_SYS_DDR_MODE			0x00480432
#define CONFIG_SYS_DDR_INTERVAL		0x030C0100
#define CONFIG_SYS_DDR_CONFIG_2		0x04400000
#define CONFIG_SYS_DDR_CONFIG			0xC3008000
#define CONFIG_SYS_DDR_CLK_CONTROL		0x03800000
#define CONFIG_SYS_SDRAM_SIZE			256 /* in Megs */

/*
 * Flash on the LocalBus
 */
#define CONFIG_SYS_LBC_CACHE_BASE	0xf0000000	/* Localbus cacheable	 */

#define CONFIG_SYS_FLASH_QUIET_TEST
#define CONFIG_SYS_FLASH0		0xFE000000
#define CONFIG_SYS_FLASH1		0xFC000000
#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH1, CONFIG_SYS_FLASH0 }

#define CONFIG_SYS_LBC_FLASH_BASE	CONFIG_SYS_FLASH1	/* Localbus flash start	*/
#define CONFIG_SYS_FLASH_BASE		CONFIG_SYS_LBC_FLASH_BASE /* start of FLASH	*/

#define CONFIG_SYS_MAX_FLASH_SECT	256		/* sectors per device	*/
#undef	CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms)	*/

#define CONFIG_SYS_LBC_LCRR		0x00030004    /* LB clock ratio reg	*/
#define CONFIG_SYS_LBC_LBCR		0x00000000    /* LB config reg		*/
#define CONFIG_SYS_LBC_LSRT		0x20000000    /* LB sdram refresh timer	*/
#define CONFIG_SYS_LBC_MRTPR		0x20000000    /* LB refresh timer presc.*/

#define CONFIG_SYS_INIT_RAM_LOCK	1
#define CONFIG_SYS_INIT_RAM_ADDR	0xe4010000	/* Initial RAM address	*/
#define CONFIG_SYS_INIT_RAM_SIZE	0x4000		/* Size used area in RAM*/

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_LEN		(384 * 1024)	/* Reserve 384KiB for Mon */

/* FPGA and NAND */
#define CONFIG_SYS_FPGA_BASE		0xc0000000
#define CONFIG_SYS_FPGA_SIZE		0x00100000	/* 1 MB		*/
#define CONFIG_SYS_HMI_BASE		0xc0010000

#define CONFIG_SYS_NAND_BASE		(CONFIG_SYS_FPGA_BASE + 0x70)
#define CONFIG_SYS_MAX_NAND_DEVICE	1

/* LIME GDC */
#define CONFIG_SYS_LIME_BASE		0xc8000000
#define CONFIG_SYS_LIME_SIZE		0x04000000	/* 64 MB	*/

#define CONFIG_SYS_SPD_BUS_NUM 0

/*
 * General PCI
 * Memory space is mapped 1-1.
 */

/* PCI is clocked by the external source at 33 MHz */
#define CONFIG_PCI_CLK_FREQ	33000000
#define CONFIG_SYS_PCI1_MEM_BASE	0x80000000
#define CONFIG_SYS_PCI1_MEM_PHYS	CONFIG_SYS_PCI1_MEM_BASE
#define CONFIG_SYS_PCI1_MEM_SIZE	0x20000000	/* 512M			*/
#define CONFIG_SYS_PCI1_IO_BASE	0xE2000000
#define CONFIG_SYS_PCI1_IO_PHYS	CONFIG_SYS_PCI1_IO_BASE
#define CONFIG_SYS_PCI1_IO_SIZE	0x01000000	/* 16M			*/

#define CONFIG_TSEC1	1
#define CONFIG_TSEC1_NAME	"TSEC0"
#define CONFIG_TSEC3	1
#define CONFIG_TSEC3_NAME	"TSEC1"
#undef CONFIG_MPC85XX_FEC

#define TSEC1_PHY_ADDR		0
#define TSEC3_PHY_ADDR		1

#define TSEC1_PHYIDX		0
#define TSEC3_PHYIDX		0
#define TSEC1_FLAGS		TSEC_GIGABIT
#define TSEC3_FLAGS		TSEC_GIGABIT

/* Options are: TSEC[0,1] */

/*
 * Environment
 */

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

/*
 * Miscellaneous configurable options
 */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(8 << 20)	/* Initial Memory map for Linux	*/


#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"consdev=ttyS0\0"						\
	"uboot_file=/home/tftp/syscon3/u-boot.bin\0"			\
	"bootfile=/home/tftp/syscon3/uImage\0"				\
	"fdt_file=/home/tftp/syscon3/socrates.dtb\0"			\
	"initrd_file=/home/tftp/syscon3/uinitrd.gz\0"			\
	"uboot_addr=FFF60000\0"						\
	"kernel_addr=FE000000\0"					\
	"fdt_addr=FE1E0000\0"						\
	"ramdisk_addr=FE200000\0"					\
	"fdt_addr_r=B00000\0"						\
	"kernel_addr_r=200000\0"					\
	"ramdisk_addr_r=400000\0"					\
	"rootpath=/opt/eldk/ppc_85xxDP\0"				\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=$serverip:$rootpath\0"				\
	"addcons=setenv bootargs $bootargs "				\
		"console=$consdev,$baudrate\0"				\
	"addip=setenv bootargs $bootargs "				\
		"ip=$ipaddr:$serverip:$gatewayip:$netmask"		\
		":$hostname:$netdev:off panic=1\0"			\
	"boot_nor=run ramargs addcons;"					\
		"bootm ${kernel_addr} ${ramdisk_addr} ${fdt_addr}\0"	\
	"net_nfs=tftp ${kernel_addr_r} ${bootfile}; "			\
		"tftp ${fdt_addr_r} ${fdt_file}; "			\
		"run nfsargs addip addcons;"				\
		"bootm ${kernel_addr_r} - ${fdt_addr_r}\0"		\
	"update_uboot=tftp 100000 ${uboot_file};"			\
		"protect off fff60000 ffffffff;"			\
		"era fff60000 ffffffff;"				\
		"cp.b 100000 fff60000 ${filesize};"			\
		"setenv filesize;saveenv\0"				\
	"update_kernel=tftp 100000 ${bootfile};"			\
		"era fe000000 fe1dffff;"				\
		"cp.b 100000 fe000000 ${filesize};"			\
		"setenv filesize;saveenv\0"				\
	"update_fdt=tftp 100000 ${fdt_file};"				\
		"era fe1e0000 fe1fffff;"				\
		"cp.b 100000 fe1e0000 ${filesize};"			\
		"setenv filesize;saveenv\0"				\
	"update_initrd=tftp 100000 ${initrd_file};"			\
		"era fe200000 fe9fffff;"				\
		"cp.b 100000 fe200000 ${filesize};"			\
		"setenv filesize;saveenv\0"				\
	"clean_data=era fea00000 fff5ffff\0"				\
	"usbargs=setenv bootargs root=/dev/sda1 rw\0"			\
	"load_usb=usb start;"						\
		"ext2load usb 0:1 ${kernel_addr_r} /boot/uImage\0"	\
	"boot_usb=run load_usb usbargs addcons;"			\
		"bootm ${kernel_addr_r} - ${fdt_addr};"			\
		"bootm ${kernel_addr} ${ramdisk_addr} ${fdt_addr}\0"	\
	""

/* pass open firmware flat tree */

/* USB support */
#define CONFIG_USB_OHCI_NEW		1
#define CONFIG_PCI_OHCI			1
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	15
#define CONFIG_SYS_USB_OHCI_SLOT_NAME		"ohci_pci"
#define CONFIG_SYS_OHCI_SWAP_REG_ACCESS	1

#endif	/* __CONFIG_H */

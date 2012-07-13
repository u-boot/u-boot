/*
 * (C) Copyright 2006-2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * Configuation settings for the PDNB3 board.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_IXP425		1       /* This is an IXP425 CPU	*/
#define CONFIG_PDNB3		1       /* on an PDNB3 board		*/

#define	CONFIG_MACH_TYPE	1002

#define CONFIG_DISPLAY_CPUINFO	1	/* display cpu info (and speed)	*/
#define CONFIG_DISPLAY_BOARDINFO 1	/* display board info		*/

/*
 * Ethernet
 */
#define CONFIG_IXP4XX_NPE	1	/* include IXP4xx NPE support	*/
#define	CONFIG_PHY_ADDR		16	/* NPE0 PHY address		*/
#define CONFIG_HAS_ETH1
#define CONFIG_PHY1_ADDR	18	/* NPE1 PHY address		*/
#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_SYS_RX_ETH_BUFFER	16	/* Number of ethernet rx buffers & descriptors */

/*
 * Misc configuration options
 */
#define CONFIG_BOOTCOUNT_LIMIT		/* support for bootcount limit	*/
#define CONFIG_SYS_BOOTCOUNT_ADDR	0x60003000 /* inside qmrg sram		*/

#define CONFIG_CMDLINE_TAG	1	/* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS 1
#define CONFIG_INITRD_TAG	1

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(1 << 20)

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_IXP_SERIAL
#define CONFIG_BAUDRATE         115200
#define CONFIG_SYS_IXP425_CONSOLE	IXP425_UART1   /* we use UART1 for console */


/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DATE
#define CONFIG_CMD_NET
#define CONFIG_CMD_MII
#define CONFIG_CMD_I2C
#define CONFIG_CMD_ELF
#define CONFIG_CMD_PING

#if !defined(CONFIG_SCPU)
#define CONFIG_CMD_NAND
#endif


#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP                            /* undef to save memory         */
#define CONFIG_SYS_PROMPT              "=> "   /* Monitor Command Prompt       */
#define CONFIG_SYS_CBSIZE              256             /* Console I/O Buffer Size      */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS             16              /* max number of command args   */
#define CONFIG_SYS_BARGSIZE            CONFIG_SYS_CBSIZE      /* Boot Argument Buffer Size    */

#define CONFIG_SYS_MEMTEST_START       0x00400000      /* memtest works on     */
#define CONFIG_SYS_MEMTEST_END         0x00800000      /* 4 ... 8 MB in DRAM   */
#define CONFIG_SYS_LOAD_ADDR           0x00010000      /* default load address */

#define CONFIG_IXP425_TIMER_CLK		66666666
#define CONFIG_SYS_HZ			1000		/* decrementer freq: 1 ms ticks */

/***************************************************************
 * Platform/Board specific defines start here.
 ***************************************************************/

/*-----------------------------------------------------------------------
 * Default configuration (environment varibles...)
 *----------------------------------------------------------------------*/
#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"hostname=pdnb3\0"						\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} ethaddr=${ethaddr} "		\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addtty=setenv bootargs ${bootargs} console=ttyS0,${baudrate} "	\
	"mtdparts=${mtdparts}\0"					\
	"flash_nfs=run nfsargs addip addtty;"				\
		"bootm ${kernel_addr}\0"				\
	"flash_self=run ramargs addip addtty;"				\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"net_nfs=tftp 200000 ${bootfile};run nfsargs addip addtty;"     \
	        "bootm\0"						\
	"rootpath=/opt/buildroot\0"					\
	"bootfile=/tftpboot/netbox/uImage\0"				\
	"kernel_addr=50080000\0"					\
	"ramdisk_addr=50200000\0"					\
	"load=tftp 100000 /tftpboot/netbox/u-boot.bin\0"		\
	"update=protect off 50000000 5007dfff;era 50000000 5007dfff;"	\
		"cp.b 100000 50000000 ${filesize};"			\
		"setenv filesize;saveenv\0"				\
	"upd=run load update\0"						\
	"ipaddr=10.0.0.233\0"						\
	"serverip=10.0.0.152\0"						\
	"netmask=255.255.0.0\0"						\
	"ethaddr=c6:6f:13:36:f3:81\0"					\
	"eth1addr=c6:6f:13:36:f3:82\0"					\
	"mtdparts=IXP4XX-Flash.0:504k@0(uboot),4k@504k(env),"		\
	"4k@508k(renv)\0"						\
	""
#define CONFIG_BOOTCOMMAND	"run net_nfs"

/*
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS    1          /* we have 1 bank of DRAM */
#define PHYS_SDRAM_1            0x00000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE       0x02000000 /* 32 MB */

#define CONFIG_SYS_TEXT_BASE	       0x50000000
#define CONFIG_SYS_FLASH_BASE          0x50000000
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE
#if defined(CONFIG_SCPU)
#define CONFIG_SYS_MONITOR_LEN		(384 << 10)	/* Reserve 512 kB for Monitor	*/
#else
#define CONFIG_SYS_MONITOR_LEN		(504 << 10)	/* Reserve 512 kB for Monitor	*/
#endif

/*
 * Expansion bus settings
 */
#if defined(CONFIG_SCPU)
#define CONFIG_SYS_EXP_CS0		0x94d23C42	/* 8bit, max size		*/
#else
#define CONFIG_SYS_EXP_CS0		0x94913C43	/* 8bit, max size		*/
#endif
#define CONFIG_SYS_EXP_CS1		0x85000043	/* 8bit, 512bytes		*/

/*
 * SDRAM settings
 */
#define CONFIG_SYS_SDR_CONFIG		0x18
#define CONFIG_SYS_SDR_MODE_CONFIG	0x1
#define CONFIG_SYS_SDRAM_REFRESH_CNT	0x81a

/*
 * FLASH and environment organization
 */
#if defined(CONFIG_SCPU)
#define CONFIG_SYS_FLASH_CFI				/* The flash is CFI compatible	*/
#define CONFIG_FLASH_CFI_DRIVER			/* Use common CFI driver	*/
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT	/* no byte writes on IXP4xx	*/
#endif

#define FLASH_BASE0_PRELIM	CONFIG_SYS_FLASH_BASE		/* FLASH bank #0	*/

#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	256	/* max number of sectors on one chip	*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	1000	/* Timeout for Flash Write (in ms)	*/

#define CONFIG_SYS_FLASH_WORD_SIZE	unsigned char	/* flash word size (width)	*/
#define CONFIG_SYS_FLASH_ADDR0		0x5555	/* 1st address for flash config cycles	*/
#define CONFIG_SYS_FLASH_ADDR1		0x2AAA	/* 2nd address for flash config cycles	*/
/*
 * The following defines are added for buggy IOP480 byte interface.
 * All other boards should use the standard values (CPCI405 etc.)
 */
#define CONFIG_SYS_FLASH_READ0		0x0000	/* 0 is standard			*/
#define CONFIG_SYS_FLASH_READ1		0x0001	/* 1 is standard			*/
#define CONFIG_SYS_FLASH_READ2		0x0002	/* 2 is standard			*/

#define CONFIG_SYS_FLASH_EMPTY_INFO		/* print 'E' for empty sector on flinfo */

#define	CONFIG_ENV_IS_IN_FLASH	1

#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + CONFIG_SYS_MONITOR_LEN)
#if defined(CONFIG_SCPU)
/* no redundant environment on SCPU */
#define CONFIG_ENV_SECT_SIZE	0x20000 /* size of one complete sector		*/
#define	CONFIG_ENV_SIZE		0x4000	/* Total Size of Environment Sector	*/
#else
#define CONFIG_ENV_SECT_SIZE	0x1000	/* size of one complete sector		*/
#define	CONFIG_ENV_SIZE		0x1000	/* Total Size of Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)
#endif

#if !defined(CONFIG_SCPU)
/*
 * NAND-FLASH stuff
 */
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x51000000	/* NAND FLASH Base Address */
#endif

/*
 * GPIO settings
 */

/* FPGA program pin configuration */
#define CONFIG_SYS_GPIO_PRG		12		/* FPGA program pin (cpu output)*/
#define CONFIG_SYS_GPIO_CLK		10		/* FPGA clk pin (cpu output)    */
#define CONFIG_SYS_GPIO_DATA		14		/* FPGA data pin (cpu output)   */
#define CONFIG_SYS_GPIO_INIT		13		/* FPGA init pin (cpu input)    */
#define CONFIG_SYS_GPIO_DONE		11		/* FPGA done pin (cpu input)    */

/* other GPIO's */
#define CONFIG_SYS_GPIO_RESTORE_INT	0
#define CONFIG_SYS_GPIO_RESTART_INT	1
#define CONFIG_SYS_GPIO_SYS_RUNNING	2
#define CONFIG_SYS_GPIO_PCI_INTA	3
#define CONFIG_SYS_GPIO_PCI_INTB	4
#define CONFIG_SYS_GPIO_I2C_SCL	6
#define CONFIG_SYS_GPIO_I2C_SDA	7
#define CONFIG_SYS_GPIO_FPGA_RESET	9
#define CONFIG_SYS_GPIO_CLK_33M	15

/*
 * I2C stuff
 */

/* enable I2C and select the hardware/software driver */
#undef	CONFIG_HARD_I2C			/* I2C with hardware support	*/
#define	CONFIG_SOFT_I2C		1	/* I2C bit-banged		*/

#define CONFIG_SYS_I2C_SPEED		83000	/* 83 kHz is supposed to work	*/
#define CONFIG_SYS_I2C_SLAVE		0xFE

/*
 * Software (bit-bang) I2C driver configuration
 */
#define PB_SCL		(1 << CONFIG_SYS_GPIO_I2C_SCL)
#define PB_SDA		(1 << CONFIG_SYS_GPIO_I2C_SDA)

#define I2C_INIT	GPIO_OUTPUT_ENABLE(CONFIG_SYS_GPIO_I2C_SCL)
#define I2C_ACTIVE	GPIO_OUTPUT_ENABLE(CONFIG_SYS_GPIO_I2C_SDA)
#define I2C_TRISTATE	GPIO_OUTPUT_DISABLE(CONFIG_SYS_GPIO_I2C_SDA)
#define I2C_READ	((*IXP425_GPIO_GPINR & PB_SDA) != 0)
#define I2C_SDA(bit)	if (bit) GPIO_OUTPUT_SET(CONFIG_SYS_GPIO_I2C_SDA);	\
	                else     GPIO_OUTPUT_CLEAR(CONFIG_SYS_GPIO_I2C_SDA)
#define I2C_SCL(bit)	if (bit) GPIO_OUTPUT_SET(CONFIG_SYS_GPIO_I2C_SCL);	\
			else     GPIO_OUTPUT_CLEAR(CONFIG_SYS_GPIO_I2C_SCL)
#define I2C_DELAY	udelay(3)	/* 1/4 I2C clock duration */

/*
 * I2C RTC
 */
#if 0 /* test-only */
#define CONFIG_RTC_DS1340	1
#define CONFIG_SYS_I2C_RTC_ADDR	0x68
#else
/* M41T11 Serial Access Timekeeper(R) SRAM */
#define CONFIG_RTC_M41T11	1
#define CONFIG_SYS_I2C_RTC_ADDR	0x68
#define CONFIG_SYS_M41T11_BASE_YEAR	1900	/* play along with the linux driver */
#endif

/*
 * Spartan3 FPGA configuration support
 */
#define CONFIG_SYS_FPGA_MAX_SIZE	700*1024	/* 700kByte for XC3S500E	*/

#define CONFIG_SYS_FPGA_PRG	(1 << CONFIG_SYS_GPIO_PRG)	/* FPGA program pin (cpu output)*/
#define CONFIG_SYS_FPGA_CLK	(1 << CONFIG_SYS_GPIO_CLK)	/* FPGA clk pin (cpu output)    */
#define CONFIG_SYS_FPGA_DATA	(1 << CONFIG_SYS_GPIO_DATA)	/* FPGA data pin (cpu output)   */
#define CONFIG_SYS_FPGA_INIT	(1 << CONFIG_SYS_GPIO_INIT)	/* FPGA init pin (cpu input)    */
#define CONFIG_SYS_FPGA_DONE	(1 << CONFIG_SYS_GPIO_DONE)	/* FPGA done pin (cpu input)    */

/*
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE	32

/* additions for new relocation code, must be added to all boards */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_INIT_SP_ADDR        \
	(CONFIG_SYS_SDRAM_BASE + 0x1000 - GENERATED_GBL_DATA_SIZE)

#endif  /* __CONFIG_H */

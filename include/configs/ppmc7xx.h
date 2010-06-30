/*
 * ppmc7xx.h
 * ---------
 *
 * Wind River PPMC 7xx/74xx board configuration file.
 *
 * By Richard Danter (richard.danter@windriver.com)
 * Copyright (C) 2005 Wind River Systems
 */


#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_PPMC7XX


/*===================================================================
 *
 * User configurable settings - Modify to your preference
 *
 *===================================================================
 */

/*
 * Debug
 *
 * DEBUG		- Define this is you want extra debug info
 * GTREGREAD		- Required to build with debug
 * do_bdinfo		- Required to build with debug
 */

#ifdef	DEBUG
#define	GTREGREAD(x)	0xFFFFFFFF
#define	do_bdinfo(a,b,c,d)
#endif

/*
 * CPU type
 *
 * CONFIG_7xx		- We have a 750 or 755 CPU
 * CONFIG_74xx		- We have a 7400 CPU
 * CONFIG_ALTIVEC	- We have altivec enabled CPU (only 7400)
 * CONFIG_BUS_CLK	- System bus clock in Hz
 */

#define	CONFIG_7xx
#undef	CONFIG_74xx
#undef	CONFIG_ALTIVEC
#define CONFIG_BUS_CLK	66000000


/*
 * Monitor configuration
 *
 * List of command sets to include in shell
 *
 * The following command sets have been tested and known to work:
 *
 * CMD_CACHE		- Cache control commands
 * CMD_MEMORY		- Memory display, change and test commands
 * CMD_FLASH		- Erase and program flash
 * CMD_ENV		- Environment commands
 * CMD_RUN		- Run commands stored in env vars
 * CMD_ELF		- Load ELF files
 * CMD_NET		- Networking/file download commands
 * CMD_PIN		- ICMP Echo Request command
 * CMD_PCI		- PCI Bus scanning command
 */

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

#define CONFIG_CMD_FLASH
#define CONFIG_CMD_SAVEENV
#define CONFIG_CMD_RUN
#define CONFIG_CMD_ELF
#define CONFIG_CMD_NET
#define CONFIG_CMD_PING
#define CONFIG_CMD_PCI

#undef CONFIG_CMD_KGDB


/*
 * Serial configuration
 *
 * CONFIG_CONS_INDEX		- Serial console port number (COM1)
 * CONFIG_BAUDRATE		- Serial speed
 */

#define CONFIG_CONS_INDEX	1
#define CONFIG_BAUDRATE		9600


/*
 * PCI config
 *
 * CONFIG_PCI			- Enable PCI bus
 * CONFIG_PCI_PNP		- Enable Plug & Play support
 * CONFIG_PCI_SCAN_SHOW		- Enable display of devices at startup
 */

#define	CONFIG_PCI
#define	CONFIG_PCI_PNP
#undef	CONFIG_PCI_SCAN_SHOW


/*
 * Network config
 *
 * CONFIG_NET_MULTI		- Support for multiple network interfaces
 * CONFIG_EEPRO100		- Intel 8255x Ethernet Controller
 * CONFIG_EEPRO100_SROM_WRITE	- Enable writing to network card ROM
 */

#define	CONFIG_NET_MULTI
#define	CONFIG_EEPRO100
#define	CONFIG_EEPRO100_SROM_WRITE


/*
 * Enable extra init functions
 *
 * CONFIG_MISC_INIT_F		- Call pre-relocation init functions
 * CONFIG_MISC_INIT_R		- Call post relocation init functions
 */

#undef	CONFIG_MISC_INIT_F
#define CONFIG_MISC_INIT_R


/*
 * Boot config
 *
 * CONFIG_BOOTCOMMAND		- Command(s) to execute to auto-boot
 * CONFIG_BOOTDELAY		- How long to wait before auto-boot (in sec)
 */

#define CONFIG_BOOTCOMMAND		\
	"bootp;" \
	"setenv bootargs root=/dev/nfs rw nfsroot=$(serverip):$(rootpath) " \
	"ip=$(ipaddr):$(serverip):$(gatewayip):$(netmask):$(hostname)::off;" \
	"bootm"
#define CONFIG_BOOTDELAY		5


/*===================================================================
 *
 * Board configuration settings - You should not need to modify these
 *
 *===================================================================
 */


/*
 * Memory map
 *
 * This board runs in a standard CHRP (Map-B) configuration.
 *
 *	Type	    Start	End	    Size    Width   Chip Sel
 *	----------- ----------- ----------- ------- ------- --------
 *	SDRAM	    0x00000000	0x04000000  64MB    64b	    SDRAMCS0
 *	User LED's  0x78000000				    RCS3
 *	UART	    0x7C000000				    RCS2
 *	Mailbox	    0xFF000000				    RCS1
 *	Flash	    0xFFC00000	0xFFFFFFFF   4MB    64b	    RCS0
 *
 * Flash sectors are laid out as follows.
 *
 *	Sector	Start		End	Size	Comments
 *	------- ----------- ----------- ------- -----------
 *	 0	0xFFC00000  0xFFC3FFFF	256KB
 *	 1	0xFFC40000  0xFFC7FFFF	256KB
 *	 2	0xFFC80000  0xFFCBFFFF	256KB
 *	 3	0xFFCC0000  0xFFCFFFFF	256KB
 *	 4	0xFFD00000  0xFFD3FFFF	256KB
 *	 5	0xFFD40000  0xFFD7FFFF	256KB
 *	 6	0xFFD80000  0xFFDBFFFF	256KB
 *	 7	0xFFDC0000  0xFFDFFFFF	256KB
 *	 8	0xFFE00000  0xFFE3FFFF	256KB
 *	 9	0xFFE40000  0xFFE7FFFF	256KB
 *	10	0xFFE80000  0xFFEBFFFF	256KB
 *	11	0xFFEC0000  0xFFEFFFFF	256KB
 *	12	0xFFF00000  0xFFF3FFFF	256KB	U-Boot code here
 *	13	0xFFF40000  0xFFF7FFFF	256KB
 *	14	0xFFF80000  0xFFFBFFFF	256KB
 *	15	0xFFFC0000  0xFFFDFFFF	128KB
 *	16	0xFFFE0000  0xFFFE7FFF	 32KB	U-Boot env vars here
 *	17	0xFFFE8000  0xFFFEFFFF	 32KB	U-Boot backup copy of env vars here
 *	18	0xFFFF0000  0xFFFFFFFF	 64KB
 */


/*
 * SDRAM config - see memory map details above.
 *
 * CONFIG_SYS_SDRAM_BASE		- Start address of SDRAM, this _must_ be zero!
 * CONFIG_SYS_SDRAM_SIZE		- Total size of contiguous SDRAM bank(s)
 */

#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_SDRAM_SIZE		0x04000000


/*
 * Flash config - see memory map details above.
 *
 * CONFIG_SYS_FLASH_BASE		- Start address of flash memory
 * CONFIG_SYS_FLASH_SIZE		- Total size of contiguous flash mem
 * CONFIG_SYS_FLASH_ERASE_TOUT		- Erase timeout in ms
 * CONFIG_SYS_FLASH_WRITE_TOUT		- Write timeout in ms
 * CONFIG_SYS_MAX_FLASH_BANKS		- Number of banks of flash on board
 * CONFIG_SYS_MAX_FLASH_SECT		- Number of sectors in a bank
 */

#define CONFIG_SYS_FLASH_BASE		0xFFC00000
#define CONFIG_SYS_FLASH_SIZE		0x00400000
#define CONFIG_SYS_FLASH_ERASE_TOUT	250000
#define CONFIG_SYS_FLASH_WRITE_TOUT	5000
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	19


/*
 * Monitor config - see memory map details above
 *
 * CONFIG_SYS_MONITOR_BASE		- Base address of monitor code
 * CONFIG_SYS_MALLOC_LEN		- Size of malloc pool (128KB)
 */

#define CONFIG_SYS_MONITOR_BASE	TEXT_BASE
#define CONFIG_SYS_MALLOC_LEN		0x20000


/*
 * Command shell settings
 *
 * CONFIG_SYS_BARGSIZE			- Boot Argument buffer size
 * CONFIG_SYS_BOOTMAPSZ		- Size of app's mapped RAM at boot (Linux=8MB)
 * CONFIG_SYS_CBSIZE			- Console Buffer (input) size
 * CONFIG_SYS_LOAD_ADDR		- Default load address
 * CONFIG_SYS_LONGHELP			- Provide more detailed help
 * CONFIG_SYS_MAXARGS			- Number of args accepted by monitor commands
 * CONFIG_SYS_MEMTEST_START		- Start address of test to run on RAM
 * CONFIG_SYS_MEMTEST_END		- End address of RAM test
 * CONFIG_SYS_PBSIZE			- Print Buffer (output) size
 * CONFIG_SYS_PROMPT			- Prompt string
 */

#define CONFIG_SYS_BARGSIZE		1024
#define CONFIG_SYS_BOOTMAPSZ		0x800000
#define CONFIG_SYS_CBSIZE		1024
#define CONFIG_SYS_LOAD_ADDR		0x100000
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_MAXARGS		16
#define CONFIG_SYS_MEMTEST_START	0x00040000
#define CONFIG_SYS_MEMTEST_END		0x00040100
#define CONFIG_SYS_PBSIZE		1024
#define CONFIG_SYS_PROMPT		"=> "


/*
 * Environment config - see memory map details above
 *
 * CONFIG_ENV_IS_IN_FLASH		- The env variables are stored in flash
 * CONFIG_ENV_ADDR			- Address of the sector containing env vars
 * CONFIG_ENV_SIZE			- Ammount of RAM for env vars (used to save RAM, 4KB)
 * CONFIG_ENV_SECT_SIZE		- Size of sector containing env vars (32KB)
 */

#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_ADDR		0xFFFE0000
#define CONFIG_ENV_SIZE		0x1000
#define CONFIG_ENV_ADDR_REDUND	0xFFFE8000
#define CONFIG_ENV_SIZE_REDUND	0x1000
#define CONFIG_ENV_SECT_SIZE	0x8000


/*
 * Initial RAM config
 *
 * Since the main system RAM is initialised very early, we place the INIT_RAM
 * in the main system RAM just above the exception vectors. The contents are
 * copied to top of RAM by the init code.
 *
 * CONFIG_SYS_INIT_RAM_ADDR		- Address of Init RAM, above exception vect
 * CONFIG_SYS_INIT_RAM_END		- Size of Init RAM
 * CONFIG_SYS_GBL_DATA_SIZE		- Ammount of RAM to reserve for global data
 * CONFIG_SYS_GBL_DATA_OFFSET		- Start of global data, top of stack
 */

#define CONFIG_SYS_INIT_RAM_ADDR	(CONFIG_SYS_SDRAM_BASE + 0x4000)
#define CONFIG_SYS_INIT_RAM_END	0x4000
#define CONFIG_SYS_GBL_DATA_SIZE	128
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END - CONFIG_SYS_GBL_DATA_SIZE)


/*
 * Initial BAT config
 *
 * BAT0	- System SDRAM
 * BAT1 - LED's and Serial Port
 * BAT2 - PCI Memory
 * BAT3 - PCI I/O including Flash Memory
 */

#define CONFIG_SYS_IBAT0L (CONFIG_SYS_SDRAM_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT0U (CONFIG_SYS_SDRAM_BASE | BATU_BL_64M | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT0L CONFIG_SYS_IBAT0L
#define CONFIG_SYS_DBAT0U CONFIG_SYS_IBAT0U

#define CONFIG_SYS_IBAT1L (0x70000000 | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT1U (0x70000000 | BATU_BL_256M | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT1L (0x70000000 | BATL_PP_RW | BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT1U (0x70000000 | BATU_BL_256M | BATU_VS | BATU_VP)

#define CONFIG_SYS_IBAT2L (0x80000000 | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT2U (0x80000000 | BATU_BL_256M | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT2L (0x80000000 | BATL_PP_RW | BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT2U (0x80000000 | BATU_BL_256M | BATU_VS | BATU_VP)

#define CONFIG_SYS_IBAT3L (0xF0000000 | BATL_PP_RW | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT3U (0xF0000000 | BATU_BL_256M | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT3L (0xF0000000 | BATL_PP_RW | BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT3U (0xF0000000 | BATU_BL_256M | BATU_VS | BATU_VP)


/*
 * Cache config
 *
 * CONFIG_SYS_CACHELINE_SIZE		- Size of a cache line (CPU specific)
 * CONFIG_SYS_L2			- L2 cache enabled if defined
 * L2_INIT			- L2 cache init flags
 * L2_ENABLE			- L2 cache enable flags
 */

#define CONFIG_SYS_CACHELINE_SIZE	32
#undef	CONFIG_SYS_L2
#define L2_INIT			0
#define L2_ENABLE		0


/*
 * Clocks config
 *
 * CONFIG_SYS_BUS_CLK			- Bus clock frequency in Hz
 * CONFIG_SYS_HZ			- Decrementer freq in Hz
 */

#define CONFIG_SYS_BUS_CLK		CONFIG_BUS_CLK
#define CONFIG_SYS_HZ			1000


/*
 * Serial port config
 *
 * CONFIG_SYS_BAUDRATE_TABLE		- List of valid baud rates
 * CONFIG_SYS_NS16550			- Include the NS16550 driver
 * CONFIG_SYS_NS16550_SERIAL		- Include the serial (wrapper) driver
 * CONFIG_SYS_NS16550_CLK		- Frequency of reference clock
 * CONFIG_SYS_NS16550_REG_SIZE		- 64-bit accesses to 8-bit port
 * CONFIG_SYS_NS16550_COM1		- Base address of 1st serial port
 */

#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_CLK		3686400
#define CONFIG_SYS_NS16550_REG_SIZE	-8
#define CONFIG_SYS_NS16550_COM1	0x7C000000


/*
 * PCI Config - Address Map B (CHRP)
 */

#define CONFIG_SYS_PCI_MEMORY_BUS	0x00000000
#define CONFIG_SYS_PCI_MEMORY_PHYS	0x00000000
#define CONFIG_SYS_PCI_MEMORY_SIZE	0x40000000
#define CONFIG_SYS_PCI_MEM_BUS		0x80000000
#define CONFIG_SYS_PCI_MEM_PHYS	0x80000000
#define CONFIG_SYS_PCI_MEM_SIZE	0x7D000000
#define CONFIG_SYS_ISA_MEM_BUS		0x00000000
#define CONFIG_SYS_ISA_MEM_PHYS	0xFD000000
#define CONFIG_SYS_ISA_MEM_SIZE	0x01000000
#define CONFIG_SYS_PCI_IO_BUS		0x00800000
#define CONFIG_SYS_PCI_IO_PHYS		0xFE800000
#define CONFIG_SYS_PCI_IO_SIZE		0x00400000
#define CONFIG_SYS_ISA_IO_BUS		0x00000000
#define CONFIG_SYS_ISA_IO_PHYS		0xFE000000
#define CONFIG_SYS_ISA_IO_SIZE		0x00800000
#define CONFIG_SYS_ISA_IO_BASE_ADDRESS CONFIG_SYS_ISA_IO_PHYS
#define CONFIG_SYS_ISA_IO		CONFIG_SYS_ISA_IO_PHYS
#define CONFIG_SYS_60X_PCI_IO_OFFSET	CONFIG_SYS_ISA_IO_PHYS


/*
 * Extra init functions
 *
 * CONFIG_SYS_BOARD_ASM_INIT		- Call assembly init code
 */

#define CONFIG_SYS_BOARD_ASM_INIT


/*
 * Boot flags
 *
 * BOOTFLAG_COLD		- Indicates a power-on boot
 * BOOTFLAG_WARM		- Indicates a software reset
 */

#define BOOTFLAG_COLD		0x01
#define BOOTFLAG_WARM		0x02


#endif /* __CONFIG_H */

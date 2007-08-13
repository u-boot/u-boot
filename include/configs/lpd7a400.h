/*
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

#ifndef __LPD7A400_H_
#define __LPD7A400_H_

#define CONFIG_LPD7A400		/* Logic LH7A400 SDK */

#undef CONFIG_USE_IRQ

/*
 * This board uses the logic LH7A400-10 card engine
 */
#include <configs/lpd7a400-10.h>

#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_INITRD_TAG		1

/*
 * Size of malloc() pool
 */
#define CFG_MALLOC_LEN		(CFG_ENV_SIZE + 128*1024)
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

/*
 * select serial console configuration
 */
#define CONFIG_CONSOLE_UART2	/* UART2 LH7A40x for console */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BAUDRATE		115200
#define CONFIG_IPADDR		192.168.1.100
#define CONFIG_NETMASK		255.255.1.0
#define CONFIG_SERVERIP		192.168.1.1

#define	CONFIG_TIMESTAMP	1	/* Print timestamp info for images */


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

#ifndef USE_920T_MMU
    #define CONFIG_CMD_PING
    #undef CONFIG_CMD_CACHE
#else
    #define CONFIG_CMD_DATE
#endif


#define CONFIG_BOOTDELAY	3

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	115200		/* speed to run kgdb serial port */
/* what's this ? it's not used anywhere */
#define CONFIG_KGDB_SER_INDEX	1		/* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */
#define	CFG_LONGHELP				/* undef to save memory		*/
#define	CFG_PROMPT		"LPD7A400> "	/* Monitor Command Prompt	*/
#define	CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define	CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0xc0300000	/* memtest works on	*/
#define CFG_MEMTEST_END		0xc0500000	/* 2 MB in DRAM	*/

#undef  CFG_CLKS_IN_HZ		/* everything, incl board info, in Hz */

#define	CFG_LOAD_ADDR		0xc0f00000	/* default load address	*/

/* valid baudrates */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/* size and location of u-boot in flash */
#define CFG_MONITOR_BASE	CFG_FLASH_BASE
#define CFG_MONITOR_LEN		(256<<10)

#define	CFG_ENV_IS_IN_FLASH	1

/* Address and size of Primary Environment Sector	*/
#define CFG_ENV_ADDR		(CFG_FLASH_BASE + 0xFC0000)
#define CFG_ENV_SIZE		0x40000

#endif  /* __LPD7A400_H_ */

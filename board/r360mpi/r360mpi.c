/*
 * (C) Copyright 2001-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

#include <common.h>
#include <config.h>
#include <mpc8xx.h>
#include <i2c.h>

#include <commproc.h>
#include <command.h>
#include <malloc.h>

#include <linux/types.h>
#include <linux/string.h>       /* for strdup */


/*
 *  Memory Controller Using
 *
 *  CS0 - Flash memory		(0x40000000)
 *  CS1 - FLASH memory		(0x????????)
 *  CS2 - SDRAM			(0x00000000)
 *  CS3 -
 *  CS4 -
 *  CS5 -
 *  CS6 - PCMCIA device
 *  CS7 - PCMCIA device
 */

/* ------------------------------------------------------------------------- */

#define _not_used_	0xffffffff

const uint sdram_table[]=
{
	/* single read. (offset 0 in upm RAM) */
	0x1f07fc04, 0xeeaefc04, 0x11adfc04, 0xefbbbc00,
	0x1ff77c47,

	/* MRS initialization (offset 5) */

	0x1ff77c34, 0xefeabc34, 0x1fb57c35,

	/* burst read. (offset 8 in upm RAM) */
	0x1f07fc04, 0xeeaefc04, 0x10adfc04, 0xf0affc00,
	0xf0affc00, 0xf1affc00, 0xefbbbc00, 0x1ff77c47,
	_not_used_, _not_used_, _not_used_, _not_used_,
	_not_used_, _not_used_, _not_used_, _not_used_,

	/* single write. (offset 18 in upm RAM) */
	0x1f27fc04, 0xeeaebc00, 0x01b93c04, 0x1ff77c47,
	_not_used_, _not_used_, _not_used_, _not_used_,

	/* burst write. (offset 20 in upm RAM) */
	0x1f07fc04, 0xeeaebc00, 0x10ad7c00, 0xf0affc00,
	0xf0affc00, 0xe1bbbc04, 0x1ff77c47, _not_used_,
	_not_used_, _not_used_, _not_used_, _not_used_,
	_not_used_, _not_used_, _not_used_, _not_used_,

	/* refresh. (offset 30 in upm RAM) */
	0x1ff5fc84, 0xfffffc04, 0xfffffc04, 0xfffffc04,
	0xfffffc84, 0xfffffc07, _not_used_, _not_used_,
	_not_used_, _not_used_, _not_used_, _not_used_,

	/* exception. (offset 3c in upm RAM) */
	0x7ffffc07, _not_used_, _not_used_, _not_used_ };

/* ------------------------------------------------------------------------- */

/*
 * Check Board Identity:
 */

int checkboard (void)
{
	puts ("Board: R360 MPI Board\n");
	return 0;
}

/* ------------------------------------------------------------------------- */

static long int dram_size (long int, long int *, long int);

/* ------------------------------------------------------------------------- */

phys_size_t initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	long int size8, size9;
	long int size_b0 = 0;
	unsigned long reg;

	upmconfig (UPMA, (uint *) sdram_table,
			   sizeof (sdram_table) / sizeof (uint));

	/*
	 * Preliminary prescaler for refresh (depends on number of
	 * banks): This value is selected for four cycles every 62.4 us
	 * with two SDRAM banks or four cycles every 31.2 us with one
	 * bank. It will be adjusted after memory sizing.
	 */
	memctl->memc_mptpr = CONFIG_SYS_MPTPR_2BK_8K;

	memctl->memc_mar = 0x00000088;

	/*
	 * Map controller bank 2 to the SDRAM bank at
	 * preliminary address - these have to be modified after the
	 * SDRAM size has been determined.
	 */
	memctl->memc_or2 = CONFIG_SYS_OR2_PRELIM;
	memctl->memc_br2 = CONFIG_SYS_BR2_PRELIM;

	memctl->memc_mamr = CONFIG_SYS_MAMR_8COL & (~(MAMR_PTAE));	/* no refresh yet */

	udelay (200);

	/* perform SDRAM initializsation sequence */

	memctl->memc_mcr = 0x80004105;	/* SDRAM bank 0 */
	udelay (200);
	memctl->memc_mcr = 0x80004230;	/* SDRAM bank 0 - execute twice */
	udelay (200);

	memctl->memc_mamr |= MAMR_PTAE;	/* enable refresh */

	udelay (1000);

	/*
	 * Check Bank 2 Memory Size for re-configuration
	 *
	 * try 8 column mode
	 */
	size8 = dram_size (CONFIG_SYS_MAMR_8COL, (long *) SDRAM_BASE2_PRELIM,
					   SDRAM_MAX_SIZE);

	udelay (1000);

	/*
	 * try 9 column mode
	 */
	size9 = dram_size (CONFIG_SYS_MAMR_9COL, (long *) SDRAM_BASE2_PRELIM,
					   SDRAM_MAX_SIZE);

	if (size8 < size9) {		/* leave configuration at 9 columns */
		size_b0 = size9;
/*	debug ("SDRAM Bank 0 in 9 column mode: %ld MB\n", size >> 20);	*/
	} else {			/* back to 8 columns            */
		size_b0 = size8;
		memctl->memc_mamr = CONFIG_SYS_MAMR_8COL;
		udelay (500);
/*	debug ("SDRAM Bank 0 in 8 column mode: %ld MB\n", size >> 20);	*/
	}

	udelay (1000);

	/*
	 * Adjust refresh rate depending on SDRAM type, both banks
	 * For types > 128 MBit leave it at the current (fast) rate
	 */
	if ((size_b0 < 0x02000000)) {
		/* reduce to 15.6 us (62.4 us / quad) */
		memctl->memc_mptpr = CONFIG_SYS_MPTPR_2BK_4K;
		udelay (1000);
	}

	/*
	 * Final mapping
	 */

	memctl->memc_or1 = ((-size_b0) & 0xFFFF0000) | CONFIG_SYS_OR_TIMING_SDRAM;
	memctl->memc_br1 = (CONFIG_SYS_SDRAM_BASE & BR_BA_MSK) | BR_MS_UPMA | BR_V;

	/* adjust refresh rate depending on SDRAM type, one bank */
	reg = memctl->memc_mptpr;
	reg >>= 1;		/* reduce to CONFIG_SYS_MPTPR_1BK_8K / _4K */
	memctl->memc_mptpr = reg;

	udelay (10000);

#ifdef CONFIG_CAN_DRIVER
	/* Initialize OR3 / BR3 */
	memctl->memc_or3 = CONFIG_SYS_OR3_CAN;		/* switch GPLB_5 to GPLA_5 */
	memctl->memc_br3 = CONFIG_SYS_BR3_CAN;

	/* Initialize MBMR */
	memctl->memc_mbmr = MBMR_GPL_B4DIS;	/* GPL_B4 works as UPWAITB */

	/* Initialize UPMB for CAN: single read */
	memctl->memc_mdr = 0xFFFFC004;
	memctl->memc_mcr = 0x0100 | UPMB;

	memctl->memc_mdr = 0x0FFFD004;
	memctl->memc_mcr = 0x0101 | UPMB;

	memctl->memc_mdr = 0x0FFFC000;
	memctl->memc_mcr = 0x0102 | UPMB;

	memctl->memc_mdr = 0x3FFFC004;
	memctl->memc_mcr = 0x0103 | UPMB;

	memctl->memc_mdr = 0xFFFFDC05;
	memctl->memc_mcr = 0x0104 | UPMB;

	/* Initialize UPMB for CAN: single write */
	memctl->memc_mdr = 0xFFFCC004;
	memctl->memc_mcr = 0x0118 | UPMB;

	memctl->memc_mdr = 0xCFFCD004;
	memctl->memc_mcr = 0x0119 | UPMB;

	memctl->memc_mdr = 0x0FFCC000;
	memctl->memc_mcr = 0x011A | UPMB;

	memctl->memc_mdr = 0x7FFCC004;
	memctl->memc_mcr = 0x011B | UPMB;

	memctl->memc_mdr = 0xFFFDCC05;
	memctl->memc_mcr = 0x011C | UPMB;
#endif

	return (size_b0);
}

/* ------------------------------------------------------------------------- */

/*
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'. Some (not all) hardware errors are detected:
 * - short between address lines
 * - short between data lines
 */

static long int dram_size (long int mamr_value,
			   long int *base, long int maxsize)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;

	memctl->memc_mamr = mamr_value;

	return (get_ram_size(base, maxsize));
}

/* ------------------------------------------------------------------------- */

void r360_i2c_lcd_write (uchar data0, uchar data1)
{
	if (i2c_write (CONFIG_SYS_I2C_LCD_ADDR, data0, 1, &data1, 1)) {
		printf("Can't write lcd data 0x%02X 0x%02X.\n", data0, data1);
	}
}

/* ------------------------------------------------------------------------- */

/*-----------------------------------------------------------------------
 * Keyboard Controller
 */

/* Number of bytes returned from Keyboard Controller */
#define KEYBD_KEY_MAX	16				/* maximum key number */
#define KEYBD_DATALEN	((KEYBD_KEY_MAX + 7) / 8)	/* normal key scan data */

static uchar *key_match (uchar *);

int misc_init_r (void)
{
	char kbd_data[KEYBD_DATALEN];
	char keybd_env[2 * KEYBD_DATALEN + 1];
	char *str;
	int i;

	i2c_init (CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);

	i2c_read (CONFIG_SYS_I2C_KEY_ADDR, 0, 0, (uchar *)kbd_data, KEYBD_DATALEN);

	for (i = 0; i < KEYBD_DATALEN; ++i) {
		sprintf (keybd_env + i + i, "%02X", kbd_data[i]);
	}
	setenv ("keybd", keybd_env);

	str = strdup ((char *)key_match ((uchar *)keybd_env));	/* decode keys */

#ifdef CONFIG_PREBOOT	/* automatically configure "preboot" command on key match */
	setenv ("preboot", str);	/* set or delete definition */
#endif /* CONFIG_PREBOOT */
	if (str != NULL) {
		free (str);
	}

	return (0);
}

/*-----------------------------------------------------------------------
 * Check if pressed key(s) match magic sequence,
 * and return the command string associated with that key(s).
 *
 * If no key press was decoded, NULL is returned.
 *
 * Note: the first character of the argument will be overwritten with
 * the "magic charcter code" of the decoded key(s), or '\0'.
 *
 *
 * Note: the string points to static environment data and must be
 * saved before you call any function that modifies the environment.
 */
#ifdef CONFIG_PREBOOT

static uchar kbd_magic_prefix[] = "key_magic";
static uchar kbd_command_prefix[] = "key_cmd";

static uchar *key_match (uchar * kbd_str)
{
	uchar magic[sizeof (kbd_magic_prefix) + 1];
	uchar cmd_name[sizeof (kbd_command_prefix) + 1];
	uchar *str, *suffix;
	uchar *kbd_magic_keys;
	char *cmd;

	/*
	 * The following string defines the characters that can pe appended
	 * to "key_magic" to form the names of environment variables that
	 * hold "magic" key codes, i. e. such key codes that can cause
	 * pre-boot actions. If the string is empty (""), then only
	 * "key_magic" is checked (old behaviour); the string "125" causes
	 * checks for "key_magic1", "key_magic2" and "key_magic5", etc.
	 */
	if ((kbd_magic_keys = (uchar *)getenv ("magic_keys")) != NULL) {
		/* loop over all magic keys;
		 * use '\0' suffix in case of empty string
		 */
		for (suffix = kbd_magic_keys;
		     *suffix || suffix == kbd_magic_keys;
		     ++suffix) {
			sprintf ((char *)magic, "%s%c", kbd_magic_prefix, *suffix);

#if 0
			printf ("### Check magic \"%s\"\n", magic);
#endif

			if ((str = (uchar *)getenv ((char *)magic)) != 0) {

#if 0
				printf ("### Compare \"%s\" \"%s\"\n",
					kbd_str, str);
#endif
				if (strcmp ((char *)kbd_str, (char *)str) == 0) {
					sprintf ((char *)cmd_name, "%s%c",
						 kbd_command_prefix,
						 *suffix);

					if ((cmd = getenv ((char *)cmd_name)) != 0) {
#if 0
						printf ("### Set PREBOOT to $(%s): \"%s\"\n",
							cmd_name, cmd);
#endif
						return ((uchar *)cmd);
					}
				}
			}
		}
	}
#if 0
	printf ("### Delete PREBOOT\n");
#endif
	*kbd_str = '\0';
	return (NULL);
}
#endif	/* CONFIG_PREBOOT */

/* Read Keyboard status */
int do_kbd (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	uchar kbd_data[KEYBD_DATALEN];
	uchar keybd_env[2 * KEYBD_DATALEN + 1];
	int i;

	i2c_init (CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);

	/* Read keys */
	i2c_read (CONFIG_SYS_I2C_KEY_ADDR, 0, 0, kbd_data, KEYBD_DATALEN);

	puts ("Keys:");
	for (i = 0; i < KEYBD_DATALEN; ++i) {
		sprintf ((char *)(keybd_env + i + i), "%02X", kbd_data[i]);
		printf (" %02x", kbd_data[i]);
	}
	putc ('\n');
	setenv ("keybd", (char *)keybd_env);
	return 0;
}

U_BOOT_CMD(
	kbd,	1,	1,	do_kbd,
	"read keyboard status",
	""
);

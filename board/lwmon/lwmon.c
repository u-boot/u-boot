/***********************************************************************
 *
M* Modul:         lwmon.c
M*
M* Content:       LWMON specific U-Boot commands.
 *
 * (C) Copyright 2001, 2002
 * DENX Software Engineering
 * Wolfgang Denk, wd@denx.de
 * All rights reserved.
 *
D* Design:        wd@denx.de
C* Coding:        wd@denx.de
V* Verification:  dzu@denx.de
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
 ***********************************************************************/

/*---------------------------- Headerfiles ----------------------------*/
#include <common.h>
#include <mpc8xx.h>
#include <commproc.h>
#include <i2c.h>
#include <command.h>
#include <malloc.h>
#include <post.h>
#include <serial.h>

#include <linux/types.h>
#include <linux/string.h>	/* for strdup */

DECLARE_GLOBAL_DATA_PTR;

/*------------------------ Local prototypes ---------------------------*/
static long int dram_size (long int, long int *, long int);
static void kbd_init (void);
static int compare_magic (uchar *kbd_data, uchar *str);


/*--------------------- Local macros and constants --------------------*/
#define	_NOT_USED_	0xFFFFFFFF

#ifdef CONFIG_MODEM_SUPPORT
static int key_pressed(void);
extern void disable_putc(void);
#endif /* CONFIG_MODEM_SUPPORT */

/*
 * 66 MHz SDRAM access using UPM A
 */
const uint sdram_table[] =
{
#if defined(CFG_MEMORY_75) || defined(CFG_MEMORY_8E)
	/*
	 * Single Read. (Offset 0 in UPM RAM)
	 */
	0x1F0DFC04, 0xEEAFBC04, 0x11AF7C04, 0xEFBAFC00,
	0x1FF5FC47, /* last */
	/*
	 * SDRAM Initialization (offset 5 in UPM RAM)
	 *
	 * This is no UPM entry point. The following definition uses
	 * the remaining space to establish an initialization
	 * sequence, which is executed by a RUN command.
	 *
	 */
		    0x1FF5FC34, 0xEFEABC34, 0x1FB57C35, /* last */
	/*
	 * Burst Read. (Offset 8 in UPM RAM)
	 */
	0x1F0DFC04, 0xEEAFBC04, 0x10AF7C04, 0xF0AFFC00,
	0xF0AFFC00, 0xF1AFFC00, 0xEFBAFC00, 0x1FF5FC47, /* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Single Write. (Offset 18 in UPM RAM)
	 */
	0x1F2DFC04, 0xEEABBC00, 0x01B27C04, 0x1FF5FC47, /* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Burst Write. (Offset 20 in UPM RAM)
	 */
	0x1F0DFC04, 0xEEABBC00, 0x10A77C00, 0xF0AFFC00,
	0xF0AFFC00, 0xE1BAFC04, 0x01FF5FC47, /* last */
					    _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Refresh  (Offset 30 in UPM RAM)
	 */
	0x1FFD7C84, 0xFFFFFC04, 0xFFFFFC04, 0xFFFFFC04,
	0xFFFFFC84, 0xFFFFFC07, /* last */
				_NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Exception. (Offset 3c in UPM RAM)
	 */
	0x7FFFFC07, /* last */
		    0xFFFFFCFF, 0xFFFFFCFF, 0xFFFFFCFF,
#endif
#ifdef CFG_MEMORY_7E
	/*
	 * Single Read. (Offset 0 in UPM RAM)
	 */
	0x0E2DBC04, 0x11AF7C04, 0xEFBAFC00, 0x1FF5FC47, /* last */
	_NOT_USED_,
	/*
	 * SDRAM Initialization (offset 5 in UPM RAM)
	 *
	 * This is no UPM entry point. The following definition uses
	 * the remaining space to establish an initialization
	 * sequence, which is executed by a RUN command.
	 *
	 */
		    0x1FF5FC34, 0xEFEABC34, 0x1FB57C35, /* last */
	/*
	 * Burst Read. (Offset 8 in UPM RAM)
	 */
	0x0E2DBC04, 0x10AF7C04, 0xF0AFFC00, 0xF0AFFC00,
	0xF1AFFC00, 0xEFBAFC00, 0x1FF5FC47, /* last */
					    _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Single Write. (Offset 18 in UPM RAM)
	 */
	0x0E29BC04, 0x01B27C04, 0x1FF5FC47, /* last */
					    _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Burst Write. (Offset 20 in UPM RAM)
	 */
	0x0E29BC04, 0x10A77C00, 0xF0AFFC00, 0xF0AFFC00,
	0xE1BAFC04, 0x1FF5FC47, /* last */
				_NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Refresh  (Offset 30 in UPM RAM)
	 */
	0x1FFD7C84, 0xFFFFFC04, 0xFFFFFC04, 0xFFFFFC04,
	0xFFFFFC84, 0xFFFFFC07, /* last */
				_NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Exception. (Offset 3c in UPM RAM)
	 */
	0x7FFFFC07, /* last */
		    0xFFFFFCFF, 0xFFFFFCFF, 0xFFFFFCFF,
#endif
};

/*
 * Check Board Identity:
 *
 */

/***********************************************************************
F* Function:     int checkboard (void) P*A*Z*
 *
P* Parameters:   none
P*
P* Returnvalue:  int - 0 is always returned
 *
Z* Intention:    This function is the checkboard() method implementation
Z*               for the lwmon board.  Only a standard message is printed.
 *
D* Design:       wd@denx.de
C* Coding:       wd@denx.de
V* Verification: dzu@denx.de
 ***********************************************************************/
int checkboard (void)
{
	puts ("Board: LICCON Konsole LCD3\n");
	return (0);
}

/***********************************************************************
F* Function:     long int initdram (int board_type) P*A*Z*
 *
P* Parameters:   int board_type
P*                - Usually type of the board - ignored here.
P*
P* Returnvalue:  long int
P*                - Size of initialized memory
 *
Z* Intention:    This function is the initdram() method implementation
Z*               for the lwmon board.
Z*               The memory controller is initialized to access the
Z*               DRAM.
 *
D* Design:       wd@denx.de
C* Coding:       wd@denx.de
V* Verification: dzu@denx.de
 ***********************************************************************/
long int initdram (int board_type)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immr->im_memctl;
	long int size_b0;
	long int size8, size9;
	int i;

	/*
	 * Configure UPMA for SDRAM
	 */
	upmconfig (UPMA, (uint *)sdram_table, sizeof(sdram_table)/sizeof(uint));

	memctl->memc_mptpr = CFG_MPTPR;

	/* burst length=4, burst type=sequential, CAS latency=2 */
	memctl->memc_mar = CFG_MAR;

	/*
	 * Map controller bank 3 to the SDRAM bank at preliminary address.
	 */
	memctl->memc_or3 = CFG_OR3_PRELIM;
	memctl->memc_br3 = CFG_BR3_PRELIM;

	/* initialize memory address register */
	memctl->memc_mamr = CFG_MAMR_8COL;	/* refresh not enabled yet */

	/* mode initialization (offset 5) */
	udelay (200);				/* 0x80006105 */
	memctl->memc_mcr = MCR_OP_RUN | MCR_MB_CS3 | MCR_MLCF (1) | MCR_MAD (0x05);

	/* run 2 refresh sequence with 4-beat refresh burst (offset 0x30) */
	udelay (1);				/* 0x80006130 */
	memctl->memc_mcr = MCR_OP_RUN | MCR_MB_CS3 | MCR_MLCF (1) | MCR_MAD (0x30);
	udelay (1);				/* 0x80006130 */
	memctl->memc_mcr = MCR_OP_RUN | MCR_MB_CS3 | MCR_MLCF (1) | MCR_MAD (0x30);

	udelay (1);				/* 0x80006106 */
	memctl->memc_mcr = MCR_OP_RUN | MCR_MB_CS3 | MCR_MLCF (1) | MCR_MAD (0x06);

	memctl->memc_mamr |= MAMR_PTAE;	/* refresh enabled */

	udelay (200);

	/* Need at least 10 DRAM accesses to stabilize */
	for (i = 0; i < 10; ++i) {
		volatile unsigned long *addr =
			(volatile unsigned long *) SDRAM_BASE3_PRELIM;
		unsigned long val;

		val = *(addr + i);
		*(addr + i) = val;
	}

	/*
	 * Check Bank 0 Memory Size for re-configuration
	 *
	 * try 8 column mode
	 */
	size8 = dram_size (CFG_MAMR_8COL, (long *)SDRAM_BASE3_PRELIM, SDRAM_MAX_SIZE);

	udelay (1000);

	/*
	 * try 9 column mode
	 */
	size9 = dram_size (CFG_MAMR_9COL, (long *)SDRAM_BASE3_PRELIM, SDRAM_MAX_SIZE);

	if (size8 < size9) {		/* leave configuration at 9 columns */
		size_b0 = size9;
		memctl->memc_mamr = CFG_MAMR_9COL | MAMR_PTAE;
		udelay (500);
	} else {			/* back to 8 columns            */
		size_b0 = size8;
		memctl->memc_mamr = CFG_MAMR_8COL | MAMR_PTAE;
		udelay (500);
	}

	/*
	 * Final mapping:
	 */

	memctl->memc_or3 = ((-size_b0) & 0xFFFF0000) |
			OR_CSNT_SAM | OR_G5LS | SDRAM_TIMING;
	memctl->memc_br3 = (CFG_SDRAM_BASE & BR_BA_MSK) | BR_MS_UPMA | BR_V;
	udelay (1000);

	return (size_b0);
}

/***********************************************************************
F* Function:     static long int dram_size (long int mamr_value,
F*                                          long int *base,
F*                                          long int maxsize) P*A*Z*
 *
P* Parameters:   long int mamr_value
P*                - Value for MAMR for the test
P*               long int *base
P*                - Base address for the test
P*               long int maxsize
P*                - Maximum size to test for
P*
P* Returnvalue:  long int
P*                - Size of probed memory
 *
Z* Intention:    Check memory range for valid RAM. A simple memory test
Z*               determines the actually available RAM size between
Z*               addresses `base' and `base + maxsize'. Some (not all)
Z*               hardware errors are detected:
Z*                - short between address lines
Z*                - short between data lines
 *
D* Design:       wd@denx.de
C* Coding:       wd@denx.de
V* Verification: dzu@denx.de
 ***********************************************************************/
static long int dram_size (long int mamr_value, long int *base, long int maxsize)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immr->im_memctl;

	memctl->memc_mamr = mamr_value;

	return (get_ram_size(base, maxsize));
}

/* ------------------------------------------------------------------------- */

#ifndef	PB_ENET_TENA
# define PB_ENET_TENA	((uint)0x00002000)	/* PB 18 */
#endif

/***********************************************************************
F* Function:     int board_early_init_f (void) P*A*Z*
 *
P* Parameters:   none
P*
P* Returnvalue:  int
P*                - 0 is always returned.
 *
Z* Intention:    This function is the board_early_init_f() method implementation
Z*               for the lwmon board.
Z*               Disable Ethernet TENA on Port B.
 *
D* Design:       wd@denx.de
C* Coding:       wd@denx.de
V* Verification: dzu@denx.de
 ***********************************************************************/
int board_early_init_f (void)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;

	/* Disable Ethernet TENA on Port B
	 * Necessary because of pull up in COM3 port.
	 *
	 * This is just a preliminary fix, intended to turn off TENA
	 * as soon as possible to avoid noise on the network. Once
	 * I²C is running we will make sure the interface is
	 * correctly initialized.
	 */
	immr->im_cpm.cp_pbpar &= ~PB_ENET_TENA;
	immr->im_cpm.cp_pbodr &= ~PB_ENET_TENA;
	immr->im_cpm.cp_pbdat &= ~PB_ENET_TENA;	/* set to 0 = disabled */
	immr->im_cpm.cp_pbdir |= PB_ENET_TENA;

	return (0);
}

/* ------------------------------------------------------------------------- */

/***********************************************************************
F* Function:     void reset_phy (void) P*A*Z*
 *
P* Parameters:   none
P*
P* Returnvalue:  none
 *
Z* Intention:    Reset the PHY.  In the lwmon case we do this by the
Z*               signaling the PIC I/O expander.
 *
D* Design:       wd@denx.de
C* Coding:       wd@denx.de
V* Verification: dzu@denx.de
 ***********************************************************************/
void reset_phy (void)
{
	uchar c;

#ifdef DEBUG
	printf ("### Switch on Ethernet for SCC2 ###\n");
#endif
	c = pic_read (0x61);
#ifdef DEBUG
	printf ("Old PIC read: reg_61 = 0x%02x\n", c);
#endif
	c |= 0x40;					/* disable COM3 */
	c &= ~0x80;					/* enable Ethernet */
	pic_write (0x61, c);
#ifdef DEBUG
	c = pic_read (0x61);
	printf ("New PIC read: reg_61 = 0x%02x\n", c);
#endif
	udelay (1000);
}


/*------------------------- Keyboard controller -----------------------*/
/* command codes */
#define	KEYBD_CMD_READ_KEYS	0x01
#define KEYBD_CMD_READ_VERSION	0x02
#define KEYBD_CMD_READ_STATUS	0x03
#define KEYBD_CMD_RESET_ERRORS	0x10

/* status codes */
#define KEYBD_STATUS_MASK	0x3F
#define	KEYBD_STATUS_H_RESET	0x20
#define KEYBD_STATUS_BROWNOUT	0x10
#define KEYBD_STATUS_WD_RESET	0x08
#define KEYBD_STATUS_OVERLOAD	0x04
#define KEYBD_STATUS_ILLEGAL_WR	0x02
#define KEYBD_STATUS_ILLEGAL_RD	0x01

/* Number of bytes returned from Keyboard Controller */
#define KEYBD_VERSIONLEN	2	/* version information */
#define	KEYBD_DATALEN		9	/* normal key scan data */

/* maximum number of "magic" key codes that can be assigned */

static uchar kbd_addr = CFG_I2C_KEYBD_ADDR;

static uchar *key_match (uchar *);

#define	KEYBD_SET_DEBUGMODE	'#'	/* Magic key to enable debug output */

/***********************************************************************
F* Function:     int board_postclk_init (void) P*A*Z*
 *
P* Parameters:   none
P*
P* Returnvalue:  int
P*                - 0 is always returned.
 *
Z* Intention:    This function is the board_postclk_init() method implementation
Z*               for the lwmon board.
 *
 ***********************************************************************/
int board_postclk_init (void)
{
	kbd_init();

#ifdef CONFIG_MODEM_SUPPORT
	if (key_pressed()) {
		disable_putc();	/* modem doesn't understand banner etc */
		gd->do_mdm_init = 1;
	}
#endif

	return (0);
}

struct serial_device * default_serial_console (void)
{
	return gd->do_mdm_init ? &serial_scc_device : &serial_smc_device;
}

static void kbd_init (void)
{
	uchar kbd_data[KEYBD_DATALEN];
	uchar tmp_data[KEYBD_DATALEN];
	uchar val, errcd;
	int i;

	i2c_init (CFG_I2C_SPEED, CFG_I2C_SLAVE);

	gd->kbd_status = 0;

	/* Forced by PIC. Delays <= 175us loose */
	udelay(1000);

	/* Read initial keyboard error code */
	val = KEYBD_CMD_READ_STATUS;
	i2c_write (kbd_addr, 0, 0, &val, 1);
	i2c_read (kbd_addr, 0, 0, &errcd, 1);
	/* clear unused bits */
	errcd &= KEYBD_STATUS_MASK;
	/* clear "irrelevant" bits. Recommended by Martin Rajek, LWN */
	errcd &= ~(KEYBD_STATUS_H_RESET|KEYBD_STATUS_BROWNOUT);
	if (errcd) {
		gd->kbd_status |= errcd << 8;
	}
	/* Reset error code and verify */
	val = KEYBD_CMD_RESET_ERRORS;
	i2c_write (kbd_addr, 0, 0, &val, 1);
	udelay(1000);	/* delay NEEDED by keyboard PIC !!! */

	val = KEYBD_CMD_READ_STATUS;
	i2c_write (kbd_addr, 0, 0, &val, 1);
	i2c_read (kbd_addr, 0, 0, &val, 1);

	val &= KEYBD_STATUS_MASK;	/* clear unused bits */
	if (val) {			/* permanent error, report it */
		gd->kbd_status |= val;
		return;
	}

	/*
	 * Read current keyboard state.
	 *
	 * After the error reset it may take some time before the
	 * keyboard PIC picks up a valid keyboard scan - the total
	 * scan time is approx. 1.6 ms (information by Martin Rajek,
	 * 28 Sep 2002). We read a couple of times for the keyboard
	 * to stabilize, using a big enough delay.
	 * 10 times should be enough. If the data is still changing,
	 * we use what we get :-(
	 */

	memset (tmp_data, 0xFF, KEYBD_DATALEN);	/* impossible value */
	for (i=0; i<10; ++i) {
		val = KEYBD_CMD_READ_KEYS;
		i2c_write (kbd_addr, 0, 0, &val, 1);
		i2c_read (kbd_addr, 0, 0, kbd_data, KEYBD_DATALEN);

		if (memcmp(kbd_data, tmp_data, KEYBD_DATALEN) == 0) {
			/* consistent state, done */
			break;
		}
		/* remeber last state, delay, and retry */
		memcpy (tmp_data, kbd_data, KEYBD_DATALEN);
		udelay (5000);
	}
}

/***********************************************************************
F* Function:     int misc_init_r (void) P*A*Z*
 *
P* Parameters:   none
P*
P* Returnvalue:  int
P*                - 0 is always returned, even in the case of a keyboard
P*                    error.
 *
Z* Intention:    This function is the misc_init_r() method implementation
Z*               for the lwmon board.
Z*               The keyboard controller is initialized and the result
Z*               of a read copied to the environment variable "keybd".
Z*               If KEYBD_SET_DEBUGMODE is defined, a check is made for
Z*               this key, and if found display to the LCD will be enabled.
Z*               The keys in "keybd" are checked against the magic
Z*               keycommands defined in the environment.
Z*               See also key_match().
 *
D* Design:       wd@denx.de
C* Coding:       wd@denx.de
V* Verification: dzu@denx.de
 ***********************************************************************/
int misc_init_r (void)
{
	uchar kbd_data[KEYBD_DATALEN];
	char keybd_env[2 * KEYBD_DATALEN + 1];
	uchar kbd_init_status = gd->kbd_status >> 8;
	uchar kbd_status = gd->kbd_status;
	uchar val;
	char *str;
	int i;

	if (kbd_init_status) {
		printf ("KEYBD: Error %02X\n", kbd_init_status);
	}
	if (kbd_status) {		/* permanent error, report it */
		printf ("*** Keyboard error code %02X ***\n", kbd_status);
		sprintf (keybd_env, "%02X", kbd_status);
		setenv ("keybd", keybd_env);
		return 0;
	}

	/*
	 * Now we know that we have a working  keyboard,  so  disable
	 * all output to the LCD except when a key press is detected.
	 */

	if ((console_assign (stdout, "serial") < 0) ||
		(console_assign (stderr, "serial") < 0)) {
		printf ("Can't assign serial port as output device\n");
	}

	/* Read Version */
	val = KEYBD_CMD_READ_VERSION;
	i2c_write (kbd_addr, 0, 0, &val, 1);
	i2c_read (kbd_addr, 0, 0, kbd_data, KEYBD_VERSIONLEN);
	printf ("KEYBD: Version %d.%d\n", kbd_data[0], kbd_data[1]);

	/* Read current keyboard state */
	val = KEYBD_CMD_READ_KEYS;
	i2c_write (kbd_addr, 0, 0, &val, 1);
	i2c_read (kbd_addr, 0, 0, kbd_data, KEYBD_DATALEN);

	for (i = 0; i < KEYBD_DATALEN; ++i) {
		sprintf (keybd_env + i + i, "%02X", kbd_data[i]);
	}
	setenv ("keybd", keybd_env);

	str = strdup ((char *)key_match (kbd_data));	/* decode keys */
#ifdef KEYBD_SET_DEBUGMODE
	if (kbd_data[0] == KEYBD_SET_DEBUGMODE) {	/* set debug mode */
		if ((console_assign (stdout, "lcd") < 0) ||
			(console_assign (stderr, "lcd") < 0)) {
			printf ("Can't assign LCD display as output device\n");
		}
	}
#endif /* KEYBD_SET_DEBUGMODE */
#ifdef CONFIG_PREBOOT	/* automatically configure "preboot" command on key match */
	setenv ("preboot", str);	/* set or delete definition */
#endif /* CONFIG_PREBOOT */
	if (str != NULL) {
		free (str);
	}
	return (0);
}

#ifdef CONFIG_PREBOOT

static uchar kbd_magic_prefix[] = "key_magic";
static uchar kbd_command_prefix[] = "key_cmd";

static int compare_magic (uchar *kbd_data, uchar *str)
{
	uchar compare[KEYBD_DATALEN-1];
	char *nxt;
	int i;

	/* Don't include modifier byte */
	memcpy (compare, kbd_data+1, KEYBD_DATALEN-1);

	for (; str != NULL; str = (*nxt) ? (uchar *)(nxt+1) : (uchar *)nxt) {
		uchar c;
		int k;

		c = (uchar) simple_strtoul ((char *)str, (char **) (&nxt), 16);

		if (str == (uchar *)nxt) {	/* invalid character */
			break;
		}

		/*
		 * Check if this key matches the input.
		 * Set matches to zero, so they match only once
		 * and we can find duplicates or extra keys
		 */
		for (k = 0; k < sizeof(compare); ++k) {
			if (compare[k] == '\0')	/* only non-zero entries */
				continue;
			if (c == compare[k]) {	/* found matching key */
				compare[k] = '\0';
				break;
			}
		}
		if (k == sizeof(compare)) {
			return -1;		/* unmatched key */
		}
	}

	/*
	 * A full match leaves no keys in the `compare' array,
	 */
	for (i = 0; i < sizeof(compare); ++i) {
		if (compare[i])
		{
			return -1;
		}
	}

	return 0;
}

/***********************************************************************
F* Function:     static uchar *key_match (uchar *kbd_data) P*A*Z*
 *
P* Parameters:   uchar *kbd_data
P*                - The keys to match against our magic definitions
P*
P* Returnvalue:  uchar *
P*                - != NULL: Pointer to the corresponding command(s)
P*                     NULL: No magic is about to happen
 *
Z* Intention:    Check if pressed key(s) match magic sequence,
Z*               and return the command string associated with that key(s).
Z*
Z*               If no key press was decoded, NULL is returned.
Z*
Z*               Note: the first character of the argument will be
Z*                     overwritten with the "magic charcter code" of the
Z*                     decoded key(s), or '\0'.
Z*
Z*               Note: the string points to static environment data
Z*                     and must be saved before you call any function that
Z*                     modifies the environment.
 *
D* Design:       wd@denx.de
C* Coding:       wd@denx.de
V* Verification: dzu@denx.de
 ***********************************************************************/
static uchar *key_match (uchar *kbd_data)
{
	char magic[sizeof (kbd_magic_prefix) + 1];
	uchar *suffix;
	char *kbd_magic_keys;

	/*
	 * The following string defines the characters that can pe appended
	 * to "key_magic" to form the names of environment variables that
	 * hold "magic" key codes, i. e. such key codes that can cause
	 * pre-boot actions. If the string is empty (""), then only
	 * "key_magic" is checked (old behaviour); the string "125" causes
	 * checks for "key_magic1", "key_magic2" and "key_magic5", etc.
	 */
	if ((kbd_magic_keys = getenv ("magic_keys")) == NULL)
		kbd_magic_keys = "";

	/* loop over all magic keys;
	 * use '\0' suffix in case of empty string
	 */
	for (suffix=(uchar *)kbd_magic_keys; *suffix || suffix==(uchar *)kbd_magic_keys; ++suffix) {
		sprintf (magic, "%s%c", kbd_magic_prefix, *suffix);
#if 0
		printf ("### Check magic \"%s\"\n", magic);
#endif
		if (compare_magic(kbd_data, (uchar *)getenv(magic)) == 0) {
			char cmd_name[sizeof (kbd_command_prefix) + 1];
			char *cmd;

			sprintf (cmd_name, "%s%c", kbd_command_prefix, *suffix);

			cmd = getenv (cmd_name);
#if 0
			printf ("### Set PREBOOT to $(%s): \"%s\"\n",
					cmd_name, cmd ? cmd : "<<NULL>>");
#endif
			*kbd_data = *suffix;
			return ((uchar *)cmd);
		}
	}
#if 0
	printf ("### Delete PREBOOT\n");
#endif
	*kbd_data = '\0';
	return (NULL);
}
#endif /* CONFIG_PREBOOT */

/*---------------Board Special Commands: PIC read/write ---------------*/

#if (CONFIG_COMMANDS & CFG_CMD_BSP) || defined(CONFIG_CMD_BSP)
/***********************************************************************
F* Function:     int do_pic (cmd_tbl_t *cmdtp, int flag,
F*                           int argc, char *argv[]) P*A*Z*
 *
P* Parameters:   cmd_tbl_t *cmdtp
P*                - Pointer to our command table entry
P*               int flag
P*                - If the CMD_FLAG_REPEAT bit is set, then this call is
P*                  a repetition
P*               int argc
P*                - Argument count
P*               char *argv[]
P*                - Array of the actual arguments
P*
P* Returnvalue:  int
P*                - 0  The command was handled successfully
P*                  1  An error occurred
 *
Z* Intention:    Implement the "pic [read|write]" commands.
Z*               The read subcommand takes one argument, the register,
Z*               whereas the write command takes two, the register and
Z*               the new value.
 *
D* Design:       wd@denx.de
C* Coding:       wd@denx.de
V* Verification: dzu@denx.de
 ***********************************************************************/
int do_pic (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	uchar reg, val;

	switch (argc) {
	case 3:					/* PIC read reg */
		if (strcmp (argv[1], "read") != 0)
			break;

		reg = simple_strtoul (argv[2], NULL, 16);

		printf ("PIC read: reg %02x: %02x\n\n", reg, pic_read (reg));

		return 0;
	case 4:					/* PIC write reg val */
		if (strcmp (argv[1], "write") != 0)
			break;

		reg = simple_strtoul (argv[2], NULL, 16);
		val = simple_strtoul (argv[3], NULL, 16);

		printf ("PIC write: reg %02x val 0x%02x: %02x => ",
				reg, val, pic_read (reg));
		pic_write (reg, val);
		printf ("%02x\n\n", pic_read (reg));
		return 0;
	default:
		break;
	}
	printf ("Usage:\n%s\n", cmdtp->usage);
	return 1;
}
U_BOOT_CMD(
	pic,	4,	1,	do_pic,
	"pic     - read and write PIC registers\n",
	"read  reg      - read PIC register `reg'\n"
	"pic write reg val  - write value `val' to PIC register `reg'\n"
);

/***********************************************************************
F* Function:     int do_kbd (cmd_tbl_t *cmdtp, int flag,
F*                           int argc, char *argv[]) P*A*Z*
 *
P* Parameters:   cmd_tbl_t *cmdtp
P*                - Pointer to our command table entry
P*               int flag
P*                - If the CMD_FLAG_REPEAT bit is set, then this call is
P*                  a repetition
P*               int argc
P*                - Argument count
P*               char *argv[]
P*                - Array of the actual arguments
P*
P* Returnvalue:  int
P*                - 0 is always returned.
 *
Z* Intention:    Implement the "kbd" command.
Z*               The keyboard status is read.  The result is printed on
Z*               the console and written into the "keybd" environment
Z*               variable.
 *
D* Design:       wd@denx.de
C* Coding:       wd@denx.de
V* Verification: dzu@denx.de
 ***********************************************************************/
int do_kbd (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	uchar kbd_data[KEYBD_DATALEN];
	char keybd_env[2 * KEYBD_DATALEN + 1];
	uchar val;
	int i;

#if 0 /* Done in kbd_init */
	i2c_init (CFG_I2C_SPEED, CFG_I2C_SLAVE);
#endif

	/* Read keys */
	val = KEYBD_CMD_READ_KEYS;
	i2c_write (kbd_addr, 0, 0, &val, 1);
	i2c_read (kbd_addr, 0, 0, kbd_data, KEYBD_DATALEN);

	puts ("Keys:");
	for (i = 0; i < KEYBD_DATALEN; ++i) {
		sprintf (keybd_env + i + i, "%02X", kbd_data[i]);
		printf (" %02x", kbd_data[i]);
	}
	putc ('\n');
	setenv ("keybd", keybd_env);
	return 0;
}

U_BOOT_CMD(
	kbd,	1,	1,	do_kbd,
	"kbd     - read keyboard status\n",
	NULL
);

/* Read and set LSB switch */
#define CFG_PC_TXD1_ENA		0x0008		/* PC.12 */

/***********************************************************************
F* Function:     int do_lsb (cmd_tbl_t *cmdtp, int flag,
F*                           int argc, char *argv[]) P*A*Z*
 *
P* Parameters:   cmd_tbl_t *cmdtp
P*                - Pointer to our command table entry
P*               int flag
P*                - If the CMD_FLAG_REPEAT bit is set, then this call is
P*                  a repetition
P*               int argc
P*                - Argument count
P*               char *argv[]
P*                - Array of the actual arguments
P*
P* Returnvalue:  int
P*                - 0  The command was handled successfully
P*                  1  An error occurred
 *
Z* Intention:    Implement the "lsb [on|off]" commands.
Z*               The lsb is switched according to the first parameter by
Z*               by signaling the PIC I/O expander.
Z*               Called with no arguments, the current setting is
Z*               printed.
 *
D* Design:       wd@denx.de
C* Coding:       wd@denx.de
V* Verification: dzu@denx.de
 ***********************************************************************/
int do_lsb (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	uchar val;
	immap_t *immr = (immap_t *) CFG_IMMR;

	switch (argc) {
	case 1:					/* lsb - print setting */
		val = pic_read (0x60);
		printf ("LSB is o%s\n", (val & 0x20) ? "n" : "ff");
		return 0;
	case 2:					/* lsb on or lsb off - set switch */
		val = pic_read (0x60);

		if (strcmp (argv[1], "on") == 0) {
			val |= 0x20;
			immr->im_ioport.iop_pcpar &= ~(CFG_PC_TXD1_ENA);
			immr->im_ioport.iop_pcdat |= CFG_PC_TXD1_ENA;
			immr->im_ioport.iop_pcdir |= CFG_PC_TXD1_ENA;
		} else if (strcmp (argv[1], "off") == 0) {
			val &= ~0x20;
			immr->im_ioport.iop_pcpar &= ~(CFG_PC_TXD1_ENA);
			immr->im_ioport.iop_pcdat &= ~(CFG_PC_TXD1_ENA);
			immr->im_ioport.iop_pcdir |= CFG_PC_TXD1_ENA;
		} else {
			break;
		}
		pic_write (0x60, val);
		return 0;
	default:
		break;
	}
	printf ("Usage:\n%s\n", cmdtp->usage);
	return 1;
}

U_BOOT_CMD(
	lsb,	2,	1,	do_lsb,
	"lsb     - check and set LSB switch\n",
	"on  - switch LSB on\n"
	"lsb off - switch LSB off\n"
	"lsb     - print current setting\n"
);

#endif /* CFG_CMD_BSP */

/*----------------------------- Utilities -----------------------------*/
/***********************************************************************
F* Function:     uchar pic_read (uchar reg) P*A*Z*
 *
P* Parameters:   uchar reg
P*                - Register to read
P*
P* Returnvalue:  uchar
P*                - Value read from register
 *
Z* Intention:    Read a register from the PIC I/O expander.
 *
D* Design:       wd@denx.de
C* Coding:       wd@denx.de
V* Verification: dzu@denx.de
 ***********************************************************************/
uchar pic_read (uchar reg)
{
	return (i2c_reg_read (CFG_I2C_PICIO_ADDR, reg));
}

/***********************************************************************
F* Function:     void pic_write (uchar reg, uchar val) P*A*Z*
 *
P* Parameters:   uchar reg
P*                - Register to read
P*               uchar val
P*                - Value to write
P*
P* Returnvalue:  none
 *
Z* Intention:    Write to a register on the PIC I/O expander.
 *
D* Design:       wd@denx.de
C* Coding:       wd@denx.de
V* Verification: dzu@denx.de
 ***********************************************************************/
void pic_write (uchar reg, uchar val)
{
	i2c_reg_write (CFG_I2C_PICIO_ADDR, reg, val);
}

/*---------------------- Board Control Functions ----------------------*/
/***********************************************************************
F* Function:     void board_poweroff (void) P*A*Z*
 *
P* Parameters:   none
P*
P* Returnvalue:  none
 *
Z* Intention:    Turn off the battery power and loop endless, so this
Z*               should better be the last function you call...
 *
D* Design:       wd@denx.de
C* Coding:       wd@denx.de
V* Verification: dzu@denx.de
 ***********************************************************************/
void board_poweroff (void)
{
    /* Turn battery off */
    ((volatile immap_t *)CFG_IMMR)->im_ioport.iop_pcdat &= ~(1 << (31 - 13));

    while (1);
}

#ifdef CONFIG_MODEM_SUPPORT
static int key_pressed(void)
{
	uchar kbd_data[KEYBD_DATALEN];
	uchar val;

	/* Read keys */
	val = KEYBD_CMD_READ_KEYS;
	i2c_write (kbd_addr, 0, 0, &val, 1);
	i2c_read (kbd_addr, 0, 0, kbd_data, KEYBD_DATALEN);

	return (compare_magic(kbd_data, (uchar *)CONFIG_MODEM_KEY_MAGIC) == 0);
}
#endif	/* CONFIG_MODEM_SUPPORT */

#ifdef CONFIG_POST
/*
 * Returns 1 if keys pressed to start the power-on long-running tests
 * Called from board_init_f().
 */
int post_hotkeys_pressed(void)
{
	uchar kbd_data[KEYBD_DATALEN];
	uchar val;

	/* Read keys */
	val = KEYBD_CMD_READ_KEYS;
	i2c_write (kbd_addr, 0, 0, &val, 1);
	i2c_read (kbd_addr, 0, 0, kbd_data, KEYBD_DATALEN);

	return (compare_magic(kbd_data, (uchar *)CONFIG_POST_KEY_MAGIC) == 0);
}
#endif

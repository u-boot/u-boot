/*
 * (C) Copyright 2000, 2001
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
 *
 */

/*
 * ehnus: change pll frequency.
 * Wed Sep  5 11:45:17 CST 2007
 * hsun@udtech.com.cn
 */


#include <common.h>
#include <config.h>
#include <command.h>
#include <i2c.h>

#ifdef CONFIG_CMD_EEPROM

#define EEPROM_CONF_OFFSET		0
#define EEPROM_TEST_OFFSET		16
#define EEPROM_SDSTP_PARAM		16

#define PLL_NAME_MAX			12
#define BUF_STEP			8

/* eeprom_wirtes 8Byte per op. */
#define EEPROM_ALTER_FREQ(freq)						\
	do {								\
		int __i;						\
		for (__i = 0; __i < 2; __i++)				\
			eeprom_write (CFG_I2C_EEPROM_ADDR,		\
				      EEPROM_CONF_OFFSET + __i*BUF_STEP, \
				      pll_select[freq],			\
				      BUF_STEP + __i*BUF_STEP);		\
	} while (0)

#define PDEBUG
#ifdef	PDEBUG
#define PLL_DEBUG	pll_debug(EEPROM_CONF_OFFSET)
#else
#define PLL_DEBUG
#endif

typedef enum {
	PLL_ebc20,
	PLL_333,
	PLL_4001,
	PLL_4002,
	PLL_533,
	PLL_600,
	PLL_666,	/* For now, kilauea can't support */
	RCONF,
	WTEST,
	PLL_TOTAL
} pll_freq_t;

static const char
pll_name[][PLL_NAME_MAX] = {
	"PLL_ebc20",
	"PLL_333",
	"PLL_400@1",
	"PLL_400@2",
	"PLL_533",
	"PLL_600",
	"PLL_666",
	"RCONF",
	"WTEST",
	""
};

/*
 * ehnus:
 */
static uchar
pll_select[][EEPROM_SDSTP_PARAM] = {
	/* 0: CPU 333MHz EBC 20MHz, for test only */
	{
		0x8c, 0x12, 0xec, 0x12, 0x88, 0x00, 0x0a, 0x00,
		0x40, 0x08, 0x23, 0x50, 0x00, 0x05, 0x00, 0x00
	},

	/* 0: 333 */
	{
		0x8c, 0x12, 0xec, 0x12, 0x98, 0x00, 0x0a, 0x00,
		0x40, 0x08, 0x23, 0x50, 0x00, 0x05, 0x00, 0x00
	},

	/* 1: 400_266 */
	{
		0x8e, 0x0e, 0xe8, 0x13, 0x98, 0x00, 0x0a, 0x00,
		0x40, 0x08, 0x23, 0x50, 0x00, 0x05, 0x00, 0x00
	},

	/* 2: 400 */
	{
		0x8e, 0x0e, 0xe8, 0x12, 0x98, 0x00, 0x0a, 0x00,
		0x40, 0x08, 0x23, 0x50, 0x00, 0x05, 0x00, 0x00
	},

	/* 3: 533 */
	{
		0x8e, 0x43, 0x60, 0x13, 0x98, 0x00, 0x0a, 0x00,
		0x40, 0x08, 0x23, 0x50, 0x00, 0x05, 0x00, 0x00
	},

	/* 4: 600 */
	{
		0x8d, 0x02, 0x34, 0x13, 0x98, 0x00, 0x0a, 0x00,
		0x40, 0x08, 0x23, 0x50, 0x00, 0x05, 0x00, 0x00
	},

	/* 5: 666 */
	{
		0x8d, 0x03, 0x78, 0x13, 0x98, 0x00, 0x0a, 0x00,
		0x40, 0x08, 0x23, 0x50, 0x00, 0x05, 0x00, 0x00
	},

	{}
};

static uchar
testbuf[EEPROM_SDSTP_PARAM] = {
	0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
};

static void
pll_debug(int off)
{
	int i;
	uchar buffer[EEPROM_SDSTP_PARAM];

	memset(buffer, 0, sizeof(buffer));
	eeprom_read(CFG_I2C_EEPROM_ADDR, off,
		    buffer, EEPROM_SDSTP_PARAM);

	printf("Debug: SDSTP[0-3] at offset \"0x%02x\" lists as follows: \n", off);
	for (i = 0; i < EEPROM_SDSTP_PARAM; i++)
		printf("%02x ", buffer[i]);
	printf("\n");
}

static void
test_write(void)
{
	printf("Debug: test eeprom_write ... ");

	/*
	 * Write twice, 8 bytes per write
	 */
	eeprom_write (CFG_I2C_EEPROM_ADDR, EEPROM_TEST_OFFSET,
		      testbuf, 8);
	eeprom_write (CFG_I2C_EEPROM_ADDR, EEPROM_TEST_OFFSET+8,
		      testbuf, 16);
	printf("done\n");

	pll_debug(EEPROM_TEST_OFFSET);
}

int
do_pll_alter (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char c = '\0';
	pll_freq_t pll_freq;
	if (argc < 2) {
		printf("Usage: \n%s\n", cmdtp->usage);
		goto ret;
	}

	for (pll_freq = PLL_ebc20; pll_freq < PLL_TOTAL; pll_freq++)
		if (!strcmp(pll_name[pll_freq], argv[1]))
			break;

	switch (pll_freq) {
	case PLL_ebc20:
	case PLL_333:
	case PLL_4001:
	case PLL_4002:
	case PLL_533:
	case PLL_600:
		EEPROM_ALTER_FREQ(pll_freq);
		break;

	case PLL_666:		/* not support */
		printf("Choose this option will result in a boot failure."
		       "\nContinue? (Y/N): ");

		c = getc(); putc('\n');

		if ((c == 'y') || (c == 'Y')) {
			EEPROM_ALTER_FREQ(pll_freq);
			break;
		}
		goto ret;

	case RCONF:
		pll_debug(EEPROM_CONF_OFFSET);
		goto ret;
	case WTEST:
		printf("DEBUG: write test\n");
		test_write();
		goto ret;

	default:
		printf("Invalid options"
		       "\n\nUsage: \n%s\n", cmdtp->usage);
		goto ret;
	}

	printf("PLL set to %s, "
	       "reset the board to take effect\n", pll_name[pll_freq]);

	PLL_DEBUG;
ret:
	return 0;
}

U_BOOT_CMD(
	pllalter, CFG_MAXARGS, 1,        do_pll_alter,
	"pllalter- change pll frequence \n",
	"pllalter <selection>      - change pll frequence \n\n\
	** New freq take effect after reset. ** \n\
	----------------------------------------------\n\
	PLL_ebc20: Board: AMCC 405EX(r) Evaluation Board\n\
	\t	Same as PLL_333	\n\
	\t	except          \n\
	\t	EBC: 20 MHz     \n\
	----------------------------------------------\n\
	PLL_333: Board: AMCC 405EX(r) Evaluation Board\n\
	\t	VCO: 666 MHz  \n\
	\t	CPU: 333 MHz  \n\
	\t	PLB: 166 MHz  \n\
	\t	OPB: 83 MHz   \n\
	\t	DDR: 83 MHz   \n\
	------------------------------------------------\n\
	PLL_400@1: Board: AMCC 405EX(r) Evaluation Board\n\
	\t	VCO: 800 MHz  \n\
	\t	CPU: 400 MHz  \n\
	\t	PLB: 133 MHz  \n\
	\t	OPB: 66  MHz  \n\
	\t	DDR: 133 MHz  \n\
	------------------------------------------------\n\
	PLL_400@2: Board: AMCC 405EX(r) Evaluation Board\n\
	\t	VCO: 800 MHz  \n\
	\t	CPU: 400 MHz  \n\
	\t	PLB: 200 MHz  \n\
	\t	OPB: 100 MHz  \n\
	\t	DDR: 200 MHz  \n\
	----------------------------------------------\n\
	PLL_533: Board: AMCC 405EX(r) Evaluation Board\n\
	\t	VCO: 1066 MHz  \n\
	\t	CPU: 533  MHz  \n\
	\t	PLB: 177  MHz  \n\
	\t	OPB: 88   MHz  \n\
	\t	DDR: 177  MHz  \n\
	----------------------------------------------\n\
	PLL_600: Board: AMCC 405EX(r) Evaluation Board\n\
	\t	VCO: 1200 MHz  \n\
	\t	CPU: 600  MHz  \n\
	\t	PLB: 200  MHz  \n\
	\t	OPB: 100  MHz  \n\
	\t	DDR: 200  MHz  \n\
	----------------------------------------------\n\
	PLL_666: Board: AMCC 405EX(r) Evaluation Board\n\
	\t	VCO: 1333 MHz  \n\
	\t	CPU: 666  MHz  \n\
	\t	PLB: 166  MHz  \n\
	\t	OPB: 83   MHz  \n\
	\t	DDR: 166  MHz  \n\
	-----------------------------------------------\n\
	RCONF: Read current eeprom configuration.      \n\
	-----------------------------------------------\n\
	WTEST: Test EEPROM write with predefined values\n\
	-----------------------------------------------\n"
	);

#endif	/* CONFIG_CMD_EEPROM */

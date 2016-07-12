/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004
 * Mark Jonas, Freescale Semiconductor, mark.jonas@motorola.com.
 *
 * (C) Copyright 2010
 * Sergei Poselenov, Emcraft Systems, sposelenov@emcraft.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mpc5xxx.h>
#include <pci.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <libfdt.h>
#include <netdev.h>
#include <led-display.h>
#include <linux/err.h>

#include "mt46v32m16.h"

#ifndef CONFIG_SYS_RAMBOOT
static void sdram_start (int hi_addr)
{
	long hi_addr_bit = hi_addr ? 0x01000000 : 0;
	long control = SDRAM_CONTROL | hi_addr_bit;

	/* unlock mode register */
	out_be32((void *)MPC5XXX_SDRAM_CTRL, control | 0x80000000);
	__asm__ volatile ("sync");

	/* precharge all banks */
	out_be32((void *)MPC5XXX_SDRAM_CTRL, control | 0x80000002);
	__asm__ volatile ("sync");

#if SDRAM_DDR
	/* set mode register: extended mode */
	out_be32((void *)MPC5XXX_SDRAM_MODE, SDRAM_EMODE);
	__asm__ volatile ("sync");

	/* set mode register: reset DLL */
	out_be32((void *)MPC5XXX_SDRAM_MODE, SDRAM_MODE | 0x04000000);
	__asm__ volatile ("sync");
#endif

	/* precharge all banks */
	out_be32((void *)MPC5XXX_SDRAM_CTRL, control | 0x80000002);
	__asm__ volatile ("sync");

	/* auto refresh */
	out_be32((void *)MPC5XXX_SDRAM_CTRL, control | 0x80000004);
	__asm__ volatile ("sync");

	/* set mode register */
	out_be32((void *)MPC5XXX_SDRAM_MODE, SDRAM_MODE);
	__asm__ volatile ("sync");

	/* normal operation */
	out_be32((void *)MPC5XXX_SDRAM_CTRL, control);
	__asm__ volatile ("sync");
}
#endif

/*
 * ATTENTION: Although partially referenced initdram does NOT make real use
 *            use of CONFIG_SYS_SDRAM_BASE. The code does not work if CONFIG_SYS_SDRAM_BASE
 *            is something else than 0x00000000.
 */

phys_size_t initdram (int board_type)
{
	ulong dramsize = 0;
	uint svr, pvr;

#ifndef CONFIG_SYS_RAMBOOT
	ulong test1, test2;

	/* setup SDRAM chip selects */
	out_be32((void *)MPC5XXX_SDRAM_CS0CFG, 0x0000001e); /* 2GB at 0x0 */
	out_be32((void *)MPC5XXX_SDRAM_CS1CFG, 0x80000000); /* disabled */
	__asm__ volatile ("sync");

	/* setup config registers */
	out_be32((void *)MPC5XXX_SDRAM_CONFIG1, SDRAM_CONFIG1);
	out_be32((void *)MPC5XXX_SDRAM_CONFIG2, SDRAM_CONFIG2);
	__asm__ volatile ("sync");

#if SDRAM_DDR
	/* set tap delay */
	out_be32((void *)MPC5XXX_CDM_PORCFG, SDRAM_TAPDELAY);
	__asm__ volatile ("sync");
#endif

	/* find RAM size using SDRAM CS0 only */
	sdram_start(0);
	test1 = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE, 0x80000000);
	sdram_start(1);
	test2 = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE, 0x80000000);
	if (test1 > test2) {
		sdram_start(0);
		dramsize = test1;
	} else {
		dramsize = test2;
	}

	/* memory smaller than 1MB is impossible */
	if (dramsize < (1 << 20)) {
		dramsize = 0;
	}

	/* set SDRAM CS0 size according to the amount of RAM found */
	if (dramsize > 0) {
		out_be32((void *)MPC5XXX_SDRAM_CS0CFG,
				 0x13 + __builtin_ffs(dramsize >> 20) - 1);
	} else {
		out_be32((void *)MPC5XXX_SDRAM_CS0CFG, 0); /* disabled */
	}

#else /* CONFIG_SYS_RAMBOOT */

	/* retrieve size of memory connected to SDRAM CS0 */
	dramsize = in_be32((void *)MPC5XXX_SDRAM_CS0CFG) & 0xFF;
	if (dramsize >= 0x13) {
		dramsize = (1 << (dramsize - 0x13)) << 20;
	} else {
		dramsize = 0;
	}

#endif /* CONFIG_SYS_RAMBOOT */

	/*
	 * On MPC5200B we need to set the special configuration delay in the
	 * DDR controller. Please refer to Freescale's AN3221 "MPC5200B SDRAM
	 * Initialization and Configuration", 3.3.1 SDelay--MBAR + 0x0190:
	 *
	 * "The SDelay should be written to a value of 0x00000004. It is
	 * required to account for changes caused by normal wafer processing
	 * parameters."
	 */
	svr = get_svr();
	pvr = get_pvr();
	if ((SVR_MJREV(svr) >= 2) &&
	    (PVR_MAJ(pvr) == 1) && (PVR_MIN(pvr) == 4)) {

		out_be32((void *)MPC5XXX_SDRAM_SDELAY, 0x04);
		__asm__ volatile ("sync");
	}

	return dramsize;
}

int checkboard (void)
{
	puts ("Board: A4M072\n");
	return 0;
}

#ifdef	CONFIG_PCI
static struct pci_controller hose;

extern void pci_mpc5xxx_init(struct pci_controller *);

void pci_init_board(void)
{
	pci_mpc5xxx_init(&hose);
}
#endif

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);

	return 0;
}
#endif /* CONFIG_OF_BOARD_SETUP */

int board_eth_init(bd_t *bis)
{
	int rv, num_if = 0;

	/* Initialize TSECs first */
	if ((rv = cpu_eth_init(bis)) >= 0)
		num_if += rv;
	else
		printf("ERROR: failed to initialize FEC.\n");

	if ((rv = pci_eth_init(bis)) >= 0)
		num_if += rv;
	else
		printf("ERROR: failed to initialize PCI Ethernet.\n");

	return num_if;
}
/*
 * Miscellaneous late-boot configurations
 *
 * Initialize EEPROM write-protect GPIO pin.
 */
int misc_init_r(void)
{
#if defined(CONFIG_SYS_EEPROM_WREN)
	/* Enable GPIO pin */
	setbits_be32((void *)MPC5XXX_WU_GPIO_ENABLE, CONFIG_SYS_EEPROM_WP);
	/* Set direction, output */
	setbits_be32((void *)MPC5XXX_WU_GPIO_DIR, CONFIG_SYS_EEPROM_WP);
	/* De-assert write enable */
	setbits_be32((void *)MPC5XXX_WU_GPIO_DATA_O, CONFIG_SYS_EEPROM_WP);
#endif
	return 0;
}
#if defined(CONFIG_SYS_EEPROM_WREN)
/* Input: <dev_addr>  I2C address of EEPROM device to enable.
 *         <state>     -1: deliver current state
 *	               0: disable write
 *		       1: enable write
 *  Returns:           -1: wrong device address
 *                      0: dis-/en- able done
 *		     0/1: current state if <state> was -1.
 */
int eeprom_write_enable (unsigned dev_addr, int state)
{
	if (CONFIG_SYS_I2C_EEPROM_ADDR != dev_addr) {
		return -1;
	} else {
		switch (state) {
		case 1:
			/* Enable write access */
			clrbits_be32((void *)MPC5XXX_WU_GPIO_DATA_O, CONFIG_SYS_EEPROM_WP);
			state = 0;
			break;
		case 0:
			/* Disable write access */
			setbits_be32((void *)MPC5XXX_WU_GPIO_DATA_O, CONFIG_SYS_EEPROM_WP);
			state = 0;
			break;
		default:
			/* Read current status back. */
			state = (0 == (in_be32((void *)MPC5XXX_WU_GPIO_DATA_O) &
						   CONFIG_SYS_EEPROM_WP));
			break;
		}
	}
	return state;
}
#endif

#ifdef CONFIG_CMD_DISPLAY
#define DISPLAY_BUF_SIZE	2
static u8 display_buf[DISPLAY_BUF_SIZE];
static u8 display_putc_pos;
static u8 display_out_pos;

void display_set(int cmd) {

	if (cmd & DISPLAY_CLEAR) {
		display_buf[0] = display_buf[1] = 0;
	}

	if (cmd & DISPLAY_HOME) {
		display_putc_pos = 0;
	}
}

#define SEG_A    (1<<0)
#define SEG_B    (1<<1)
#define SEG_C    (1<<2)
#define SEG_D    (1<<3)
#define SEG_E    (1<<4)
#define SEG_F    (1<<5)
#define SEG_G    (1<<6)
#define SEG_P    (1<<7)
#define SEG__    0

/*
 * +- A -+
 * |     |
 * F     B
 * |     |
 * +- G -+
 * |     |
 * E     C
 * |     |
 * +- D -+  P
 *
 * 0..9		index 0..9
 * A..Z		index 10..35
 * -		index 36
 * _		index 37
 * .		index 38
 */

#define SYMBOL_DASH		(36)
#define SYMBOL_UNDERLINE	(37)
#define SYMBOL_DOT		(38)

static u8 display_char2seg7_tbl[]=
{
	SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,		/* 0 */
	SEG_B | SEG_C,						/* 1 */
	SEG_A | SEG_B | SEG_D | SEG_E | SEG_G,			/* 2 */
	SEG_A | SEG_B | SEG_C | SEG_D | SEG_G,			/* 3 */
	SEG_B | SEG_C | SEG_F | SEG_G,				/* 4 */
	SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,			/* 5 */
	SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,		/* 6 */
	SEG_A | SEG_B | SEG_C,					/* 7 */
	SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,	/* 8 */
	SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G,		/* 9 */
	SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,		/* A */
	SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,			/* b */
	SEG_A | SEG_D | SEG_E | SEG_F,				/* C */
	SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,			/* d */
	SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,			/* E */
	SEG_A | SEG_E | SEG_F | SEG_G,				/* F */
	0,					/* g - not displayed */
	SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,			/* H */
	SEG_B | SEG_C,						/* I */
	0,					/* J - not displayed */
	0,					/* K - not displayed */
	SEG_D | SEG_E | SEG_F,					/* L */
	0,					/* m - not displayed */
	0,					/* n - not displayed */
	SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,		/* O */
	SEG_A | SEG_B | SEG_E | SEG_F | SEG_G,			/* P */
	0,					/* q - not displayed */
	0,					/* r - not displayed */
	SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,			/* S */
	SEG_D | SEG_E | SEG_F | SEG_G,				/* t */
	SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,			/* U */
	0,					/* V - not displayed */
	0,					/* w - not displayed */
	0,					/* X - not displayed */
	SEG_B | SEG_C | SEG_D | SEG_F | SEG_G,			/* Y */
	0,					/* Z - not displayed */
	SEG_G,							/* - */
	SEG_D,							/* _ */
	SEG_P							/* . */
};

/* Convert char to the LED segments representation */
static u8 display_char2seg7(char c)
{
	u8 val = 0;

	if (c >= '0' && c <= '9')
		c -= '0';
	else if (c >= 'a' && c <= 'z')
		c -= 'a' - 10;
	else if (c >= 'A' && c <= 'Z')
		c -= 'A' - 10;
	else if (c == '-')
		c = SYMBOL_DASH;
	else if (c == '_')
		c = SYMBOL_UNDERLINE;
	else if (c == '.')
		c = SYMBOL_DOT;
	else
		c = ' ';	/* display unsupported symbols as space */

	if (c != ' ')
		val = display_char2seg7_tbl[(int)c];

	return val;
}

int display_putc(char c)
{
	if (display_putc_pos >= DISPLAY_BUF_SIZE)
		return -1;

	display_buf[display_putc_pos++] = display_char2seg7(c);
	/* one-symbol message should be steady */
	if (display_putc_pos == 1)
		display_buf[display_putc_pos] = display_char2seg7(c);

	return c;
}

/*
 * Flush current symbol to the LED display hardware
 */
static inline void display_flush(void)
{
	u32 val = display_buf[display_out_pos];

	val |= (val << 8) | (val << 16) | (val << 24);
	out_be32((void *)CONFIG_SYS_DISP_CHR_RAM, val);
}

/*
 * Output contents of the software display buffer to the LED display every 0.5s
 */
void board_show_activity(ulong timestamp)
{
	static ulong last;
	static u8 once;

	if (!once || (timestamp - last >= (CONFIG_SYS_HZ / 2))) {
		display_flush();
		display_out_pos ^= 1;
		last = timestamp;
		once = 1;
	}
}

/*
 * Empty fake function
 */
void show_activity(int arg)
{
}
#endif
#if defined (CONFIG_SHOW_BOOT_PROGRESS)
static int a4m072_status2code(int status, char *buf)
{
	char c = 0;

	if (((status > 0) && (status <= 8)) ||
				((status >= 100) && (status <= 108)) ||
				((status < 0) && (status >= -9)) ||
				(status == -100) || (status == -101) ||
				((status <= -103) && (status >= -113))) {
		c = '5';
	} else if (((status >= 9) && (status <= 14)) ||
			((status >= 120) && (status <= 123)) ||
			((status >= 125) && (status <= 129)) ||
			((status >= -13) && (status <= -10)) ||
			(status == -120) || (status == -122) ||
			((status <= -124) && (status >= -127)) ||
			(status == -129)) {
		c = '8';
	} else if (status == 15) {
		c = '9';
	} else if ((status <= -30) && (status >= -32)) {
		c = 'A';
	} else if (((status <= -35) && (status >= -40)) ||
			((status <= -42) && (status >= -51)) ||
			((status <= -53) && (status >= -58)) ||
			(status == -64) ||
			((status <= -80) && (status >= -83)) ||
			(status == -130) || (status == -140) ||
			(status == -150)) {
		c = 'B';
	}

	if (c == 0)
		return -EINVAL;

	buf[0] = (status < 0) ? '-' : c;
	buf[1] = c;

	return 0;
}

void show_boot_progress(int status)
{
	char buf[2];

	if (a4m072_status2code(status, buf) < 0)
		return;

	display_putc(buf[0]);
	display_putc(buf[1]);
	display_set(DISPLAY_HOME);
	display_out_pos = 0;	/* reset output position */

	/* we want to flush status 15 now */
	if (status == BOOTSTAGE_ID_RUN_OS)
		display_flush();
}
#endif

/*
 * (C) Copyright 2003-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004
 * Mark Jonas, Freescale Semiconductor, mark.jonas@motorola.com.
 *
 * (C) Copyright 2004
 * Martin Krause, TQ-Systems GmbH, martin.krause@tqs.de
 *
 * (C) Copyright 2008
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fdt_support.h>
#include <mpc5xxx.h>
#include <pci.h>
#include <malloc.h>
#include <asm/processor.h>
#include <asm/io.h>

#ifndef CONFIG_SYS_RAMBOOT
static void sdram_start (int hi_addr)
{
	long hi_addr_bit = hi_addr ? 0x01000000 : 0;

	/* unlock mode register */
	out_be32 ((unsigned __iomem *)MPC5XXX_SDRAM_CTRL,
		(SDRAM_CONTROL | 0x80000000 | hi_addr_bit));
	__asm__ volatile ("sync");

	/* precharge all banks */
	out_be32 ((unsigned __iomem *)MPC5XXX_SDRAM_CTRL,
		(SDRAM_CONTROL | 0x80000002 | hi_addr_bit));
	__asm__ volatile ("sync");

#if SDRAM_DDR
	/* set mode register: extended mode */
	out_be32 ((unsigned __iomem *)MPC5XXX_SDRAM_MODE, (SDRAM_EMODE));
	__asm__ volatile ("sync");

	/* set mode register: reset DLL */
	out_be32 ((unsigned __iomem *)MPC5XXX_SDRAM_MODE,
		(SDRAM_MODE | 0x04000000));
	__asm__ volatile ("sync");
#endif

	/* precharge all banks */
	out_be32 ((unsigned __iomem *)MPC5XXX_SDRAM_CTRL,
		(SDRAM_CONTROL | 0x80000002 | hi_addr_bit));
	__asm__ volatile ("sync");

	/* auto refresh */
	out_be32 ((unsigned __iomem *)MPC5XXX_SDRAM_CTRL,
		(SDRAM_CONTROL | 0x80000004 | hi_addr_bit));
	__asm__ volatile ("sync");

	/* set mode register */
	out_be32 ((unsigned __iomem *)MPC5XXX_SDRAM_MODE, (SDRAM_MODE));
	__asm__ volatile ("sync");

	/* normal operation */
	out_be32 ((unsigned __iomem *)MPC5XXX_SDRAM_CTRL,
		(SDRAM_CONTROL | hi_addr_bit));
	__asm__ volatile ("sync");
}
#endif

/*
 * ATTENTION: Although partially referenced initdram does NOT make real use
 *	      use of CONFIG_SYS_SDRAM_BASE. The code does not work if CONFIG_SYS_SDRAM_BASE
 *	      is something else than 0x00000000.
 */

phys_size_t initdram (int board_type)
{
	ulong dramsize = 0;
	ulong dramsize2 = 0;
	uint svr, pvr;

#ifndef CONFIG_SYS_RAMBOOT
	ulong test1, test2;

	/* setup SDRAM chip selects */
	out_be32 ((unsigned __iomem *)MPC5XXX_SDRAM_CS0CFG, 0x0000001c); /* 512MB at 0x0 */
	out_be32 ((unsigned __iomem *)MPC5XXX_SDRAM_CS1CFG, 0x80000000);/* disabled */
	__asm__ volatile ("sync");

	/* setup config registers */
	out_be32 ((unsigned __iomem *)MPC5XXX_SDRAM_CONFIG1, SDRAM_CONFIG1);
	out_be32 ((unsigned __iomem *)MPC5XXX_SDRAM_CONFIG2, SDRAM_CONFIG2);
	__asm__ volatile ("sync");

#if SDRAM_DDR
	/* set tap delay */
	out_be32 ((unsigned __iomem *)MPC5XXX_CDM_PORCFG, SDRAM_TAPDELAY);
	__asm__ volatile ("sync");
#endif

	/* find RAM size using SDRAM CS0 only */
	sdram_start (0);
	test1 = get_ram_size ((long *)CONFIG_SYS_SDRAM_BASE, 0x20000000);
	sdram_start(1);
	test2 = get_ram_size ((long *)CONFIG_SYS_SDRAM_BASE, 0x20000000);
	if (test1 > test2) {
		sdram_start (0);
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
		out_be32 ((unsigned __iomem *)MPC5XXX_SDRAM_CS0CFG,
			(0x13 + __builtin_ffs(dramsize >> 20) - 1));
	} else {
		out_be32 ((unsigned __iomem *)MPC5XXX_SDRAM_CS0CFG, 0); /* disabled */
	}

	/* let SDRAM CS1 start right after CS0 */
	out_be32 ((unsigned __iomem *)MPC5XXX_SDRAM_CS1CFG, (dramsize + 0x0000001c));/*512MB*/

	/* find RAM size using SDRAM CS1 only */
	if (!dramsize)
		sdram_start (0);
	test2 = test1 = get_ram_size ((long *)(CONFIG_SYS_SDRAM_BASE + dramsize), 0x20000000);
	if (!dramsize) {
		sdram_start (1);
		test2 = get_ram_size ((long *)(CONFIG_SYS_SDRAM_BASE + dramsize), 0x20000000);
	}
	if (test1 > test2) {
		sdram_start (0);
		dramsize2 = test1;
	} else {
		dramsize2 = test2;
	}

	/* memory smaller than 1MB is impossible */
	if (dramsize2 < (1 << 20)) {
		dramsize2 = 0;
	}

	/* set SDRAM CS1 size according to the amount of RAM found */
	if (dramsize2 > 0) {
		out_be32 ((unsigned __iomem *)MPC5XXX_SDRAM_CS1CFG,
			(dramsize | (0x13 + __builtin_ffs(dramsize2 >> 20) - 1)));
	} else {
		out_be32 ((unsigned __iomem *)MPC5XXX_SDRAM_CS1CFG, dramsize); /* disabled */
	}

#else /* CONFIG_SYS_RAMBOOT */

	/* retrieve size of memory connected to SDRAM CS0 */
	dramsize = in_be32 ((unsigned __iomem *)MPC5XXX_SDRAM_CS0CFG) & 0xFF;
	if (dramsize >= 0x13) {
		dramsize = (1 << (dramsize - 0x13)) << 20;
	} else {
		dramsize = 0;
	}

	/* retrieve size of memory connected to SDRAM CS1 */
	dramsize2 = in_be32 ((unsigned __iomem *)MPC5XXX_SDRAM_CS1CFG) & 0xFF;
	if (dramsize2 >= 0x13) {
		dramsize2 = (1 << (dramsize2 - 0x13)) << 20;
	} else {
		dramsize2 = 0;
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

		out_be32 ((unsigned __iomem *)MPC5XXX_SDRAM_SDELAY, 0x04);
		__asm__ volatile ("sync");
	}

	return dramsize + dramsize2;
}

int checkboard (void)
{
	puts ("Board: MUC.MC-52 HW WDT ");
#if defined(CONFIG_HW_WATCHDOG)
	puts ("enabled\n");
#else
	puts ("disabled\n");
#endif
	return 0;
}

#ifdef CONFIG_PREBOOT

static uchar kbd_magic_prefix[]		= "key_magic";
static uchar kbd_command_prefix[]	= "key_cmd";

#define S1_ROT	0xf0
#define S2_Q	0x40
#define S2_M	0x20

struct kbd_data_t {
	char s1;
	char s2;
};

struct kbd_data_t* get_keys (struct kbd_data_t *kbd_data)
{
	kbd_data->s1 = in_8 ((volatile uchar*)CONFIG_SYS_STATUS1_BASE);
	kbd_data->s2 = in_8 ((volatile uchar*)CONFIG_SYS_STATUS2_BASE);

	return kbd_data;
}

static int compare_magic (const struct kbd_data_t *kbd_data, char *str)
{
	char s1 = str[0];
	char s2;

	if (s1 >= '0' && s1 <= '9')
		s1 -= '0';
	else if (s1 >= 'a' && s1 <= 'f')
		s1 = s1 - 'a' + 10;
	else if (s1 >= 'A' && s1 <= 'F')
		s1 = s1 - 'A' + 10;
	else
		return -1;

	if (((S1_ROT & kbd_data->s1) >> 4) != s1)
		return -1;

	s2 = (S2_Q | S2_M) & kbd_data->s2;

	switch (str[1]) {
	case 'q':
	case 'Q':
		if (s2 == S2_Q)
			return -1;
		break;
	case 'm':
	case 'M':
		if (s2 == S2_M)
			return -1;
		break;
	case '\0':
		if (s2 == (S2_Q | S2_M))
			return 0;
	default:
		return -1;
	}

	if (str[2])
		return -1;

	return 0;
}

static char *key_match (const struct kbd_data_t *kbd_data)
{
	char magic[sizeof (kbd_magic_prefix) + 1];
	char *suffix;
	char *kbd_magic_keys;

	/*
	 * The following string defines the characters that can be appended
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
	for (suffix = kbd_magic_keys; *suffix ||
		     suffix == kbd_magic_keys; ++suffix) {
		sprintf (magic, "%s%c", kbd_magic_prefix, *suffix);

		if (compare_magic(kbd_data, getenv(magic)) == 0) {
			char cmd_name[sizeof (kbd_command_prefix) + 1];
			char *cmd;

			sprintf (cmd_name, "%s%c", kbd_command_prefix, *suffix);
			cmd = getenv (cmd_name);

			return (cmd);
		}
	}

	return (NULL);
}

#endif /* CONFIG_PREBOOT */

int misc_init_r (void)
{
#ifdef CONFIG_PREBOOT
	struct kbd_data_t kbd_data;
	/* Decode keys */
	char *str = strdup (key_match (get_keys (&kbd_data)));
	/* Set or delete definition */
	setenv ("preboot", str);
	free (str);
#endif /* CONFIG_PREBOOT */

	out_8 ((volatile uchar *)(CONFIG_SYS_DISPLAY_BASE + 0x38), ' ');
	out_8 ((volatile uchar *)(CONFIG_SYS_DISPLAY_BASE + 0x39), ' ');
	out_8 ((volatile uchar *)(CONFIG_SYS_DISPLAY_BASE + 0x3A), ' ');
	out_8 ((volatile uchar *)(CONFIG_SYS_DISPLAY_BASE + 0x3B), ' ');
	out_8 ((volatile uchar *)(CONFIG_SYS_DISPLAY_BASE + 0x3C), ' ');
	out_8 ((volatile uchar *)(CONFIG_SYS_DISPLAY_BASE + 0x3D), ' ');
	out_8 ((volatile uchar *)(CONFIG_SYS_DISPLAY_BASE + 0x3E), ' ');
	out_8 ((volatile uchar *)(CONFIG_SYS_DISPLAY_BASE + 0x3F), ' ');

	return 0;
}

int board_early_init_r (void)
{
	out_be32 ((unsigned __iomem *)MPC5XXX_BOOTCS_CFG, in_be32 ((unsigned __iomem *)MPC5XXX_BOOTCS_CFG) & ~0x1);
	out_be32 ((unsigned __iomem *)MPC5XXX_BOOTCS_START, START_REG(CONFIG_SYS_FLASH_BASE));
	out_be32 ((unsigned __iomem *)MPC5XXX_CS0_START, START_REG(CONFIG_SYS_FLASH_BASE));
	out_be32 ((unsigned __iomem *)MPC5XXX_BOOTCS_STOP,
		STOP_REG(CONFIG_SYS_FLASH_BASE, CONFIG_SYS_FLASH_SIZE));
	out_be32 ((unsigned __iomem *)MPC5XXX_CS0_STOP,
		STOP_REG(CONFIG_SYS_FLASH_BASE, CONFIG_SYS_FLASH_SIZE));
	return 0;
}

int last_stage_init (void)
{
	out_8 ((volatile uchar *)(CONFIG_SYS_DISPLAY_BASE + 0x38), 'M');
	out_8 ((volatile uchar *)(CONFIG_SYS_DISPLAY_BASE + 0x39), 'U');
	out_8 ((volatile uchar *)(CONFIG_SYS_DISPLAY_BASE + 0x3A), 'C');
	out_8 ((volatile uchar *)(CONFIG_SYS_DISPLAY_BASE + 0x3B), '.');
	out_8 ((volatile uchar *)(CONFIG_SYS_DISPLAY_BASE + 0x3C), 'M');
	out_8 ((volatile uchar *)(CONFIG_SYS_DISPLAY_BASE + 0x3D), 'C');
	out_8 ((volatile uchar *)(CONFIG_SYS_DISPLAY_BASE + 0x3E), '5');
	out_8 ((volatile uchar *)(CONFIG_SYS_DISPLAY_BASE + 0x3F), '2');

	return 0;
}

#if defined(CONFIG_HW_WATCHDOG)
#define GPT_OUT_0	0x00000027
#define GPT_OUT_1	0x00000037
void hw_watchdog_reset (void)
{
	/* Trigger HW Watchdog with TIMER_0 */
	out_be32 ((unsigned __iomem *)MPC5XXX_GPT0_ENABLE, GPT_OUT_1);
	out_be32 ((unsigned __iomem *)MPC5XXX_GPT0_ENABLE, GPT_OUT_0);
}
#endif

#ifdef	CONFIG_PCI
static struct pci_controller hose;

extern void pci_mpc5xxx_init (struct pci_controller *);

void pci_init_board (void)
{
	pci_mpc5xxx_init (&hose);
}
#endif

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
}
#endif /* defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP) */

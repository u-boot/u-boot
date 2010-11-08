/*
 * (C) Copyright 2006
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * (C) Copyright 2003-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004
 * Mark Jonas, Freescale Semiconductor, mark.jonas@motorola.com.
 *
 * (C) Copyright 2004
 * Martin Krause, TQ-Systems GmbH, martin.krause@tqs.de
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <fdt_support.h>
#include <mpc5xxx.h>
#include <pci.h>
#include <malloc.h>

/* some SIMPLE GPIO Pins */
#define GPIO_USB_8	(31-12)
#define GPIO_USB_7	(31-13)
#define GPIO_USB_6	(31-14)
#define GPIO_USB_0	(31-15)
#define GPIO_PSC3_7	(31-18)
#define GPIO_PSC3_6	(31-19)
#define GPIO_PSC3_1	(31-22)
#define GPIO_PSC3_0	(31-23)

/* some simple Interrupt GPIO Pins */
#define GPIO_PSC3_8	2
#define GPIO_USB1_9	3

#define GPT_OUT_0	0x00000027
#define GPT_OUT_1	0x00000037
#define	GPT_DISABLE	0x00000000	/* GPT pin disabled */

#define GP_SIMP_ENABLE_O(n, v) {pgpio->simple_dvo |= (v << n); \
				pgpio->simple_ddr |= (1 << n); \
				pgpio->simple_gpioe |= (1 << n); \
				}

#define GP_SIMP_ENABLE_I(n) {	pgpio->simple_ddr |= ~(1 << n); \
				pgpio->simple_gpioe |= (1 << n); \
				}

#define GP_SIMP_SET_O(n, v)  (pgpio->simple_dvo = v ? \
				(pgpio->simple_dvo | (1 << n)) : \
				(pgpio->simple_dvo & ~(1 << n)) )

#define GP_SIMP_GET_O(n)  ((pgpio->simple_dvo >> n) & 1)
#define GP_SIMP_GET_I(n)  ((pgpio->simple_ival >> n) & 1)

#define GP_SINT_SET_O(n, v)  (pgpio->sint_dvo = v ? \
				(pgpio->sint_dvo | (1 << n)) : \
				(pgpio->sint_dvo & ~(1 << n)) )

#define GP_SINT_ENABLE_O(n, v) {pgpio->sint_ode &= ~(1 << n); \
				pgpio->sint_ddr |= (1 << n); \
				GP_SINT_SET_O(n, v); \
				pgpio->sint_gpioe |= (1 << n); \
				}

#define GP_SINT_ENABLE_I(n) {	pgpio->sint_ddr |= ~(1 << n); \
				pgpio->sint_gpioe |= (1 << n); \
				}

#define GP_SINT_GET_O(n)  ((pgpio->sint_ival >> n) & 1)
#define GP_SINT_GET_I(n)  ((pgpio-ntt_ival >> n) & 1)

#define GP_TIMER_ENABLE_O(n, v) ( \
	((volatile struct mpc5xxx_gpt *)(MPC5XXX_GPT + n))->emsr = v ? \
				GPT_OUT_1 : \
				GPT_OUT_0 )

#define GP_TIMER_SET_O(n, v)	GP_TIMER_ENABLE_O(n, v)

#define GP_TIMER_GET_O(n, v) ( \
	(((volatile struct mpc5xxx_gpt *)(MPC5XXX_GPT + n))->emsr & 0x10) >> 4)

#define GP_TIMER_GET_I(n, v) ( \
	(((volatile struct mpc5xxx_gpt *)(MPC5XXX_GPT + n))->sr & 0x100) >> 8)

#ifndef CONFIG_SYS_RAMBOOT
static void sdram_start (int hi_addr)
{
	long hi_addr_bit = hi_addr ? 0x01000000 : 0;

	/* unlock mode register */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = SDRAM_CONTROL | 0x80000000 | hi_addr_bit;
	__asm__ volatile ("sync");

	/* precharge all banks */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = SDRAM_CONTROL | 0x80000002 | hi_addr_bit;
	__asm__ volatile ("sync");

#if SDRAM_DDR
	/* set mode register: extended mode */
	*(vu_long *)MPC5XXX_SDRAM_MODE = SDRAM_EMODE;
	__asm__ volatile ("sync");

	/* set mode register: reset DLL */
	*(vu_long *)MPC5XXX_SDRAM_MODE = SDRAM_MODE | 0x04000000;
	__asm__ volatile ("sync");
#endif

	/* precharge all banks */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = SDRAM_CONTROL | 0x80000002 | hi_addr_bit;
	__asm__ volatile ("sync");

	/* auto refresh */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = SDRAM_CONTROL | 0x80000004 | hi_addr_bit;
	__asm__ volatile ("sync");

	/* set mode register */
	*(vu_long *)MPC5XXX_SDRAM_MODE = SDRAM_MODE;
	__asm__ volatile ("sync");

	/* normal operation */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = SDRAM_CONTROL | hi_addr_bit;
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
#ifndef CONFIG_SYS_RAMBOOT
	ulong test1, test2;

	/* setup SDRAM chip selects */
	*(vu_long *)MPC5XXX_SDRAM_CS0CFG = 0x0000001c; /* 512MB at 0x0 */
	*(vu_long *)MPC5XXX_SDRAM_CS1CFG = 0x40000000; /* disabled */
	__asm__ volatile ("sync");

	/* setup config registers */
	*(vu_long *)MPC5XXX_SDRAM_CONFIG1 = SDRAM_CONFIG1;
	*(vu_long *)MPC5XXX_SDRAM_CONFIG2 = SDRAM_CONFIG2;
	__asm__ volatile ("sync");

#if SDRAM_DDR
	/* set tap delay */
	*(vu_long *)MPC5XXX_CDM_PORCFG = SDRAM_TAPDELAY;
	__asm__ volatile ("sync");
#endif

	/* find RAM size using SDRAM CS0 only */
	sdram_start(0);
	test1 = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE, 0x20000000);
	sdram_start(1);
	test2 = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE, 0x20000000);
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
		*(vu_long *)MPC5XXX_SDRAM_CS0CFG = 0x13 +
			__builtin_ffs(dramsize >> 20) - 1;
	} else {
		*(vu_long *)MPC5XXX_SDRAM_CS0CFG = 0; /* disabled */
	}

	*(vu_long *)MPC5XXX_SDRAM_CS1CFG = dramsize; /* disabled */
#else /* CONFIG_SYS_RAMBOOT */

	/* retrieve size of memory connected to SDRAM CS0 */
	dramsize = *(vu_long *)MPC5XXX_SDRAM_CS0CFG & 0xFF;
	if (dramsize >= 0x13) {
		dramsize = (1 << (dramsize - 0x13)) << 20;
	} else {
		dramsize = 0;
	}

	/* retrieve size of memory connected to SDRAM CS1 */
	dramsize2 = *(vu_long *)MPC5XXX_SDRAM_CS1CFG & 0xFF;
	if (dramsize2 >= 0x13) {
		dramsize2 = (1 << (dramsize2 - 0x13)) << 20;
	} else {
		dramsize2 = 0;
	}

#endif /* CONFIG_SYS_RAMBOOT */

/*	return dramsize + dramsize2; */
	return dramsize;
}

int checkboard (void)
{
	puts ("Board: MAN UC101\n");
	/* clear the Display */
	*(char *)(CONFIG_SYS_DISP_CWORD) = 0x80;
	return 0;
}

static void init_ports (void)
{
	volatile struct mpc5xxx_gpio *pgpio =
		(struct mpc5xxx_gpio *)MPC5XXX_GPIO;

	GP_SIMP_ENABLE_I(GPIO_USB_8);	/* HEX Bit 3 */
	GP_SIMP_ENABLE_I(GPIO_USB_7);	/* HEX Bit 2 */
	GP_SIMP_ENABLE_I(GPIO_USB_6);	/* HEX Bit 1 */
	GP_SIMP_ENABLE_I(GPIO_USB_0);	/* HEX Bit 0 */
	GP_SIMP_ENABLE_I(GPIO_PSC3_0);	/* Switch Menue A */
	GP_SIMP_ENABLE_I(GPIO_PSC3_1);	/* Switch Menue B */
	GP_SIMP_ENABLE_I(GPIO_PSC3_6);	/* Switch Cold_Warm */
	GP_SIMP_ENABLE_I(GPIO_PSC3_7);	/* Switch Restart */
	GP_SINT_ENABLE_O(GPIO_PSC3_8, 0);	/* LED H2 */
	GP_SINT_ENABLE_O(GPIO_USB1_9, 0);	/* LED H3 */
	GP_TIMER_ENABLE_O(4, 0);	/* LED H4 */
	GP_TIMER_ENABLE_O(5, 0);	/* LED H5 */
	GP_TIMER_ENABLE_O(3, 0);	/* LED HB */
	GP_TIMER_ENABLE_O(1, 0);	/* RES_COLDSTART */
}

#ifdef CONFIG_PREBOOT

static uchar kbd_magic_prefix[]		= "key_magic";
static uchar kbd_command_prefix[]	= "key_cmd";

struct kbd_data_t {
	char s1;
};

struct kbd_data_t* get_keys (struct kbd_data_t *kbd_data)
{
	volatile struct mpc5xxx_gpio *pgpio =
		(struct mpc5xxx_gpio *)MPC5XXX_GPIO;

	kbd_data->s1 = GP_SIMP_GET_I(GPIO_USB_8) << 3 | \
			GP_SIMP_GET_I(GPIO_USB_7) << 2 | \
			GP_SIMP_GET_I(GPIO_USB_6) << 1 | \
			GP_SIMP_GET_I(GPIO_USB_0) << 0;
	return kbd_data;
}

static int compare_magic (const struct kbd_data_t *kbd_data, char *str)
{
	char s1 = str[0];

	if (s1 >= '0' && s1 <= '9')
		s1 -= '0';
	else if (s1 >= 'a' && s1 <= 'f')
		s1 = s1 - 'a' + 10;
	else if (s1 >= 'A' && s1 <= 'F')
		s1 = s1 - 'A' + 10;
	else
		return -1;

	if (s1 != kbd_data->s1) return -1;
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
	/* Init the I/O ports */
	init_ports ();

#ifdef CONFIG_PREBOOT
	struct kbd_data_t kbd_data;
	/* Decode keys */
	char *str = strdup (key_match (get_keys (&kbd_data)));
	/* Set or delete definition */
	setenv ("preboot", str);
	free (str);
#endif /* CONFIG_PREBOOT */
	return 0;
}

int board_early_init_r (void)
{
	*(vu_long *)MPC5XXX_BOOTCS_CFG &= ~0x1; /* clear RO */
	*(vu_long *)MPC5XXX_BOOTCS_START =
	*(vu_long *)MPC5XXX_CS0_START = START_REG(CONFIG_SYS_FLASH_BASE);
	*(vu_long *)MPC5XXX_BOOTCS_STOP =
	*(vu_long *)MPC5XXX_CS0_STOP = STOP_REG(CONFIG_SYS_FLASH_BASE, CONFIG_SYS_FLASH_SIZE);
	/* Interbus enable it here ?? */
	*(vu_long *)MPC5XXX_GPT6_ENABLE = GPT_OUT_1;
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

#if defined(CONFIG_HW_WATCHDOG)
void hw_watchdog_reset(void)
{
	/* Trigger HW Watchdog with TIMER_0 */
	*(vu_long *)MPC5XXX_GPT0_ENABLE = GPT_OUT_1;
	*(vu_long *)MPC5XXX_GPT0_ENABLE = GPT_OUT_0;
}
#endif

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
}
#endif /* defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP) */

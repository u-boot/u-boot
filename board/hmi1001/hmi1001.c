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
#include <mpc5xxx.h>
#include <pci.h>
#include <malloc.h>

#ifndef CFG_RAMBOOT
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
 *	      use of CFG_SDRAM_BASE. The code does not work if CFG_SDRAM_BASE
 *	      is something else than 0x00000000.
 */

long int initdram (int board_type)
{
	ulong dramsize = 0;
#ifndef CFG_RAMBOOT
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
	test1 = get_ram_size((long *)CFG_SDRAM_BASE, 0x20000000);
	sdram_start(1);
	test2 = get_ram_size((long *)CFG_SDRAM_BASE, 0x20000000);
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
#else /* CFG_RAMBOOT */

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

#endif /* CFG_RAMBOOT */

/*	return dramsize + dramsize2; */
	return dramsize;
}

int checkboard (void)
{
	puts ("Board: HMI1001\n");
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
	kbd_data->s1 = *((volatile uchar*)(CFG_STATUS1_BASE));
	kbd_data->s2 = *((volatile uchar*)(CFG_STATUS2_BASE));

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

	return 0;
}

int board_early_init_r (void)
{
	*(vu_long *)MPC5XXX_BOOTCS_CFG &= ~0x1; /* clear RO */
	*(vu_long *)MPC5XXX_BOOTCS_START =
	*(vu_long *)MPC5XXX_CS0_START = START_REG(CFG_FLASH_BASE);
	*(vu_long *)MPC5XXX_BOOTCS_STOP =
	*(vu_long *)MPC5XXX_CS0_STOP = STOP_REG(CFG_FLASH_BASE, CFG_FLASH_SIZE);
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

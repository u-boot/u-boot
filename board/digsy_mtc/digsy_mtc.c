/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004
 * Mark Jonas, Freescale Semiconductor, mark.jonas@motorola.com.
 *
 * (C) Copyright 2005-2009
 * Modified for InterControl digsyMTC MPC5200 board by
 * Frank Bodammer, GCD Hard- & Software GmbH,
 *                 frank.bodammer@gcd-solutions.de
 *
 * (C) Copyright 2009
 * Grzegorz Bernacki, Semihalf, gjb@semihalf.com
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
#include <mpc5xxx.h>
#include <net.h>
#include <pci.h>
#include <asm/processor.h>
#include <asm/io.h>
#include "eeprom.h"
#include "is42s16800a-7t.h"

DECLARE_GLOBAL_DATA_PTR;

extern int usb_cpu_init(void);

#ifndef CONFIG_SYS_RAMBOOT
static void sdram_start(int hi_addr)
{
	long hi_addr_bit = hi_addr ? 0x01000000 : 0;
	long control = SDRAM_CONTROL | hi_addr_bit;

	/* unlock mode register */
	out_be32((void *)MPC5XXX_SDRAM_CTRL, control | 0x80000000);

	/* precharge all banks */
	out_be32((void *)MPC5XXX_SDRAM_CTRL, control | 0x80000002);

	/* auto refresh */
	out_be32((void *)MPC5XXX_SDRAM_CTRL, control | 0x80000004);

	/* set mode register */
	out_be32((void *)MPC5XXX_SDRAM_MODE, SDRAM_MODE);

	/* normal operation */
	out_be32((void *)MPC5XXX_SDRAM_CTRL, control);
}
#endif

/*
 * ATTENTION: Although partially referenced initdram does NOT make real use
 *            use of CONFIG_SYS_SDRAM_BASE. The code does not work if
 *            CONFIG_SYS_SDRAM_BASE is something else than 0x00000000.
 */

phys_size_t initdram(int board_type)
{
	ulong dramsize = 0;
	ulong dramsize2 = 0;
	uint svr, pvr;
#ifndef CONFIG_SYS_RAMBOOT
	ulong test1, test2;

	/* setup SDRAM chip selects */
	out_be32((void *)MPC5XXX_SDRAM_CS0CFG, 0x0000001C); /* 512MB at 0x0 */
	out_be32((void *)MPC5XXX_SDRAM_CS1CFG, 0x80000000); /* disabled */

	/* setup config registers */
	out_be32((void *)MPC5XXX_SDRAM_CONFIG1, SDRAM_CONFIG1);
	out_be32((void *)MPC5XXX_SDRAM_CONFIG2, SDRAM_CONFIG2);

	/* find RAM size using SDRAM CS0 only */
	sdram_start(0);
	test1 = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE, 0x08000000);
	sdram_start(1);
	test2 = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE, 0x08000000);
	if (test1 > test2) {
		sdram_start(0);
		dramsize = test1;
	} else {
		dramsize = test2;
	}

	/* memory smaller than 1MB is impossible */
	if (dramsize < (1 << 20))
		dramsize = 0;

	/* set SDRAM CS0 size according to the amount of RAM found */
	if (dramsize > 0) {
		out_be32((void *)MPC5XXX_SDRAM_CS0CFG,
			(0x13 + __builtin_ffs(dramsize >> 20) - 1));
	} else {
		out_be32((void *)MPC5XXX_SDRAM_CS0CFG, 0); /* disabled */
	}

	/* let SDRAM CS1 start right after CS0 */
	out_be32((void *)MPC5XXX_SDRAM_CS1CFG, dramsize + 0x0000001C);

	/* find RAM size using SDRAM CS1 only */
	test1 = get_ram_size((long *)(CONFIG_SYS_SDRAM_BASE + dramsize),
			0x08000000);
		dramsize2 = test1;

	/* memory smaller than 1MB is impossible */
	if (dramsize2 < (1 << 20))
		dramsize2 = 0;

	/* set SDRAM CS1 size according to the amount of RAM found */
	if (dramsize2 > 0) {
		out_be32((void *)MPC5XXX_SDRAM_CS1CFG, (dramsize |
			(0x13 + __builtin_ffs(dramsize2 >> 20) - 1)));
	} else {
		out_be32((void *)MPC5XXX_SDRAM_CS1CFG, dramsize); /* disabled */
	}

#else /* CONFIG_SYS_RAMBOOT */

	/* retrieve size of memory connected to SDRAM CS0 */
	dramsize = in_be32((void *)MPC5XXX_SDRAM_CS0CFG) & 0xFF;
	if (dramsize >= 0x13)
		dramsize = (1 << (dramsize - 0x13)) << 20;
	else
		dramsize = 0;

	/* retrieve size of memory connected to SDRAM CS1 */
	dramsize2 = in_be32((void *)MPC5XXX_SDRAM_CS1CFG) & 0xFF;
	if (dramsize2 >= 0x13)
		dramsize2 = (1 << (dramsize2 - 0x13)) << 20;
	else
		dramsize2 = 0;

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
	    (PVR_MAJ(pvr) == 1) && (PVR_MIN(pvr) == 4))
		out_be32((void *)MPC5XXX_SDRAM_SDELAY, 0x04);

	return dramsize + dramsize2;
}

int checkboard(void)
{
	char *s = getenv("serial#");

	puts ("Board: InterControl digsyMTC");
	if (s != NULL) {
		puts(", ");
		puts(s);
	}
	putc('\n');

	return 0;
}

int board_early_init_r(void)
{
#ifdef CONFIG_MPC52XX_SPI
	struct mpc5xxx_gpt *gpt = (struct mpc5xxx_gpt*)MPC5XXX_GPT;
#endif
	/*
	 * Now, when we are in RAM, enable flash write access for detection
	 * process.  Note that CS_BOOT cannot be cleared when executing in
	 * flash.
	 */
	/* disable CS_BOOT */
	clrbits_be32((void *)MPC5XXX_ADDECR, (1 << 25));
	/* enable CS1 */
	setbits_be32((void *)MPC5XXX_ADDECR, (1 << 17));
	/* enable CS0 */
	setbits_be32((void *)MPC5XXX_ADDECR, (1 << 16));

#if defined(CONFIG_USB_OHCI_NEW) && defined(CONFIG_SYS_USB_OHCI_CPU_INIT)
	/* Low level USB init, required for proper kernel operation */
	usb_cpu_init();
#endif
#ifdef CONFIG_MPC52XX_SPI
	/* GPT 6 Output Enable */
	out_be32(&gpt[6].emsr, 0x00000034);
	/* GPT 7 Output Enable */
	out_be32(&gpt[7].emsr, 0x00000034);
#endif

	return (0);
}

void board_get_enetaddr (uchar * enet)
{
	ushort read = 0;
	ushort addr_of_eth_addr = 0;
	ushort len_sys = 0;
	ushort len_sys_cfg = 0;

	/* check identification word */
	eeprom_read(EEPROM_ADDR, EEPROM_ADDR_IDENT, (uchar *)&read, 2);
	if (read != EEPROM_IDENT)
		return;

	/* calculate offset of config area */
	eeprom_read(EEPROM_ADDR, EEPROM_ADDR_LEN_SYS, (uchar *)&len_sys, 2);
	eeprom_read(EEPROM_ADDR, EEPROM_ADDR_LEN_SYSCFG,
		(uchar *)&len_sys_cfg, 2);
	addr_of_eth_addr = (len_sys + len_sys_cfg + EEPROM_ADDR_ETHADDR) << 1;
	if (addr_of_eth_addr >= EEPROM_LEN)
		return;

	eeprom_read(EEPROM_ADDR, addr_of_eth_addr, enet, 6);
}

int misc_init_r(void)
{
	uchar enetaddr[6];

	if (!eth_getenv_enetaddr("ethaddr", enetaddr)) {
		board_get_enetaddr(enetaddr);
		eth_setenv_enetaddr("ethaddr", enetaddr);
	}

	return 0;
}

#ifdef CONFIG_PCI
static struct pci_controller hose;

extern void pci_mpc5xxx_init(struct pci_controller *);

void pci_init_board(void)
{
	pci_mpc5xxx_init(&hose);
}
#endif

#ifdef CONFIG_CMD_IDE

#ifdef CONFIG_IDE_RESET

void init_ide_reset(void)
{
	debug ("init_ide_reset\n");

	/* set gpio output value to 1 */
	setbits_be32((void *)MPC5XXX_WU_GPIO_DATA_O, (1 << 25));
	/* open drain output */
	setbits_be32((void *)MPC5XXX_WU_GPIO_ODE, (1 << 25));
	/* direction output */
	setbits_be32((void *)MPC5XXX_WU_GPIO_DIR, (1 << 25));
	/* enable gpio */
	setbits_be32((void *)MPC5XXX_WU_GPIO_ENABLE, (1 << 25));

}

void ide_set_reset(int idereset)
{
	debug ("ide_reset(%d)\n", idereset);

	/* set gpio output value to 0 */
	clrbits_be32((void *)MPC5XXX_WU_GPIO_DATA_O, (1 << 25));
	/* open drain output */
	setbits_be32((void *)MPC5XXX_WU_GPIO_ODE, (1 << 25));
	/* direction output */
	setbits_be32((void *)MPC5XXX_WU_GPIO_DIR, (1 << 25));
	/* enable gpio */
	setbits_be32((void *)MPC5XXX_WU_GPIO_ENABLE, (1 << 25));

	udelay(10000);

	/* set gpio output value to 1 */
	setbits_be32((void *)MPC5XXX_WU_GPIO_DATA_O, (1 << 25));
	/* open drain output */
	setbits_be32((void *)MPC5XXX_WU_GPIO_ODE, (1 << 25));
	/* direction output */
	setbits_be32((void *)MPC5XXX_WU_GPIO_DIR, (1 << 25));
	/* enable gpio */
	setbits_be32((void *)MPC5XXX_WU_GPIO_ENABLE, (1 << 25));
}
#endif /* CONFIG_IDE_RESET */

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
}
#endif /* defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP) */

#endif /* CONFIG_CMD_IDE */

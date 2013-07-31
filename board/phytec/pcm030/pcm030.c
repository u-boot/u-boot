/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004
 * Mark Jonas, Freescale Semiconductor, mark.jonas@motorola.com.
 *
 * (C) Copyright 2006
 * Eric Schumann, Phytec Messtechnik GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mpc5xxx.h>
#include <pci.h>
#include <asm/io.h>

#include "mt46v32m16-75.h"

#ifndef CONFIG_SYS_RAMBOOT
static void sdram_start(int hi_addr)
{
	volatile struct mpc5xxx_cdm *cdm =
		(struct mpc5xxx_cdm *)MPC5XXX_CDM;
	volatile struct mpc5xxx_sdram *sdram =
		(struct mpc5xxx_sdram *)MPC5XXX_SDRAM;

	long hi_addr_bit = hi_addr ? 0x01000000 : 0;

	/* unlock mode register */
	out_be32 (&sdram->ctrl,
		(SDRAM_CONTROL | 0x80000000 | hi_addr_bit));

	/* precharge all banks */
	out_be32 (&sdram->ctrl,
		(SDRAM_CONTROL | 0x80000002 | hi_addr_bit));

#ifdef SDRAM_DDR
	/* set mode register: extended mode */
	out_be32 (&sdram->mode, (SDRAM_EMODE));

	/* set mode register: reset DLL */
	out_be32 (&sdram->mode,
		(SDRAM_MODE | 0x04000000));
#endif

	/* precharge all banks */
	out_be32 (&sdram->ctrl,
		(SDRAM_CONTROL | 0x80000002 | hi_addr_bit));

	/* auto refresh */
	out_be32 (&sdram->ctrl,
		(SDRAM_CONTROL | 0x80000004 | hi_addr_bit));

	/* set mode register */
	out_be32 (&sdram->mode, (SDRAM_MODE));

	/* normal operation */
	out_be32 (&sdram->ctrl,
		(SDRAM_CONTROL | hi_addr_bit));

	/* set CDM clock enable register, set MPC5200B SDRAM bus */
	/* to reduced driver strength */
	out_be32 (&cdm->clock_enable, (0x00CFFFFF));
}
#endif

/*
 * ATTENTION: Although partially referenced initdram does NOT make
 *	real use of CONFIG_SYS_SDRAM_BASE. The code does not
 *	work if CONFIG_SYS_SDRAM_BASE
 *	is something else than 0x00000000.
 */

phys_size_t initdram(int board_type)
{
	volatile struct mpc5xxx_mmap_ctl *mm =
		(struct mpc5xxx_mmap_ctl *)CONFIG_SYS_MBAR;
	volatile struct mpc5xxx_cdm *cdm =
		(struct mpc5xxx_cdm *)MPC5XXX_CDM;
	volatile struct mpc5xxx_sdram *sdram =
		(struct mpc5xxx_sdram *)MPC5XXX_SDRAM;
	ulong dramsize = 0;
	ulong dramsize2 = 0;
#ifndef CONFIG_SYS_RAMBOOT
	ulong test1, test2;

	/* setup SDRAM chip selects */
							 /* 256MB at 0x0 */
	out_be32 (&mm->sdram0, 0x0000001b);
							 /* disabled */
	out_be32 (&mm->sdram1, 0x10000000);

	/* setup config registers */
	out_be32 (&sdram->config1, SDRAM_CONFIG1);
	out_be32 (&sdram->config2, SDRAM_CONFIG2);

#if defined(SDRAM_DDR) && defined(SDRAM_TAPDELAY)
	/* set tap delay */
	out_be32 (&cdm->porcfg, SDRAM_TAPDELAY);
#endif

	/* find RAM size using SDRAM CS0 only */
	sdram_start(0);
	test1 = get_ram_size((long *) CONFIG_SYS_SDRAM_BASE, 0x10000000);
	sdram_start(1);
	test2 = get_ram_size((long *) CONFIG_SYS_SDRAM_BASE, 0x10000000);
	if (test1 > test2) {
		sdram_start(0);
		dramsize = test1;
	} else
		dramsize = test2;

	/* memory smaller than 1MB is impossible */
	if (dramsize < (1 << 20))
		dramsize = 0;

	/* set SDRAM CS0 size according to the amount of RAM found */
	if (dramsize > 0) {
		out_be32 (&mm->sdram0,
			(0x13 + __builtin_ffs(dramsize >> 20) - 1));
	} else {
							/* disabled */
		out_be32 (&mm->sdram0, 0);
	}

#else /* CONFIG_SYS_RAMBOOT */

	/* retrieve size of memory connected to SDRAM CS0 */
	dramsize = in_be32(&mm->sdram0) & 0xFF;
	if (dramsize >= 0x13)
		dramsize = (1 << (dramsize - 0x13)) << 20;
	else
		dramsize = 0;

	/* retrieve size of memory connected to SDRAM CS1 */
	dramsize2 = in_be32(&mm->sdram1) & 0xFF;
	if (dramsize2 >= 0x13)
		dramsize2 = (1 << (dramsize2 - 0x13)) << 20;
	else
		dramsize2 = 0;

#endif /* CONFIG_SYS_RAMBOOT */

	return dramsize + dramsize2;
}

int checkboard(void)
{
	puts("Board: phyCORE-MPC5200B-tiny\n");
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

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t * bd)
{
	ft_cpu_setup(blob, bd);
}
#endif /* defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP) */

#if defined(CONFIG_CMD_IDE) && defined(CONFIG_IDE_RESET)

#define GPIO_PSC2_4	0x02000000UL

void init_ide_reset(void)
{
	volatile struct mpc5xxx_wu_gpio *wu_gpio =
		(struct mpc5xxx_wu_gpio *)MPC5XXX_WU_GPIO;
	debug("init_ide_reset\n");

	/* Configure PSC2_4 as GPIO output for ATA reset */
	setbits_be32(&wu_gpio->enable, GPIO_PSC2_4);
	setbits_be32(&wu_gpio->ddr, GPIO_PSC2_4);
	/* Deassert reset */
	setbits_be32(&wu_gpio->dvo, GPIO_PSC2_4);
}

void ide_set_reset(int idereset)
{
	volatile struct mpc5xxx_wu_gpio *wu_gpio =
		(struct mpc5xxx_wu_gpio *)MPC5XXX_WU_GPIO;
	debug("ide_reset(%d)\n", idereset);

	if (idereset) {
		clrbits_be32(&wu_gpio->dvo, GPIO_PSC2_4);
		/* Make a delay. MPC5200 spec says 25 usec min */
		udelay(500000);
	} else
		setbits_be32(&wu_gpio->dvo, GPIO_PSC2_4);
}
#endif /* defined(CONFIG_CMD_IDE) && defined(CONFIG_IDE_RESET) */

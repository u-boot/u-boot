/*
 * (C) Copyright 2008-2009
 * Andreas Pfefferle, DENX Software Engineering, ap@denx.de.
 *
 * (C) Copyright 2009
 * Detlev Zundel, DENX Software Engineering, dzu@denx.de.
 *
 * (C) Copyright 2004
 * Mark Jonas, Freescale Semiconductor, mark.jonas@motorola.com.
 *
 * (C) Copyright 2004
 * Martin Krause, TQ-Systems GmbH, martin.krause@tqs.de
 *
 * (C) Copyright 2003-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/io.h>
#include <common.h>
#include <mpc5xxx.h>
#include <pci.h>

#if defined(CONFIG_DDR_MT46V16M16)
#include "mt46v16m16-75.h"
#elif defined(CONFIG_SDR_MT48LC16M16A2)
#include "mt48lc16m16a2-75.h"
#elif defined(CONFIG_DDR_MT46V32M16)
#include "mt46v32m16.h"
#elif defined(CONFIG_DDR_HYB25D512160BF)
#include "hyb25d512160bf.h"
#elif defined(CONFIG_DDR_K4H511638C)
#include "k4h511638c.h"
#else
#error "INKA4x0 SDRAM: invalid chip type specified!"
#endif

#ifndef CONFIG_SYS_RAMBOOT
static void sdram_start (int hi_addr)
{
	volatile struct mpc5xxx_sdram *sdram =
		(struct mpc5xxx_sdram *)MPC5XXX_SDRAM;
	long hi_addr_bit = hi_addr ? 0x01000000 : 0;

	/* unlock mode register */
	out_be32(&sdram->ctrl, SDRAM_CONTROL | 0x80000000 | hi_addr_bit);

	/* precharge all banks */
	out_be32(&sdram->ctrl, SDRAM_CONTROL | 0x80000002 | hi_addr_bit);

#if SDRAM_DDR
	/* set mode register: extended mode */
	out_be32(&sdram->mode, SDRAM_EMODE);

	/* set mode register: reset DLL */
	out_be32(&sdram->mode, SDRAM_MODE | 0x04000000);
#endif

	/* precharge all banks */
	out_be32(&sdram->ctrl, SDRAM_CONTROL | 0x80000002 | hi_addr_bit);

	/* auto refresh */
	out_be32(&sdram->ctrl, SDRAM_CONTROL | 0x80000004 | hi_addr_bit);

	/* set mode register */
	out_be32(&sdram->mode, SDRAM_MODE);

	/* normal operation */
	out_be32(&sdram->ctrl, SDRAM_CONTROL | hi_addr_bit);
}
#endif

/*
 * ATTENTION: Although partially referenced initdram does NOT make real use
 *	      use of CONFIG_SYS_SDRAM_BASE. The code does not work if CONFIG_SYS_SDRAM_BASE
 *	      is something else than 0x00000000.
 */

phys_size_t initdram (int board_type)
{
	volatile struct mpc5xxx_mmap_ctl *mm =
		(struct mpc5xxx_mmap_ctl *) CONFIG_SYS_MBAR;
	volatile struct mpc5xxx_cdm     *cdm =
		(struct mpc5xxx_cdm *)      MPC5XXX_CDM;
	volatile struct mpc5xxx_sdram *sdram =
		(struct mpc5xxx_sdram *)    MPC5XXX_SDRAM;
	ulong dramsize = 0;
#ifndef CONFIG_SYS_RAMBOOT
	long test1, test2;

	/* setup SDRAM chip selects */
	out_be32(&mm->sdram0, 0x0000001c);	/* 512MB at 0x0 */
	out_be32(&mm->sdram1, 0x40000000);	/* disabled */

	/* setup config registers */
	out_be32(&sdram->config1, SDRAM_CONFIG1);
	out_be32(&sdram->config2, SDRAM_CONFIG2);

#if SDRAM_DDR
	/* set tap delay */
	out_be32(&cdm->porcfg, SDRAM_TAPDELAY);
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
		out_be32(&mm->sdram0, 0x13 +
			 __builtin_ffs(dramsize >> 20) - 1);
	} else {
		out_be32(&mm->sdram0, 0); /* disabled */
	}

	out_be32(&mm->sdram1, dramsize); /* disabled */
#else /* CONFIG_SYS_RAMBOOT */

	/* retrieve size of memory connected to SDRAM CS0 */
	dramsize = in_be32(&mm->sdram0) & 0xFF;
	if (dramsize >= 0x13) {
		dramsize = (1 << (dramsize - 0x13)) << 20;
	} else {
		dramsize = 0;
	}
#endif /* CONFIG_SYS_RAMBOOT */

	return dramsize;
}

int checkboard (void)
{
	puts ("Board: INKA 4X0\n");
	return 0;
}

void flash_preinit(void)
{
	volatile struct mpc5xxx_lpb *lpb = (struct mpc5xxx_lpb *)MPC5XXX_LPB;

	/*
	 * Now, when we are in RAM, enable flash write
	 * access for detection process.
	 * Note that CS_BOOT (CS0) cannot be cleared when
	 * executing in flash.
	 */
	clrbits_be32(&lpb->cs0_cfg, 0x1); /* clear RO */
}

int misc_init_f (void)
{
	volatile struct mpc5xxx_gpio	*gpio    =
		(struct mpc5xxx_gpio *)   MPC5XXX_GPIO;
	volatile struct mpc5xxx_wu_gpio	*wu_gpio =
		(struct mpc5xxx_wu_gpio *)MPC5XXX_WU_GPIO;
	volatile struct mpc5xxx_gpt	*gpt;
	char tmp[10];
	int i, br;

	i = getenv_f("brightness", tmp, sizeof(tmp));
	br = (i > 0)
		? (int) simple_strtoul (tmp, NULL, 10)
		: CONFIG_SYS_BRIGHTNESS;
	if (br > 255)
		br = 255;

	/* Initialize GPIO output pins.
	 */
	/* Configure GPT as GPIO output (and set them as they control low-active LEDs */
	for (i = 0; i <= 5; i++) {
		gpt = (struct mpc5xxx_gpt *)(MPC5XXX_GPT + (i * 0x10));
		out_be32(&gpt->emsr, 0x34);
	}

	/* Configure GPT7 as PWM timer, 1kHz, no ints. */
	gpt = (struct mpc5xxx_gpt *)(MPC5XXX_GPT + (7 * 0x10));
	out_be32(&gpt->emsr,  0);		/* Disable */
	out_be32(&gpt->cir,   0x020000fe);
	out_be32(&gpt->pwmcr, (br << 16));
	out_be32(&gpt->emsr,  0x3);		/* Enable PWM mode and start */

	/* Configure PSC3_6,7 as GPIO output */
	setbits_be32(&gpio->simple_gpioe, MPC5XXX_GPIO_SIMPLE_PSC3_6 |
					  MPC5XXX_GPIO_SIMPLE_PSC3_7);
	setbits_be32(&gpio->simple_ddr,   MPC5XXX_GPIO_SIMPLE_PSC3_6 |
					  MPC5XXX_GPIO_SIMPLE_PSC3_7);

	/* Configure PSC3_9 and GPIO_WKUP6,7 as GPIO output */
	setbits_8(&wu_gpio->enable,  MPC5XXX_GPIO_WKUP_6 |
				     MPC5XXX_GPIO_WKUP_7 |
				     MPC5XXX_GPIO_WKUP_PSC3_9);
	setbits_8(&wu_gpio->ddr,     MPC5XXX_GPIO_WKUP_6 |
				     MPC5XXX_GPIO_WKUP_7 |
				     MPC5XXX_GPIO_WKUP_PSC3_9);

	/* Set LR mirror bit because it is low-active */
	setbits_8(&wu_gpio->dvo,     MPC5XXX_GPIO_WKUP_7);

	/* Reset Coral-P graphics controller */
	setbits_8(&wu_gpio->dvo,     MPC5XXX_GPIO_WKUP_PSC3_9);

	/* Enable display backlight */
	clrbits_8(&gpio->sint_inten, MPC5XXX_GPIO_SINT_PSC3_8);
	setbits_8(&gpio->sint_gpioe, MPC5XXX_GPIO_SINT_PSC3_8);
	setbits_8(&gpio->sint_ddr,   MPC5XXX_GPIO_SINT_PSC3_8);
	setbits_8(&gpio->sint_dvo,   MPC5XXX_GPIO_SINT_PSC3_8);

	/*
	 * Configure three wire serial interface to RTC (PSC1_4,
	 * PSC2_4, PSC3_4, PSC3_5)
	 */
	setbits_8(&wu_gpio->enable,  MPC5XXX_GPIO_WKUP_PSC1_4 |
				     MPC5XXX_GPIO_WKUP_PSC2_4);
	setbits_8(&wu_gpio->ddr,     MPC5XXX_GPIO_WKUP_PSC1_4 |
				     MPC5XXX_GPIO_WKUP_PSC2_4);
	clrbits_8(&wu_gpio->dvo,     MPC5XXX_GPIO_WKUP_PSC1_4);
	clrbits_8(&gpio->sint_inten, MPC5XXX_GPIO_SINT_PSC3_4 |
				     MPC5XXX_GPIO_SINT_PSC3_5);
	setbits_8(&gpio->sint_gpioe, MPC5XXX_GPIO_SINT_PSC3_4 |
				     MPC5XXX_GPIO_SINT_PSC3_5);
	setbits_8(&gpio->sint_ddr,   MPC5XXX_GPIO_SINT_PSC3_5);
	clrbits_8(&gpio->sint_dvo,   MPC5XXX_GPIO_SINT_PSC3_5);

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

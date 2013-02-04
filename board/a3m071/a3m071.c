/*
 * (C) Copyright 2003-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004
 * Mark Jonas, Freescale Semiconductor, mark.jonas@motorola.com.
 *
 * (C) Copyright 2006
 * MicroSys GmbH
 *
 * Copyright 2012 Stefan Roese <sr@denx.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#include <common.h>
#include <command.h>
#include <mpc5xxx.h>
#include <pci.h>
#include <miiphy.h>
#include <asm/processor.h>
#include <asm/io.h>

#include "mt46v16m16-75.h"

DECLARE_GLOBAL_DATA_PTR;

#if !defined(CONFIG_SYS_RAMBOOT) && \
	(defined(CONFIG_SPL) && defined(CONFIG_SPL_BUILD))
static void sdram_start(int hi_addr)
{
	long hi_addr_bit = hi_addr ? 0x01000000 : 0;
	long control = SDRAM_CONTROL | hi_addr_bit;

	/* unlock mode register */
	out_be32((void *)MPC5XXX_SDRAM_CTRL, control | 0x80000000);

	/* precharge all banks */
	out_be32((void *)MPC5XXX_SDRAM_CTRL, control | 0x80000002);

#ifdef SDRAM_DDR
	/* set mode register: extended mode */
	out_be32((void *)MPC5XXX_SDRAM_MODE, SDRAM_EMODE);

	/* set mode register: reset DLL */
	out_be32((void *)MPC5XXX_SDRAM_MODE, SDRAM_MODE | 0x04000000);
#endif

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
 * use of CONFIG_SYS_SDRAM_BASE. The code does not work if
 * CONFIG_SYS_SDRAM_BASE is something else than 0x00000000.
 */
phys_size_t initdram(int board_type)
{
	ulong dramsize = 0;
	ulong dramsize2 = 0;
	uint svr, pvr;
#if !defined(CONFIG_SYS_RAMBOOT) && \
	(defined(CONFIG_SPL) && defined(CONFIG_SPL_BUILD))
	ulong test1, test2;

	/* setup SDRAM chip selects */
	out_be32((void *)MPC5XXX_SDRAM_CS0CFG, 0x0000001e);	/* 2GB at 0x0 */
	out_be32((void *)MPC5XXX_SDRAM_CS1CFG, 0x80000000);	/* disabled */

	/* setup config registers */
	out_be32((void *)MPC5XXX_SDRAM_CONFIG1, SDRAM_CONFIG1);
	out_be32((void *)MPC5XXX_SDRAM_CONFIG2, SDRAM_CONFIG2);

#ifdef SDRAM_DDR
	/* set tap delay */
	out_be32((void *)MPC5XXX_CDM_PORCFG, SDRAM_TAPDELAY);
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
	if (dramsize < (1 << 20))
		dramsize = 0;

	/* set SDRAM CS0 size according to the amount of RAM found */
	if (dramsize > 0) {
		out_be32((void *)MPC5XXX_SDRAM_CS0CFG,
			 0x13 + __builtin_ffs(dramsize >> 20) - 1);
	} else {
		out_be32((void *)MPC5XXX_SDRAM_CS0CFG, 0);	/* disabled */
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
	if ((SVR_MJREV(svr) >= 2) && (PVR_MAJ(pvr) == 1) && (PVR_MIN(pvr) == 4))
		out_be32((void *)MPC5XXX_SDRAM_SDELAY, 0x04);

	return dramsize + dramsize2;
}

static void get_revisions(int *failsavelevel, int *digiboardversion,
	int *fpgaversion)
{
	struct mpc5xxx_gpt_0_7 *gpt = (struct mpc5xxx_gpt_0_7 *)MPC5XXX_GPT;
	u8 val;

	/*
	 * Figure out failsavelevel
	 * see ticket dsvk#59
	 */
	*failsavelevel = 0;	/* 0=failsave, 1=board ok, 2=fpga ok */

	/* read digitalboard-version from TMR[2..4] */
	val = 0;
	val |= (gpt->gpt2.sr & (1 << (31 - 23))) ? (1) : 0;
	val |= (gpt->gpt3.sr & (1 << (31 - 23))) ? (1 << 1) : 0;
	val |= (gpt->gpt4.sr & (1 << (31 - 23))) ? (1 << 2) : 0;
	*digiboardversion = val;

	if (*digiboardversion == 0) {
		*failsavelevel = 1;	/* digiboard-version ok */

		/* read fpga-version from TMR[5..7] */
		val = 0;
		val |= (gpt->gpt5.sr & (1 << (31 - 23))) ? (1) : 0;
		val |= (gpt->gpt6.sr & (1 << (31 - 23))) ? (1 << 1) : 0;
		val |= (gpt->gpt7.sr & (1 << (31 - 23))) ? (1 << 2) : 0;
		*fpgaversion = val;

		if (*fpgaversion == 1)
			*failsavelevel = 2;	/* fpga-version ok */
	}
}

/*
 * This function is called from the SPL U-Boot version for
 * early init stuff, that needs to be done for OS (e.g. Linux)
 * booting. Doing it later in the real U-Boot would not work
 * in case that the SPL U-Boot boots Linux directly.
 */
void spl_board_init(void)
{
	struct mpc5xxx_gpio *gpio = (struct mpc5xxx_gpio *)MPC5XXX_GPIO;
	struct mpc5xxx_mmap_ctl *mm =
		(struct mpc5xxx_mmap_ctl *)CONFIG_SYS_MBAR;
	int digiboardversion;
	int failsavelevel;
	int fpgaversion;
	u32 val;

	get_revisions(&failsavelevel, &digiboardversion, &fpgaversion);

	val = in_be32(&mm->ipbi_ws_ctrl);

	/* first clear bits 19..21 (CS3...5) */
	val &= ~((1 << 19) | (1 << 20) | (1 << 21));
	if (failsavelevel == 2) {
		/* FPGA ok */
		val |= (1 << 19) | (1 << 21);
	}

	if (failsavelevel >= 1) {
		/* at least digiboard-version ok */
		val |= (1 << 20);
	}

	/* And write new value back to register */
	out_be32(&mm->ipbi_ws_ctrl, val);

	/*
	 * No need to change the pin multiplexing (MPC5XXX_GPS_PORT_CONFIG)
	 * as all 3 config versions (failsave level) have the same setup.
	 */

	/*
	 * Setup gpio_wkup_7 as watchdog AS INPUT to disable it - see
	 * ticket #60
	 *
	 * MPC5XXX_WU_GPIO_DIR direction is already 0 (INPUT)
	 * set bit 0(msb) to 1
	 */
	setbits_be32((void *)MPC5XXX_WU_GPIO_ENABLE, 1 << (31 - 0));

	/* setup GPIOs for status-leds if needed - see ticket #57 */
	if (failsavelevel > 0) {
		/* digiboard-version is OK */
		/* LED is LOW ACTIVE - so deactivate by set output to 1 */
		gpio->simple_dvo |= 1 << (31 - 12);
		gpio->simple_dvo |= 1 << (31 - 13);
		/* set GPIO direction to output */
		gpio->simple_ddr |= 1 << (31 - 12);
		gpio->simple_ddr |= 1 << (31 - 13);
		/* open drain config is set to "normal output" at reset */
		/* gpio->simple_ode &=~ ( 1 << (31-12) ); */
		/* gpio->simple_ode &=~ ( 1 << (31-13) ); */
		/* enable as GPIO */
		gpio->simple_gpioe |= 1 << (31 - 12);
		gpio->simple_gpioe |= 1 << (31 - 13);
	}

	/* setup fpga irq - see ticket #65 */
	if (failsavelevel > 1) {
		/*
		 * The main irq initialisation is done in interrupts.c
		 * mpc5xxx_init_irq
		 */
		struct mpc5xxx_intr *intr =
		    (struct mpc5xxx_intr *)(MPC5XXX_ICTL);

		setbits_be32(&intr->ctrl, 0x08C01801);

		/*
		 * The MBAR+0x0524 Bit 21:23 CSe are ignored here due to the
		 * already cleared (intr_ctrl) MBAR+0x0510 ECLR[0] bit above
		 */
	}

}

int checkboard(void)
{
	int digiboardversion;
	int failsavelevel;
	int fpgaversion;

	get_revisions(&failsavelevel, &digiboardversion, &fpgaversion);

	puts("Board: A3M071\n");
	printf("Rev:   failsave level       %u\n", failsavelevel);
	printf("       digiboard IO version %u\n", digiboardversion);
	if (failsavelevel > 0)	/* only if fpga-version red */
		printf("       fpga IO version      %u\n", fpgaversion);

	return 0;
}

/* miscellaneous platform dependent initialisations */
int misc_init_r(void)
{
	/* adjust flash start and offset to detected values */
	gd->bd->bi_flashstart = flash_info[0].start[0];
	gd->bd->bi_flashoffset = 0;

	/* adjust mapping */
	out_be32((void *)MPC5XXX_BOOTCS_START,
		 START_REG(gd->bd->bi_flashstart));
	out_be32((void *)MPC5XXX_CS0_START, START_REG(gd->bd->bi_flashstart));
	out_be32((void *)MPC5XXX_BOOTCS_STOP,
		 STOP_REG(gd->bd->bi_flashstart, gd->bd->bi_flashsize));
	out_be32((void *)MPC5XXX_CS0_STOP,
		 STOP_REG(gd->bd->bi_flashstart, gd->bd->bi_flashsize));

	return 0;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t * bd)
{
	ft_cpu_setup(blob, bd);
}
#endif /* defined(CONFIG_OF_FLAT_TREE) && defined(CONFIG_OF_BOARD_SETUP) */

#ifdef CONFIG_SPL_OS_BOOT
/*
 * A3M071 specific implementation of spl_start_uboot()
 *
 * RETURN
 * 0 if booting into OS is selected (default)
 * 1 if booting into U-Boot is selected
 */
int spl_start_uboot(void)
{
	char s[8];

	env_init();
	getenv_f("boot_os", s, sizeof(s));
	if ((s != NULL) && (strcmp(s, "yes") == 0))
		return 0;

	return 1;
}
#endif

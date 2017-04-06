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
 * Copyright 2012-2013 Stefan Roese <sr@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <mpc5xxx.h>
#include <pci.h>
#include <miiphy.h>
#include <linux/compiler.h>
#include <asm/processor.h>
#include <asm/io.h>

#ifdef CONFIG_A4M2K
#include "is46r16320d.h"
#else
#include "mt46v16m16-75.h"
#endif

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

	/*
	 * Wait a short while for the DLL to lock before accessing
	 * the SDRAM
	 */
	udelay(100);
}
#endif

/*
 * ATTENTION: Although partially referenced dram_init does NOT make real use
 * use of CONFIG_SYS_SDRAM_BASE. The code does not work if
 * CONFIG_SYS_SDRAM_BASE is something else than 0x00000000.
 */
int dram_init(void)
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

	gd->ram_size = dramsize + dramsize2;

	return 0;
}

static void get_revisions(int *failsavelevel, int *digiboardversion,
	int *fpgaversion)
{
	struct mpc5xxx_gpt_0_7 *gpt = (struct mpc5xxx_gpt_0_7 *)MPC5XXX_GPT;
	u8 val;

	/* read digitalboard-version from TMR[2..4] */
	val = 0;
	val |= (gpt->gpt2.sr & (1 << (31 - 23))) ? (1) : 0;
	val |= (gpt->gpt3.sr & (1 << (31 - 23))) ? (1 << 1) : 0;
	val |= (gpt->gpt4.sr & (1 << (31 - 23))) ? (1 << 2) : 0;
	*digiboardversion = val;

	/*
	 * A4M2K only supports digiboardversion. No failsavelevel and
	 * fpgaversion here.
	 */
#if !defined(CONFIG_A4M2K)
	/*
	 * Figure out failsavelevel
	 * see ticket dsvk#59
	 */
	*failsavelevel = 0;	/* 0=failsave, 1=board ok, 2=fpga ok */

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
#endif
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

#if defined(CONFIG_A4M2K)
	/* enable CS3 and CS5 (FPGA) */
	setbits_be32(&mm->ipbi_ws_ctrl, (1 << 19) | (1 << 21));
#else
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


	/* Setup pin multiplexing */
	if (failsavelevel == 2) {
		/* fpga-version ok */
#if defined(CONFIG_SYS_GPS_PORT_CONFIG_2)
		out_be32(&gpio->port_config, CONFIG_SYS_GPS_PORT_CONFIG_2);
#endif
	} else if (failsavelevel == 1) {
		/* digiboard-version ok - fpga not */
#if defined(CONFIG_SYS_GPS_PORT_CONFIG_1)
		out_be32(&gpio->port_config, CONFIG_SYS_GPS_PORT_CONFIG_1);
#endif
	} else {
		/* full failsave-mode */
#if defined(CONFIG_SYS_GPS_PORT_CONFIG)
		out_be32(&gpio->port_config, CONFIG_SYS_GPS_PORT_CONFIG);
#endif
	}
#endif

	/*
	 * Setup gpio_wkup_7 as watchdog AS INPUT to disable it - see
	 * ticket #60
	 *
	 * MPC5XXX_WU_GPIO_DIR direction is already 0 (INPUT)
	 * set bit 0(msb) to 1
	 */
	setbits_be32((void *)MPC5XXX_WU_GPIO_ENABLE, CONFIG_WDOG_GPIO_PIN);

#if defined(CONFIG_A4M2K)
	/* Setup USB[x] as MPCDiag[0..3] GPIO outputs */

	/* set USB0,6,7,8 (MPCDiag[0..3]) direction to output */
	gpio->simple_ddr |= 1 << (31 - 15);
	gpio->simple_ddr |= 1 << (31 - 14);
	gpio->simple_ddr |= 1 << (31 - 13);
	gpio->simple_ddr |= 1 << (31 - 12);

	/* enable USB0,6,7,8 (MPCDiag[0..3]) as GPIO */
	gpio->simple_gpioe |= 1 << (31 - 15);
	gpio->simple_gpioe |= 1 << (31 - 14);
	gpio->simple_gpioe |= 1 << (31 - 13);
	gpio->simple_gpioe |= 1 << (31 - 12);

	/* Setup PSC2[0..2] as STSLED[0..2] GPIO outputs */

	/* set PSC2[0..2] (STSLED[0..2]) direction to output */
	gpio->simple_ddr |= 1 << (31 - 27);
	gpio->simple_ddr |= 1 << (31 - 26);
	gpio->simple_ddr |= 1 << (31 - 25);

	/* enable PSC2[0..2] (STSLED[0..2]) as GPIO */
	gpio->simple_gpioe |= 1 << (31 - 27);
	gpio->simple_gpioe |= 1 << (31 - 26);
	gpio->simple_gpioe |= 1 << (31 - 25);

	/* Setup PSC6[2] as MRST2 self reset GPIO output */

	/* set PSC6[2]/IRDA_TX (MRST2) direction to output */
	gpio->simple_ddr |= 1 << (31 - 3);

	/* set PSC6[2]/IRDA_TX (MRST2) output as open drain */
	gpio->simple_ode |= 1 << (31 - 3);

	/* set PSC6[2]/IRDA_TX (MRST2) output as default high */
	gpio->simple_dvo |= 1 << (31 - 3);

	/* enable PSC6[2]/IRDA_TX (MRST2) as GPIO */
	gpio->simple_gpioe |= 1 << (31 - 3);

	/* Setup PSC6[3] as HARNSSCD harness code GPIO input */

	/* set PSC6[3]/IR_USB_CLK (HARNSSCD) direction to input */
	gpio->simple_ddr |= 0 << (31 - 2);

	/* enable PSC6[3]/IR_USB_CLK (HARNSSCD) as GPIO */
	gpio->simple_gpioe |= 1 << (31 - 2);
#else
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
#endif
}

int checkboard(void)
{
	int digiboardversion;
	int failsavelevel;
	int fpgaversion;

	get_revisions(&failsavelevel, &digiboardversion, &fpgaversion);

#ifdef CONFIG_A4M2K
	puts("Board: A4M2K\n");
	printf("       digiboard IO version %u\n", digiboardversion);
#else
	puts("Board: A3M071\n");
	printf("Rev:   failsave level       %u\n", failsavelevel);
	printf("       digiboard IO version %u\n", digiboardversion);
	if (failsavelevel > 0)	/* only if fpga-version red */
		printf("       fpga IO version      %u\n", fpgaversion);
#endif

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

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);

	return 0;
}
#endif /* CONFIG_OF_BOARD_SETUP */

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
	if ((s != NULL) && (*s == '1' || *s == 'y' || *s == 'Y' ||
			    *s == 't' || *s == 'T'))
		return 0;

	return 1;
}
#endif

#if defined(CONFIG_HW_WATCHDOG)
static int watchdog_toggle;

void hw_watchdog_reset(void)
{
	int val;

	/*
	 * Check if watchdog is enabled via user command
	 */
	if ((gd->flags & GD_FLG_RELOC) && watchdog_toggle) {
		/* Set direction to output */
		setbits_be32((void *)MPC5XXX_WU_GPIO_DIR, CONFIG_WDOG_GPIO_PIN);

		/*
		 * Toggle watchdog output
		 */
		val = (in_be32((void *)MPC5XXX_WU_GPIO_DATA_O) &
		       CONFIG_WDOG_GPIO_PIN);
		if (val) {
			clrbits_be32((void *)MPC5XXX_WU_GPIO_DATA_O,
				     CONFIG_WDOG_GPIO_PIN);
		} else {
			setbits_be32((void *)MPC5XXX_WU_GPIO_DATA_O,
				     CONFIG_WDOG_GPIO_PIN);
		}
	}
}

int do_wdog_toggle(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc != 2)
		goto usage;

	if (strncmp(argv[1], "on", 2) == 0)
		watchdog_toggle = 1;
	else if (strncmp(argv[1], "off", 3) == 0)
		watchdog_toggle = 0;
	else
		goto usage;

	return 0;
usage:
	printf("Usage: wdogtoggle %s\n", cmdtp->usage);
	return 1;
}

U_BOOT_CMD(
	wdogtoggle, CONFIG_SYS_MAXARGS, 2, do_wdog_toggle,
	"toggle GPIO pin to service watchdog",
	"[on/off] - Switch watchdog toggling via GPIO pin on/off"
);
#endif

/*
 * (C) Copyright 2000-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mpc5xxx.h>
#include <asm/io.h>
#include <watchdog.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Breath some life into the CPU...
 *
 * Set up the memory map,
 * initialize a bunch of registers.
 */
void cpu_init_f (void)
{
	volatile struct mpc5xxx_mmap_ctl *mm =
		(struct mpc5xxx_mmap_ctl *) CONFIG_SYS_MBAR;
	volatile struct mpc5xxx_lpb *lpb =
		(struct mpc5xxx_lpb *) MPC5XXX_LPB;
	volatile struct mpc5xxx_gpio *gpio =
		(struct mpc5xxx_gpio *) MPC5XXX_GPIO;
	volatile struct mpc5xxx_xlb *xlb =
		(struct mpc5xxx_xlb *) MPC5XXX_XLBARB;
#if defined(CONFIG_SYS_IPBCLK_EQUALS_XLBCLK)
	volatile struct mpc5xxx_cdm *cdm =
		(struct mpc5xxx_cdm *) MPC5XXX_CDM;
#endif	/* CONFIG_SYS_IPBCLK_EQUALS_XLBCLK */
#if defined(CONFIG_WATCHDOG)
	volatile struct mpc5xxx_gpt *gpt0 =
		(struct mpc5xxx_gpt *) MPC5XXX_GPT;
#endif /* CONFIG_WATCHDOG */
	unsigned long addecr = (1 << 25); /* Boot_CS */
	/* Pointer is writable since we allocated a register for it */
	gd = (gd_t *) (CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_GBL_DATA_OFFSET);

	/* Clear initial global data */
	memset ((void *) gd, 0, sizeof (gd_t));

	/*
	 * Memory Controller: configure chip selects and enable them
	 */
#if defined(CONFIG_SYS_BOOTCS_START) && defined(CONFIG_SYS_BOOTCS_SIZE)
	out_be32(&mm->boot_start, START_REG(CONFIG_SYS_BOOTCS_START));
	out_be32(&mm->boot_stop, STOP_REG(CONFIG_SYS_BOOTCS_START,
					  CONFIG_SYS_BOOTCS_SIZE));
#endif
#if defined(CONFIG_SYS_BOOTCS_CFG)
	out_be32(&lpb->cs0_cfg, CONFIG_SYS_BOOTCS_CFG);
#endif

#if defined(CONFIG_SYS_CS0_START) && defined(CONFIG_SYS_CS0_SIZE)
	out_be32(&mm->cs0_start, START_REG(CONFIG_SYS_CS0_START));
	out_be32(&mm->cs0_stop, STOP_REG(CONFIG_SYS_CS0_START,
					 CONFIG_SYS_CS0_SIZE));
	/* CS0 and BOOT_CS cannot be enabled at once. */
	/*	addecr |= (1 << 16); */
#endif
#if defined(CONFIG_SYS_CS0_CFG)
	out_be32(&lpb->cs0_cfg, CONFIG_SYS_CS0_CFG);
#endif

#if defined(CONFIG_SYS_CS1_START) && defined(CONFIG_SYS_CS1_SIZE)
	out_be32(&mm->cs1_start, START_REG(CONFIG_SYS_CS1_START));
	out_be32(&mm->cs1_stop, STOP_REG(CONFIG_SYS_CS1_START,
					 CONFIG_SYS_CS1_SIZE));
	addecr |= (1 << 17);
#endif
#if defined(CONFIG_SYS_CS1_CFG)
	out_be32(&lpb->cs1_cfg, CONFIG_SYS_CS1_CFG);
#endif

#if defined(CONFIG_SYS_CS2_START) && defined(CONFIG_SYS_CS2_SIZE)
	out_be32(&mm->cs2_start, START_REG(CONFIG_SYS_CS2_START));
	out_be32(&mm->cs2_stop, STOP_REG(CONFIG_SYS_CS2_START,
					 CONFIG_SYS_CS2_SIZE));
	addecr |= (1 << 18);
#endif
#if defined(CONFIG_SYS_CS2_CFG)
	out_be32(&lpb->cs2_cfg, CONFIG_SYS_CS2_CFG);
#endif

#if defined(CONFIG_SYS_CS3_START) && defined(CONFIG_SYS_CS3_SIZE)
	out_be32(&mm->cs3_start, START_REG(CONFIG_SYS_CS3_START));
	out_be32(&mm->cs3_stop, STOP_REG(CONFIG_SYS_CS3_START,
					 CONFIG_SYS_CS3_SIZE));
	addecr |= (1 << 19);
#endif
#if defined(CONFIG_SYS_CS3_CFG)
	out_be32(&lpb->cs3_cfg, CONFIG_SYS_CS3_CFG);
#endif

#if defined(CONFIG_SYS_CS4_START) && defined(CONFIG_SYS_CS4_SIZE)
	out_be32(&mm->cs4_start, START_REG(CONFIG_SYS_CS4_START));
	out_be32(&mm->cs4_stop, STOP_REG(CONFIG_SYS_CS4_START,
					  CONFIG_SYS_CS4_SIZE));
	addecr |= (1 << 20);
#endif
#if defined(CONFIG_SYS_CS4_CFG)
	out_be32(&lpb->cs4_cfg, CONFIG_SYS_CS4_CFG);
#endif

#if defined(CONFIG_SYS_CS5_START) && defined(CONFIG_SYS_CS5_SIZE)
	out_be32(&mm->cs5_start, START_REG(CONFIG_SYS_CS5_START));
	out_be32(&mm->cs5_stop, STOP_REG(CONFIG_SYS_CS5_START,
					  CONFIG_SYS_CS5_SIZE));
	addecr |= (1 << 21);
#endif
#if defined(CONFIG_SYS_CS5_CFG)
	out_be32(&lpb->cs5_cfg, CONFIG_SYS_CS5_CFG);
#endif

	addecr |= 1;
#if defined(CONFIG_SYS_CS6_START) && defined(CONFIG_SYS_CS6_SIZE)
	out_be32(&mm->cs6_start, START_REG(CONFIG_SYS_CS6_START));
	out_be32(&mm->cs6_stop, STOP_REG(CONFIG_SYS_CS6_START,
					  CONFIG_SYS_CS6_SIZE));
	addecr |= (1 << 26);
#endif
#if defined(CONFIG_SYS_CS6_CFG)
	out_be32(&lpb->cs6_cfg, CONFIG_SYS_CS6_CFG);
#endif

#if defined(CONFIG_SYS_CS7_START) && defined(CONFIG_SYS_CS7_SIZE)
	out_be32(&mm->cs7_start, START_REG(CONFIG_SYS_CS7_START));
	out_be32(&mm->cs7_stop, STOP_REG(CONFIG_SYS_CS7_START,
					  CONFIG_SYS_CS7_SIZE));
	addecr |= (1 << 27);
#endif
#if defined(CONFIG_SYS_CS7_CFG)
	out_be32(&lpb->cs7_cfg, CONFIG_SYS_CS7_CFG);
#endif

#if defined(CONFIG_SYS_CS_BURST)
	out_be32(&lpb->cs_burst, CONFIG_SYS_CS_BURST);
#endif
#if defined(CONFIG_SYS_CS_DEADCYCLE)
	out_be32(&lpb->cs_deadcycle, CONFIG_SYS_CS_DEADCYCLE);
#endif

	/* Enable chip selects */
	out_be32(&mm->ipbi_ws_ctrl, addecr);
	out_be32(&lpb->cs_ctrl, (1 << 24));

	/* Setup pin multiplexing */
#if defined(CONFIG_SYS_GPS_PORT_CONFIG)
	out_be32(&gpio->port_config, CONFIG_SYS_GPS_PORT_CONFIG);
#endif

	/* Setup gpios */
#if defined(CONFIG_SYS_GPIO_DATADIR)
	out_be32(&gpio->simple_ddr, CONFIG_SYS_GPIO_DATADIR);
#endif
#if defined(CONFIG_SYS_GPIO_OPENDRAIN)
	out_be32(&gpio->simple_ode, CONFIG_SYS_GPIO_OPENDRAIN);
#endif
#if defined(CONFIG_SYS_GPIO_DATAVALUE)
	out_be32(&gpio->simple_dvo, CONFIG_SYS_GPIO_DATAVALUE);
#endif
#if defined(CONFIG_SYS_GPIO_ENABLE)
	out_be32(&gpio->simple_gpioe, CONFIG_SYS_GPIO_ENABLE);
#endif

	/* enable timebase */
	setbits_be32(&xlb->config, (1 << 13));

	/* Enable snooping for RAM */
	setbits_be32(&xlb->config, (1 << 15));
	out_be32(&xlb->snoop_window, CONFIG_SYS_SDRAM_BASE | 0x1d);

#if defined(CONFIG_SYS_IPBCLK_EQUALS_XLBCLK)
	/* Motorola reports IPB should better run at 133 MHz. */
	setbits_be32(&mm->ipbi_ws_ctrl, 1);
	/* pci_clk_sel = 0x02, ipb_clk_sel = 0x00; */
	addecr = in_be32(&cdm->cfg);
	addecr &= ~0x103;
# if defined(CONFIG_SYS_PCICLK_EQUALS_IPBCLK_DIV2)
	/* pci_clk_sel = 0x01 -> IPB_CLK/2 */
	addecr |= 0x01;
# else
	/* pci_clk_sel = 0x02 -> XLB_CLK/4 = IPB_CLK/4 */
	addecr |= 0x02;
# endif /* CONFIG_SYS_PCICLK_EQUALS_IPBCLK_DIV2 */
	out_be32(&cdm->cfg, addecr);
#endif	/* CONFIG_SYS_IPBCLK_EQUALS_XLBCLK */
	/* Configure the XLB Arbiter */
	out_be32(&xlb->master_pri_enable, 0xff);
	out_be32(&xlb->master_priority, 0x11111111);

#if defined(CONFIG_SYS_XLB_PIPELINING)
	/* Enable piplining */
	clrbits_be32(&xlb->config, (1 << 31));
#endif

#if defined(CONFIG_WATCHDOG)
	/* Charge the watchdog timer - prescaler = 64k, count = 64k*/
	out_be32(&gpt0->cir, 0x0000ffff);
	out_be32(&gpt0->emsr, 0x9004);	/* wden|ce|timer_ms */

	reset_5xxx_watchdog();
#endif /* CONFIG_WATCHDOG */
}

/*
 * initialize higher level parts of CPU like time base and timers
 */
int cpu_init_r (void)
{
	volatile struct mpc5xxx_intr *intr =
		(struct mpc5xxx_intr *) MPC5XXX_ICTL;

	/* mask all interrupts */
	out_be32(&intr->per_mask, 0xffffff00);
	setbits_be32(&intr->main_mask, 0x0001ffff);
	clrbits_be32(&intr->ctrl, 0x00000f00);
	/* route critical ints to normal ints */
	setbits_be32(&intr->ctrl, 0x00000001);

#if defined(CONFIG_CMD_NET) && defined(CONFIG_MPC5xxx_FEC)
	/* load FEC microcode */
	loadtask(0, 2);
#endif

	return (0);
}

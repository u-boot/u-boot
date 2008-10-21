/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

DECLARE_GLOBAL_DATA_PTR;

/*
 * Breath some life into the CPU...
 *
 * Set up the memory map,
 * initialize a bunch of registers.
 */
void cpu_init_f (void)
{
	unsigned long addecr = (1 << 25); /* Boot_CS */
#if defined(CONFIG_SYS_RAMBOOT) && defined(CONFIG_MGT5100)
	addecr |= (1 << 22); /* SDRAM enable */
#endif
	/* Pointer is writable since we allocated a register for it */
	gd = (gd_t *) (CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_GBL_DATA_OFFSET);

	/* Clear initial global data */
	memset ((void *) gd, 0, sizeof (gd_t));

	/*
	 * Memory Controller: configure chip selects and enable them
	 */
#if defined(CONFIG_SYS_BOOTCS_START) && defined(CONFIG_SYS_BOOTCS_SIZE)
	*(vu_long *)MPC5XXX_BOOTCS_START = START_REG(CONFIG_SYS_BOOTCS_START);
	*(vu_long *)MPC5XXX_BOOTCS_STOP = STOP_REG(CONFIG_SYS_BOOTCS_START,
			CONFIG_SYS_BOOTCS_SIZE);
#endif
#if defined(CONFIG_SYS_BOOTCS_CFG)
	*(vu_long *)MPC5XXX_BOOTCS_CFG = CONFIG_SYS_BOOTCS_CFG;
#endif

#if defined(CONFIG_SYS_CS0_START) && defined(CONFIG_SYS_CS0_SIZE)
	*(vu_long *)MPC5XXX_CS0_START = START_REG(CONFIG_SYS_CS0_START);
	*(vu_long *)MPC5XXX_CS0_STOP = STOP_REG(CONFIG_SYS_CS0_START, CONFIG_SYS_CS0_SIZE);
	/* CS0 and BOOT_CS cannot be enabled at once. */
	/*	addecr |= (1 << 16); */
#endif
#if defined(CONFIG_SYS_CS0_CFG)
	*(vu_long *)MPC5XXX_CS0_CFG = CONFIG_SYS_CS0_CFG;
#endif

#if defined(CONFIG_SYS_CS1_START) && defined(CONFIG_SYS_CS1_SIZE)
	*(vu_long *)MPC5XXX_CS1_START = START_REG(CONFIG_SYS_CS1_START);
	*(vu_long *)MPC5XXX_CS1_STOP = STOP_REG(CONFIG_SYS_CS1_START, CONFIG_SYS_CS1_SIZE);
	addecr |= (1 << 17);
#endif
#if defined(CONFIG_SYS_CS1_CFG)
	*(vu_long *)MPC5XXX_CS1_CFG = CONFIG_SYS_CS1_CFG;
#endif

#if defined(CONFIG_SYS_CS2_START) && defined(CONFIG_SYS_CS2_SIZE)
	*(vu_long *)MPC5XXX_CS2_START = START_REG(CONFIG_SYS_CS2_START);
	*(vu_long *)MPC5XXX_CS2_STOP = STOP_REG(CONFIG_SYS_CS2_START, CONFIG_SYS_CS2_SIZE);
	addecr |= (1 << 18);
#endif
#if defined(CONFIG_SYS_CS2_CFG)
	*(vu_long *)MPC5XXX_CS2_CFG = CONFIG_SYS_CS2_CFG;
#endif

#if defined(CONFIG_SYS_CS3_START) && defined(CONFIG_SYS_CS3_SIZE)
	*(vu_long *)MPC5XXX_CS3_START = START_REG(CONFIG_SYS_CS3_START);
	*(vu_long *)MPC5XXX_CS3_STOP = STOP_REG(CONFIG_SYS_CS3_START, CONFIG_SYS_CS3_SIZE);
	addecr |= (1 << 19);
#endif
#if defined(CONFIG_SYS_CS3_CFG)
	*(vu_long *)MPC5XXX_CS3_CFG = CONFIG_SYS_CS3_CFG;
#endif

#if defined(CONFIG_SYS_CS4_START) && defined(CONFIG_SYS_CS4_SIZE)
	*(vu_long *)MPC5XXX_CS4_START = START_REG(CONFIG_SYS_CS4_START);
	*(vu_long *)MPC5XXX_CS4_STOP = STOP_REG(CONFIG_SYS_CS4_START, CONFIG_SYS_CS4_SIZE);
	addecr |= (1 << 20);
#endif
#if defined(CONFIG_SYS_CS4_CFG)
	*(vu_long *)MPC5XXX_CS4_CFG = CONFIG_SYS_CS4_CFG;
#endif

#if defined(CONFIG_SYS_CS5_START) && defined(CONFIG_SYS_CS5_SIZE)
	*(vu_long *)MPC5XXX_CS5_START = START_REG(CONFIG_SYS_CS5_START);
	*(vu_long *)MPC5XXX_CS5_STOP = STOP_REG(CONFIG_SYS_CS5_START, CONFIG_SYS_CS5_SIZE);
	addecr |= (1 << 21);
#endif
#if defined(CONFIG_SYS_CS5_CFG)
	*(vu_long *)MPC5XXX_CS5_CFG = CONFIG_SYS_CS5_CFG;
#endif

#if defined(CONFIG_MPC5200)
	addecr |= 1;
#if defined(CONFIG_SYS_CS6_START) && defined(CONFIG_SYS_CS6_SIZE)
	*(vu_long *)MPC5XXX_CS6_START = START_REG(CONFIG_SYS_CS6_START);
	*(vu_long *)MPC5XXX_CS6_STOP = STOP_REG(CONFIG_SYS_CS6_START, CONFIG_SYS_CS6_SIZE);
	addecr |= (1 << 26);
#endif
#if defined(CONFIG_SYS_CS6_CFG)
	*(vu_long *)MPC5XXX_CS6_CFG = CONFIG_SYS_CS6_CFG;
#endif

#if defined(CONFIG_SYS_CS7_START) && defined(CONFIG_SYS_CS7_SIZE)
	*(vu_long *)MPC5XXX_CS7_START = START_REG(CONFIG_SYS_CS7_START);
	*(vu_long *)MPC5XXX_CS7_STOP = STOP_REG(CONFIG_SYS_CS7_START, CONFIG_SYS_CS7_SIZE);
	addecr |= (1 << 27);
#endif
#if defined(CONFIG_SYS_CS7_CFG)
	*(vu_long *)MPC5XXX_CS7_CFG = CONFIG_SYS_CS7_CFG;
#endif

#if defined(CONFIG_SYS_CS_BURST)
	*(vu_long *)MPC5XXX_CS_BURST = CONFIG_SYS_CS_BURST;
#endif
#if defined(CONFIG_SYS_CS_DEADCYCLE)
	*(vu_long *)MPC5XXX_CS_DEADCYCLE = CONFIG_SYS_CS_DEADCYCLE;
#endif
#endif /* CONFIG_MPC5200 */

	/* Enable chip selects */
	*(vu_long *)MPC5XXX_ADDECR = addecr;
	*(vu_long *)MPC5XXX_CS_CTRL = (1 << 24);

	/* Setup pin multiplexing */
#if defined(CONFIG_SYS_GPS_PORT_CONFIG)
	*(vu_long *)MPC5XXX_GPS_PORT_CONFIG = CONFIG_SYS_GPS_PORT_CONFIG;
#endif

#if defined(CONFIG_MPC5200)
	/* enable timebase */
	*(vu_long *)(MPC5XXX_XLBARB + 0x40) |= (1 << 13);

	/* Enable snooping for RAM */
	*(vu_long *)(MPC5XXX_XLBARB + 0x40) |= (1 << 15);
	*(vu_long *)(MPC5XXX_XLBARB + 0x70) = CONFIG_SYS_SDRAM_BASE | 0x1d;

# if defined(CONFIG_SYS_IPBCLK_EQUALS_XLBCLK)
	/* Motorola reports IPB should better run at 133 MHz. */
	*(vu_long *)MPC5XXX_ADDECR |= 1;
	/* pci_clk_sel = 0x02, ipb_clk_sel = 0x00; */
	addecr = *(vu_long *)MPC5XXX_CDM_CFG;
	addecr &= ~0x103;
#  if defined(CONFIG_SYS_PCICLK_EQUALS_IPBCLK_DIV2)
	/* pci_clk_sel = 0x01 -> IPB_CLK/2 */
	addecr |= 0x01;
#  else
	/* pci_clk_sel = 0x02 -> XLB_CLK/4 = IPB_CLK/4 */
	addecr |= 0x02;
#  endif /* CONFIG_SYS_PCICLK_EQUALS_IPBCLK_DIV2 */
	*(vu_long *)MPC5XXX_CDM_CFG = addecr;
# endif	/* CONFIG_SYS_IPBCLK_EQUALS_XLBCLK */
	/* Configure the XLB Arbiter */
	*(vu_long *)MPC5XXX_XLBARB_MPRIEN = 0xff;
	*(vu_long *)MPC5XXX_XLBARB_MPRIVAL = 0x11111111;

# if defined(CONFIG_SYS_XLB_PIPELINING)
	/* Enable piplining */
	*(vu_long *)(MPC5XXX_XLBARB + 0x40) &= ~(1 << 31);
# endif
#endif	/* CONFIG_MPC5200 */
}

/*
 * initialize higher level parts of CPU like time base and timers
 */
int cpu_init_r (void)
{
	/* mask all interrupts */
#if defined(CONFIG_MGT5100)
	*(vu_long *)MPC5XXX_ICTL_PER_MASK = 0xfffffc00;
#elif defined(CONFIG_MPC5200)
	*(vu_long *)MPC5XXX_ICTL_PER_MASK = 0xffffff00;
#endif
	*(vu_long *)MPC5XXX_ICTL_CRIT |= 0x0001ffff;
	*(vu_long *)MPC5XXX_ICTL_EXT &= ~0x00000f00;
	/* route critical ints to normal ints */
	*(vu_long *)MPC5XXX_ICTL_EXT |= 0x00000001;

#if defined(CONFIG_CMD_NET) && defined(CONFIG_MPC5xxx_FEC)
	/* load FEC microcode */
	loadtask(0, 2);
#endif

	return (0);
}

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

/*
 * Breath some life into the CPU...
 *
 * Set up the memory map,
 * initialize a bunch of registers.
 */
void cpu_init_f (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	unsigned long addecr = (1 << 25); /* Boot_CS */
#if defined(CFG_RAMBOOT) && defined(CONFIG_MGT5100)
	addecr |= (1 << 22); /* SDRAM enable */
#endif
	/* Pointer is writable since we allocated a register for it */
	gd = (gd_t *) (CFG_INIT_RAM_ADDR + CFG_GBL_DATA_OFFSET);

	/* Clear initial global data */
	memset ((void *) gd, 0, sizeof (gd_t));

	/*
	 * Memory Controller: configure chip selects and enable them
	 */
#if defined(CFG_BOOTCS_START) && defined(CFG_BOOTCS_SIZE)
	*(vu_long *)MPC5XXX_BOOTCS_START = START_REG(CFG_BOOTCS_START);
	*(vu_long *)MPC5XXX_BOOTCS_STOP = STOP_REG(CFG_BOOTCS_START,
			CFG_BOOTCS_SIZE);
#endif
#if defined(CFG_BOOTCS_CFG)
	*(vu_long *)MPC5XXX_BOOTCS_CFG = CFG_BOOTCS_CFG;
#endif

#if defined(CFG_CS0_START) && defined(CFG_CS0_SIZE)
	*(vu_long *)MPC5XXX_CS0_START = START_REG(CFG_CS0_START);
	*(vu_long *)MPC5XXX_CS0_STOP = STOP_REG(CFG_CS0_START, CFG_CS0_SIZE);
	/* CS0 and BOOT_CS cannot be enabled at once. */
	/*	addecr |= (1 << 16); */
#endif
#if defined(CFG_CS0_CFG)
	*(vu_long *)MPC5XXX_CS0_CFG = CFG_CS0_CFG;
#endif

#if defined(CFG_CS1_START) && defined(CFG_CS1_SIZE)
	*(vu_long *)MPC5XXX_CS1_START = START_REG(CFG_CS1_START);
	*(vu_long *)MPC5XXX_CS1_STOP = STOP_REG(CFG_CS1_START, CFG_CS1_SIZE);
	addecr |= (1 << 17);
#endif
#if defined(CFG_CS1_CFG)
	*(vu_long *)MPC5XXX_CS1_CFG = CFG_CS1_CFG;
#endif

#if defined(CFG_CS2_START) && defined(CFG_CS2_SIZE)
	*(vu_long *)MPC5XXX_CS2_START = START_REG(CFG_CS2_START);
	*(vu_long *)MPC5XXX_CS2_STOP = STOP_REG(CFG_CS2_START, CFG_CS2_SIZE);
	addecr |= (1 << 18);
#endif
#if defined(CFG_CS2_CFG)
	*(vu_long *)MPC5XXX_CS2_CFG = CFG_CS2_CFG;
#endif

#if defined(CFG_CS3_START) && defined(CFG_CS3_SIZE)
	*(vu_long *)MPC5XXX_CS3_START = START_REG(CFG_CS3_START);
	*(vu_long *)MPC5XXX_CS3_STOP = STOP_REG(CFG_CS3_START, CFG_CS3_SIZE);
	addecr |= (1 << 19);
#endif
#if defined(CFG_CS3_CFG)
	*(vu_long *)MPC5XXX_CS3_CFG = CFG_CS3_CFG;
#endif

#if defined(CFG_CS4_START) && defined(CFG_CS4_SIZE)
	*(vu_long *)MPC5XXX_CS4_START = START_REG(CFG_CS4_START);
	*(vu_long *)MPC5XXX_CS4_STOP = STOP_REG(CFG_CS4_START, CFG_CS4_SIZE);
	addecr |= (1 << 20);
#endif
#if defined(CFG_CS4_CFG)
	*(vu_long *)MPC5XXX_CS4_CFG = CFG_CS4_CFG;
#endif

#if defined(CFG_CS5_START) && defined(CFG_CS5_SIZE)
	*(vu_long *)MPC5XXX_CS5_START = START_REG(CFG_CS5_START);
	*(vu_long *)MPC5XXX_CS5_STOP = STOP_REG(CFG_CS5_START, CFG_CS5_SIZE);
	addecr |= (1 << 21);
#endif
#if defined(CFG_CS5_CFG)
	*(vu_long *)MPC5XXX_CS5_CFG = CFG_CS5_CFG;
#endif

#if defined(CONFIG_MPC5200)
	addecr |= 1;
#if defined(CFG_CS6_START) && defined(CFG_CS6_SIZE)
	*(vu_long *)MPC5XXX_CS6_START = START_REG(CFG_CS6_START);
	*(vu_long *)MPC5XXX_CS6_STOP = STOP_REG(CFG_CS6_START, CFG_CS6_SIZE);
	addecr |= (1 << 26);
#endif
#if defined(CFG_CS6_CFG)
	*(vu_long *)MPC5XXX_CS6_CFG = CFG_CS6_CFG;
#endif

#if defined(CFG_CS7_START) && defined(CFG_CS7_SIZE)
	*(vu_long *)MPC5XXX_CS7_START = START_REG(CFG_CS5_START);
	*(vu_long *)MPC5XXX_CS7_STOP = STOP_REG(CFG_CS7_START, CFG_CS7_SIZE);
	addecr |= (1 << 27);
#endif
#if defined(CFG_CS7_CFG)
	*(vu_long *)MPC5XXX_CS7_CFG = CFG_CS7_CFG;
#endif

#if defined(CFG_CS_BURST)
	*(vu_long *)MPC5XXX_CS_BURST = CFG_CS_BURST;
#endif
#if defined(CFG_CS_DEADCYCLE)
	*(vu_long *)MPC5XXX_CS_DEADCYCLE = CFG_CS_DEADCYCLE;
#endif
#endif /* CONFIG_MPC5200 */

	/* Enable chip selects */
	*(vu_long *)MPC5XXX_ADDECR = addecr;
	*(vu_long *)MPC5XXX_CS_CTRL = (1 << 24);

	/* Setup pin multiplexing */
#if defined(CFG_GPS_PORT_CONFIG)
	*(vu_long *)MPC5XXX_GPS_PORT_CONFIG = CFG_GPS_PORT_CONFIG;
#endif

#if defined(CONFIG_MPC5200)
	/* enable timebase */
	*(vu_long *)(MPC5XXX_XLBARB + 0x40) |= (1 << 13);

# if defined(CFG_IPBSPEED_133)
	/* Motorola reports IPB should better run at 133 MHz. */
	*(vu_long *)MPC5XXX_ADDECR |= 1;
	/* pci_clk_sel = 0x02, ipb_clk_sel = 0x00; */
	addecr = *(vu_long *)MPC5XXX_CDM_CFG;
	addecr &= ~0x103;
#  if defined(CFG_PCISPEED_66)
	/* pci_clk_sel = 0x01 -> IPB_CLK/2 */
	addecr |= 0x01;
#  else
	/* pci_clk_sel = 0x02 -> XLB_CLK/4 = IPB_CLK/4 */
	addecr |= 0x02;
#  endif /* CFG_PCISPEED_66 */
	*(vu_long *)MPC5XXX_CDM_CFG = addecr;
# endif	/* CFG_IPBSPEED_133 */
	/* Configure the XLB Arbiter */
	*(vu_long *)MPC5XXX_XLBARB_MPRIEN = 0xff;
	*(vu_long *)MPC5XXX_XLBARB_MPRIVAL = 0x11111111;

# if defined(CFG_XLB_PIPELINING)
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

#if (CONFIG_COMMANDS & CFG_CMD_NET) && defined(CONFIG_MPC5xxx_FEC)
	/* load FEC microcode */
	loadtask(0, 2);
#endif

	return (0);
}

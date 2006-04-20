/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * 2004 (c) MontaVista Software, Inc.
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
#include <SA-1100.h>

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */

/*
 * Board dependent initialisation
 */

#define ECOR			0x8000
#define ECOR_RESET		0x80
#define ECOR_LEVEL_IRQ		0x40
#define ECOR_WR_ATTRIB		0x04
#define ECOR_ENABLE		0x01

#define ECSR			0x8002
#define ECSR_IOIS8		0x20
#define ECSR_PWRDWN		0x04
#define ECSR_INT		0x02
#define SMC_IO_SHIFT		2
#define NCR_0           	(*((volatile u_char *)(0x100000a0)))
#define NCR_ENET_OSC_EN		(1<<3)

static inline u8
readb(volatile u8 * p)
{
	return *p;
}

static inline void
writeb(u8 v, volatile u8 * p)
{
	*p = v;
}

static void
smc_init(void)
{
	u8 ecor;
	u8 ecsr;
	volatile u8 *addr = (volatile u8 *)(0x18000000 + (1 << 25));

	NCR_0 |= NCR_ENET_OSC_EN;
	udelay(100);

	ecor = readb(addr + (ECOR << SMC_IO_SHIFT)) & ~ECOR_RESET;
	writeb(ecor | ECOR_RESET, addr + (ECOR << SMC_IO_SHIFT));
	udelay(100);

	/*
	 * The device will ignore all writes to the enable bit while
	 * reset is asserted, even if the reset bit is cleared in the
	 * same write.  Must clear reset first, then enable the device.
	 */
	writeb(ecor, addr + (ECOR << SMC_IO_SHIFT));
	writeb(ecor | ECOR_ENABLE, addr + (ECOR << SMC_IO_SHIFT));

	/*
	 * Set the appropriate byte/word mode.
	 */
	ecsr = readb(addr + (ECSR << SMC_IO_SHIFT)) & ~ECSR_IOIS8;
	ecsr |= ECSR_IOIS8;
	writeb(ecsr, addr + (ECSR << SMC_IO_SHIFT));
	udelay(100);
}

static void
neponset_init(void)
{
	smc_init();
}

int
board_init(void)
{
	gd->bd->bi_arch_number = MACH_TYPE_ASSABET;
	gd->bd->bi_boot_params = 0xc0000100;

	neponset_init();

	return 0;
}

int
dram_init(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return (0);
}

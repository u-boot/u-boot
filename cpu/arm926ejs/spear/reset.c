/*
 * (C) Copyright 2009
 * Vipin Kumar, ST Micoelectronics, vipin.kumar@st.com.
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
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/spr_syscntl.h>

void reset_cpu(ulong ignored)
{
	struct syscntl_regs *syscntl_regs_p =
	    (struct syscntl_regs *)CONFIG_SPEAR_SYSCNTLBASE;

	printf("System is going to reboot ...\n");

	/*
	 * This 1 second delay will allow the above message
	 * to be printed before reset
	 */
	udelay((1000 * 1000));

	/* Going into slow mode before resetting SOC */
	writel(0x02, &syscntl_regs_p->scctrl);

	/*
	 * Writing any value to the system status register will
	 * reset the SoC
	 */
	writel(0x00, &syscntl_regs_p->scsysstat);

	/* system will restart */
	while (1)
		;
}

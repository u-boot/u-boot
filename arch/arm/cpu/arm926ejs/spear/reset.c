// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009
 * Vipin Kumar, ST Micoelectronics, vipin.kumar@st.com.
 */

#include <common.h>
#include <cpu_func.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/spr_syscntl.h>
#include <linux/delay.h>

void reset_cpu(void)
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

/*
 * Altera CF drvier
 *
 * (C) Copyright 2010, Thomas Chou <thomas@wytron.com.tw>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <common.h>
#include <asm/io.h>

#if defined(CONFIG_IDE_RESET) && defined(CONFIG_SYS_CF_CTL_BASE)
/* ide_set_reset for Altera CF interface */
#define ALTERA_CF_CTL_STATUS			0
#define ALTERA_CF_IDE_CTL			4
#define ALTERA_CF_CTL_STATUS_PRESENT_MSK	(0x1)
#define ALTERA_CF_CTL_STATUS_POWER_MSK		(0x2)
#define ALTERA_CF_CTL_STATUS_RESET_MSK		(0x4)
#define ALTERA_CF_CTL_STATUS_IRQ_EN_MSK	(0x8)
#define ALTERA_CF_IDE_CTL_IRQ_EN_MSK		(0x1)

void ide_set_reset(int idereset)
{
	int i;
	writel(idereset ? ALTERA_CF_CTL_STATUS_RESET_MSK :
	       ALTERA_CF_CTL_STATUS_POWER_MSK,
	       CONFIG_SYS_CF_CTL_BASE + ALTERA_CF_CTL_STATUS);
	/* wait 500 ms for power to stabilize */
	for (i = 0; i < 500; i++)
		udelay(1000);
}
#endif

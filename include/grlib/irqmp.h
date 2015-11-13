/* GRLIB IRQMP (IRQ Multi-processor controller) definitions
 *
 * (C) Copyright 2010, 2015
 * Daniel Hellstrom, Cobham Gaisler, daniel@gaisler.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __GRLIB_IRQMP_H__
#define __GRLIB_IRQMP_H__

typedef struct {
	volatile unsigned int ilevel;
	volatile unsigned int ipend;
	volatile unsigned int iforce;
	volatile unsigned int iclear;
	volatile unsigned int mstatus;
	volatile unsigned int notused[11];
	volatile unsigned int cpu_mask[16];
	volatile unsigned int cpu_force[16];
} ambapp_dev_irqmp;

#endif

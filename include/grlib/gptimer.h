/* GRLIB GPTIMER (General Purpose Timer) definitions
 *
 * (C) Copyright 2010, 2015
 * Daniel Hellstrom, Cobham Gaisler, daniel@gaisler.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __GRLIB_GPTIMER_H__
#define __GRLIB_GPTIMER_H__

typedef struct {
	volatile unsigned int val;
	volatile unsigned int rld;
	volatile unsigned int ctrl;
	volatile unsigned int unused;
} ambapp_dev_gptimer_element;

#define GPTIMER_CTRL_EN	0x1	/* Timer enable */
#define GPTIMER_CTRL_RS	0x2	/* Timer reStart  */
#define GPTIMER_CTRL_LD	0x4	/* Timer reLoad */
#define GPTIMER_CTRL_IE	0x8	/* interrupt enable */
#define GPTIMER_CTRL_IP	0x10	/* interrupt flag/pending */
#define GPTIMER_CTRL_CH	0x20	/* Chain with previous timer */

typedef struct {
	volatile unsigned int scalar;
	volatile unsigned int scalar_reload;
	volatile unsigned int config;
	volatile unsigned int unused;
	volatile ambapp_dev_gptimer_element e[8];
} ambapp_dev_gptimer;

#endif

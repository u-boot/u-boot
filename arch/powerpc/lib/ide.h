/*
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _MPC8XX_IDE_H_
#define _MPC8XX_IDE_H_ 1

#ifdef CONFIG_IDE_8xx_PCCARD
int pcmcia_on(void);
extern int ide_devices_found;
#endif
#endif

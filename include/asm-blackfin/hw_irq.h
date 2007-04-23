/*
 * U-boot - hw_irq.h
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
 *
 * This file is based on
 * linux/arch/$(ARCH)/platform/$(PLATFORM)/hw_irq.h
 * BlackFin (ADI) assembler restricted values by Ted Ma <mated@sympatico.ca>
 * Copyright (c) 2002 Arcturus Networks Inc. (www.arcturusnetworks.com)
 * Copyright (c) 2002 Lineo, Inc <mattw@lineo.com>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <linux/config.h>
#ifdef CONFIG_EZKIT533
#include <asm/arch-bf533/irq.h>
#endif
#ifdef CONFIG_EZKIT561
#include <asm/arch-bf561/irq.h>
#endif
#ifdef CONFIG_STAMP
#include <asm/arch-bf533/irq.h>
#endif
#ifdef CONFIG_BF537
#include <asm/arch-bf537/irq.h>
#endif

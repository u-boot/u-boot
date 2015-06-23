/*
 * From coreboot file of the same name
 *
 * Copyright (C) 2010 coresystems GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef __ASM_IOAPIC_H
#define __ASM_IOAPIC_H

#define IO_APIC_ADDR		0xfec00000

/* Direct addressed register */
#define IO_APIC_INDEX		(IO_APIC_ADDR + 0x00)
#define IO_APIC_DATA		(IO_APIC_ADDR + 0x10)

#endif

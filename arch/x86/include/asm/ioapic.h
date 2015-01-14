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
#define IO_APIC_INDEX		IO_APIC_ADDR
#define IO_APIC_DATA		(IO_APIC_ADDR + 0x10)
#define IO_APIC_INTERRUPTS	24

#define ALL		(0xff << 24)
#define NONE		0
#define DISABLED	(1 << 16)
#define ENABLED		(0 << 16)
#define TRIGGER_EDGE	(0 << 15)
#define TRIGGER_LEVEL	(1 << 15)
#define POLARITY_HIGH	(0 << 13)
#define POLARITY_LOW	(1 << 13)
#define PHYSICAL_DEST	(0 << 11)
#define LOGICAL_DEST	(1 << 11)
#define ExtINT		(7 << 8)
#define NMI		(4 << 8)
#define SMI		(2 << 8)
#define INT		(1 << 8)

u32 io_apic_read(u32 ioapic_base, u32 reg);
void io_apic_write(u32 ioapic_base, u32 reg, u32 value);
void set_ioapic_id(u32 ioapic_base, u8 ioapic_id);
void setup_ioapic(u32 ioapic_base, u8 ioapic_id);
void clear_ioapic(u32 ioapic_base);

#endif

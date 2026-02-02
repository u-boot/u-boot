/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2025, Kuan-Wei Chiu <visitorckw@gmail.com>
 *
 * Definitions for the m68k bootinfo interface.
 */

#ifndef _ASM_M68K_BOOTINFO_H
#define _ASM_M68K_BOOTINFO_H

#ifndef __ASSEMBLY__

struct bi_record {
	unsigned short tag;        /* tag ID */
	unsigned short size;       /* size of record (in bytes) */
	unsigned long data[0];     /* data */
};

#endif /* __ASSEMBLY__ */

/* Bootinfo Tag IDs */
#define BI_LAST            0x0000
#define BI_MACHTYPE        0x0001
#define BI_CPUTYPE         0x0002
#define BI_FPUTYPE         0x0003
#define BI_MMUTYPE         0x0004
#define BI_MEMCHUNK        0x0005
#define BI_RAMDISK         0x0006
#define BI_COMMAND_LINE    0x0007

/* QEMU virt specific tags */
#define BI_VIRT_QEMU_VERSION    0x8000
#define BI_VIRT_GF_PIC_BASE     0x8001
#define BI_VIRT_GF_RTC_BASE     0x8002
#define BI_VIRT_GF_TTY_BASE     0x8003
#define BI_VIRT_VIRTIO_BASE     0x8004
#define BI_VIRT_CTRL_BASE       0x8005

#endif /* _ASM_M68K_BOOTINFO_H */

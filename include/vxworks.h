/*
 * (C) Copyright 2008
 * Niklaus Giger, niklaus.giger@member.fsf.org
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _VXWORKS_H_
#define _VXWORKS_H_

/*
 * VxWorks x86 E820 related stuff
 *
 * VxWorks on x86 gets E820 information from pre-defined address @
 * 0x4a00 and 0x4000. At 0x4a00 it's an information table defined
 * by VxWorks and the actual E820 table entries starts from 0x4000.
 * As defined by the BIOS E820 spec, the maximum number of E820 table
 * entries is 128 and each entry occupies 20 bytes, so it's 128 * 20
 * = 2560 (0xa00) bytes in total. That's where VxWorks stores some
 * information that is retrieved from the BIOS E820 call and saved
 * later for sanity test during the kernel boot-up.
 */
#define VXWORKS_E820_DATA_ADDR	0x4000
#define VXWORKS_E820_INFO_ADDR	0x4a00

/* E820 info signatiure "SMAP" - System MAP */
#define E820_SIGNATURE	0x534d4150

struct e820info {
	u32 sign;	/* "SMAP" signature */
	u32 x0;		/* don't care, used by VxWorks */
	u32 x1;		/* don't care, used by VxWorks */
	u32 x2;		/* don't care, used by VxWorks */
	u32 addr;	/* last e820 table entry addr */
	u32 x3;		/* don't care, used by VxWorks */
	u32 entries;	/* e820 table entry count */
	u32 error;	/* must be zero */
};

int do_bootvx(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
void boot_prep_vxworks(bootm_headers_t *images);
void boot_jump_vxworks(bootm_headers_t *images);
void do_bootvx_fdt(bootm_headers_t *images);

#endif

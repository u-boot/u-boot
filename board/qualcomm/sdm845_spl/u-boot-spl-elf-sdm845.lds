/* SPDX-License-Identifier: GPL-2.0+ */

/*
 * The boot rom uses the segment type to identify the xbl_sec program header.
 * See https://github.com/coreboot/coreboot/blob/643efabd2af9f7ac/util/qualcomm/mbn_tools.py#L143.
 */
#define PF_SEGMENT_TYPE_XBL_SEC 5

/*
 * The boot rom expects this to be equal to a seemingly magic value possibly specific
 * to a particular xbl_sec.elf. If you're extracting your xbl_sec.elf from an xbl elf,
 * you can just reuse the value; otherwise you can either compute it yourself or use
 * coreboot's tool.
 * See https://github.com/coreboot/coreboot/blob/643efabd2af9f7ac/util/qualcomm/createxbl.py#L638.
 */
#define XLB_SEC_SEGMENT_ADDR 0x0000000014699000


TARGET("binary")
INPUT("./xbl_sec.elf")

OUTPUT_FORMAT("default")

ENTRY(CONFIG_PLATFORM_ELFENTRY)
PHDRS
{
	data PT_LOAD FLAGS(7);
	xbl_sec PT_LOAD FLAGS(5 | (PF_SEGMENT_TYPE_XBL_SEC << 24));
}
SECTIONS
{

	. = XLB_SEC_SEGMENT_ADDR;
	.xbl_sec : { // XBL_SEC nested ELF
		. = .;
		"./xbl_sec.elf"
	} :xbl_sec

	. = CONFIG_PLATFORM_ELFENTRY;

	.data : {
		*(.data*)
	} :data
}

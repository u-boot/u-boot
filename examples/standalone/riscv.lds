/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 */

OUTPUT_ARCH(riscv)
ENTRY(_start)
SECTIONS
{
    . = ALIGN(4);
    .text :
    {
        *(.text)
    }

    . = ALIGN(4);
    .data : {
			__global_pointer$ = . + 0x800;
			*(.data)
		}

    . = ALIGN(4);

    .got : {
        __got_start = .;
        *(.got)
        __got_end = .;
    }

     . = ALIGN(4);
    __bss_start = .;
    .bss : { *(.bss) }
    __bss_end = .;

    . = ALIGN(4);
    .rela.text : { *(.rela.text .rela.text.* .rela.gnu.linkonce.t.*) }

    _end = .;
}

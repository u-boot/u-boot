#!/usr/bin/env python2
"""
A script to generate FIT image source for rockchip boards
with ARM Trusted Firmware
and multiple device trees (given on the command line)

usage: $0 <dt_name> [<dt_name> [<dt_name] ...]
"""

import os
import sys
import getopt

# pip install pyelftools
from elftools.elf.elffile import ELFFile

ELF_SEG_P_TYPE='p_type'
ELF_SEG_P_PADDR='p_paddr'
ELF_SEG_P_VADDR='p_vaddr'
ELF_SEG_P_OFFSET='p_offset'
ELF_SEG_P_FILESZ='p_filesz'
ELF_SEG_P_MEMSZ='p_memsz'

DT_HEADER="""// SPDX-License-Identifier: GPL-2.0+ OR X11
/*
 * Copyright (C) 2017 Fuzhou Rockchip Electronics Co., Ltd
 *
 * Minimal dts for a SPL FIT image payload.
 */
/dts-v1/;

/ {
	description = "Configuration to load ATF before U-Boot";
	#address-cells = <1>;

	images {
		uboot@1 {
			description = "U-Boot (64-bit)";
			data = /incbin/("u-boot-nodtb.bin");
			type = "standalone";
			os = "U-Boot";
			arch = "arm64";
			compression = "none";
			load = <0x%08x>;
		};
"""

DT_IMAGES_NODE_END="""
    };
"""

DT_END="""
};
"""

def append_atf_node(file, atf_index, phy_addr):
    """
    Append ATF DT node to input FIT dts file.
    """
    data = 'bl31_0x%08x.bin' % phy_addr
    print >> file, '\t\tatf@%d {' % atf_index
    print >> file, '\t\t\tdescription = \"ARM Trusted Firmware\";'
    print >> file, '\t\t\tdata = /incbin/("%s");' % data
    print >> file, '\t\t\ttype = "firmware";'
    print >> file, '\t\t\tarch = "arm64";'
    print >> file, '\t\t\tos = "arm-trusted-firmware";'
    print >> file, '\t\t\tcompression = "none";'
    print >> file, '\t\t\tload = <0x%08x>;' % phy_addr
    if atf_index == 1:
        print >> file, '\t\t\tentry = <0x%08x>;' % phy_addr
    print >> file, '\t\t};'
    print >> file, ''

def append_fdt_node(file, dtbs):
    """
    Append FDT nodes.
    """
    cnt = 1
    for dtb in dtbs:
        dtname = os.path.basename(dtb)
        print >> file, '\t\tfdt@%d {' % cnt
        print >> file, '\t\t\tdescription = "%s";' % dtname
        print >> file, '\t\t\tdata = /incbin/("%s");' % dtb
        print >> file, '\t\t\ttype = "flat_dt";'
        print >> file, '\t\t\tcompression = "none";'
        print >> file, '\t\t};'
        print >> file, ''
        cnt = cnt + 1

def append_conf_section(file, cnt, dtname, atf_cnt):
    print >> file, '\t\tconfig@%d {' % cnt
    print >> file, '\t\t\tdescription = "%s";' % dtname
    print >> file, '\t\t\tfirmware = "atf@1";'
    print >> file, '\t\t\tloadables = "uboot@1",',
    for i in range(1, atf_cnt):
        print >> file, '"atf@%d"' % (i+1),
        if i != (atf_cnt - 1):
            print >> file, ',',
        else:
            print >> file, ';'
    print >> file, '\t\t\tfdt = "fdt@1";'
    print >> file, '\t\t};'
    print >> file, ''

def append_conf_node(file, dtbs, atf_cnt):
    """
    Append configeration nodes.
    """
    cnt = 1
    print >> file, '\tconfigurations {'
    print >> file, '\t\tdefault = "config@1";'
    for dtb in dtbs:
        dtname = os.path.basename(dtb)
        append_conf_section(file, cnt, dtname, atf_cnt)
        cnt = cnt + 1
    print >> file, '\t};'
    print >> file, ''

def generate_atf_fit_dts(fit_file_name, bl31_file_name, uboot_file_name, dtbs_file_name):
    """
    Generate FIT script for ATF image.
    """
    if fit_file_name != sys.stdout:
        fit_file = open(fit_file_name, "wb")
    else:
        fit_file = sys.stdout

    num_load_seg = 0
    p_paddr = 0xFFFFFFFF
    with open(uboot_file_name) as uboot_file:
        uboot = ELFFile(uboot_file)
        for i in range(uboot.num_segments()):
            seg = uboot.get_segment(i)
            if ('PT_LOAD' == seg.__getitem__(ELF_SEG_P_TYPE)):
                p_paddr = seg.__getitem__(ELF_SEG_P_PADDR)
                num_load_seg = num_load_seg + 1

    assert (p_paddr != 0xFFFFFFFF and num_load_seg == 1)

    print >> fit_file, DT_HEADER % p_paddr

    with open(bl31_file_name) as bl31_file:
        bl31 = ELFFile(bl31_file)
        for i in range(bl31.num_segments()):
            seg = bl31.get_segment(i)
            if ('PT_LOAD' == seg.__getitem__(ELF_SEG_P_TYPE)):
                paddr = seg.__getitem__(ELF_SEG_P_PADDR)
                p= seg.__getitem__(ELF_SEG_P_PADDR)
                append_atf_node(fit_file, i+1, paddr)
    atf_cnt = i+1
    append_fdt_node(fit_file, dtbs_file_name)
    print >> fit_file, '%s' % DT_IMAGES_NODE_END
    append_conf_node(fit_file, dtbs_file_name, atf_cnt)
    print >> fit_file, '%s' % DT_END

    if fit_file_name != sys.stdout:
        fit_file.close()

def generate_atf_binary(bl31_file_name):
    with open(bl31_file_name) as bl31_file:
        bl31 = ELFFile(bl31_file)

        num = bl31.num_segments()
        for i in range(num):
            seg = bl31.get_segment(i)
            if ('PT_LOAD' == seg.__getitem__(ELF_SEG_P_TYPE)):
                paddr = seg.__getitem__(ELF_SEG_P_PADDR)
                file_name = 'bl31_0x%08x.bin' % paddr
                with open(file_name, "wb") as atf:
                    atf.write(seg.data());

def get_bl31_segments_info(bl31_file_name):
    """
    Get load offset, physical offset, file size
    from bl31 elf file program headers.
    """
    with open(bl31_file_name) as bl31_file:
        bl31 = ELFFile(bl31_file)

        num = bl31.num_segments()
        print 'Number of Segments : %d' % bl31.num_segments()
        for i in range(num):
            print 'Segment %d' % i
            seg = bl31.get_segment(i)
            ptype = seg[ELF_SEG_P_TYPE]
            poffset = seg[ELF_SEG_P_OFFSET]
            pmemsz = seg[ELF_SEG_P_MEMSZ]
            pfilesz = seg[ELF_SEG_P_FILESZ]
            print 'type: %s\nfilesz: %08x\nmemsz: %08x\noffset: %08x' % (ptype, pfilesz, pmemsz, poffset)
            paddr = seg[ELF_SEG_P_PADDR]
            print 'paddr: %08x' % paddr

def main():
    uboot_elf="./u-boot"
    bl31_elf="./bl31.elf"
    FIT_ITS=sys.stdout

    opts, args = getopt.getopt(sys.argv[1:], "o:u:b:h")
    for opt, val in opts:
        if opt == "-o":
            FIT_ITS=val
        elif opt == "-u":
            uboot_elf=val
        elif opt == "-b":
            bl31_elf=val
        elif opt == "-h":
            print __doc__
            sys.exit(2)

    dtbs = args
    #get_bl31_segments_info("u-boot")
    #get_bl31_segments_info("bl31.elf")

    generate_atf_fit_dts(FIT_ITS, bl31_elf, uboot_elf, dtbs)
    generate_atf_binary(bl31_elf);

if __name__ == "__main__":
    main()

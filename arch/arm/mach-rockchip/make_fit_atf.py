#!/usr/bin/env python
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
		uboot {
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

def append_atf_node(file, atf_index, phy_addr, elf_entry):
    """
    Append ATF DT node to input FIT dts file.
    """
    data = 'bl31_0x%08x.bin' % phy_addr
    file.write('\t\tatf_%d {\n' % atf_index)
    file.write('\t\t\tdescription = \"ARM Trusted Firmware\";\n')
    file.write('\t\t\tdata = /incbin/("%s");\n' % data)
    file.write('\t\t\ttype = "firmware";\n')
    file.write('\t\t\tarch = "arm64";\n')
    file.write('\t\t\tos = "arm-trusted-firmware";\n')
    file.write('\t\t\tcompression = "none";\n')
    file.write('\t\t\tload = <0x%08x>;\n' % phy_addr)
    if atf_index == 1:
        file.write('\t\t\tentry = <0x%08x>;\n' % elf_entry)
    file.write('\t\t};\n')
    file.write('\n')

def append_fdt_node(file, dtbs):
    """
    Append FDT nodes.
    """
    cnt = 1
    for dtb in dtbs:
        dtname = os.path.basename(dtb)
        file.write('\t\tfdt_%d {\n' % cnt)
        file.write('\t\t\tdescription = "%s";\n' % dtname)
        file.write('\t\t\tdata = /incbin/("%s");\n' % dtb)
        file.write('\t\t\ttype = "flat_dt";\n')
        file.write('\t\t\tcompression = "none";\n')
        file.write('\t\t};\n')
        file.write('\n')
        cnt = cnt + 1

def append_conf_section(file, cnt, dtname, atf_cnt):
    file.write('\t\tconfig_%d {\n' % cnt)
    file.write('\t\t\tdescription = "%s";\n' % dtname)
    file.write('\t\t\tfirmware = "atf_1";\n')
    file.write('\t\t\tloadables = "uboot",')
    for i in range(1, atf_cnt):
        file.write('"atf_%d"' % (i+1))
        if i != (atf_cnt - 1):
            file.write(',')
        else:
            file.write(';\n')
    file.write('\t\t\tfdt = "fdt_1";\n')
    file.write('\t\t};\n')
    file.write('\n')

def append_conf_node(file, dtbs, atf_cnt):
    """
    Append configeration nodes.
    """
    cnt = 1
    file.write('\tconfigurations {\n')
    file.write('\t\tdefault = "config_1";\n')
    for dtb in dtbs:
        dtname = os.path.basename(dtb)
        append_conf_section(file, cnt, dtname, atf_cnt)
        cnt = cnt + 1
    file.write('\t};\n')
    file.write('\n')

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
    with open(uboot_file_name, 'rb') as uboot_file:
        uboot = ELFFile(uboot_file)
        for i in range(uboot.num_segments()):
            seg = uboot.get_segment(i)
            if ('PT_LOAD' == seg.__getitem__(ELF_SEG_P_TYPE)):
                p_paddr = seg.__getitem__(ELF_SEG_P_PADDR)
                num_load_seg = num_load_seg + 1

    assert (p_paddr != 0xFFFFFFFF and num_load_seg == 1)

    fit_file.write(DT_HEADER % p_paddr)

    with open(bl31_file_name, 'rb') as bl31_file:
        bl31 = ELFFile(bl31_file)
        elf_entry = bl31.header['e_entry']
        for i in range(bl31.num_segments()):
            seg = bl31.get_segment(i)
            if ('PT_LOAD' == seg.__getitem__(ELF_SEG_P_TYPE)):
                paddr = seg.__getitem__(ELF_SEG_P_PADDR)
                p= seg.__getitem__(ELF_SEG_P_PADDR)
                append_atf_node(fit_file, i+1, paddr, elf_entry)
    atf_cnt = i+1
    append_fdt_node(fit_file, dtbs_file_name)
    fit_file.write('%s\n' % DT_IMAGES_NODE_END)
    append_conf_node(fit_file, dtbs_file_name, atf_cnt)
    fit_file.write('%s\n' % DT_END)

    if fit_file_name != sys.stdout:
        fit_file.close()

def generate_atf_binary(bl31_file_name):
    with open(bl31_file_name, 'rb') as bl31_file:
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
        print('Number of Segments : %d' % bl31.num_segments())
        for i in range(num):
            print('Segment %d' % i)
            seg = bl31.get_segment(i)
            ptype = seg[ELF_SEG_P_TYPE]
            poffset = seg[ELF_SEG_P_OFFSET]
            pmemsz = seg[ELF_SEG_P_MEMSZ]
            pfilesz = seg[ELF_SEG_P_FILESZ]
            print('type: %s\nfilesz: %08x\nmemsz: %08x\noffset: %08x' % (ptype, pfilesz, pmemsz, poffset))
            paddr = seg[ELF_SEG_P_PADDR]
            print('paddr: %08x' % paddr)

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
            print(__doc__)
            sys.exit(2)

    dtbs = args
    #get_bl31_segments_info("u-boot")
    #get_bl31_segments_info("bl31.elf")

    generate_atf_fit_dts(FIT_ITS, bl31_elf, uboot_elf, dtbs)
    generate_atf_binary(bl31_elf);

if __name__ == "__main__":
    main()

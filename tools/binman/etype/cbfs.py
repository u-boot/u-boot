# SPDX-License-Identifier: GPL-2.0+
# Copyright 2019 Google LLC
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for a Coreboot Filesystem (CBFS)
#

from collections import OrderedDict

import cbfs_util
from cbfs_util import CbfsWriter
from entry import Entry
import fdt_util

class Entry_cbfs(Entry):
    """Entry containing a Coreboot Filesystem (CBFS)

    A CBFS provides a way to group files into a group. It has a simple directory
    structure and allows the position of individual files to be set, since it is
    designed to support execute-in-place in an x86 SPI-flash device. Where XIP
    is not used, it supports compression and storing ELF files.

    CBFS is used by coreboot as its way of orgnanising SPI-flash contents.

    The contents of the CBFS are defined by subnodes of the cbfs entry, e.g.:

        cbfs {
            size = <0x100000>;
            u-boot {
                cbfs-type = "raw";
            };
            u-boot-dtb {
                cbfs-type = "raw";
            };
        };

    This creates a CBFS 1MB in size two files in it: u-boot.bin and u-boot.dtb.
    Note that the size is required since binman does not support calculating it.
    The contents of each entry is just what binman would normally provide if it
    were not a CBFS node. A blob type can be used to import arbitrary files as
    with the second subnode below:

        cbfs {
            size = <0x100000>;
            u-boot {
                cbfs-name = "BOOT";
                cbfs-type = "raw";
            };

            dtb {
                type = "blob";
                filename = "u-boot.dtb";
                cbfs-type = "raw";
                cbfs-compress = "lz4";
            };
        };

    This creates a CBFS 1MB in size with u-boot.bin (named "BOOT") and
    u-boot.dtb (named "dtb") and compressed with the lz4 algorithm.


    Properties supported in the top-level CBFS node:

    cbfs-arch:
        Defaults to "x86", but you can specify the architecture if needed.


    Properties supported in the CBFS entry subnodes:

    cbfs-name:
        This is the name of the file created in CBFS. It defaults to the entry
        name (which is the node name), but you can override it with this
        property.

    cbfs-type:
        This is the CBFS file type. The following are supported:

        raw:
            This is a 'raw' file, although compression is supported. It can be
            used to store any file in CBFS.

        stage:
            This is an ELF file that has been loaded (i.e. mapped to memory), so
            appears in the CBFS as a flat binary. The input file must be an ELF
            image, for example this puts "u-boot" (the ELF image) into a 'stage'
            entry:

                cbfs {
                    size = <0x100000>;
                    u-boot-elf {
                        cbfs-name = "BOOT";
                        cbfs-type = "stage";
                    };
                };

            You can use your own ELF file with something like:

                cbfs {
                    size = <0x100000>;
                    something {
                        type = "blob";
                        filename = "cbfs-stage.elf";
                        cbfs-type = "stage";
                    };
                };

            As mentioned, the file is converted to a flat binary, so it is
            equivalent to adding "u-boot.bin", for example, but with the load and
            start addresses specified by the ELF. At present there is no option
            to add a flat binary with a load/start address, similar to the
            'add-flat-binary' option in cbfstool.


    The current implementation supports only a subset of CBFS features. It does
    not support other file types (e.g. payload), adding multiple files (like the
    'files' entry with a pattern supported by binman), putting files at a
    particular offset in the CBFS and a few other things.

    Of course binman can create images containing multiple CBFSs, simply by
    defining these in the binman config:


        binman {
            size = <0x800000>;
            cbfs {
                offset = <0x100000>;
                size = <0x100000>;
                u-boot {
                    cbfs-type = "raw";
                };
                u-boot-dtb {
                    cbfs-type = "raw";
                };
            };

            cbfs2 {
                offset = <0x700000>;
                size = <0x100000>;
                u-boot {
                    cbfs-type = "raw";
                };
                u-boot-dtb {
                    cbfs-type = "raw";
                };
                image {
                    type = "blob";
                    filename = "image.jpg";
                };
            };
        };

    This creates an 8MB image with two CBFSs, one at offset 1MB, one at 7MB,
    both of size 1MB.
    """
    def __init__(self, section, etype, node):
        Entry.__init__(self, section, etype, node)
        self._cbfs_arg = fdt_util.GetString(node, 'cbfs-arch', 'x86')
        self._cbfs_entries = OrderedDict()
        self._ReadSubnodes()

    def ObtainContents(self):
        arch = cbfs_util.find_arch(self._cbfs_arg)
        if arch is None:
            self.Raise("Invalid architecture '%s'" % self._cbfs_arg)
        if self.size is None:
            self.Raise("'cbfs' entry must have a size property")
        cbfs = CbfsWriter(self.size, arch)
        for entry in self._cbfs_entries.values():
            # First get the input data and put it in a file. If not available,
            # try later.
            if not entry.ObtainContents():
                return False
            data = entry.GetData()
            if entry._type == 'raw':
                cbfs.add_file_raw(entry._cbfs_name, data, entry._cbfs_compress)
            elif entry._type == 'stage':
                cbfs.add_file_stage(entry._cbfs_name, data)
        data = cbfs.get_data()
        self.SetContents(data)
        return True

    def _ReadSubnodes(self):
        """Read the subnodes to find out what should go in this IFWI"""
        for node in self._node.subnodes:
            entry = Entry.Create(self.section, node)
            entry._cbfs_name = fdt_util.GetString(node, 'cbfs-name', entry.name)
            entry._type = fdt_util.GetString(node, 'cbfs-type')
            compress = fdt_util.GetString(node, 'cbfs-compress', 'none')
            entry._cbfs_compress = cbfs_util.find_compress(compress)
            if entry._cbfs_compress is None:
                self.Raise("Invalid compression in '%s': '%s'" %
                           (node.name, compress))
            self._cbfs_entries[entry._cbfs_name] = entry

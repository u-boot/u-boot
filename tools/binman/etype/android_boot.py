# SPDX-License-Identifier: GPL-2.0+
# Entry-type module for Android boot images

import hashlib
import struct

from binman.entry import Entry
from binman.etype.section import Entry_section
from dtoc import fdt_util


BOOT_MAGIC = b'ANDROID!'
BOOT_NAME_SIZE = 16
BOOT_ARGS_SIZE = 512
IMAGE_ID_SIZE = 32
BOOT_EXTRA_ARGS_SIZE = 1024

BOOT_IMAGE_HEADER_V0 = '<{}s10I{}s{}s{}s'.format(len(BOOT_MAGIC),
                                                    BOOT_NAME_SIZE,
                                                    BOOT_ARGS_SIZE,
                                                    IMAGE_ID_SIZE)
BOOT_IMAGE_HEADER_V0_SIZE = struct.calcsize(BOOT_IMAGE_HEADER_V0)
BOOT_IMAGE_HEADER_V2 = (BOOT_IMAGE_HEADER_V0 +
                        '{}sIQIIQ'.format(BOOT_EXTRA_ARGS_SIZE))
BOOT_IMAGE_HEADER_V2_SIZE = struct.calcsize(BOOT_IMAGE_HEADER_V2)


class Entry_android_boot(Entry_section):
    """Android boot image

    This creates an Android v0 or v2 boot image.

    A kernel payload, optional ramdisk payload can be supplied. A DTB payload
    can also be provided when header_version == v2.

    Vendor-specific payloads are also supported. These are non-standard
    v0 images with a special DT container format appended.

    Properties / Entry arguments:
        - header-version: Android boot image header version, must be 0 or 2,
          defaults to 0
        - page-size: Image page size, defaults to 2048
        - base: Base address added to the offsets below, defaults to 0x10000000
        - kernel-offset: Kernel load offset from base, defaults to 0x00008000
        - ramdisk-offset: Ramdisk load offset from base, defaults to 0x01000000
        - tags-offset: ATAGS/FDT offset from base, defaults to 0x00000100
        - dtb-offset: DTB load offset from base, defaults to 0x01f00000
        - os-version: Encoded Android OS version and patch level, defaults to 0
        - boot-name: Android boot image board name
        - cmdline: Android boot command line

    This entry uses the following subnodes:
        - kernel: section containing the executable payload
        - dtb: section containing the DTB payload, used by header version 2 only
        - ramdisk: optional section containing a ramdisk payload
        - vendor-dt: legacy vendor DT payload, used by header version 0 only

    Example::
        A v2 abootimg with control FDT placed in the DTB section:

        android-boot {
            header-version = <2>;
            page-size = <4096>;
            base = <0x12345678>;
            kernel-offset = <0xCAFED00D>;
            ramdisk-offset = <0xBEEFBABE>;
            tags-offset = <0xFEEDDEAD>;
            dtb-offset = <0x06660666>;
            cmdline = "foo bar";

            kernel {
                u-boot-nodtb {
                    # Many Android bootloaders support gzipped kernels
                    compress = "gzip";
                };
            };

            dtb {
                u-boot-dtb {
                };
            };
        };

    Example::
        A v0 abootimg with embedded control FDT (v0 doesn't support DTBs) and
        an empty ramdisk (some bootloaders insist on a ramdisk being present):

        android-boot {
            header-version = <0>;
            page-size = <2048>;
            base = <0x80200000>;

            kernel {
                u-boot {
                    no-expanded;
                };
            };

            ramdisk {
                fill {
                    size = <1>;
                };
            };
        };
    """

    def ReadNode(self):
        super().ReadNode()
        self.header_version = fdt_util.GetInt(self._node, 'header-version', 0)
        self.page_size = fdt_util.GetInt(self._node, 'page-size', 2048)
        self.base = self._GetIntCells('base', 0x10000000)
        self.kernel_offset = self._GetIntCells('kernel-offset', 0x00008000)
        self.ramdisk_offset = self._GetIntCells('ramdisk-offset', 0x01000000)
        self.tags_offset = self._GetIntCells('tags-offset', 0x00000100)
        self.dtb_offset = self._GetIntCells('dtb-offset', 0x01f00000)
        self.os_version = fdt_util.GetInt(self._node, 'os-version', 0)
        self.boot_name = fdt_util.GetString(self._node, 'boot-name', '')
        self.cmdline = fdt_util.GetString(self._node, 'cmdline', '')
        self.vendor_dt_node = self._node.FindNode('vendor-dt')

        if self.header_version not in (0, 2):
            self.Raise('Only Android boot image header versions 0 and 2 are '
                       'supported')
        if self.page_size <= 0 or self.page_size & (self.page_size - 1):
            self.Raise('page-size must be a power of two')
        if 'kernel' not in self._entries:
            self.Raise("Missing required subnode 'kernel'")

        if self.header_version == 0:
            if self.page_size < BOOT_IMAGE_HEADER_V0_SIZE:
                self.Raise('page-size must fit the Android boot image header')
            if 'dtb' in self._entries:
                self.Raise("Subnode 'dtb' requires header-version 2")
        else:
            # v2
            if self.page_size < BOOT_IMAGE_HEADER_V2_SIZE:
                self.Raise('page-size must fit the Android boot image header')
            if 'dtb' not in self._entries:
                self.Raise("Missing required subnode 'dtb'")
            if self.vendor_dt_node:
                self.Raise("Subnode 'vendor-dt' requires header-version 0")

    def ReadEntries(self):
        for node in self._node.subnodes:
            if node.name == 'vendor-dt':
                self._ReadVendorDtEntries(node)
                continue

            if node.name not in ('kernel', 'ramdisk', 'dtb'):
                self.Raise("Unexpected subnode '%s'" % node.name)

            entry = Entry.Create(self, node, etype='section',
                                 expanded=self.GetImage().use_expanded,
                                 missing_etype=self.GetImage().missing_etype)
            entry.ReadNode()
            entry.SetPrefix(self._name_prefix)
            self._entries[node.name] = entry

    def _ReadVendorDtEntries(self, vendor_dt_node):
        entry = Entry.Create(self, vendor_dt_node, etype='section',
                             expanded=self.GetImage().use_expanded,
                             missing_etype=self.GetImage().missing_etype)
        entry.ReadNode()
        entry.SetPrefix(self._name_prefix)
        self._entries[vendor_dt_node.name] = entry

    def _GetIntCells(self, propname, default):
        prop = self._node.props.get(propname)
        if not prop:
            return default

        values = prop.value if isinstance(prop.value, list) else [prop.value]
        if len(values) > 2:
            self.Raise("Property '%s' must contain one or two cells" %
                       propname)

        value = 0
        for cell in values:
            value = value << 32 | fdt_util.fdt32_to_cpu(cell)

        return value

    def _GetAddr(self, offset, name, size=32):
        addr = self.base + offset
        if addr >= 1 << size:
            self.Raise('%s address %#x does not fit in %d bits' %
                       (name, addr, size))

        return addr

    def _CheckFit(self, name, data, size):
        if len(data) > size:
            self.Raise('%s is %d bytes, maximum is %d' %
                       (name, len(data), size))

        return data + b'\0' * (size - len(data))

    @staticmethod
    def _BootId(*payloads):
        digest = hashlib.sha1()
        for data in payloads:
            digest.update(data)
            digest.update(struct.pack('<I', len(data)))

        return digest.digest() + b'\0' * 12

    def _SplitCmdline(self):
        cmdline = self.cmdline.encode('ascii') + b'\0'
        return (self._CheckFit('cmdline', cmdline[:BOOT_ARGS_SIZE],
                               BOOT_ARGS_SIZE),
                self._CheckFit('extra-cmdline', cmdline[BOOT_ARGS_SIZE:],
                               BOOT_EXTRA_ARGS_SIZE))

    def _GetEntryData(self, name, required, default=None):
        entry = self._entries.get(name)
        if not entry and default is not None:
            return default
        return entry.GetData(required)

    def _BuildVendorDt(self, required):
        if not self.vendor_dt_node:
            return b''
        return self._GetEntryData('vendor-dt', required)

    def _BuildV0SectionData(self, required):
        kernel = self._GetEntryData('kernel', required)
        vendor_dt = self._BuildVendorDt(required)
        ramdisk = self._GetEntryData('ramdisk', required, b'')
        if not required and (kernel is None or vendor_dt is None or
                             ramdisk is None):
            return None

        boot_name = self._CheckFit('boot-name', self.boot_name.encode('ascii'),
                                   BOOT_NAME_SIZE)
        cmdline = self._CheckFit('cmdline', self.cmdline.encode('ascii'),
                                 BOOT_ARGS_SIZE)

        boot_id_payloads = [kernel, ramdisk, b'']
        if self.vendor_dt_node:
            boot_id_payloads.append(vendor_dt)
        image_id = self._BootId(*boot_id_payloads)

        overloaded_header_version =  self.header_version
        if self.vendor_dt_node:
            # vendor DTs overload the header_version field to store the length
            # of the appended payload. Hopefully AOSP abootimg never progresses
            # to v8192-ish or we might have some real specificity problems on
            # our hands.
            overloaded_header_version = len(vendor_dt)

        header = struct.pack(BOOT_IMAGE_HEADER_V0,
                             BOOT_MAGIC,
                             len(kernel),
                             self._GetAddr(self.kernel_offset, 'kernel'),
                             len(ramdisk),
                             self._GetAddr(self.ramdisk_offset, 'ramdisk'),
                             0, # second_len
                             0, # second_offset
                             self._GetAddr(self.tags_offset, 'tags'),
                             self.page_size,
                             overloaded_header_version,
                             self.os_version,
                             boot_name,
                             cmdline,
                             image_id)

        image = bytearray()
        image += self.PadToAlignment(header, self.page_size)
        image += self.PadToAlignment(kernel, self.page_size)
        image += self.PadToAlignment(ramdisk, self.page_size)
        image += self.PadToAlignment(vendor_dt, self.page_size)

        return bytes(image)

    def _BuildV2SectionData(self, required):
        kernel = self._GetEntryData('kernel', required)
        dtb = self._GetEntryData('dtb', required)
        ramdisk = self._GetEntryData('ramdisk', required, b'')
        if not required and (kernel is None or dtb is None or
                             ramdisk is None):
            return None

        boot_name = self._CheckFit('boot-name', self.boot_name.encode('ascii'),
                                   BOOT_NAME_SIZE)
        cmdline, extra_cmdline = self._SplitCmdline()
        image_id = self._BootId(kernel, ramdisk, b'', b'', dtb)

        header = struct.pack(BOOT_IMAGE_HEADER_V2,
                             BOOT_MAGIC,
                             len(kernel),
                             self._GetAddr(self.kernel_offset, 'kernel'),
                             len(ramdisk),
                             self._GetAddr(self.ramdisk_offset, 'ramdisk'),
                             0, # second_len
                             0, # second_offset
                             self._GetAddr(self.tags_offset, 'tags'),
                             self.page_size,
                             self.header_version,
                             self.os_version,
                             boot_name,
                             cmdline,
                             image_id,
                             extra_cmdline,
                             0, # recovery_dtbo_len
                             0, # recovery_dtbo_offset
                             BOOT_IMAGE_HEADER_V2_SIZE,
                             len(dtb),
                             self._GetAddr(self.dtb_offset, 'dtb', size=64))

        image = bytearray()
        image += self.PadToAlignment(header, self.page_size)
        image += self.PadToAlignment(kernel, self.page_size)
        image += self.PadToAlignment(ramdisk, self.page_size)
        image += self.PadToAlignment(dtb, self.page_size)

        return bytes(image)

    def BuildSectionData(self, required):
        if self.header_version == 0:
            return self._BuildV0SectionData(required)

        return self._BuildV2SectionData(required)

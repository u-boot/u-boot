# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2023 Linaro Limited
#
# Entry-type module for producing an empty  EFI capsule
#

import os

from binman.entry import Entry
from binman.etype.section import Entry_section
from dtoc import fdt_util
from u_boot_pylib import tools

class Entry_efi_empty_capsule(Entry_section):
    """Generate EFI empty capsules

    The parameters needed for generation of the empty capsules can
    be provided as properties in the entry.

    Properties / Entry arguments:
    - image-guid: Image GUID which will be used for identifying the
      updatable image on the board. Mandatory for accept capsule.
    - accept-capsule - Boolean property to generate an accept capsule.
      image-type-id
    - revert-capsule - Boolean property to generate a revert capsule

    For more details on the description of the capsule format, and the capsule
    update functionality, refer Section 8.5 and Chapter 23 in the `UEFI
    specification`_. For more information on the empty capsule, refer the
    sections 2.3.2 and 2.3.3 in the `Dependable Boot specification`_.

    A typical accept empty capsule entry node would then look something like this

    empty-capsule {
            type = "efi-empty-capsule";
            /* Image GUID for testing capsule update */
            image-type-id = SANDBOX_UBOOT_IMAGE_GUID;
            accept-capsule;
    };

    A typical revert empty capsule entry node would then look something like this

    empty-capsule {
            type = "efi-empty-capsule";
            revert-capsule;
    };

    The empty capsules do not have any input payload image.

    .. _`UEFI specification`: https://uefi.org/sites/default/files/resources/UEFI_Spec_2_10_Aug29.pdf
    .. _`Dependable Boot specification`: https://git.codelinaro.org/linaro/dependable-boot/mbfw/uploads/6f7ddfe3be24e18d4319e108a758d02e/mbfw.pdf
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        self.accept = 0
        self.revert = 0

    def ReadNode(self):
        super().ReadNode()

        self.image_guid = fdt_util.GetString(self._node, 'image-guid')
        self.accept = fdt_util.GetBool(self._node, 'accept-capsule')
        self.revert = fdt_util.GetBool(self._node, 'revert-capsule')

        if self.accept and not self.image_guid:
            self.Raise('Image GUID needed for generating accept capsule')

        if self.accept and self.revert:
            self.Raise('Need to enable either Accept or Revert capsule')

    def BuildSectionData(self, required):
        def get_binman_test_guid(type_str):
            TYPE_TO_GUID = {
                'binman-test' : '09d7cf52-0720-4710-91d1-08469b7fe9c8'
            }
            return TYPE_TO_GUID[type_str]

        uniq = self.GetUniqueName()
        outfile = self._filename if self._filename else 'capsule.%s' % uniq
        capsule_fname = tools.get_output_filename(outfile)
        guid = self.image_guid
        if self.image_guid == "binman-test":
            guid = get_binman_test_guid('binman-test')

        ret = self.mkeficapsule.generate_empty_capsule(self.accept, self.revert,
                                                       guid, capsule_fname)
        if ret is not None:
            return tools.read_file(capsule_fname)

    def AddBintools(self, btools):
        self.mkeficapsule = self.AddBintool(btools, 'mkeficapsule')

# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2023 Linaro Limited
#
# Entry-type module for producing a EFI capsule
#

from collections import OrderedDict
import os

from binman.entry import Entry
from binman.etype.section import Entry_section
from dtoc import fdt_util
from u_boot_pylib import tools

class Entry_efi_capsule(Entry_section):
    """Entry for generating EFI capsules

    This is an entry for generating EFI capsules.

    The parameters needed for generation of the capsules can
    either be provided as properties in the entry.

    Properties / Entry arguments:
    - image-index: Unique number for identifying corresponding
      payload image. Number between 1 and descriptor count, i.e.
      the total number of firmware images that can be updated.
    - image-type-id: Image GUID which will be used for identifying the
      updatable image on the board.
    - hardware-instance: Optional number for identifying unique
      hardware instance of a device in the system. Default value of 0
      for images where value is not to be used.
    - fw-version: Optional value of image version that can be put on
      the capsule through the Firmware Management Protocol(FMP) header.
    - monotonic-count: Count used when signing an image.
    - private-key: Path to PEM formatted .key private key file.
    - pub-key-cert: Path to PEM formatted .crt public key certificate
      file.
    - capoemflags - Optional OEM flags to be passed through capsule header.
    - filename: Optional path to the output capsule file. A capsule is a
      continuous set of data as defined by the EFI specification. Refer
      to the specification for more details.

    For more details on the description of the capsule format, and the capsule
    update functionality, refer Section 8.5 and Chapter 23 in the UEFI
    specification.
    https://uefi.org/sites/default/files/resources/UEFI_Spec_2_10_Aug29.pdf

    The capsule parameters like image index and image GUID are passed as
    properties in the entry. The payload to be used in the capsule is to be
    provided as a subnode of the capsule entry.

    A typical capsule entry node would then look something like this

    capsule {
            type = "efi-capsule";
            image-index = <0x1>;
            /* Image GUID for testing capsule update */
            image-type-id = "09D7CF52-0720-4710-91D1-08469B7FE9C8";
            hardware-instance = <0x0>;
            private-key = "tools/binman/test/key.key";
            pub-key-cert = "tools/binman/test/key.pem";
            capoemflags = <0x8000>;

            u-boot {
            };
    };

    In the above example, the capsule payload is the u-boot image. The
    capsule entry would read the contents of the payload and put them
    into the capsule. Any external file can also be specified as the
    payload using the blob-ext subnode.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        self.required_props = ['image-index', 'image-type-id']
        self.image_index = 0
        self.image_guid = ''
        self.hardware_instance = 0
        self.monotonic_count = 0
        self.fw_version = 0
        self.capoemflags = 0
        self.private_key = ''
        self.pub_key_cert = ''
        self.auth = 0
        self.capsule_fname = ''
        self._entries = OrderedDict()

    def ReadNode(self):
        self.ReadEntries()
        super().ReadNode()

        self.image_index = fdt_util.GetInt(self._node, 'image-index')
        self.image_guid = fdt_util.GetString(self._node, 'image-type-id')
        self.fw_version = fdt_util.GetInt(self._node, 'fw-version')
        self.hardware_instance = fdt_util.GetInt(self._node, 'hardware-instance')
        self.monotonic_count = fdt_util.GetInt(self._node, 'monotonic-count')
        self.capoemflags = fdt_util.GetInt(self._node, 'capoemflags')

        self.private_key = fdt_util.GetString(self._node, 'private-key')
        self.pub_key_cert = fdt_util.GetString(self._node, 'pub-key-cert')
        if ((self.private_key and not self.pub_key_cert) or (self.pub_key_cert and not self.private_key)):
            self.Raise('Both private key and public key certificate need to be provided')
        elif not (self.private_key and self.pub_key_cert):
            self.auth = 0
        else:
            self.auth = 1

    def ReadEntries(self):
        """Read the subnode to get the payload for this capsule"""
        # We can have a single payload per capsule
        if len(self._node.subnodes) != 1:
            self.Raise('The capsule entry expects a single subnode for payload')

        for node in self._node.subnodes:
            entry = Entry.Create(self, node)
            entry.ReadNode()
            self._entries[entry.name] = entry

    def _GenCapsule(self):
        return self.mkeficapsule.generate_capsule(self.image_index,
                                                  self.image_guid,
                                                  self.hardware_instance,
                                                  self.payload,
                                                  self.capsule_fname,
                                                  self.private_key,
                                                  self.pub_key_cert,
                                                  self.monotonic_count,
                                                  self.fw_version,
                                                  self.capoemflags)

    def BuildSectionData(self, required):
        if self.auth:
            if not os.path.isabs(self.private_key):
                self.private_key =  tools.get_input_filename(self.private_key)
            if not os.path.isabs(self.pub_key_cert):
                self.pub_key_cert = tools.get_input_filename(self.pub_key_cert)
        data, self.payload, _ = self.collect_contents_to_file(
            self._entries.values(), 'capsule_payload')
        outfile = self._filename if self._filename else self._node.name
        self.capsule_fname = tools.get_output_filename(outfile)
        if self._GenCapsule() is not None:
            os.remove(self.payload)
            return tools.read_file(self.capsule_fname)

    def ProcessContents(self):
        ok = super().ProcessContents()
        data = self.BuildSectionData(True)
        ok2 = self.ProcessContentsUpdate(data)
        return ok and ok2

    def SetImagePos(self, image_pos):
        upto = 0
        for entry in self.GetEntries().values():
            entry.SetOffsetSize(upto, None)
            upto += entry.size

        super().SetImagePos(image_pos)

    def AddBintools(self, btools):
        self.mkeficapsule = self.AddBintool(btools, 'mkeficapsule')

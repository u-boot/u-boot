# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2023 Linaro Limited
#
# Entry-type module for producing a capsule
#

import os

from binman.entry import Entry
from dtoc import fdt_util
from u_boot_pylib import tools

class Entry_capsule(Entry):
    """Entry for generating EFI capsules

    This is an entry for generating EFI capsules.

    The parameters needed for generation of the capsules can
    either be provided separately, or through a config file.

    Properties / Entry arguments:
    - cfg-file: Config file for providing capsule
      parameters. These are parameters needed for generating the
      capsules. The parameters can be listed by running the
      './tools/mkeficapsule -h' command.
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
    - monotomic-count: Count used when signing an image.
    - private-key: Path to PEM formatted .key private key file.
    - pub-key-cert: Path to PEM formatted .crt public key certificate
      file.
    - filename: Path to the input(payload) file. File can be any
      format, a binary or an elf, platform specific.
    - capsule: Path to the output capsule file. A capsule is a
      continous set of data as defined by the EFI specification. Refer
      to the specification for more details.

    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        self.image_index = 0
        self.image_guid = ""
        self.hardware_instance = 0
        self.monotonic_count = 0
        self.fw_version = 0
        self.private_key = ""
        self.pub_key_cert = ""
        self.auth = 0
        self.payload = ""
        self.capsule_fname = ""

    def ReadNode(self):
        super().ReadNode()

        self.cfg_file = fdt_util.GetString(self._node, 'cfg-file')
        if not self.cfg_file:
            self.image_index = fdt_util.GetInt(self._node, 'image-index')
            if not self.image_index:
                self.Raise('mkeficapsule must be provided an Image Index')

            self.image_guid = fdt_util.GetString(self._node, 'image-type-id')
            if not self.image_guid:
                self.Raise('mkeficapsule must be provided an Image GUID')

            self.fw_version = fdt_util.GetInt(self._node, 'fw-version')
            self.hardware_instance = fdt_util.GetInt(self._node, 'hardware-instance')
            self.monotonic_count = fdt_util.GetInt(self._node, 'monotonic-count')

            self.private_key = fdt_util.GetString(self._node, 'private-key')
            self.pub_key_cert = fdt_util.GetString(self._node, 'pub-key-cert')

            if ((self.private_key and not self.pub_key_cert) or (self.pub_key_cert and not self.private_key)):
                self.Raise('Both private-key and public key Certificate need to be provided')
            elif not (self.private_key and self.pub_key_cert):
                self.auth = 0
            else:
                self.auth = 1

            self.payload = fdt_util.GetString(self._node, 'filename')
            if not self.payload:
                self.Raise('mkeficapsule must be provided an input filename(payload)')

            if not os.path.isabs(self.payload):
                self.payload_path = tools.get_input_filename(self.payload)
                if not os.path.exists(self.payload_path):
                    self.Raise('Cannot resolve path to the input filename(payload)')
                else:
                    self.payload = self.payload_path

            self.capsule_fname = fdt_util.GetString(self._node, 'capsule')
            if not self.capsule_fname:
                self.Raise('Specify the output capsule file')

            if not os.path.isabs(self.capsule_fname):
                self.capsule_path = tools.get_output_filename(self.capsule_fname)
                self.capsule_fname = self.capsule_path

    def _GenCapsule(self):
        if self.cfg_file:
            return self.mkeficapsule.capsule_cfg_file(self.cfg_file)
        elif self.auth:
            return self.mkeficapsule.cmdline_auth_capsule(self.image_index,
                                                          self.image_guid,
                                                          self.hardware_instance,
                                                          self.monotonic_count,
                                                          self.private_key,
                                                          self.pub_key_cert,
                                                          self.payload,
                                                          self.capsule_fname,
                                                          self.fw_version)
        else:
            return self.mkeficapsule.cmdline_capsule(self.image_index,
                                                     self.image_guid,
                                                     self.hardware_instance,
                                                     self.payload,
                                                     self.capsule_fname,
                                                     self.fw_version)

    def ObtainContents(self):
        self.SetContents(tools.to_bytes(self._GenCapsule()))
        return True

    def AddBintools(self, btools):
        self.mkeficapsule = self.AddBintool(btools, 'mkeficapsule')

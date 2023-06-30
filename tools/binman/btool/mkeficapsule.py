# SPDX-License-Identifier: GPL-2.0+
# Copyright 2023 Linaro Limited
#
"""Bintool implementation for mkeficapsule tool

mkeficapsule is a tool used for generating EFI capsules.

The following are the command-line options to be provided
to the tool
Usage: mkeficapsule [options] <image blob> <output file>
Options:
	-g, --guid <guid string>    guid for image blob type
	-i, --index <index>         update image index
	-I, --instance <instance>   update hardware instance
	-p, --private-key <privkey file>  private key file
	-c, --certificate <cert file>     signer's certificate file
	-m, --monotonic-count <count>     monotonic count
	-d, --dump_sig              dump signature (*.p7)
	-A, --fw-accept  firmware accept capsule, requires GUID, no image blob
	-R, --fw-revert  firmware revert capsule, takes no GUID, no image blob
	-o, --capoemflag Capsule OEM Flag, an integer between 0x0000 and 0xffff
	-f, --cfg-file <config file> config file with capsule parameters
	-h, --help                  print a help message

"""

from binman import bintool

class Bintoolmkeficapsule(bintool.Bintool):
    """Handles the 'mkeficapsule' tool

    This bintool is used for generating the EFI capsules. The
    capsule generation parameters can either be specified through
    command-line, or through a config file.

    """
    def __init__(self, name):
        super().__init__(name, 'mkeficapsule tool for generating capsules')

    def capsule_cfg_file(self, cfg_file):

        args = [
            f'--cfg-file={cfg_file}'
        ]
        self.run_cmd(*args)

    def cmdline_capsule(self, image_index, image_guid, hardware_instance,
                        payload, output_fname):

        args = [
            f'--index={image_index}',
            f'--guid={image_guid}',
            f'--instance={hardware_instance}',
            payload,
            output_fname
        ]
        self.run_cmd(*args)

    def cmdline_auth_capsule(self, image_index, image_guid, hardware_instance,
                             monotonic_count, priv_key, pub_key,
                             payload, output_fname):

        args = [
            f'--index={image_index}',
            f'--guid={image_guid}',
            f'--instance={hardware_instance}',
            f'--monotonic-count={monotonic_count}',
            f'--private-key={priv_key}',
            f'--certificate={pub_key}',
            payload,
            output_fname
        ]
        self.run_cmd(*args)

    def fetch(self, method):
        """Fetch handler for mkeficapsule

        This builds the tool from source

        Returns:
            tuple:
                str: Filename of fetched file to copy to a suitable directory
                str: Name of temp directory to remove, or None
        """
        if method != bintool.FETCH_BUILD:
            return None
        result = self.build_from_git(
            'https://source.denx.de/u-boot/u-boot.git',
            'tools',
            'tools/mkeficapsule')
        return result

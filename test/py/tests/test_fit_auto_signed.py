# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2022 Massimo Pegorer

"""
Test that mkimage generates auto-FIT with signatures and/or hashes as expected.

The mkimage tool can create auto generated (i.e. without an ITS file
provided as input) FIT in three different flavours: with crc32 checksums
of 'images' subnodes; with signatures of 'images' subnodes; with sha1
hashes of 'images' subnodes and signatures of 'configurations' subnodes.
This test verifies that auto-FIT are generated as expected, in all of
the three flavours, including check of hashes and signatures (except for
configurations ones).

The test does not run the sandbox. It only checks the host tool mkimage.
"""

import os
import pytest
import utils
import binascii
from Cryptodome.Hash import SHA1
from Cryptodome.Hash import SHA256
from Cryptodome.PublicKey import RSA
from Cryptodome.Signature import pkcs1_15

class SignedFitHelper(object):
    """Helper to manipulate a FIT with signed/hashed images/configs."""
    def __init__(self, ubman, file_name):
        self.fit = file_name
        self.ubman = ubman
        self.images_nodes = set()
        self.confgs_nodes = set()

    def __fdt_list(self, path):
        return utils.run_and_log(self.ubman,
            f'fdtget -l {self.fit} {path}')

    def __fdt_get_string(self, node, prop):
        return utils.run_and_log(self.ubman,
            f'fdtget -ts {self.fit} {node} {prop}')

    def __fdt_get_binary(self, node, prop):
        numbers = utils.run_and_log(self.ubman,
            f'fdtget -tbi {self.fit} {node} {prop}')

        bignum = bytearray()
        for little_num in numbers.split():
            bignum.append(int(little_num))

        return bignum

    def build_nodes_sets(self):
        """Fill sets with FIT images and configurations subnodes."""
        for node in self.__fdt_list('/images').split():
            subnode = f'/images/{node}'
            self.images_nodes.add(subnode)

        for node in self.__fdt_list('/configurations').split():
            subnode = f'/configurations/{node}'
            self.confgs_nodes.add(subnode)

        return len(self.images_nodes) + len(self.confgs_nodes)

    def check_fit_crc32_images(self):
        """Test that all images in the set are hashed as expected.

        Each image must have an hash with algo=crc32 and hash value must match
        the one calculated over image data.
        """
        for node in self.images_nodes:
            algo = self.__fdt_get_string(f'{node}/hash', 'algo')
            assert algo == "crc32\n", "Missing expected crc32 image hash!"

            raw_crc32 = self.__fdt_get_binary(f'{node}/hash', 'value')
            raw_bin = self.__fdt_get_binary(node, 'data')
            assert raw_crc32 == (binascii.crc32(raw_bin) &
                0xffffffff).to_bytes(4, 'big'), "Wrong crc32 hash!"

    def check_fit_signed_images(self, key_name, sign_algo, verifier):
        """Test that all images in the set are signed as expected.

        Each image must have a signature with: key-name-hint matching key_name
        argument; algo matching sign_algo argument; value matching the one
        calculated over image data using verifier argument.
        """
        for node in self.images_nodes:
            hint = self.__fdt_get_string(f'{node}/signature', 'key-name-hint')
            assert hint == key_name + "\n", "Missing expected key name hint!"
            algo = self.__fdt_get_string(f'{node}/signature', 'algo')
            assert algo == sign_algo + "\n", "Missing expected signature algo!"

            raw_sig = self.__fdt_get_binary(f'{node}/signature', 'value')
            raw_bin = self.__fdt_get_binary(node, 'data')
            verifier.verify(SHA256.new(raw_bin), bytes(raw_sig))

    def check_fit_signed_confgs(self, key_name, sign_algo):
        """Test that all configs are signed, and images hashed, as expected.

        Each image must have an hash with algo=sha1 and hash value must match
        the one calculated over image data. Each configuration must have a
        signature with key-name-hint matching key_name argument and algo
        matching sign_algo argument.
        TODO: configurations signature checking.
        """
        for node in self.images_nodes:
            algo = self.__fdt_get_string(f'{node}/hash', 'algo')
            assert algo == "sha1\n", "Missing expected sha1 image hash!"

            raw_hash = self.__fdt_get_binary(f'{node}/hash', 'value')
            raw_bin = self.__fdt_get_binary(node, 'data')
            assert raw_hash == SHA1.new(raw_bin).digest(), "Wrong sha1 hash!"

        for node in self.confgs_nodes:
            hint = self.__fdt_get_string(f'{node}/signature', 'key-name-hint')
            assert hint == key_name + "\n", "Missing expected key name hint!"
            algo = self.__fdt_get_string(f'{node}/signature', 'algo')
            assert algo == sign_algo + "\n", "Missing expected signature algo!"

    def check_fit_loadables(self, bl31present, teepresent):
        """Test that loadables contains both kernel, TFA BL31, TEE entries.

        Each configuration must have a loadables property which lists both
        kernel-1, tfa-bl31-1 and tee-1 strings in the string list.
        """
        if bl31present:
            assert "/images/tfa-bl31-1" in self.images_nodes
        else:
            assert "/images/tfa-bl31-1" not in self.images_nodes
        if teepresent:
            assert "/images/tee-1" in self.images_nodes
        else:
            assert "/images/tee-1" not in self.images_nodes
        for node in self.confgs_nodes:
            loadables = self.__fdt_get_string(f'{node}', 'loadables')
            assert "kernel-1" in loadables
            if bl31present:
                assert "tfa-bl31-1" in loadables
            else:
                assert "tfa-bl31-1" not in loadables
            if teepresent:
                assert "tee-1" in loadables
            else:
                assert "tee-1" not in loadables

@pytest.mark.buildconfigspec('fit_signature')
@pytest.mark.requiredtool('fdtget')
def test_fit_auto_signed(ubman):
    def generate_and_check_fit_image(cmd, crc=False, simgs=False, scfgs=False, bl31present=False, teepresent=False, key_name="", sign_algo="", verifier=""):
        """Generate fitImage and test for expected entries.

        Generate a fitImage and test whether suitable entries are part of
        the generated fitImage. Test whether checksums and signatures are
        part of the generated fitImage.
        """
        mkimage = ubman.config.build_dir + '/tools/mkimage'
        utils.run_and_log(ubman, mkimage + cmd)

        fit = SignedFitHelper(ubman, fit_file)
        if fit.build_nodes_sets() == 0:
            raise ValueError(f'FIT has no "/image" nor "/configuration" nodes, test settings: cmd={cmd} crc={crc} simgs={simgs} scfgs={scfgs} bl31present={bl31present} teepresent={teepresent} key_name={key_name} sign_algo={sign_algo} verifier={verifier}')
        if crc:
            fit.check_fit_crc32_images()
        if simgs:
            fit.check_fit_signed_images(key_name, sign_algo, verifier)
        if scfgs:
            fit.check_fit_signed_confgs(key_name, sign_algo)

        fit.check_fit_loadables(bl31present, teepresent)

    """Test that mkimage generates auto-FIT with signatures/hashes as expected.

    The mkimage tool can create auto generated (i.e. without an ITS file
    provided as input) FIT in three different flavours: with crc32 checksums
    of 'images' subnodes; with signatures of 'images' subnodes; with sha1
    hashes of 'images' subnodes and signatures of 'configurations' subnodes.
    This test verifies that auto-FIT are generated as expected, in all of
    the three flavours, including check of hashes and signatures (except for
    configurations ones).

    The test does not run the sandbox. It only checks the host tool mkimage.
    """
    tempdir = os.path.join(ubman.config.result_dir, 'auto_fit')
    os.makedirs(tempdir, exist_ok=True)
    kernel_file = f'{tempdir}/vmlinuz'
    dt1_file = f'{tempdir}/dt-1.dtb'
    dt2_file = f'{tempdir}/dt-2.dtb'
    tfa_file = f'{tempdir}/tfa-bl31.bin'
    tee_file = f'{tempdir}/tee.bin'
    key_name = 'sign-key'
    sign_algo = 'sha256,rsa4096'
    key_file = f'{tempdir}/{key_name}.key'
    fit_file = f'{tempdir}/test.fit'

    # Create a fake kernel image and two dtb files with random data
    with open(kernel_file, 'wb') as fd:
        fd.write(os.urandom(512))

    with open(dt1_file, 'wb') as fd:
        fd.write(os.urandom(256))

    with open(dt2_file, 'wb') as fd:
        fd.write(os.urandom(256))

    with open(tfa_file, 'wb') as fd:
        fd.write(os.urandom(256))

    with open(tee_file, 'wb') as fd:
        fd.write(os.urandom(256))

    # Create 4096 RSA key and write to file to be read by mkimage
    key = RSA.generate(bits=4096)
    verifier = pkcs1_15.new(key)

    with open(key_file, 'w') as fd:
        fd.write(str(key.export_key(format='PEM').decode('ascii')))

    b_args = " -d" + kernel_file + " -b" + dt1_file + " -b" + dt2_file
    s_args = " -k" + tempdir + " -g" + key_name + " -o" + sign_algo

    # 1 - Create auto FIT with images crc32 checksum, and verify it
    generate_and_check_fit_image(' -fauto' + b_args + " " + fit_file,
                                 crc=True)

    # 2 - Create auto FIT with signed images, and verify it
    generate_and_check_fit_image(' -fauto' + b_args + s_args + " " + fit_file,
                                 simgs=True,
                                 key_name=key_name, sign_algo=sign_algo, verifier=verifier)

    # 3 - Create auto FIT with signed configs and hashed images, and verify it
    generate_and_check_fit_image(' -fauto-conf' + b_args + s_args + " " + fit_file,
                                 scfgs=True,
                                 key_name=key_name, sign_algo=sign_algo)

    # Run the same tests as 1/2/3 above, but this time with TFA BL31
    # options -y tfa-bl31.bin -Y 0x12340000 to cover both mkimage with
    # and without TFA BL31 use cases.
    b_args = " -d" + kernel_file + " -b" + dt1_file + " -b" + dt2_file + " -y" + tfa_file + " -Y 0x12340000"

    # 4 - Create auto FIT with images crc32 checksum, and verify it
    generate_and_check_fit_image(' -fauto' + b_args + " " + fit_file,
                                 crc=True, bl31present=True)

    # 5 - Create auto FIT with signed images, and verify it
    generate_and_check_fit_image(' -fauto' + b_args + s_args + " " + fit_file,
                                 simgs=True, bl31present=True,
                                 key_name=key_name, sign_algo=sign_algo, verifier=verifier)

    # 6 - Create auto FIT with signed configs and hashed images, and verify it
    generate_and_check_fit_image(' -fauto-conf' + b_args + s_args + " " + fit_file,
                                 scfgs=True, bl31present=True,
                                 key_name=key_name, sign_algo=sign_algo)

    # Run the same tests as 1/2/3 above, but this time with TEE
    # options -z tee.bin -Z 0x56780000 to cover both mkimage with
    # and without TEE use cases.
    b_args = " -d" + kernel_file + " -b" + dt1_file + " -b" + dt2_file + " -z" + tee_file + " -Z 0x56780000"

    # 7 - Create auto FIT with images crc32 checksum, and verify it
    generate_and_check_fit_image(' -fauto' + b_args + " " + fit_file,
                                 crc=True, teepresent=True)

    # 8 - Create auto FIT with signed images, and verify it
    generate_and_check_fit_image(' -fauto' + b_args + s_args + " " + fit_file,
                                 simgs=True, teepresent=True,
                                 key_name=key_name, sign_algo=sign_algo, verifier=verifier)

    # 9 - Create auto FIT with signed configs and hashed images, and verify it
    generate_and_check_fit_image(' -fauto-conf' + b_args + s_args + " " + fit_file,
                                 scfgs=True, teepresent=True,
                                 key_name=key_name, sign_algo=sign_algo)

    # Run the same tests as 1/2/3 above, but this time with both
    # TFA BL31 and TEE options -y tfa-bl31.bin -Y 0x12340000 and
    # -z tee.bin -Z 0x56780000 to cover both mkimage with and
    # without both TFA BL31 and TEE use cases.
    b_args = " -d" + kernel_file + " -b" + dt1_file + " -b" + dt2_file + " -y" + tfa_file + " -Y 0x12340000" + " -z" + tee_file + " -Z 0x56780000"

    # 10 - Create auto FIT with images crc32 checksum, and verify it
    generate_and_check_fit_image(' -fauto' + b_args + " " + fit_file,
                                 crc=True, bl31present=True, teepresent=True)

    # 11 - Create auto FIT with signed images, and verify it
    generate_and_check_fit_image(' -fauto' + b_args + s_args + " " + fit_file,
                                 simgs=True, bl31present=True, teepresent=True,
                                 key_name=key_name, sign_algo=sign_algo, verifier=verifier)

    # 12 - Create auto FIT with signed configs and hashed images, and verify it
    generate_and_check_fit_image(' -fauto-conf' + b_args + s_args + " " + fit_file,
                                 scfgs=True, bl31present=True, teepresent=True,
                                 key_name=key_name, sign_algo=sign_algo)

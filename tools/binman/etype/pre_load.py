# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2022 Softathome
# Written by Philippe Reynes <philippe.reynes@softathome.com>
#
# Entry-type for the global header
#

import os
import struct
from dtoc import fdt_util
from u_boot_pylib import tools

from binman.entry import Entry
from binman.etype.collection import Entry_collection
from binman.entry import EntryArg

from Cryptodome.Hash import SHA256, SHA384, SHA512
from Cryptodome.PublicKey import RSA
from Cryptodome.PublicKey import ECC
from Cryptodome.Signature import pkcs1_15
from Cryptodome.Signature import pss
from Cryptodome.Signature import DSS

PRE_LOAD_MAGIC = b'UBSH'

RSAS = {
    'rsa1024': 1024 / 8,
    'rsa2048': 2048 / 8,
    'rsa4096': 4096 / 8
}

ECDSAS = {
    'ecdsa256': 256 / 8 * 2,
    'ecdsa384': 384 / 8 * 2,
    'ecdsa521': (521 + 7) / 8 * 2
}

SHAS = {
    'sha256': SHA256,
    'sha384': SHA384,
    'sha512': SHA512
}

class Entry_pre_load(Entry_collection):
    """Pre load image header

    Properties / Entry arguments:
        - pre-load-key-path: Path of the directory that store key (provided by
          the environment variable PRE_LOAD_KEY_PATH)
        - content: List of phandles to entries to sign
        - algo-name: Hash and signature algo to use for the signature
        - padding-name: Name of the padding (pkcs-1.5 or pss)
        - key-name: Filename of the private key to sign
        - header-size: Total size of the header
        - version: Version of the header

    This entry creates a pre-load header that contains a global
    image signature.

    For example, this creates an image with a pre-load header and a binary::

        binman {
            image2 {
                filename = "sandbox.bin";

                pre-load {
                    content = <&image>;
                    algo-name = "sha256,rsa2048";
                    padding-name = "pss";
                    key-name = "private.pem";
                    header-size = <4096>;
                    version = <1>;
                };

                image: blob-ext {
                    filename = "sandbox.itb";
                };
            };
        };
    """

    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        self.algo_name = fdt_util.GetString(self._node, 'algo-name')
        self.padding_name = fdt_util.GetString(self._node, 'padding-name')
        self.key_name = fdt_util.GetString(self._node, 'key-name')
        self.header_size = fdt_util.GetInt(self._node, 'header-size')
        self.version = fdt_util.GetInt(self._node, 'version')

    def ReadNode(self):
        super().ReadNode()
        self.key_path, = self.GetEntryArgsOrProps(
            [EntryArg('pre-load-key-path', str)])
        if self.key_path is None:
            self.key_path = ''

    def _CreateHeaderRsa(self, hash_name, sign_name, padding_name, key_name):
        # Check hash and signature name/type
        if hash_name not in SHAS:
            self.Raise(hash_name + " is not supported")

        # Read the key
        key = RSA.import_key(tools.read_file(key_name))

        # Check if the key has the expected size
        if key.size_in_bytes() != RSAS[sign_name]:
            self.Raise("The key " + self.key_name + " doesn't have the expected size")

        # Compute the hash
        hash_image = SHAS[hash_name].new()
        hash_image.update(self.image)

        # Compute the signature
        if padding_name is None:
            padding_name = "pkcs-1.5"
        padding = None
        padding_args = None
        if padding_name == "pss":
            salt_len = key.size_in_bytes() - hash_image.digest_size - 2
            padding = pss
            padding_args = {'salt_bytes': salt_len}
        elif padding_name == "pkcs-1.5":
            padding = pkcs1_15
            padding_args = {}
        else:
            self.Raise(padding_name + " is not supported")

        sig = padding.new(key, **padding_args).sign(hash_image)

        hash_sig = SHA256.new()
        hash_sig.update(sig)

        version = self.version
        header_size = self.header_size
        image_size = len(self.image)
        ofs_img_sig = 64 + len(sig)
        flags = 0
        reserved0 = 0
        reserved1 = 0

        first_header = struct.pack('>4sIIIIIII32s', PRE_LOAD_MAGIC,
                                   version, header_size, image_size,
                                   ofs_img_sig, flags, reserved0,
                                   reserved1, hash_sig.digest())

        hash_first_header = SHAS[hash_name].new()
        hash_first_header.update(first_header)
        sig_first_header = padding.new(key, **padding_args).sign(hash_first_header)

        data = first_header + sig_first_header + sig
        pad  = bytearray(self.header_size - len(data))

        return data + pad

    def _CreateHeaderEcdsa(self, hash_name, sign_name, key_name):
        # Check hash and signature name/type
        if hash_name not in SHAS:
            self.Raise(hash_name + " is not supported")

        # Read the key
        key = ECC.import_key(tools.read_file(key_name))

        # Check if the key has the expected size
        if key.pointQ.size_in_bytes() * 2 != ECDSAS[sign_name]:
            self.Raise("The key " + self.key_name + " doesn't have the expected size")

        # Compute the hash
        hash_image = SHAS[hash_name].new()
        hash_image.update(self.image)

        # Compute the signature
        signer = DSS.new(key, 'fips-186-3')
        sig = signer.sign(hash_image)

        hash_sig = SHA256.new()
        hash_sig.update(sig)

        version = self.version
        header_size = self.header_size
        image_size = len(self.image)
        ofs_img_sig = 64 + len(sig)
        flags = 0
        reserved0 = 0
        reserved1 = 0

        first_header = struct.pack('>4sIIIIIII32s', PRE_LOAD_MAGIC,
                                   version, header_size, image_size,
                                   ofs_img_sig, flags, reserved0,
                                   reserved1, hash_sig.digest())

        hash_first_header = SHAS[hash_name].new()
        hash_first_header.update(first_header)
        sig_first_header = signer.sign(hash_first_header)

        data = first_header + sig_first_header + sig
        pad  = bytearray(self.header_size - len(data))

        return data + pad

    def _CreateHeader(self):
        """Create a pre load header"""
        hash_name, sign_name = self.algo_name.split(',')
        padding_name = self.padding_name
        key_name = os.path.join(self.key_path, self.key_name)

        if sign_name in RSAS:
            return self._CreateHeaderRsa(hash_name, sign_name, padding_name, key_name)

        if sign_name in ECDSAS:
            return self._CreateHeaderEcdsa(hash_name, sign_name, key_name)

        self.Raise(sign_name + " is not supported")

    def ObtainContents(self):
        """Create a placeholder for the header"""
        self.SetContents(tools.get_bytes(0, self.header_size))
        return True

    def ProcessContents(self):
        self.image = self.GetContents(True)
        data = self._CreateHeader()
        return self.ProcessContentsUpdate(data)

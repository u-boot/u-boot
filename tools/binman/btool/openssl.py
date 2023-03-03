# SPDX-License-Identifier: GPL-2.0+
# Copyright 2022 Google LLC
#
"""Bintool implementation for openssl

openssl provides a number of features useful for signing images

Documentation is at https://www.coreboot.org/CBFS

Source code is at https://www.openssl.org/
"""

import hashlib

from binman import bintool
from u_boot_pylib import tools

class Bintoolopenssl(bintool.Bintool):
    """openssl tool

    This bintool supports creating new openssl certificates.

    It also supports fetching a binary openssl

    Documentation about openssl is at https://www.openssl.org/
    """
    def __init__(self, name):
        super().__init__(
            name, 'openssl cryptography toolkit',
            version_regex=r'OpenSSL (.*) \(', version_args='version')

    def x509_cert(self, cert_fname, input_fname, key_fname, cn, revision,
                  config_fname):
        """Create a certificate

        Args:
            cert_fname (str): Filename of certificate to create
            input_fname (str): Filename containing data to sign
            key_fname (str): Filename of .pem file
            cn (str): Common name
            revision (int): Revision number
            config_fname (str): Filename to write fconfig into

        Returns:
            str: Tool output
        """
        indata = tools.read_file(input_fname)
        hashval = hashlib.sha512(indata).hexdigest()
        with open(config_fname, 'w', encoding='utf-8') as outf:
            print(f'''[ req ]
distinguished_name     = req_distinguished_name
x509_extensions        = v3_ca
prompt                 = no
dirstring_type         = nobmp

[ req_distinguished_name ]
CN                     = {cert_fname}

[ v3_ca ]
basicConstraints       = CA:true
1.3.6.1.4.1.294.1.3    = ASN1:SEQUENCE:swrv
1.3.6.1.4.1.294.1.34   = ASN1:SEQUENCE:sysfw_image_integrity

[ swrv ]
swrv = INTEGER:{revision}

[ sysfw_image_integrity ]
shaType                = OID:2.16.840.1.101.3.4.2.3
shaValue               = FORMAT:HEX,OCT:{hashval}
imageSize              = INTEGER:{len(indata)}
''', file=outf)
        args = ['req', '-new', '-x509', '-key', key_fname, '-nodes',
                '-outform', 'DER', '-out', cert_fname, '-config', config_fname,
                '-sha512']
        return self.run_cmd(*args)

    def fetch(self, method):
        """Fetch handler for openssl

        This installs the openssl package using the apt utility.

        Args:
            method (FETCH_...): Method to use

        Returns:
            True if the file was fetched and now installed, None if a method
            other than FETCH_BIN was requested

        Raises:
            Valuerror: Fetching could not be completed
        """
        if method != bintool.FETCH_BIN:
            return None
        return self.apt_install('openssl')

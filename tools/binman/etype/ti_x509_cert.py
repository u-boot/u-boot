# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2018 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#

# Support for a TI x509 certificate for signing K3 devices

from subprocess import Popen, PIPE
from sys import stderr, stdout
import os
import tempfile

from Crypto.PublicKey import RSA

from binman.etype.collection import Entry_collection
from dtoc import fdt_util
from patman import tools

class Entry_ti_x509_cert(Entry_collection):
    """An entry which contains an x509 certificate binary signed with 1024 bit RSA key

    Properties / Entry arguments:
        - content: Phandle of binary to generate signature for
        - key_file: File with key inside it. If not provided, script generates RSA degenrate key
        - core: Target core ID on which image would be running
        - load: Target load address of the binary in hex
    
    Output files:
        - certificate.bin: Signed certificate binary"""

    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        self.key_file = fdt_util.GetString(self._node, 'key-file', "")
        self.core = fdt_util.GetInt(self._node, 'core', 0)
        self.load_addr = fdt_util.GetInt(self._node, 'load', 0x41c00000)
        self.cert = fdt_util.GetString(self._node, 'cert', 'certificate.bin')

        # temporary directory for intermediate files
        self.outdir = tempfile.mkdtemp(prefix='binman.x509.')

    def ReadNode(self):
        super().ReadNode()
        if self.key_file == "":
            self.key_int_file = os.path.join(self.outdir, 'eckey.pem')
            self.GenerateDegenKey()
        else:
            self.key_int_file = self.key_file

    def ObtainContents(self):
        self.image = self.GetContents(False)
        if self.image is None:
            return False
        self.image_file = os.path.join(self.outdir, 'x509.image')
        with open(self.image_file, 'wb') as f:
            f.write(self.image)
        self.cert_data = self._TICreateCertificateLegacy()
        self.SetContents(self.cert_data)
        return True
    
    def ProcessContents(self):
        # The blob may have changed due to WriteSymbols()
        return super().ProcessContentsUpdate(self.cert_data)
    
    def _TICreateCertificateLegacy(self):
        """Create certificate for legacy boot flow"""

        sha_val = self.GetShaVal(self.image_file)
        bin_size = self.GetFileSize(self.image_file)
        addr = "%08x" % self.load_addr
        if self.core == 16:
            self.cert_type = 1
        else:
            self.cert_type = 2
        self.debug_type = 0
        self.bootcore_opts = 0

        self.GenerateTemplate()
        self.GenerateCertificate(bin_size, sha_val, addr)

        return tools.read_file(self.cert_file)

    def GetShaVal(self, binary_file):
        process = Popen(['openssl', 'dgst', '-sha512', '-hex',
                    binary_file], stdout=PIPE, stderr=PIPE)
        stdout, stderr = process.communicate()
        sha_val = stdout.split()[1]
        return sha_val
    
    def GetFileSize(self, binary_file):
        return os.path.getsize(binary_file)

    def ParseKey(self, inp_key, section):
        parsed_key = ""
        section_true = False
        with open(inp_key, 'r') as file:
            for line in file:
                if section in line:
                    section_true = True
                elif section_true:
                    if "    " not in line:
                        break
                    else:
                        parsed_key += line.replace(":", "").replace("    ", "")
        return parsed_key.replace("\n", "")
    
    def GenerateDegenKey(self):
        """Generate a 4096 bit RSA key"""
        # generates 1024 bit PEM encoded RSA key in PKCS#1 format
        private_key = RSA.generate(1024)
        self.key_pem_file = os.path.join(self.outdir, 'key.pem')
        with open(self.key_pem_file, 'wb') as f:
            f.write(private_key.exportKey('PEM'))
        
        self.key_text_file = os.path.join(self.outdir, 'key.txt')
        process = Popen(['openssl', 'rsa', '-in', self.key_pem_file,
                    '-text', '-out', self.key_text_file], stdout=PIPE, stderr=PIPE)
        stdout, stderr = process.communicate()

        DEGEN_MODULUS = self.ParseKey(self.key_text_file, "modulus")
        DEGEN_P = self.ParseKey(self.key_text_file, "prime1")
        DEGEN_Q = self.ParseKey(self.key_text_file, "prime2")
        DEGEN_COEFF = self.ParseKey(self.key_text_file, "coefficient")

        self.GenerateDegenTemplate()

        self.degen_key = os.path.join(self.outdir, 'x509.degenerateKey.txt')
        with open(self.degen_temp_file, 'r') as file_input:
            with open(self.degen_key, 'w') as file_output:
                for line in file_input:
                    s = line.replace("DEGEN_MODULUS", DEGEN_MODULUS).replace(
                        "DEGEN_P", DEGEN_P).replace("DEGEN_Q", DEGEN_Q).replace("DEGEN_COEFF", DEGEN_COEFF)
                    file_output.write(s)
        
        self.degen_key_der = os.path.join(
            self.outdir, 'x509.degenerateKey.der')
        process = Popen(['openssl', 'asn1parse', '-genconf', self.degen_key,
                         '-out', self.degen_key_der], stdout=PIPE, stderr=PIPE)
        stdout, stderr = process.communicate()

        process = Popen(['openssl', 'rsa', '-in', self.degen_key_der,
                         '-inform', 'DER', '-outform', 'PEM', '-out', self.key_int_file])
        stdout, stderr = process.communicate()
        
    def GenerateCertificate(self, bin_size, sha_val, addr):
        self.temp_x509 = os.path.join(self.outdir, 'x509.temp.cert')
        self.cert_file = os.path.join(self.outdir, 'x509.certificate.bin')

        with open(self.temp_x509, "w") as output_file:
            with open(self.temp_file, "r") as input_file:
                for line in input_file:
                    output_file.write(line.replace("TEST_IMAGE_LENGTH", str(bin_size)).replace("TEST_IMAGE_SHA_VAL", sha_val.decode("utf-8")).replace("TEST_CERT_TYPE", str(self.cert_type)).replace(
                        "TEST_BOOT_CORE_OPTS", str(self.bootcore_opts)).replace("TEST_BOOT_CORE", str(self.core)).replace("TEST_BOOT_ADDR", str(addr)).replace("TEST_DEBUG_TYPE", str(self.debug_type)))
        process = Popen(['openssl', 'req', '-new', '-x509', '-key', self.key_int_file, '-nodes', '-outform',
                        'DER', '-out', self.cert_file, '-config', self.temp_x509, '-sha512'], stdout=PIPE, stderr=PIPE)
        stdout, stderr = process.communicate()

    def GenerateDegenTemplate(self):
        self.degen_temp_file = os.path.join(self.outdir, 'x509.degen-template')
        with open(self.degen_temp_file, 'w+', encoding='utf-8') as f:
            degen_temp = """
asn1=SEQUENCE:rsa_key

[rsa_key]
version=INTEGER:0
modulus=INTEGER:0xDEGEN_MODULUS
pubExp=INTEGER:1
privExp=INTEGER:1
p=INTEGER:0xDEGEN_P
q=INTEGER:0xDEGEN_Q
e1=INTEGER:1
e2=INTEGER:1
coeff=INTEGER:0xDEGEN_COEFF"""
            f.write(degen_temp)

    def GenerateTemplate(self):
        self.temp_file = os.path.join(self.outdir, 'x509.template')
        with open(self.temp_file, 'w+', encoding='utf-8') as f:
            x509template = """
[ req ]
distinguished_name     = req_distinguished_name
x509_extensions        = v3_ca
prompt                 = no
dirstring_type         = nobmp

[ req_distinguished_name ]
C                      = US
ST                     = TX
L                      = Dallas
O                      = Texas Instruments Incorporated
OU                     = Processors
CN                     = TI support
emailAddress           = support@ti.com

[ v3_ca ]
basicConstraints = CA:true
1.3.6.1.4.1.294.1.1 = ASN1:SEQUENCE:boot_seq
1.3.6.1.4.1.294.1.2 = ASN1:SEQUENCE:image_integrity
1.3.6.1.4.1.294.1.3 = ASN1:SEQUENCE:swrv
# 1.3.6.1.4.1.294.1.4 = ASN1:SEQUENCE:encryption
1.3.6.1.4.1.294.1.8 = ASN1:SEQUENCE:debug

[ boot_seq ]
certType = INTEGER:TEST_CERT_TYPE
bootCore = INTEGER:TEST_BOOT_CORE
bootCoreOpts = INTEGER:TEST_BOOT_CORE_OPTS
destAddr = FORMAT:HEX,OCT:TEST_BOOT_ADDR
imageSize = INTEGER:TEST_IMAGE_LENGTH

[ image_integrity ]
shaType = OID:2.16.840.1.101.3.4.2.3
shaValue = FORMAT:HEX,OCT:TEST_IMAGE_SHA_VAL

[ swrv ]
swrv = INTEGER:0

# [ encryption ]
# initalVector = FORMAT:HEX,OCT:TEST_IMAGE_ENC_IV
# randomString = FORMAT:HEX,OCT:TEST_IMAGE_ENC_RS
# iterationCnt = INTEGER:TEST_IMAGE_KEY_DERIVE_INDEX
# salt = FORMAT:HEX,OCT:TEST_IMAGE_KEY_DERIVE_SALT

[ debug ]
debugUID = FORMAT:HEX,OCT:0000000000000000000000000000000000000000000000000000000000000000
debugType = INTEGER:TEST_DEBUG_TYPE
coreDbgEn = INTEGER:0
coreDbgSecEn = INTEGER:0"""
            f.write(x509template)

# SPDX-License-Identifier: GPL-2.0-only AND BSD-3-Clause
# Copyright (C) 2021-2023 Stephan Gerhold (GPL-2.0-only)
# MBN header format adapted from:
#   - signlk: https://git.linaro.org/landing-teams/working/qualcomm/signlk.git
#   - coreboot (util/qualcomm/mbn_tools.py, util/cbfstool/platform_fixups.c)
# Copyright (c) 2016, 2018, The Linux Foundation. All rights reserved. (BSD-3-Clause)
# See also:
#   - https://www.qualcomm.com/media/documents/files/secure-boot-and-image-authentication-technical-overview-v1-0.pdf
#   - https://www.qualcomm.com/media/documents/files/secure-boot-and-image-authentication-technical-overview-v2-0.pdf
from __future__ import annotations

import dataclasses
import hashlib
from dataclasses import dataclass
from io import BytesIO
from struct import Struct

import cert
import elf

# A typical Qualcomm firmware might have the following program headers:
#     LOAD off    0x00000800 vaddr 0x86400000 paddr 0x86400000 align 2**11
#          filesz 0x00001000 memsz 0x00001000 flags rwx
#
# The signed version will then look like:
#     NULL off    0x00000000 vaddr 0x00000000 paddr 0x00000000 align 2**0
#          filesz 0x000000e8 memsz 0x00000000 flags --- 7000000
#     NULL off    0x00001000 vaddr 0x86401000 paddr 0x86401000 align 2**12
#          filesz 0x00000988 memsz 0x00001000 flags --- 2200000
#     LOAD off    0x00002000 vaddr 0x86400000 paddr 0x86400000 align 2**11
#          filesz 0x00001000 memsz 0x00001000 flags rwx
#
# The second NULL program header with off 0x1000 and filesz 0x988 is the actual
# "hash table segment" or shortly "hash segment" (see Figure 2 on page 6 in the PDF).
# It contains the MBN header specified below, then a couple of hashes (e.g. SHA256):
#   1. Hash of ELF header and program headers
#   2. Empty hash for hash segment
#   3. Hashes for data of each memory segment (described by program header)
# Finally, it contains an RSA signature and the concatenated certificate chain.
#
# The first NULL program header is never loaded anywhere, because
# vaddr = paddr = memsz = 0. However, the "off" and "filesz" cover exactly
# the ELF header (including all program headers). It is a placeholder so that
# each hash covers the data of exactly one program header.

# For definitions of the ELF PHDR flags used by Qualcomm, see:
# https://github.com/coreboot/coreboot/blob/812d0e2f626dfea7e7deb960a8dc08ff0e026bc1/util/qualcomm/mbn_tools.py#L108-L189
PHDR_FLAGS_SEGMENT_TYPE_MASK	= 0x07000000
PHDR_FLAGS_SEGMENT_TYPE_SHIFT	= 0x18
PHDR_FLAGS_SEGMENT_TYPE_HASH	= (0x2 << PHDR_FLAGS_SEGMENT_TYPE_SHIFT)
PHDR_FLAGS_SEGMENT_TYPE_HDR	= (0x7 << PHDR_FLAGS_SEGMENT_TYPE_SHIFT)

PDHR_FLAGS_ACCESS_TYPE_MASK	= 0x00E00000
PHDR_FLAGS_ACCESS_TYPE_SHIFT	= 0x15
PHDR_FLAGS_ACCESS_TYPE_RO	= (0x1 << PHDR_FLAGS_ACCESS_TYPE_SHIFT)

# Flags we use for placeholder for hash over ELF header and hash segment
PHDR_FLAGS_HDR_PLACEHOLDER = PHDR_FLAGS_SEGMENT_TYPE_HDR
PHDR_FLAGS_HASH_SEGMENT = (PHDR_FLAGS_SEGMENT_TYPE_HASH | PHDR_FLAGS_ACCESS_TYPE_RO)

EXTRA_PHDRS = 2  # header placeholder + hash segment

# Note: None of the alignments seem to be truly required,
# this could probably be reduced to get smaller file sizes.
HASH_SEG_ALIGN = 0x1000
CERT_CHAIN_ALIGN = 16

# According to the v2.0 PDF the metadata is 128 bytes long, but this does not
# seem to work. All official firmware seems to use 120 bytes instead.
MBN_V6_METADATA_SIZE = 120

# See OEM Metadata 2.0 definition in coreboot source code:
# https://github.com/coreboot/coreboot/blob/812d0e2f626dfea7e7deb960a8dc08ff0e026bc1/util/qualcomm/mbn_tools.py#L506-L691
MBN_V7_OEM_2_0_METADATA_SIZE = 224


@dataclass
class _HashSegment:
	image_id: int = 0  # Type of image (unused?)
	version: int = 0  # Header version number

	hash_size = 0
	signature_size = 0
	cert_chain_size = 0
	total_size = 0

	hashes = []
	signature = b''
	cert_chain = b''

	FORMAT = Struct('<10L')
	Hash = hashlib.sha256

	@property
	def size_with_header(self):
		return self.FORMAT.size + self.total_size

	def update(self, dest_addr: int):
		self.hash_size = len(self.hashes) * self.Hash().digest_size
		self.signature_size = len(self.signature)
		self.cert_chain_size = len(self.cert_chain)
		self.total_size = self.hash_size + self.signature_size + self.cert_chain_size

	def check(self):
		assert len(self.hashes) * self.Hash().digest_size == self.hash_size
		assert len(self.signature) == self.signature_size
		assert len(self.cert_chain) == self.cert_chain_size

	def pack_header(self):
		self.check()
		return self.FORMAT.pack(*dataclasses.astuple(self))

	def pack(self):
		return self.pack_header() \
			+ b''.join(self.hashes) \
			+ self.signature + self.cert_chain


@dataclass
class HashSegmentV3(_HashSegment):
	version: int = 3  # Header version number

	flash_addr: int = 0  # Location of image in flash (historical)
	dest_addr: int = 0  # Physical address of loaded hash segment data
	total_size: int = 0  # = hash_size + signature_size + cert_chain_size
	hash_size: int = 0  # Size of hashes for all program segments
	signature_addr: int = 0  # Physical address of loaded attestation signature
	signature_size: int = 0  # Size of attestation signature
	cert_chain_addr: int = 0  # Physical address of loaded certificate chain
	cert_chain_size: int = 0  # Size of certificate chain

	def update(self, dest_addr: int):
		super().update(dest_addr)
		self.dest_addr = dest_addr + self.FORMAT.size
		self.signature_addr = self.dest_addr + self.hash_size
		self.cert_chain_addr = self.signature_addr + self.signature_size


@dataclass
class HashSegmentV5(_HashSegment):
	version: int = 5  # Header version number

	signature_size_qcom: int = 0  # Size of signature from Qualcomm
	cert_chain_size_qcom: int = 0  # Size of certificate chain from Qualcomm
	total_size: int = 0  # = hash_size + signature_size + cert_chain_size
	hash_size: int = 0  # Size of hashes for all program segments
	signature_addr: int = 0xffffffff  # unused?
	signature_size: int = 0  # Size of attestation signature
	cert_chain_addr: int = 0xffffffff  # unused?
	cert_chain_size: int = 0  # Size of certificate chain

	signature_qcom = b''
	cert_chain_qcom = b''

	def update(self, dest_addr: int):
		super().update(dest_addr)
		self.signature_size_qcom = len(self.signature_qcom)
		self.cert_chain_size_qcom = len(self.cert_chain_qcom)
		self.total_size += self.signature_size_qcom + self.cert_chain_size_qcom

	def check(self):
		super().check()
		assert len(self.signature_qcom) == self.signature_size_qcom
		assert len(self.cert_chain_qcom) == self.cert_chain_size_qcom

	def pack(self):
		return self.pack_header() \
			+ b''.join(self.hashes) \
			+ self.signature_qcom + self.cert_chain_qcom \
			+ self.signature + self.cert_chain


@dataclass
class HashSegmentV6(HashSegmentV5):
	version: int = 6  # Header version number

	metadata_size_qcom: int = 0  # Size of metadata from Qualcomm
	metadata_size: int = 0  # Size of metadata

	metadata_qcom = b''
	metadata = b''

	FORMAT = Struct('<12L')
	Hash = hashlib.sha384

	def update(self, dest_addr: int):
		super().update(dest_addr)
		self.metadata_size_qcom = len(self.metadata_qcom)
		self.metadata_size = len(self.metadata)
		self.total_size += self.metadata_size_qcom + self.metadata_size

	def check(self):
		super().check()
		assert len(self.metadata_qcom) == self.metadata_size_qcom
		assert len(self.metadata) == self.metadata_size

	def pack(self):
		return self.pack_header() \
			+ self.metadata_qcom + self.metadata \
			+ b''.join(self.hashes) \
			+ self.signature_qcom + self.cert_chain_qcom \
			+ self.signature + self.cert_chain


@dataclass
# Information from MBNv7 definition in Coreboot source code:
# https://github.com/coreboot/coreboot/blob/812d0e2f626dfea7e7deb960a8dc08ff0e026bc1/util/qualcomm/mbn_tools.py#L506-L691
class HashSegmentV7(_HashSegment):
	version: int = 7  # Header version number

	common_metadata_size: int = 24  # Size of "common metadata" below
	metadata_size_qcom: int = 0  # Size of metadata from Qualcomm
	metadata_size: int = 0  # Size of metadata from OEM
	hash_size: int = 0  # Size of hashes for all program segments
	signature_size_qcom: int = 0  # Size of signature from Qualcomm
	cert_chain_size_qcom: int = 0  # Size of certificate chain from Qualcomm
	signature_size: int = 0  # Size of attestation signature
	cert_chain_size: int = 0  # Size of certificate chain

	# Common metadata, placed directly after MBNv7 header
	common_metadata_major_version: int = 0
	common_metadata_minor_version: int = 0
	software_id: int = 0  # Type of software image, mandatory
	secondary_software_id: int = 0
	hash_table_algorithm: int = 3  # SHA384
	measurement_register_target: int = 0

	metadata_qcom = b''
	metadata = b''
	signature_qcom = b''
	cert_chain_qcom = b''

	FORMAT = Struct('<16L')
	Hash = hashlib.sha384

	def update(self, dest_addr: int):
		super().update(dest_addr)
		self.metadata_size_qcom = len(self.metadata_qcom)
		self.metadata_size = len(self.metadata)
		self.signature_size_qcom = len(self.signature_qcom)
		self.cert_chain_size_qcom = len(self.cert_chain_qcom)
		# self.common_metadata_size is already included as part of the header
		self.total_size += self.metadata_size_qcom + self.metadata_size
		self.total_size += self.signature_size_qcom + self.cert_chain_size_qcom

	def check(self):
		super().check()
		assert len(self.metadata_qcom) == self.metadata_size_qcom
		assert len(self.metadata) == self.metadata_size
		assert len(self.signature_qcom) == self.signature_size_qcom
		assert len(self.cert_chain_qcom) == self.cert_chain_size_qcom

	def pack(self):
		return self.pack_header() \
			+ self.metadata_qcom + self.metadata \
			+ b''.join(self.hashes) \
			+ self.signature_qcom + self.cert_chain_qcom \
			+ self.signature + self.cert_chain

HashSegment = {
	3: HashSegmentV3,
	5: HashSegmentV5,
	6: HashSegmentV6,
	7: HashSegmentV7,
}


def drop(elff: elf.Elf):
	# Drop existing hash segments
	elff.phdrs = [phdr for phdr in elff.phdrs if phdr.p_type != elf.Phdr.PT_NULL
		      or (phdr.p_flags & PHDR_FLAGS_SEGMENT_TYPE_MASK) not in
		      [PHDR_FLAGS_SEGMENT_TYPE_HASH, PHDR_FLAGS_SEGMENT_TYPE_HDR]]


def generate(elff: elf.Elf, version: int, sw_id: int):
	drop(elff)
	assert elff.phdrs, "Need at least one program header"

	hash_seg = HashSegment[version]()

	if version == 6:
		# TODO: Figure out metadata format and fill this with useful data
		hash_seg.metadata = b'\0' * MBN_V6_METADATA_SIZE

	# Software ID is mandatory for MBN v7
	if version == 7:
		hash_seg.software_id = sw_id
		# The format is documented in Coreboot util/qualcomm/mbn_tools.py
		# (see class Boot_Hdr), but for simplicity we just keep this empty.
		hash_seg.metadata = b'\0' * MBN_V7_OEM_2_0_METADATA_SIZE

	# Generate hash for all existing segments with data
	digest_size = hash_seg.Hash().digest_size
	hash_seg.hashes = [b'\0' * digest_size] * (len(elff.phdrs) + EXTRA_PHDRS)
	for i, phdr in enumerate(elff.phdrs, start=EXTRA_PHDRS):
		if phdr.data:
			hash_seg.hashes[i] = hash_seg.Hash(phdr.data).digest()
	total_hashes_size = len(hash_seg.hashes) * digest_size

	# Generate certificate chain with specified OU fields (for < v6)
	# on >= v6 this is part of the metadata instead
	ou_fields = []
	if version < 6:
		ou_fields = [
			# Note: The SW_ID is checked by the firmware on some platforms (even if secure boot
			# is disabled), so it must match the firmware type being signed. Everything else seems
			# to be mostly ignored when secure boot is off and is just added here to match the
			# documentation and better mimic the official firmware.
			"01 %016X SW_ID" % sw_id,
			"02 %016X HW_ID" % 0,
			"03 %016X DEBUG" % 2,  # DISABLED
			"04 %04X OEM_ID" % 0,
			"05 %08X SW_SIZE" % (hash_seg.FORMAT.size + total_hashes_size),
			"06 %04X MODEL_ID" % 0,
			"07 %04X SHA256" % 1,
		]
	hash_seg.cert_chain = cert.generate_chain(ou_fields)
	hash_seg.cert_chain = hash_seg.cert_chain.ljust(elf.align(len(hash_seg.cert_chain), CERT_CHAIN_ALIGN), b'\xff')
	# hash_seg.cert_chain = b''  # uncomment this to omit the certificate chain in the signed image

	# TODO: Generate actual signature with our generated attestation certificate!
	# There are different signature schemes that could be implemented (RSASSA-PKCS#1 v1.5
	# RSASSA-PSS, ECDSA over P-384) but it's not entirely clear yet which chipsets supports/
	# uses which. The signature does not seem to be checked on devices without secure boot,
	# so just use a dummy value for now.
	hash_seg.signature = b'\xff' * (cert.ATT_KEY.key_size // 8)
	# hash_seg.signature = b''  # uncomment this to omit the signature in the signed image

	# Align maximum end address to get address for hash table header, then update header
	hash_addr = elf.align(max(phdr.p_paddr + phdr.p_memsz for phdr in elff.phdrs), HASH_SEG_ALIGN)
	hash_seg.update(hash_addr)

	# Insert new hash NULL segment
	hash_phdr = elf.Phdr(elf.Phdr.PT_NULL, HASH_SEG_ALIGN, hash_addr, hash_addr, hash_seg.size_with_header,
						 elf.align(hash_seg.size_with_header, HASH_SEG_ALIGN),
						 PHDR_FLAGS_HASH_SEGMENT, HASH_SEG_ALIGN)
	elff.phdrs.insert(0, hash_phdr)

	# Insert new ELF header placeholder program header
	hdr_hash_phdr = elf.Phdr(elf.Phdr.PT_NULL, 0, 0, 0, 0, 0, PHDR_FLAGS_HDR_PLACEHOLDER, 0)
	elff.phdrs.insert(0, hdr_hash_phdr)

	# Now determine size of ELF header (including program headers)
	hdr_hash_phdr.p_filesz = elff.total_header_size()

	# Recompute attributes to match final output (e.g. adjust e_phnum)
	elff.update()

	# Compute the hash for the ELF header
	with BytesIO() as hdr_io:
		elff.save_header(hdr_io)
		hash_seg.hashes[0] = hash_seg.Hash(hdr_io.getbuffer()).digest()

	# And finally, assemble the hash segment
	hash_phdr.data = hash_seg.pack()
	assert len(hash_phdr.data) == hash_phdr.p_filesz
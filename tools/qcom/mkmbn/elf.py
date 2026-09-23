# SPDX-License-Identifier: GPL-2.0-only
# Copyright (C) 2021 Stephan Gerhold
# Data classes are based on the header definitions in the ELF(5) man page.
# Also see: https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
from __future__ import annotations

import dataclasses
from dataclasses import dataclass
from struct import Struct
from typing import List, BinaryIO


@dataclass
class Ehdr:
	ei_magic: bytes
	ei_class: int
	ei_data: int
	ei_version: int
	ei_os_abi: int
	ei_abi_version: int
	e_type: int
	e_machine: int
	e_version: int
	# Address size specific part
	e_entry: int = 0
	e_phoff: int = 0
	e_shoff: int = 0
	# End part
	e_flags: int = 0
	e_ehsize: int = 0
	e_phentsize: int = 0
	e_phnum: int = 0
	e_shentsize: int = 0
	e_shnum: int = 0
	e_shstrndx: int = 0

	START_FORMAT = Struct('<4s5B7xHHL')
	START_COUNT = 9
	MEM_FORMAT32 = Struct('<LLL')
	MEM_FORMAT64 = Struct('<QQQ')
	MEM_COUNT = 3
	END_FORMAT = Struct('<L6H')
	END_COUNT = 7

	CLASS32 = 1
	CLASS64 = 2

	# Init a qcom XBL style ELF header
	def __init__(self):
		self.ei_magic = b"\x7fELF"
		self.ei_class = 2
		self.ei_data = 1
		self.ei_version = 1
		self.ei_os_abi = 0
		self.ei_abi_version = 0
		self.e_type = 2
		self.e_machine = 183
		self.e_version = 1

		self.e_ehsize = 64
		self.e_phoff = 64
		self.e_phentsize = 56

	@staticmethod
	def parse(b: bytes) -> Ehdr:
		hdr_unpack = Ehdr.START_FORMAT.unpack_from(b)
		hdr = Ehdr(*hdr_unpack)
		assert hdr.ei_magic == b'\x7fELF', f"Invalid ELF header magic: {hdr.ei_magic}"
		assert hdr.ei_data == 1, "Only little endian supported at the moment"
		assert hdr.ei_version == 1, f"Unexpected ei_version: {hdr.ei_version}"
		assert hdr.e_version == 1, f"Unexpected e_version: {hdr.e_version}"

		if hdr.ei_class == Ehdr.CLASS32:
			mem_format = Ehdr.MEM_FORMAT32
		else:
			assert hdr.ei_class == Ehdr.CLASS64, f"Unexpected ei_class: {hdr.ei_class}"
			mem_format = Ehdr.MEM_FORMAT64

		mem_unpack = mem_format.unpack_from(b, Ehdr.START_FORMAT.size)
		end_unpack = Ehdr.END_FORMAT.unpack_from(b, Ehdr.START_FORMAT.size + mem_format.size)
		return Ehdr(*hdr_unpack, *mem_unpack, *end_unpack)

	def save(self, f: BinaryIO) -> int:
		unpack = dataclasses.astuple(self)
		written = f.write(Ehdr.START_FORMAT.pack(*unpack[:Ehdr.START_COUNT]))

		if self.ei_class == Ehdr.CLASS32:
			mem_format = Ehdr.MEM_FORMAT32
		else:
			mem_format = Ehdr.MEM_FORMAT64
		written += f.write(
			mem_format.pack(*unpack[Ehdr.START_COUNT:Ehdr.START_COUNT + Ehdr.MEM_COUNT]))
		written += f.write(Ehdr.END_FORMAT.pack(*unpack[-Ehdr.END_COUNT:]))
		return written


@dataclass
class Phdr:
	p_type: int
	p_offset: int
	p_vaddr: int
	p_paddr: int
	p_filesz: int
	p_memsz: int
	p_flags: int
	p_align: int

	data = None

	FORMAT32 = Struct('<8L')
	FORMAT64 = Struct('<LL6Q')

	PT_NULL = 0
	PT_LOAD = 1

	@staticmethod
	def parse(b: bytes, offset: int, ei_class: int) -> Phdr:
		if ei_class == Ehdr.CLASS32:
			unpack = list(Phdr.FORMAT32.unpack_from(b, offset))
		else:
			unpack = list(Phdr.FORMAT64.unpack_from(b, offset))

			# ELFCLASS64 has flags directly before offset for alignment
			flags = unpack.pop(1)
			unpack.insert(-1, flags)

		return Phdr(*unpack)

	@staticmethod
	def from_bin(b: bytes, loadaddr: int) -> Phdr:
		# p_offset is fixed later
		phdr = Phdr(
			p_type=1,
			p_offset=0xFFFFFFFF,
			p_vaddr=loadaddr,
			p_paddr=loadaddr,
			p_filesz=len(b),
			p_memsz=len(b),
			p_flags=7,
			p_align=0x1000,
		)
		phdr.data = memoryview(b)
		return phdr

	def save(self, f: BinaryIO, ei_class: int) -> int:
		unpack: tuple|list = dataclasses.astuple(self)

		if ei_class == Ehdr.CLASS32:
			return f.write(Phdr.FORMAT32.pack(*unpack))
		else:
			unpack = list(unpack)

			# ELFCLASS64 has flags directly before offset for alignment
			flags = unpack.pop(-2)
			unpack.insert(1, flags)

			return f.write(Phdr.FORMAT64.pack(*unpack))


def _pad(f: BinaryIO, offset: int, pos: int) -> int:
	assert offset >= pos, f"{offset} >= {pos}"
	pad = offset - pos
	if pad:
		assert f.write(b'\0' * pad) == pad
	return offset


def align(i: int, alignment: int) -> int:
	mask = max(alignment - 1, 0)
	return (i + mask) & ~mask


@dataclass
class Elf:
	ehdr: Ehdr
	phdrs: List[Phdr]

	def __init__(self, ehdr: Ehdr = Ehdr(), phdrs: List[Phdr] = []):
		self.ehdr = ehdr
		self.phdrs = phdrs

	def total_header_size(self):
		return self.ehdr.e_phoff + len(self.phdrs) * self.ehdr.e_phentsize

	@staticmethod
	def parse(b: bytes) -> Elf:
		ehdr = Ehdr.parse(b)
		view = memoryview(b)

		# Parse program headers
		phdrs = []
		offset = ehdr.e_phoff
		for i in range(ehdr.e_phnum):
			phdr = Phdr.parse(b, offset, ehdr.ei_class)
			phdrs.append(phdr)

			# Store data if necessary
			if phdr.p_filesz and phdr.p_offset:
				phdr.data = view[phdr.p_offset:phdr.p_offset + phdr.p_filesz]

			offset += ehdr.e_phentsize

		return Elf(ehdr, phdrs)

	def update(self):
		# Rearrange all segments according to their alignment
		pos = self.total_header_size()
		for phdr in sorted(self.phdrs, key=lambda phdr: phdr.p_offset):
			if phdr.p_offset and phdr.p_filesz:
				phdr.p_offset = align(pos, phdr.p_align)
				pos = phdr.p_offset + phdr.p_filesz

		# Ensure program header count is correct
		self.ehdr.e_phnum = len(self.phdrs)

		# TODO: Clear out sections for now. Those are not read at the moment.
		# Also, I don't think the Qualcomm firmware loader has any use for these.
		self.ehdr.e_shoff = 0
		self.ehdr.e_shnum = 0
		self.ehdr.e_shstrndx = 0

	def save_header(self, f: BinaryIO) -> int:
		pos = self.ehdr.save(f)
		pos = _pad(f, self.ehdr.e_phoff, pos)

		# Write program headers
		for phdr in self.phdrs:
			pos += phdr.save(f, self.ehdr.ei_class)

		return pos

	def save(self, f: BinaryIO) -> int:
		pos = self.save_header(f)

		# Write segment data
		for phdr in sorted(self.phdrs, key=lambda phdr: phdr.p_offset):
			if phdr.data:
				pos = _pad(f, phdr.p_offset, pos)
				pos += f.write(phdr.data)

		return pos

// SPDX-License-Identifier: GPL-2.0+ OR BSD-2-Clause
/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * 64-bit and little-endian target only until we need to support a different
 * arch that needs this.
 */

#include <elf.h>
#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"

#ifndef R_AARCH64_RELATIVE
#define R_AARCH64_RELATIVE	1027
#endif

static int ei_class;

static uint64_t rela_start, rela_end, text_base, dyn_start;

static const bool debug_en;

static void debug(const char *fmt, ...)
{
	va_list args;

	if (debug_en) {
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
	}
}

static bool supported_rela(Elf64_Rela *rela)
{
	uint64_t mask = 0xffffffffULL; /* would be different on 32-bit */
	uint32_t type = rela->r_info & mask;

	switch (type) {
#ifdef R_AARCH64_RELATIVE
	case R_AARCH64_RELATIVE:
		return true;
#endif
	default:
		fprintf(stderr, "warning: unsupported relocation type %"
				PRIu32 " at %" PRIx64 "\n",
			type, rela->r_offset);

		return false;
	}
}

static int decode_elf64(FILE *felf, char **argv)
{
	size_t size;
	Elf64_Ehdr header;
	uint64_t section_header_base, section_header_size, sh_offset, sh_size;
	Elf64_Shdr *sh_table; /* Elf symbol table */
	int ret, i, machine;
	char *sh_str;

	debug("64bit version\n");

	/* Make sure we are at start */
	rewind(felf);

	size = fread(&header, 1, sizeof(header), felf);
	if (size != sizeof(header)) {
		fclose(felf);
		return 25;
	}

	machine = header.e_machine;
	debug("Machine\t%d\n", machine);

	if (machine != EM_AARCH64) {
		fprintf(stderr, "%s: Not supported machine type\n", argv[0]);
		return 30;
	}

	text_base = header.e_entry;
	section_header_base = header.e_shoff;
	section_header_size = header.e_shentsize * header.e_shnum;

	sh_table = malloc(section_header_size);
	if (!sh_table) {
		fprintf(stderr, "%s: Cannot allocate space for section header\n",
			argv[0]);
		fclose(felf);
		return 26;
	}

	ret = fseek(felf, section_header_base, SEEK_SET);
	if (ret) {
		fprintf(stderr, "%s: Can't set pointer to section header: %x/%lx\n",
			argv[0], ret, section_header_base);
		free(sh_table);
		fclose(felf);
		return 26;
	}

	size = fread(sh_table, 1, section_header_size, felf);
	if (size != section_header_size) {
		fprintf(stderr, "%s: Can't read section header: %lx/%lx\n",
			argv[0], size, section_header_size);
		free(sh_table);
		fclose(felf);
		return 27;
	}

	sh_size = sh_table[header.e_shstrndx].sh_size;
	debug("e_shstrndx\t0x%08x\n", header.e_shstrndx);
	debug("sh_size\t\t0x%08lx\n", sh_size);

	sh_str = malloc(sh_size);
	if (!sh_str) {
		fprintf(stderr, "malloc failed\n");
		free(sh_table);
		fclose(felf);
		return 28;
	}

	/*
	 * Specifies the byte offset from the beginning of the file
	 * to the first byte in the section.
	 */
	sh_offset = sh_table[header.e_shstrndx].sh_offset;

	debug("sh_offset\t0x%08x\n", header.e_shnum);

	ret = fseek(felf, sh_offset, SEEK_SET);
	if (ret) {
		fprintf(stderr, "Setting up sh_offset failed\n");
		free(sh_str);
		free(sh_table);
		fclose(felf);
		return 29;
	}

	size = fread(sh_str, 1, sh_size, felf);
	if (size != sh_size) {
		fprintf(stderr, "%s: Can't read section: %lx/%lx\n",
			argv[0], size, sh_size);
		free(sh_str);
		free(sh_table);
		fclose(felf);
		return 30;
	}

	for (i = 0; i < header.e_shnum; i++) {
		/* fprintf(stderr, "%s\n", sh_str + sh_table[i].sh_name); Debug only */
		if (!strcmp(".rela.dyn", (sh_str + sh_table[i].sh_name))) {
			debug("Found section\t\".rela_dyn\"\n");
			debug(" at addr\t0x%08x\n",
			      (unsigned int)sh_table[i].sh_addr);
			debug(" at offset\t0x%08x\n",
			      (unsigned int)sh_table[i].sh_offset);
			debug(" of size\t0x%08x\n",
			      (unsigned int)sh_table[i].sh_size);
			rela_start = sh_table[i].sh_addr;
			rela_end = rela_start + sh_table[i].sh_size;
			break;
		}
	}

	/* Clean up */
	free(sh_str);
	free(sh_table);
	fclose(felf);

	debug("text_base\t0x%08lx\n", text_base);
	debug("rela_start\t0x%08lx\n", rela_start);
	debug("rela_end\t0x%08lx\n", rela_end);

	if (!rela_start)
		return 1;

	return 0;
}

static int decode_elf32(FILE *felf, char **argv)
{
	size_t size;
	Elf32_Ehdr header;
	uint64_t section_header_base, section_header_size, sh_offset, sh_size;
	Elf32_Shdr *sh_table; /* Elf symbol table */
	int ret, i, machine;
	char *sh_str;

	debug("32bit version\n");

	/* Make sure we are at start */
	rewind(felf);

	size = fread(&header, 1, sizeof(header), felf);
	if (size != sizeof(header)) {
		fclose(felf);
		return 25;
	}

	machine = header.e_machine;
	debug("Machine %d\n", machine);

	if (machine != EM_MICROBLAZE) {
		fprintf(stderr, "%s: Not supported machine type\n", argv[0]);
		return 30;
	}

	text_base = header.e_entry;
	section_header_base = header.e_shoff;

	debug("Section header base %x\n", section_header_base);

	section_header_size = header.e_shentsize * header.e_shnum;

	debug("Section header size %d\n", section_header_size);

	sh_table = malloc(section_header_size);
	if (!sh_table) {
		fprintf(stderr, "%s: Cannot allocate space for section header\n",
			argv[0]);
		fclose(felf);
		return 26;
	}

	ret = fseek(felf, section_header_base, SEEK_SET);
	if (ret) {
		fprintf(stderr, "%s: Can't set pointer to section header: %x/%lx\n",
			argv[0], ret, section_header_base);
		free(sh_table);
		fclose(felf);
		return 26;
	}

	size = fread(sh_table, 1, section_header_size, felf);
	if (size != section_header_size) {
		fprintf(stderr, "%s: Can't read section header: %lx/%lx\n",
			argv[0], size, section_header_size);
		free(sh_table);
		fclose(felf);
		return 27;
	}

	sh_size = sh_table[header.e_shstrndx].sh_size;
	debug("e_shstrndx %x, sh_size %lx\n", header.e_shstrndx, sh_size);

	sh_str = malloc(sh_size);
	if (!sh_str) {
		fprintf(stderr, "malloc failed\n");
		free(sh_table);
		fclose(felf);
		return 28;
	}

	/*
	 * Specifies the byte offset from the beginning of the file
	 * to the first byte in the section.
	 */
	sh_offset = sh_table[header.e_shstrndx].sh_offset;

	debug("sh_offset %x\n", header.e_shnum);

	ret = fseek(felf, sh_offset, SEEK_SET);
	if (ret) {
		fprintf(stderr, "Setting up sh_offset failed\n");
		free(sh_str);
		free(sh_table);
		fclose(felf);
		return 29;
	}

	size = fread(sh_str, 1, sh_size, felf);
	if (size != sh_size) {
		fprintf(stderr, "%s: Can't read section: %lx/%lx\n",
			argv[0], size, sh_size);
		free(sh_str);
		free(sh_table);
		fclose(felf);
		return 30;
	}

	for (i = 0; i < header.e_shnum; i++) {
		debug("%s\n", sh_str + sh_table[i].sh_name);
		if (!strcmp(".rela.dyn", (sh_str + sh_table[i].sh_name))) {
			debug("Found section\t\".rela_dyn\"\n");
			debug(" at addr\t0x%08x\n", (unsigned int)sh_table[i].sh_addr);
			debug(" at offset\t0x%08x\n", (unsigned int)sh_table[i].sh_offset);
			debug(" of size\t0x%08x\n", (unsigned int)sh_table[i].sh_size);
			rela_start = sh_table[i].sh_addr;
			rela_end = rela_start + sh_table[i].sh_size;
		}
		if (!strcmp(".dynsym", (sh_str + sh_table[i].sh_name))) {
			debug("Found section\t\".dynsym\"\n");
			debug(" at addr\t0x%08x\n", (unsigned int)sh_table[i].sh_addr);
			debug(" at offset\t0x%08x\n", (unsigned int)sh_table[i].sh_offset);
			debug(" of size\t0x%08x\n", (unsigned int)sh_table[i].sh_size);
			dyn_start = sh_table[i].sh_addr;
		}
	}

	/* Clean up */
	free(sh_str);
	free(sh_table);
	fclose(felf);

	debug("text_base\t0x%08lx\n", text_base);
	debug("rela_start\t0x%08lx\n", rela_start);
	debug("rela_end\t0x%08lx\n", rela_end);
	debug("dyn_start\t0x%08lx\n", dyn_start);

	if (!rela_start)
		return 1;

	return 0;
}

static int decode_elf(char **argv)
{
	FILE *felf;
	size_t size;
	unsigned char e_ident[EI_NIDENT];

	felf = fopen(argv[2], "r+b");
	if (!felf) {
		fprintf(stderr, "%s: Cannot open %s: %s\n",
			argv[0], argv[5], strerror(errno));
		return 2;
	}

	size = fread(e_ident, 1, EI_NIDENT, felf);
	if (size != EI_NIDENT) {
		fclose(felf);
		return 25;
	}

	/* Check if this is really ELF file */
	if (e_ident[0] != 0x7f &&
	    e_ident[1] != 'E' &&
	    e_ident[2] != 'L' &&
	    e_ident[3] != 'F') {
		fclose(felf);
		return 1;
	}

	ei_class = e_ident[4];
	debug("EI_CLASS(1=32bit, 2=64bit) %d\n", ei_class);

	if (ei_class == 2)
		return decode_elf64(felf, argv);

	return decode_elf32(felf, argv);
}

static int rela_elf64(char **argv, FILE *f)
{
	int i, num;

	if ((rela_end - rela_start) % sizeof(Elf64_Rela)) {
		fprintf(stderr, "%s: rela size isn't a multiple of Elf64_Rela\n", argv[0]);
		return 3;
	}

	num = (rela_end - rela_start) / sizeof(Elf64_Rela);

	for (i = 0; i < num; i++) {
		Elf64_Rela rela, swrela;
		uint64_t pos = rela_start + sizeof(Elf64_Rela) * i;
		uint64_t addr;

		if (fseek(f, pos, SEEK_SET) < 0) {
			fprintf(stderr, "%s: %s: seek to %" PRIx64
					" failed: %s\n",
				argv[0], argv[1], pos, strerror(errno));
		}

		if (fread(&rela, sizeof(rela), 1, f) != 1) {
			fprintf(stderr, "%s: %s: read rela failed at %"
					PRIx64 "\n",
				argv[0], argv[1], pos);
			return 4;
		}

		swrela.r_offset = cpu_to_le64(rela.r_offset);
		swrela.r_info = cpu_to_le64(rela.r_info);
		swrela.r_addend = cpu_to_le64(rela.r_addend);

		if (!supported_rela(&swrela))
			continue;

		debug("Rela %" PRIx64 " %" PRIu64 " %" PRIx64 "\n",
		      swrela.r_offset, swrela.r_info, swrela.r_addend);

		if (swrela.r_offset < text_base) {
			fprintf(stderr, "%s: %s: bad rela at %" PRIx64 "\n",
				argv[0], argv[1], pos);
			return 4;
		}

		addr = swrela.r_offset - text_base;

		if (fseek(f, addr, SEEK_SET) < 0) {
			fprintf(stderr, "%s: %s: seek to %"
					PRIx64 " failed: %s\n",
				argv[0], argv[1], addr, strerror(errno));
		}

		if (fwrite(&rela.r_addend, sizeof(rela.r_addend), 1, f) != 1) {
			fprintf(stderr, "%s: %s: write failed at %" PRIx64 "\n",
				argv[0], argv[1], addr);
			return 4;
		}
	}

	return 0;
}

int main(int argc, char **argv)
{
	FILE *f;
	int ret;
	uint64_t file_size;

	if (argc != 3) {
		fprintf(stderr, "Statically apply ELF rela relocations\n");
		fprintf(stderr, "Usage: %s <bin file> <u-boot ELF>\n",
			argv[0]);
		return 1;
	}

	ret = decode_elf(argv);
	if (ret) {
		fprintf(stderr, "ELF decoding failed\n");
		return ret;
	}

	if (rela_start > rela_end || rela_start < text_base) {
		fprintf(stderr, "%s: bad rela bounds\n", argv[0]);
		return 3;
	}

	rela_start -= text_base;
	rela_end -= text_base;
	dyn_start -= text_base;

	f = fopen(argv[1], "r+b");
	if (!f) {
		fprintf(stderr, "%s: Cannot open %s: %s\n",
			argv[0], argv[1], strerror(errno));
		return 2;
	}

	fseek(f, 0, SEEK_END);
	file_size = ftell(f);
	rewind(f);

	if (rela_end > file_size) {
		// Most likely compiler inserted some section that didn't get
		// objcopy-ed into the final binary
		rela_end = file_size;
	}

	if (ei_class == 2)
		ret = rela_elf64(argv, f);

	if (fclose(f) < 0) {
		fprintf(stderr, "%s: %s: close failed: %s\n",
			argv[0], argv[1], strerror(errno));
		return 4;
	}

	return ret;
}

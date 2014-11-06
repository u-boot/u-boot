/*
 * Copyright (c) 2001 William L. Pitts
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are freely
 * permitted provided that the above copyright notice and this
 * paragraph and the following disclaimer are duplicated in all
 * such forms.
 *
 * This software is provided "AS IS" and without any express or
 * implied warranties, including, without limitation, the implied
 * warranties of merchantability and fitness for a particular
 * purpose.
 */

#include <common.h>
#include <bootm.h>
#include <command.h>
#include <linux/ctype.h>
#include <net.h>
#include <elf.h>
#include <vxworks.h>

#if defined(CONFIG_WALNUT) || defined(CONFIG_SYS_VXWORKS_MAC_PTR)
DECLARE_GLOBAL_DATA_PTR;
#endif

static unsigned long load_elf_image_phdr(unsigned long addr);
static unsigned long load_elf_image_shdr(unsigned long addr);

/* Allow ports to override the default behavior */
static unsigned long do_bootelf_exec(ulong (*entry)(int, char * const[]),
			       int argc, char * const argv[])
{
	unsigned long ret;

	/*
	 * QNX images require the data cache is disabled.
	 * Data cache is already flushed, so just turn it off.
	 */
	int dcache = dcache_status();
	if (dcache)
		dcache_disable();

	/*
	 * pass address parameter as argv[0] (aka command name),
	 * and all remaining args
	 */
	ret = entry(argc, argv);

	if (dcache)
		dcache_enable();

	return ret;
}

/* ======================================================================
 * Determine if a valid ELF image exists at the given memory location.
 * First looks at the ELF header magic field, the makes sure that it is
 * executable and makes sure that it is for a PowerPC.
 * ====================================================================== */
int valid_elf_image(unsigned long addr)
{
	Elf32_Ehdr *ehdr;		/* Elf header structure pointer */

	/* -------------------------------------------------- */

	ehdr = (Elf32_Ehdr *) addr;

	if (!IS_ELF(*ehdr)) {
		printf("## No elf image at address 0x%08lx\n", addr);
		return 0;
	}

	if (ehdr->e_type != ET_EXEC) {
		printf("## Not a 32-bit elf image at address 0x%08lx\n", addr);
		return 0;
	}

#if 0
	if (ehdr->e_machine != EM_PPC) {
		printf("## Not a PowerPC elf image at address 0x%08lx\n", addr);
		return 0;
	}
#endif

	return 1;
}

/* ======================================================================
 * Interpreter command to boot an arbitrary ELF image from memory.
 * ====================================================================== */
int do_bootelf(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned long addr;		/* Address of the ELF image     */
	unsigned long rc;		/* Return value from user code  */
	char *sload, *saddr;

	/* -------------------------------------------------- */
	int rcode = 0;

	sload = saddr = NULL;
	if (argc == 3) {
		sload = argv[1];
		saddr = argv[2];
	} else if (argc == 2) {
		if (argv[1][0] == '-')
			sload = argv[1];
		else
			saddr = argv[1];
	}

	if (saddr)
		addr = simple_strtoul(saddr, NULL, 16);
	else
		addr = load_addr;

	if (!valid_elf_image(addr))
		return 1;

	if (sload && sload[1] == 'p')
		addr = load_elf_image_phdr(addr);
	else
		addr = load_elf_image_shdr(addr);

	printf("## Starting application at 0x%08lx ...\n", addr);

	/*
	 * pass address parameter as argv[0] (aka command name),
	 * and all remaining args
	 */
	rc = do_bootelf_exec((void *)addr, argc - 1, argv + 1);
	if (rc != 0)
		rcode = 1;

	printf("## Application terminated, rc = 0x%lx\n", rc);
	return rcode;
}

/* ======================================================================
 * Interpreter command to boot VxWorks from a memory image.  The image can
 * be either an ELF image or a raw binary.  Will attempt to setup the
 * bootline and other parameters correctly.
 * ====================================================================== */
int do_bootvx(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned long addr;		/* Address of image            */
	unsigned long bootaddr;	/* Address to put the bootline */
	char *bootline;			/* Text of the bootline        */
	char *tmp;			/* Temporary char pointer      */
	char build_buf[128];		/* Buffer for building the bootline */

	/* ---------------------------------------------------
	 *
	 * Check the loadaddr variable.
	 * If we don't know where the image is then we're done.
	 */

	if (argc < 2)
		addr = load_addr;
	else
		addr = simple_strtoul(argv[1], NULL, 16);

#if defined(CONFIG_CMD_NET)
	/*
	 * Check to see if we need to tftp the image ourselves before starting
	 */
	if ((argc == 2) && (strcmp(argv[1], "tftp") == 0)) {
		if (NetLoop(TFTPGET) <= 0)
			return 1;
		printf("Automatic boot of VxWorks image at address 0x%08lx ...\n",
			addr);
	}
#endif

	/* This should equate
	 * to NV_RAM_ADRS + NV_BOOT_OFFSET + NV_ENET_OFFSET
	 * from the VxWorks BSP header files.
	 * This will vary from board to board
	 */

#if defined(CONFIG_WALNUT)
	tmp = (char *) CONFIG_SYS_NVRAM_BASE_ADDR + 0x500;
	eth_getenv_enetaddr("ethaddr", (uchar *)build_buf);
	memcpy(tmp, &build_buf[3], 3);
#elif defined(CONFIG_SYS_VXWORKS_MAC_PTR)
	tmp = (char *) CONFIG_SYS_VXWORKS_MAC_PTR;
	eth_getenv_enetaddr("ethaddr", (uchar *)build_buf);
	memcpy(tmp, build_buf, 6);
#else
	puts("## Ethernet MAC address not copied to NV RAM\n");
#endif

	/*
	 * Use bootaddr to find the location in memory that VxWorks
	 * will look for the bootline string. The default value for
	 * PowerPC is LOCAL_MEM_LOCAL_ADRS + BOOT_LINE_OFFSET which
	 * defaults to 0x4200
	 */
	tmp = getenv("bootaddr");
	if (!tmp)
		bootaddr = CONFIG_SYS_VXWORKS_BOOT_ADDR;
	else
		bootaddr = simple_strtoul(tmp, NULL, 16);

	/*
	 * Check to see if the bootline is defined in the 'bootargs'
	 * parameter. If it is not defined, we may be able to
	 * construct the info
	 */
	bootline = getenv("bootargs");
	if (bootline) {
		memcpy((void *)bootaddr, bootline,
		       max(strlen(bootline), (size_t)255));
		flush_cache(bootaddr, max(strlen(bootline), (size_t)255));
	} else {
		sprintf(build_buf, CONFIG_SYS_VXWORKS_BOOT_DEVICE);
		tmp = getenv("bootfile");
		if (tmp)
			sprintf(&build_buf[strlen(build_buf)],
				 "%s:%s ", CONFIG_SYS_VXWORKS_SERVERNAME, tmp);
		else
			sprintf(&build_buf[strlen(build_buf)],
				 "%s:file ", CONFIG_SYS_VXWORKS_SERVERNAME);

		tmp = getenv("ipaddr");
		if (tmp)
			sprintf(&build_buf[strlen(build_buf)], "e=%s ", tmp);

		tmp = getenv("serverip");
		if (tmp)
			sprintf(&build_buf[strlen(build_buf)], "h=%s ", tmp);

		tmp = getenv("hostname");
		if (tmp)
			sprintf(&build_buf[strlen(build_buf)], "tn=%s ", tmp);

#ifdef CONFIG_SYS_VXWORKS_ADD_PARAMS
		sprintf(&build_buf[strlen(build_buf)],
			 CONFIG_SYS_VXWORKS_ADD_PARAMS);
#endif

		memcpy((void *)bootaddr, build_buf,
		       max(strlen(build_buf), (size_t)255));
		flush_cache(bootaddr, max(strlen(build_buf), (size_t)255));
	}

	/*
	 * If the data at the load address is an elf image, then
	 * treat it like an elf image. Otherwise, assume that it is a
	 * binary image
	 */

	if (valid_elf_image(addr)) {
		addr = load_elf_image_shdr(addr);
	} else {
		puts("## Not an ELF image, assuming binary\n");
		/* leave addr as load_addr */
	}

	printf("## Using bootline (@ 0x%lx): %s\n", bootaddr,
			(char *) bootaddr);
	printf("## Starting vxWorks at 0x%08lx ...\n", addr);

	dcache_disable();
	((void (*)(int)) addr) (0);

	puts("## vxWorks terminated\n");
	return 1;
}

/* ======================================================================
 * A very simple elf loader, assumes the image is valid, returns the
 * entry point address.
 * ====================================================================== */
static unsigned long load_elf_image_phdr(unsigned long addr)
{
	Elf32_Ehdr *ehdr;		/* Elf header structure pointer     */
	Elf32_Phdr *phdr;		/* Program header structure pointer */
	int i;

	ehdr = (Elf32_Ehdr *) addr;
	phdr = (Elf32_Phdr *) (addr + ehdr->e_phoff);

	/* Load each program header */
	for (i = 0; i < ehdr->e_phnum; ++i) {
		void *dst = (void *)(uintptr_t) phdr->p_paddr;
		void *src = (void *) addr + phdr->p_offset;
		debug("Loading phdr %i to 0x%p (%i bytes)\n",
			i, dst, phdr->p_filesz);
		if (phdr->p_filesz)
			memcpy(dst, src, phdr->p_filesz);
		if (phdr->p_filesz != phdr->p_memsz)
			memset(dst + phdr->p_filesz, 0x00,
				phdr->p_memsz - phdr->p_filesz);
		flush_cache((unsigned long)dst, phdr->p_filesz);
		++phdr;
	}

	return ehdr->e_entry;
}

static unsigned long load_elf_image_shdr(unsigned long addr)
{
	Elf32_Ehdr *ehdr;		/* Elf header structure pointer     */
	Elf32_Shdr *shdr;		/* Section header structure pointer */
	unsigned char *strtab = 0;	/* String table pointer             */
	unsigned char *image;		/* Binary image pointer             */
	int i;				/* Loop counter                     */

	/* -------------------------------------------------- */

	ehdr = (Elf32_Ehdr *) addr;

	/* Find the section header string table for output info */
	shdr = (Elf32_Shdr *) (addr + ehdr->e_shoff +
			       (ehdr->e_shstrndx * sizeof(Elf32_Shdr)));

	if (shdr->sh_type == SHT_STRTAB)
		strtab = (unsigned char *) (addr + shdr->sh_offset);

	/* Load each appropriate section */
	for (i = 0; i < ehdr->e_shnum; ++i) {
		shdr = (Elf32_Shdr *) (addr + ehdr->e_shoff +
				       (i * sizeof(Elf32_Shdr)));

		if (!(shdr->sh_flags & SHF_ALLOC)
		   || shdr->sh_addr == 0 || shdr->sh_size == 0) {
			continue;
		}

		if (strtab) {
			debug("%sing %s @ 0x%08lx (%ld bytes)\n",
				(shdr->sh_type == SHT_NOBITS) ?
					"Clear" : "Load",
				&strtab[shdr->sh_name],
				(unsigned long) shdr->sh_addr,
				(long) shdr->sh_size);
		}

		if (shdr->sh_type == SHT_NOBITS) {
			memset((void *)(uintptr_t) shdr->sh_addr, 0,
				shdr->sh_size);
		} else {
			image = (unsigned char *) addr + shdr->sh_offset;
			memcpy((void *)(uintptr_t) shdr->sh_addr,
				(const void *) image,
				shdr->sh_size);
		}
		flush_cache(shdr->sh_addr, shdr->sh_size);
	}

	return ehdr->e_entry;
}

/* ====================================================================== */
U_BOOT_CMD(
	bootelf,      3,      0,      do_bootelf,
	"Boot from an ELF image in memory",
	"[-p|-s] [address]\n"
	"\t- load ELF image at [address] via program headers (-p)\n"
	"\t  or via section headers (-s)"
);

U_BOOT_CMD(
	bootvx,      2,      0,      do_bootvx,
	"Boot vxWorks from an ELF image",
	" [address] - load address of vxWorks ELF image."
);

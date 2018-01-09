/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <bootm.h>
#include <command.h>
#include <image.h>
#include <lmb.h>
#include <mapmem.h>
#include <linux/kernel.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

/* See Documentation/arm64/booting.txt in the Linux kernel */
struct Image_header {
	uint32_t	code0;		/* Executable code */
	uint32_t	code1;		/* Executable code */
	uint64_t	text_offset;	/* Image load offset, LE */
	uint64_t	image_size;	/* Effective Image size, LE */
	uint64_t	flags;		/* Kernel flags, LE */
	uint64_t	res2;		/* reserved */
	uint64_t	res3;		/* reserved */
	uint64_t	res4;		/* reserved */
	uint32_t	magic;		/* Magic number */
	uint32_t	res5;
};

#define LINUX_ARM64_IMAGE_MAGIC	0x644d5241

static int booti_setup(bootm_headers_t *images)
{
	struct Image_header *ih;
	uint64_t dst;
	uint64_t image_size, text_offset;

	ih = (struct Image_header *)map_sysmem(images->ep, 0);

	if (ih->magic != le32_to_cpu(LINUX_ARM64_IMAGE_MAGIC)) {
		puts("Bad Linux ARM64 Image magic!\n");
		return 1;
	}

	/*
	 * Prior to Linux commit a2c1d73b94ed, the text_offset field
	 * is of unknown endianness.  In these cases, the image_size
	 * field is zero, and we can assume a fixed value of 0x80000.
	 */
	if (ih->image_size == 0) {
		puts("Image lacks image_size field, assuming 16MiB\n");
		image_size = 16 << 20;
		text_offset = 0x80000;
	} else {
		image_size = le64_to_cpu(ih->image_size);
		text_offset = le64_to_cpu(ih->text_offset);
	}

	/*
	 * If bit 3 of the flags field is set, the 2MB aligned base of the
	 * kernel image can be anywhere in physical memory, so respect
	 * images->ep.  Otherwise, relocate the image to the base of RAM
	 * since memory below it is not accessible via the linear mapping.
	 */
	if (le64_to_cpu(ih->flags) & BIT(3))
		dst = images->ep - text_offset;
	else
		dst = gd->bd->bi_dram[0].start;

	dst = ALIGN(dst, SZ_2M) + text_offset;

	unmap_sysmem(ih);

	if (images->ep != dst) {
		void *src;

		debug("Moving Image from 0x%lx to 0x%llx\n", images->ep, dst);

		src = (void *)images->ep;
		images->ep = dst;
		memmove((void *)dst, src, image_size);
	}

	return 0;
}

/*
 * Image booting support
 */
static int booti_start(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[], bootm_headers_t *images)
{
	int ret;
	struct Image_header *ih;

	ret = do_bootm_states(cmdtp, flag, argc, argv, BOOTM_STATE_START,
			      images, 1);

	/* Setup Linux kernel Image entry point */
	if (!argc) {
		images->ep = load_addr;
		debug("*  kernel: default image load address = 0x%08lx\n",
				load_addr);
	} else {
		images->ep = simple_strtoul(argv[0], NULL, 16);
		debug("*  kernel: cmdline image address = 0x%08lx\n",
			images->ep);
	}

	ret = booti_setup(images);
	if (ret != 0)
		return 1;

	ih = (struct Image_header *)map_sysmem(images->ep, 0);

	lmb_reserve(&images->lmb, images->ep, le32_to_cpu(ih->image_size));

	unmap_sysmem(ih);

	/*
	 * Handle the BOOTM_STATE_FINDOTHER state ourselves as we do not
	 * have a header that provide this informaiton.
	 */
	if (bootm_find_images(flag, argc, argv))
		return 1;

	return 0;
}

int do_booti(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret;

	/* Consume 'booti' */
	argc--; argv++;

	if (booti_start(cmdtp, flag, argc, argv, &images))
		return 1;

	/*
	 * We are doing the BOOTM_STATE_LOADOS state ourselves, so must
	 * disable interrupts ourselves
	 */
	bootm_disable_interrupts();

	images.os.os = IH_OS_LINUX;
	images.os.arch = IH_ARCH_ARM64;
	ret = do_bootm_states(cmdtp, flag, argc, argv,
#ifdef CONFIG_SYS_BOOT_RAMDISK_HIGH
			      BOOTM_STATE_RAMDISK |
#endif
			      BOOTM_STATE_OS_PREP | BOOTM_STATE_OS_FAKE_GO |
			      BOOTM_STATE_OS_GO,
			      &images, 1);

	return ret;
}

#ifdef CONFIG_SYS_LONGHELP
static char booti_help_text[] =
	"[addr [initrd[:size]] [fdt]]\n"
	"    - boot arm64 Linux Image stored in memory\n"
	"\tThe argument 'initrd' is optional and specifies the address\n"
	"\tof an initrd in memory. The optional parameter ':size' allows\n"
	"\tspecifying the size of a RAW initrd.\n"
#if defined(CONFIG_OF_LIBFDT)
	"\tSince booting a Linux kernel requires a flat device-tree, a\n"
	"\tthird argument providing the address of the device-tree blob\n"
	"\tis required. To boot a kernel with a device-tree blob but\n"
	"\twithout an initrd image, use a '-' for the initrd argument.\n"
#endif
	"";
#endif

U_BOOT_CMD(
	booti,	CONFIG_SYS_MAXARGS,	1,	do_booti,
	"boot arm64 Linux Image image from memory", booti_help_text
);

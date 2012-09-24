/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <asm/u-boot.h>
#include <asm/utils.h>
#include <asm/arch/sys_proto.h>
#include <nand.h>
#include <mmc.h>
#include <fat.h>
#include <version.h>
#include <asm/omap_common.h>
#include <asm/arch/mmc_host_def.h>
#include <i2c.h>
#include <image.h>
#include <malloc.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

u32* boot_params_ptr = NULL;
struct spl_image_info spl_image;

/* Define global data structure pointer to it*/
static gd_t gdata __attribute__ ((section(".data")));
static bd_t bdata __attribute__ ((section(".data")));

inline void hang(void)
{
	puts("### ERROR ### Please RESET the board ###\n");
	for (;;)
		;
}

void board_init_f(ulong dummy)
{
	/*
	 * We call relocate_code() with relocation target same as the
	 * CONFIG_SYS_SPL_TEXT_BASE. This will result in relocation getting
	 * skipped. Instead, only .bss initialization will happen. That's
	 * all we need
	 */
	debug(">>board_init_f()\n");
	relocate_code(CONFIG_SPL_STACK, &gdata, CONFIG_SPL_TEXT_BASE);
}

/*
 * Default function to determine if u-boot or the OS should
 * be started. This implementation always returns 1.
 *
 * Please implement your own board specific funcion to do this.
 *
 * RETURN
 * 0 to not start u-boot
 * positive if u-boot should start
 */
#ifdef CONFIG_SPL_OS_BOOT
__weak int spl_start_uboot(void)
{
	printf("SPL: Please implement spl_start_uboot() for your board\n");
	printf("SPL: Direct Linux boot not active!\n");
	return 1;
}
#endif

void spl_parse_image_header(const struct image_header *header)
{
	u32 header_size = sizeof(struct image_header);

	if (__be32_to_cpu(header->ih_magic) == IH_MAGIC) {
		spl_image.size = __be32_to_cpu(header->ih_size) + header_size;
		spl_image.entry_point = __be32_to_cpu(header->ih_load);
		/* Load including the header */
		spl_image.load_addr = spl_image.entry_point - header_size;
		spl_image.os = header->ih_os;
		spl_image.name = (const char *)&header->ih_name;
		debug("spl: payload image: %s load addr: 0x%x size: %d\n",
			spl_image.name, spl_image.load_addr, spl_image.size);
	} else {
		/* Signature not found - assume u-boot.bin */
		printf("mkimage signature not found - ih_magic = %x\n",
			header->ih_magic);
		debug("Assuming u-boot.bin ..\n");
		/* Let's assume U-Boot will not be more than 200 KB */
		spl_image.size = 200 * 1024;
		spl_image.entry_point = CONFIG_SYS_TEXT_BASE;
		spl_image.load_addr = CONFIG_SYS_TEXT_BASE;
		spl_image.os = IH_OS_U_BOOT;
		spl_image.name = "U-Boot";
	}
}

/*
 * This function jumps to an image with argument. Normally an FDT or ATAGS
 * image.
 * arg: Pointer to paramter image in RAM
 */
#ifdef CONFIG_SPL_OS_BOOT
static void __noreturn jump_to_image_linux(void *arg)
{
	debug("Entering kernel arg pointer: 0x%p\n", arg);
	typedef void (*image_entry_arg_t)(int, int, void *)
		__attribute__ ((noreturn));
	image_entry_arg_t image_entry =
		(image_entry_arg_t) spl_image.entry_point;
	cleanup_before_linux();
	image_entry(0, CONFIG_MACH_TYPE, arg);
}
#endif

static void __noreturn jump_to_image_no_args(void)
{
	typedef void __noreturn (*image_entry_noargs_t)(u32 *);
	image_entry_noargs_t image_entry =
			(image_entry_noargs_t) spl_image.entry_point;

	debug("image entry point: 0x%X\n", spl_image.entry_point);
	/* Pass the saved boot_params from rom code */
#if defined(CONFIG_VIRTIO) || defined(CONFIG_ZEBU)
	image_entry = (image_entry_noargs_t)0x80100000;
#endif
	u32 boot_params_ptr_addr = (u32)&boot_params_ptr;
	image_entry((u32 *)boot_params_ptr_addr);
}

void board_init_r(gd_t *id, ulong dummy)
{
	u32 boot_device;
	debug(">>spl:board_init_r()\n");

	mem_malloc_init(CONFIG_SYS_SPL_MALLOC_START,
			CONFIG_SYS_SPL_MALLOC_SIZE);

#ifdef CONFIG_SPL_BOARD_INIT
	spl_board_init();
#endif

	boot_device = omap_boot_device();
	debug("boot device - %d\n", boot_device);
	switch (boot_device) {
#ifdef CONFIG_SPL_MMC_SUPPORT
	case BOOT_DEVICE_MMC1:
	case BOOT_DEVICE_MMC2:
	case BOOT_DEVICE_MMC2_2:
		spl_mmc_load_image();
		break;
#endif
#ifdef CONFIG_SPL_NAND_SUPPORT
	case BOOT_DEVICE_NAND:
		spl_nand_load_image();
		break;
#endif
#ifdef CONFIG_SPL_YMODEM_SUPPORT
	case BOOT_DEVICE_UART:
		spl_ymodem_load_image();
		break;
#endif
	default:
		printf("SPL: Un-supported Boot Device - %d!!!\n", boot_device);
		hang();
		break;
	}

	switch (spl_image.os) {
	case IH_OS_U_BOOT:
		debug("Jumping to U-Boot\n");
		jump_to_image_no_args();
		break;
#ifdef CONFIG_SPL_OS_BOOT
	case IH_OS_LINUX:
		debug("Jumping to Linux\n");
		spl_board_prepare_for_linux();
		jump_to_image_linux((void *)CONFIG_SYS_SPL_ARGS_ADDR);
		break;
#endif
	default:
		puts("Unsupported OS image.. Jumping nevertheless..\n");
		jump_to_image_no_args();
	}
}

/* This requires UART clocks to be enabled */
void preloader_console_init(void)
{
	const char *u_boot_rev = U_BOOT_VERSION;

	gd = &gdata;
	gd->bd = &bdata;
	gd->flags |= GD_FLG_RELOC;
	gd->baudrate = CONFIG_BAUDRATE;

	serial_init();		/* serial communications setup */

	gd->have_console = 1;

	/* Avoid a second "U-Boot" coming from this string */
	u_boot_rev = &u_boot_rev[7];

	printf("\nU-Boot SPL %s (%s - %s)\n", u_boot_rev, U_BOOT_DATE,
		U_BOOT_TIME);
	omap_rev_string();
}

void __weak omap_rev_string()
{
	printf("Texas Instruments Revision detection unimplemented\n");
}

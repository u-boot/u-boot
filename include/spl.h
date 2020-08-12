/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012
 * Texas Instruments, <www.ti.com>
 */
#ifndef	_SPL_H_
#define	_SPL_H_

#include <binman_sym.h>
#include <linker_lists.h>

/* Platform-specific defines */
#include <linux/compiler.h>
#include <asm/global_data.h>
#include <asm/spl.h>
#include <handoff.h>

struct blk_desc;
struct image_header;

/* Value in r0 indicates we booted from U-Boot */
#define UBOOT_NOT_LOADED_FROM_SPL	0x13578642

/* Boot type */
#define MMCSD_MODE_UNDEFINED	0
#define MMCSD_MODE_RAW		1
#define MMCSD_MODE_FS		2
#define MMCSD_MODE_EMMCBOOT	3

struct blk_desc;
struct image_header;

/*
 * u_boot_first_phase() - check if this is the first U-Boot phase
 *
 * U-Boot has up to three phases: TPL, SPL and U-Boot proper. Depending on the
 * build flags we can determine whether the current build is for the first
 * phase of U-Boot or not. If there is no SPL, then this is U-Boot proper. If
 * there is SPL but no TPL, the the first phase is SPL. If there is TPL, then
 * it is the first phase.
 *
 * @returns true if this is the first phase of U-Boot
 *
 */
static inline bool u_boot_first_phase(void)
{
	if (IS_ENABLED(CONFIG_TPL)) {
		if (IS_ENABLED(CONFIG_TPL_BUILD))
			return true;
	} else if (IS_ENABLED(CONFIG_SPL)) {
		if (IS_ENABLED(CONFIG_SPL_BUILD))
			return true;
	} else {
		return true;
	}

	return false;
}

enum u_boot_phase {
	PHASE_TPL,	/* Running in TPL */
	PHASE_SPL,	/* Running in SPL */
	PHASE_BOARD_F,	/* Running in U-Boot before relocation */
	PHASE_BOARD_R,	/* Running in U-Boot after relocation */
};

/**
 * spl_phase() - Find out the phase of U-Boot
 *
 * This can be used to avoid #ifdef logic and use if() instead.
 *
 * For example, to include code only in TPL, you might do:
 *
 *    #ifdef CONFIG_TPL_BUILD
 *    ...
 *    #endif
 *
 * but with this you can use:
 *
 *    if (spl_phase() == PHASE_TPL) {
 *       ...
 *    }
 *
 * To include code only in SPL, you might do:
 *
 *    #if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
 *    ...
 *    #endif
 *
 * but with this you can use:
 *
 *    if (spl_phase() == PHASE_SPL) {
 *       ...
 *    }
 *
 * To include code only in U-Boot proper, you might do:
 *
 *    #ifndef CONFIG_SPL_BUILD
 *    ...
 *    #endif
 *
 * but with this you can use:
 *
 *    if (spl_phase() == PHASE_BOARD_F) {
 *       ...
 *    }
 *
 * @return U-Boot phase
 */
static inline enum u_boot_phase spl_phase(void)
{
#ifdef CONFIG_TPL_BUILD
	return PHASE_TPL;
#elif CONFIG_SPL_BUILD
	return PHASE_SPL;
#else
	DECLARE_GLOBAL_DATA_PTR;

	if (!(gd->flags & GD_FLG_RELOC))
		return PHASE_BOARD_F;
	else
		return PHASE_BOARD_R;
#endif
}

/* A string name for SPL or TPL */
#ifdef CONFIG_SPL_BUILD
# ifdef CONFIG_TPL_BUILD
#  define SPL_TPL_NAME	"TPL"
# else
#  define SPL_TPL_NAME	"SPL"
# endif
# define SPL_TPL_PROMPT	SPL_TPL_NAME ": "
#else
# define SPL_TPL_NAME	""
# define SPL_TPL_PROMPT	""
#endif

struct spl_image_info {
	const char *name;
	u8 os;
	uintptr_t load_addr;
	uintptr_t entry_point;
#if CONFIG_IS_ENABLED(LOAD_FIT) || CONFIG_IS_ENABLED(LOAD_FIT_FULL)
	void *fdt_addr;
#endif
	u32 boot_device;
	u32 size;
	u32 flags;
	void *arg;
#ifdef CONFIG_SPL_LEGACY_IMAGE_CRC_CHECK
	ulong dcrc_data;
	ulong dcrc_length;
	ulong dcrc;
#endif
};

/**
 * Information required to load data from a device
 *
 * @dev: Pointer to the device, e.g. struct mmc *
 * @priv: Private data for the device
 * @bl_len: Block length for reading in bytes
 * @filename: Name of the fit image file.
 * @read: Function to call to read from the device
 */
struct spl_load_info {
	void *dev;
	void *priv;
	int bl_len;
	const char *filename;
	ulong (*read)(struct spl_load_info *load, ulong sector, ulong count,
		      void *buf);
};

/*
 * We need to know the position of U-Boot in memory so we can jump to it. We
 * allow any U-Boot binary to be used (u-boot.bin, u-boot-nodtb.bin,
 * u-boot.img), hence the '_any'. These is no checking here that the correct
 * image is found. For example if u-boot.img is used we don't check that
 * spl_parse_image_header() can parse a valid header.
 *
 * Similarly for SPL, so that TPL can jump to SPL.
 */
binman_sym_extern(ulong, u_boot_any, image_pos);
binman_sym_extern(ulong, u_boot_any, size);
binman_sym_extern(ulong, spl, image_pos);
binman_sym_extern(ulong, spl, size);

/**
 * spl_get_image_pos() - get the image position of the next phase
 *
 * This returns the image position to use to load the next phase of U-Boot
 */
ulong spl_get_image_pos(void);

/**
 * spl_get_image_size() - get the size of the next phase
 *
 * This returns the size to use to load the next phase of U-Boot
 */
ulong spl_get_image_size(void);

/**
 * spl_load_simple_fit_skip_processing() - Hook to allow skipping the FIT
 *	image processing during spl_load_simple_fit().
 *
 * Return true to skip FIT processing, false to preserve the full code flow
 * of spl_load_simple_fit().
 */
bool spl_load_simple_fit_skip_processing(void);

/**
 * spl_load_simple_fit() - Loads a fit image from a device.
 * @spl_image:	Image description to set up
 * @info:	Structure containing the information required to load data.
 * @sector:	Sector number where FIT image is located in the device
 * @fdt:	Pointer to the copied FIT header.
 *
 * Reads the FIT image @sector in the device. Loads u-boot image to
 * specified load address and copies the dtb to end of u-boot image.
 * Returns 0 on success.
 */
int spl_load_simple_fit(struct spl_image_info *spl_image,
			struct spl_load_info *info, ulong sector, void *fdt);

#define SPL_COPY_PAYLOAD_ONLY	1
#define SPL_FIT_FOUND		2

/**
 * spl_load_legacy_img() - Loads a legacy image from a device.
 * @spl_image:	Image description to set up
 * @load:	Structure containing the information required to load data.
 * @header:	Pointer to image header (including appended image)
 *
 * Reads an legacy image from the device. Loads u-boot image to
 * specified load address.
 * Returns 0 on success.
 */
int spl_load_legacy_img(struct spl_image_info *spl_image,
			struct spl_load_info *load, ulong header);

/**
 * spl_load_imx_container() - Loads a imx container image from a device.
 * @spl_image:	Image description to set up
 * @info:	Structure containing the information required to load data.
 * @sector:	Sector number where container image is located in the device
 *
 * Reads the container image @sector in the device. Loads u-boot image to
 * specified load address.
 */
int spl_load_imx_container(struct spl_image_info *spl_image,
			   struct spl_load_info *info, ulong sector);

/* SPL common functions */
void preloader_console_init(void);
u32 spl_boot_device(void);

/**
 * spl_mmc_boot_mode() - Lookup function for the mode of an MMC boot source.
 * @boot_device:	ID of the device which the MMC driver wants to read
 *			from.  Common values are e.g. BOOT_DEVICE_MMC1,
 *			BOOT_DEVICE_MMC2, BOOT_DEVICE_MMC2_2.
 *
 * This function should return one of MMCSD_MODE_FS, MMCSD_MODE_EMMCBOOT, or
 * MMCSD_MODE_RAW for each MMC boot source which is defined for the target.  The
 * boot_device parameter tells which device the MMC driver is interested in.
 *
 * If not overridden, it is weakly defined in common/spl/spl_mmc.c.
 *
 * Note:  It is important to use the boot_device parameter instead of e.g.
 * spl_boot_device() as U-Boot is not always loaded from the same device as SPL.
 */
u32 spl_mmc_boot_mode(const u32 boot_device);

/**
 * spl_mmc_boot_partition() - MMC partition to load U-Boot from.
 * @boot_device:	ID of the device which the MMC driver wants to load
 *			U-Boot from.
 *
 * This function should return the partition number which the SPL
 * should load U-Boot from (on the given boot_device) when
 * CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION is set.
 *
 * If not overridden, it is weakly defined in common/spl/spl_mmc.c.
 */
int spl_mmc_boot_partition(const u32 boot_device);
void spl_set_bd(void);

/**
 * spl_set_header_raw_uboot() - Set up a standard SPL image structure
 *
 * This sets up the given spl_image which the standard values obtained from
 * config options: CONFIG_SYS_MONITOR_LEN, CONFIG_SYS_UBOOT_START,
 * CONFIG_SYS_TEXT_BASE.
 *
 * @spl_image: Image description to set up
 */
void spl_set_header_raw_uboot(struct spl_image_info *spl_image);

/**
 * spl_parse_image_header() - parse the image header and set up info
 *
 * This parses the legacy image header information at @header and sets up
 * @spl_image according to what is found. If no image header is found, then
 * a raw image or bootz is assumed. If CONFIG_SPL_PANIC_ON_RAW_IMAGE is
 * enabled, then this causes a panic. If CONFIG_SPL_RAW_IMAGE_SUPPORT is not
 * enabled then U-Boot gives up. Otherwise U-Boot sets up the image using
 * spl_set_header_raw_uboot(), or possibly the bootz header.
 *
 * @spl_image: Image description to set up
 * @header image header to parse
 * @return 0 if a header was correctly parsed, -ve on error
 */
int spl_parse_image_header(struct spl_image_info *spl_image,
			   const struct image_header *header);

void spl_board_prepare_for_linux(void);
void spl_board_prepare_for_boot(void);
int spl_board_ubi_load_image(u32 boot_device);
int spl_board_boot_device(u32 boot_device);

/**
 * jump_to_image_linux() - Jump to a Linux kernel from SPL
 *
 * This jumps into a Linux kernel using the information in @spl_image.
 *
 * @spl_image: Image description to set up
 */
void __noreturn jump_to_image_linux(struct spl_image_info *spl_image);

/**
 * spl_start_uboot() - Check if SPL should start the kernel or U-Boot
 *
 * This is called by the various SPL loaders to determine whether the board
 * wants to load the kernel or U-Boot. This function should be provided by
 * the board.
 *
 * @return 0 if SPL should start the kernel, 1 if U-Boot must be started
 */
int spl_start_uboot(void);

/**
 * spl_display_print() - Display a board-specific message in SPL
 *
 * If CONFIG_SPL_DISPLAY_PRINT is enabled, U-Boot will call this function
 * immediately after displaying the SPL console banner ("U-Boot SPL ...").
 * This function should be provided by the board.
 */
void spl_display_print(void);

/**
 * struct spl_boot_device - Describes a boot device used by SPL
 *
 * @boot_device: A number indicating the BOOT_DEVICE type. There are various
 * BOOT_DEVICE... #defines and enums in U-Boot and they are not consistently
 * numbered.
 * @boot_device_name: Named boot device, or NULL if none.
 *
 * Note: Additional fields can be added here, bearing in mind that SPL is
 * size-sensitive and common fields will be present on all boards. This
 * struct can also be used to return additional information about the load
 * process if that becomes useful.
 */
struct spl_boot_device {
	uint boot_device;
	const char *boot_device_name;
};

/**
 * Holds information about a way of loading an SPL image
 *
 * @name: User-friendly name for this method (e.g. "MMC")
 * @boot_device: Boot device that this loader supports
 * @load_image: Function to call to load image
 */
struct spl_image_loader {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
	const char *name;
#endif
	uint boot_device;
	/**
	 * load_image() - Load an SPL image
	 *
	 * @spl_image: place to put image information
	 * @bootdev: describes the boot device to load from
	 */
	int (*load_image)(struct spl_image_info *spl_image,
			  struct spl_boot_device *bootdev);
};

/* Declare an SPL image loader */
#define SPL_LOAD_IMAGE(__name)					\
	ll_entry_declare(struct spl_image_loader, __name, spl_image_loader)

/*
 * _priority is the priority of this method, 0 meaning it will be the top
 * choice for this device, 9 meaning it is the bottom choice.
 * _boot_device is the BOOT_DEVICE_... value
 * _method is the load_image function to call
 */
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
#define SPL_LOAD_IMAGE_METHOD(_name, _priority, _boot_device, _method) \
	SPL_LOAD_IMAGE(_boot_device ## _priority ## _method) = { \
		.name = _name, \
		.boot_device = _boot_device, \
		.load_image = _method, \
	}
#else
#define SPL_LOAD_IMAGE_METHOD(_name, _priority, _boot_device, _method) \
	SPL_LOAD_IMAGE(_boot_device ## _priority ## _method) = { \
		.boot_device = _boot_device, \
		.load_image = _method, \
	}
#endif

/* SPL FAT image functions */
int spl_load_image_fat(struct spl_image_info *spl_image,
		       struct blk_desc *block_dev, int partition,
		       const char *filename);
int spl_load_image_fat_os(struct spl_image_info *spl_image,
			  struct blk_desc *block_dev, int partition);

void __noreturn jump_to_image_no_args(struct spl_image_info *spl_image);

/* SPL EXT image functions */
int spl_load_image_ext(struct spl_image_info *spl_image,
		       struct blk_desc *block_dev, int partition,
		       const char *filename);
int spl_load_image_ext_os(struct spl_image_info *spl_image,
			  struct blk_desc *block_dev, int partition);

/**
 * spl_early_init() - Set up device tree and driver model in SPL if enabled
 *
 * Call this function in board_init_f() if you want to use device tree and
 * driver model early, before board_init_r() is called.
 *
 * If this is not called, then driver model will be inactive in SPL's
 * board_init_f(), and no device tree will be available.
 */
int spl_early_init(void);

/**
 * spl_init() - Set up device tree and driver model in SPL if enabled
 *
 * You can optionally call spl_early_init(), then optionally call spl_init().
 * This function will be called from board_init_r() if not called earlier.
 *
 * Both spl_early_init() and spl_init() perform a similar function except that
 * the latter will not set up the malloc() area if
 * CONFIG_SPL_STACK_R_MALLOC_SIMPLE_LEN is enabled, since it is assumed to
 * already be done by a calll to spl_relocate_stack_gd() before board_init_r()
 * is reached.
 *
 * This function will be called from board_init_r() if not called earlier.
 *
 * If this is not called, then driver model will be inactive in SPL's
 * board_init_f(), and no device tree will be available.
 */
int spl_init(void);

#ifdef CONFIG_SPL_BOARD_INIT
void spl_board_init(void);
#endif

/**
 * spl_was_boot_source() - check if U-Boot booted from SPL
 *
 * This will normally be true, but if U-Boot jumps to second U-Boot, it will
 * be false. This should be implemented by board-specific code.
 *
 * @return true if U-Boot booted from SPL, else false
 */
bool spl_was_boot_source(void);

/**
 * spl_dfu_cmd- run dfu command with chosen mmc device interface
 * @param usb_index - usb controller number
 * @param mmc_dev -  mmc device nubmer
 *
 * @return 0 on success, otherwise error code
 */
int spl_dfu_cmd(int usbctrl, char *dfu_alt_info, char *interface, char *devstr);

int spl_mmc_load_image(struct spl_image_info *spl_image,
		       struct spl_boot_device *bootdev);

/**
 * spl_mmc_load() - Load an image file from MMC/SD media
 *
 * @param spl_image	Image data filled in by loading process
 * @param bootdev	Describes which device to load from
 * @param filename	Name of file to load (in FS mode)
 * @param raw_part	Partition to load from (in RAW mode)
 * @param raw_sect	Sector to load from (in RAW mode)
 *
 * @return 0 on success, otherwise error code
 */
int spl_mmc_load(struct spl_image_info *spl_image,
		 struct spl_boot_device *bootdev,
		 const char *filename,
		 int raw_part,
		 unsigned long raw_sect);

/**
 * spl_usb_load() - Load an image file from USB mass storage
 *
 * @param spl_image	Image data filled in by loading process
 * @param bootdev	Describes which device to load from
 * @param raw_part	Fat partition to load from
 * @param filename	Name of file to load
 *
 * @return 0 on success, otherwise error code
 */
int spl_usb_load(struct spl_image_info *spl_image,
		 struct spl_boot_device *bootdev,
		 int partition, const char *filename);

int spl_ymodem_load_image(struct spl_image_info *spl_image,
			  struct spl_boot_device *bootdev);

/**
 * spl_invoke_atf - boot using an ARM trusted firmware image
 */
void spl_invoke_atf(struct spl_image_info *spl_image);

/**
 * bl2_plat_get_bl31_params() - prepare params for bl31.
 * @bl32_entry	address of BL32 executable (secure)
 * @bl33_entry	address of BL33 executable (non secure)
 * @fdt_addr	address of Flat Device Tree
 *
 * This function assigns a pointer to the memory that the platform has kept
 * aside to pass platform specific and trusted firmware related information
 * to BL31. This memory is allocated by allocating memory to
 * bl2_to_bl31_params_mem structure which is a superset of all the
 * structure whose information is passed to BL31
 * NOTE: This function should be called only once and should be done
 * before generating params to BL31
 *
 * @return bl31 params structure pointer
 */
struct bl31_params *bl2_plat_get_bl31_params(uintptr_t bl32_entry,
					     uintptr_t bl33_entry,
					     uintptr_t fdt_addr);

/**
 * spl_optee_entry - entry function for optee
 *
 * args defind in op-tee project
 * https://github.com/OP-TEE/optee_os/
 * core/arch/arm/kernel/generic_entry_a32.S
 * @arg0: pagestore
 * @arg1: (ARMv7 standard bootarg #1)
 * @arg2: device tree address, (ARMv7 standard bootarg #2)
 * @arg3: non-secure entry address (ARMv7 bootarg #0)
 */
void spl_optee_entry(void *arg0, void *arg1, void *arg2, void *arg3);

/**
 * spl_invoke_opensbi - boot using a RISC-V OpenSBI image
 */
void spl_invoke_opensbi(struct spl_image_info *spl_image);

/**
 * board_return_to_bootrom - allow for boards to continue with the boot ROM
 *
 * If a board (e.g. the Rockchip RK3368 boards) provide some
 * supporting functionality for SPL in their boot ROM and the SPL
 * stage wants to return to the ROM code to continue booting, boards
 * can implement 'board_return_to_bootrom'.
 */
int board_return_to_bootrom(struct spl_image_info *spl_image,
			    struct spl_boot_device *bootdev);

/**
 * board_spl_fit_post_load - allow process images after loading finished
 *
 */
void board_spl_fit_post_load(ulong load_addr, size_t length);

/**
 * board_spl_fit_size_align - specific size align before processing payload
 *
 */
ulong board_spl_fit_size_align(ulong size);

/**
 * spl_perform_fixups() - arch/board-specific callback before processing
 *                        the boot-payload
 */
void spl_perform_fixups(struct spl_image_info *spl_image);

/*
 * spl_get_load_buffer() - get buffer for loading partial image data
 *
 * Returns memory area which can be populated by partial image data,
 * ie. uImage or fitImage header.
 */
struct image_header *spl_get_load_buffer(ssize_t offset, size_t size);

void spl_save_restore_data(void);
#endif

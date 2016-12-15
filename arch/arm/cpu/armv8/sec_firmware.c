/*
 * Copyright 2016 NXP Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/types.h>
#include <asm/macro.h>
#include <asm/armv8/sec_firmware.h>

DECLARE_GLOBAL_DATA_PTR;
extern void c_runtime_cpu_setup(void);

#define SEC_FIRMWARE_LOADED	0x1
#define SEC_FIRMWARE_RUNNING	0x2
#define SEC_FIRMWARE_ADDR_MASK	(~0x3)
	/*
	 * Secure firmware load addr
	 * Flags used: 0x1 secure firmware has been loaded to secure memory
	 *             0x2 secure firmware is running
	 */
	phys_addr_t sec_firmware_addr;

static int sec_firmware_get_data(const void *sec_firmware_img,
				const void **data, size_t *size)
{
	int conf_node_off, fw_node_off;
	char *conf_node_name = NULL;
	char *desc;
	int ret;

	conf_node_name = SEC_FIRMEWARE_FIT_CNF_NAME;

	conf_node_off = fit_conf_get_node(sec_firmware_img, conf_node_name);
	if (conf_node_off < 0) {
		printf("SEC Firmware: %s: no such config\n", conf_node_name);
		return -ENOENT;
	}

	fw_node_off = fit_conf_get_prop_node(sec_firmware_img, conf_node_off,
			SEC_FIRMWARE_FIT_IMAGE);
	if (fw_node_off < 0) {
		printf("SEC Firmware: No '%s' in config\n",
		       SEC_FIRMWARE_FIT_IMAGE);
		return -ENOLINK;
	}

	/* Verify secure firmware image */
	if (!(fit_image_verify(sec_firmware_img, fw_node_off))) {
		printf("SEC Firmware: Bad firmware image (bad CRC)\n");
		return -EINVAL;
	}

	if (fit_image_get_data(sec_firmware_img, fw_node_off, data, size)) {
		printf("SEC Firmware: Can't get %s subimage data/size",
		       SEC_FIRMWARE_FIT_IMAGE);
		return -ENOENT;
	}

	ret = fit_get_desc(sec_firmware_img, fw_node_off, &desc);
	if (ret)
		printf("SEC Firmware: Can't get description\n");
	else
		printf("%s\n", desc);

	return ret;
}

/*
 * SEC Firmware FIT image parser checks if the image is in FIT
 * format, verifies integrity of the image and calculates raw
 * image address and size values.
 *
 * Returns 0 on success and a negative errno on error task fail.
 */
static int sec_firmware_parse_image(const void *sec_firmware_img,
					const void **raw_image_addr,
					size_t *raw_image_size)
{
	int ret;

	ret = sec_firmware_get_data(sec_firmware_img, raw_image_addr,
					raw_image_size);
	if (ret)
		return ret;

	debug("SEC Firmware: raw_image_addr = 0x%p, raw_image_size = 0x%lx\n",
	      *raw_image_addr, *raw_image_size);

	return 0;
}

static int sec_firmware_copy_image(const char *title,
			 u64 image_addr, u32 image_size, u64 sec_firmware)
{
	debug("%s copied to address 0x%p\n", title, (void *)sec_firmware);
	memcpy((void *)sec_firmware, (void *)image_addr, image_size);
	flush_dcache_range(sec_firmware, sec_firmware + image_size);

	return 0;
}

/*
 * This function will parse the SEC Firmware image, and then load it
 * to secure memory.
 */
static int sec_firmware_load_image(const void *sec_firmware_img)
{
	const void *raw_image_addr;
	size_t raw_image_size = 0;
	int ret;

	/*
	 * The Excetpion Level must be EL3 to load and initialize
	 * the SEC Firmware.
	 */
	if (current_el() != 3) {
		ret = -EACCES;
		goto out;
	}

#ifdef CONFIG_SYS_MEM_RESERVE_SECURE
	/*
	 * The SEC Firmware must be stored in secure memory.
	 * Append SEC Firmware to secure mmu table.
	 */
	if (!(gd->arch.secure_ram & MEM_RESERVE_SECURE_MAINTAINED)) {
		ret = -ENXIO;
		goto out;
	}

	sec_firmware_addr = (gd->arch.secure_ram & MEM_RESERVE_SECURE_ADDR_MASK) +
			gd->arch.tlb_size;
#else
#error "The CONFIG_SYS_MEM_RESERVE_SECURE must be defined when enabled SEC Firmware support"
#endif

	/* Align SEC Firmware base address to 4K */
	sec_firmware_addr = (sec_firmware_addr + 0xfff) & ~0xfff;
	debug("SEC Firmware: Load address: 0x%llx\n",
	      sec_firmware_addr & SEC_FIRMWARE_ADDR_MASK);

	ret = sec_firmware_parse_image(sec_firmware_img, &raw_image_addr,
			&raw_image_size);
	if (ret)
		goto out;

	/* TODO:
	 * Check if the end addr of SEC Firmware has been extend the secure
	 * memory.
	 */

	/* Copy the secure firmware to secure memory */
	ret = sec_firmware_copy_image("SEC Firmware", (u64)raw_image_addr,
			raw_image_size, sec_firmware_addr &
			SEC_FIRMWARE_ADDR_MASK);
	if (ret)
		goto out;

	sec_firmware_addr |= SEC_FIRMWARE_LOADED;
	debug("SEC Firmware: Entry point: 0x%llx\n",
	      sec_firmware_addr & SEC_FIRMWARE_ADDR_MASK);

	return 0;

out:
	printf("SEC Firmware: error (%d)\n", ret);
	sec_firmware_addr = 0;

	return ret;
}

static int sec_firmware_entry(u32 *eret_hold_l, u32 *eret_hold_h)
{
	const void *entry = (void *)(sec_firmware_addr &
				SEC_FIRMWARE_ADDR_MASK);

	return _sec_firmware_entry(entry, eret_hold_l, eret_hold_h);
}

/* Check the secure firmware FIT image */
__weak bool sec_firmware_is_valid(const void *sec_firmware_img)
{
	if (fdt_check_header(sec_firmware_img)) {
		printf("SEC Firmware: Bad firmware image (not a FIT image)\n");
		return false;
	}

	if (!fit_check_format(sec_firmware_img)) {
		printf("SEC Firmware: Bad firmware image (bad FIT header)\n");
		return false;
	}

	return true;
}

#ifdef CONFIG_ARMV8_PSCI
/*
 * The PSCI_VERSION function is added from PSCI v0.2. When the PSCI
 * v0.1 received this function, the NOT_SUPPORTED (0xffff_ffff) error
 * number will be returned according to SMC Calling Conventions. But
 * when getting the NOT_SUPPORTED error number, we cannot ensure if
 * the PSCI version is v0.1 or other error occurred. So, PSCI v0.1
 * won't be supported by this framework.
 * And if the secure firmware isn't running, return NOT_SUPPORTED.
 *
 * The return value on success is PSCI version in format
 * major[31:16]:minor[15:0].
 */
unsigned int sec_firmware_support_psci_version(void)
{
	if (sec_firmware_addr & SEC_FIRMWARE_RUNNING)
		return _sec_firmware_support_psci_version();

	return 0xffffffff;
}
#endif

/*
 * sec_firmware_init - Initialize the SEC Firmware
 * @sec_firmware_img:	the SEC Firmware image address
 * @eret_hold_l:	the address to hold exception return address low
 * @eret_hold_h:	the address to hold exception return address high
 */
int sec_firmware_init(const void *sec_firmware_img,
			u32 *eret_hold_l,
			u32 *eret_hold_h)
{
	int ret;

	if (!sec_firmware_is_valid(sec_firmware_img))
		return -EINVAL;

	ret = sec_firmware_load_image(sec_firmware_img);
	if (ret) {
		printf("SEC Firmware: Failed to load image\n");
		return ret;
	} else if (sec_firmware_addr & SEC_FIRMWARE_LOADED) {
		ret = sec_firmware_entry(eret_hold_l, eret_hold_h);
		if (ret) {
			printf("SEC Firmware: Failed to initialize\n");
			return ret;
		}
	}

	debug("SEC Firmware: Return from SEC Firmware: current_el = %d\n",
	      current_el());

	/*
	 * The PE will be turned into target EL when returned from
	 * SEC Firmware.
	 */
	if (current_el() != SEC_FIRMWARE_TARGET_EL)
		return -EACCES;

	sec_firmware_addr |= SEC_FIRMWARE_RUNNING;

	/* Set exception table and enable caches if it isn't EL3 */
	if (current_el() != 3) {
		c_runtime_cpu_setup();
		enable_caches();
	}

	return 0;
}

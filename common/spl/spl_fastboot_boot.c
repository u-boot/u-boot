// SPDX-License-Identifier: GPL-2.0+
/* Copyright (C) 2026 Carlo Caione <ccaione@baylibre.com> */

#include <android_image.h>
#include <fastboot.h>
#include <spl.h>
#include <linux/log2.h>

struct fastboot_image {
	const void *data;
	size_t size;
};

/* The standard fastboot client wraps non-Android images in a v0 header. */
static int spl_fastboot_unwrap(struct fastboot_image *image)
{
	const struct andr_boot_img_hdr_v0 *hdr = image->data;
	u32 page_size, kernel_size;
	size_t hdr_size = offsetof(struct andr_boot_img_hdr_v0,
				   recovery_dtbo_size);

	if (image->size < ANDR_BOOT_MAGIC_SIZE ||
	    memcmp(image->data, ANDR_BOOT_MAGIC, ANDR_BOOT_MAGIC_SIZE))
		return 0;

	if (image->size < hdr_size)
		return -EINVAL;
	if (le32_to_cpu(hdr->header_version) || hdr->ramdisk_size ||
	    hdr->second_size)
		return -EPROTONOSUPPORT;

	page_size = le32_to_cpu(hdr->page_size);
	kernel_size = le32_to_cpu(hdr->kernel_size);
	if (!is_power_of_2(page_size) || page_size < hdr_size ||
	    page_size > image->size || kernel_size > image->size - page_size)
		return -EINVAL;

	image->data += page_size;
	image->size = kernel_size;
	return 0;
}

int spl_fastboot_load(struct spl_image_info *spl_image)
{
	struct fastboot_image image;
	void *buf;
	int ret;

	image.size = fastboot_get_boot_image(&buf);
	image.data = buf;
	if (!image.size)
		return -EAGAIN;

	ret = spl_fastboot_unwrap(&image);
	if (ret)
		return ret;
	return spl_ram_load(spl_image, (void *)image.data, image.size);
}

/* Sandbox tests the loader directly, without a physical USB boot method. */
#if !IS_ENABLED(CONFIG_SANDBOX)
static int spl_fastboot_load_image(struct spl_image_info *spl_image,
				   struct spl_boot_device *bootdev)
{
	int ret;

	ret = fastboot_usb_run(CONFIG_SPL_FASTBOOT_USB_DEV, NULL, 0);
	if (ret)
		return ret;

	return spl_fastboot_load(spl_image);
}

SPL_LOAD_IMAGE_METHOD("USB fastboot", 0, BOOT_DEVICE_BOARD,
		      spl_fastboot_load_image);
#endif

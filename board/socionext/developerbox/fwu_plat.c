// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021, Linaro Limited
 */

#include <dfu.h>
#include <efi_loader.h>
#include <flash.h>
#include <fwu.h>
#include <fwu_mdata.h>
#include <malloc.h>
#include <memalign.h>
#include <mtd.h>
#include <spi.h>
#include <spi_flash.h>
#include <uuid.h>

#include <linux/errno.h>
#include <linux/types.h>
#include <u-boot/crc.h>

/* SPI Flash accessors */
static struct spi_flash *plat_spi_flash;

static int __plat_sf_get_flash(void)
{
	struct udevice *new;
	int	ret;

	//TODO: define platform spi-flash somewhere.
	ret = spi_flash_probe_bus_cs(CONFIG_SF_DEFAULT_BUS, CONFIG_SF_DEFAULT_CS,
				     CONFIG_SF_DEFAULT_SPEED, CONFIG_SF_DEFAULT_MODE,
				     &new);
	if (ret)
		return ret;

	plat_spi_flash = dev_get_uclass_priv(new);
	return 0;
}

static int plat_sf_get_flash(struct spi_flash **flash)
{
	int ret = 0;

	if (!plat_spi_flash)
		ret = __plat_sf_get_flash();

	*flash = plat_spi_flash;

	return ret;
}

static int sf_load_data(u32 offs, u32 size, void **data)
{
	struct spi_flash *flash;
	int ret;

	ret = plat_sf_get_flash(&flash);
	if (ret < 0)
		return ret;

	*data = memalign(ARCH_DMA_MINALIGN, size);
	if (!*data)
		return -ENOMEM;

	ret = spi_flash_read(flash, offs, size, *data);
	if (ret < 0) {
		free(*data);
		*data = NULL;
	}

	return ret;
}

static int sf_save_data(u32 offs, u32 size, void *data)
{
	struct spi_flash *flash;
	u32 sect_size, nsect;
	void *buf;
	int ret;

	ret = plat_sf_get_flash(&flash);
	if (ret < 0)
		return ret;

	sect_size = flash->mtd.erasesize;
	nsect = DIV_ROUND_UP(size, sect_size);
	ret = spi_flash_erase(flash, offs, nsect * sect_size);
	if (ret < 0)
		return ret;

	buf = memalign(ARCH_DMA_MINALIGN, size);
	if (!buf)
		return -ENOMEM;
	memcpy(buf, data, size);

	ret = spi_flash_write(flash, offs, size, buf);

	free(buf);

	return ret;
}

#define DFU_ALT_BUF_LEN 256
#define DFU_ALT_NUM_MAX (CONFIG_FWU_NUM_IMAGES_PER_BANK * CONFIG_FWU_NUM_BANKS)

/* Generate dfu_alt_info from partitions */
void set_dfu_alt_info(char *interface, char *devstr)
{
	int ret;
	struct mtd_info *mtd;
	static char *buf = NULL;

	if (!buf) {
		buf = malloc_cache_aligned(DFU_ALT_BUF_LEN);
		memset(buf, 0, DFU_ALT_BUF_LEN);

		mtd_probe_devices();

		mtd = get_mtd_device_nm("nor1");
		if (IS_ERR_OR_NULL(mtd))
			return;

		ret = fwu_gen_alt_info_from_mtd(buf, DFU_ALT_BUF_LEN, mtd);
		if (ret < 0) {
			log_err("Error: Failed to generate dfu_alt_info. (%d)\n", ret);
			return;
		}
		log_debug("Make dfu_alt_info: '%s'\n", buf);
	}
	env_set("dfu_alt_info", buf);
}

#define PLAT_METADATA_OFFSET	0x510000
#define PLAT_METADATA_SIZE	(sizeof(struct devbox_metadata))

struct __packed devbox_metadata {
	u32 boot_index;
	u32 boot_count;
} *devbox_plat_metadata;

int fwu_plat_get_alt_num(struct udevice __always_unused *dev,
			 efi_guid_t *image_id, int *alt_num)
{
	return fwu_get_mtd_alt_num(image_id, alt_num, "nor1", 0);
}

int fwu_plat_get_update_index(u32 *update_idx)
{
	int ret;
	u32 active_idx;

	ret = fwu_get_active_index(&active_idx);

	if (ret < 0)
		return ret;

	*update_idx = (active_idx + 1) % CONFIG_FWU_NUM_BANKS;

	return ret;
}

static int devbox_load_plat_metadata(void)
{
	if (devbox_plat_metadata)
		return 0;

	return sf_load_data(PLAT_METADATA_OFFSET, PLAT_METADATA_SIZE,
			 (void **)&devbox_plat_metadata);
}

void fwu_plat_get_bootidx(void *boot_idx)
{
	u32 *bootidx = boot_idx;

	if (devbox_load_plat_metadata() < 0)
		*bootidx = 0;
	else
		*bootidx = devbox_plat_metadata->boot_index;
}

int board_late_init(void)
{
	int ret;

	/* Make mmc available for EFI */
	run_command("mmc dev 0", 0);

	ret = devbox_load_plat_metadata();
	if (ret < 0)
		return ret;

	if (devbox_plat_metadata->boot_count) {
		/* We are in the platform trial boot. Finish it. */
		devbox_plat_metadata->boot_count = 0;
		ret = sf_save_data(PLAT_METADATA_OFFSET, PLAT_METADATA_SIZE,
				   (void *)devbox_plat_metadata);
		if (ret < 0)
			return ret;

		pr_debug("FWU: Finish platform trial boot safely.\n");
	}

	return 0;
}

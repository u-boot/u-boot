// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Kontron Electronics GmbH
 */

#include <asm/arch/imx-regs.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <dfu.h>
#include <efi.h>
#include <efi_loader.h>
#include <fdt_support.h>
#include <memalign.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/sizes.h>
#include <net.h>

DECLARE_GLOBAL_DATA_PTR;

#if CONFIG_IS_ENABLED(EFI_HAVE_CAPSULE_SUPPORT)
struct efi_fw_images fw_images[] = {
	{
		.image_type_id = KONTRON_SL_MX8MM_FIT_IMAGE_GUID,
		.fw_name = u"KONTROL-SL-MX8MM-UBOOT",
		.image_index = 1
	},
};

u8 num_image_type_guids = ARRAY_SIZE(fw_images);
#endif /* EFI_HAVE_CAPSULE_SUPPORT */

int board_phys_sdram_size(phys_size_t *size)
{
	u32 ddr_size = readl(M4_BOOTROM_BASE_ADDR);

	if (ddr_size == 4) {
		*size = 0x100000000;
	} else if (ddr_size == 3) {
		*size = 0xc0000000;
	} else if (ddr_size == 2) {
		*size = 0x80000000;
	} else if (ddr_size == 1) {
		*size = 0x40000000;
	} else {
		printf("Unknown DDR type!!!\n");
		*size = 0x40000000;
	}

	return 0;
}

/*
 * If the SoM is mounted on a baseboard with a USB ethernet controller,
 * there might be an additional MAC address programmed to the MAC OTP fuses.
 * Although the i.MX8MM has only one MAC, the MAC0, MAC1 and MAC2 registers
 * in the OTP fuses can still be used to store two separate addresses.
 * Try to read the secondary address from MAC1 and MAC2 and adjust the
 * devicetree so Linux can pick up the MAC address.
 */
int fdt_set_usb_eth_addr(void *blob)
{
	u32 value = readl(OCOTP_BASE_ADDR + 0x660);
	unsigned char mac[6];
	int node, ret;

	mac[0] = value >> 24;
	mac[1] = value >> 16;
	mac[2] = value >> 8;
	mac[3] = value;

	value = readl(OCOTP_BASE_ADDR + 0x650);
	mac[4] = value >> 24;
	mac[5] = value >> 16;

	node = fdt_path_offset(blob, fdt_get_alias(blob, "ethernet1"));
	if (node < 0) {
		/*
		 * There is no node for the USB ethernet in the devicetree. Just skip.
		 */
		return 0;
	}

	if (is_zero_ethaddr(mac)) {
		printf("\nNo MAC address for USB ethernet set in OTP fuses!\n");
		return 0;
	}

	if (!is_valid_ethaddr(mac)) {
		printf("\nInvalid MAC address for USB ethernet set in OTP fuses!\n");
		return -EINVAL;
	}

	ret = fdt_setprop(blob, node, "local-mac-address", &mac, 6);
	if (ret)
		ret = fdt_setprop(blob, node, "mac-address", &mac, 6);

	if (ret)
		printf("\nMissing mac-address or local-mac-address property in dt, skip setting MAC address for USB ethernet\n");

	return 0;
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	int ret = fdt_set_usb_eth_addr(blob);

	if (ret)
		return ret;

	return fdt_fixup_memory(blob, PHYS_SDRAM, gd->ram_size);
}

int board_init(void)
{
	return 0;
}

#if defined(CONFIG_SET_DFU_ALT_INFO)

#define DFU_ALT_BUF_LEN		SZ_1K

void set_dfu_alt_info(char *interface, char *devstr)
{
	ALLOC_CACHE_ALIGN_BUFFER(char, buf, DFU_ALT_BUF_LEN);

	if (!CONFIG_IS_ENABLED(EFI_HAVE_CAPSULE_SUPPORT) &&
	    env_get("dfu_alt_info"))
		return;

	memset(buf, 0, sizeof(buf));

	snprintf(buf, DFU_ALT_BUF_LEN,
		 "sf 0:0=flash-bin raw 0x400 0x1f0000");

	env_set("dfu_alt_info", buf);
}
#endif /* CONFIG_SET_DFU_ALT_INFO */

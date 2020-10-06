// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Beniamino Galvani <b.galvani@gmail.com>
 * (C) Copyright 2018 Neil Armstrong <narmstrong@baylibre.com>
 */

#include <common.h>
#include <init.h>
#include <net.h>
#include <asm/arch/boot.h>
#include <asm/arch/eth.h>
#include <asm/arch/gx.h>
#include <asm/arch/mem.h>
#include <asm/arch/meson-vpu.h>
#include <asm/io.h>
#include <asm/armv8/mmu.h>
#include <linux/sizes.h>
#include <usb.h>
#include <linux/usb/otg.h>
#include <asm/arch/usb-gx.h>
#include <usb/dwc2_udc.h>
#include <clk.h>
#include <phy.h>

DECLARE_GLOBAL_DATA_PTR;

int meson_get_boot_device(void)
{
	return readl(GX_AO_SEC_GP_CFG0) & GX_AO_BOOT_DEVICE;
}

/* Configure the reserved memory zones exported by the secure registers
 * into EFI and DTB reserved memory entries.
 */
void meson_init_reserved_memory(void *fdt)
{
	u64 bl31_size, bl31_start;
	u64 bl32_size, bl32_start;
	u32 reg;

	/*
	 * Get ARM Trusted Firmware reserved memory zones in :
	 * - AO_SEC_GP_CFG3: bl32 & bl31 size in KiB, can be 0
	 * - AO_SEC_GP_CFG5: bl31 physical start address, can be NULL
	 * - AO_SEC_GP_CFG4: bl32 physical start address, can be NULL
	 */
	reg = readl(GX_AO_SEC_GP_CFG3);

	bl31_size = ((reg & GX_AO_BL31_RSVMEM_SIZE_MASK)
			>> GX_AO_BL31_RSVMEM_SIZE_SHIFT) * SZ_1K;
	bl32_size = (reg & GX_AO_BL32_RSVMEM_SIZE_MASK) * SZ_1K;

	bl31_start = readl(GX_AO_SEC_GP_CFG5);
	bl32_start = readl(GX_AO_SEC_GP_CFG4);

	/*
	 * Early Meson GX Firmware revisions did not provide the reserved
	 * memory zones in the registers, keep fixed memory zone handling.
	 */
	if (IS_ENABLED(CONFIG_MESON_GX) &&
	    !reg && !bl31_start && !bl32_start) {
		bl31_start = 0x10000000;
		bl31_size = 0x200000;
	}

	/* Add first 16MiB reserved zone */
	meson_board_add_reserved_memory(fdt, 0, GX_FIRMWARE_MEM_SIZE);

	/* Add BL31 reserved zone */
	if (bl31_start && bl31_size)
		meson_board_add_reserved_memory(fdt, bl31_start, bl31_size);

	/* Add BL32 reserved zone */
	if (bl32_start && bl32_size)
		meson_board_add_reserved_memory(fdt, bl32_start, bl32_size);

#if defined(CONFIG_VIDEO_MESON)
	meson_vpu_rsv_fb(fdt);
#endif
}

phys_size_t get_effective_memsize(void)
{
	/* Size is reported in MiB, convert it in bytes */
	return ((readl(GX_AO_SEC_GP_CFG0) & GX_AO_MEM_SIZE_MASK)
			>> GX_AO_MEM_SIZE_SHIFT) * SZ_1M;
}

static struct mm_region gx_mem_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0xc0000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0xc0000000UL,
		.phys = 0xc0000000UL,
		.size = 0x30000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = gx_mem_map;

/* Configure the Ethernet MAC with the requested interface mode
 * with some optional flags.
 */
void meson_eth_init(phy_interface_t mode, unsigned int flags)
{
	switch (mode) {
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		/* Set RGMII mode */
		setbits_le32(GX_ETH_REG_0, GX_ETH_REG_0_PHY_INTF |
			     GX_ETH_REG_0_TX_PHASE(1) |
			     GX_ETH_REG_0_TX_RATIO(4) |
			     GX_ETH_REG_0_PHY_CLK_EN |
			     GX_ETH_REG_0_CLK_EN);

		/* Reset to external PHY */
		if(!IS_ENABLED(CONFIG_MESON_GXBB))
			writel(0x2009087f, GX_ETH_REG_3);

		break;

	case PHY_INTERFACE_MODE_RMII:
		/* Set RMII mode */
		out_le32(GX_ETH_REG_0, GX_ETH_REG_0_INVERT_RMII_CLK |
					 GX_ETH_REG_0_CLK_EN);

		/* Use GXL RMII Internal PHY (also on GXM) */
		if (!IS_ENABLED(CONFIG_MESON_GXBB)) {
			if ((flags & MESON_USE_INTERNAL_RMII_PHY)) {
				writel(0x10110181, GX_ETH_REG_2);
				writel(0xe40908ff, GX_ETH_REG_3);
			} else
				writel(0x2009087f, GX_ETH_REG_3);
		}

		break;

	default:
		printf("Invalid Ethernet interface mode\n");
		return;
	}

	/* Enable power gate */
	clrbits_le32(GX_MEM_PD_REG_0, GX_MEM_PD_REG_0_ETH_MASK);
}

#if CONFIG_IS_ENABLED(USB_DWC3_MESON_GXL) && \
	CONFIG_IS_ENABLED(USB_GADGET_DWC2_OTG)
static struct dwc2_plat_otg_data meson_gx_dwc2_data;

int board_usb_init(int index, enum usb_init_type init)
{
	struct fdtdec_phandle_args args;
	const void *blob = gd->fdt_blob;
	int node, dwc2_node;
	struct udevice *dev, *clk_dev;
	struct clk clk;
	int ret;

	/* find the usb glue node */
	node = fdt_node_offset_by_compatible(blob, -1,
					     "amlogic,meson-gxl-usb-ctrl");
	if (node < 0) {
		node = fdt_node_offset_by_compatible(blob, -1,
					"amlogic,meson-gxm-usb-ctrl");
		if (node < 0) {
			debug("Not found usb-control node\n");
			return -ENODEV;
		}
	}

	if (!fdtdec_get_is_enabled(blob, node)) {
		debug("usb is disabled in the device tree\n");
		return -ENODEV;
	}

	ret = uclass_get_device_by_of_offset(UCLASS_SIMPLE_BUS, node, &dev);
	if (ret) {
		debug("Not found usb-control device\n");
		return ret;
	}

	/* find the dwc2 node */
	dwc2_node = fdt_node_offset_by_compatible(blob, node,
						  "amlogic,meson-g12a-usb");
	if (dwc2_node < 0) {
		debug("Not found dwc2 node\n");
		return -ENODEV;
	}

	if (!fdtdec_get_is_enabled(blob, dwc2_node)) {
		debug("dwc2 is disabled in the device tree\n");
		return -ENODEV;
	}

	meson_gx_dwc2_data.regs_otg = fdtdec_get_addr(blob, dwc2_node, "reg");
	if (meson_gx_dwc2_data.regs_otg == FDT_ADDR_T_NONE) {
		debug("usbotg: can't get base address\n");
		return -ENODATA;
	}

	/* Enable clock */
	ret = fdtdec_parse_phandle_with_args(blob, dwc2_node, "clocks",
					     "#clock-cells", 0, 0, &args);
	if (ret) {
		debug("usbotg has no clocks defined in the device tree\n");
		return ret;
	}

	ret = uclass_get_device_by_of_offset(UCLASS_CLK, args.node, &clk_dev);
	if (ret)
		return ret;

	if (args.args_count != 1) {
		debug("Can't find clock ID in the device tree\n");
		return -ENODATA;
	}

	clk.dev = clk_dev;
	clk.id = args.args[0];

	ret = clk_enable(&clk);
	if (ret) {
		debug("Failed to enable usbotg clock\n");
		return ret;
	}

	meson_gx_dwc2_data.rx_fifo_sz = fdtdec_get_int(blob, dwc2_node,
						     "g-rx-fifo-size", 0);
	meson_gx_dwc2_data.np_tx_fifo_sz = fdtdec_get_int(blob, dwc2_node,
							"g-np-tx-fifo-size", 0);
	meson_gx_dwc2_data.tx_fifo_sz = fdtdec_get_int(blob, dwc2_node,
						     "g-tx-fifo-size", 0);

	/* Switch to peripheral mode */
	ret = dwc3_meson_gxl_force_mode(dev, USB_DR_MODE_PERIPHERAL);
	if (ret)
		return ret;

	return dwc2_udc_probe(&meson_gx_dwc2_data);
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	const void *blob = gd->fdt_blob;
	struct udevice *dev;
	int node;
	int ret;

	/* find the usb glue node */
	node = fdt_node_offset_by_compatible(blob, -1,
					     "amlogic,meson-gxl-usb-ctrl");
	if (node < 0) {
		node = fdt_node_offset_by_compatible(blob, -1,
					"amlogic,meson-gxm-usb-ctrl");
		if (node < 0) {
			debug("Not found usb-control node\n");
			return -ENODEV;
		}
	}

	if (!fdtdec_get_is_enabled(blob, node))
		return -ENODEV;

	ret = uclass_get_device_by_of_offset(UCLASS_SIMPLE_BUS, node, &dev);
	if (ret)
		return ret;

	/* Switch to OTG mode */
	ret = dwc3_meson_gxl_force_mode(dev, USB_DR_MODE_HOST);
	if (ret)
		return ret;

	return 0;
}
#endif

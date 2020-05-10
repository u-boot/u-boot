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

#if CONFIG_IS_ENABLED(USB_XHCI_DWC3_OF_SIMPLE) && \
	CONFIG_IS_ENABLED(USB_GADGET_DWC2_OTG)
static struct dwc2_plat_otg_data meson_gx_dwc2_data;
static struct phy usb_phys[2];

int board_usb_init(int index, enum usb_init_type init)
{
	struct ofnode_phandle_args args;
	struct udevice *clk_dev;
	ofnode dwc2_node;
	struct clk clk;
	int ret, i;
	u32 val;

	/* find the dwc2 node */
	dwc2_node = ofnode_by_compatible(ofnode_null(), "snps,dwc2");
	if (!ofnode_valid(dwc2_node)) {
		debug("Not found dwc2 node\n");
		return -ENODEV;
	}

	if (!ofnode_is_available(dwc2_node)) {
		debug("dwc2 is disabled in the device tree\n");
		return -ENODEV;
	}

	/* get the PHYs */
	for (i = 0; i < 2; i++) {
		ret = generic_phy_get_by_index_nodev(dwc2_node, i,
						     &usb_phys[i]);
		if (ret && ret != -ENOENT) {
			pr_err("Failed to get USB PHY%d for %s\n",
			       i, ofnode_get_name(dwc2_node));
			return ret;
		}
	}

	for (i = 0; i < 2; i++) {
		ret = generic_phy_init(&usb_phys[i]);
		if (ret) {
			pr_err("Can't init USB PHY%d for %s\n",
			       i, ofnode_get_name(dwc2_node));
			return ret;
		}
	}

	for (i = 0; i < 2; i++) {
		ret = generic_phy_power_on(&usb_phys[i]);
		if (ret) {
			pr_err("Can't power USB PHY%d for %s\n",
			       i, ofnode_get_name(dwc2_node));
			return ret;
		}
	}

	phy_meson_gxl_usb3_set_mode(&usb_phys[0], USB_DR_MODE_PERIPHERAL);
	phy_meson_gxl_usb2_set_mode(&usb_phys[1], USB_DR_MODE_PERIPHERAL);

	meson_gx_dwc2_data.regs_otg = ofnode_get_addr(dwc2_node);
	if (meson_gx_dwc2_data.regs_otg == FDT_ADDR_T_NONE) {
		debug("usbotg: can't get base address\n");
		return -ENODATA;
	}

	/* Enable clock */
	ret = ofnode_parse_phandle_with_args(dwc2_node, "clocks",
					     "#clock-cells", 0, 0, &args);
	if (ret) {
		debug("usbotg has no clocks defined in the device tree\n");
		return ret;
	}

	ret = uclass_get_device_by_ofnode(UCLASS_CLK, args.node, &clk_dev);
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

	ofnode_read_u32(dwc2_node, "g-rx-fifo-size", &val);
	meson_gx_dwc2_data.rx_fifo_sz = val;
	ofnode_read_u32(dwc2_node, "g-np-tx-fifo-size", &val);
	meson_gx_dwc2_data.np_tx_fifo_sz = val;
	ofnode_read_u32(dwc2_node, "g-tx-fifo-size", &val);
	meson_gx_dwc2_data.tx_fifo_sz = val;

	return dwc2_udc_probe(&meson_gx_dwc2_data);
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	int i;

	phy_meson_gxl_usb3_set_mode(&usb_phys[0], USB_DR_MODE_HOST);
	phy_meson_gxl_usb2_set_mode(&usb_phys[1], USB_DR_MODE_HOST);

	for (i = 0; i < 2; i++)
		usb_phys[i].dev = NULL;

	return 0;
}
#endif

// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Beniamino Galvani <b.galvani@gmail.com>
 * (C) Copyright 2018 Neil Armstrong <narmstrong@baylibre.com>
 */

#include <common.h>
#include <asm/arch/boot.h>
#include <asm/arch/eth.h>
#include <asm/arch/g12a.h>
#include <asm/arch/mem.h>
#include <asm/io.h>
#include <asm/armv8/mmu.h>
#include <linux/sizes.h>
#include <phy.h>

DECLARE_GLOBAL_DATA_PTR;

int meson_get_boot_device(void)
{
	return readl(G12A_AO_SEC_GP_CFG0) & G12A_AO_BOOT_DEVICE;
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
	reg = readl(G12A_AO_SEC_GP_CFG3);

	bl31_size = ((reg & G12A_AO_BL31_RSVMEM_SIZE_MASK)
			>> G12A_AO_BL31_RSVMEM_SIZE_SHIFT) * SZ_1K;
	bl32_size = (reg & G12A_AO_BL32_RSVMEM_SIZE_MASK) * SZ_1K;

	bl31_start = readl(G12A_AO_SEC_GP_CFG5);
	bl32_start = readl(G12A_AO_SEC_GP_CFG4);

	/* Add BL31 reserved zone */
	if (bl31_start && bl31_size)
		meson_board_add_reserved_memory(fdt, bl31_start, bl31_size);

	/* Add BL32 reserved zone */
	if (bl32_start && bl32_size)
		meson_board_add_reserved_memory(fdt, bl32_start, bl32_size);
}

phys_size_t get_effective_memsize(void)
{
	/* Size is reported in MiB, convert it in bytes */
	return ((readl(G12A_AO_SEC_GP_CFG0) & G12A_AO_MEM_SIZE_MASK)
			>> G12A_AO_MEM_SIZE_SHIFT) * SZ_1M;
}

static struct mm_region g12a_mem_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0xf0000000UL,
		.phys = 0xf0000000UL,
		.size = 0x10000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = g12a_mem_map;

static void g12a_enable_external_mdio(void)
{
	writel(0x0, ETH_PHY_CNTL2);
}

static void g12a_enable_internal_mdio(void)
{
	/* Fire up the PHY PLL */
	writel(0x29c0040a, ETH_PLL_CNTL0);
	writel(0x927e0000, ETH_PLL_CNTL1);
	writel(0xac5f49e5, ETH_PLL_CNTL2);
	writel(0x00000000, ETH_PLL_CNTL3);
	writel(0x00000000, ETH_PLL_CNTL4);
	writel(0x20200000, ETH_PLL_CNTL5);
	writel(0x0000c002, ETH_PLL_CNTL6);
	writel(0x00000023, ETH_PLL_CNTL7);
	writel(0x39c0040a, ETH_PLL_CNTL0);
	writel(0x19c0040a, ETH_PLL_CNTL0);

	/* Select the internal MDIO */
	writel(0x33000180, ETH_PHY_CNTL0);
	writel(0x00074043, ETH_PHY_CNTL1);
	writel(0x00000260, ETH_PHY_CNTL2);
}

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
		setbits_le32(G12A_ETH_REG_0, G12A_ETH_REG_0_PHY_INTF_RGMII |
			     G12A_ETH_REG_0_TX_PHASE(1) |
			     G12A_ETH_REG_0_TX_RATIO(4) |
			     G12A_ETH_REG_0_PHY_CLK_EN |
			     G12A_ETH_REG_0_CLK_EN);
		break;

	case PHY_INTERFACE_MODE_RMII:
		/* Set RMII mode */
		out_le32(G12A_ETH_REG_0, G12A_ETH_REG_0_PHY_INTF_RMII |
					G12A_ETH_REG_0_INVERT_RMII_CLK |
					G12A_ETH_REG_0_CLK_EN);

		/* Use G12A RMII Internal PHY */
		if (flags & MESON_USE_INTERNAL_RMII_PHY)
			g12a_enable_internal_mdio();
		else
			g12a_enable_external_mdio();

		break;

	default:
		printf("Invalid Ethernet interface mode\n");
		return;
	}

	/* Enable power gate */
	clrbits_le32(G12A_MEM_PD_REG_0, G12A_MEM_PD_REG_0_ETH_MASK);
}

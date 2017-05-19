/*
 * CPSW common - libs used across TI ethernet devices.
 *
 * Copyright (C) 2016, Texas Instruments, Incorporated
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <fdt_support.h>
#include <asm/io.h>
#include <cpsw.h>

DECLARE_GLOBAL_DATA_PTR;

#define CTRL_MAC_REG(offset, id) ((offset) + 0x8 * (id))

static int davinci_emac_3517_get_macid(struct udevice *dev, u16 offset,
				       int slave, u8 *mac_addr)
{
	void *fdt = (void *)gd->fdt_blob;
	int node = dev_of_offset(dev);
	u32 macid_lsb;
	u32 macid_msb;
	fdt32_t gmii = 0;
	int syscon;
	u32 addr;

	syscon = fdtdec_lookup_phandle(fdt, node, "syscon");
	if (syscon < 0) {
		error("Syscon offset not found\n");
		return -ENOENT;
	}

	addr = (u32)map_physmem(fdt_translate_address(fdt, syscon, &gmii),
				sizeof(u32), MAP_NOCACHE);
	if (addr == FDT_ADDR_T_NONE) {
		error("Not able to get syscon address to get mac efuse address\n");
		return -ENOENT;
	}

	addr += CTRL_MAC_REG(offset, slave);

	/* try reading mac address from efuse */
	macid_lsb = readl(addr);
	macid_msb = readl(addr + 4);

	mac_addr[0] = (macid_msb >> 16) & 0xff;
	mac_addr[1] = (macid_msb >> 8)  & 0xff;
	mac_addr[2] = macid_msb & 0xff;
	mac_addr[3] = (macid_lsb >> 16) & 0xff;
	mac_addr[4] = (macid_lsb >> 8)  & 0xff;
	mac_addr[5] = macid_lsb & 0xff;

	return 0;
}

static int cpsw_am33xx_cm_get_macid(struct udevice *dev, u16 offset, int slave,
				    u8 *mac_addr)
{
	void *fdt = (void *)gd->fdt_blob;
	int node = dev_of_offset(dev);
	u32 macid_lo;
	u32 macid_hi;
	fdt32_t gmii = 0;
	int syscon;
	u32 addr;

	syscon = fdtdec_lookup_phandle(fdt, node, "syscon");
	if (syscon < 0) {
		error("Syscon offset not found\n");
		return -ENOENT;
	}

	addr = (u32)map_physmem(fdt_translate_address(fdt, syscon, &gmii),
				sizeof(u32), MAP_NOCACHE);
	if (addr == FDT_ADDR_T_NONE) {
		error("Not able to get syscon address to get mac efuse address\n");
		return -ENOENT;
	}

	addr += CTRL_MAC_REG(offset, slave);

	/* try reading mac address from efuse */
	macid_lo = readl(addr);
	macid_hi = readl(addr + 4);

	mac_addr[5] = (macid_lo >> 8) & 0xff;
	mac_addr[4] = macid_lo & 0xff;
	mac_addr[3] = (macid_hi >> 24) & 0xff;
	mac_addr[2] = (macid_hi >> 16) & 0xff;
	mac_addr[1] = (macid_hi >> 8) & 0xff;
	mac_addr[0] = macid_hi & 0xff;

	return 0;
}

int ti_cm_get_macid(struct udevice *dev, int slave, u8 *mac_addr)
{
	if (of_machine_is_compatible("ti,dm8148"))
		return cpsw_am33xx_cm_get_macid(dev, 0x630, slave, mac_addr);

	if (of_machine_is_compatible("ti,am33xx"))
		return cpsw_am33xx_cm_get_macid(dev, 0x630, slave, mac_addr);

	if (device_is_compatible(dev, "ti,am3517-emac"))
		return davinci_emac_3517_get_macid(dev, 0x110, slave, mac_addr);

	if (device_is_compatible(dev, "ti,dm816-emac"))
		return cpsw_am33xx_cm_get_macid(dev, 0x30, slave, mac_addr);

	if (of_machine_is_compatible("ti,am43"))
		return cpsw_am33xx_cm_get_macid(dev, 0x630, slave, mac_addr);

	if (of_machine_is_compatible("ti,dra7"))
		return davinci_emac_3517_get_macid(dev, 0x514, slave, mac_addr);

	dev_err(dev, "incompatible machine/device type for reading mac address\n");
	return -ENOENT;
}

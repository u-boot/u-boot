/*
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <pci.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/post.h>
#include <asm/arch/device.h>
#include <asm/arch/tnc.h>
#include <asm/fsp/fsp_support.h>
#include <asm/processor.h>

static int __maybe_unused disable_igd(void)
{
	struct udevice *igd, *sdvo;
	int ret;

	ret = dm_pci_bus_find_bdf(TNC_IGD, &igd);
	if (ret)
		return ret;
	if (!igd)
		return 0;

	ret = dm_pci_bus_find_bdf(TNC_SDVO, &sdvo);
	if (ret)
		return ret;
	if (!sdvo)
		return 0;

	/*
	 * According to Atom E6xx datasheet, setting VGA Disable (bit17)
	 * of Graphics Controller register (offset 0x50) prevents IGD
	 * (D2:F0) from reporting itself as a VGA display controller
	 * class in the PCI configuration space, and should also prevent
	 * it from responding to VGA legacy memory range and I/O addresses.
	 *
	 * However test result shows that with just VGA Disable bit set and
	 * a PCIe graphics card connected to one of the PCIe controllers on
	 * the E6xx, accessing the VGA legacy space still causes system hang.
	 * After a number of attempts, it turns out besides VGA Disable bit,
	 * the SDVO (D3:F0) device should be disabled to make it work.
	 *
	 * To simplify, use the Function Disable register (offset 0xc4)
	 * to disable both IGD (D2:F0) and SDVO (D3:F0) devices. Now these
	 * two devices will be completely disabled (invisible in the PCI
	 * configuration space) unless a system reset is performed.
	 */
	dm_pci_write_config32(igd, IGD_FD, FUNC_DISABLE);
	dm_pci_write_config32(sdvo, IGD_FD, FUNC_DISABLE);

	return 0;
}

int arch_cpu_init(void)
{
	int ret;

	post_code(POST_CPU_INIT);

	ret = x86_cpu_init_f();
	if (ret)
		return ret;

	return 0;
}

int arch_early_init_r(void)
{
	int ret = 0;

#ifdef CONFIG_DISABLE_IGD
	ret = disable_igd();
#endif

	return ret;
}

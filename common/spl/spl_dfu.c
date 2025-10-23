// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016
 * Texas Instruments, <www.ti.com>
 *
 * Ravi B <ravibabu@ti.com>
 */
#include <env.h>
#include <spl.h>
#include <linux/compiler.h>
#include <errno.h>
#include <watchdog.h>
#include <console.h>
#include <g_dnl.h>
#include <usb.h>
#include <dfu.h>
#include <linux/printk.h>
#include <pci_ep.h>
#include <dm/uclass.h>
#include <cpu_func.h>
#include <linux/io.h>

/*
 * Macros define size of magic word and boot phase string
 * in bytes.
 */
#define MAGIC_WORD_SIZE 4
#define BOOT_PHASE_STRING_SIZE 63

static int run_dfu(int usb_index, char *interface, char *devstring)
{
	int ret;

	ret = dfu_init_env_entities(interface, devstring);
	if (ret) {
		dfu_free_entities();
		goto exit;
	}

	run_usb_dnl_gadget(usb_index, "usb_dnl_dfu");
exit:
	dfu_free_entities();
	return ret;
}

#ifdef CONFIG_SPL_PCI_DFU
static int dfu_over_pcie(void)
{
	u32 offset, magic_word;
	volatile void *addr;
	struct udevice *dev;
	struct pci_bar bar;
	struct pci_ep_header hdr;
	uint fn = 0;
	int ret;
	char *bootphase;

	uclass_get_device_by_seq(UCLASS_PCI_EP, 0, &dev);
	if (!dev) {
		pr_err("Failed to get pci ep device\n");
		return -ENODEV;
	}

	hdr.deviceid = CONFIG_SPL_PCI_DFU_DEVICE_ID;
	hdr.vendorid = CONFIG_SPL_PCI_DFU_VENDOR_ID;
	hdr.baseclass_code = PCI_BASE_CLASS_MEMORY;
	hdr.subclass_code = PCI_CLASS_MEMORY_RAM;

	ret = pci_ep_write_header(dev, fn, &hdr);
	if (ret) {
		pr_err("Failed to write header: %d\n", ret);
		return ret;
	}

	bar.barno = BAR_0;
	bar.phys_addr = (dma_addr_t)CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
	bar.flags = PCI_BASE_ADDRESS_SPACE_MEMORY |
			  PCI_BASE_ADDRESS_MEM_TYPE_32 |
			  PCI_BASE_ADDRESS_MEM_PREFETCH;

	bar.size = CONFIG_SPL_PCI_DFU_BAR_SIZE;

	ret = pci_ep_set_bar(dev, fn, &bar);
	if (ret) {
		pr_err("Failed to set bar: %d\n", ret);
		return ret;
	}

	ret = pci_ep_start(dev);
	if (ret) {
		pr_err("Failed to start ep: %d\n", ret);
		return ret;
	}

	addr = (void *)CONFIG_SPL_PCI_DFU_SPL_LOAD_FIT_ADDRESS;
	offset = CONFIG_SPL_PCI_DFU_BAR_SIZE - MAGIC_WORD_SIZE;

	if (sizeof(CONFIG_SPL_PCI_DFU_BOOT_PHASE) > BOOT_PHASE_STRING_SIZE) {
		pr_err("Not copying boot phase. String too long\n");
	} else {
		bootphase = (char *)(addr + CONFIG_SPL_PCI_DFU_BAR_SIZE -
				  (BOOT_PHASE_STRING_SIZE + MAGIC_WORD_SIZE + 1));
		strlcpy(bootphase, CONFIG_SPL_PCI_DFU_BOOT_PHASE,
			sizeof(CONFIG_SPL_PCI_DFU_BOOT_PHASE) + 1);
	}

	addr = addr + offset;
	magic_word = CONFIG_SPL_PCI_DFU_MAGIC_WORD;
	(*(int *)addr) = 0;
	flush_dcache_all();
	for (;;) {
		if (*(int *)addr == magic_word)
			break;
		invalidate_dcache_all();
	}

	return 0;
}
#endif

int spl_dfu_cmd(int usbctrl, char *dfu_alt_info, char *interface, char *devstr)
{
	char *str_env;
	int ret;

#ifdef CONFIG_SPL_PCI_DFU
	if (spl_boot_device() == BOOT_DEVICE_PCIE)
		return dfu_over_pcie();
#endif

	/* set default environment */
	env_set_default(NULL, 0);
	str_env = env_get(dfu_alt_info);
	if (!str_env) {
		pr_err("\"%s\" env variable not defined!\n", dfu_alt_info);
		return -EINVAL;
	}

	ret = env_set("dfu_alt_info", str_env);
	if (ret) {
		pr_err("unable to set env variable \"dfu_alt_info\"!\n");
		return -EINVAL;
	}

	/* invoke dfu command */
	return run_dfu(usbctrl, interface, devstr);
}

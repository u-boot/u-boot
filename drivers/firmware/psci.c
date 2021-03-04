// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * Based on drivers/firmware/psci.c from Linux:
 * Copyright (C) 2015 ARM Limited
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <irq_func.h>
#include <log.h>
#include <dm/lists.h>
#include <efi_loader.h>
#include <sysreset.h>
#include <linux/delay.h>
#include <linux/libfdt.h>
#include <linux/arm-smccc.h>
#include <linux/errno.h>
#include <linux/printk.h>
#include <linux/psci.h>
#include <asm/system.h>

#define DRIVER_NAME "psci"

#define PSCI_METHOD_HVC 1
#define PSCI_METHOD_SMC 2

/*
 * While a 64-bit OS can make calls with SMC32 calling conventions, for some
 * calls it is necessary to use SMC64 to pass or return 64-bit values.
 * For such calls PSCI_FN_NATIVE(version, name) will choose the appropriate
 * (native-width) function ID.
 */
#if defined(CONFIG_ARM64)
#define PSCI_FN_NATIVE(version, name)	PSCI_##version##_FN64_##name
#else
#define PSCI_FN_NATIVE(version, name)	PSCI_##version##_FN_##name
#endif

#if CONFIG_IS_ENABLED(EFI_LOADER)
int __efi_runtime_data psci_method;
#else
int psci_method __section(".data");
#endif

unsigned long __efi_runtime invoke_psci_fn
		(unsigned long function_id, unsigned long arg0,
		 unsigned long arg1, unsigned long arg2)
{
	struct arm_smccc_res res;

	/*
	 * In the __efi_runtime we need to avoid the switch statement. In some
	 * cases the compiler creates lookup tables to implement switch. These
	 * tables are not correctly relocated when SetVirtualAddressMap is
	 * called.
	 */
	if (psci_method == PSCI_METHOD_SMC)
		arm_smccc_smc(function_id, arg0, arg1, arg2, 0, 0, 0, 0, &res);
	else if (psci_method == PSCI_METHOD_HVC)
		arm_smccc_hvc(function_id, arg0, arg1, arg2, 0, 0, 0, 0, &res);
	else
		res.a0 = PSCI_RET_DISABLED;
	return res.a0;
}

static int request_psci_features(u32 psci_func_id)
{
	return invoke_psci_fn(PSCI_1_0_FN_PSCI_FEATURES,
			      psci_func_id, 0, 0);
}

static u32 psci_0_2_get_version(void)
{
	return invoke_psci_fn(PSCI_0_2_FN_PSCI_VERSION, 0, 0, 0);
}

static bool psci_is_system_reset2_supported(void)
{
	int ret;
	u32 ver;

	ver = psci_0_2_get_version();

	if (PSCI_VERSION_MAJOR(ver) >= 1) {
		ret = request_psci_features(PSCI_FN_NATIVE(1_1,
							   SYSTEM_RESET2));

		if (ret != PSCI_RET_NOT_SUPPORTED)
			return true;
	}

	return false;
}

static int psci_bind(struct udevice *dev)
{
	/* No SYSTEM_RESET support for PSCI 0.1 */
	if (device_is_compatible(dev, "arm,psci-0.2") ||
	    device_is_compatible(dev, "arm,psci-1.0")) {
		int ret;

		/* bind psci-sysreset optionally */
		ret = device_bind_driver(dev, "psci-sysreset", "psci-sysreset",
					 NULL);
		if (ret)
			pr_debug("PSCI System Reset was not bound.\n");
	}

	return 0;
}

static int psci_probe(struct udevice *dev)
{
	const char *method;

#if defined(CONFIG_ARM64)
	if (current_el() == 3)
		return -EINVAL;
#endif

	method = ofnode_read_string(dev_ofnode(dev), "method");
	if (!method) {
		pr_warn("missing \"method\" property\n");
		return -ENXIO;
	}

	if (!strcmp("hvc", method)) {
		psci_method = PSCI_METHOD_HVC;
	} else if (!strcmp("smc", method)) {
		psci_method = PSCI_METHOD_SMC;
	} else {
		pr_warn("invalid \"method\" property: %s\n", method);
		return -EINVAL;
	}

	return 0;
}

/**
 * void do_psci_probe() - probe PSCI firmware driver
 *
 * Ensure that psci_method is initialized.
 */
static void __maybe_unused do_psci_probe(void)
{
	struct udevice *dev;

	uclass_get_device_by_name(UCLASS_FIRMWARE, DRIVER_NAME, &dev);
}

#if IS_ENABLED(CONFIG_EFI_LOADER) && IS_ENABLED(CONFIG_PSCI_RESET)
efi_status_t efi_reset_system_init(void)
{
	do_psci_probe();
	return EFI_SUCCESS;
}

void __efi_runtime EFIAPI efi_reset_system(enum efi_reset_type reset_type,
					   efi_status_t reset_status,
					   unsigned long data_size,
					   void *reset_data)
{
	if (reset_type == EFI_RESET_COLD ||
	    reset_type == EFI_RESET_WARM ||
	    reset_type == EFI_RESET_PLATFORM_SPECIFIC) {
		invoke_psci_fn(PSCI_0_2_FN_SYSTEM_RESET, 0, 0, 0);
	} else if (reset_type == EFI_RESET_SHUTDOWN) {
		invoke_psci_fn(PSCI_0_2_FN_SYSTEM_OFF, 0, 0, 0);
	}
	while (1)
		;
}
#endif /* IS_ENABLED(CONFIG_EFI_LOADER) && IS_ENABLED(CONFIG_PSCI_RESET) */

#ifdef CONFIG_PSCI_RESET
void reset_misc(void)
{
	do_psci_probe();
	invoke_psci_fn(PSCI_0_2_FN_SYSTEM_RESET, 0, 0, 0);
}
#endif /* CONFIG_PSCI_RESET */

void psci_sys_reset(u32 type)
{
	bool reset2_supported;

	do_psci_probe();

	reset2_supported = psci_is_system_reset2_supported();

	if (type == SYSRESET_WARM && reset2_supported) {
		/*
		 * reset_type[31] = 0 (architectural)
		 * reset_type[30:0] = 0 (SYSTEM_WARM_RESET)
		 * cookie = 0 (ignored by the implementation)
		 */
		invoke_psci_fn(PSCI_FN_NATIVE(1_1, SYSTEM_RESET2), 0, 0, 0);
	} else {
		invoke_psci_fn(PSCI_0_2_FN_SYSTEM_RESET, 0, 0, 0);
	}
}

void psci_sys_poweroff(void)
{
	do_psci_probe();

	invoke_psci_fn(PSCI_0_2_FN_SYSTEM_OFF, 0, 0, 0);
}

#ifdef CONFIG_CMD_POWEROFF
int do_poweroff(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	do_psci_probe();

	puts("poweroff ...\n");
	udelay(50000); /* wait 50 ms */

	disable_interrupts();
	invoke_psci_fn(PSCI_0_2_FN_SYSTEM_OFF, 0, 0, 0);
	enable_interrupts();

	log_err("Power off not supported on this platform\n");
	return CMD_RET_FAILURE;
}
#endif

static const struct udevice_id psci_of_match[] = {
	{ .compatible = "arm,psci" },
	{ .compatible = "arm,psci-0.2" },
	{ .compatible = "arm,psci-1.0" },
	{},
};

U_BOOT_DRIVER(psci) = {
	.name = DRIVER_NAME,
	.id = UCLASS_FIRMWARE,
	.of_match = psci_of_match,
	.bind = psci_bind,
	.probe = psci_probe,
};

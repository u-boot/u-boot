// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 Google LLC
 */

#include <common.h>
#include <bloblist.h>
#include <command.h>
#include <dm.h>
#include <log.h>
#include <acpi/acpigen.h>
#include <asm-generic/gpio.h>
#include <asm/acpi_nhlt.h>
#include <asm/intel_gnvs.h>
#include <asm/intel_pinctrl.h>
#include <dm/acpi.h>
#include "variant_gpio.h"

struct cros_gpio_info {
	const char *linux_name;
	enum cros_gpio_t type;
	int gpio_num;
	int flags;
};

int arch_misc_init(void)
{
	return 0;
}

/* This function is needed if CONFIG_CMDLINE is not enabled */
int board_run_command(const char *cmdline)
{
	printf("No command line\n");

	return 0;
}

int chromeos_get_gpio(const struct udevice *dev, const char *prop,
		      enum cros_gpio_t type, struct cros_gpio_info *info)
{
	struct udevice *pinctrl;
	struct gpio_desc desc;
	int ret;

	ret = gpio_request_by_name((struct udevice *)dev, prop, 0, &desc, 0);
	if (ret == -ENOTBLK)
		info->gpio_num = CROS_GPIO_VIRTUAL;
	else if (ret)
		return log_msg_ret("gpio", ret);
	else
		info->gpio_num = desc.offset;
	info->linux_name = dev_read_string(desc.dev, "linux-name");
	if (!info->linux_name)
		return log_msg_ret("linux-name", -ENOENT);
	info->type = type;
	/* Get ACPI pin from GPIO library if available */
	if (info->gpio_num != CROS_GPIO_VIRTUAL) {
		pinctrl = dev_get_parent(desc.dev);
		info->gpio_num = intel_pinctrl_get_acpi_pin(pinctrl,
							    info->gpio_num);
	}
	info->flags = desc.flags & GPIOD_ACTIVE_LOW ? CROS_GPIO_ACTIVE_LOW :
		CROS_GPIO_ACTIVE_HIGH;

	return 0;
}

static int chromeos_acpi_gpio_generate(const struct udevice *dev,
				       struct acpi_ctx *ctx)
{
	struct cros_gpio_info info[3];
	int count, i;
	int ret;

	count = 3;
	ret = chromeos_get_gpio(dev, "recovery-gpios", CROS_GPIO_REC, &info[0]);
	if (ret)
		return log_msg_ret("rec", ret);
	ret = chromeos_get_gpio(dev, "write-protect-gpios", CROS_GPIO_WP,
				&info[1]);
	if (ret)
		return log_msg_ret("rec", ret);
	ret = chromeos_get_gpio(dev, "phase-enforce-gpios", CROS_GPIO_PE,
				&info[2]);
	if (ret)
		return log_msg_ret("rec", ret);
	acpigen_write_scope(ctx, "\\");
	acpigen_write_name(ctx, "OIPG");
	acpigen_write_package(ctx, count);
	for (i = 0; i < count; i++) {
		acpigen_write_package(ctx, 4);
		acpigen_write_integer(ctx, info[i].type);
		acpigen_write_integer(ctx, info[i].flags);
		acpigen_write_integer(ctx, info[i].gpio_num);
		acpigen_write_string(ctx, info[i].linux_name);
		acpigen_pop_len(ctx);
	}

	acpigen_pop_len(ctx);
	acpigen_pop_len(ctx);

	return 0;
}

static int coral_write_acpi_tables(const struct udevice *dev,
				   struct acpi_ctx *ctx)
{
	struct acpi_global_nvs *gnvs;
	struct nhlt *nhlt;
	const char *oem_id = "coral";
	const char *oem_table_id = "coral";
	u32 oem_revision = 3;
	int ret;

	gnvs = bloblist_find(BLOBLISTT_ACPI_GNVS, sizeof(*gnvs));
	if (!gnvs)
		return log_msg_ret("bloblist", -ENOENT);

	nhlt = nhlt_init();
	if (!nhlt)
		return -ENOMEM;

	log_debug("Setting up NHLT\n");
	ret = acpi_setup_nhlt(ctx, nhlt);
	if (ret)
		return log_msg_ret("setup", ret);

	/* Update NHLT GNVS Data */
	gnvs->nhla = (uintptr_t)ctx->current;
	gnvs->nhll = nhlt_current_size(nhlt);

	ret = nhlt_serialise_oem_overrides(ctx, nhlt, oem_id, oem_table_id,
					   oem_revision);
	if (ret)
		return log_msg_ret("serialise", ret);

	return 0;
}

struct acpi_ops coral_acpi_ops = {
	.write_tables	= coral_write_acpi_tables,
	.inject_dsdt	= chromeos_acpi_gpio_generate,
};

static const struct udevice_id coral_ids[] = {
	{ .compatible = "google,coral" },
	{ }
};

U_BOOT_DRIVER(coral_drv) = {
	.name		= "coral",
	.id		= UCLASS_SYSINFO,
	.of_match	= coral_ids,
	ACPI_OPS_PTR(&coral_acpi_ops)
};

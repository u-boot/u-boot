// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019 Google LLC
 */

#include <common.h>
#include <cpu.h>
#include <dm.h>
#include <log.h>
#include <acpi/acpigen.h>
#include <acpi/acpi_table.h>
#include <asm/cpu_common.h>
#include <asm/cpu_x86.h>
#include <asm/intel_acpi.h>
#include <asm/msr.h>
#include <dm/acpi.h>

#define CSTATE_RES(address_space, width, offset, address)		\
	{								\
	.space_id = address_space,					\
	.bit_width = width,						\
	.bit_offset = offset,						\
	.addrl = address,						\
	}

static struct acpi_cstate cstate_map[] = {
	{
		/* C1 */
		.ctype = 1,		/* ACPI C1 */
		.latency = 1,
		.power = 1000,
		.resource = {
			.space_id = ACPI_ADDRESS_SPACE_FIXED,
		},
	}, {
		.ctype = 2,		/* ACPI C2 */
		.latency = 50,
		.power = 10,
		.resource = {
			.space_id = ACPI_ADDRESS_SPACE_IO,
			.bit_width = 8,
			.addrl = 0x415,
		},
	}, {
		.ctype = 3,		/* ACPI C3 */
		.latency = 150,
		.power = 10,
		.resource = {
			.space_id = ACPI_ADDRESS_SPACE_IO,
			.bit_width = 8,
			.addrl = 0x419,
		},
	},
};

static int apl_get_info(const struct udevice *dev, struct cpu_info *info)
{
	return cpu_intel_get_info(info, INTEL_BCLK_MHZ);
}

static int acpi_cpu_fill_ssdt(const struct udevice *dev, struct acpi_ctx *ctx)
{
	uint core_id = dev->req_seq;
	int cores_per_package;
	int ret;

	cores_per_package = cpu_get_cores_per_package();
	ret = acpi_generate_cpu_header(ctx, core_id, cstate_map,
				       ARRAY_SIZE(cstate_map));

	/* Generate P-state tables */
	generate_p_state_entries(ctx, core_id, cores_per_package);

	/* Generate T-state tables */
	generate_t_state_entries(ctx, core_id, cores_per_package, NULL, 0);

	acpigen_pop_len(ctx);

	if (device_is_last_sibling(dev)) {
		ret = acpi_generate_cpu_package_final(ctx, cores_per_package);

		if (ret)
			return ret;
	}

	return 0;
}

struct acpi_ops apl_cpu_acpi_ops = {
	.fill_ssdt	= acpi_cpu_fill_ssdt,
};

static const struct cpu_ops cpu_x86_apl_ops = {
	.get_desc	= cpu_x86_get_desc,
	.get_info	= apl_get_info,
	.get_count	= cpu_x86_get_count,
	.get_vendor	= cpu_x86_get_vendor,
};

static const struct udevice_id cpu_x86_apl_ids[] = {
	{ .compatible = "intel,apl-cpu" },
	{ }
};

U_BOOT_DRIVER(intel_apl_cpu) = {
	.name		= "intel_apl_cpu",
	.id		= UCLASS_CPU,
	.of_match	= cpu_x86_apl_ids,
	.bind		= cpu_x86_bind,
	.ops		= &cpu_x86_apl_ops,
	ACPI_OPS_PTR(&apl_cpu_acpi_ops)
	.flags		= DM_FLAG_PRE_RELOC,
};

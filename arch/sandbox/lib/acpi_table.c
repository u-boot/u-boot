// SPDX-License-Identifier: GPL-2.0+
#include <acpi/acpi_table.h>

void acpi_fill_fadt(struct acpi_fadt *fadt)
{
}

void *acpi_fill_madt(struct acpi_madt *madt, struct acpi_ctx *ctx)
{
	return ctx->current;
}

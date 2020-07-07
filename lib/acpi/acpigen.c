// SPDX-License-Identifier: GPL-2.0
/*
 * Generation of ACPI (Advanced Configuration and Power Interface) tables
 *
 * Copyright 2019 Google LLC
 * Mostly taken from coreboot
 */

#define LOG_CATEGORY LOGC_ACPI

#include <common.h>
#include <dm.h>
#include <acpi/acpigen.h>
#include <dm/acpi.h>

u8 *acpigen_get_current(struct acpi_ctx *ctx)
{
	return ctx->current;
}

void acpigen_emit_byte(struct acpi_ctx *ctx, uint data)
{
	*(u8 *)ctx->current++ = data;
}

void acpigen_emit_word(struct acpi_ctx *ctx, uint data)
{
	acpigen_emit_byte(ctx, data & 0xff);
	acpigen_emit_byte(ctx, (data >> 8) & 0xff);
}

void acpigen_emit_dword(struct acpi_ctx *ctx, uint data)
{
	/* Output the value in little-endian format */
	acpigen_emit_byte(ctx, data & 0xff);
	acpigen_emit_byte(ctx, (data >> 8) & 0xff);
	acpigen_emit_byte(ctx, (data >> 16) & 0xff);
	acpigen_emit_byte(ctx, (data >> 24) & 0xff);
}

void acpigen_emit_stream(struct acpi_ctx *ctx, const char *data, int size)
{
	int i;

	for (i = 0; i < size; i++)
		acpigen_emit_byte(ctx, data[i]);
}

void acpigen_emit_string(struct acpi_ctx *ctx, const char *str)
{
	acpigen_emit_stream(ctx, str, str ? strlen(str) : 0);
	acpigen_emit_byte(ctx, '\0');
}

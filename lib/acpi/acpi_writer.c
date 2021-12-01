// SPDX-License-Identifier: GPL-2.0+
/*
 * Handles writing the declared ACPI tables
 *
 * Copyright 2021 Google LLC
 */

#define LOG_CATEGORY LOGC_ACPI

#include <common.h>
#include <log.h>
#include <malloc.h>
#include <mapmem.h>
#include <acpi/acpi_table.h>
#include <asm/global_data.h>
#include <dm/acpi.h>

DECLARE_GLOBAL_DATA_PTR;

int acpi_write_one(struct acpi_ctx *ctx, const struct acpi_writer *entry)
{
	int ret;

	log_debug("%s: writing table '%s'\n", entry->name,
		  entry->table);
	ctx->tab_start = ctx->current;
	ret = entry->h_write(ctx, entry);
	if (ret == -ENOENT) {
		log_debug("%s: Omitted due to being empty\n",
			  entry->name);
		ret = 0;
		ctx->current = ctx->tab_start;	/* drop the table */
		return ret;
	}
	if (ret)
		return log_msg_ret("write", ret);

	acpi_align(ctx);

	return 0;
}

static int acpi_write_all(struct acpi_ctx *ctx)
{
	const struct acpi_writer *writer =
		 ll_entry_start(struct acpi_writer, acpi_writer);
	const int n_ents = ll_entry_count(struct acpi_writer, acpi_writer);
	const struct acpi_writer *entry;
	int ret;

	for (entry = writer; entry != writer + n_ents; entry++) {
		ret = acpi_write_one(ctx, entry);
		if (ret && ret != -ENOENT)
			return log_msg_ret("one", ret);
	}

	return 0;
}

/*
 * QEMU's version of write_acpi_tables is defined in drivers/misc/qfw.c
 */
ulong write_acpi_tables(ulong start_addr)
{
	struct acpi_ctx *ctx;
	ulong addr;
	void *start;
	int ret;

	ctx = calloc(1, sizeof(*ctx));
	if (!ctx)
		return log_msg_ret("mem", -ENOMEM);
	gd->acpi_ctx = ctx;

	start = map_sysmem(start_addr, 0);

	log_debug("ACPI: Writing ACPI tables at %lx\n", start_addr);

	acpi_reset_items();

	ret = acpi_write_all(ctx);
	if (ret) {
		log_err("Failed to write ACPI tables (err=%d)\n", ret);
		return log_msg_ret("write", -ENOMEM);
	}

	addr = map_to_sysmem(ctx->current);
	log_debug("ACPI current = %lx\n", addr);

	return addr;
}

// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2019 Stephan Gerhold <stephan@gerhold.net>
 */
#include <common.h>
#include <env.h>
#include <init.h>
#include <log.h>
#include <stdlib.h>
#include <asm/global_data.h>
#include <asm/setup.h>
#include <asm/system.h>

DECLARE_GLOBAL_DATA_PTR;

/* Parse atags provided by Samsung bootloader to get available memory */
static ulong fw_mach __section(".data");
static ulong fw_atags __section(".data");

static const struct tag *fw_atags_copy;
static uint fw_atags_size;

void save_boot_params(ulong r0, ulong r1, ulong r2, ulong r3)
{
	fw_mach = r1;
	fw_atags = r2;
	save_boot_params_ret();
}

static const struct tag *fw_atags_get(void)
{
	const struct tag *tags = (const struct tag *)fw_atags;

	if (tags->hdr.tag != ATAG_CORE) {
		log_err("Invalid atags: tag 0x%x at %p\n", tags->hdr.tag, tags);
		return NULL;
	}

	return tags;
}

int dram_init(void)
{
	const struct tag *t, *tags = fw_atags_get();

	if (!tags)
		return -EINVAL;

	for_each_tag(t, tags) {
		if (t->hdr.tag != ATAG_MEM)
			continue;

		debug("Memory: %#x-%#x (size %#x)\n", t->u.mem.start,
		      t->u.mem.start + t->u.mem.size, t->u.mem.size);
		gd->ram_size += t->u.mem.size;
	}
	return 0;
}

int dram_init_banksize(void)
{
	const struct tag *t, *tags = fw_atags_get();
	unsigned int bank = 0;

	if (!tags)
		return -EINVAL;

	for_each_tag(t, tags) {
		if (t->hdr.tag != ATAG_MEM)
			continue;

		gd->bd->bi_dram[bank].start = t->u.mem.start;
		gd->bd->bi_dram[bank].size = t->u.mem.size;
		if (++bank == CONFIG_NR_DRAM_BANKS)
			break;
	}
	return 0;
}

int board_init(void)
{
	gd->bd->bi_arch_number = fw_mach;
	gd->bd->bi_boot_params = fw_atags;
	return 0;
}

static void parse_serial(const struct tag_serialnr *serialnr)
{
	char serial[17];

	if (env_get("serial#"))
		return;

	sprintf(serial, "%08x%08x", serialnr->high, serialnr->low);
	env_set("serial#", serial);
}

/*
 * The downstream/vendor kernel (provided by Samsung) uses ATAGS for booting.
 * It also requires an extremely long cmdline provided by the primary bootloader
 * that is not suitable for booting mainline.
 *
 * Since downstream is the only user of ATAGS, we emulate the behavior of the
 * Samsung bootloader by generating only the initrd atag in U-Boot, and copying
 * all other ATAGS as-is from the primary bootloader.
 */
static inline bool skip_atag(u32 tag)
{
	return (tag == ATAG_NONE || tag == ATAG_CORE ||
		tag == ATAG_INITRD || tag == ATAG_INITRD2);
}

static void copy_atags(const struct tag *tags)
{
	const struct tag *t;
	struct tag *copy;

	if (!tags)
		return;

	/* Calculate necessary size for tags we want to copy */
	for_each_tag(t, tags) {
		if (skip_atag(t->hdr.tag))
			continue;

		if (t->hdr.tag == ATAG_SERIAL)
			parse_serial(&t->u.serialnr);

		fw_atags_size += t->hdr.size * sizeof(u32);
	}

	if (!fw_atags_size)
		return;  /* No tags to copy */

	copy = malloc(fw_atags_size);
	if (!copy)
		return;
	fw_atags_copy = copy;

	/* Copy tags */
	for_each_tag(t, tags) {
		if (skip_atag(t->hdr.tag))
			continue;

		memcpy(copy, t, t->hdr.size * sizeof(u32));
		copy = tag_next(copy);
	}
}

int misc_init_r(void)
{
	copy_atags(fw_atags_get());
	return 0;
}

void setup_board_tags(struct tag **in_params)
{
	if (!fw_atags_copy)
		return;

	/*
	 * fw_atags_copy contains only full "struct tag" (plus data)
	 * so copying it bytewise here should be fine.
	 */
	memcpy(*in_params, fw_atags_copy, fw_atags_size);
	*(u8 **)in_params += fw_atags_size;
}

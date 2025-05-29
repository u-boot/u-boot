// SPDX-License-Identifier: GPL-2.0+
/*
 * UPL handoff testing
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <abuf.h>
#include <env.h>
#include <mapmem.h>
#include <upl.h>
#include <dm/ofnode.h>
#include <test/test.h>
#include <test/ut.h>
#include "bootstd_common.h"

/* Declare a new upl test */
#define UPL_TEST(_name, _flags)	UNIT_TEST(_name, _flags, upl)

static int add_region(struct unit_test_state *uts, struct alist *lst,
		      ulong base, ulong size)
{
	struct memregion region;

	region.base = base;
	region.size = size;
	ut_assertnonnull(alist_add(lst, region));

	return 0;
}

int upl_get_test_data(struct unit_test_state *uts, struct upl *upl)
{
	struct upl_memmap memmap;
	struct upl_memres memres;
	struct upl_image img;
	struct upl_mem mem;

	upl_init(upl);

	upl->addr_cells = 1;
	upl->size_cells = 1;
	upl->smbios = 0x123;
	upl->acpi = 0x456;
	upl->bootmode = BIT(UPLBM_DEFAULT) | BIT(UPLBM_S3);
	upl->fit = 0x789;
	upl->conf_offset = 0x234;
	upl->addr_width = 46;
	upl->acpi_nvs_size = 0x100;

	/* image[0] */
	img.load = 0x1;
	img.size = 0x2;
	img.offset = 0x3;
	img.description = "U-Boot";
	ut_assertnonnull(alist_add(&upl->image, img));

	/* image[1] */
	img.load = 0x4;
	img.size = 0x5;
	img.offset = 0x6;
	img.description = "ATF";
	ut_assertnonnull(alist_add(&upl->image, img));

	/* mem[0] : 3 regions */
	memset(&mem, '\0', sizeof(mem));
	alist_init_struct(&mem.region, struct memregion);
	ut_assertok(add_region(uts, &mem.region, 0x10, 0x20));
	ut_assertok(add_region(uts, &mem.region, 0x30, 0x40));
	ut_assertok(add_region(uts, &mem.region, 0x40, 0x50));
	ut_assertnonnull(alist_add(&upl->mem, mem));

	/* mem[0] : 1 region */
	alist_init_struct(&mem.region, struct memregion);
	ut_assertok(add_region(uts, &mem.region, 0x70, 0x80));
	mem.hotpluggable = true;
	ut_assertnonnull(alist_add(&upl->mem, mem));
	mem.hotpluggable = false;

	/* memmap[0] : 5 regions */
	alist_init_struct(&memmap.region, struct memregion);
	memmap.name = "acpi";
	memmap.usage = BIT(UPLUS_ACPI_RECLAIM);
	ut_assertok(add_region(uts, &memmap.region, 0x11, 0x12));
	ut_assertok(add_region(uts, &memmap.region, 0x13, 0x14));
	ut_assertok(add_region(uts, &memmap.region, 0x15, 0x16));
	ut_assertok(add_region(uts, &memmap.region, 0x17, 0x18));
	ut_assertok(add_region(uts, &memmap.region, 0x19, 0x1a));
	ut_assertnonnull(alist_add(&upl->memmap, memmap));

	/* memmap[1] : 1 region */
	memmap.name = "u-boot";
	memmap.usage = BIT(UPLUS_BOOT_DATA);
	alist_init_struct(&memmap.region, struct memregion);
	ut_assertok(add_region(uts, &memmap.region, 0x21, 0x22));
	ut_assertnonnull(alist_add(&upl->memmap, memmap));

	/* memmap[2] : 1 region */
	alist_init_struct(&memmap.region, struct memregion);
	memmap.name = "efi";
	memmap.usage = BIT(UPLUS_RUNTIME_CODE);
	ut_assertok(add_region(uts, &memmap.region, 0x23, 0x24));
	ut_assertnonnull(alist_add(&upl->memmap, memmap));

	/* memmap[3]: 2 regions */
	alist_init_struct(&memmap.region, struct memregion);
	memmap.name = "empty";
	memmap.usage = 0;
	ut_assertok(add_region(uts, &memmap.region, 0x25, 0x26));
	ut_assertok(add_region(uts, &memmap.region, 0x27, 0x28));
	ut_assertnonnull(alist_add(&upl->memmap, memmap));

	/* memmap[4]: 1 region */
	alist_init_struct(&memmap.region, struct memregion);
	memmap.name = "acpi-things";
	memmap.usage = BIT(UPLUS_RUNTIME_CODE) | BIT(UPLUS_ACPI_NVS);
	ut_assertok(add_region(uts, &memmap.region, 0x29, 0x2a));
	ut_assertnonnull(alist_add(&upl->memmap, memmap));

	/* memres[0]: 1 region */
	alist_init_struct(&memres.region, struct memregion);
	memset(&memres, '\0', sizeof(memres));
	memres.name = "mmio";
	ut_assertok(add_region(uts, &memres.region, 0x2b, 0x2c));
	ut_assertnonnull(alist_add(&upl->memres, memres));

	/* memres[1]: 2 regions */
	alist_init_struct(&memres.region, struct memregion);
	memres.name = "memory";
	ut_assertok(add_region(uts, &memres.region, 0x2d, 0x2e));
	ut_assertok(add_region(uts, &memres.region, 0x2f, 0x30));
	memres.no_map = true;
	ut_assertnonnull(alist_add(&upl->memres, memres));

	upl->serial.compatible = "ns16550a";
	upl->serial.clock_frequency = 1843200;
	upl->serial.current_speed = 115200;
	alist_init_struct(&upl->serial.reg, struct memregion);
	ut_assertok(add_region(uts, &upl->serial.reg, 0xf1de0000, 0x100));
	upl->serial.reg_io_shift = 2;
	upl->serial.reg_offset = 0x40;
	upl->serial.reg_io_width = 1;
	upl->serial.virtual_reg = 0x20000000;
	upl->serial.access_type = UPLSAT_MMIO;

	alist_init_struct(&upl->graphics.reg, struct memregion);
	ut_assertok(add_region(uts, &upl->graphics.reg, 0xd0000000, 0x10000000));
	upl->graphics.width = 1280;
	upl->graphics.height = 1280;
	upl->graphics.stride = upl->graphics.width * 4;
	upl->graphics.format = UPLGF_ARGB32;

	return 0;
}

static int compare_upl_image(struct unit_test_state *uts,
			     const struct upl_image *base,
			     const struct upl_image *cmp)
{
	ut_asserteq(base->load, cmp->load);
	ut_asserteq(base->size, cmp->size);
	ut_asserteq(base->offset, cmp->offset);
	ut_asserteq_str(base->description, cmp->description);

	return 0;
}

static int compare_upl_memregion(struct unit_test_state *uts,
				 const struct memregion *base,
				 const struct memregion *cmp)
{
	ut_asserteq(base->base, cmp->base);
	ut_asserteq(base->size, cmp->size);

	return 0;
}

static int compare_upl_mem(struct unit_test_state *uts,
			   const struct upl_mem *base,
			   const struct upl_mem *cmp)
{
	int i;

	ut_asserteq(base->region.count, cmp->region.count);
	ut_asserteq(base->hotpluggable, cmp->hotpluggable);
	for (i = 0; i < base->region.count; i++) {
		ut_assertok(compare_upl_memregion(uts,
			alist_get(&base->region, i, struct memregion),
			alist_get(&cmp->region, i, struct memregion)));
	}

	return 0;
}

static int check_device_name(struct unit_test_state *uts, const char *base,
			     const char *cmp)
{
	const char *p;

	p = strchr(cmp, '@');
	if (p) {
		ut_assertnonnull(p);
		ut_asserteq_strn(base, cmp);
		ut_asserteq(p - cmp, strlen(base));
	} else {
		ut_asserteq_str(base, cmp);
	}

	return 0;
}

static int compare_upl_memmap(struct unit_test_state *uts,
			      const struct upl_memmap *base,
			      const struct upl_memmap *cmp)
{
	int i;

	ut_assertok(check_device_name(uts, base->name, cmp->name));
	ut_asserteq(base->region.count, cmp->region.count);
	ut_asserteq(base->usage, cmp->usage);
	for (i = 0; i < base->region.count; i++)
		ut_assertok(compare_upl_memregion(uts,
			alist_get(&base->region, i, struct memregion),
			alist_get(&cmp->region, i, struct memregion)));

	return 0;
}

static int compare_upl_memres(struct unit_test_state *uts,
			      const struct upl_memres *base,
			      const struct upl_memres *cmp)
{
	int i;

	ut_assertok(check_device_name(uts, base->name, cmp->name));
	ut_asserteq(base->region.count, cmp->region.count);
	ut_asserteq(base->no_map, cmp->no_map);
	for (i = 0; i < base->region.count; i++)
		ut_assertok(compare_upl_memregion(uts,
			alist_get(&base->region, i, struct memregion),
			alist_get(&cmp->region, i, struct memregion)));

	return 0;
}

static int compare_upl_serial(struct unit_test_state *uts,
			      struct upl_serial *base, struct upl_serial *cmp)
{
	int i;

	ut_asserteq_str(base->compatible, cmp->compatible);
	ut_asserteq(base->clock_frequency, cmp->clock_frequency);
	ut_asserteq(base->current_speed, cmp->current_speed);
	for (i = 0; i < base->reg.count; i++)
		ut_assertok(compare_upl_memregion(uts,
			alist_get(&base->reg, i, struct memregion),
			alist_get(&cmp->reg, i, struct memregion)));
	ut_asserteq(base->reg_io_shift, cmp->reg_io_shift);
	ut_asserteq(base->reg_offset, cmp->reg_offset);
	ut_asserteq(base->reg_io_width, cmp->reg_io_width);
	ut_asserteq(base->virtual_reg, cmp->virtual_reg);
	ut_asserteq(base->access_type, cmp->access_type);

	return 0;
}

static int compare_upl_graphics(struct unit_test_state *uts,
				struct upl_graphics *base,
				struct upl_graphics *cmp)
{
	int i;

	for (i = 0; i < base->reg.count; i++)
		ut_assertok(compare_upl_memregion(uts,
			alist_get(&base->reg, i, struct memregion),
			alist_get(&cmp->reg, i, struct memregion)));
	ut_asserteq(base->width, cmp->width);
	ut_asserteq(base->height, cmp->height);
	ut_asserteq(base->stride, cmp->stride);
	ut_asserteq(base->format, cmp->format);

	return 0;
}

static int compare_upl(struct unit_test_state *uts, struct upl *base,
		       struct upl *cmp)
{
	int i;

	ut_asserteq(base->addr_cells, cmp->addr_cells);
	ut_asserteq(base->size_cells, cmp->size_cells);

	ut_asserteq(base->smbios, cmp->smbios);
	ut_asserteq(base->acpi, cmp->acpi);
	ut_asserteq(base->bootmode, cmp->bootmode);
	ut_asserteq(base->fit, cmp->fit);
	ut_asserteq(base->conf_offset, cmp->conf_offset);
	ut_asserteq(base->addr_width, cmp->addr_width);
	ut_asserteq(base->acpi_nvs_size, cmp->acpi_nvs_size);

	ut_asserteq(base->image.count, cmp->image.count);
	for (i = 0; i < base->image.count; i++)
		ut_assertok(compare_upl_image(uts,
			alist_get(&base->image, i, struct upl_image),
			alist_get(&cmp->image, i, struct upl_image)));

	ut_asserteq(base->mem.count, cmp->mem.count);
	for (i = 0; i < base->mem.count; i++)
		ut_assertok(compare_upl_mem(uts,
			alist_get(&base->mem, i, struct upl_mem),
			alist_get(&cmp->mem, i, struct upl_mem)));

	ut_asserteq(base->memmap.count, cmp->memmap.count);
	for (i = 0; i < base->memmap.count; i++)
		ut_assertok(compare_upl_memmap(uts,
			alist_get(&base->memmap, i, struct upl_memmap),
			alist_get(&cmp->memmap, i, struct upl_memmap)));

	ut_asserteq(base->memres.count, cmp->memres.count);
	for (i = 0; i < base->memres.count; i++)
		ut_assertok(compare_upl_memres(uts,
			alist_get(&base->memres, i, struct upl_memres),
			alist_get(&cmp->memres, i, struct upl_memres)));

	ut_assertok(compare_upl_serial(uts, &base->serial, &cmp->serial));
	ut_assertok(compare_upl_graphics(uts, &base->graphics, &cmp->graphics));

	return 0;
}

/* Basic test of writing and reading UPL handoff */
static int upl_test_base(struct unit_test_state *uts)
{
	oftree tree, check_tree;
	struct upl upl, check;
	struct abuf buf;

	if (!CONFIG_IS_ENABLED(OFNODE_MULTI_TREE))
		return -EAGAIN;  /* skip test */
	ut_assertok(upl_get_test_data(uts, &upl));

	ut_assertok(upl_create_handoff_tree(&upl, &tree));
	ut_assertok(oftree_to_fdt(tree, &buf));

	/*
	 * strings in check_tree and therefore check are only valid so long as
	 * buf stays around. As soon as we call abuf_uninit they go away
	 */
	check_tree = oftree_from_fdt(abuf_data(&buf));
	ut_assert(ofnode_valid(oftree_path(check_tree, "/")));

	ut_assertok(upl_read_handoff(&check, check_tree));
	ut_assertok(compare_upl(uts, &upl, &check));
	abuf_uninit(&buf);

	return 0;
}
UPL_TEST(upl_test_base, 0);

/* Test 'upl info' command */
static int upl_test_info(struct unit_test_state *uts)
{
	gd_set_upl(NULL);
	ut_assertok(run_command("upl info", 0));
	ut_assert_nextline("UPL state: inactive");
	ut_assert_console_end();

	gd_set_upl((struct upl *)uts);	/* set it to any non-zero value */
	ut_assertok(run_command("upl info", 0));
	ut_assert_nextline("UPL state: active");
	ut_assert_console_end();
	gd_set_upl(NULL);

	return 0;
}
UPL_TEST(upl_test_info, UTF_CONSOLE);

/* Test 'upl read' and 'upl_write' commands */
static int upl_test_read_write(struct unit_test_state *uts)
{
	ulong addr;

	if (!CONFIG_IS_ENABLED(OFNODE_MULTI_TREE))
		return -EAGAIN;  /* skip test */
	ut_assertok(run_command("upl write", 0));

	addr = env_get_hex("upladdr", 0);
	ut_assert_nextline("UPL handoff written to %lx size %lx", addr,
			   env_get_hex("uplsize", 0));
	ut_assert_console_end();

	ut_assertok(run_command("upl read ${upladdr}", 0));
	ut_assert_nextline("Reading UPL at %lx", addr);
	ut_assert_console_end();

	return 0;
}
UPL_TEST(upl_test_read_write, UTF_CONSOLE);

/* Test UPL passthrough */
static int upl_test_info_norun(struct unit_test_state *uts)
{
	const struct upl_image *img;
	struct upl *upl = gd_upl();
	const void *fit;

	ut_assertok(run_command("upl info -v", 0));
	ut_assert_nextline("UPL state: active");
	ut_assert_nextline("fit %lx", upl->fit);
	ut_assert_nextline("conf_offset %x", upl->conf_offset);
	ut_assert_nextlinen("image 0");
	ut_assert_nextlinen("image 1");
	ut_assert_console_end();

	/* check the offsets */
	fit = map_sysmem(upl->fit, 0);
	ut_asserteq_str("conf-1", fdt_get_name(fit, upl->conf_offset, NULL));

	ut_asserteq(2, upl->image.count);

	img = alist_get(&upl->image, 1, struct upl_image);
	ut_asserteq_str("firmware-1", fdt_get_name(fit, img->offset, NULL));
	ut_asserteq(CONFIG_TEXT_BASE, img->load);

	return 0;
}
UPL_TEST(upl_test_info_norun, UTF_CONSOLE | UTF_MANUAL);

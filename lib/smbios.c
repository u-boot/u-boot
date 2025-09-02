// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * Adapted from coreboot src/arch/x86/smbios.c
 */

#define LOG_CATEGORY	LOGC_BOARD

#include <display_options.h>
#include <dm.h>
#include <env.h>
#include <linux/stringify.h>
#include <linux/string.h>
#include <mapmem.h>
#include <smbios.h>
#include <sysinfo.h>
#include <tables_csum.h>
#include <version.h>
#include <malloc.h>
#include <dm/ofnode.h>
#ifdef CONFIG_CPU
#include <cpu.h>
#include <dm/uclass-internal.h>
#endif
#include <linux/sizes.h>

/* Safeguard for checking that U_BOOT_VERSION_NUM macros are compatible with U_BOOT_DMI */
#if U_BOOT_VERSION_NUM < 2000 || U_BOOT_VERSION_NUM > 2099 || \
    U_BOOT_VERSION_NUM_PATCH < 1 || U_BOOT_VERSION_NUM_PATCH > 12
#error U_BOOT_VERSION_NUM macros are not compatible with DMI, fix U_BOOT_DMI macros
#endif

/*
 * U_BOOT_DMI_DATE contains BIOS Release Date in format mm/dd/yyyy.
 * BIOS Release Date is calculated from U-Boot version and fixed day 01.
 * So for U-Boot version 2021.04 it is calculated as "04/01/2021".
 * BIOS Release Date should contain date when code was released
 * and not when it was built or compiled.
 */
#if U_BOOT_VERSION_NUM_PATCH < 10
#define U_BOOT_DMI_MONTH "0" __stringify(U_BOOT_VERSION_NUM_PATCH)
#else
#define U_BOOT_DMI_MONTH __stringify(U_BOOT_VERSION_NUM_PATCH)
#endif
#define U_BOOT_DMI_DAY "01"
#define U_BOOT_DMI_YEAR __stringify(U_BOOT_VERSION_NUM)
#define U_BOOT_DMI_DATE U_BOOT_DMI_MONTH "/" U_BOOT_DMI_DAY "/" U_BOOT_DMI_YEAR

DECLARE_GLOBAL_DATA_PTR;

/**
 * struct map_sysinfo - Mapping of sysinfo strings to DT
 *
 * @si_str: sysinfo string
 * @dt_str: DT string
 * @max: Max index of the tokenized string to pick. Counting starts from 0
 *
 */
struct map_sysinfo {
	const char *si_node;
	const char *si_str;
	const char *dt_str;
	int max;
};

static const struct map_sysinfo sysinfo_to_dt[] = {
	{ .si_node = "system", .si_str = "product", .dt_str = "model", 2 },
	{ .si_node = "system", .si_str = "manufacturer",
	  .dt_str = "compatible", 1 },
	{ .si_node = "baseboard", .si_str = "product",
	  .dt_str = "model", 2 },
	{ .si_node = "baseboard", .si_str = "manufacturer",
	  .dt_str = "compatible", 1 },
	{ .si_node = "system-slot", .si_str = "slot-type",
	  .dt_str = "device_type", 0},
	{ .si_node = "system-slot", .si_str = "segment-group-number",
	  .dt_str = "linux,pci-domain", 0},
};

#if IS_ENABLED(CONFIG_GENERATE_SMBIOS_TABLE_VERBOSE)
static const struct pci_attr_lookup_table pci_attr[] = {
	{ "pci-host-ecam-generic", SMBIOS_SYSSLOT_TYPE_PCIE,
	  SMBIOS_SYSSLOT_WIDTH_8X, SMBIOS_SYSSLOT_LENG_LONG,
	  SMBIOS_SYSSLOT_CHAR_3_3V, SMBIOS_SYSSLOT_CHAR_PCIPME },
	{ "pci-host-cam-generic", SMBIOS_SYSSLOT_TYPE_PCI,
	  SMBIOS_SYSSLOT_WIDTH_32BIT, SMBIOS_SYSSLOT_LENG_SHORT,
	  SMBIOS_SYSSLOT_CHAR_5V | SMBIOS_SYSSLOT_CHAR_3_3V,
	  SMBIOS_SYSSLOT_CHAR_PCIPME },
	{ "pci-host-thunder-ecam", SMBIOS_SYSSLOT_TYPE_PCIEGEN3,
	  SMBIOS_SYSSLOT_WIDTH_8X, SMBIOS_SYSSLOT_LENG_LONG,
	  SMBIOS_SYSSLOT_CHAR_3_3V,
	  SMBIOS_SYSSLOT_CHAR_PCIPME | SMBIOS_SYSSLOT_CHAR_HOTPLUG },
	{ "pci-host-octeontx-ecam", SMBIOS_SYSSLOT_TYPE_PCIEGEN3X16,
	  SMBIOS_SYSSLOT_WIDTH_16X, SMBIOS_SYSSLOT_LENG_LONG,
	  SMBIOS_SYSSLOT_CHAR_3_3V,
	  SMBIOS_SYSSLOT_CHAR_PCIPME | SMBIOS_SYSSLOT_CHAR_HOTPLUG },
	{ "pci-host-thunder-pem", SMBIOS_SYSSLOT_TYPE_PCIEGEN4X8,
	  SMBIOS_SYSSLOT_WIDTH_8X, SMBIOS_SYSSLOT_LENG_LONG,
	  SMBIOS_SYSSLOT_CHAR_3_3V,
	  SMBIOS_SYSSLOT_CHAR_PCIPME | SMBIOS_SYSSLOT_CHAR_HOTPLUG },
	{ "pci-host-octeontx2-pem", SMBIOS_SYSSLOT_TYPE_PCIEGEN4X16,
	  SMBIOS_SYSSLOT_WIDTH_16X, SMBIOS_SYSSLOT_LENG_LONG,
	  SMBIOS_SYSSLOT_CHAR_3_3V,
	  SMBIOS_SYSSLOT_CHAR_PCIPME | SMBIOS_SYSSLOT_CHAR_HOTPLUG |
	  SMBIOS_SYSSLOT_CHAR_PCIBIF },
};
#endif

/**
 * struct smbios_ctx - context for writing SMBIOS tables
 *
 * @node:		node containing the information to write (ofnode_null()
 *			if none)
 * @dev:		sysinfo device to use (NULL if none)
 * @subnode_name:	sysinfo subnode_name. Used for DT fallback
 * @eos:		end-of-string pointer for the table being processed.
 *			This is set up when we start processing a table
 * @next_ptr:		pointer to the start of the next string to be added.
 *			When the table is not empty, this points to the byte
 *			after the \0 of the previous string.
 * @last_str:		points to the last string that was written to the table,
 *			or NULL if none
 */
struct smbios_ctx {
	ofnode node;
	struct udevice *dev;
	const char *subnode_name;
	char *eos;
	char *next_ptr;
	char *last_str;
};

typedef int (*smbios_write_subnode)(ulong *current, int handle,
				   struct smbios_ctx *ctx, int idx,
				   int type);

typedef int (*smbios_write_memnode)(ulong *current, int handle,
				    struct smbios_ctx *ctx, int idx,
				    int type);

typedef int (*smbios_write_memctrlnode)(ulong *current, int handle,
				      struct smbios_ctx *ctx, int idx,
				      u64 base, u64 sz);

/**
 * Function prototype to write a specific type of SMBIOS structure
 *
 * @addr:	start address to write the structure
 * @handle:	the structure's handle, a unique 16-bit number
 * @ctx:	context for writing the tables
 * Return:	size of the structure
 */
typedef int (*smbios_write_type)(ulong *addr, int *handle,
				 struct smbios_ctx *ctx);

/**
 * struct smbios_write_method - Information about a table-writing function
 *
 * @write: Function to call
 * @subnode_name: Name of subnode which has the information for this function,
 *	NULL if none
 */
struct smbios_write_method {
	smbios_write_type write;
	const char *subnode_name;
};

static const struct map_sysinfo *convert_sysinfo_to_dt(const char *node, const char *si)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(sysinfo_to_dt); i++) {
		if (node && !strcmp(node, sysinfo_to_dt[i].si_node) &&
		    !strcmp(si, sysinfo_to_dt[i].si_str))
			return &sysinfo_to_dt[i];
	}

	return NULL;
}

/**
 * smbios_add_string() - add a string to the string area
 *
 * This adds a string to the string area which is appended directly after
 * the formatted portion of an SMBIOS structure.
 *
 * @ctx:	SMBIOS context
 * @str:	string to add
 * Return:	string number in the string area. 0 if str is NULL.
 */
static int smbios_add_string(struct smbios_ctx *ctx, const char *str)
{
	int i = 1;
	char *p = ctx->eos;

	if (!str)
		return 0;

	for (;;) {
		if (!*p) {
			ctx->last_str = p;
			strcpy(p, str);
			p += strlen(str);
			*p++ = '\0';
			ctx->next_ptr = p;
			*p++ = '\0';

			return i;
		}

		if (!strcmp(p, str)) {
			ctx->last_str = p;
			return i;
		}

		p += strlen(p) + 1;
		i++;
	}
}

/**
 * get_str_from_dt - Get a substring from a DT property.
 *                   After finding the property in the DT, the function
 *                   will parse comma-separated values and return the value.
 *                   If nprop->max exceeds the number of comma-separated
 *                   elements, the last non NULL value will be returned.
 *                   Counting starts from zero.
 *
 * @nprop: sysinfo property to use
 * @str: pointer to fill with data
 * @size: str buffer length
 */
static
void get_str_from_dt(const struct map_sysinfo *nprop, char *str, size_t size)
{
	const char *dt_str;
	int cnt = 0;
	char *token;

	memset(str, 0, size);
	if (!nprop || !nprop->max)
		return;

	dt_str = ofnode_read_string(ofnode_root(), nprop->dt_str);
	if (!dt_str)
		return;

	memcpy(str, dt_str, size);
	token = strtok(str, ",");
	while (token && cnt < nprop->max) {
		strlcpy(str, token, strlen(token) + 1);
		token = strtok(NULL, ",");
		cnt++;
	}
}

/**
 * smbios_get_val_si() - Get value from the devicetree or sysinfo
 *
 * @ctx:	context of SMBIOS
 * @prop:	property to read
 * @sysinfo_id: unique identifier for the value to be read
 * @val_def:	Default value
 * Return:	Valid value from sysinfo or device tree, otherwise val_def.
 */
static int smbios_get_val_si(struct smbios_ctx * __maybe_unused ctx,
			     const char * __maybe_unused prop,
			     int __maybe_unused sysinfo_id, int val_def)
{
#if IS_ENABLED(CONFIG_GENERATE_SMBIOS_TABLE_VERBOSE)
	int val;
	const struct map_sysinfo *nprop;

	if (!ctx->dev)
		return val_def;

	if (!sysinfo_get_int(ctx->dev, sysinfo_id, &val))
		return val;

	if (!IS_ENABLED(CONFIG_OF_CONTROL) || !prop)
		return val_def;

	if (ofnode_valid(ctx->node) && !ofnode_read_u32(ctx->node, prop, &val))
		return val;

	/*
	 * If the node or property is not valid fallback and try the root
	 */
	if (!ofnode_read_u32(ofnode_root(), prop, &val))
		return val;

	/* If the node is still missing, try with the mapping values */
	nprop = convert_sysinfo_to_dt(ctx->subnode_name, prop);
	if (!ofnode_read_u32(ofnode_root(), nprop->dt_str, &val))
		return val;
#endif
	return val_def;
}

#if IS_ENABLED(CONFIG_GENERATE_SMBIOS_TABLE_VERBOSE)
static u64 smbios_get_u64_si(struct smbios_ctx * __maybe_unused ctx,
			     const char * __maybe_unused prop,
			     int __maybe_unused sysinfo_id, u64 val_def)
{
	size_t len;
	void *data;
	const fdt32_t *prop_val;
	int prop_len;
	u64 val = 0;

	if (!ctx->dev)
		return val_def;

	if (!sysinfo_get_data(ctx->dev, sysinfo_id, &data, &len))
		return *((u64 *)data);

	if (!IS_ENABLED(CONFIG_OF_CONTROL) || !prop || !ofnode_valid(ctx->node))
		return val_def;

	prop_val = ofnode_read_prop(ctx->node, prop, &prop_len);
	if (!prop_val || prop_len < sizeof(fdt32_t) ||
	    prop_len % sizeof(fdt32_t)) {
		/*
		 * If the node or property is not valid fallback and try the root
		 */
		prop_val = ofnode_read_prop(ofnode_root(), prop, &prop_len);
		if (!prop_val || prop_len < sizeof(fdt32_t) ||
		    prop_len % sizeof(fdt32_t))
			return val_def;
	}

	/* 64-bit: <hi lo> or 32-bit */
	if (prop_len >= sizeof(fdt32_t) * 2) {
		val = ((u64)fdt32_to_cpu(prop_val[0]) << 32) |
		     fdt32_to_cpu(prop_val[1]);
	} else {
		val = fdt32_to_cpu(prop_val[0]);
	}
	return val;
}
#endif

/**
 * smbios_add_prop_si() - Add a property from the devicetree or sysinfo
 *
 * Sysinfo is used if available, with a fallback to devicetree
 *
 * @ctx:	context for writing the tables
 * @prop:	property to write
 * @sysinfo_id: unique identifier for the string value to be read
 * @dval:	Default value to use if the string is not found or is empty
 * Return:	0 if not found, else SMBIOS string number (1 or more)
 */
static int smbios_add_prop_si(struct smbios_ctx *ctx, const char *prop,
			      int sysinfo_id, const char *dval)
{
	int ret;

	if (!dval || !*dval)
		dval = NULL;

	if (sysinfo_id && ctx->dev) {
		char val[SMBIOS_STR_MAX];

		ret = sysinfo_get_str(ctx->dev, sysinfo_id, sizeof(val), val);
		if (!ret)
			return smbios_add_string(ctx, val);
	}
	if (!prop)
		return smbios_add_string(ctx, dval);

	if (IS_ENABLED(CONFIG_OF_CONTROL)) {
		const char *str = NULL;
		char str_dt[128] = { 0 };
		/*
		 * If the node is not valid fallback and try the entire DT
		 * so we can at least fill in manufacturer and board type
		 */
		if (ofnode_valid(ctx->node)) {
			str = ofnode_read_string(ctx->node, prop);
		} else {
			const struct map_sysinfo *nprop;

			nprop = convert_sysinfo_to_dt(ctx->subnode_name, prop);
			get_str_from_dt(nprop, str_dt, sizeof(str_dt));
			str = (const char *)str_dt;
		}

		ret = smbios_add_string(ctx, str && *str ? str : dval);
		return ret;
	}

	return 0;
}

/**
 * smbios_add_prop() - Add a property from the devicetree
 *
 * @prop:	property to write. The default string will be written if
 *		prop is NULL
 * @dval:	Default value to use if the string is not found or is empty
 * Return:	0 if not found, else SMBIOS string number (1 or more)
 */
static int smbios_add_prop(struct smbios_ctx *ctx, const char *prop,
			   const char *dval)
{
	return smbios_add_prop_si(ctx, prop, SYSID_NONE, dval);
}

static void smbios_set_eos(struct smbios_ctx *ctx, char *eos)
{
	ctx->eos = eos;
	ctx->next_ptr = eos;
	ctx->last_str = NULL;
}

int smbios_update_version(const char *version)
{
	char *ptr = gd->smbios_version;
	uint old_len, len;

	if (!ptr)
		return log_ret(-ENOENT);

	/*
	 * This string is supposed to have at least enough bytes and is
	 * padded with spaces. Update it, taking care not to move the
	 * \0 terminator, so that other strings in the string table
	 * are not disturbed. See smbios_add_string()
	 */
	old_len = strnlen(ptr, SMBIOS_STR_MAX);
	len = strnlen(version, SMBIOS_STR_MAX);
	if (len > old_len)
		return log_ret(-ENOSPC);

	log_debug("Replacing SMBIOS type 0 version string '%s'\n", ptr);
	memcpy(ptr, version, len);
#ifdef LOG_DEBUG
	print_buffer((ulong)ptr, ptr, 1, old_len + 1, 0);
#endif

	return 0;
}

/**
 * smbios_string_table_len() - compute the string area size
 *
 * This computes the size of the string area including the string terminator.
 *
 * @ctx:	SMBIOS context
 * Return:	string area size
 */
static int smbios_string_table_len(const struct smbios_ctx *ctx)
{
	/* In case no string is defined we have to return two \0 */
	if (ctx->next_ptr == ctx->eos)
		return 2;

	/* Allow for the final \0 after all strings */
	return (ctx->next_ptr + 1) - ctx->eos;
}

static int smbios_write_type0(ulong *current, int *handle,
			      struct smbios_ctx *ctx)
{
	struct smbios_type0 *t;
	int len = sizeof(*t);

	t = map_sysmem(*current, len);
	memset(t, 0, len);
	fill_smbios_header(t, SMBIOS_BIOS_INFORMATION, len, *handle);
	smbios_set_eos(ctx, t->eos);
	t->vendor = smbios_add_prop_si(ctx, NULL, SYSID_SM_BIOS_VENDOR,
				       "U-Boot");

	t->bios_ver = smbios_add_prop_si(ctx, "version", SYSID_SM_BIOS_VER,
					 PLAIN_VERSION);
	if (t->bios_ver)
		gd->smbios_version = ctx->last_str;
	log_debug("smbios_version = %p: '%s'\n", gd->smbios_version,
		  gd->smbios_version);
#ifdef LOG_DEBUG
	print_buffer((ulong)gd->smbios_version, gd->smbios_version,
		     1, strlen(gd->smbios_version) + 1, 0);
#endif
	t->bios_release_date = smbios_add_prop_si(ctx, NULL,
						  SYSID_SM_BIOS_REL_DATE,
						  U_BOOT_DMI_DATE);
#ifdef CONFIG_ROM_SIZE
	if (CONFIG_ROM_SIZE < SZ_16M) {
		t->bios_rom_size = (CONFIG_ROM_SIZE / 65536) - 1;
	} else {
		/* CONFIG_ROM_SIZE < 8 GiB */
		t->bios_rom_size = 0xff;
		t->extended_bios_rom_size = CONFIG_ROM_SIZE >> 20;
	}
#endif
	t->bios_characteristics = BIOS_CHARACTERISTICS_PCI_SUPPORTED |
				  BIOS_CHARACTERISTICS_SELECTABLE_BOOT |
				  BIOS_CHARACTERISTICS_UPGRADEABLE;
#ifdef CONFIG_GENERATE_ACPI_TABLE
	t->bios_characteristics_ext1 = BIOS_CHARACTERISTICS_EXT1_ACPI;
#endif
#ifdef CONFIG_EFI_LOADER
	t->bios_characteristics_ext2 |= BIOS_CHARACTERISTICS_EXT2_UEFI;
#endif
	t->bios_characteristics_ext2 |= BIOS_CHARACTERISTICS_EXT2_TARGET;

	/* bios_major_release has only one byte, so drop century */
	t->bios_major_release = U_BOOT_VERSION_NUM % 100;
	t->bios_minor_release = U_BOOT_VERSION_NUM_PATCH;
	t->ec_major_release = 0xff;
	t->ec_minor_release = 0xff;

	len = t->hdr.length + smbios_string_table_len(ctx);
	*current += len;
	unmap_sysmem(t);

	return len;
}

static int smbios_write_type1(ulong *current, int *handle,
			      struct smbios_ctx *ctx)
{
	struct smbios_type1 *t;
	int len = sizeof(*t);
	char *serial_str = env_get("serial#");
	size_t uuid_len;
	void *uuid;

	t = map_sysmem(*current, len);
	memset(t, 0, len);
	fill_smbios_header(t, SMBIOS_SYSTEM_INFORMATION, len, *handle);
	smbios_set_eos(ctx, t->eos);

	t->manufacturer = smbios_add_prop_si(ctx, "manufacturer",
					     SYSID_SM_SYSTEM_MANUFACTURER,
					     NULL);
	t->product_name = smbios_add_prop_si(ctx, "product",
					     SYSID_SM_SYSTEM_PRODUCT, NULL);
	t->version = smbios_add_prop_si(ctx, "version",	SYSID_SM_SYSTEM_VERSION,
					NULL);
	if (serial_str) {
		t->serial_number = smbios_add_prop(ctx, NULL, serial_str);
		strlcpy((char *)t->uuid, serial_str, sizeof(t->uuid));
	} else {
		t->serial_number = smbios_add_prop_si(ctx, "serial",
						      SYSID_SM_SYSTEM_SERIAL,
						      NULL);
	}
	if (!sysinfo_get_data(ctx->dev, SYSID_SM_SYSTEM_UUID, &uuid,
			      &uuid_len) &&
	    uuid_len == sizeof(t->uuid))
		memcpy(t->uuid, uuid, sizeof(t->uuid));
	t->wakeup_type = smbios_get_val_si(ctx, "wakeup-type",
					   SYSID_SM_SYSTEM_WAKEUP,
					   SMBIOS_WAKEUP_TYPE_UNKNOWN);
	t->sku_number = smbios_add_prop_si(ctx, "sku", SYSID_SM_SYSTEM_SKU,
					   NULL);
	t->family = smbios_add_prop_si(ctx, "family", SYSID_SM_SYSTEM_FAMILY,
				       NULL);

	len = t->hdr.length + smbios_string_table_len(ctx);
	*current += len;
	unmap_sysmem(t);

	return len;
}

static int smbios_write_type2(ulong *current, int *handle,
			      struct smbios_ctx *ctx)
{
	struct smbios_type2 *t;
	int len = sizeof(*t);
	u8 *eos_addr;

	/*
	 * reserve the space for the dynamic bytes of contained object handles.
	 * TODO: len += <obj_handle_num> * SMBIOS_TYPE2_CON_OBJ_HANDLE_SIZE
	 * obj_handle_num can be from DT node "baseboard" or sysinfo driver.
	 */
	t = map_sysmem(*current, len);
	memset(t, 0, len);
	fill_smbios_header(t, SMBIOS_BOARD_INFORMATION, len, *handle);

	/* eos is at the end of the structure */
	eos_addr = (u8 *)t + len - sizeof(t->eos);
	smbios_set_eos(ctx, eos_addr);

	t->manufacturer = smbios_add_prop_si(ctx, "manufacturer",
					     SYSID_SM_BASEBOARD_MANUFACTURER,
					     NULL);
	t->product_name = smbios_add_prop_si(ctx, "product",
					     SYSID_SM_BASEBOARD_PRODUCT, NULL);
	t->version = smbios_add_prop_si(ctx, "version",
					SYSID_SM_BASEBOARD_VERSION, NULL);
	t->serial_number = smbios_add_prop_si(ctx, "serial",
					      SYSID_SM_BASEBOARD_SERIAL, NULL);
	t->asset_tag_number = smbios_add_prop_si(ctx, "asset-tag",
						 SYSID_SM_BASEBOARD_ASSET_TAG,
						 NULL);
	t->feature_flags = smbios_get_val_si(ctx, "feature-flags",
					     SYSID_SM_BASEBOARD_FEATURE, 0);

	t->chassis_location =
		smbios_add_prop_si(ctx, "chassis-location",
				   SYSID_SM_BASEBOARD_CHASSIS_LOCAT, NULL);
	t->board_type =	smbios_get_val_si(ctx, "board-type",
					  SYSID_SM_BASEBOARD_TYPE,
					  SMBIOS_BOARD_TYPE_UNKNOWN);

	/*
	 * TODO:
	 * Populate the Contained Object Handles if they exist
	 * t->number_contained_objects = <obj_handle_num>;
	 */

	t->chassis_handle = *handle + 1;

	len = t->hdr.length + smbios_string_table_len(ctx);
	*current += len;
	unmap_sysmem(t);

	return len;
}

static int smbios_write_type3(ulong *current, int *handle,
			      struct smbios_ctx *ctx)
{
	struct smbios_type3 *t;
	int len = sizeof(*t);
	u8 *eos_addr;
	size_t elem_size = 0;
	__maybe_unused u8 *elem_addr;
	__maybe_unused u8 *sku_num_addr;

	/*
	 * reserve the space for the dynamic bytes of contained elements.
	 * TODO: elem_size = <element_count> * <element_record_length>
	 * element_count and element_record_length can be from DT node
	 * "chassis" or sysinfo driver.
	 */
	len += elem_size;

	t = map_sysmem(*current, len);
	memset(t, 0, len);
	fill_smbios_header(t, SMBIOS_SYSTEM_ENCLOSURE, len, *handle);
#if IS_ENABLED(CONFIG_GENERATE_SMBIOS_TABLE_VERBOSE)
	elem_addr = (u8 *)t + offsetof(struct smbios_type3, sku_number);
	sku_num_addr = elem_addr + elem_size;
#endif
	/* eos is at the end of the structure */
	eos_addr = (u8 *)t + len - sizeof(t->eos);
	smbios_set_eos(ctx, eos_addr);

	t->manufacturer = smbios_add_prop_si(ctx, "manufacturer",
					     SYSID_SM_ENCLOSURE_MANUFACTURER,
					     NULL);
	t->chassis_type = smbios_get_val_si(ctx, "chassis-type",
					    SYSID_SM_ENCLOSURE_TYPE,
					    SMBIOS_ENCLOSURE_UNKNOWN);
	t->bootup_state = smbios_get_val_si(ctx, "bootup-state",
					    SYSID_SM_ENCLOSURE_BOOTUP,
					    SMBIOS_STATE_UNKNOWN);
	t->power_supply_state = smbios_get_val_si(ctx, "power-supply-state",
						  SYSID_SM_ENCLOSURE_POW,
						  SMBIOS_STATE_UNKNOWN);
	t->thermal_state = smbios_get_val_si(ctx, "thermal-state",
					     SYSID_SM_ENCLOSURE_THERMAL,
					     SMBIOS_STATE_UNKNOWN);
	t->security_status = smbios_get_val_si(ctx, "security-status",
					       SYSID_SM_ENCLOSURE_SECURITY,
					       SMBIOS_SECURITY_UNKNOWN);

#if IS_ENABLED(CONFIG_GENERATE_SMBIOS_TABLE_VERBOSE)
	t->version = smbios_add_prop_si(ctx, "version",
					SYSID_SM_ENCLOSURE_VERSION, NULL);
	t->serial_number = smbios_add_prop_si(ctx, "serial",
					      SYSID_SM_ENCLOSURE_SERIAL, NULL);
	t->asset_tag_number = smbios_add_prop_si(ctx, "asset-tag",
						 SYSID_SM_BASEBOARD_ASSET_TAG,
						 NULL);
	t->oem_defined = smbios_get_val_si(ctx, "oem-defined",
					   SYSID_SM_ENCLOSURE_OEM, 0);
	t->height = smbios_get_val_si(ctx, "height",
				      SYSID_SM_ENCLOSURE_HEIGHT, 0);
	t->number_of_power_cords =
		smbios_get_val_si(ctx, "number-of-power-cords",
				  SYSID_SM_ENCLOSURE_POWCORE_NUM, 0);

	/*
	 * TODO: Populate the Contained Element Record if they exist
	 * t->element_count = <element_num>;
	 * t->element_record_length = <element_len>;
	 */

	*sku_num_addr =	smbios_add_prop_si(ctx, "sku", SYSID_SM_ENCLOSURE_SKU,
					   NULL);
#endif

	len = t->hdr.length + smbios_string_table_len(ctx);
	*current += len;
	unmap_sysmem(t);

	return len;
}

static void smbios_write_type4_dm(struct smbios_type4 *t,
				  struct smbios_ctx *ctx)
{
	u16 processor_family = SMBIOS_PROCESSOR_FAMILY_UNKNOWN;
	const char *vendor = NULL;
	const char *name = NULL;
	__maybe_unused void *id_data = NULL;
	__maybe_unused size_t id_size = 0;

#ifdef CONFIG_CPU
	char processor_name[49];
	char vendor_name[49];
	struct udevice *cpu = NULL;

	uclass_find_first_device(UCLASS_CPU, &cpu);
	if (cpu) {
		struct cpu_plat *plat = dev_get_parent_plat(cpu);

		if (plat->family)
			processor_family = plat->family;
		t->processor_id[0] = plat->id[0];
		t->processor_id[1] = plat->id[1];

		if (!cpu_get_vendor(cpu, vendor_name, sizeof(vendor_name)))
			vendor = vendor_name;
		if (!cpu_get_desc(cpu, processor_name, sizeof(processor_name)))
			name = processor_name;
	}
#endif
	if (processor_family == SMBIOS_PROCESSOR_FAMILY_UNKNOWN)
		processor_family =
			smbios_get_val_si(ctx, "family",
					  SYSID_SM_PROCESSOR_FAMILY,
					  SMBIOS_PROCESSOR_FAMILY_UNKNOWN);

	if (processor_family == SMBIOS_PROCESSOR_FAMILY_EXT)
		t->processor_family2 =
			smbios_get_val_si(ctx, "family2",
					  SYSID_SM_PROCESSOR_FAMILY2,
					  SMBIOS_PROCESSOR_FAMILY_UNKNOWN);

	t->processor_family = processor_family;
	t->processor_manufacturer =
		smbios_add_prop_si(ctx, "manufacturer",
				   SYSID_SM_PROCESSOR_MANUFACT, vendor);
	t->processor_version = smbios_add_prop_si(ctx, "version",
						  SYSID_SM_PROCESSOR_VERSION,
						  name);

#if IS_ENABLED(CONFIG_GENERATE_SMBIOS_TABLE_VERBOSE)
	if (t->processor_id[0] || t->processor_id[1] ||
	    sysinfo_get_data(ctx->dev, SYSID_SM_PROCESSOR_ID, &id_data,
			     &id_size))
		return;

	if (id_data && id_size == sizeof(t->processor_id))
		memcpy((u8 *)t->processor_id, id_data, id_size);
#endif
}

static int smbios_write_type4(ulong *current, int *handle,
			      struct smbios_ctx *ctx)
{
	struct smbios_type4 *t;
	int len = sizeof(*t);
	__maybe_unused void *hdl;
	__maybe_unused size_t hdl_size;

	t = map_sysmem(*current, len);
	memset(t, 0, len);
	fill_smbios_header(t, SMBIOS_PROCESSOR_INFORMATION, len, *handle);
	smbios_set_eos(ctx, t->eos);
	t->socket_design = smbios_add_prop_si(ctx, "socket-design",
					      SYSID_SM_PROCESSOR_SOCKET, NULL);
	t->processor_type = smbios_get_val_si(ctx, "processor-type",
					      SYSID_SM_PROCESSOR_TYPE,
					      SMBIOS_PROCESSOR_TYPE_UNKNOWN);
	smbios_write_type4_dm(t, ctx);

	t->status = smbios_get_val_si(ctx, "processor-status",
				      SYSID_SM_PROCESSOR_STATUS,
				      SMBIOS_PROCESSOR_STATUS_UNKNOWN);
	t->processor_upgrade =
		smbios_get_val_si(ctx, "upgrade", SYSID_SM_PROCESSOR_UPGRADE,
				  SMBIOS_PROCESSOR_UPGRADE_UNKNOWN);

	t->l1_cache_handle = SMBIOS_CACHE_HANDLE_NONE;
	t->l2_cache_handle = SMBIOS_CACHE_HANDLE_NONE;
	t->l3_cache_handle = SMBIOS_CACHE_HANDLE_NONE;

#if IS_ENABLED(CONFIG_GENERATE_SMBIOS_TABLE_VERBOSE)
	t->voltage = smbios_get_val_si(ctx, "voltage",
				       SYSID_SM_PROCESSOR_VOLTAGE, 0);
	t->external_clock = smbios_get_val_si(ctx, "external-clock",
					      SYSID_SM_PROCESSOR_EXT_CLOCK, 0);
	t->max_speed = smbios_get_val_si(ctx, "max-speed",
					 SYSID_SM_PROCESSOR_MAX_SPEED, 0);
	t->current_speed = smbios_get_val_si(ctx, "current-speed",
					     SYSID_SM_PROCESSOR_CUR_SPEED, 0);

	/* Read the cache handles */
	if (!sysinfo_get_data(ctx->dev, SYSID_SM_CACHE_HANDLE, &hdl,
			      &hdl_size) &&
	    (hdl_size == SYSINFO_CACHE_LVL_MAX * sizeof(u16))) {
		u16 *handle = (u16 *)hdl;

		if (*handle)
			t->l1_cache_handle = *handle;

		handle++;
		if (*handle)
			t->l2_cache_handle = *handle;

		handle++;
		if (*handle)
			t->l3_cache_handle = *handle;
	}

	t->serial_number = smbios_add_prop_si(ctx, "serial",
					      SYSID_SM_PROCESSOR_SN, NULL);
	t->asset_tag = smbios_add_prop_si(ctx, "asset-tag",
					  SYSID_SM_PROCESSOR_ASSET_TAG, NULL);
	t->part_number = smbios_add_prop_si(ctx, "part-number",
					    SYSID_SM_PROCESSOR_PN, NULL);
	t->core_count =	smbios_get_val_si(ctx, "core-count",
					  SYSID_SM_PROCESSOR_CORE_CNT, 0);
	t->core_enabled = smbios_get_val_si(ctx, "core-enabled",
					    SYSID_SM_PROCESSOR_CORE_EN, 0);
	t->thread_count = smbios_get_val_si(ctx, "thread-count",
					    SYSID_SM_PROCESSOR_THREAD_CNT, 0);
	t->processor_characteristics =
		smbios_get_val_si(ctx, "characteristics",
				  SYSID_SM_PROCESSOR_CHARA,
				  SMBIOS_PROCESSOR_UND);
	t->core_count2 = smbios_get_val_si(ctx, "core-count2",
					   SYSID_SM_PROCESSOR_CORE_CNT2, 0);
	t->core_enabled2 = smbios_get_val_si(ctx, "core-enabled2",
					     SYSID_SM_PROCESSOR_CORE_EN2, 0);
	t->thread_count2 = smbios_get_val_si(ctx, "thread-count2",
					     SYSID_SM_PROCESSOR_THREAD_CNT2, 0);
	t->thread_enabled = smbios_get_val_si(ctx, "thread-enabled",
					      SYSID_SM_PROCESSOR_THREAD_EN, 0);
#endif

	len = t->hdr.length + smbios_string_table_len(ctx);
	*current += len;
	unmap_sysmem(t);

	return len;
}

#if IS_ENABLED(CONFIG_GENERATE_SMBIOS_TABLE_VERBOSE)

static int smbios_write_type7_1level(ulong *current, int handle,
				     struct smbios_ctx *ctx, int level)
{
	struct smbios_type7 *t;
	int len = sizeof(*t);
	void *hdl;
	size_t hdl_size;

	t = map_sysmem(*current, len);
	memset(t, 0, len);
	fill_smbios_header(t, SMBIOS_CACHE_INFORMATION, len, handle);
	smbios_set_eos(ctx, t->eos);

	t->socket_design = smbios_add_prop_si(ctx, "socket-design",
					      SYSID_SM_CACHE_SOCKET + level,
					      NULL);
	t->config.data = smbios_get_val_si(ctx, "config",
					   SYSID_SM_CACHE_CONFIG + level,
					   (level - 1) | SMBIOS_CACHE_OP_UND);
	t->max_size.data = smbios_get_val_si(ctx, "max-size",
					     SYSID_SM_CACHE_MAX_SIZE + level,
					     0);
	t->inst_size.data = smbios_get_val_si(ctx, "installed-size",
					      SYSID_SM_CACHE_INST_SIZE + level,
					      0);
	t->supp_sram_type.data =
		smbios_get_val_si(ctx, "supported-sram-type",
				  SYSID_SM_CACHE_SUPSRAM_TYPE + level,
				  SMBIOS_CACHE_SRAM_TYPE_UNKNOWN);
	t->curr_sram_type.data =
		smbios_get_val_si(ctx, "current-sram-type",
				  SYSID_SM_CACHE_CURSRAM_TYPE + level,
				  SMBIOS_CACHE_SRAM_TYPE_UNKNOWN);
	t->speed = smbios_get_val_si(ctx, "speed", SYSID_SM_CACHE_SPEED + level,
				     0);
	t->err_corr_type = smbios_get_val_si(ctx, "error-correction-type",
					     SYSID_SM_CACHE_ERRCOR_TYPE + level,
					     SMBIOS_CACHE_ERRCORR_UNKNOWN);
	t->sys_cache_type =
		smbios_get_val_si(ctx, "system-cache-type",
				  SYSID_SM_CACHE_SCACHE_TYPE + level,
				  SMBIOS_CACHE_SYSCACHE_TYPE_UNKNOWN);
	t->associativity = smbios_get_val_si(ctx, "associativity",
					     SYSID_SM_CACHE_ASSOC + level,
					     SMBIOS_CACHE_ASSOC_UNKNOWN);
	t->max_size2.data = smbios_get_val_si(ctx, "max-size2",
					      SYSID_SM_CACHE_MAX_SIZE2 + level,
					      0);
	t->inst_size2.data =
		smbios_get_val_si(ctx, "installed-size2",
				  SYSID_SM_CACHE_INST_SIZE2 + level, 0);

	/* Save the cache handles */
	if (!sysinfo_get_data(ctx->dev, SYSID_SM_CACHE_HANDLE, &hdl,
			      &hdl_size)) {
		if (hdl_size == SYSINFO_CACHE_LVL_MAX * sizeof(u16))
			*((u16 *)hdl + level) = handle;
	}

	len = t->hdr.length + smbios_string_table_len(ctx);
	*current += len;
	unmap_sysmem(t);

	return len;
}

static int smbios_write_type7(ulong *current, int *handle,
			      struct smbios_ctx *ctx)
{
	int len = 0;
	int i, level;
	ofnode parent = ctx->node;
	struct smbios_ctx ctx_bak;
	int hdl_base = *handle;

	memcpy(&ctx_bak, ctx, sizeof(ctx_bak));

	/* Get the number of level */
	level =	smbios_get_val_si(ctx, NULL, SYSID_SM_CACHE_LEVEL, 0);
	if (level >= SYSINFO_CACHE_LVL_MAX) /* Error, return 0-length */
		return 0;

	for (i = 0; i <= level; i++) {
		char buf[9] = "";

		if (!snprintf(buf, sizeof(buf), "l%d-cache", i + 1))
			return 0;
		ctx->subnode_name = buf;
		ctx->node = ofnode_find_subnode(parent, ctx->subnode_name);
		*handle = hdl_base + i;
		len += smbios_write_type7_1level(current, *handle, ctx, i);
		memcpy(ctx, &ctx_bak, sizeof(*ctx));
	}

	return len;
}

static int smbios_scan_subnodes(ulong *current, struct smbios_ctx *ctx,
				int *handle, smbios_write_subnode cb, int type)
{
	ofnode child;
	int i;
	int hdl_base = *handle;
	int len = 0;
	struct smbios_ctx ctx_bak;

	memcpy(&ctx_bak, ctx, sizeof(ctx_bak));

	for (i = 0, child = ofnode_first_subnode(ctx->node);
	     ofnode_valid(child); child = ofnode_next_subnode(child), i++) {
		ctx->node = child;
		*handle = hdl_base + i;
		len += cb(current, *handle, ctx, i, type);
		memcpy(ctx, &ctx_bak, sizeof(*ctx));
	}

	return len;
}

static void smbios_lookup_pci_attr(struct smbios_ctx *ctx,
				   struct smbios_type9 *t)
{
	const char *compatible;
	u32 addr_cells, size_cells, total_cells;
	const fdt32_t *reg;
	int reglen;
	int i;

	/* default attributes */
	t->slot_type = SMBIOS_SYSSLOT_TYPE_PCI;
	t->slot_data_bus_width = SMBIOS_SYSSLOT_WIDTH_UNKNOWN;
	t->slot_characteristics_1 = SMBIOS_SYSSLOT_CHAR_UND;
	t->current_usage = SMBIOS_SYSSLOT_USAGE_UNKNOWN;
	t->slot_length = SMBIOS_SYSSLOT_LENG_UNKNOWN;
	t->segment_group_number = smbios_get_val_si(ctx, "segment-group-number",
						    SYSID_NONE,
						    SMBIOS_SYSSLOT_SGGNUM_UND);

	/*
	 * Get #address-cells and #size-cells dynamically
	 * Default 3 for #address-cells and 2 for #size-cells
	 */
	addr_cells = ofnode_read_u32_default(ctx->node, "#address-cells", 3);
	size_cells = ofnode_read_u32_default(ctx->node, "#size-cells", 2);
	total_cells = addr_cells + size_cells;

	/* Read property 'reg' from the node */
	reg = ofnode_read_prop(ctx->node, "reg", &reglen);
	if (reg && reglen > addr_cells * sizeof(*reg)) {
		/* First address-cell: Bus Number */
		if (addr_cells >= 1)
			t->bus_number = fdt32_to_cpu(reg[0]);
		/* Second address-cell: Device/Function */
		if (addr_cells >= 2)
			t->device_function_number.data = fdt32_to_cpu(reg[1]);
		/*
		 * Third address-cell 'Register Offset' and the following
		 * size-cell bytes are not useful for SMBIOS type 9, just
		 * ignore them.
		 */
		/*
		 * As neither PCI IRQ Routing Table ($PIRQ) nor FDT
		 * property to represent a Slot ID, try to derive a
		 * Slot ID programmatically.
		 */
		t->slot_id = t->device_function_number.fields.dev_num |
			     (t->bus_number << 5);
	}

	/* Read 'compatible' property */
	compatible = ofnode_read_string(ctx->node, "compatible");
	if (!compatible)
		return;

	for (i = 0; i < ARRAY_SIZE(pci_attr); i++) {
		if (strstr(compatible, pci_attr[i].str)) {
			t->slot_type = pci_attr[i].slot_type;
			t->slot_data_bus_width = pci_attr[i].data_bus_width;
			t->slot_length = pci_attr[i].slot_length;
			t->slot_characteristics_1 = pci_attr[i].chara1;
			t->slot_characteristics_2 = pci_attr[i].chara2;
			/* mark it as in-use arbitrarily */
			t->current_usage = SMBIOS_SYSSLOT_USAGE_INUSE;
			return;
		}
	}
}

static void smbios_write_type9_fields(struct smbios_ctx *ctx,
				      struct smbios_type9 *t)
{
	t->slot_type = smbios_get_val_si(ctx, "slot-type", SYSID_NONE,
					 SMBIOS_SYSSLOT_TYPE_UNKNOWN);
	t->slot_data_bus_width =
		smbios_get_val_si(ctx, "data-bus-width",
				  SYSID_NONE, SMBIOS_SYSSLOT_WIDTH_UNKNOWN);
	t->current_usage = smbios_get_val_si(ctx, "current-usage", SYSID_NONE,
					     SMBIOS_SYSSLOT_USAGE_UNKNOWN);
	t->slot_length = smbios_get_val_si(ctx, "slot-length", SYSID_NONE,
					   SMBIOS_SYSSLOT_LENG_UNKNOWN);
	t->slot_id = smbios_get_val_si(ctx, "slot-id", SYSID_NONE, 0);
	t->slot_characteristics_1 =
		smbios_get_val_si(ctx, "slot-characteristics-1", SYSID_NONE,
				  SMBIOS_SYSSLOT_CHAR_UND);
	t->slot_characteristics_2 = smbios_get_val_si(ctx,
						      "slot-characteristics-2",
						      SYSID_NONE, 0);
	t->segment_group_number = smbios_get_val_si(ctx, "segment-group-number",
						    SYSID_NONE, 0);
	t->bus_number = smbios_get_val_si(ctx, "bus-number", SYSID_NONE, 0);
	t->device_function_number.data =
		smbios_get_val_si(ctx, "device-function-number", SYSID_NONE, 0);
}

static int smbios_write_type9_1slot(ulong *current, int handle,
				    struct smbios_ctx *ctx,
				    int __maybe_unused idx, int devtype)
{
	struct smbios_type9 *t;
	int len = sizeof(*t);
	u8 pgroups_cnt;
	u8 *eos_addr;
	size_t pgroups_size;
	void *wp;

	pgroups_cnt = smbios_get_val_si(ctx, "peer-grouping-count",
					SYSID_NONE, 0);
	pgroups_size = pgroups_cnt * SMBIOS_TYPE9_PGROUP_SIZE;

	/*
	 * reserve the space for the dynamic bytes of peer_groups.
	 * TODO:
	 * peer_groups = <peer_grouping_count> * SMBIOS_TYPE9_PGROUP_SIZE
	 */
	len += pgroups_size;

	t = map_sysmem(*current, len);
	memset(t, 0, len);

	fill_smbios_header(t, SMBIOS_SYSTEM_SLOTS, len, handle);

	/* eos is at the end of the structure */
	eos_addr = (u8 *)t + len - sizeof(t->eos);
	smbios_set_eos(ctx, eos_addr);

	/* Write the general fields */
	t->peer_grouping_count = pgroups_cnt;
	t->socket_design = smbios_add_prop_si(ctx, "socket-design", SYSID_NONE,
					      NULL);
	t->electrical_bus_width = smbios_get_val_si(ctx, "data-bus-width",
						    SYSID_NONE, 0);

	/* skip the reserved peer groups and write the following fields from eos */
	/* t->slot_height */
	wp = eos_addr - sizeof(t->slot_height);
	*((u8 *)wp) = smbios_get_val_si(ctx, "slot-height", SYSID_NONE, 0);
	/* t->slot_pitch */
	wp -= sizeof(t->slot_pitch);
	*((u16 *)wp) = smbios_get_val_si(ctx, "slot-pitch", SYSID_NONE, 0);
	/* t->slot_physical_width */
	wp -= sizeof(t->slot_physical_width);
	*((u8 *)wp) = smbios_get_val_si(ctx, "slot-physical-width", SYSID_NONE, 0);
	/* t->slot_information */
	wp -= sizeof(t->slot_information);
	*((u8 *)wp) = smbios_get_val_si(ctx, "slot-information", SYSID_NONE, 0);

	/* For PCI, some fields can be extracted from FDT node */
	if (devtype == SMBIOS_SYSSLOT_TYPE_PCI)
		/* Populate PCI attributes from existing PCI properties */
		smbios_lookup_pci_attr(ctx, t);
	else if (devtype == SMBIOS_SYSSLOT_TYPE_UNKNOWN) {
		/* Properties that expected in smbios subnode 'system-slot' */
		smbios_write_type9_fields(ctx, t);
	}
	len = t->hdr.length + smbios_string_table_len(ctx);
	*current += len;
	unmap_sysmem(t);

	return len;
}

static int smbios_scan_slot_type(ulong *current, int *handle,
				 struct smbios_ctx *ctx)
{
	int i = 0;
	struct smbios_ctx ctx_bak;
	ofnode child;
	const struct map_sysinfo *prop;
	int hdl_base = *handle;
	int len = 0;

	memcpy(&ctx_bak, ctx, sizeof(ctx_bak));
	prop = convert_sysinfo_to_dt(ctx->subnode_name, "slot-type");
	for (child = ofnode_first_subnode(ofnode_root()); ofnode_valid(child);
	     child = ofnode_next_subnode(child)) {
		const char *dev_type_str;
		u8 dev_type = SMBIOS_SYSSLOT_TYPE_UNKNOWN;

		dev_type_str = ofnode_read_string(child, prop->dt_str);
		if (!dev_type_str)
			continue;

		if (!strcmp(dev_type_str, "pci"))
			dev_type = SMBIOS_SYSSLOT_TYPE_PCI;
		else if (!strcmp(dev_type_str, "isa"))
			dev_type = SMBIOS_SYSSLOT_TYPE_ISA;
		else if (!strcmp(dev_type_str, "pcmcia"))
			dev_type = SMBIOS_SYSSLOT_TYPE_PCMCIA;
		else
			continue;

		*handle = hdl_base + i;
		ctx->node = child;
		len += smbios_write_type9_1slot(current, *handle, ctx, 0,
						dev_type);
		memcpy(ctx, &ctx_bak, sizeof(*ctx));
		i++;
	}

	return len;
}

static int smbios_write_type9(ulong *current, int *handle,
			      struct smbios_ctx *ctx)
{
	int len;

	/* TODO: Get system slot information via pci subsystem */
	if (!IS_ENABLED(CONFIG_OF_CONTROL))
		return 0;	/* Error, return 0-length */

	len = smbios_scan_subnodes(current, ctx, handle,
				   smbios_write_type9_1slot,
				   SMBIOS_SYSSLOT_TYPE_UNKNOWN);
	if (len)
		return len;

	/* if no subnode under 'system-slot', try scan the entire FDT */
	len = smbios_scan_slot_type(current, handle, ctx);

	return len;
}

static u64 smbios_pop_size_from_memory_node(ofnode node)
{
	const fdt32_t *reg;
	int len;
	u64 size_bytes;

	/* Read property 'reg' from the node */
	reg = ofnode_read_prop(node, "reg", &len);
	if (!reg || len < sizeof(fdt32_t) * 4 || len % sizeof(fdt32_t))
		return 0;

	/* Combine hi/lo for size (typically 64-bit) */
	size_bytes = ((u64)fdt32_to_cpu(reg[2]) << 32) | fdt32_to_cpu(reg[3]);

	return size_bytes;
}

static int
smbios_write_type16_sum_memory_nodes(ulong *current, int handle,
				     struct smbios_ctx *ctx, u16 cnt, u64 size)
{
	struct smbios_type16 *t;
	int len = sizeof(*t);
	u8 *eos_addr;
	void *hdl;
	size_t hdl_size;

	t = map_sysmem(*current, len);
	memset(t, 0, len);

	fill_smbios_header(t, SMBIOS_PHYS_MEMORY_ARRAY, len, handle);

	/* eos is at the end of the structure */
	eos_addr = (u8 *)t + len - sizeof(t->eos);
	smbios_set_eos(ctx, eos_addr);

	/* default attributes */
	t->location = SMBIOS_MA_LOCATION_MOTHERBOARD;
	t->use = SMBIOS_MA_USE_SYSTEM;
	t->mem_err_corr = SMBIOS_MA_ERRCORR_UNKNOWN;
	t->mem_err_info_hdl = SMBIOS_MA_ERRINFO_NONE;
	t->num_of_mem_dev = cnt;

	/* Use extended field */
	t->max_cap = cpu_to_le32(0x80000000);
	t->ext_max_cap = cpu_to_le64(size >> 10); /* In KB */

	/* Save the memory array handles */
	if (!sysinfo_get_data(ctx->dev, SYSID_SM_MEMARRAY_HANDLE, &hdl,
			      &hdl_size) &&
	    hdl_size == SYSINFO_MEM_HANDLE_MAX * sizeof(u16))
		*((u16 *)hdl) = handle;

	len = t->hdr.length + smbios_string_table_len(ctx);
	*current += len;
	unmap_sysmem(t);

	return len;
}

static void
smbios_pop_type16_from_memcontroller_node(ofnode node, struct smbios_type16 *t)
{
	ofnode child;
	int count = 0;
	u64 total = 0;

	/* default attributes */
	t->location = SMBIOS_MA_LOCATION_MOTHERBOARD;
	t->use = SMBIOS_MA_USE_SYSTEM;
	t->mem_err_info_hdl = SMBIOS_MA_ERRINFO_NONE;

	/* Check custom property 'ecc-enabled' */
	if (ofnode_read_bool(node, "ecc-enabled"))
		t->mem_err_corr = SMBIOS_MA_ERRCORR_SBITECC;
	else
		t->mem_err_corr = SMBIOS_MA_ERRCORR_UNKNOWN;

	/* Read subnodes with 'size' property */
	for (child = ofnode_first_subnode(node); ofnode_valid(child);
	     child = ofnode_next_subnode(child)) {
		u64 sz = 0;
		const fdt32_t *size;
		int len;

		size = ofnode_read_prop(child, "size", &len);
		if (!size || len < sizeof(fdt32_t) || len % sizeof(fdt32_t))
			continue;

		/* 64-bit size: <hi lo> or 32-bit size */
		if (len >= sizeof(fdt32_t) * 2)
			sz = ((u64)fdt32_to_cpu(size[0]) << 32) |
			     fdt32_to_cpu(size[1]);
		else
			sz = fdt32_to_cpu(size[0]);

		count++;
		total += sz;
	}

	/*
	 * Number of memory devices associated with this array
	 * (i.e., how many Type17 entries link to this Type16 array)
	 */
	t->num_of_mem_dev = count;

	/* Use extended field */
	t->max_cap = cpu_to_le32(0x80000000);
	t->ext_max_cap = cpu_to_le64(total >> 10); /* In KB */
}

static void smbios_pop_type16_si(struct smbios_ctx *ctx,
				 struct smbios_type16 *t)
{
	t->location = smbios_get_val_si(ctx, "location", SYSID_NONE,
					SMBIOS_MA_LOCATION_UNKNOWN);
	t->use = smbios_get_val_si(ctx, "use", SYSID_NONE,
				   SMBIOS_MA_USE_UNKNOWN);
	t->mem_err_corr = smbios_get_val_si(ctx, "memory-error-correction", SYSID_NONE,
					    SMBIOS_MA_ERRCORR_UNKNOWN);
	t->max_cap = smbios_get_val_si(ctx, "maximum-capacity", SYSID_NONE, 0);
	t->mem_err_info_hdl = smbios_get_val_si(ctx, "memory-error-information-handle",
						SYSID_NONE, SMBIOS_MA_ERRINFO_NONE);
	t->num_of_mem_dev = smbios_get_val_si(ctx, "number-of-memory-devices", SYSID_NONE, 1);
	t->ext_max_cap = smbios_get_u64_si(ctx, "extended-maximum-capacity", SYSID_NONE, 0);
}

static int smbios_write_type16_1array(ulong *current, int handle,
				      struct smbios_ctx *ctx, int idx,
				      int type)
{
	struct smbios_type16 *t;
	int len = sizeof(*t);
	u8 *eos_addr;
	void *hdl;
	size_t hdl_size;

	t = map_sysmem(*current, len);
	memset(t, 0, len);

	fill_smbios_header(t, SMBIOS_PHYS_MEMORY_ARRAY, len, handle);

	/* eos is at the end of the structure */
	eos_addr = (u8 *)t + len - sizeof(t->eos);
	smbios_set_eos(ctx, eos_addr);

	if (type == SMBIOS_MEM_CUSTOM)
		smbios_pop_type16_si(ctx, t);
	else if (type == SMBIOS_MEM_FDT_MEMCON_NODE)
		smbios_pop_type16_from_memcontroller_node(ctx->node, t);

	/* Save the memory array handles */
	if (!sysinfo_get_data(ctx->dev, SYSID_SM_MEMARRAY_HANDLE, &hdl,
			      &hdl_size) &&
	    hdl_size == SYSINFO_MEM_HANDLE_MAX * sizeof(u16))
		*((u16 *)hdl + idx) = handle;

	len = t->hdr.length + smbios_string_table_len(ctx);
	*current += len;
	unmap_sysmem(t);

	return len;
}

static int smbios_write_type16(ulong *current, int *handle,
			       struct smbios_ctx *ctx)
{
	int len;
	struct smbios_ctx ctx_bak;
	ofnode child;
	int idx;
	u64 total = 0;
	int count = 0;
	int hdl_base = *handle;

	if (!IS_ENABLED(CONFIG_OF_CONTROL))
		return 0;	/* Error, return 0-length */

	/* Step 1: Scan any subnode exists under 'memory-array' */
	len = smbios_scan_subnodes(current, ctx, handle,
				   smbios_write_type16_1array,
				   SMBIOS_MEM_CUSTOM);
	if (len)
		return len;

	/* Step 2: Scan 'memory' node from the entire FDT */
	for (child = ofnode_first_subnode(ofnode_root());
	     ofnode_valid(child); child = ofnode_next_subnode(child)) {
		const char *str;

		/* Look up for 'device_type = "memory"' */
		str = ofnode_read_string(child, "device_type");
		if (str && !strcmp(str, "memory")) {
			count++;
			total += smbios_pop_size_from_memory_node(child);
		}
	}
	/*
	 * Generate one type16 instance for all 'memory' nodes,
	 * use idx=0 implicitly
	 */
	if (count)
		len += smbios_write_type16_sum_memory_nodes(current, *handle,
							    ctx, count, total);

	/* Step 3: Scan 'memory-controller' node from the entire FDT */
	/* idx starts from 1 */
	memcpy(&ctx_bak, ctx, sizeof(ctx_bak));
	for (idx = 1, child = ofnode_first_subnode(ofnode_root());
	     ofnode_valid(child); child = ofnode_next_subnode(child)) {
		const char *compat;
		const char *name;

		/*
		 * Look up for node with name or property 'compatible'
		 * containing 'memory-controller'.
		 */
		name = ofnode_get_name(child);
		compat = ofnode_read_string(child, "compatible");
		if ((!compat || !strstr(compat, "memory-controller")) &&
		    (!name || !strstr(name, "memory-controller")))
			continue;

		*handle = hdl_base + idx;
		ctx->node = child;
		/*
		 * Generate one type16 instance for each 'memory-controller'
		 * node, sum the 'size' of all subnodes.
		 */
		len += smbios_write_type16_1array(current, *handle, ctx, idx,
						  SMBIOS_MEM_FDT_MEMCON_NODE);
		idx++;
		memcpy(ctx, &ctx_bak, sizeof(*ctx));
	}

	return len;
}

static void smbios_pop_type17_general_si(struct smbios_ctx *ctx,
					 struct smbios_type17 *t)
{
	t->mem_err_info_hdl =
		smbios_get_val_si(ctx, "memory-error-information-handle",
				  SYSID_NONE, SMBIOS_MD_ERRINFO_NONE);
	t->total_width = smbios_get_val_si(ctx, "total-width", SYSID_NONE, 0);
	t->data_width = smbios_get_val_si(ctx, "data-width", SYSID_NONE, 0);
	t->form_factor = smbios_get_val_si(ctx, "form-factor",
					   SYSID_NONE, SMBIOS_MD_FF_UNKNOWN);
	t->dev_set = smbios_get_val_si(ctx, "device-set", SYSID_NONE,
				       SMBIOS_MD_DEVSET_UNKNOWN);
	t->data_width = smbios_get_val_si(ctx, "data-width", SYSID_NONE, 0);
	t->dev_locator = smbios_add_prop_si(ctx, "device-locator", SYSID_NONE,
					    NULL);
	t->bank_locator = smbios_add_prop_si(ctx, "bank-locator", SYSID_NONE,
					     NULL);
	t->mem_type = smbios_get_val_si(ctx, "memory-type",
					SYSID_NONE, SMBIOS_MD_TYPE_UNKNOWN);
	t->type_detail = smbios_get_val_si(ctx, "type-detail",
					   SYSID_NONE, SMBIOS_MD_TD_UNKNOWN);
	t->speed = smbios_get_val_si(ctx, "speed", SYSID_NONE,
				     SMBIOS_MD_SPEED_UNKNOWN);
	t->manufacturer = smbios_add_prop_si(ctx, "manufacturer", SYSID_NONE,
					     NULL);
	t->serial_number = smbios_add_prop_si(ctx, "serial-number", SYSID_NONE,
					      NULL);
	t->asset_tag = smbios_add_prop_si(ctx, "asset-tag", SYSID_NONE, NULL);
	t->part_number = smbios_add_prop_si(ctx, "part-number", SYSID_NONE,
					    NULL);
	t->attributes = smbios_get_val_si(ctx, "attributes", SYSID_NONE,
					  SMBIOS_MD_ATTR_RANK_UNKNOWN);
	t->config_mem_speed = smbios_get_val_si(ctx, "configured-memory-speed",
						SYSID_NONE,
						SMBIOS_MD_CONFSPEED_UNKNOWN);
	t->min_voltage = smbios_get_val_si(ctx, "minimum-voltage", SYSID_NONE,
					   SMBIOS_MD_VOLTAGE_UNKNOWN);
	t->max_voltage = smbios_get_val_si(ctx, "maximum-voltage", SYSID_NONE,
					   SMBIOS_MD_VOLTAGE_UNKNOWN);
	t->config_voltage = smbios_get_val_si(ctx, "configured-voltage",
					      SYSID_NONE,
					      SMBIOS_MD_VOLTAGE_UNKNOWN);
	t->mem_tech = smbios_get_val_si(ctx, "memory-technology",
					SYSID_NONE, SMBIOS_MD_TECH_UNKNOWN);
	t->mem_op_mode_cap =
		smbios_get_val_si(ctx, "memory-operating-mode-capability",
				  SYSID_NONE, SMBIOS_MD_OPMC_UNKNOWN);
	t->fw_ver = smbios_add_prop_si(ctx, "firmware-version", SYSID_NONE,
				       NULL);
	t->module_man_id = smbios_get_val_si(ctx, "module-manufacturer-id",
					     SYSID_NONE, 0);
	t->module_prod_id = smbios_get_val_si(ctx, "module-product-id",
					      SYSID_NONE, 0);
	t->mem_subsys_con_man_id =
		smbios_get_val_si(ctx,
				  "memory-subsystem-controller-manufacturer-id",
				  SYSID_NONE, 0);
	t->mem_subsys_con_prod_id =
		smbios_get_val_si(ctx,
				  "memory-subsystem-controller-product-id",
				  SYSID_NONE, 0);
	t->nonvolatile_size = smbios_get_u64_si(ctx, "non-volatile-size",
						SYSID_NONE,
						SMBIOS_MS_PORT_SIZE_UNKNOWN);
	t->volatile_size = smbios_get_u64_si(ctx, "volatile-size",
					     SYSID_NONE,
					     SMBIOS_MS_PORT_SIZE_UNKNOWN);
	t->cache_size = smbios_get_u64_si(ctx, "cache-size",
					  SYSID_NONE,
					  SMBIOS_MS_PORT_SIZE_UNKNOWN);
	t->logical_size = smbios_get_u64_si(ctx, "logical-size",
					    SYSID_NONE,
					    SMBIOS_MS_PORT_SIZE_UNKNOWN);
	t->ext_speed = smbios_get_val_si(ctx, "extended-speed", SYSID_NONE, 0);
	t->ext_config_mem_speed =
		smbios_get_val_si(ctx, "extended-configured-memory-speed",
				  SYSID_NONE, 0);
	t->pmic0_man_id = smbios_get_val_si(ctx, "pmic0-manufacturer-id",
					    SYSID_NONE, 0);
	t->pmic0_rev_num = smbios_get_val_si(ctx, "pmic0-revision-number",
					     SYSID_NONE, 0);
	t->rcd_man_id = smbios_get_val_si(ctx, "rcd-manufacturer-id",
					  SYSID_NONE, 0);
	t->rcd_rev_num = smbios_get_val_si(ctx, "rcd-revision-number",
					   SYSID_NONE, 0);
}

static void
smbios_pop_type17_size_from_memory_node(ofnode node, struct smbios_type17 *t)
{
	const fdt32_t *reg;
	int len;
	u64 sz;
	u32 size_mb;

	/* Read property 'reg' from the node */
	reg = ofnode_read_prop(node, "reg", &len);
	if (!reg || len < sizeof(fdt32_t) * 4 || len % sizeof(fdt32_t))
		return;

	/* Combine hi/lo for size (typically 64-bit) */
	sz = ((u64)fdt32_to_cpu(reg[2]) << 32) | fdt32_to_cpu(reg[3]);

	/* Convert size to MB */
	size_mb = (u32)(sz >> 20); /* 1 MB = 2^20 */
	if (size_mb < SMBIOS_MD_SIZE_EXT) {
		t->size = cpu_to_le16(size_mb);
		t->ext_size = 0;
		return;
	}

	t->size = cpu_to_le16(SMBIOS_MD_SIZE_EXT); /* Signal extended used */
	t->ext_size = cpu_to_le32((u32)(sz >> 10)); /* In KB */
}

static void smbios_pop_type17_size_si(struct smbios_ctx *ctx,
				      struct smbios_type17 *t)
{
	t->size = smbios_get_val_si(ctx, "size", SYSID_NONE,
				    SMBIOS_MD_SIZE_UNKNOWN);
	t->ext_size = smbios_get_val_si(ctx, "extended-size", SYSID_NONE, 0);
}

static int
smbios_scan_memctrl_subnode(ulong *current, int *handle, struct smbios_ctx *ctx,
			    int idx, smbios_write_memctrlnode cb)
{
	int total_len = 0;
	ofnode child;
	int i = 0;
	int hdl_base = *handle;
	u64 base = 0;

	/*
	 * Enumerate all subnodes of 'memory-controller' that contain 'size'
	 * property and generate one instance for each.
	 */
	for (child = ofnode_first_subnode(ctx->node); ofnode_valid(child);
	     child = ofnode_next_subnode(child)) {
		u64 sz = 0;
		const fdt32_t *size;
		int proplen;

		size = ofnode_read_prop(child, "size", &proplen);
		if (!size || proplen < sizeof(fdt32_t) ||
		    proplen % sizeof(fdt32_t))
			continue;

		/* 64-bit size: <hi lo> or 32-bit size */
		if (proplen >= sizeof(fdt32_t) * 2)
			sz = ((u64)fdt32_to_cpu(size[0]) << 32) |
			     fdt32_to_cpu(size[1]);
		else
			sz = fdt32_to_cpu(size[0]);

		*handle = hdl_base + i;
		total_len += cb(current, *handle, ctx, idx, base, sz);
		base += sz;
		i++;
	}

	return total_len;
}

static int
smbios_write_type17_from_memctrl_node(ulong *current, int handle,
				      struct smbios_ctx *ctx, int idx,
				      u64 __maybe_unused base, u64 sz)
{
	struct smbios_type17 *t;
	int len;
	u8 *eos_addr;
	u32 size_mb;
	void *hdl;
	size_t hdl_size;

	len = sizeof(*t);
	t = map_sysmem(*current, len);
	memset(t, 0, len);

	fill_smbios_header(t, SMBIOS_MEMORY_DEVICE, len, handle);

	/* eos is at the end of the structure */
	eos_addr = (u8 *)t + len - sizeof(t->eos);
	smbios_set_eos(ctx, eos_addr);

	/* Read the memory array handles */
	if (!sysinfo_get_data(ctx->dev, SYSID_SM_MEMARRAY_HANDLE, &hdl,
			      &hdl_size) &&
		hdl_size == SYSINFO_MEM_HANDLE_MAX * sizeof(u16))
		t->phy_mem_array_hdl = *((u16 *)hdl + idx);

	/* Convert to MB */
	size_mb = (u32)(sz >> 20);
	if (size_mb < SMBIOS_MD_SIZE_EXT) {
		/* Use 16-bit size field */
		t->size = cpu_to_le16(size_mb);  /* In MB */
		t->ext_size = cpu_to_le32(0);
	} else {
		/* Signal use of extended size field */
		t->size = cpu_to_le16(SMBIOS_MD_SIZE_EXT);
		t->ext_size = cpu_to_le32((u32)(sz >> 10)); /* In KB */
	}

	/* Write other general fields */
	smbios_pop_type17_general_si(ctx, t);

	len = t->hdr.length + smbios_string_table_len(ctx);
	*current += len;
	unmap_sysmem(t);

	return len;
}

static int smbios_write_type17_mem(ulong *current, int handle,
				   struct smbios_ctx *ctx, int idx,
				   int type)
{
	struct smbios_type17 *t;
	int len;
	u8 *eos_addr;
	void *hdl;
	size_t hdl_size;

	len = sizeof(*t);
	t = map_sysmem(*current, len);
	memset(t, 0, len);

	fill_smbios_header(t, SMBIOS_MEMORY_DEVICE, len, handle);

	/* eos is at the end of the structure */
	eos_addr = (u8 *)t + len - sizeof(t->eos);
	smbios_set_eos(ctx, eos_addr);

	if (type == SMBIOS_MEM_CUSTOM) {
		smbios_pop_type17_size_si(ctx, t);

		t->phy_mem_array_hdl =
			smbios_get_val_si(ctx, "physical-memory-array-handle",
					  SYSID_NONE, 0);
	} else if (type == SMBIOS_MEM_FDT_MEM_NODE) {
		smbios_pop_type17_size_from_memory_node(ctx->node, t);

		/* Read the memory array handles */
		if (!sysinfo_get_data(ctx->dev, SYSID_SM_MEMARRAY_HANDLE, &hdl,
				      &hdl_size) &&
		    hdl_size == SYSINFO_MEM_HANDLE_MAX * sizeof(u16))
			t->phy_mem_array_hdl = *((u16 *)hdl + idx);
	}

	/* Write other general fields */
	smbios_pop_type17_general_si(ctx, t);

	len = t->hdr.length + smbios_string_table_len(ctx);
	*current += len;
	unmap_sysmem(t);

	return len;
}

static int smbios_scan_mem_nodes(ulong *current, int *handle,
				 struct smbios_ctx *ctx,
				 smbios_write_memnode mem_cb,
				 int *idx)
{
	int len = 0;
	struct smbios_ctx ctx_bak;
	ofnode child;
	int hdl_base = *handle;

	memcpy(&ctx_bak, ctx, sizeof(ctx_bak));

	for (child = ofnode_first_subnode(ofnode_root());
	     ofnode_valid(child); child = ofnode_next_subnode(child)) {
		const char *str;

		/* Look up for 'device_type = "memory"' */
		str = ofnode_read_string(child, "device_type");
		if (!str || strcmp(str, "memory"))
			continue;

		ctx->node = child;
		*handle = hdl_base + *idx;
		/* Generate one instance for each 'memory' node */
		len += mem_cb(current, *handle, ctx, *idx,
			      SMBIOS_MEM_FDT_MEM_NODE);
		memcpy(ctx, &ctx_bak, sizeof(*ctx));
		(*idx)++;
	}

	return len;
}

static int smbios_scan_mctrl_subnodes(ulong *current, int *handle,
				      struct smbios_ctx *ctx,
				      smbios_write_memctrlnode mctrl_wcb,
				      int *idx)
{
	int len = 0;
	struct smbios_ctx ctx_bak;
	ofnode child;

	memcpy(&ctx_bak, ctx, sizeof(ctx_bak));

	for (child = ofnode_first_subnode(ofnode_root());
	     ofnode_valid(child); child = ofnode_next_subnode(child)) {
		const char *compat;
		const char *name;

		/*
		 * Look up for node with name or property 'compatible'
		 * containing 'memory-controller'.
		 */
		name = ofnode_get_name(child);
		compat = ofnode_read_string(child, "compatible");
		if ((!compat || !strstr(compat, "memory-controller")) &&
		    (!name || !strstr(name, "memory-controller")))
			continue;

		(*handle)++;
		ctx->node = child;
		/*
		 * Generate one instance for each subnode of
		 * 'memory-controller' which contains property 'size'.
		 */
		len += smbios_scan_memctrl_subnode(current, handle, ctx,
						   *idx, mctrl_wcb);
		memcpy(ctx, &ctx_bak, sizeof(*ctx));
		(*idx)++;
	}
	return len;
}

static int smbios_write_type1719(ulong *current, int *handle,
				 struct smbios_ctx *ctx,
				 smbios_write_memnode mem_cb,
				 smbios_write_memctrlnode mctrl_wcb)
{
	int len = 0;
	int idx;

	if (!IS_ENABLED(CONFIG_OF_CONTROL))
		return 0;	/* Error, return 0-length */

	/* Step 1: Scan any subnode exists */
	len = smbios_scan_subnodes(current, ctx, handle, mem_cb,
				   SMBIOS_MEM_CUSTOM);
	if (len)
		return len;

	/* Step 2: Scan 'memory' node from the entire FDT */
	idx = 0;
	len += smbios_scan_mem_nodes(current, handle, ctx, mem_cb, &idx);

	/* Step 3: Scan 'memory-controller' node from the entire FDT */
	len += smbios_scan_mctrl_subnodes(current, handle, ctx, mctrl_wcb, &idx);

	return len;
}

static int smbios_write_type17(ulong *current, int *handle,
			       struct smbios_ctx *ctx)
{
	return smbios_write_type1719(current, handle, ctx,
				     smbios_write_type17_mem,
				     smbios_write_type17_from_memctrl_node);
}

static void smbios_pop_type19_general_si(struct smbios_ctx *ctx,
					 struct smbios_type19 *t)
{
	t->partition_wid =
		smbios_get_val_si(ctx, "partition-width ",
				  SYSID_NONE, SMBIOS_MAMA_PW_DEF);
}

static void smbios_pop_type19_addr_si(struct smbios_ctx *ctx,
				      struct smbios_type19 *t)
{
	t->start_addr = smbios_get_val_si(ctx, "starting-address", SYSID_NONE,
					  0);
	t->end_addr = smbios_get_val_si(ctx, "ending-address", SYSID_NONE, 0);
	t->ext_start_addr = smbios_get_u64_si(ctx, "extended-starting-address",
					      SYSID_NONE, 0);
	t->ext_end_addr = smbios_get_u64_si(ctx, "extended-ending-address",
					    SYSID_NONE, 0);
}

static void
smbios_pop_type19_addr_from_memory_node(ofnode node, struct smbios_type19 *t)
{
	const fdt32_t *reg;
	int len;
	u64 sz;
	u64 addr;

	/* Read property 'reg' from the node */
	reg = ofnode_read_prop(node, "reg", &len);
	if (!reg || len < sizeof(fdt32_t) * 4 || len % sizeof(fdt32_t))
		return;

	/* Combine hi/lo for size and address (typically 64-bit) */
	sz = ((u64)fdt32_to_cpu(reg[2]) << 32) | fdt32_to_cpu(reg[3]);
	addr = ((u64)fdt32_to_cpu(reg[0]) << 32) | fdt32_to_cpu(reg[1]);

	t->ext_start_addr = cpu_to_le64(addr);
	t->ext_end_addr = cpu_to_le64(addr + sz - 1);

	/* If address range fits in 32-bit, populate legacy fields */
	if ((addr + sz - 1) <= 0xFFFFFFFFULL) {
		t->start_addr = cpu_to_le32((u32)addr);
		t->end_addr   = cpu_to_le32((u32)(addr + sz - 1));
	} else {
		t->start_addr = cpu_to_le32(0xFFFFFFFF);
		t->end_addr   = cpu_to_le32(0xFFFFFFFF);
	}
}

static int
smbios_write_type19_from_memctrl_node(ulong *current, int handle,
				      struct smbios_ctx *ctx, int idx,
				      u64 base, u64 sz)
{
	struct smbios_type19 *t;
	int len;
	u8 *eos_addr;
	void *hdl;
	size_t hdl_size;

	len = sizeof(*t);
	t = map_sysmem(*current, len);
	memset(t, 0, len);

	fill_smbios_header(t, SMBIOS_MEMORY_ARRAY_MAPPED_ADDRESS, len, handle);

	/* eos is at the end of the structure */
	eos_addr = (u8 *)t + len - sizeof(t->eos);
	smbios_set_eos(ctx, eos_addr);

	/* Read the memory array handles */
	if (!sysinfo_get_data(ctx->dev, SYSID_SM_MEMARRAY_HANDLE, &hdl,
			      &hdl_size) &&
	    hdl_size == SYSINFO_MEM_HANDLE_MAX * sizeof(u16))
		t->mem_array_hdl = *((u16 *)hdl + idx);

	t->ext_start_addr = cpu_to_le64(base);
	t->ext_end_addr = cpu_to_le64(base + sz - 1);

	if ((base + sz - 1) <= 0xFFFFFFFFULL) {
		t->start_addr = cpu_to_le32((u32)base);
		t->end_addr   = cpu_to_le32((u32)(base + sz - 1));
	} else {
		t->start_addr = cpu_to_le32(0xFFFFFFFF);
		t->end_addr   = cpu_to_le32(0xFFFFFFFF);
	}

	/* Write other general fields */
	smbios_pop_type19_general_si(ctx, t);

	len = t->hdr.length + smbios_string_table_len(ctx);
	*current += len;
	unmap_sysmem(t);

	return len;
}

static int smbios_write_type19_mem(ulong *current, int handle,
				   struct smbios_ctx *ctx, int idx,
				   int type)
{
	struct smbios_type19 *t;
	int len;
	u8 *eos_addr;
	void *hdl;
	size_t hdl_size;

	len = sizeof(*t);
	t = map_sysmem(*current, len);
	memset(t, 0, len);

	fill_smbios_header(t, SMBIOS_MEMORY_ARRAY_MAPPED_ADDRESS, len, handle);

	/* eos is at the end of the structure */
	eos_addr = (u8 *)t + len - sizeof(t->eos);
	smbios_set_eos(ctx, eos_addr);

	if (type == SMBIOS_MEM_CUSTOM) {
		smbios_pop_type19_addr_si(ctx, t);
		t->mem_array_hdl = smbios_get_val_si(ctx, "memory-array-handle",
						     SYSID_NONE, 0);
	} else if (type == SMBIOS_MEM_FDT_MEM_NODE) {
		smbios_pop_type19_addr_from_memory_node(ctx->node, t);
		/* Read the memory array handles */
		if (!sysinfo_get_data(ctx->dev, SYSID_SM_MEMARRAY_HANDLE, &hdl,
				      &hdl_size) &&
		    hdl_size == SYSINFO_MEM_HANDLE_MAX * sizeof(u16))
			t->mem_array_hdl = *((u16 *)hdl + idx);
	}

	/* Write other general fields */
	smbios_pop_type19_general_si(ctx, t);

	len = t->hdr.length + smbios_string_table_len(ctx);
	*current += len;
	unmap_sysmem(t);

	return len;
}

static int smbios_write_type19(ulong *current, int *handle,
			       struct smbios_ctx *ctx)
{
	return smbios_write_type1719(current, handle, ctx,
				     smbios_write_type19_mem,
				     smbios_write_type19_from_memctrl_node);
}

#endif /* #if IS_ENABLED(CONFIG_GENERATE_SMBIOS_TABLE_VERBOSE) */

static int smbios_write_type32(ulong *current, int *handle,
			       struct smbios_ctx *ctx)
{
	struct smbios_type32 *t;
	int len = sizeof(*t);

	t = map_sysmem(*current, len);
	memset(t, 0, len);
	fill_smbios_header(t, SMBIOS_SYSTEM_BOOT_INFORMATION, len, *handle);
	smbios_set_eos(ctx, t->eos);

	*current += len;
	unmap_sysmem(t);

	return len;
}

static int smbios_write_type127(ulong *current, int *handle,
				struct smbios_ctx *ctx)
{
	struct smbios_type127 *t;
	int len = sizeof(*t);

	t = map_sysmem(*current, len);
	memset(t, 0, len);
	fill_smbios_header(t, SMBIOS_END_OF_TABLE, len, *handle);

	*current += len;
	unmap_sysmem(t);

	return len;
}

static struct smbios_write_method smbios_write_funcs[] = {
	{ smbios_write_type0, "bios", },
	{ smbios_write_type1, "system", },
	{ smbios_write_type2, "baseboard", },
	/* Type 3 must immediately follow type 2 due to chassis handle. */
	{ smbios_write_type3, "chassis", },
#if IS_ENABLED(CONFIG_GENERATE_SMBIOS_TABLE_VERBOSE)
	/* Type 7 must ahead of type 4 to get cache handles. */
	{ smbios_write_type7, "cache", },
#endif
	{ smbios_write_type4, "processor"},
#if IS_ENABLED(CONFIG_GENERATE_SMBIOS_TABLE_VERBOSE)
	{ smbios_write_type9, "system-slot"},
	{ smbios_write_type16, "memory-array"},
	{ smbios_write_type17, "memory-device"},
	{ smbios_write_type19, "memory-array-mapped-address"},
#endif
	{ smbios_write_type32, },
	{ smbios_write_type127 },
};

ulong write_smbios_table(ulong addr)
{
	ofnode parent_node = ofnode_null();
	ulong table_addr, start_addr;
	struct smbios3_entry *se;
	struct smbios_ctx ctx;
	ulong tables;
	int len = 0;
	int handle = 0;
	int i;

	ctx.node = ofnode_null();
	if (CONFIG_IS_ENABLED(SYSINFO)) {
		uclass_first_device(UCLASS_SYSINFO, &ctx.dev);
		if (ctx.dev) {
			int ret;

			parent_node = dev_read_subnode(ctx.dev, "smbios");
			ret = sysinfo_detect(ctx.dev);

			/*
			 * ignore the error since many boards don't implement
			 * this and we can still use the info in the devicetree
			 */
			ret = log_msg_ret("sys", ret);
		}
	} else {
		ctx.dev = NULL;
	}

	start_addr = addr;

	/* move past the (so-far-unwritten) header to start writing structs */
	addr = ALIGN(addr + sizeof(struct smbios3_entry), 16);
	tables = addr;

	/* populate minimum required tables */
	for (i = 0; i < ARRAY_SIZE(smbios_write_funcs); i++) {
		const struct smbios_write_method *method;

		method = &smbios_write_funcs[i];
		ctx.subnode_name = NULL;
		if (method->subnode_name) {
			ctx.subnode_name = method->subnode_name;
			if (ofnode_valid(parent_node))
				ctx.node = ofnode_find_subnode(parent_node,
							       method->subnode_name);
		}
		len += method->write((ulong *)&addr, &handle, &ctx);
		handle++;
	}

	/*
	 * We must use a pointer here so things work correctly on sandbox. The
	 * user of this table is not aware of the mapping of addresses to
	 * sandbox's DRAM buffer.
	 */
	table_addr = (ulong)map_sysmem(tables, 0);

	/* now go back and write the SMBIOS3 header */
	se = map_sysmem(start_addr, sizeof(struct smbios3_entry));
	memset(se, '\0', sizeof(struct smbios3_entry));
	memcpy(se->anchor, "_SM3_", 5);
	se->length = sizeof(struct smbios3_entry);
	se->major_ver = SMBIOS_MAJOR_VER;
	se->minor_ver = SMBIOS_MINOR_VER;
	se->doc_rev = 0;
	se->entry_point_rev = 1;
	se->table_maximum_size = len;
	se->struct_table_address = table_addr;
	se->checksum = table_compute_checksum(se, sizeof(struct smbios3_entry));
	unmap_sysmem(se);

	return addr;
}

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
	{ .si_node = "system", .si_str = "manufacturer", .dt_str = "compatible", 1 },
	{ .si_node = "baseboard", .si_str = "product", .dt_str = "model", 2 },
	{ .si_node = "baseboard", .si_str = "manufacturer", .dt_str = "compatible", 1 },
};

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

/**
 * Function prototype to write a specific type of SMBIOS structure
 *
 * @addr:	start address to write the structure
 * @handle:	the structure's handle, a unique 16-bit number
 * @ctx:	context for writing the tables
 * Return:	size of the structure
 */
typedef int (*smbios_write_type)(ulong *addr, int handle,
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
#endif
	return val_def;
}

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

static int smbios_write_type0(ulong *current, int handle,
			      struct smbios_ctx *ctx)
{
	struct smbios_type0 *t;
	int len = sizeof(*t);

	t = map_sysmem(*current, len);
	memset(t, 0, len);
	fill_smbios_header(t, SMBIOS_BIOS_INFORMATION, len, handle);
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

static int smbios_write_type1(ulong *current, int handle,
			      struct smbios_ctx *ctx)
{
	struct smbios_type1 *t;
	int len = sizeof(*t);
	char *serial_str = env_get("serial#");
	size_t uuid_len;
	void *uuid;

	t = map_sysmem(*current, len);
	memset(t, 0, len);
	fill_smbios_header(t, SMBIOS_SYSTEM_INFORMATION, len, handle);
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

static int smbios_write_type2(ulong *current, int handle,
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
	fill_smbios_header(t, SMBIOS_BOARD_INFORMATION, len, handle);

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

	t->chassis_handle = handle + 1;

	len = t->hdr.length + smbios_string_table_len(ctx);
	*current += len;
	unmap_sysmem(t);

	return len;
}

static int smbios_write_type3(ulong *current, int handle,
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
	fill_smbios_header(t, SMBIOS_SYSTEM_ENCLOSURE, len, handle);
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

static int smbios_write_type4(ulong *current, int handle,
			      struct smbios_ctx *ctx)
{
	struct smbios_type4 *t;
	int len = sizeof(*t);
	__maybe_unused void *hdl;
	__maybe_unused size_t hdl_size;

	t = map_sysmem(*current, len);
	memset(t, 0, len);
	fill_smbios_header(t, SMBIOS_PROCESSOR_INFORMATION, len, handle);
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

static int smbios_write_type7(ulong *current, int handle,
			      struct smbios_ctx *ctx)
{
	int len = 0;
	int i, level;
	ofnode parent = ctx->node;
	struct smbios_ctx ctx_bak;

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
		len += smbios_write_type7_1level(current, handle++, ctx, i);
		memcpy(ctx, &ctx_bak, sizeof(*ctx));
	}
	return len;
}

#endif /* #if IS_ENABLED(CONFIG_GENERATE_SMBIOS_TABLE_VERBOSE) */

static int smbios_write_type32(ulong *current, int handle,
			       struct smbios_ctx *ctx)
{
	struct smbios_type32 *t;
	int len = sizeof(*t);

	t = map_sysmem(*current, len);
	memset(t, 0, len);
	fill_smbios_header(t, SMBIOS_SYSTEM_BOOT_INFORMATION, len, handle);
	smbios_set_eos(ctx, t->eos);

	*current += len;
	unmap_sysmem(t);

	return len;
}

static int smbios_write_type127(ulong *current, int handle,
				struct smbios_ctx *ctx)
{
	struct smbios_type127 *t;
	int len = sizeof(*t);

	t = map_sysmem(*current, len);
	memset(t, 0, len);
	fill_smbios_header(t, SMBIOS_END_OF_TABLE, len, handle);

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
		len += method->write((ulong *)&addr, handle++, &ctx);
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

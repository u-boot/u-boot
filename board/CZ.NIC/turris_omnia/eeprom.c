// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Marek Behún <kabel@kernel.org>
 */

#include <asm/unaligned.h>
#include <ctype.h>
#include <linux/compiler.h>
#include <linux/kernel.h>
#include <eeprom_field.h>
#include <eeprom_layout.h>
#include <u-boot/crc.h>

#define _DEF_FIELD(_n, _s, _t) \
	{ _n, _s, NULL, eeprom_field_print_ ## _t, eeprom_field_update_ ## _t }

static void eeprom_field_print_ramsz(const struct eeprom_field *field)
{
	printf(PRINT_FIELD_SEGMENT, field->name);
	printf("%u\n", get_unaligned_le32(field->buf));
}

static int eeprom_field_update_ramsz(struct eeprom_field *field, char *value)
{
	u32 sz;

	if (value[0] == '1' || value[0] == '2' || value[0] == '4')
		sz = value[0] - '0';
	else
		return -1;

	if (value[1] != '\0')
		return -1;

	put_unaligned_le32(sz, field->buf);

	return 0;
}

static void eeprom_field_print_region(const struct eeprom_field *field)
{
	eeprom_field_print_ascii(field);
}

static int eeprom_field_update_region(struct eeprom_field *field, char *value)
{
	if (strlen(value) != 2) {
		printf("%s: has to be 2 characters\n", field->name);
		return -1;
	}

	memcpy(field->buf, value, 2);
	memset(&field->buf[2], '\0', 2);

	return 0;
}

static void eeprom_field_print_ddr_speed(const struct eeprom_field *field)
{
	printf(PRINT_FIELD_SEGMENT, field->name);

	if (field->buf[0] == '\0' || field->buf[0] == 0xff)
		puts("(empty, defaults to 1600K)\n");
	else
		printf("%.5s\n", field->buf);
}

bool omnia_valid_ddr_speed(const char *name);
void omnia_print_ddr_speeds(void);

static int eeprom_field_update_ddr_speed(struct eeprom_field *field,
					 char *value)
{
	if (value[0] == '\0') {
		/* setting default value */
		memset(field->buf, 0xff, field->size);

		return 0;
	}

	if (!omnia_valid_ddr_speed(value)) {
		printf("%s: invalid setting, supported values are:\n  ",
		       field->name);
		omnia_print_ddr_speeds();

		return -1;
	}

	strncpy(field->buf, value, field->size);

	return 0;
}

static void eeprom_field_print_bool(const struct eeprom_field *field)
{
	unsigned char val = field->buf[0];

	printf(PRINT_FIELD_SEGMENT, field->name);

	if (val == 0xff)
		puts("(empty, defaults to 0)\n");
	else
		printf("%u\n", val);
}

static int eeprom_field_update_bool(struct eeprom_field *field, char *value)
{
	unsigned char *val = &field->buf[0];

	if (value[0] == '\0') {
		/* setting default value */
		*val = 0xff;

		return 0;
	}

	if (value[1] != '\0')
		return -1;

	if (value[0] == '1' || value[0] == '0')
		*val = value[0] - '0';
	else
		return -1;

	return 0;
}

static struct eeprom_field omnia_layout[] = {
	_DEF_FIELD("Magic constant", 4, bin),
	_DEF_FIELD("RAM size in GB", 4, ramsz),
	_DEF_FIELD("Wi-Fi Region", 4, region),
	_DEF_FIELD("CRC32 checksum", 4, bin),
	_DEF_FIELD("DDR speed", 5, ddr_speed),
	_DEF_FIELD("Use old DDR training", 1, bool),
	_DEF_FIELD("Extended reserved fields", 38, reserved),
	_DEF_FIELD("Extended CRC32 checksum", 4, bin),
};

static struct eeprom_field *crc_field = &omnia_layout[3];
static struct eeprom_field *ext_crc_field =
	&omnia_layout[ARRAY_SIZE(omnia_layout) - 1];

static int omnia_update_field(struct eeprom_layout *layout, char *field_name,
			      char *new_data)
{
	struct eeprom_field *field;
	int err;

	if (!new_data)
		return 0;

	if (!field_name)
		return -1;

	field = eeprom_layout_find_field(layout, field_name, true);
	if (!field)
		return -1;

	err = field->update(field, new_data);
	if (err) {
		printf("Invalid data for field %s\n", field_name);
		return err;
	}

	if (field < crc_field) {
		u32 crc = crc32(0, layout->data, 12);
		put_unaligned_le32(crc, crc_field->buf);
	}

	if (field < ext_crc_field) {
		u32 crc = crc32(0, layout->data, 60);
		put_unaligned_le32(crc, ext_crc_field->buf);
	}

	return 0;
}

void eeprom_layout_assign(struct eeprom_layout *layout, int)
{
	layout->fields = omnia_layout;
	layout->num_of_fields = ARRAY_SIZE(omnia_layout);
	layout->update = omnia_update_field;
	layout->data_size = 64;
}

int eeprom_layout_detect(unsigned char *)
{
	/* Turris Omnia has only one version of EEPROM layout */
	return 0;
}

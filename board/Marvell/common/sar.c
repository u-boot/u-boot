/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:       GPL-2.0+
 * https://spdx.org/licenses
 */

#include <common.h>
#include <errno.h>
#include <asm/io.h>
#include <i2c.h>
#include <fdtdec.h>
#include <mvebu/sar.h>

DECLARE_GLOBAL_DATA_PTR;

struct sar_data  __attribute__((section(".data"))) sar_board;
#define board_get_sar() (&sar_board)
#define board_get_sar_table() (sar_board.sar_lookup)

static u32 swap_value(u32 val, u32 bit_length)
{
	u32 var_mask = (1 << bit_length) - 1;

	val = (val & 0xFFFF0000) >> 16 | (val & 0x0000FFFF) << 16;
	val = (val & 0xFF00FF00) >> 8 | (val & 0x00FF00FF) << 8;
	val = (val & 0xF0F0F0F0) >> 4 | (val & 0x0F0F0F0F) << 4;
	val = (val & 0xCCCCCCCC) >> 2 | (val & 0x33333333) << 2;
	val = (val & 0xAAAAAAAA) >> 1 | (val & 0x55555555) << 1;
	val = (val >> (32 - bit_length)) & var_mask;
	debug("value is swaped, new value = 0x%x\n", val);

	return val;
}

static int sar_read_reg(u32 *reg)
{
	struct udevice *dev;
	uchar byte = 0;
	int ret, chip;
	u32 sar_reg = 0;
	struct sar_data *sar = board_get_sar();
	int reg_width = sar->bit_width;
	u8  reg_mask = (1 << reg_width) - 1;

	for (chip = 0; chip  < sar->chip_count; chip++) {
		ret = i2c_get_chip_for_busnum(0, sar->chip_addr[chip], 1, &dev);
		if (ret) {
			printf("Error: %s: Failed getting chip busnum 0x%x\n",
			       __func__, sar->chip_addr[chip]);
			return -1;
		}
		ret = dm_i2c_read(dev, 0, &byte, 1);
		if (ret) {
			printf("Error: %s: Failed reading from chip 0x%x\n",
			       __func__, sar->chip_addr[chip]);
			return -1;
		}
		sar_reg |= (byte & reg_mask) << (chip * reg_width);
	}
	debug("read: sar register = 0x%08x\n", sar_reg);
	*reg = sar_reg;

	return 0;
}

int sar_write_reg(u32 sar_reg)
{
	struct udevice *dev;
	uchar byte = 0;
	int ret, chip;
	struct sar_data *sar = board_get_sar();
	int reg_width = sar->bit_width;
	u8  reg_mask = (1 << reg_width) - 1;

	for (chip = 0; chip  < sar->chip_count; chip++) {
		byte = (sar_reg >> (chip * reg_width)) & reg_mask;
		ret = i2c_get_chip_for_busnum(0, sar->chip_addr[chip], 1, &dev);
		if (ret) {
			printf("Error: %s: Failed getting chip busnum 0x%x\n",
			       __func__, sar->chip_addr[chip]);
			return -1;
		}

		ret = dm_i2c_write(dev, 0, &byte, 1);
		if (ret) {
			printf("Error: %s: Failed writing to chip 0x%x\n",
			       __func__, sar->chip_addr[chip]);
			return -1;
		}
	}
	debug("write: sar register = 0x%08x\n", sar_reg);
	/*
	 * Wait for the write to complete. The write can take
	 * up to 10mSec (we allow a little more time).
	 */
	mdelay(11);

	return 0;
}

int sar_read_var(struct sar_var *var, int *val)
{
	u32 sar_reg;
	u32 var_mask = (1 << var->bit_length) - 1;

	if (sar_read_reg(&sar_reg))
		return -1;

	(*val) = (sar_reg >> var->start_bit) & var_mask;
	if (var->swap_bit)
		(*val) = swap_value(*val, var->bit_length);

	debug("var offset = %d len = %d val = 0x%x\n", var->start_bit,
	      var->bit_length, (*val));

	return 0;
}

int sar_write_var(struct sar_var *var, int val)
{
	u32 sar_reg;
	u32 var_mask = (1 << var->bit_length) - 1;

	if (sar_read_reg(&sar_reg))
		return -1;

	/* Update the bitfield inside the sar register */
	if (var->swap_bit)
		val = swap_value(val, var->bit_length);
	val &= var_mask;
	sar_reg &= ~(var_mask << var->start_bit);
	sar_reg |= (val << var->start_bit);

	/* Write the full sar register back to i2c */
	if (sar_write_reg(sar_reg))
		return -1;

	return 0;
}

static int sar_default_var(struct sar_var *var)
{
	struct var_opts *opts;
	struct var_opts *dflt =	NULL;
	int i;

	opts = var->option_desc;

	for (i = 0; i < var->option_cnt; i++, opts++) {
		if (opts->flags & VAR_IS_DEFAULT)
			dflt = opts;
	}

	if (!dflt) {
		printf("Error: Failed to find default option");
		return -1;
	}

	if (sar_write_var(var, dflt->value)) {
		printf("Error: Failed to write default value");
		return -1;
	}

	debug("Wrote default value 0x%x = %s\n", dflt->value, dflt->desc);
	return 0;
}

int sar_get_key_id(const char *key)
{
	struct sar_var *sar_table = board_get_sar_table();
	int id;

	for (id = 0; id < MAX_SAR; id++) {
		if (strcmp(key, sar_table[id].key) == 0)
			return id;
	}
	return -1;
}

int sar_is_var_active(int id)
{
	struct sar_var *sar_table = board_get_sar_table();

	return sar_table[id].active;
}

struct var_opts *sar_get_var_opts(int id, int *cnt)
{
	struct sar_var *sar_table = board_get_sar_table();

	(*cnt) = sar_table[id].option_cnt;

	return sar_table[id].option_desc;
}

int sar_validate_key(const char *key)
{
	int id = sar_get_key_id(key);

	if (id  == -1) {
		printf("Satr: Error: Unknown key \"%s\"\n", key);
		return -1;
	}
	if (sar_is_var_active(id) == 0) {
		printf("Satr: Error: Key \"%s\" is inactive on this board\n",
		       key);
		return -1;
	}
	return id;
}

struct sar_var *sar_id_to_var(int id)
{
	struct sar_var *sar_table = board_get_sar_table();

	sar_table += id;
	return sar_table;
}

/* Interface to SatR command */
int sar_is_available(void)
{
	if (!board_get_sar_table())
		return 0;
	else
		return 1;
}

void sar_print_var(int id, bool print_opts)
{
	int cnt;
	struct var_opts *opts;
	struct sar_var *sar_table = board_get_sar_table();

	printf("%-10s %s\n", sar_table[id].key, sar_table[id].desc);

	if (print_opts) {
		opts = sar_get_var_opts(id, &cnt);
		while (cnt--) {
			printf("\t0x%-2x %s ", opts->value, opts->desc);
			if (opts->flags & VAR_IS_DEFAULT)
				printf("[Default]");
			printf("\n");
			opts++;
		}
	}
}

int sar_read_all(void)
{
	struct sar_var *var;
	int id;

	for (id = 0; id < MAX_SAR; id++)
		if (sar_is_var_active(id)) {
			var = sar_id_to_var(id);
			sar_print_key(var->key);
		}

	return 0;
}

void sar_list_keys(void)
{
	int id;

	printf("\n");
	for (id = 0; id < MAX_SAR; id++) {
		if (sar_is_var_active(id))
			sar_print_var(id, 0);
	}
	printf("\n");
}

int sar_list_key_opts(const char *key)
{
	int id = sar_validate_key(key);

	if (id == -1)
		return -EINVAL;

	printf("\n");
	sar_print_var(id, 1);
	printf("\n");

	return 0;
}

int  sar_print_key(const char *key)
{
	int id = sar_validate_key(key);
	struct sar_var *var;
	struct var_opts *opts;
	char *desc = NULL;
	int val, ret, cnt;

	if (id == -1)
		return -EINVAL;

	var = sar_id_to_var(id);
	ret = sar_read_var(var, &val);
	if (ret)
		return ret;

	opts = sar_get_var_opts(id, &cnt);
	while (cnt--) {
		if (opts->value == val)
			desc = opts->desc;
		opts++;
	}

	if (!desc)
		printf("%s = 0x%x  ERROR: UNKNOWN OPTION\n", key, val);
	else
		printf("%s = 0x%x  %s\n", key, val, desc);

	return 0;
}

int  sar_write_key(const char *key, int val)
{
	int id = sar_validate_key(key);
	struct sar_var *var;
	struct var_opts *opts;
	char *desc = NULL;
	int cnt;

	if (id == -1)
		return -EINVAL;

	var = sar_id_to_var(id);
	opts = sar_get_var_opts(id, &cnt);
	while (cnt--) {
		if (opts->value == val)
			desc = opts->desc;
		opts++;
	}

	if (!desc) {
		printf("ERROR: value 0x%x not supported for key %s\n",
		       val, key);
		printf("use \"SatR list %s\" to print supported values\n",
		       key);
		return -1;
	}

	if (sar_write_var(var, val))
		return -1;

	/* Display the updated variable */
	sar_print_key(key);

	return 0;
}

int sar_default_all(void)
{
	struct sar_var *var;
	int id;
	int ret = 0;

	for (id = 0; id < MAX_SAR; id++) {
		if (sar_is_var_active(id)) {
			var = sar_id_to_var(id);
			ret |= sar_default_var(var);
			sar_print_key(var->key);
		}
	}

	return ret;
}

int  sar_default_key(const char *key)
{
	int id = sar_validate_key(key);
	struct sar_var *var;
	int ret;

	if (id == -1)
		return -EINVAL;

	var = sar_id_to_var(id);
	ret = sar_default_var(var);
	if (ret)
		return ret;

	/* Display the updated variable */
	sar_print_key(key);

	return 0;
}

static void sar_dump(void)
{
#ifdef DEBUG
	struct sar_data *sar = board_get_sar();
	struct sar_var *sar_var;
	int i, id;

	printf("Sample at reset Dumper:\n");
	printf("\tSatR had %d chip addresses: ", sar->chip_count);
	for (i = 0; i < sar->chip_count; i++)
		printf("0x%x ", sar->chip_addr[i]);
	printf("\n\tBit width for the I2C chip is: 0x%x\n", sar->bit_width);
	printf("\tAll SatR variables thet available:\n");
	for (i = 0, sar_var = sar->sar_lookup; i < MAX_SAR; i++, sar_var++) {
		if (sar_var->active == 0)
			continue;
		printf("\t\tID = %d, ", i);
		printf("Key = %s, ", sar_var->key);
		printf("Desc. = %s", sar_var->desc);
		if (sar_var->swap_bit)
			printf(", BIT is swapped");
		printf("\n\t\tStart bit = 0x%x, ", sar_var->start_bit);
		printf("Bit length = %d\n", sar_var->bit_length);
		printf("\t\tThis variable had %d options:\n",
		       sar_var->option_cnt);
		for (id = 0; id < sar_var->option_cnt; id++) {
			printf("\t\t\tValue = 0x%x, ",
			       sar_var->option_desc[id].value);
			printf("Desc. = %s, ", sar_var->option_desc[id].desc);
			printf("Is Default = %d\n",
			       sar_var->option_desc[id].flags);
		}
	}
#endif
}

void sar_init(void)
{
	int i, var_default;
	int node, var, len, lenp;
	const char *str;
	const void *blob = gd->fdt_blob;
	struct sar_var *sar_var;
	struct sar_data *sar = board_get_sar();

	/* Get sar node from the FDT blob */
	node = fdt_node_offset_by_compatible(blob, -1,
					     fdtdec_get_compatible(
					     COMPAT_MVEBU_SAR));
	if (node < 0) {
		debug("No sar node found in FDT blob\n");
		return;
	}

	/* Get the bit width of the sapmple at reset i2c register */
	sar->bit_width = fdtdec_get_int(blob, node, "bit_width", 1);
	/* Get the address count of sample at reset i2c */
	sar->chip_count = fdtdec_get_int(blob, node, "chip_count", 1);
	/* get the address in array */
	if (fdtdec_get_int_array(blob, node, "reg",
				 sar->chip_addr, sar->chip_count) != 0) {
		pr_err("No sample at reset addresses found in FDT blob\n");
		return;
	}
	/* Get the fisrt variable in sample at reset */
	var = fdt_first_subnode(blob, node);
	if (!var) {
		pr_err("No sample at reset variables found in FDT\n");
		return;
	}
	sar_var = sar->sar_lookup;
	/* Find the variables under sample at reset node */
	while (var > 0) {
		/* if the variable is disabled skip it */
		if (!fdtdec_get_is_enabled(blob, var)) {
			/* Get the offset of the next subnode */
			var = fdt_next_subnode(blob, var);
			sar_var++;
			continue;
		}
		/* Get the key of the var option */
		sar_var->key = (char *)fdt_stringlist_get(blob, var, "key",
							  0, &lenp);
		/* Get the description of the var */
		sar_var->desc = (char *)fdt_stringlist_get(blob, var,
							   "description", 0,
							   &lenp);
		/* set the different options of the var */
		sar_var->active = 1;
		sar_var->start_bit = fdtdec_get_int(blob, var, "start-bit", 0);
		sar_var->bit_length = fdtdec_get_int(blob, var,
						     "bit-length", 0);
		sar_var->option_cnt = fdtdec_get_int(blob, var,
						     "option-cnt", 0);
		sar_var->swap_bit = fdtdec_get_bool(blob, var, "swap-bit");
		/* Get the options list */
		len = fdt_stringlist_count(blob, var, "options");
		if ((len < 0) || (sar_var->option_cnt * 2 != len)) {
			pr_err("%s: failed to parse the \"options\" property",
			       __func__);
			return;
		}
		var_default = fdtdec_get_int(blob, var, "default", 0);
		/* Fill the struct with the options from the FDT */
		for (i = 0; i < len; i += 2) {
			str = fdt_stringlist_get(blob, var, "options", i,
						 &lenp);
			sar_var->option_desc[i / 2].value = simple_strtoul(
							  str, NULL, 16);
			sar_var->option_desc[i / 2].desc =
			(char *)fdt_stringlist_get(blob, var, "options",
						   i + 1, &lenp);
			if (sar_var->option_desc[i / 2].value == var_default)
				sar_var->option_desc[i / 2].flags =
				VAR_IS_DEFAULT;
		}
		/* Get the offset of the next subnode */
		var = fdt_next_subnode(blob, var);
		sar_var++;
	}

	sar_dump();
}

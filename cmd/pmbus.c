// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2026 Free Mobile, Vincent Jardin
 *
 * pmbus U-Boot CLI command.
 *
 * Generic command surface over the PMBus 1.x framework defined in
 * <pmbus.h> + lib/pmbus.c.
 *
 * See doc/develop/pmbus.rst for the full usage reference.
 */

#include <command.h>
#include <dm.h>
#include <i2c.h>
#include <log.h>
#include <pmbus.h>
#include <vsprintf.h>
#include <linux/ctype.h>
#include <power/regulator.h>

static int parse_bus_addr(const char *s, int *bus_seq, u8 *addr)
{
	char busbuf[8];
	const char *colon;
	unsigned long b, a;
	size_t buslen;

	colon = strchr(s, ':');
	if (!colon)
		return -EINVAL;
	buslen = colon - s;
	if (buslen == 0 || buslen >= sizeof(busbuf))
		return -EINVAL;
	memcpy(busbuf, s, buslen);
	busbuf[buslen] = '\0';
	if (strict_strtoul(busbuf, 10, &b))
		return -EINVAL;
	if (strict_strtoul(colon + 1, 16, &a) || a > 0x7f)
		return -EINVAL;
	*bus_seq = (int)b;
	*addr = (u8)a;
	return 0;
}

static const struct {
	const char *name;
	u8 reg;
} pmbus_reg_syms[] = {
	{ "PAGE",            PMBUS_PAGE },
	{ "OPERATION",       PMBUS_OPERATION },
	{ "ON_OFF_CONFIG",   PMBUS_ON_OFF_CONFIG },
	{ "CLEAR_FAULTS",    PMBUS_CLEAR_FAULTS },
	{ "WRITE_PROTECT",   PMBUS_WRITE_PROTECT },
	{ "CAPABILITY",      PMBUS_CAPABILITY },
	{ "VOUT_MODE",       PMBUS_VOUT_MODE },
	{ "VOUT_COMMAND",    PMBUS_VOUT_COMMAND },
	{ "VOUT_TRIM",       PMBUS_VOUT_TRIM },
	{ "VOUT_MAX",        PMBUS_VOUT_MAX },
	{ "VOUT_SCALE_LOOP", PMBUS_VOUT_SCALE_LOOP },
	{ "STATUS_BYTE",     PMBUS_STATUS_BYTE },
	{ "STATUS_WORD",     PMBUS_STATUS_WORD },
	{ "STATUS_VOUT",     PMBUS_STATUS_VOUT },
	{ "STATUS_IOUT",     PMBUS_STATUS_IOUT },
	{ "STATUS_INPUT",    PMBUS_STATUS_INPUT },
	{ "STATUS_TEMP",     PMBUS_STATUS_TEMPERATURE },
	{ "STATUS_CML",      PMBUS_STATUS_CML },
	{ "READ_VIN",        PMBUS_READ_VIN },
	{ "READ_IIN",        PMBUS_READ_IIN },
	{ "READ_VOUT",       PMBUS_READ_VOUT },
	{ "READ_IOUT",       PMBUS_READ_IOUT },
	{ "READ_TEMP1",      PMBUS_READ_TEMPERATURE_1 },
	{ "READ_TEMP2",      PMBUS_READ_TEMPERATURE_2 },
	{ "READ_TEMP3",      PMBUS_READ_TEMPERATURE_3 },
	{ "READ_DUTY",       PMBUS_READ_DUTY_CYCLE },
	{ "READ_FREQ",       PMBUS_READ_FREQUENCY },
	{ "READ_POUT",       PMBUS_READ_POUT },
	{ "READ_PIN",        PMBUS_READ_PIN },
	{ "REVISION",        PMBUS_REVISION },
	{ "MFR_ID",          PMBUS_MFR_ID },
	{ "MFR_MODEL",       PMBUS_MFR_MODEL },
	{ "MFR_REVISION",    PMBUS_MFR_REVISION },
};

static int parse_reg(const char *s, u8 *reg)
{
	unsigned long v;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(pmbus_reg_syms); i++) {
		if (!strcasecmp(s, pmbus_reg_syms[i].name)) {
			*reg = pmbus_reg_syms[i].reg;
			return 0;
		}
	}
	if (strict_strtoul(s, 16, &v) || v > 0xff)
		return -EINVAL;
	*reg = (u8)v;
	return 0;
}

static const char *pmbus_reg_name(u8 reg)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(pmbus_reg_syms); i++)
		if (pmbus_reg_syms[i].reg == reg)
			return pmbus_reg_syms[i].name;
	return "?";
}

static int require_active(struct udevice **chip,
			  const struct pmbus_active_dev **act)
{
	*act = pmbus_active();
	if (!*act) {
		printf("pmbus: no active device. Use 'pmbus dev <bus>:<addr>' first.\n");
		return CMD_RET_FAILURE;
	}
	if (pmbus_active_get_i2c(chip)) {
		printf("pmbus: cannot reach i2c%d:0x%02x\n",
		       (*act)->bus_seq, (*act)->addr);
		return CMD_RET_FAILURE;
	}
	return CMD_RET_SUCCESS;
}

static void print_micro(s64 micro, const char *unit)
{
	s64 abs_milli;

	abs_milli = (micro < 0 ? -micro : micro) / 1000LL;
	printf("%lld.%03lld%s",
	       (long long)(micro / 1000000LL),
	       (long long)(abs_milli % 1000LL), unit);
}

static void print_active(const struct pmbus_active_dev *act)
{
	printf("pmbus: active i2c%d:0x%02x", act->bus_seq, act->addr);
	if (act->name[0])
		printf("  rail=\"%s\"", act->name);
	printf("  MFR_ID=\"%s\"  MODEL=\"%s\"  vendor=%s%s\n",
	       act->mfr_id[0] ? act->mfr_id : "?",
	       act->mfr_model[0] ? act->mfr_model : "?",
	       act->vendor[0] ? act->vendor : "(generic)",
	       act->info ? "" : "  [no driver_info]");
}

static int do_dev(struct cmd_tbl *cmdtp, int flag, int argc,
		  char *const argv[])
{
	const struct pmbus_active_dev *act;
	int bus_seq, ret;
	u8 addr;

	if (argc < 2) {
		act = pmbus_active();
		if (!act) {
			printf("pmbus: no active device\n");
			return CMD_RET_SUCCESS;
		}
		print_active(act);
		return CMD_RET_SUCCESS;
	}

	if (parse_bus_addr(argv[1], &bus_seq, &addr) < 0) {
		ret = pmbus_resolve_by_name(argv[1], &bus_seq, &addr);
		if (ret) {
			printf("pmbus: '%s' is neither <bus>:<addr> nor a known regulator-name (%d)\n",
			       argv[1], ret);
			return CMD_RET_FAILURE;
		}
	}
	ret = pmbus_set_active(bus_seq, addr);
	if (ret) {
		printf("pmbus: cannot select i2c%d:0x%02x (%d)\n",
		       bus_seq, addr, ret);
		return CMD_RET_FAILURE;
	}
	act = pmbus_active();
	if (act)
		print_active(act);
	return CMD_RET_SUCCESS;
}

static int do_list(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[])
{
	struct udevice *dev;
	struct uclass *uc;
	int ret, n = 0;

	if (!IS_ENABLED(CONFIG_DM_REGULATOR)) {
		printf("pmbus: CONFIG_DM_REGULATOR not enabled in this build. Use 'pmbus dev <bus>:<addr>'.\n");
		return CMD_RET_SUCCESS;
	}

	ret = uclass_get(UCLASS_REGULATOR, &uc);
	if (ret) {
		printf("pmbus: UCLASS_REGULATOR not available\n");
		return CMD_RET_SUCCESS;
	}
	uclass_foreach_dev(dev, uc) {
		struct dm_regulator_uclass_plat *up = dev_get_uclass_plat(dev);
		struct udevice *parent = dev_get_parent(dev);
		const char *rname = (up && up->name) ? up->name : "";
		const char *drv = (dev->driver && dev->driver->name)
				   ? dev->driver->name : "?";
		int bus_seq = -1;
		int addr = -1;

		if (parent && device_get_uclass_id(parent) == UCLASS_I2C) {
			bus_seq = dev_seq(parent);
			addr = dev_read_addr(dev);
		}

		if (n == 0)
			printf("UCLASS_REGULATOR devices (no PMBus filter):\n");
		if (bus_seq >= 0 && addr >= 0) {
			printf("  i2c%d:0x%02x  rail=\"%s\"  node=%s  driver=%s\n",
			       bus_seq, addr, rname, dev->name, drv);
		} else {
			printf("  (non-I2C)   rail=\"%s\"  node=%s  driver=%s\n",
			       rname, dev->name, drv);
		}
		n++;
	}
	if (!n)
		printf("pmbus: no UCLASS_REGULATOR devices bound. Use 'pmbus dev <bus>:<addr>' to select a chip directly.\n");
	return CMD_RET_SUCCESS;
}

static int do_info(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[])
{
	static const char * const cls_names[PSC_NUM_CLASSES] = {
		"VOLTAGE_IN", "VOLTAGE_OUT", "CURRENT_IN", "CURRENT_OUT",
		"POWER", "TEMPERATURE",
	};
	static const char * const fmt_names[] = {
		"LINEAR", "IEEE754", "DIRECT", "VID",
	};
	const struct pmbus_active_dev *act;
	struct udevice *chip;
	int rc, c, rrev;
	u8 rev = 0;

	rc = require_active(&chip, &act);
	if (rc)
		return rc;

	rrev = pmbus_read_byte(chip, PMBUS_REVISION, &rev);

	printf("pmbus device i2c%d:0x%02x\n", act->bus_seq, act->addr);
	if (act->name[0])
		printf("  regulator-name: \"%s\"\n", act->name);
	printf("  MFR_ID        : \"%s\"\n", act->mfr_id[0] ? act->mfr_id : "?");
	printf("  MFR_MODEL     : \"%s\"\n", act->mfr_model[0] ? act->mfr_model : "?");

	/*
	 * MFR_REVISION may encodes the revision as a non printable byte
	 * (BCD nibbles, packed major / minor, etc.). Show both the
	 * printable form and the raw bytes the chip returned.
	 */
	{
		u8 raw[PMBUS_MFR_STRING_MAX];
		int len, i;

		if (dm_i2c_read(chip, PMBUS_MFR_REVISION, raw, 1) ||
		    raw[0] < 1 || raw[0] > sizeof(raw) - 1 ||
		    dm_i2c_read(chip, PMBUS_MFR_REVISION, raw, raw[0] + 1)) {
			printf("  MFR_REVISION  : \"%s\"\n",
			       act->mfr_revision[0] ? act->mfr_revision : "?");
		} else {
			len = raw[0];
			printf("  MFR_REVISION  : \"%s\"  raw=0x",
			       act->mfr_revision[0] ? act->mfr_revision : "?");
			for (i = 1; i <= len; i++)
				printf("%02x", raw[i]);
			printf("\n");
		}
	}

	if (rrev)
		printf("  PMBUS_REVISION: <read failed (%d)>\n", rrev);
	else
		printf("  PMBUS_REVISION: 0x%02x (%s)\n", rev,
		       rev == PMBUS_REV_13 ? "PMBus 1.3" :
		       rev == PMBUS_REV_12 ? "PMBus 1.2" :
		       rev == PMBUS_REV_11 ? "PMBus 1.1" :
		       rev == PMBUS_REV_10 ? "PMBus 1.0" : "unknown");
	printf("  vendor        : %s\n", act->vendor[0] ? act->vendor : "(none)");

	if (!act->info) {
		printf("  driver_info   : not registered (decoders fall back to LINEAR16 / LINEAR11)\n");
		return CMD_RET_SUCCESS;
	}
	printf("  driver_info   : pages=%d\n", act->info->pages);
	for (c = 0; c < PSC_NUM_CLASSES; c++) {
		printf("    [%-12s] format=%s",
		       cls_names[c], fmt_names[act->info->format[c]]);
		if (act->info->format[c] == pmbus_fmt_direct)
			printf(", m=%d, b=%d, R=%d",
			       act->info->m[c], act->info->b[c], act->info->R[c]);
		printf("\n");
	}
	return CMD_RET_SUCCESS;
}

static int do_telemetry(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	const struct pmbus_active_dev *act;
	struct udevice *chip;
	int rc;

	rc = require_active(&chip, &act);
	if (rc)
		return rc;

	printf("pmbus telemetry @ i2c%d:0x%02x\n", act->bus_seq, act->addr);
	pmbus_print_telemetry(chip);
	return CMD_RET_SUCCESS;
}

static int do_status(struct cmd_tbl *cmdtp, int flag, int argc,
		     char *const argv[])
{
	const struct pmbus_active_dev *act;
	struct udevice *chip;
	u16 word = 0;
	u8 b;
	int rc;

	rc = require_active(&chip, &act);
	if (rc)
		return rc;

	if (pmbus_read_word(chip, PMBUS_STATUS_WORD, &word)) {
		printf("pmbus: STATUS_WORD read failed\n");
		return CMD_RET_FAILURE;
	}

	const struct pmbus_status_override *ovr =
		act->info ? act->info->status_overrides : NULL;

	printf("pmbus status @ i2c%d:0x%02x\n", act->bus_seq, act->addr);
	printf("  STATUS_WORD    (79h) = 0x%04x  [", word);
	pmbus_print_status_bits(PMBUS_STATUS_WORD, word,
				pmbus_status_word_bits, ovr);
	printf("]\n");

	if (pmbus_read_byte(chip, PMBUS_STATUS_VOUT, &b) == 0) {
		printf("  STATUS_VOUT    (7Ah) = 0x%02x  [", b);
		pmbus_print_status_bits(PMBUS_STATUS_VOUT, b,
					pmbus_status_vout_bits, ovr);
		printf("]\n");
	}
	if (pmbus_read_byte(chip, PMBUS_STATUS_IOUT, &b) == 0) {
		printf("  STATUS_IOUT    (7Bh) = 0x%02x  [", b);
		pmbus_print_status_bits(PMBUS_STATUS_IOUT, b,
					pmbus_status_iout_bits, ovr);
		printf("]\n");
	}
	if (pmbus_read_byte(chip, PMBUS_STATUS_INPUT, &b) == 0) {
		printf("  STATUS_INPUT   (7Ch) = 0x%02x  [", b);
		pmbus_print_status_bits(PMBUS_STATUS_INPUT, b,
					pmbus_status_input_bits, ovr);
		printf("]\n");
	}
	if (pmbus_read_byte(chip, PMBUS_STATUS_TEMPERATURE, &b) == 0) {
		printf("  STATUS_TEMP    (7Dh) = 0x%02x  [", b);
		pmbus_print_status_bits(PMBUS_STATUS_TEMPERATURE, b,
					pmbus_status_temp_bits, ovr);
		printf("]\n");
	}
	if (pmbus_read_byte(chip, PMBUS_STATUS_CML, &b) == 0) {
		printf("  STATUS_CML     (7Eh) = 0x%02x  [", b);
		pmbus_print_status_bits(PMBUS_STATUS_CML, b,
					pmbus_status_cml_bits, ovr);
		printf("]\n");
	}

	return CMD_RET_SUCCESS;
}

static int do_dump(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[])
{
	const struct pmbus_active_dev *act;
	struct udevice *chip;
	unsigned int i;
	int rc;

	rc = require_active(&chip, &act);
	if (rc)
		return rc;

	printf("pmbus dump @ i2c%d:0x%02x (registers known to <pmbus.h>)\n",
	       act->bus_seq, act->addr);
	for (i = 0; i < ARRAY_SIZE(pmbus_reg_syms); i++) {
		u8 reg = pmbus_reg_syms[i].reg;
		u8 b = 0;
		u16 w = 0;

		switch (reg) {
		case PMBUS_PAGE:
		case PMBUS_OPERATION:
		case PMBUS_ON_OFF_CONFIG:
		case PMBUS_WRITE_PROTECT:
		case PMBUS_CAPABILITY:
		case PMBUS_VOUT_MODE:
		case PMBUS_STATUS_BYTE:
		case PMBUS_STATUS_VOUT:
		case PMBUS_STATUS_IOUT:
		case PMBUS_STATUS_INPUT:
		case PMBUS_STATUS_TEMPERATURE:
		case PMBUS_STATUS_CML:
		case PMBUS_REVISION:
			if (pmbus_read_byte(chip, reg, &b) == 0)
				printf("  %02xh  %-15s b=0x%02x\n",
				       reg, pmbus_reg_syms[i].name, b);
			break;
		case PMBUS_MFR_ID:
		case PMBUS_MFR_MODEL:
		case PMBUS_MFR_REVISION: {
			char s[PMBUS_MFR_STRING_MAX];

			if (pmbus_read_string(chip, reg, s, sizeof(s),
					      act->mfr_reverse) >= 0)
				printf("  %02xh  %-15s s=\"%s\"\n",
				       reg, pmbus_reg_syms[i].name, s);
			break;
		}
		default:
			if (pmbus_read_word(chip, reg, &w) == 0)
				printf("  %02xh  %-15s w=0x%04x\n",
				       reg, pmbus_reg_syms[i].name, w);
			break;
		}
	}
	return CMD_RET_SUCCESS;
}

static int do_read(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[])
{
	const struct pmbus_active_dev *act;
	struct udevice *chip;
	const char *fmt = "b";
	u8 reg, b;
	u16 w;
	char s[PMBUS_MFR_STRING_MAX];
	int rc, ret;

	if (argc < 2)
		return CMD_RET_USAGE;
	rc = require_active(&chip, &act);
	if (rc)
		return rc;
	if (parse_reg(argv[1], &reg) < 0) {
		printf("pmbus: invalid register '%s'\n", argv[1]);
		return CMD_RET_USAGE;
	}
	if (argc >= 3)
		fmt = argv[2];

	if (!strcmp(fmt, "b")) {
		ret = pmbus_read_byte(chip, reg, &b);
		if (ret) {
			printf("pmbus: read byte 0x%02x failed (%d)\n", reg, ret);
			return CMD_RET_FAILURE;
		}
		printf("  %02xh  %-15s b=0x%02x\n", reg, pmbus_reg_name(reg), b);
	} else if (!strcmp(fmt, "w")) {
		ret = pmbus_read_word(chip, reg, &w);
		if (ret) {
			printf("pmbus: read word 0x%02x failed (%d)\n", reg, ret);
			return CMD_RET_FAILURE;
		}
		printf("  %02xh  %-15s w=0x%04x\n", reg, pmbus_reg_name(reg), w);
	} else if (!strcmp(fmt, "s")) {
		ret = pmbus_read_string(chip, reg, s, sizeof(s), false);
		if (ret < 0) {
			printf("pmbus: read string 0x%02x failed (%d)\n", reg, ret);
			return CMD_RET_FAILURE;
		}
		printf("  %02xh  %-15s s=\"%s\"\n", reg, pmbus_reg_name(reg), s);
	} else {
		printf("pmbus: unknown format '%s' (expected b, w, or s)\n", fmt);
		return CMD_RET_USAGE;
	}
	return CMD_RET_SUCCESS;
}

static int do_write(struct cmd_tbl *cmdtp, int flag, int argc,
		    char *const argv[])
{
	const struct pmbus_active_dev *act;
	struct udevice *chip;
	const char *fmt = "b";
	unsigned long val;
	u8 reg, b;
	u8 buf[2];
	int rc, ret;

	if (argc < 3)
		return CMD_RET_USAGE;
	rc = require_active(&chip, &act);
	if (rc)
		return rc;
	if (parse_reg(argv[1], &reg) < 0) {
		printf("pmbus: invalid register '%s'\n", argv[1]);
		return CMD_RET_USAGE;
	}
	if (strict_strtoul(argv[2], 16, &val)) {
		printf("pmbus: invalid value '%s'\n", argv[2]);
		return CMD_RET_USAGE;
	}
	if (argc >= 4)
		fmt = argv[3];

	if (!strcmp(fmt, "b")) {
		if (val > 0xff) {
			printf("pmbus: byte value out of range\n");
			return CMD_RET_USAGE;
		}
		b = (u8)val;
		ret = dm_i2c_write(chip, reg, &b, 1);
	} else if (!strcmp(fmt, "w")) {
		if (val > 0xffff) {
			printf("pmbus: word value out of range\n");
			return CMD_RET_USAGE;
		}
		buf[0] = (u8)(val & 0xff);
		buf[1] = (u8)((val >> 8) & 0xff);
		ret = dm_i2c_write(chip, reg, buf, 2);
	} else {
		printf("pmbus: unknown format '%s' (expected b or w)\n", fmt);
		return CMD_RET_USAGE;
	}
	if (ret) {
		printf("pmbus: write 0x%02x failed (%d)\n", reg, ret);
		return CMD_RET_FAILURE;
	}
	printf("pmbus: wrote 0x%lx to %02xh (%s)\n", val, reg, pmbus_reg_name(reg));
	return CMD_RET_SUCCESS;
}

static int do_clear(struct cmd_tbl *cmdtp, int flag, int argc,
		    char *const argv[])
{
	const struct pmbus_active_dev *act;
	struct udevice *chip;
	int rc, ret;

	if (argc >= 2 && strcmp(argv[1], "faults") != 0) {
		printf("pmbus: unknown clear subcommand '%s' (expected 'faults')\n",
		       argv[1]);
		return CMD_RET_USAGE;
	}
	rc = require_active(&chip, &act);
	if (rc)
		return rc;
	ret = pmbus_clear_faults(chip);
	if (ret) {
		printf("pmbus: CLEAR_FAULTS (03h) failed (%d)\n", ret);
		return CMD_RET_FAILURE;
	}
	printf("pmbus: CLEAR_FAULTS (03h) issued (RAM sticky STATUS_* cleared)\n");
	return CMD_RET_SUCCESS;
}

static int do_vout(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[])
{
	const struct pmbus_active_dev *act;
	struct udevice *chip;
	u8 vout_mode = 0;
	u16 raw;
	s64 uv;
	int rc, ret;

	rc = require_active(&chip, &act);
	if (rc)
		return rc;

	if (pmbus_read_byte(chip, PMBUS_VOUT_MODE, &vout_mode)) {
		printf("pmbus: VOUT_MODE read failed\n");
		return CMD_RET_FAILURE;
	}

	if (argc < 2) {
		if (pmbus_read_word(chip, PMBUS_READ_VOUT, &raw)) {
			printf("pmbus: READ_VOUT failed\n");
			return CMD_RET_FAILURE;
		}
		if (act->info)
			uv = pmbus_reg2data(act->info, PSC_VOLTAGE_OUT, raw, vout_mode);
		else
			uv = pmbus_reg2data_linear16(raw, vout_mode);
		printf("pmbus VOUT @ i2c%d:0x%02x  raw=0x%04x  ",
		       act->bus_seq, act->addr, raw);
		print_micro(uv, "V");
		printf("\n");
		return CMD_RET_SUCCESS;
	}

	{
		unsigned long target_uv;
		const char *fmt_name;
		u8 buf[2];

		if (strict_strtoul(argv[1], 10, &target_uv)) {
			printf("pmbus: invalid microvolt value '%s'\n", argv[1]);
			return CMD_RET_USAGE;
		}

		switch (vout_mode & PB_VOUT_MODE_MODE_MASK) {
		case PB_VOUT_MODE_LINEAR:
			raw = pmbus_data2reg_linear16((s64)target_uv, vout_mode);
			fmt_name = "LINEAR16";
			break;
		case PB_VOUT_MODE_DIRECT:
			if (!act->info ||
			    act->info->format[PSC_VOLTAGE_OUT] != pmbus_fmt_direct) {
				printf("pmbus: VOUT_MODE selects DIRECT but the active driver_info has no DIRECT coefficients for VOLTAGE_OUT\n");
				return CMD_RET_FAILURE;
			}
			raw = pmbus_data2reg_direct((s64)target_uv,
						    act->info->m[PSC_VOLTAGE_OUT],
						    act->info->b[PSC_VOLTAGE_OUT],
						    act->info->R[PSC_VOLTAGE_OUT]);
			fmt_name = "DIRECT";
			break;
		default:
			printf("pmbus: VOUT_MODE 0x%02x selects an encoder not yet implemented (VID / IEEE754)\n",
			       vout_mode);
			return CMD_RET_FAILURE;
		}

		buf[0] = raw & 0xff;
		buf[1] = (raw >> 8) & 0xff;
		ret = dm_i2c_write(chip, PMBUS_VOUT_COMMAND, buf, 2);
		if (ret) {
			printf("pmbus: VOUT_COMMAND write failed (%d)\n", ret);
			return CMD_RET_FAILURE;
		}
		printf("pmbus: VOUT_COMMAND <- 0x%04x (%s, target %lu uV)\n",
		       raw, fmt_name, target_uv);
	}
	return CMD_RET_SUCCESS;
}

static int pmbus_scan_one_bus(struct udevice *bus, int bus_seq)
{
	int hits = 0;
	int addr;

	for (addr = 0x08; addr <= 0x77; addr++) {
		struct udevice *chip;
		u8 b;

		if (i2c_get_chip(bus, addr, 1, &chip))
			continue;
		/* MFR_ID block read: 1st byte is the length of the string. */
		if (dm_i2c_read(chip, PMBUS_MFR_ID, &b, 1))
			continue;
		if (b >= 1 && b <= PMBUS_MFR_STRING_MAX - 1) {
			char s[PMBUS_MFR_STRING_MAX] = "";

			pmbus_read_string(chip, PMBUS_MFR_ID, s, sizeof(s), false);
			if (!s[0])
				pmbus_read_string(chip, PMBUS_MFR_ID, s, sizeof(s), true);
			printf("  i2c%d:0x%02x  MFR_ID=\"%s\"\n",
			       bus_seq, addr, s[0] ? s : "(unprintable)");
			hits++;
		}
	}
	return hits;
}

static int do_scan(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[])
{
	int total = 0;

	if (argc >= 2) {
		struct udevice *bus;
		unsigned long val;
		int bus_seq;

		if (strict_strtoul(argv[1], 10, &val)) {
			printf("pmbus: invalid bus seq '%s'\n", argv[1]);
			return CMD_RET_USAGE;
		}
		bus_seq = (int)val;
		if (uclass_get_device_by_seq(UCLASS_I2C, bus_seq, &bus)) {
			printf("pmbus: i2c%d not available\n", bus_seq);
			return CMD_RET_FAILURE;
		}
		printf("pmbus scan i2c%d:\n", bus_seq);
		total = pmbus_scan_one_bus(bus, bus_seq);
	} else {
		struct uclass *uc;
		struct udevice *bus;

		if (uclass_get(UCLASS_I2C, &uc))
			return CMD_RET_FAILURE;
		uclass_foreach_dev(bus, uc) {
			int seq = dev_seq(bus);

			if (seq < 0)
				continue;
			printf("pmbus scan i2c%d:\n", seq);
			total += pmbus_scan_one_bus(bus, seq);
		}
	}
	if (!total)
		printf("pmbus: no PMBus responders found\n");
	return CMD_RET_SUCCESS;
}

static int do_help(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[])
{
	unsigned int i, n = pmbus_vendor_count();

	if (n == 0) {
		printf("pmbus: no vendor extensions registered.\n");
		printf("       Vendor handlers are registered by per chip drivers at\n");
		printf("       probe time; trigger a probe via 'pmbus dev <name>' or a\n");
		printf("       board hook (boot snapshot) and re run 'pmbus help'.\n");
		return CMD_RET_SUCCESS;
	}

	printf("Registered pmbus vendor extensions (%u):\n\n", n);
	for (i = 0; i < n; i++) {
		const struct pmbus_vendor_op *op = pmbus_vendor_at(i);

		if (!op)
			continue;
		printf("[vendor: %s]\n", op->vendor);
		if (op->help)
			printf("%s", op->help);
		printf("\n");
	}
	return CMD_RET_SUCCESS;
}

static struct cmd_tbl pmbus_subcmd[] = {
	U_BOOT_CMD_MKENT(dev,       2, 1, do_dev,       "", ""),
	U_BOOT_CMD_MKENT(list,      1, 1, do_list,      "", ""),
	U_BOOT_CMD_MKENT(info,      1, 1, do_info,      "", ""),
	U_BOOT_CMD_MKENT(telemetry, 1, 1, do_telemetry, "", ""),
	U_BOOT_CMD_MKENT(status,    1, 1, do_status,    "", ""),
	U_BOOT_CMD_MKENT(dump,      1, 1, do_dump,      "", ""),
	U_BOOT_CMD_MKENT(read,      3, 1, do_read,      "", ""),
	U_BOOT_CMD_MKENT(write,     4, 1, do_write,     "", ""),
	U_BOOT_CMD_MKENT(clear,     2, 1, do_clear,     "", ""),
	U_BOOT_CMD_MKENT(vout,      2, 1, do_vout,      "", ""),
	U_BOOT_CMD_MKENT(scan,      2, 1, do_scan,      "", ""),
	U_BOOT_CMD_MKENT(help,      1, 1, do_help,      "", ""),
};

static int do_pmbus(struct cmd_tbl *cmdtp, int flag, int argc,
		    char *const argv[])
{
	const struct pmbus_vendor_op *vop;
	struct cmd_tbl *cmd;

	if (argc < 2)
		return CMD_RET_USAGE;

	argc--;
	argv++;

	cmd = find_cmd_tbl(argv[0], pmbus_subcmd, ARRAY_SIZE(pmbus_subcmd));
	if (cmd) {
		if (argc > cmd->maxargs)
			return CMD_RET_USAGE;
		return cmd->cmd(cmdtp, flag, argc, argv);
	}

	/* Vendor namespace dispatch */
	vop = pmbus_lookup_vendor(argv[0]);
	if (vop)
		return vop->handler(cmdtp, flag, argc, argv);

	printf("pmbus: unknown subcommand '%s'\n", argv[0]);
	return CMD_RET_USAGE;
}

U_BOOT_CMD(pmbus, CONFIG_SYS_MAXARGS, 1, do_pmbus,
	   "PMBus 1.x device interrogation and control",
	   "list                     - list UCLASS_REGULATOR devices (DM bound)\n"
	   "pmbus dev [<bus>:<addr>|<name>] - show / select active PMBus device\n"
	   "                                 (<bus> decimal, <addr>/<reg>/<val> hex)\n"
	   "pmbus info                     - identification banner + driver_info\n"
	   "pmbus telemetry                - decoded VIN, VOUT, IIN, IOUT, TEMP\n"
	   "pmbus status                   - decode every STATUS_* register\n"
	   "pmbus dump                     - hex dump of every standard register\n"
	   "pmbus read <reg> [b|w|s]       - raw read (b=byte, w=word, s=string)\n"
	   "pmbus write <reg> <val> [b|w]  - raw write\n"
	   "pmbus clear [faults]           - issue CLEAR_FAULTS (03h)\n"
	   "pmbus vout [<uV>]              - read / set VOUT_COMMAND (microvolts)\n"
	   "pmbus scan [<bus>]             - PMBus aware probe of one or all I2C buses\n"
	   "pmbus help                     - list registered vendor extensions\n"
	   "\n"
	   "Vendor extensions (pmbus <vendor> ...) are registered by per chip\n"
	   "drivers at probe time. Run 'pmbus help' after a chip is probed to\n"
	   "see the available subcommands.\n"
);

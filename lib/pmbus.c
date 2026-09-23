// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2026 Free Mobile, Vincent Jardin
 *
 * PMBus 1.x decoders, transport helpers, and standard status bit
 * tables for U-Boot. See include/pmbus.h for the API surface and
 * doc/develop/pmbus.rst for the porting guide.
 *
 * Decoder math is implemented from the PMBus 1.3 specification:
 *   1. Part I  (transport): see
 *      doc/PMBus/PMBus_Specification_Rev_1_3_1_Part_I_20150313.{pdf,txt}
 *   2. Part II (commands):  see
 *      doc/PMBus/PMBus_Specification_Rev_1_3_1_Part_II_20150313.{pdf,txt}
 *
 * Reference Linux implementation: linux/drivers/hwmon/pmbus/pmbus_core.c
 * (the kernel side `struct pmbus_data` caching and hwmon publication
 * layers do not apply; only the arithmetic carries over).
 *
 * No code in this file may reference a specific board, SoC, or
 * product. Per chip quirks (MPS DIRECT format LSBs, vendor registers,
 * VID coercion, ADDR_VBOOT auto promotion, and the like) belong in
 * per chip drivers under drivers/power/regulator/ or in board local
 * files under board/<vendor>/<board>/.
 */

#include <ctype.h>
#include <dm.h>
#include <i2c.h>
#include <log.h>
#include <pmbus.h>
#include <limits.h>
#include <linux/bitops.h>
#include <power/regulator.h>

static int pmbus_sign_extend(unsigned int v, unsigned int width)
{
	unsigned int mask = (1U << width) - 1U;
	unsigned int sign = 1U << (width - 1U);

	v &= mask;
	if (v & sign)
		v |= ~mask;
	return (int)v;
}

s64 pmbus_reg2data_linear11(u16 raw)
{
	int mantissa = pmbus_sign_extend(raw & PB_LINEAR11_MANT_MASK,
					 PB_LINEAR11_MANT_BITS);
	int exponent = pmbus_sign_extend((raw >> PB_LINEAR11_EXP_SHIFT) &
					 PB_LINEAR11_EXP_MASK,
					 PB_LINEAR11_EXP_BITS);
	s64 micro;

	/* Engineering value = mantissa * 2^exponent, scaled to micro units. */
	micro = (s64)mantissa * 1000000LL;
	if (exponent >= 0)
		micro <<= exponent;
	else
		micro >>= -exponent;
	return micro;
}

s64 pmbus_reg2data_linear16(u16 raw, u8 vout_mode)
{
	int exponent;
	s64 micro;

	/*
	 * VOUT_MODE bits[7:5] = mode; bits[4:0] = parameter. Linear mode
	 * (000) treats bits[4:0] as the signed 5-bit exponent. For other
	 * modes the caller must dispatch elsewhere.
	 */
	if ((vout_mode & PB_VOUT_MODE_MODE_MASK) != PB_VOUT_MODE_LINEAR)
		return 0;

	exponent = pmbus_sign_extend(vout_mode & PB_VOUT_MODE_PARAM_MASK, 5);

	/* Mantissa is unsigned 16-bit; scale to micro units. */
	micro = (s64)raw * 1000000LL;
	if (exponent >= 0)
		micro <<= exponent;
	else
		micro >>= -exponent;
	return micro;
}

s64 pmbus_reg2data_direct(s16 raw, int m, int b, int R)
{
	s64 acc;

	if (m == 0)
		return 0;

	/*
	 * PMBus Part II sec 8.4:  Y = (1/m) * (X * 10^-R - b)
	 *
	 * Pre scale acc to micro units so the final integer division by
	 * m absorbs the rounding loss into the least significant micro
	 * digit rather than into a coarser place.
	 */
	acc = (s64)raw * 1000000LL;

	/* Apply 10^-R: positive R means divide; negative R means multiply. */
	while (R > 0) {
		acc /= 10;
		R--;
	}
	while (R < 0) {
		acc *= 10;
		R++;
	}

	/* Subtract the offset b, also in micro units. */
	acc -= (s64)b * 1000000LL;

	/* Final: divide by m. */
	acc /= m;
	return acc;
}

u16 pmbus_data2reg_linear16(s64 micro, u8 vout_mode)
{
	int exponent;
	s64 raw;

	if ((vout_mode & PB_VOUT_MODE_MODE_MASK) != PB_VOUT_MODE_LINEAR)
		return 0;

	exponent = pmbus_sign_extend(vout_mode & PB_VOUT_MODE_PARAM_MASK, 5);

	/* raw = micro / (2^exponent * 10^6). */
	raw = micro;
	if (exponent >= 0)
		raw >>= exponent;
	else
		raw <<= -exponent;
	raw /= 1000000LL;

	if (raw < 0)
		raw = 0;
	if (raw > U16_MAX)
		raw = U16_MAX;
	return (u16)raw;
}

u16 pmbus_data2reg_direct(s64 micro, int m, int b, int R)
{
	s64 acc;

	if (m == 0)
		return 0;

	/*
	 * Inverse of pmbus_reg2data_direct():  X = (m * Y + b) * 10^R.
	 * Work in micro units throughout: acc = m * Y_micro + b * 10^6,
	 * then scale by 10^R, finally divide by 10^6 to get the raw
	 * chip count. Order chosen to match the decoder's quantisation
	 * pattern so a round trip (data2reg then reg2data) returns the
	 * input within +/- one LSB.
	 */
	acc = (s64)m * micro + (s64)b * 1000000LL;

	while (R > 0) {
		acc *= 10;
		R--;
	}
	while (R < 0) {
		acc /= 10;
		R++;
	}

	acc /= 1000000LL;

	/* PMBus 1.3 Part II sec 8.4 mandates a signed 16 bit raw value. */
	if (acc > S16_MAX)
		acc = S16_MAX;
	if (acc < S16_MIN)
		acc = S16_MIN;
	return (u16)(s16)acc;
}

s64 pmbus_reg2data(const struct pmbus_driver_info *info,
		   enum pmbus_sensor_classes class,
		   u16 raw, u8 vout_mode)
{
	if (!info || class >= PSC_NUM_CLASSES)
		return 0;

	switch (info->format[class]) {
	case pmbus_fmt_linear:
		if (class == PSC_VOLTAGE_OUT)
			return pmbus_reg2data_linear16(raw, vout_mode);
		return pmbus_reg2data_linear11(raw);

	case pmbus_fmt_direct:
		return pmbus_reg2data_direct((s16)raw,
					     info->m[class],
					     info->b[class],
					     info->R[class]);

	case pmbus_fmt_vid:
	case pmbus_fmt_ieee754:
		/*
		 * Not yet wired up. Add when a consumer lands. VID needs
		 * the per page vrm_version table from the kernel's
		 * pmbus_reg2data_vid(); IEEE754 needs the half precision
		 * decoder from pmbus_reg2data_ieee754().
		 */
		return 0;
	}

	return 0;
}

int pmbus_read_byte(struct udevice *dev, u8 cmd, u8 *val)
{
	return dm_i2c_read(dev, cmd, val, 1);
}

int pmbus_read_word(struct udevice *dev, u8 cmd, u16 *val)
{
	u8 raw[2];
	int ret;

	ret = dm_i2c_read(dev, cmd, raw, 2);
	if (ret)
		return ret;
	*val = (u16)raw[0] | ((u16)raw[1] << 8);
	return 0;
}

int pmbus_write_byte(struct udevice *dev, u8 cmd, u8 val)
{
	return dm_i2c_write(dev, cmd, &val, 1);
}

int pmbus_write_word(struct udevice *dev, u8 cmd, u16 val)
{
	u8 raw[2];

	raw[0] = (u8)(val & 0xff);	/* PMBus words are little-endian */
	raw[1] = (u8)(val >> 8);
	return dm_i2c_write(dev, cmd, raw, 2);
}

int pmbus_read_string(struct udevice *dev, u8 cmd, char *out, int outsz,
		      bool reverse_bytes)
{
	u8 raw[PMBUS_MFR_STRING_MAX + 1];	/* length byte + payload */
	int ret, len, i;

	if (outsz < 2)
		return -EINVAL;

	/* Stage 1: read the length byte. */
	ret = dm_i2c_read(dev, cmd, raw, 1);
	if (ret)
		return ret;

	len = raw[0];
	if (len <= 0 || len > (int)sizeof(raw) - 1)
		return -EBADMSG;
	if (len > outsz - 1)
		len = outsz - 1;

	/* Stage 2: reread length + payload (some controllers mandate this). */
	ret = dm_i2c_read(dev, cmd, raw, len + 1);
	if (ret)
		return ret;

	if (reverse_bytes) {
		for (i = 0; i < len; i++) {
			u8 b = raw[len - i];

			out[i] = isprint(b) ? (char)b : '.';
		}
	} else {
		for (i = 0; i < len; i++) {
			u8 b = raw[i + 1];

			out[i] = isprint(b) ? (char)b : '.';
		}
	}
	out[len] = '\0';
	return len;
}

int pmbus_clear_faults(struct udevice *dev)
{
	return dm_i2c_write(dev, PMBUS_CLEAR_FAULTS, NULL, 0);
}

void pmbus_print_bits(u16 v, const struct pmbus_bit *tab)
{
	const struct pmbus_bit *t;
	int first = 1;

	if (v == 0) {
		printf("clean");
		return;
	}
	for (t = tab; t && t->name; t++) {
		if (v & t->mask) {
			printf("%s%s", first ? "" : "|", t->name);
			first = 0;
		}
	}
}

void pmbus_print_status_bits(u8 reg, u16 v,
			     const struct pmbus_bit *std,
			     const struct pmbus_status_override *ovr)
{
	const struct pmbus_bit *s;
	const struct pmbus_status_override *o;
	int first = 1;

	if (v == 0) {
		printf("clean");
		return;
	}

	/*
	 * Pass 1: walk the standard table in declared order so the
	 * printout retains the conventional bit-15-first ordering. For
	 * each set bit, prefer a chip override matching (reg, mask).
	 */
	for (s = std; s && s->name; s++) {
		const char *name;

		if (!(v & s->mask))
			continue;
		name = s->name;
		for (o = ovr; o && o->name; o++) {
			if (o->reg == reg && o->mask == s->mask) {
				name = o->name;
				break;
			}
		}
		printf("%s%s", first ? "" : "|", name);
		first = 0;
	}

	/*
	 * Pass 2: print overrides whose mask is not in the standard
	 * table at all (chip-specific bit at a position the spec
	 * leaves RESERVED). These would otherwise be swallowed.
	 */
	for (o = ovr; o && o->name; o++) {
		bool in_std = false;

		if (o->reg != reg)
			continue;
		if (!(v & o->mask))
			continue;
		for (s = std; s && s->name; s++) {
			if (s->mask == o->mask) {
				in_std = true;
				break;
			}
		}
		if (in_std)
			continue;
		printf("%s%s", first ? "" : "|", o->name);
		first = 0;
	}
}

/*
 * Standard PMBus 1.3 status bit tables. Per-chip drivers may publish
 * their own tables for vendor extended bits (e.g. NVM summary bits,
 * DR MOS faults) but the standard layout below is the safe baseline.
 *
 * All tables are NULL terminated (`name = NULL` sentinel), matching
 * the convention used elsewhere in U-Boot for driver tables.
 */
const struct pmbus_bit pmbus_status_word_bits[] = {
	{ PB_STATUS_VOUT,        "VOUT" },
	{ PB_STATUS_IOUT_POUT,   "IOUT_POUT" },
	{ PB_STATUS_INPUT,       "INPUT" },
	{ PB_STATUS_WORD_MFR,    "MFR" },
	{ PB_STATUS_POWER_GOOD_N, "PG#" },
	{ PB_STATUS_FANS,        "FANS" },
	{ PB_STATUS_OTHER,       "OTHER" },
	{ PB_STATUS_UNKNOWN,     "UNKNOWN" },
	{ PB_STATUS_BUSY,        "BUSY" },
	{ PB_STATUS_OFF,         "OFF" },
	{ PB_STATUS_VOUT_OV,     "VOUT_OV" },
	{ PB_STATUS_IOUT_OC,     "IOUT_OC" },
	{ PB_STATUS_VIN_UV,      "VIN_UV" },
	{ PB_STATUS_TEMPERATURE, "TEMP" },
	{ PB_STATUS_CML,         "CML" },
	{ PB_STATUS_NONE_ABOVE,  "NONE_ABOVE" },
	{ /* sentinel */ }
};

const struct pmbus_bit pmbus_status_vout_bits[] = {
	{ PB_VOLTAGE_OV_FAULT,         "VOUT_OV_FAULT" },
	{ PB_VOLTAGE_OV_WARNING,       "VOUT_OV_WARN" },
	{ PB_VOLTAGE_UV_WARNING,       "VOUT_UV_WARN" },
	{ PB_VOLTAGE_UV_FAULT,         "VOUT_UV_FAULT" },
	{ PB_VOLTAGE_VOUT_MAX_MIN_WARN, "VOUT_MAX_MIN_WARN" },
	{ /* sentinel */ }
};

const struct pmbus_bit pmbus_status_iout_bits[] = {
	{ PB_IOUT_OC_FAULT,    "IOUT_OC_FAULT" },
	{ PB_IOUT_OC_LV_FAULT, "IOUT_OC_LV_FAULT" },
	{ PB_IOUT_OC_WARNING,  "IOUT_OC_WARN" },
	{ PB_IOUT_UC_FAULT,    "IOUT_UC_FAULT" },
	{ PB_CURRENT_SHARE_FAULT, "ISHARE_FAULT" },
	{ PB_POWER_LIMITING,   "POWER_LIMITING" },
	{ PB_POUT_OP_FAULT,    "POUT_OP_FAULT" },
	{ PB_POUT_OP_WARNING,  "POUT_OP_WARN" },
	{ /* sentinel */ }
};

const struct pmbus_bit pmbus_status_input_bits[] = {
	{ PB_IIN_OC_FAULT,     "IIN_OC_FAULT" },
	{ PB_IIN_OC_WARNING,   "IIN_OC_WARN" },
	{ PB_PIN_OP_WARNING,   "PIN_OP_WARN" },
	{ /* sentinel */ }
};

const struct pmbus_bit pmbus_status_temp_bits[] = {
	{ PB_TEMP_OT_FAULT,    "OT_FAULT" },
	{ PB_TEMP_OT_WARNING,  "OT_WARN" },
	{ PB_TEMP_UT_WARNING,  "UT_WARN" },
	{ PB_TEMP_UT_FAULT,    "UT_FAULT" },
	{ /* sentinel */ }
};

const struct pmbus_bit pmbus_status_cml_bits[] = {
	{ PB_CML_FAULT_INVALID_COMMAND, "INVALID_CMD" },
	{ PB_CML_FAULT_INVALID_DATA,    "INVALID_DATA" },
	{ PB_CML_FAULT_PACKET_ERROR,    "PEC" },
	{ PB_CML_FAULT_MEMORY,          "MEM" },
	{ PB_CML_FAULT_PROCESSOR,       "PROC" },
	{ PB_CML_FAULT_OTHER_COMM,      "OTHER_COMM" },
	{ PB_CML_FAULT_OTHER_MEM_LOGIC, "OTHER_MEM_LOGIC" },
	{ /* sentinel */ }
};

/*
 * Active device tracking + chip / vendor registries (consumed by the
 * `pmbus` U-Boot CLI command in cmd/pmbus.c).
 */

#define PMBUS_MAX_CHIP_MATCHES		8
#define PMBUS_MAX_VENDOR_HANDLERS	4

static struct pmbus_active_dev pmbus_active_state;
static const struct pmbus_chip_match *pmbus_chip_table[PMBUS_MAX_CHIP_MATCHES];
static unsigned int pmbus_chip_table_n;
static const struct pmbus_vendor_op *pmbus_vendor_table[PMBUS_MAX_VENDOR_HANDLERS];
static unsigned int pmbus_vendor_table_n;

const struct pmbus_active_dev *pmbus_active(void)
{
	return pmbus_active_state.valid ? &pmbus_active_state : NULL;
}

void pmbus_clear_active(void)
{
	memset(&pmbus_active_state, 0, sizeof(pmbus_active_state));
}

int pmbus_active_get_i2c(struct udevice **i2c_dev)
{
	struct udevice *bus;
	int ret;

	if (!pmbus_active_state.valid)
		return -ENODEV;
	ret = uclass_get_device_by_seq(UCLASS_I2C, pmbus_active_state.bus_seq, &bus);
	if (ret)
		return ret;
	return i2c_get_chip(bus, pmbus_active_state.addr, 1, i2c_dev);
}

/* engineering value in micro units -> "I.FFF<unit>" (3 fractional digits) */
static void pmbus_emit_micro(s64 micro, const char *unit)
{
	s64 abs_milli = (micro < 0 ? -micro : micro) / 1000LL;

	printf("%lld.%03lld%s", (long long)(micro / 1000000LL),
	       (long long)(abs_milli % 1000LL), unit);
}

struct pmbus_telem_entry {
	u8 reg;
	const char *label;
	enum pmbus_sensor_classes class;
	const char *unit;
};

/*
 * Telemetry register set, in print order. POUT is included so PSU
 * class parts report input/output power; chips that do not implement
 * a given command are skipped via pmbus_word_command_supported().
 */
static const struct pmbus_telem_entry pmbus_telem_table[] = {
	{ PMBUS_READ_VIN,           "VIN ", PSC_VOLTAGE_IN,  "V" },
	{ PMBUS_READ_VOUT,          "VOUT", PSC_VOLTAGE_OUT, "V" },
	{ PMBUS_READ_IIN,           "IIN ", PSC_CURRENT_IN,  "A" },
	{ PMBUS_READ_IOUT,          "IOUT", PSC_CURRENT_OUT, "A" },
	{ PMBUS_READ_POUT,          "POUT", PSC_POWER,       "W" },
	{ PMBUS_READ_TEMPERATURE_1, "TEMP", PSC_TEMPERATURE, "C" },
};

bool pmbus_word_command_supported(struct udevice *dev, u8 reg)
{
	u8 cml_before = 0, cml_after = 0;
	bool have_cml;
	u16 w;

	have_cml = !pmbus_read_byte(dev, PMBUS_STATUS_CML, &cml_before);

	if (pmbus_read_word(dev, reg, &w))
		return false;	/* NAK: unsupported command not ACKed */

	if (have_cml && !(cml_before & PB_CML_FAULT_INVALID_COMMAND) &&
	    !pmbus_read_byte(dev, PMBUS_STATUS_CML, &cml_after) &&
	    (cml_after & PB_CML_FAULT_INVALID_COMMAND))
		return false;	/* ACKed but chip raised INVALID_COMMAND */

	return true;
}

/* Telemetry of the currently selected page. */
static void pmbus_print_telemetry_page(struct udevice *chip,
				       const struct pmbus_active_dev *act)
{
	u8 vout_mode = 0;
	unsigned int i;

	/*
	 * On a read failure vout_mode stays 0 (LINEAR, exponent 0). That is
	 * a silent mis-scale of every VOLTAGE_OUT reading, so make the
	 * fallback visible rather than printing a wrong voltage as if good.
	 */
	if (pmbus_read_byte(chip, PMBUS_VOUT_MODE, &vout_mode))
		printf("  (VOUT_MODE read failed; VOUT decode assumes LINEAR exp 0)\n");

	for (i = 0; i < ARRAY_SIZE(pmbus_telem_table); i++) {
		const struct pmbus_telem_entry *e = &pmbus_telem_table[i];
		u16 raw = 0;

		/*
		 * Class gating. A chip driver that declares classes_present
		 * lists exactly the sensors it implements (kernel-style
		 * per-chip sensor set), so unlisted classes are skipped
		 * silently -- this is what hides the MPS buck's uncalibrated
		 * POUT / IIN. A generic / undeclared device instead gets a
		 * live capability probe per class.
		 */
		if (act->info && act->info->classes_present) {
			if (!(act->info->classes_present & BIT(e->class)))
				continue;
		} else if (!pmbus_word_command_supported(chip, e->reg)) {
			printf("  %s : (not supported)\n", e->label);
			continue;
		}

		if (pmbus_read_word(chip, e->reg, &raw)) {
			printf("  %s : (read failed)\n", e->label);
			continue;
		}

		printf("  %s : raw=0x%04x  ", e->label, raw);
		if (act->info) {
			u16 dec = raw;

			/*
			 * Some DIRECT format parts (e.g. MPS) report
			 * temperature as 1 degC/LSB in the low byte only;
			 * mask there. LINEAR temperatures use all 16 bits
			 * and must NOT be masked.
			 */
			if (e->class == PSC_TEMPERATURE &&
			    act->info->format[PSC_TEMPERATURE] == pmbus_fmt_direct)
				dec = raw & 0x00ff;

			pmbus_emit_micro(pmbus_reg2data(act->info, e->class,
							dec, vout_mode),
					 e->unit);
		} else if (e->class == PSC_VOLTAGE_OUT) {
			pmbus_emit_micro(pmbus_reg2data_linear16(raw, vout_mode),
					 e->unit);
		} else {
			pmbus_emit_micro(pmbus_reg2data_linear11(raw), e->unit);
			printf(" (LINEAR11 fallback)");
		}
		printf("\n");
	}
}

void pmbus_print_telemetry(struct udevice *chip)
{
	const struct pmbus_active_dev *act = pmbus_active();
	int npages, p;
	u8 zero = 0;

	if (!act)
		return;

	/*
	 * Multi-rail parts (PSU bricks) expose one rail per PMBUS_PAGE.
	 * Chip drivers set pmbus_driver_info.pages; the generic driver
	 * takes it from the DT `pmbus,num-pages` (default 1). We always
	 * write PMBUS_PAGE before reading a page -- including page 0 --
	 * because a device may power up selected on a different page, which
	 * is what made the 48V PSU read all-zeros before. Only valid pages
	 * (0..npages-1) are ever written, so we never induce the
	 * out-of-range-PAGE STATUS_CML fault and the device's sticky fault
	 * log is left untouched (no CLEAR_FAULTS, no scrubbing).
	 */
	npages = (act->info && act->info->pages > 0) ? act->info->pages : 1;

	for (p = 0; p < npages; p++) {
		u8 pg = (u8)p;

		if (dm_i2c_write(chip, PMBUS_PAGE, &pg, 1)) {
			printf("  [page %d] PAGE select failed\n", p);
			continue;
		}
		if (npages > 1)
			printf("  [page %d]\n", p);
		pmbus_print_telemetry_page(chip, act);
	}

	if (npages > 1)
		dm_i2c_write(chip, PMBUS_PAGE, &zero, 1);	/* leave on page 0 */
}

void pmbus_print_status_word(struct udevice *chip)
{
	const struct pmbus_active_dev *act = pmbus_active();
	const struct pmbus_status_override *ovr =
		(act && act->info) ? act->info->status_overrides : NULL;
	u16 word = 0;

	if (pmbus_read_word(chip, PMBUS_STATUS_WORD, &word)) {
		printf("  STATUS_WORD    (79h) = (read failed)\n");
		return;
	}
	printf("  STATUS_WORD    (79h) = 0x%04x  [", word);
	pmbus_print_status_bits(PMBUS_STATUS_WORD, word,
				pmbus_status_word_bits, ovr);
	printf("]\n");
}

static const struct pmbus_chip_match *pmbus_match_mfr(const char *id)
{
	unsigned int i;

	if (!id || !id[0])
		return NULL;
	for (i = 0; i < pmbus_chip_table_n; i++) {
		const struct pmbus_chip_match *m = pmbus_chip_table[i];
		size_t plen = strlen(m->mfr_id);

		if (strlen(id) >= plen && !strncmp(id, m->mfr_id, plen))
			return m;
	}
	return NULL;
}

/*
 * Walk UCLASS_REGULATOR looking for a regulator whose I2C parent
 * bus seq + DT reg address match the requested (bus_seq, addr).
 * Returns the regulator-name (uclass plat .name) on hit, or NULL if
 * no UCLASS_REGULATOR device matches (chip not bound through DT, or
 * CONFIG_DM_REGULATOR disabled).
 */
static const char *pmbus_lookup_regname(int bus_seq, u8 addr)
{
	struct uclass *uc;
	struct udevice *r;

	if (!IS_ENABLED(CONFIG_DM_REGULATOR))
		return NULL;

	if (uclass_get(UCLASS_REGULATOR, &uc))
		return NULL;
	uclass_foreach_dev(r, uc) {
		struct dm_regulator_uclass_plat *up;
		struct udevice *parent = dev_get_parent(r);
		int ra;

		if (!parent || device_get_uclass_id(parent) != UCLASS_I2C)
			continue;
		if (dev_seq(parent) != bus_seq)
			continue;
		ra = dev_read_addr(r);
		if (ra < 0 || (u8)ra != addr)
			continue;
		up = dev_get_uclass_plat(r);
		if (up && up->name)
			return up->name;
		return r->name;
	}
	return NULL;
}

int pmbus_set_active(int bus_seq, u8 addr)
{
	const struct pmbus_chip_match *match = NULL;
	struct udevice *bus, *chip;
	char id_fwd[PMBUS_MFR_STRING_MAX] = "";
	char id_rev[PMBUS_MFR_STRING_MAX] = "";
	const char *rname;
	int ret;

	pmbus_clear_active();

	ret = uclass_get_device_by_seq(UCLASS_I2C, bus_seq, &bus);
	if (ret)
		return ret;
	ret = i2c_get_chip(bus, addr, 1, &chip);
	if (ret)
		return ret;

	pmbus_active_state.bus_seq = bus_seq;
	pmbus_active_state.addr = addr;

	/*
	 * Probe MFR_ID in both byte orders. Spec compliant chips return
	 * "MPS" / "TI" / etc. in the natural reading (forward); MPS NVM
	 * personalities store the string LSB first and need the reverse
	 * read. Chip table entries declare which one is canonical for
	 * the chip family they describe.
	 */
	if (pmbus_read_string(chip, PMBUS_MFR_ID, id_fwd, sizeof(id_fwd), false) < 0)
		id_fwd[0] = '\0';
	if (pmbus_read_string(chip, PMBUS_MFR_ID, id_rev, sizeof(id_rev), true) < 0)
		id_rev[0] = '\0';

	match = pmbus_match_mfr(id_fwd);
	if (match && !match->mfr_id_reverse) {
		strlcpy(pmbus_active_state.mfr_id, id_fwd,
			sizeof(pmbus_active_state.mfr_id));
	} else {
		match = pmbus_match_mfr(id_rev);
		if (match && match->mfr_id_reverse) {
			strlcpy(pmbus_active_state.mfr_id, id_rev,
				sizeof(pmbus_active_state.mfr_id));
		} else {
			/* No registered match; cache the forward read as best effort. */
			strlcpy(pmbus_active_state.mfr_id,
				id_fwd[0] ? id_fwd : id_rev,
				sizeof(pmbus_active_state.mfr_id));
		}
	}

	if (match) {
		pmbus_active_state.info = match->info;
		if (match->vendor)
			strlcpy(pmbus_active_state.vendor, match->vendor,
				sizeof(pmbus_active_state.vendor));
	}

	/*
	 * No MFR_ID chip-match (a spec compliant part with no per chip
	 * driver, e.g. a Flex / Delta PSU): if a generic / chip
	 * UCLASS_REGULATOR is bound at this address, reuse its
	 * VOUT_MODE detected driver_info so telemetry decodes through
	 * the right per class formats instead of the blanket
	 * LINEAR16 / LINEAR11 fallback.
	 */
	if (CONFIG_IS_ENABLED(DM_REGULATOR_PMBUS_HELPER) &&
	    !pmbus_active_state.info) {
		const struct pmbus_driver_info *di =
			pmbus_regulator_info_by_addr(bus_seq, addr);

		if (di)
			pmbus_active_state.info = di;
	}

	/*
	 * MFR_MODEL / MFR_REVISION are best effort. Use the same byte
	 * order the matched chip declared; if nothing matched, use the
	 * forward order.
	 */
	{
		bool reverse = match && match->mfr_id_reverse;

		pmbus_active_state.mfr_reverse = reverse;
		pmbus_read_string(chip, PMBUS_MFR_MODEL,
				  pmbus_active_state.mfr_model,
				  sizeof(pmbus_active_state.mfr_model), reverse);
		pmbus_read_string(chip, PMBUS_MFR_REVISION,
				  pmbus_active_state.mfr_revision,
				  sizeof(pmbus_active_state.mfr_revision), reverse);
	}

	rname = pmbus_lookup_regname(bus_seq, addr);
	if (rname)
		strlcpy(pmbus_active_state.name, rname,
			sizeof(pmbus_active_state.name));

	pmbus_active_state.valid = true;
	return 0;
}

int pmbus_register_chip(const struct pmbus_chip_match *match)
{
	if (!match || !match->mfr_id)
		return -EINVAL;
	if (pmbus_chip_table_n >= PMBUS_MAX_CHIP_MATCHES)
		return -ENOSPC;
	pmbus_chip_table[pmbus_chip_table_n++] = match;
	return 0;
}

int pmbus_register_vendor_handler(const struct pmbus_vendor_op *op)
{
	if (!op || !op->vendor || !op->handler)
		return -EINVAL;
	if (pmbus_vendor_table_n >= PMBUS_MAX_VENDOR_HANDLERS)
		return -ENOSPC;
	pmbus_vendor_table[pmbus_vendor_table_n++] = op;
	return 0;
}

const struct pmbus_vendor_op *pmbus_lookup_vendor(const char *vendor)
{
	unsigned int i;

	if (!vendor)
		return NULL;
	for (i = 0; i < pmbus_vendor_table_n; i++)
		if (!strcmp(pmbus_vendor_table[i]->vendor, vendor))
			return pmbus_vendor_table[i];
	return NULL;
}

unsigned int pmbus_vendor_count(void)
{
	return pmbus_vendor_table_n;
}

const struct pmbus_vendor_op *pmbus_vendor_at(unsigned int i)
{
	return i < pmbus_vendor_table_n ? pmbus_vendor_table[i] : NULL;
}

int pmbus_resolve_by_name(const char *name, int *bus_seq, u8 *addr)
{
	struct udevice *reg;
	struct udevice *parent;
	int ret;
	int a;

	if (!IS_ENABLED(CONFIG_DM_REGULATOR))
		return -ENOSYS;

	if (!name || !bus_seq || !addr)
		return -EINVAL;

	ret = regulator_get_by_platname(name, &reg);
	if (ret)
		return ret;

	parent = dev_get_parent(reg);
	if (!parent || device_get_uclass_id(parent) != UCLASS_I2C)
		return -ENODEV;

	a = dev_read_addr(reg);
	if (a < 0 || a > 0x7f)
		return -EINVAL;

	*bus_seq = dev_seq(parent);
	*addr = (u8)a;
	return 0;
}

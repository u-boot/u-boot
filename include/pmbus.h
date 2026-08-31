/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2026 Free Mobile, Vincent Jardin
 *
 * PMBus 1.x command codes, numeric format decoders, and driver_info
 * scaffolding for U-Boot.
 *
 * Intents
 *
 * U-Boot's PMBus support is not a hwmon clone. It shall be used to:
 *   1. identify PMBus regulators a board carries at boot,
 *   2. print telemetry so an operator can confirm rail voltages and
 *      fault status before handing off to the kernel,
 *   3. decode chip alerts when a rail trips an over/under voltage,
 *      over current, or thermal threshold,
 *   4. optionally trim a critical rail before kernel boot, to better
 *      protect the board.
 *
 * No periodic polling, no /sys, no userspace surface, no fan control
 * loops. Linux owns those. See doc/develop/pmbus.rst for the full
 * porting guide and policy notes.
 *
 * Linux relationship
 *
 * Constants (command codes, status bit names, sensor class enum,
 * format enum) are mirrored verbatim from
 *   linux/drivers/hwmon/pmbus/pmbus.h
 * with attribution; this is standardised data and copying it avoids
 * accidental drift. Decoders (LINEAR11/16, DIRECT, VID, IEEE754) are
 * reimplemented from the PMBus 1.x spec rather than copied. The
 * surrounding kernel context (struct pmbus_data, hwmon caching,
 * sysfs publication) does not apply to U-Boot.
 *
 * Tree level files (this header, lib/pmbus.c, future per chip drivers
 * under drivers/power/regulator/) must stay platform agnostic. They
 * may reference only the PMBus 1.x specification and chip datasheets,
 * never a specific board, SoC, or product. Board specific quirks
 * live under board/<vendor>/<board>/.
 */

#ifndef _PMBUS_H_
#define _PMBUS_H_

#include <linux/types.h>

struct udevice;
struct pmbus_driver_info;
struct pmbus_status_override;

/*
 * PMBus 1.3 standard command codes (Part II).
 *
 * Subset relevant to U-Boot's needs:
 * - configuration (PAGE, OPERATION, VOUT_*),
 * - telemetry (READ_VIN, READ_IOUT, READ_TEMPERATURE_1, ...),
 * - status (STATUS_WORD, STATUS_VOUT, ...),
 * - identification (MFR_ID, MFR_MODEL, MFR_REVISION).
 * Chip specific extensions (for example MPS PROTECTION_LAST 0xFB) shall be in
 * the per chip driver file.
 */
#define PMBUS_PAGE			0x00
#define PMBUS_OPERATION			0x01
#define PMBUS_ON_OFF_CONFIG		0x02
#define PMBUS_CLEAR_FAULTS		0x03
#define PMBUS_PHASE			0x04

#define PMBUS_WRITE_PROTECT		0x10

#define PMBUS_CAPABILITY		0x19
#define PMBUS_QUERY			0x1a
#define PMBUS_SMBALERT_MASK		0x1b
#define PMBUS_VOUT_MODE			0x20
#define PMBUS_VOUT_COMMAND		0x21
#define PMBUS_VOUT_TRIM			0x22
#define PMBUS_VOUT_CAL_OFFSET		0x23
#define PMBUS_VOUT_MAX			0x24
#define PMBUS_VOUT_MARGIN_HIGH		0x25
#define PMBUS_VOUT_MARGIN_LOW		0x26
#define PMBUS_VOUT_TRANSITION_RATE	0x27
#define PMBUS_VOUT_DROOP		0x28
#define PMBUS_VOUT_SCALE_LOOP		0x29
#define PMBUS_VOUT_SCALE_MONITOR	0x2a

#define PMBUS_COEFFICIENTS		0x30
#define PMBUS_POUT_MAX			0x31

#define PMBUS_STATUS_BYTE		0x78
#define PMBUS_STATUS_WORD		0x79
#define PMBUS_STATUS_VOUT		0x7a
#define PMBUS_STATUS_IOUT		0x7b
#define PMBUS_STATUS_INPUT		0x7c
#define PMBUS_STATUS_TEMPERATURE	0x7d
#define PMBUS_STATUS_CML		0x7e
#define PMBUS_STATUS_OTHER		0x7f
#define PMBUS_STATUS_MFR_SPECIFIC	0x80

#define PMBUS_READ_VIN			0x88
#define PMBUS_READ_IIN			0x89
#define PMBUS_READ_VCAP			0x8a
#define PMBUS_READ_VOUT			0x8b
#define PMBUS_READ_IOUT			0x8c
#define PMBUS_READ_TEMPERATURE_1	0x8d
#define PMBUS_READ_TEMPERATURE_2	0x8e
#define PMBUS_READ_TEMPERATURE_3	0x8f
#define PMBUS_READ_DUTY_CYCLE		0x94
#define PMBUS_READ_FREQUENCY		0x95
#define PMBUS_READ_POUT			0x96
#define PMBUS_READ_PIN			0x97

#define PMBUS_REVISION			0x98
#define PMBUS_MFR_ID			0x99
#define PMBUS_MFR_MODEL			0x9a
#define PMBUS_MFR_REVISION		0x9b
#define PMBUS_MFR_LOCATION		0x9c
#define PMBUS_MFR_DATE			0x9d
#define PMBUS_MFR_SERIAL		0x9e

#define PMBUS_IC_DEVICE_ID		0xad
#define PMBUS_IC_DEVICE_REV		0xae

/* VOUT_MODE upper bits: numeric format selector (Part II sec 8.3). */
#define PB_VOUT_MODE_MODE_MASK		0xe0
#define PB_VOUT_MODE_PARAM_MASK		0x1f
#define PB_VOUT_MODE_LINEAR		0x00
#define PB_VOUT_MODE_VID		0x20
#define PB_VOUT_MODE_DIRECT		0x40
#define PB_VOUT_MODE_IEEE754		0x60

/* STATUS_WORD lower byte (= STATUS_BYTE), Part II sec 10.1.1. */
#define PB_STATUS_NONE_ABOVE		BIT(0)
#define PB_STATUS_CML			BIT(1)
#define PB_STATUS_TEMPERATURE		BIT(2)
#define PB_STATUS_VIN_UV		BIT(3)
#define PB_STATUS_IOUT_OC		BIT(4)
#define PB_STATUS_VOUT_OV		BIT(5)
#define PB_STATUS_OFF			BIT(6)
#define PB_STATUS_BUSY			BIT(7)

/* STATUS_WORD upper byte. */
#define PB_STATUS_UNKNOWN		BIT(8)
#define PB_STATUS_OTHER			BIT(9)
#define PB_STATUS_FANS			BIT(10)
#define PB_STATUS_POWER_GOOD_N		BIT(11)
#define PB_STATUS_WORD_MFR		BIT(12)
#define PB_STATUS_INPUT			BIT(13)
#define PB_STATUS_IOUT_POUT		BIT(14)
#define PB_STATUS_VOUT			BIT(15)

/* STATUS_VOUT (PMBus 1.3.1 Part II sec 17.3, Table 17). */
#define PB_VOLTAGE_VOUT_MAX_MIN_WARN	BIT(3)
#define PB_VOLTAGE_UV_FAULT		BIT(4)
#define PB_VOLTAGE_UV_WARNING		BIT(5)
#define PB_VOLTAGE_OV_WARNING		BIT(6)
#define PB_VOLTAGE_OV_FAULT		BIT(7)

/* STATUS_IOUT (Part II sec 10.6). */
#define PB_POUT_OP_WARNING		BIT(0)
#define PB_POUT_OP_FAULT		BIT(1)
#define PB_POWER_LIMITING		BIT(2)
#define PB_CURRENT_SHARE_FAULT		BIT(3)
#define PB_IOUT_UC_FAULT		BIT(4)
#define PB_IOUT_OC_WARNING		BIT(5)
#define PB_IOUT_OC_LV_FAULT		BIT(6)
#define PB_IOUT_OC_FAULT		BIT(7)

/* STATUS_INPUT (Part II sec 10.7). */
#define PB_PIN_OP_WARNING		BIT(0)
#define PB_IIN_OC_WARNING		BIT(1)
#define PB_IIN_OC_FAULT			BIT(2)

/* STATUS_TEMPERATURE (Part II sec 10.8). */
#define PB_TEMP_UT_FAULT		BIT(4)
#define PB_TEMP_UT_WARNING		BIT(5)
#define PB_TEMP_OT_WARNING		BIT(6)
#define PB_TEMP_OT_FAULT		BIT(7)

/* STATUS_CML (Part II sec 10.9). */
#define PB_CML_FAULT_OTHER_MEM_LOGIC	BIT(0)
#define PB_CML_FAULT_OTHER_COMM		BIT(1)
#define PB_CML_FAULT_PROCESSOR		BIT(3)
#define PB_CML_FAULT_MEMORY		BIT(4)
#define PB_CML_FAULT_PACKET_ERROR	BIT(5)
#define PB_CML_FAULT_INVALID_DATA	BIT(6)
#define PB_CML_FAULT_INVALID_COMMAND	BIT(7)

/*
 * OPERATION (01h) command bits per PMBus 1.3 Part II sec 9.1. Bit[7]
 * is the master rail enable; the lower bits select margin high/low
 * and turn off behaviour (subset surfaced for the regulator helper).
 */
#define PB_OPERATION_ON			BIT(7)

/*
 * LINEAR11 numeric format (PMBus 1.3 Part II sec 7): 16 bit register
 * with a signed 11 bit mantissa in bits[10:0] and a signed 5 bit
 * exponent in bits[15:11]. Engineering value = mantissa * 2^exponent.
 */
#define PB_LINEAR11_MANT_MASK		0x07ff
#define PB_LINEAR11_MANT_BITS		11
#define PB_LINEAR11_EXP_SHIFT		11
#define PB_LINEAR11_EXP_MASK		0x1f
#define PB_LINEAR11_EXP_BITS		5

/*
 * Cache buffer sizes for the active device singleton + MFR_* block
 * reads. PMBus block reads return up to 32 bytes per the SMBus spec;
 * 16 covers every MFR string seen in practice on regulator class
 * chips and keeps the singleton compact.
 */
#define PMBUS_MFR_STRING_MAX		16
#define PMBUS_VENDOR_NAME_MAX		8
#define PMBUS_REGULATOR_NAME_MAX	24

/* PMBus revision identifiers reported by PMBUS_REVISION (98h). */
#define PMBUS_REV_10			0x00	/* PMBus 1.0 */
#define PMBUS_REV_11			0x11	/* PMBus 1.1 */
#define PMBUS_REV_12			0x22	/* PMBus 1.2 */
#define PMBUS_REV_13			0x33	/* PMBus 1.3 */

/*
 * Numeric formats and sensor classes.
 *
 * Mirrors linux/drivers/hwmon/pmbus/pmbus.h enum pmbus_data_format
 * and enum pmbus_sensor_classes. A chip's pmbus_driver_info wires
 * each sensor class to one format and (for DIRECT) to its m/b/R
 * coefficients.
 */
enum pmbus_data_format {
	pmbus_fmt_linear = 0,
	pmbus_fmt_ieee754,
	pmbus_fmt_direct,
	pmbus_fmt_vid,
};

enum pmbus_sensor_classes {
	PSC_VOLTAGE_IN = 0,
	PSC_VOLTAGE_OUT,
	PSC_CURRENT_IN,
	PSC_CURRENT_OUT,
	PSC_POWER,
	PSC_TEMPERATURE,
	PSC_NUM_CLASSES
};

/*
 * Per chip identification record. Each per chip driver declares one
 * of these and points the framework at it. Subset of the kernel
 * struct pmbus_driver_info: U-Boot has no per page caches, no fan
 * accessors, no virtual registers, no async sysfs publication.
 *
 *   pages         number of PAGE distinct rails the chip exposes
 *                 (1 for single rail parts).
 *   format[]      numeric format per sensor class.
 *   m/b/R[]       DIRECT format coefficients per sensor class. See
 *                 pmbus_reg2data_direct() below for the formula.
 *                 Unused for non DIRECT classes.
 *   read_byte_data, read_word_data
 *                 optional per chip register translators. Return the
 *                 standard register value on success, ENODATA to fall
 *                 through to the generic transport, any other negative
 *                 errno on bus error.
 *   identify      optional probe time hook to discover format and
 *                 page count from the chip itself (for example, the
 *                 MPQ8785 VOUT_MODE switch between LINEAR and DIRECT).
 */
struct pmbus_driver_info {
	int pages;
	enum pmbus_data_format format[PSC_NUM_CLASSES];
	int m[PSC_NUM_CLASSES];
	int b[PSC_NUM_CLASSES];
	int R[PSC_NUM_CLASSES];

	int (*read_byte_data)(struct udevice *dev, int page, int reg);
	int (*read_word_data)(struct udevice *dev, int page, int reg);
	int (*identify)(struct udevice *dev, struct pmbus_driver_info *info);

	/*
	 * Optional sparse table of chip specific STATUS_* bit name
	 * substitutions. Terminated by an entry with .name = NULL
	 * (matching the convention used by struct udevice_id and
	 * other U-Boot driver tables). NULL pointer means the chip
	 * uses only PMBus 1.x standard names. See struct
	 * pmbus_status_override and pmbus_print_status_bits().
	 */
	const struct pmbus_status_override *status_overrides;

	/*
	 * Bitmask (BIT(enum pmbus_sensor_classes)) of the sensor classes
	 * this chip actually implements. When non-zero, pmbus_print_telemetry
	 * prints exactly these classes -- mirroring the kernel's per chip
	 * sensor set -- and skips the rest. This is how an MPS buck that
	 * ACKs READ_POUT / READ_IIN with an uncalibrated value still hides
	 * POWER / CURRENT_IN (the kernel's mpq8646 driver exposes neither).
	 * Zero means "not declared": the telemetry printer falls back to a
	 * live pmbus_word_command_supported() probe per class, which is what
	 * the generic driver (compatible = "pmbus") relies on.
	 */
	u8 classes_present;
};

/*
 * Decoder helpers (raw register, returns engineering value in micro
 * units).
 *
 * All return signed micro units (uV, uA, udegC), 64 bit to avoid
 * overflow on large mantissa times exponent products. The caller
 * divides by 1000 for milli units, or by 1_000_000 for the integer
 * engineering value.
 */

/*
 * LINEAR11. Bits[15:11] = signed 5 bit exponent Y, bits[10:0] =
 * signed 11 bit mantissa N. Engineering value = N * 2^Y.
 *
 * Used by most PMBus chips for VIN, IIN, IOUT, TEMP. Some MPS
 * parts deviate (they report DIRECT format with chip specific m/b/R
 * coefficients); check the chip datasheet against PMBUS_VOUT_MODE
 * and the Linux per chip driver if porting.
 */
s64 pmbus_reg2data_linear11(u16 raw);

/*
 * LINEAR16. 16 bit unsigned mantissa multiplied by 2^Y, where Y is
 * the signed 5 bit exponent supplied via VOUT_MODE bits[4:0]. The
 * caller must read VOUT_MODE (cmd 0x20) and pass it in vout_mode.
 * Only the mode_mask bits[7:5] = 0 selector is the LINEAR16 path.
 *
 * Used for READ_VOUT (8Bh) on chips whose VOUT_MODE selects Linear.
 * Returns 0 if VOUT_MODE indicates a non Linear mode; the caller
 * is then expected to dispatch to pmbus_reg2data_direct() with the
 * appropriate per chip m/b/R, or to pmbus_reg2data_vid() / _ieee754()
 * if the chip uses those formats.
 */
s64 pmbus_reg2data_linear16(u16 raw, u8 vout_mode);

/*
 * DIRECT. PMBus 1.3 Part II sec 8.4. The chip stores a signed 16 bit
 * value X; the engineering value Y is one over m, multiplied by the
 * quantity (X scaled by ten to the power minus R, then offset by
 * minus b), with chip specific (m, b, R) coefficients.
 *
 * In symbolic form: Y = (1/m) * (X * 10**(-R) - b). The negative
 * exponent and trailing subtraction are math operators in the
 * formula, not punctuation in the prose.
 *
 * Returns micro units. Implementation order matches the Linux
 * reference (multiply before subtract, scale R then divide by m) to
 * minimise quantisation drift. m == 0 returns 0.
 */
s64 pmbus_reg2data_direct(s16 raw, int m, int b, int R);

/*
 * Encoder: engineering value (micro units) to LINEAR16 raw register
 * value. Used by pre kernel rail trim code (see board/nxp/common/vid.c)
 * to write VOUT_COMMAND from a target voltage. The exponent is
 * recovered from VOUT_MODE.
 *
 * Returns 0 if VOUT_MODE indicates a non Linear format; the caller
 * must then dispatch to pmbus_data2reg_direct() (DIRECT format) or
 * the VID / IEEE754 encoders (when those land) per the chip's actual
 * VOUT_MODE selector.
 */
u16 pmbus_data2reg_linear16(s64 micro, u8 vout_mode);

/*
 * Encoder: engineering value (micro units) to DIRECT raw register
 * value. Inverse of pmbus_reg2data_direct():
 *
 *   X = (m * Y + b) * 10^R
 *
 * with chip specific (m, b, R) coefficients (typically taken from
 * the chip's pmbus_driver_info[PSC_VOLTAGE_OUT]). m == 0 returns 0.
 *
 * The result is saturated to the s16 range mandated by the PMBus
 * 1.3 Part II sec 8.4 DIRECT encoding; out of range targets return
 * 0x7fff or 0x8000 rather than wrapping.
 */
u16 pmbus_data2reg_direct(s64 micro, int m, int b, int R);

/*
 * Dispatcher: pick the right reg2data_* helper based on the chip's
 * pmbus_driver_info[class]. vout_mode is consulted only for
 * VOLTAGE_OUT in linear format. For DIRECT, m/b/R are taken from
 * info. For VID and IEEE754 the dispatcher returns 0 (formats
 * not yet wired up; add when a consumer lands).
 */
s64 pmbus_reg2data(const struct pmbus_driver_info *info,
		   enum pmbus_sensor_classes class,
		   u16 raw, u8 vout_mode);

/*
 * Transport helpers.
 *
 * Thin wrappers over the U-Boot DM I2C primitives that handle PMBus
 * framing details (little endian word layout, two stage block read,
 * CLEAR_FAULTS pseudo command without payload).
 */

/* Read a byte register. */
int pmbus_read_byte(struct udevice *dev, u8 cmd, u8 *val);

/* Read a 16 bit register, little endian on the wire. */
int pmbus_read_word(struct udevice *dev, u8 cmd, u16 *val);

/* Write a byte register. */
int pmbus_write_byte(struct udevice *dev, u8 cmd, u8 val);

/* Write a 16 bit register, little endian on the wire. */
int pmbus_write_word(struct udevice *dev, u8 cmd, u16 val);

/*
 * Block read of a vendor string register (MFR_ID, MFR_MODEL,
 * MFR_REVISION). The first wire byte is the payload length; the
 * helper does the second read for the payload itself, so even strict
 * I2C controllers (which forbid over read on block transactions)
 * work. Output is null terminated and printable only (non printable
 * bytes are substituted with '.').
 *
 * reverse_bytes: some MPS NVM personalities store ASCII strings
 * LSB first (chip returns "SPM" for the human string "MPS"); pass
 * true to reverse on copy. Spec compliant chips pass false.
 *
 * Returns string length on success or a negative errno on bus error
 * or invalid length byte. outsz must be at least 2.
 */
int pmbus_read_string(struct udevice *dev, u8 cmd, char *out, int outsz,
		      bool reverse_bytes);

/* Issue a CLEAR_FAULTS (03h) write. Clears RAM sticky STATUS_*. */
int pmbus_clear_faults(struct udevice *dev);

/*
 * Capability probe: is a word sized command implemented by the chip?
 *
 * Primary signal is the bus NAK -- compliant parts do not ACK a
 * command they do not implement, so the word read fails. Secondary
 * signal is a clean -> dirty transition of STATUS_CML[INVALID_COMMAND]
 * across the read (a chip that ACKs but does not implement the
 * register raises it). Non destructive: never issues CLEAR_FAULTS, so
 * sticky fault history survives for a subsequent pmbus status; a
 * pre existing CML fault disables the secondary signal so it cannot
 * produce a false "unsupported".
 *
 * Returns true if the command appears supported, false otherwise.
 */
bool pmbus_word_command_supported(struct udevice *dev, u8 reg);

/*
 * High level snapshot printers shared by the pmbus CLI and board
 * boot time diagnostics. Both operate on the current pmbus_active()
 * device (select it first via pmbus_set_active()); chip is the I2C
 * handle from pmbus_active_get_i2c() / the CLI's require_active().
 *
 * pmbus_print_telemetry: decodes VIN / VOUT / IIN / IOUT / POUT / TEMP
 * through the active device's pmbus_driver_info (LINEAR / DIRECT / VID
 * per VOUT_MODE and per class format), skipping commands the chip does
 * not implement (pmbus_word_command_supported). Falls back to
 * LINEAR16 / LINEAR11 when no driver_info is cached. Caller prints the
 * header line.
 *
 * pmbus_print_status_word: reads + decodes STATUS_WORD with the active
 * device's chip specific status_overrides, if any.
 */
void pmbus_print_telemetry(struct udevice *chip);
void pmbus_print_status_word(struct udevice *chip);

/*
 * Regulator -> thermal bridge.
 *
 * Read READ_TEMPERATURE_1 (8Dh) from a UCLASS_REGULATOR device that
 * was bound by a pmbus_helper based chip driver, decode it through
 * the chip's pmbus_driver_info (so the MPS DIRECT 1 degC/LSB quirk
 * and the standard LINEAR11 encoding are both handled), select the
 * regulator's PAGE first on multi rail parts, and return the result
 * in millidegrees Celsius.
 *
 * This is what the generic drivers/thermal/pmbus_thermal.c companion
 * calls on its parent; keeping the decode here avoids exposing the
 * regulator-private pmbus_regulator_priv layout to other subsystems.
 *
 * Returns 0 on success, or a negative errno (-ENODEV if reg is not a
 * probed pmbus regulator, -EIO on bus error).
 */
int pmbus_regulator_read_temp(struct udevice *reg_dev, int *temp_mc);

/*
 * Look up the pmbus_driver_info of a probed UCLASS_REGULATOR device at
 * (bus_seq, addr) that is driven by a pmbus_helper based chip driver
 * (mpq8785, pmbus_generic, ...). Probes the device so its identify
 * hook has run and format[] is populated, then returns its
 * driver_info. Returns NULL if no such regulator is bound at that
 * address, if the device is not a pmbus regulator, or if
 * CONFIG_DM_REGULATOR_PMBUS_HELPER is disabled.
 *
 * Lets pmbus_set_active() -- and thus the pmbus CLI and the board
 * boot snapshots -- reuse the rich, VOUT_MODE detected driver_info of
 * a DT bound generic / chip regulator when the device is selected by
 * raw <bus>:<addr> (which has no chip-match registry entry and would
 * otherwise fall back to blanket LINEAR16 / LINEAR11 decoding).
 */
const struct pmbus_driver_info *pmbus_regulator_info_by_addr(int bus_seq,
							     u8 addr);

/*
 * Status bit name decoding.
 *
 * Sparse mask to name table. pmbus_print_bits() emits only the bits
 * that are SET in v, joined by |; if no bit is set, prints
 * clean. Bits not in the table are silently ignored (RESERVED bits
 * or chip specific bits handled by a separate per chip table).
 */
struct pmbus_bit {
	u16 mask;
	const char *name;
};

void pmbus_print_bits(u16 v, const struct pmbus_bit *tab);

/*
 * Per chip override entry. When a chip reuses a PMBus standard
 * STATUS bit for a documented chip specific signal (for example MPS
 * uses STATUS_WORD bit[12] = MFR_SPECIFIC as NVM_SUMMARY, bit[8] =
 * UNKNOWN as WATCH_DOG, bit[0] = NONE_ABOVE as DRMOS_FAULT), the
 * per chip driver supplies a sparse table of (reg, mask, name)
 * triples and pmbus_print_status_bits() substitutes the chip name
 * for the standard one when the bit is set.
 *
 * reg is one of PMBUS_STATUS_WORD / VOUT / IOUT / INPUT /
 * TEMPERATURE / CML, so the same table can carry overrides for
 * every status register on the chip in one place. Tables that omit
 * a (reg, mask) leave the standard name in place.
 *
 * Override entries whose mask is NOT in the standard table are
 * still printed (the chip can extend coverage beyond PMBus 1.x for
 * vendor specific bits in standard registers).
 *
 * Tables are NULL terminated: the last entry has .name = NULL.
 * Following the same convention U-Boot uses for struct udevice_id
 * and other driver tables avoids the explicit-count foot-gun.
 */
struct pmbus_status_override {
	u8 reg;
	u16 mask;
	const char *name;
};

/*
 * Print the bit names of a STATUS_* register value. For each bit
 * set in v, prefer a chip override matching (reg, mask) over the
 * standard std table entry; if neither matches, the bit is
 * silently skipped (RESERVED). If no bit is set at all, prints
 * clean. Pass ovr = NULL to disable the override path.
 */
void pmbus_print_status_bits(u8 reg, u16 v,
			     const struct pmbus_bit *std,
			     const struct pmbus_status_override *ovr);

/*
 * Built in PMBus 1.3 standard bit tables (use these from per chip
 * drivers and board diagnostics; vendor extensions go in chip local
 * tables that the per chip driver passes alongside these).
 */
/*
 * All tables below are NULL terminated (last entry has .name = NULL),
 * so callers walk with for (t = tab; t && t->name; t++) and the
 * helpers above need no count argument.
 */
extern const struct pmbus_bit pmbus_status_word_bits[];
extern const struct pmbus_bit pmbus_status_vout_bits[];
extern const struct pmbus_bit pmbus_status_iout_bits[];
extern const struct pmbus_bit pmbus_status_input_bits[];
extern const struct pmbus_bit pmbus_status_temp_bits[];
extern const struct pmbus_bit pmbus_status_cml_bits[];

/*
 * Active device tracking for the pmbus U-Boot CLI.
 *
 * The framework keeps one active PMBus device. It is selected by
 * pmbus dev <bus>:<addr> (raw I2C tuple) and remembered across
 * subcommands so subsequent invocations of pmbus telemetry,
 * pmbus status, pmbus dump, etc. operate on the same chip
 * without re-typing the address.
 *
 * pmbus_set_active() probes the chip's MFR_ID at the given address,
 * looks the result up in the chip-match registry (populated by
 * per chip drivers via pmbus_register_chip()), and caches the
 * resulting struct pmbus_driver_info. Subcommands consult
 * pmbus_active() to find the cached metadata.
 */
struct pmbus_active_dev {
	bool valid;
	int bus_seq;
	u8 addr;
	char vendor[PMBUS_VENDOR_NAME_MAX];
	char name[PMBUS_REGULATOR_NAME_MAX];	/* DT regulator-name when bound; "" otherwise */
	char mfr_id[PMBUS_MFR_STRING_MAX];
	char mfr_model[PMBUS_MFR_STRING_MAX];
	char mfr_revision[PMBUS_MFR_STRING_MAX];
	bool mfr_reverse; /* chip stores MFR strings LSB first */
	const struct pmbus_driver_info *info;
};

const struct pmbus_active_dev *pmbus_active(void);
int pmbus_active_get_i2c(struct udevice **i2c_dev);
int pmbus_set_active(int bus_seq, u8 addr);
void pmbus_clear_active(void);

/*
 * Per chip driver / board file registers a chip match so the
 * framework can associate an MFR_ID prefix (read at probe time)
 * with a vendor namespace ("mps", "lltc", "renesas", ...) and a
 * pmbus_driver_info pointer. The first matching entry wins.
 *
 * mfr_id_reverse flags MPS style chips that store the MFR_ID
 * string LSB first (chip returns "SPM" for the human string
 * "MPS"); the framework reads the string in both orderings and
 * matches against the prefix in the natural reading.
 */
struct pmbus_chip_match {
	const char *mfr_id;
	bool mfr_id_reverse;
	const char *vendor;
	const struct pmbus_driver_info *info;
};

int pmbus_register_chip(const struct pmbus_chip_match *match);

/*
 * Resolve a regulator-name (DT regulator-name property) to its
 * (bus, addr) tuple by walking UCLASS_REGULATOR. Used by the
 * pmbus dev <name> CLI alias so a chip bound through DT can be
 * selected by its human readable rail name (e.g. "+0V8_VDD")
 * instead of the i2c bus / address pair. Returns 0 on success
 * (out parameters populated and the regulator probed), or a
 * negative errno if no match is found or the bus / address cannot
 * be derived. Available only when CONFIG_DM_REGULATOR is set.
 */
int pmbus_resolve_by_name(const char *name, int *bus_seq, u8 *addr);

/*
 * Vendor extension dispatcher.
 *
 * When the user types pmbus <vendor> <args...>, the framework
 * looks up the registered handler for <vendor> and calls it with
 * the argv tail (argv[0] = "<vendor>"). The handler operates on
 * pmbus_active(), or returns CMD_RET_USAGE if the active device is
 * not from this vendor.
 *
 * Per chip drivers register their vendor handler at init time. The
 * MPS extension publishes pmbus mps last, pmbus mps clear last,
 * and pmbus mps clear force.
 */
typedef int (*pmbus_vendor_handler_t)(struct cmd_tbl *cmdtp, int flag,
				      int argc, char *const argv[]);

struct pmbus_vendor_op {
	const char *vendor;
	pmbus_vendor_handler_t handler;
	const char *help;
};

int pmbus_register_vendor_handler(const struct pmbus_vendor_op *op);
const struct pmbus_vendor_op *pmbus_lookup_vendor(const char *vendor);
unsigned int pmbus_vendor_count(void);
const struct pmbus_vendor_op *pmbus_vendor_at(unsigned int i);

#endif /* _PMBUS_H_ */

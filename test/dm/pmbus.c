// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * Unit tests for the PMBus 1.x framework, the generic
 * PMBus regulator and the pmbus CLI command.
 */

#include <dm.h>
#include <i2c.h>
#include <pmbus.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

/* The line pmbus dev prints when a chip is selected */
#define PMBUS_ACTIVE_LINE \
	"pmbus: active i2c0:0x70  rail=\"sandbox-pmbus-vout\"  " \
	"MFR_ID=\"SANDBOX\"  MODEL=\"PMBUS-EMUL\"  vendor=(generic)"

/* The line pmbus list prints for the bound chip */
#define PMBUS_LIST_LINE \
	"  i2c0:0x70  rail=\"sandbox-pmbus-vout\"  node=pmbus@70  " \
	"driver=pmbus_generic_regulator"

/* Select the emulated chip and check the resulting banner line */
static int pmbus_select(struct unit_test_state *uts)
{
	ut_assertok(run_command("pmbus dev 0:70", 0));
	ut_assert_nextline(PMBUS_ACTIVE_LINE);
	return 0;
}

/* The chip is reachable via UCLASS_REGULATOR (compatible = "pmbus") */
static int dm_test_pmbus_bind(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(uclass_get_device_by_name(UCLASS_REGULATOR, "pmbus@70",
					      &dev));
	ut_asserteq_str("pmbus_generic_regulator", dev->driver->name);
	ut_asserteq(UCLASS_I2C, device_get_uclass_id(dev_get_parent(dev)));

	return 0;
}

DM_TEST(dm_test_pmbus_bind, UTF_SCAN_FDT);

/* pmbus dev by <bus>:<addr> and by regulator-name select the chip */
static int dm_test_pmbus_dev(struct unit_test_state *uts)
{
	ut_assertok(run_command("pmbus dev 0:70", 0));
	ut_assert_nextline(PMBUS_ACTIVE_LINE);
	ut_assert_console_end();

	/* Selecting by DT regulator-name resolves to the same chip */
	ut_assertok(run_command("pmbus dev sandbox-pmbus-vout", 0));
	ut_assert_nextline(PMBUS_ACTIVE_LINE);
	ut_assert_console_end();

	/* pmbus dev with no argument reprints the active chip */
	ut_assertok(run_command("pmbus dev", 0));
	ut_assert_nextline(PMBUS_ACTIVE_LINE);
	ut_assert_console_end();

	return 0;
}

DM_TEST(dm_test_pmbus_dev, UTF_SCAN_FDT | UTF_CONSOLE);

/* pmbus list enumerates the bound chip among the UCLASS_REGULATOR devices */
static int dm_test_pmbus_list(struct unit_test_state *uts)
{
	ut_assertok(run_command("pmbus list", 0));
	ut_assert_skip_to_line(PMBUS_LIST_LINE);

	return 0;
}

DM_TEST(dm_test_pmbus_list, UTF_SCAN_FDT | UTF_CONSOLE);

/* pmbus info decodes identification + the detected driver_info */
static int dm_test_pmbus_info(struct unit_test_state *uts)
{
	ut_assertok(pmbus_select(uts));

	ut_assertok(run_command("pmbus info", 0));
	ut_assert_nextline("pmbus device i2c0:0x70");
	ut_assert_nextline("  regulator-name: \"sandbox-pmbus-vout\"");
	ut_assert_nextline("  MFR_ID        : \"SANDBOX\"");
	ut_assert_nextline("  MFR_MODEL     : \"PMBUS-EMUL\"");
	ut_assert_nextline("  MFR_REVISION  : \"1.0\"  raw=0x312e30");
	ut_assert_nextline("  PMBUS_REVISION: 0x33 (PMBus 1.3)");
	ut_assert_nextline("  vendor        : (none)");
	ut_assert_nextline("  driver_info   : pages=1");
	ut_assert_nextline("    [VOLTAGE_IN  ] format=LINEAR");
	ut_assert_nextline("    [VOLTAGE_OUT ] format=LINEAR");
	ut_assert_nextline("    [CURRENT_IN  ] format=LINEAR");
	ut_assert_nextline("    [CURRENT_OUT ] format=LINEAR");
	ut_assert_nextline("    [POWER       ] format=LINEAR");
	ut_assert_nextline("    [TEMPERATURE ] format=LINEAR");
	ut_assert_console_end();

	return 0;
}

DM_TEST(dm_test_pmbus_info, UTF_SCAN_FDT | UTF_CONSOLE);

/*
 * pmbus telemetry decodes the implemented sensors and prints
 * "(not supported)" for the commands the emulator NAKs (READ_IIN,
 * READ_POUT). LINEAR11 is used for VIN/IOUT/TEMP, LINEAR16 (with the
 * VOUT_MODE 2^-8 exponent) for VOUT.
 */
static int dm_test_pmbus_telemetry(struct unit_test_state *uts)
{
	ut_assertok(pmbus_select(uts));

	ut_assertok(run_command("pmbus telemetry", 0));
	ut_assert_nextline("pmbus telemetry @ i2c0:0x70");
	ut_assert_nextline("  VIN  : raw=0x0abc  1400.000V");
	ut_assert_nextline("  VOUT : raw=0x0200  2.000V");
	ut_assert_nextline("  IIN  : (not supported)");
	ut_assert_nextline("  IOUT : raw=0x0123  291.000A");
	ut_assert_nextline("  POUT : (not supported)");
	ut_assert_nextline("  TEMP : raw=0x0019  25.000C");
	ut_assert_console_end();

	return 0;
}

DM_TEST(dm_test_pmbus_telemetry, UTF_SCAN_FDT | UTF_CONSOLE);

/* pmbus status decodes every STATUS_* register; the emulator is clean */
static int dm_test_pmbus_status(struct unit_test_state *uts)
{
	ut_assertok(pmbus_select(uts));

	ut_assertok(run_command("pmbus status", 0));
	ut_assert_nextline("pmbus status @ i2c0:0x70");
	ut_assert_nextline("  STATUS_WORD    (79h) = 0x0000  [clean]");
	ut_assert_nextline("  STATUS_VOUT    (7Ah) = 0x00  [clean]");
	ut_assert_nextline("  STATUS_IOUT    (7Bh) = 0x00  [clean]");
	ut_assert_nextline("  STATUS_INPUT   (7Ch) = 0x00  [clean]");
	ut_assert_nextline("  STATUS_TEMP    (7Dh) = 0x00  [clean]");
	ut_assert_nextline("  STATUS_CML     (7Eh) = 0x00  [clean]");
	ut_assert_console_end();

	return 0;
}

DM_TEST(dm_test_pmbus_status, UTF_SCAN_FDT | UTF_CONSOLE);

/* pmbus read/pmbus write raw register access (byte, word, string) */
static int dm_test_pmbus_read_write(struct unit_test_state *uts)
{
	ut_assertok(pmbus_select(uts));

	/* Symbolic and numeric register names both resolve */
	ut_assertok(run_command("pmbus read VOUT_MODE", 0));
	ut_assert_nextline("  20h  VOUT_MODE       b=0x18");
	ut_assertok(run_command("pmbus read 8b w", 0));
	ut_assert_nextline("  8bh  READ_VOUT       w=0x0200");
	ut_assertok(run_command("pmbus read MFR_ID s", 0));
	ut_assert_nextline("  99h  MFR_ID          s=\"SANDBOX\"");

	/* A word write is observable on the next read-back */
	ut_assertok(run_command("pmbus write VOUT_COMMAND 123 w", 0));
	ut_assert_nextline("pmbus: wrote 0x123 to 21h (VOUT_COMMAND)");
	ut_assertok(run_command("pmbus read VOUT_COMMAND w", 0));
	ut_assert_nextline("  21h  VOUT_COMMAND    w=0x0123");
	ut_assert_console_end();

	return 0;
}

DM_TEST(dm_test_pmbus_read_write, UTF_SCAN_FDT | UTF_CONSOLE);

/* pmbus vout reads back VOUT via the active driver_info decoder */
static int dm_test_pmbus_vout(struct unit_test_state *uts)
{
	ut_assertok(pmbus_select(uts));

	ut_assertok(run_command("pmbus vout", 0));
	ut_assert_nextline("pmbus VOUT @ i2c0:0x70  raw=0x0200  2.000V");
	ut_assert_console_end();

	return 0;
}

DM_TEST(dm_test_pmbus_vout, UTF_SCAN_FDT | UTF_CONSOLE);

/* pmbus dump walks every standard register; spot-check one line */
static int dm_test_pmbus_dump(struct unit_test_state *uts)
{
	ut_assertok(pmbus_select(uts));

	ut_assertok(run_command("pmbus dump", 0));
	ut_assert_nextline("pmbus dump @ i2c0:0x70 (registers known to <pmbus.h>)");
	ut_assert_skip_to_line("  20h  VOUT_MODE       b=0x18");

	return 0;
}

DM_TEST(dm_test_pmbus_dump, UTF_SCAN_FDT | UTF_CONSOLE);

/* pmbus clear [faults] issues CLEAR_FAULTS (03h) */
static int dm_test_pmbus_clear(struct unit_test_state *uts)
{
	ut_assertok(pmbus_select(uts));

	ut_assertok(run_command("pmbus clear faults", 0));
	ut_assert_nextline("pmbus: CLEAR_FAULTS (03h) issued (RAM sticky STATUS_* cleared)");
	ut_assert_console_end();

	return 0;
}

DM_TEST(dm_test_pmbus_clear, UTF_SCAN_FDT | UTF_CONSOLE);

/* pmbus scan finds the emulated chip by its MFR_ID block read */
static int dm_test_pmbus_scan(struct unit_test_state *uts)
{
	ut_assertok(run_command("pmbus scan 0", 0));
	ut_assert_skip_to_line("  i2c0:0x70  MFR_ID=\"SANDBOX\"");

	return 0;
}

DM_TEST(dm_test_pmbus_scan, UTF_SCAN_FDT | UTF_CONSOLE);

/* pmbus help lists vendor extensions; none are registered here */
static int dm_test_pmbus_help(struct unit_test_state *uts)
{
	ut_assertok(run_command("pmbus help", 0));
	ut_assert_nextline("pmbus: no vendor extensions registered.");
	ut_assert_skip_to_line("       board hook (boot snapshot) and re run 'pmbus help'.");

	return 0;
}

DM_TEST(dm_test_pmbus_help, UTF_SCAN_FDT | UTF_CONSOLE);

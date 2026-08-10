/* Copyright 2014 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* Host communication command constants for Chrome EC */

#ifndef __CROS_EC_EC_COMMANDS_H
#define __CROS_EC_EC_COMMANDS_H

#if !defined(__ACPI__) && (!defined(__KERNEL__) || defined(__UBOOT__))
#include <stdint.h>
#endif

#ifdef __UBOOT__
#include <linux/bitops.h>
#endif

#ifdef CHROMIUM_EC
/*
 * CHROMIUM_EC is defined by the Makefile system of Chromium EC repository.
 * It is used to not include macros that may cause conflicts in foreign
 * projects (refer to crbug.com/984623).
 */

/*
 * Include common.h for CONFIG_HOSTCMD_ALIGNED, if it's defined. This
 * generates more efficient code for accessing request/response structures on
 * ARM Cortex-M if the structures are guaranteed 32-bit aligned.
 */
#include "common.h"
#include "compile_time_macros.h"

#else
/* If BUILD_ASSERT isn't already defined, make it a no-op */
#ifndef BUILD_ASSERT
#define BUILD_ASSERT(_cond)
#endif /* !BUILD_ASSERT */
#endif /* CHROMIUM_EC */

#if defined(__KERNEL__)
#if !defined(__UBOOT__)
#include <linux/limits.h>
#endif
#else
/*
 * Defines macros that may be needed but are for sure defined by the linux
 * kernel. This section is removed when cros_ec_commands.h is generated (by
 * util/make_linux_ec_commands_h.sh).
 * cros_ec_commands.h looks more integrated to the kernel.
 */

#ifndef BIT
#define BIT(nr) (1UL << (nr))
#endif

#ifndef BIT_ULL
#define BIT_ULL(nr) (1ULL << (nr))
#endif

/*
 * When building Zephyr, this file ends up being included before Zephyr's
 * include/sys/util.h so causes a warning there. We don't want to add an #ifdef
 * in that file since it won't be accepted upstream. So work around it here.
 */
#ifndef CONFIG_ZEPHYR
#ifndef GENMASK
#define GENMASK(h, l) (((BIT(h) << 1) - 1) ^ (BIT(l) - 1))
#endif

#ifndef GENMASK_ULL
#define GENMASK_ULL(h, l) (((BIT_ULL(h) << 1) - 1) ^ (BIT_ULL(l) - 1))
#endif
#endif

#endif /* __KERNEL__ */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Constant for creation of flexible array members that work in both C and
 * C++. Flexible array members were added in C99 and are not part of the C++
 * standard. However, clang++ supports them for C++.
 * When compiling with gcc, flexible array members are not allowed to appear
 * in an otherwise empty struct, so we use the GCC zero-length array
 * extension that works with both clang/gcc/g++.
 */
#if defined(__cplusplus) && defined(__clang__)
#define FLEXIBLE_ARRAY_MEMBER_SIZE
#else
#define FLEXIBLE_ARRAY_MEMBER_SIZE 0
#endif

/*
 * Current version of this protocol
 *
 * TODO(crosbug.com/p/11223): This is effectively useless; protocol is
 * determined in other ways.  Remove this once the kernel code no longer
 * depends on it.
 */
#define EC_PROTO_VERSION 0x00000002

/* Command version mask */
#define EC_VER_MASK(version) BIT(version)

/* I/O addresses for ACPI commands */
#define EC_LPC_ADDR_ACPI_DATA 0x62
#define EC_LPC_ADDR_ACPI_CMD 0x66

/* I/O addresses for host command */
#define EC_LPC_ADDR_HOST_DATA 0x200
#define EC_LPC_ADDR_HOST_CMD 0x204

/* I/O addresses for host command args and params */
/* Protocol version 2 */
#define EC_LPC_ADDR_HOST_ARGS 0x800 /* And 0x801, 0x802, 0x803 */
/* For version 2 params; size is EC_PROTO2_MAX_PARAM_SIZE */
#define EC_LPC_ADDR_HOST_PARAM 0x804

/* Protocol version 3 */
#define EC_LPC_ADDR_HOST_PACKET 0x800 /* Offset of version 3 packet */
#define EC_LPC_HOST_PACKET_SIZE 0x100 /* Max size of version 3 packet */

/*
 * The actual block is 0x800-0x8ff, but some BIOSes think it's 0x880-0x8ff
 * and they tell the kernel that so we have to think of it as two parts.
 *
 * Other BIOSes report only the I/O port region spanned by the Microchip
 * MEC series EC; an attempt to address a larger region may fail.
 */
#define EC_HOST_CMD_REGION0 0x800
#define EC_HOST_CMD_REGION1 0x880
#define EC_HOST_CMD_REGION_SIZE 0x80
#define EC_HOST_CMD_MEC_REGION_SIZE 0x8

/* EC command register bit functions */
#define EC_LPC_CMDR_DATA BIT(0) /* Data ready for host to read */
#define EC_LPC_CMDR_PENDING BIT(1) /* Write pending to EC */
#define EC_LPC_CMDR_BUSY BIT(2) /* EC is busy processing a command */
#define EC_LPC_CMDR_CMD BIT(3) /* Last host write was a command */
#define EC_LPC_CMDR_ACPI_BRST BIT(4) /* Burst mode (not used) */
#define EC_LPC_CMDR_SCI BIT(5) /* SCI event is pending */
#define EC_LPC_CMDR_SMI BIT(6) /* SMI event is pending */

#define EC_LPC_ADDR_MEMMAP 0x900
#define EC_MEMMAP_SIZE 255 /* ACPI IO buffer max is 255 bytes */
#define EC_MEMMAP_TEXT_MAX 8 /* Size of a string in the memory map */

#define EC_LPC_ADDR_MEMMAP_INDEXED_IO 0x380

/* The offset address of each type of data in mapped memory. */
#define EC_MEMMAP_TEMP_SENSOR 0x00 /* Temp sensors 0x00 - 0x0f */
#define EC_MEMMAP_FAN 0x10 /* Fan speeds 0x10 - 0x17 */
#define EC_MEMMAP_TEMP_SENSOR_B 0x18 /* More temp sensors 0x18 - 0x1f */
#define EC_MEMMAP_ID 0x20 /* 0x20 == 'E', 0x21 == 'C' */
#define EC_MEMMAP_ID_VERSION 0x22 /* Version of data in 0x20 - 0x2f */
#define EC_MEMMAP_THERMAL_VERSION 0x23 /* Version of data in 0x00 - 0x1f */
#define EC_MEMMAP_BATTERY_VERSION 0x24 /* Version of data in 0x40 - 0x7f */
#define EC_MEMMAP_SWITCHES_VERSION 0x25 /* Version of data in 0x30 - 0x33 */
#define EC_MEMMAP_EVENTS_VERSION 0x26 /* Version of data in 0x34 - 0x3f */
#define EC_MEMMAP_HOST_CMD_FLAGS 0x27 /* Host cmd interface flags (8 bits) */
/* Unused 0x28 - 0x2f */
#define EC_MEMMAP_SWITCHES 0x30 /* 8 bits */
/* Unused 0x31 - 0x33 */
#define EC_MEMMAP_HOST_EVENTS 0x34 /* 64 bits */
/* Battery values are all 32 bits, unless otherwise noted. */
#define EC_MEMMAP_BATT_VOLT 0x40 /* Battery Present Voltage */
#define EC_MEMMAP_BATT_RATE 0x44 /* Battery Present Rate */
#define EC_MEMMAP_BATT_CAP 0x48 /* Battery Remaining Capacity */
#define EC_MEMMAP_BATT_FLAG 0x4c /* Battery State, see below (8-bit) */
#define EC_MEMMAP_BATT_COUNT 0x4d /* Battery Count (8-bit) */
#define EC_MEMMAP_BATT_INDEX 0x4e /* Current Battery Data Index (8-bit) */
/* Unused 0x4f */
#define EC_MEMMAP_BATT_DCAP 0x50 /* Battery Design Capacity */
#define EC_MEMMAP_BATT_DVLT 0x54 /* Battery Design Voltage */
#define EC_MEMMAP_BATT_LFCC 0x58 /* Battery Last Full Charge Capacity */
#define EC_MEMMAP_BATT_CCNT 0x5c /* Battery Cycle Count */
/* Strings are all 8 bytes (EC_MEMMAP_TEXT_MAX) */
#define EC_MEMMAP_BATT_MFGR 0x60 /* Battery Manufacturer String */
#define EC_MEMMAP_BATT_MODEL 0x68 /* Battery Model Number String */
#define EC_MEMMAP_BATT_SERIAL 0x70 /* Battery Serial Number String */
#define EC_MEMMAP_BATT_TYPE 0x78 /* Battery Type String */
#define EC_MEMMAP_ALS 0x80 /* ALS readings in lux (2 X 16 bits) */
/* Unused 0x84 - 0x8f */
#define EC_MEMMAP_ACC_STATUS 0x90 /* Accelerometer status (8 bits )*/
/* Unused 0x91 */
#define EC_MEMMAP_ACC_DATA 0x92 /* Accelerometers data 0x92 - 0x9f */
/* 0x92: Lid Angle if available, LID_ANGLE_UNRELIABLE otherwise */
/* 0x94 - 0x99: 1st Accelerometer */
/* 0x9a - 0x9f: 2nd Accelerometer */

#define EC_MEMMAP_GYRO_DATA 0xa0 /* Gyroscope data 0xa0 - 0xa5 */
#define EC_MEMMAP_GPU 0xa6 /* GPU-specific, 8 bits */

/*
 * Bit fields for EC_MEMMAP_GPU
 * 0:2: D-Notify level (0:D1, ... 4:D5)
 * 3: Over temperature
 */
#define EC_MEMMAP_GPU_D_NOTIFY_MASK GENMASK(2, 0)
#define EC_MEMMAP_GPU_OVERT_BIT BIT(3)

/* Power Participant related components */
#define EC_MEMMAP_PWR_SRC 0xa7 /* Power source (8-bit) */
/* Unused 0xa8 - 0xdf */

/*
 * ACPI is unable to access memory mapped data at or above this offset due to
 * limitations of the ACPI protocol. Do not place data in the range 0xe0 - 0xfe
 * which might be needed by ACPI.
 */
#define EC_MEMMAP_NO_ACPI 0xe0

/* Define the format of the accelerometer mapped memory status byte. */
#define EC_MEMMAP_ACC_STATUS_SAMPLE_ID_MASK 0x0f
#define EC_MEMMAP_ACC_STATUS_BUSY_BIT BIT(4)
#define EC_MEMMAP_ACC_STATUS_PRESENCE_BIT BIT(7)

/* Number of temp sensors at EC_MEMMAP_TEMP_SENSOR */
#define EC_TEMP_SENSOR_ENTRIES 16
/*
 * Number of temp sensors at EC_MEMMAP_TEMP_SENSOR_B.
 *
 * Valid only if EC_MEMMAP_THERMAL_VERSION returns >= 2.
 */
#define EC_TEMP_SENSOR_B_ENTRIES 8

/* Max temp sensor entries for host commands */
#define EC_MAX_TEMP_SENSOR_ENTRIES \
	(EC_TEMP_SENSOR_ENTRIES + EC_TEMP_SENSOR_B_ENTRIES)

/* Special values for mapped temperature sensors */
#define EC_TEMP_SENSOR_NOT_PRESENT 0xff
#define EC_TEMP_SENSOR_ERROR 0xfe
#define EC_TEMP_SENSOR_NOT_POWERED 0xfd
#define EC_TEMP_SENSOR_NOT_CALIBRATED 0xfc
/*
 * The offset of temperature value stored in mapped memory.  This allows
 * reporting a temperature range of 200K to 454K = -73C to 181C.
 */
#define EC_TEMP_SENSOR_OFFSET 200

/*
 * Number of ALS readings at EC_MEMMAP_ALS
 */
#define EC_ALS_ENTRIES 2

/*
 * The default value a temperature sensor will return when it is present but
 * has not been read this boot.  This is a reasonable number to avoid
 * triggering alarms on the host.
 */
#define EC_TEMP_SENSOR_DEFAULT (296 - EC_TEMP_SENSOR_OFFSET)

#define EC_FAN_SPEED_ENTRIES 4 /* Number of fans at EC_MEMMAP_FAN */
#define EC_FAN_SPEED_NOT_PRESENT 0xffff /* Entry not present */

/* Report 0 for fan stalled so userspace applications can take
 * an appropriate action based on this value to control the fan.
 */
#define EC_FAN_SPEED_STALLED 0x0
/* This should be used only for ectool to support old ECs. */
#define EC_FAN_SPEED_STALLED_DEPRECATED 0xfffe

/* Battery bit flags at EC_MEMMAP_BATT_FLAG. */
#define EC_BATT_FLAG_AC_PRESENT 0x01
#define EC_BATT_FLAG_BATT_PRESENT 0x02
#define EC_BATT_FLAG_DISCHARGING 0x04
#define EC_BATT_FLAG_CHARGING 0x08
#define EC_BATT_FLAG_LEVEL_CRITICAL 0x10
/* Set if some of the static/dynamic data is invalid (or outdated). */
#define EC_BATT_FLAG_INVALID_DATA 0x20
#define EC_BATT_FLAG_CUT_OFF 0x40

/*
 * Value written to EC_MEMMAP_BATT_DCAP, EC_MEMMAP_BATT_DVLT, EC_MEMMAP_CCNT,
 * EC_MEMMAP_BATT_VOLT, EC_MEMMAP_BATT_RATE, EC_MEMMAP_BATT_CAP, and
 * EC_MEMMAP_BATT_LFCC if the actual value is unknown.
 *
 * This corresponds with the unknown value specified by ACPI release 6.5
 * Section 10.2.2 (and earlier versions), to match expectations of ACPI
 * firmware.
 */
#define EC_MEMMAP_BATT_UNKNOWN_VALUE (-1)

/* Switch flags at EC_MEMMAP_SWITCHES */
#define EC_SWITCH_LID_OPEN 0x01
#define EC_SWITCH_POWER_BUTTON_PRESSED 0x02
/* Was write protect disabled; now unused. */
#define EC_SWITCH_IGNORE2 0x04
/* Was recovery requested via keyboard; now unused. */
#define EC_SWITCH_IGNORE1 0x08
/* Recovery requested via dedicated signal (from servo board) */
#define EC_SWITCH_DEDICATED_RECOVERY 0x10
/* Was fake developer mode switch; now unused.  Remove in next refactor. */
#define EC_SWITCH_IGNORE0 0x20

/* Host command interface flags */
/* Host command interface supports LPC args (LPC interface only) */
#define EC_HOST_CMD_FLAG_LPC_ARGS_SUPPORTED 0x01
/* Host command interface supports version 3 protocol */
#define EC_HOST_CMD_FLAG_VERSION_3 0x02

/* Wireless switch flags */
#define EC_WIRELESS_SWITCH_ALL ~0x00 /* All flags */
#define EC_WIRELESS_SWITCH_WLAN 0x01 /* WLAN radio */
#define EC_WIRELESS_SWITCH_BLUETOOTH 0x02 /* Bluetooth radio */
#define EC_WIRELESS_SWITCH_WWAN 0x04 /* WWAN power */
#define EC_WIRELESS_SWITCH_WLAN_POWER 0x08 /* WLAN power */

/*****************************************************************************/
/*
 * ACPI commands
 *
 * These are valid ONLY on the ACPI command/data port.
 */

/*
 * ACPI Read Embedded Controller
 *
 * This reads from ACPI memory space on the EC (EC_ACPI_MEM_*).
 *
 * Use the following sequence:
 *
 *    - Write EC_CMD_ACPI_READ to EC_LPC_ADDR_ACPI_CMD
 *    - Wait for EC_LPC_CMDR_PENDING bit to clear
 *    - Write address to EC_LPC_ADDR_ACPI_DATA
 *    - Wait for EC_LPC_CMDR_DATA bit to set
 *    - Read value from EC_LPC_ADDR_ACPI_DATA
 */
#define EC_CMD_ACPI_READ 0x0080

/*
 * ACPI Write Embedded Controller
 *
 * This reads from ACPI memory space on the EC (EC_ACPI_MEM_*).
 *
 * Use the following sequence:
 *
 *    - Write EC_CMD_ACPI_WRITE to EC_LPC_ADDR_ACPI_CMD
 *    - Wait for EC_LPC_CMDR_PENDING bit to clear
 *    - Write address to EC_LPC_ADDR_ACPI_DATA
 *    - Wait for EC_LPC_CMDR_PENDING bit to clear
 *    - Write value to EC_LPC_ADDR_ACPI_DATA
 */
#define EC_CMD_ACPI_WRITE 0x0081

/*
 * ACPI Burst Enable Embedded Controller
 *
 * This enables burst mode on the EC to allow the host to issue several
 * commands back-to-back. While in this mode, writes to mapped multi-byte
 * data are locked out to ensure data consistency.
 */
#define EC_CMD_ACPI_BURST_ENABLE 0x0082

/*
 * ACPI Burst Disable Embedded Controller
 *
 * This disables burst mode on the EC and stops preventing EC writes to mapped
 * multi-byte data.
 */
#define EC_CMD_ACPI_BURST_DISABLE 0x0083

/*
 * ACPI Query Embedded Controller
 *
 * This clears the lowest-order bit in the currently pending host events, and
 * sets the result code to the 1-based index of the bit (event 0x00000001 = 1,
 * event 0x80000000 = 32), or 0 if no event was pending.
 */
#define EC_CMD_ACPI_QUERY_EVENT 0x0084

/* Valid addresses in ACPI memory space, for read/write commands */

/* Memory space version; set to EC_ACPI_MEM_VERSION_CURRENT */
#define EC_ACPI_MEM_VERSION 0x00
/*
 * Test location; writing value here updates test compliment byte to (0xff -
 * value).
 */
#define EC_ACPI_MEM_TEST 0x01
/* Test compliment; writes here are ignored. */
#define EC_ACPI_MEM_TEST_COMPLIMENT 0x02

/* Keyboard backlight brightness percent (0 - 100) */
#define EC_ACPI_MEM_KEYBOARD_BACKLIGHT 0x03
/* DPTF Target Fan Duty (0-100, 0xff for auto/none) */
#define EC_ACPI_MEM_FAN_DUTY 0x04

/*
 * DPTF temp thresholds. Any of the EC's temp sensors can have up to two
 * independent thresholds attached to them. The current value of the ID
 * register determines which sensor is affected by the THRESHOLD and COMMIT
 * registers. The THRESHOLD register uses the same EC_TEMP_SENSOR_OFFSET scheme
 * as the memory-mapped sensors. The COMMIT register applies those settings.
 *
 * The spec does not mandate any way to read back the threshold settings
 * themselves, but when a threshold is crossed the AP needs a way to determine
 * which sensor(s) are responsible. Each reading of the ID register clears and
 * returns one sensor ID that has crossed one of its threshold (in either
 * direction) since the last read. A value of 0xFF means "no new thresholds
 * have tripped". Setting or enabling the thresholds for a sensor will clear
 * the unread event count for that sensor.
 */
#define EC_ACPI_MEM_TEMP_ID 0x05
#define EC_ACPI_MEM_TEMP_THRESHOLD 0x06
#define EC_ACPI_MEM_TEMP_COMMIT 0x07
/*
 * Here are the bits for the COMMIT register:
 *   bit 0 selects the threshold index for the chosen sensor (0/1)
 *   bit 1 enables/disables the selected threshold (0 = off, 1 = on)
 * Each write to the commit register affects one threshold.
 */
#define EC_ACPI_MEM_TEMP_COMMIT_SELECT_MASK BIT(0)
#define EC_ACPI_MEM_TEMP_COMMIT_ENABLE_MASK BIT(1)
/*
 * Example:
 *
 * Set the thresholds for sensor 2 to 50 C and 60 C:
 *   write 2 to [0x05]      --  select temp sensor 2
 *   write 0x7b to [0x06]   --  C_TO_K(50) - EC_TEMP_SENSOR_OFFSET
 *   write 0x2 to [0x07]    --  enable threshold 0 with this value
 *   write 0x85 to [0x06]   --  C_TO_K(60) - EC_TEMP_SENSOR_OFFSET
 *   write 0x3 to [0x07]    --  enable threshold 1 with this value
 *
 * Disable the 60 C threshold, leaving the 50 C threshold unchanged:
 *   write 2 to [0x05]      --  select temp sensor 2
 *   write 0x1 to [0x07]    --  disable threshold 1
 */

/* DPTF battery charging current limit */
#define EC_ACPI_MEM_CHARGING_LIMIT 0x08

/* Charging limit is specified in 64 mA steps */
#define EC_ACPI_MEM_CHARGING_LIMIT_STEP_MA 64
/* Value to disable DPTF battery charging limit */
#define EC_ACPI_MEM_CHARGING_LIMIT_DISABLED 0xff

/*
 * Report device orientation
 *  Bits       Definition
 *  4          Off Body/On Body status: 0 = Off Body.
 *  3:1        Device DPTF Profile Number (DDPN)
 *               0   = Reserved for backward compatibility (indicates no valid
 *                     profile number. Host should fall back to using TBMD).
 *              1..7 = DPTF Profile number to indicate to host which table needs
 *                     to be loaded.
 *   0         Tablet Mode Device Indicator (TBMD)
 */
#define EC_ACPI_MEM_DEVICE_ORIENTATION 0x09
#define EC_ACPI_MEM_TBMD_SHIFT 0
#define EC_ACPI_MEM_TBMD_MASK 0x1
#define EC_ACPI_MEM_DDPN_SHIFT 1
#define EC_ACPI_MEM_DDPN_MASK 0x7
#define EC_ACPI_MEM_STTB_SHIFT 4
#define EC_ACPI_MEM_STTB_MASK 0x1

/*
 * Report device features. Uses the same format as the host command, except:
 *
 * bit 0 (EC_FEATURE_LIMITED) changes meaning from "EC code has a limited set
 * of features", which is of limited interest when the system is already
 * interpreting ACPI bytecode, to "EC_FEATURES[0-7] is not supported". Since
 * these are supported, it defaults to 0.
 * This allows detecting the presence of this field since older versions of
 * the EC codebase would simply return 0xff to that unknown address. Check
 * FEATURES0 != 0xff (or FEATURES0[0] == 0) to make sure that the other bits
 * are valid.
 */
#define EC_ACPI_MEM_DEVICE_FEATURES0 0x0a
#define EC_ACPI_MEM_DEVICE_FEATURES1 0x0b
#define EC_ACPI_MEM_DEVICE_FEATURES2 0x0c
#define EC_ACPI_MEM_DEVICE_FEATURES3 0x0d
#define EC_ACPI_MEM_DEVICE_FEATURES4 0x0e
#define EC_ACPI_MEM_DEVICE_FEATURES5 0x0f
#define EC_ACPI_MEM_DEVICE_FEATURES6 0x10
#define EC_ACPI_MEM_DEVICE_FEATURES7 0x11

#define EC_ACPI_MEM_BATTERY_INDEX 0x12

/*
 * USB Port Power. Each bit indicates whether the corresponding USB ports' power
 * is enabled (1) or disabled (0).
 *   bit 0 USB port ID 0
 *   ...
 *   bit 7 USB port ID 7
 */
#define EC_ACPI_MEM_USB_PORT_POWER 0x13

/*
 * USB Retimer firmware update.
 * Read:
 *      Result of last operation AP requested
 * Write:
 *      bits[3:0]: USB-C port number
 *      bits[7:4]: Operation requested by AP
 *
 * NDA (no device attached) case:
 * To update retimer firmware, AP needs set up TBT Alt mode.
 * AP requests operations in this sequence:
 * 1. Get port information about which ports support retimer firmware update.
 * In the query result, each bit represents one port.
 * 2. Get current MUX mode, it's NDA.
 * 3. Suspend specified PD port's task.
 * 4. AP requests EC to enter USB mode -> enter Safe mode -> enter TBT mode ->
 * update firmware -> disconnect MUX -> resume PD task.
 *
 * DA (device attached) cases:
 * Retimer firmware update is not supported in DA cases.
 * 1. Get port information about which ports support retimer firmware update
 * 2. Get current MUX mode, it's DA.
 * 3. AP continues. No more retimer firmware update activities.
 *
 */
#define EC_ACPI_MEM_USB_RETIMER_FW_UPDATE 0x14

#define USB_RETIMER_FW_UPDATE_OP_SHIFT 4
#define USB_RETIMER_FW_UPDATE_ERR 0xfe
#define USB_RETIMER_FW_UPDATE_INVALID_MUX 0xff
/* Mask to clear unused MUX bits in retimer firmware update  */
#define USB_RETIMER_FW_UPDATE_MUX_MASK                          \
	(USB_PD_MUX_USB_ENABLED | USB_PD_MUX_DP_ENABLED |       \
	 USB_PD_MUX_SAFE_MODE | USB_PD_MUX_TBT_COMPAT_ENABLED | \
	 USB_PD_MUX_USB4_ENABLED)

/* Retimer firmware update operations */
#define USB_RETIMER_FW_UPDATE_QUERY_PORT 0 /* Which ports has retimer */
#define USB_RETIMER_FW_UPDATE_SUSPEND_PD 1 /* Suspend PD port */
#define USB_RETIMER_FW_UPDATE_RESUME_PD 2 /* Resume PD port  */
#define USB_RETIMER_FW_UPDATE_GET_MUX 3 /* Read current USB MUX  */
#define USB_RETIMER_FW_UPDATE_SET_USB 4 /* Set MUX to USB mode   */
#define USB_RETIMER_FW_UPDATE_SET_SAFE 5 /* Set MUX to Safe mode  */
#define USB_RETIMER_FW_UPDATE_SET_TBT 6 /* Set MUX to TBT mode   */
#define USB_RETIMER_FW_UPDATE_DISCONNECT 7 /* Set MUX to disconnect */

#define EC_ACPI_MEM_USB_RETIMER_PORT(x) ((x) & 0x0f)
#define EC_ACPI_MEM_USB_RETIMER_OP(x) \
	(((x) & 0xf0) >> USB_RETIMER_FW_UPDATE_OP_SHIFT)

/*
 * Offset 0x15 is reserved for PBOK, added to coreboot in
 * https://crrev.com/c/3840943 and proposed for inclusion here
 * in https://crrev.com/c/3547317.
 */

/*
 * Get extended strings from the EC.
 * Write:
 *     String index, or 0 to probe for EC support.
 * Read:
 *     String bytes, following by repeating null bytes.
 *
 * Writing a byte (EC_ACPI_MEM_STRINGS_FIFO_ID_*) selects a string, and the
 * following reads return the non-null bytes of the string in sequence until
 * the end of the string is reached. After the end of the string, reads 0 until
 * another byte is written. This interface allows ACPI firmware to read longer
 * strings from the EC than can reasonably fit into the shared memory region.
 *
 * To probe for EC support, write FIFO_ID_VERSION and read will return at least
 * one nonzero (MEM_STRINGS_FIFO_V1 for example) if MEM_STRINGS_FIFO is
 * supported. Returned values will indicate which strings are supported. If the
 * first byte is 0xff, the strings FIFO is unsupported.
 */
#define EC_ACPI_MEM_STRINGS_FIFO 0x16

/* String index to probe EC support. */
#define EC_ACPI_MEM_STRINGS_FIFO_ID_VERSION 0
#define EC_ACPI_MEM_STRINGS_FIFO_V1 1
/*
 * 0xff is the value the EC returns for unimplemented reads, indicating
 * the current EC firmware does not implement this command.
 */
#define EC_ACPI_MEM_STRINGS_FIFO_UNSUPPORTED 0xff

/*
 * Battery model number for the selected battery. Supported since V1.
 * Presents the same data as EC_MEMMAP_BATT_MODEL, but can provide more
 * than 8 bytes.
 *
 * This and the other FIFO_ID_BATTERY strings can select one of multiple
 * batteries by changing the value at EC_MEMMAP_BATT_INDEX. Once that index
 * is changed, reads of these strings will return information for the
 * corresponding battery, if present.
 */
#define EC_ACPI_MEM_STRINGS_FIFO_ID_BATTERY_MODEL 1
/*
 * Battery serial number for the selected battery. Supported since V1.
 * Presents the same data as EC_MEMMAP_BATT_SERIAL, but can provide more
 * than 8 bytes.
 */
#define EC_ACPI_MEM_STRINGS_FIFO_ID_BATTERY_SERIAL 2
/*
 * Battery manufacturer for the selected battery. Supported since V1.
 * Presents the same data as EC_MEMMAP_BATT_MFGR, but can provide more
 * than 8 bytes.
 */
#define EC_ACPI_MEM_STRINGS_FIFO_ID_BATTERY_MANUFACTURER 3

/*
 * ACPI addresses 0x20 - 0xff map to EC_MEMMAP offset 0x00 - 0xdf.  This data
 * is read-only from the AP.  Added in EC_ACPI_MEM_VERSION 2.
 */
#define EC_ACPI_MEM_MAPPED_BEGIN 0x20
#define EC_ACPI_MEM_MAPPED_SIZE 0xe0

/* Current version of ACPI memory address space */
#define EC_ACPI_MEM_VERSION_CURRENT 2

/*
 * This header file is used in coreboot both in C and ACPI code.  The ACPI code
 * is pre-processed to handle constants but the ASL compiler is unable to
 * handle actual C code so keep it separate.
 */
#ifndef __ACPI__

#ifndef __KERNEL__
/*
 * Define __packed if someone hasn't beat us to it.  Linux kernel style
 * checking prefers __packed over __attribute__((packed)).
 */
#ifndef __packed
#define __packed __attribute__((packed))
#endif

#ifndef __aligned
#define __aligned(x) __attribute__((aligned(x)))
#endif
#endif /* __KERNEL__ */

/*
 * Attributes for EC request and response packets.  Just defining __packed
 * results in inefficient assembly code on ARM, if the structure is actually
 * 32-bit aligned, as it should be for all buffers.
 *
 * Be very careful when adding these to existing structures.  They will round
 * up the structure size to the specified boundary.
 *
 * Also be very careful to make that if a structure is included in some other
 * parent structure that the alignment will still be true given the packing of
 * the parent structure.  This is particularly important if the sub-structure
 * will be passed as a pointer to another function, since that function will
 * not know about the misalignment caused by the parent structure's packing.
 *
 * Also be very careful using __packed - particularly when nesting non-packed
 * structures inside packed ones.  In fact, DO NOT use __packed directly;
 * always use one of these attributes.
 *
 * Once everything is annotated properly, the following search strings should
 * not return ANY matches in this file other than right here:
 *
 * "__packed" - generates inefficient code; all sub-structs must also be packed
 *
 * "struct [^_]" - all structs should be annotated, except for structs that are
 * members of other structs/unions (and their original declarations should be
 * annotated).
 */
#ifdef CONFIG_HOSTCMD_ALIGNED

/*
 * Packed structures where offset and size are always aligned to 1, 2, or 4
 * byte boundary.
 */
#define __ec_align1 __packed
#define __ec_align2 __packed __aligned(2)
#define __ec_align4 __packed __aligned(4)

/*
 * Packed structure which must be under-aligned, because its size is not a
 * 4-byte multiple.  This is sub-optimal because it forces byte-wise access
 * of all multi-byte fields in it, even though they are themselves aligned.
 *
 * In theory, we could duplicate the structure with __aligned(4) for accessing
 * its members, but use the __packed version for sizeof().
 */
#define __ec_align_size1 __packed

/*
 * Packed structure which must be under-aligned, because its offset inside a
 * parent structure is not a 4-byte multiple.
 */
#define __ec_align_offset1 __packed
#define __ec_align_offset2 __packed __aligned(2)

/*
 * Structures which are complicated enough that I'm skipping them on the first
 * pass.  They are effectively unchanged from their previous definitions.
 *
 * TODO(rspangler): Figure out what to do with these.  It's likely necessary
 * to work out the size and offset of each member and add explicit padding to
 * maintain those.
 */
#define __ec_todo_packed __packed
#define __ec_todo_unpacked

#else /* !CONFIG_HOSTCMD_ALIGNED */

/*
 * Packed structures make no assumption about alignment, so they do inefficient
 * byte-wise reads.
 */
#define __ec_align1 __packed
#define __ec_align2 __packed
#define __ec_align4 __packed
#define __ec_align_size1 __packed
#define __ec_align_offset1 __packed
#define __ec_align_offset2 __packed
#define __ec_todo_packed __packed
#define __ec_todo_unpacked

#endif /* !CONFIG_HOSTCMD_ALIGNED */

/* LPC command status byte masks */
/* EC has written a byte in the data register and host hasn't read it yet */
#define EC_LPC_STATUS_TO_HOST 0x01
/* Host has written a command/data byte and the EC hasn't read it yet */
#define EC_LPC_STATUS_FROM_HOST 0x02
/* EC is processing a command */
#define EC_LPC_STATUS_PROCESSING 0x04
/* Last write to EC was a command, not data */
#define EC_LPC_STATUS_LAST_CMD 0x08
/* EC is in burst mode */
#define EC_LPC_STATUS_BURST_MODE 0x10
/* SCI event is pending (requesting SCI query) */
#define EC_LPC_STATUS_SCI_PENDING 0x20
/* SMI event is pending (requesting SMI query) */
#define EC_LPC_STATUS_SMI_PENDING 0x40
/* (reserved) */
#define EC_LPC_STATUS_RESERVED 0x80

/*
 * EC is busy.  This covers both the EC processing a command, and the host has
 * written a new command but the EC hasn't picked it up yet.
 */
#define EC_LPC_STATUS_BUSY_MASK \
	(EC_LPC_STATUS_FROM_HOST | EC_LPC_STATUS_PROCESSING)

/*
 * Host command response codes (16-bit).
 */
enum ec_status {
	EC_RES_SUCCESS = 0,
	EC_RES_INVALID_COMMAND = 1,
	EC_RES_ERROR = 2,
	EC_RES_INVALID_PARAM = 3,
	EC_RES_ACCESS_DENIED = 4,
	EC_RES_INVALID_RESPONSE = 5,
	EC_RES_INVALID_VERSION = 6,
	EC_RES_INVALID_CHECKSUM = 7,
	EC_RES_IN_PROGRESS = 8, /* Accepted, command in progress */
	EC_RES_UNAVAILABLE = 9, /* No response available */
	EC_RES_TIMEOUT = 10, /* We got a timeout */
	EC_RES_OVERFLOW = 11, /* Table / data overflow */
	EC_RES_INVALID_HEADER = 12, /* Header contains invalid data */
	EC_RES_REQUEST_TRUNCATED = 13, /* Didn't get the entire request */
	EC_RES_RESPONSE_TOO_BIG = 14, /* Response was too big to handle */
	EC_RES_BUS_ERROR = 15, /* Communications bus error */
	EC_RES_BUSY = 16, /* Up but too busy.  Should retry */
	EC_RES_INVALID_HEADER_VERSION = 17, /* Header version invalid */
	EC_RES_INVALID_HEADER_CRC = 18, /* Header CRC invalid */
	EC_RES_INVALID_DATA_CRC = 19, /* Data CRC invalid */
	EC_RES_DUP_UNAVAILABLE = 20, /* Can't resend response */

	EC_RES_COUNT,

	EC_RES_MAX = UINT16_MAX, /**< Force enum to be 16 bits */
} __packed;
BUILD_ASSERT(sizeof(enum ec_status) == sizeof(uint16_t));
#ifdef CONFIG_EC_HOST_CMD
#ifdef CONFIG_ZEPHYR
/*
 * Make sure Zephyre uses the same status codes.
 */
#include <zephyr/mgmt/ec_host_cmd/ec_host_cmd.h>
#endif

BUILD_ASSERT((uint16_t)EC_RES_SUCCESS == (uint16_t)EC_HOST_CMD_SUCCESS);
BUILD_ASSERT((uint16_t)EC_RES_INVALID_COMMAND ==
	     (uint16_t)EC_HOST_CMD_INVALID_COMMAND);
BUILD_ASSERT((uint16_t)EC_RES_ERROR == (uint16_t)EC_HOST_CMD_ERROR);
BUILD_ASSERT((uint16_t)EC_RES_INVALID_PARAM ==
	     (uint16_t)EC_HOST_CMD_INVALID_PARAM);
BUILD_ASSERT((uint16_t)EC_RES_ACCESS_DENIED ==
	     (uint16_t)EC_HOST_CMD_ACCESS_DENIED);
BUILD_ASSERT((uint16_t)EC_RES_INVALID_RESPONSE ==
	     (uint16_t)EC_HOST_CMD_INVALID_RESPONSE);
BUILD_ASSERT((uint16_t)EC_RES_INVALID_VERSION ==
	     (uint16_t)EC_HOST_CMD_INVALID_VERSION);
BUILD_ASSERT((uint16_t)EC_RES_INVALID_CHECKSUM ==
	     (uint16_t)EC_HOST_CMD_INVALID_CHECKSUM);
BUILD_ASSERT((uint16_t)EC_RES_IN_PROGRESS == (uint16_t)EC_HOST_CMD_IN_PROGRESS);
BUILD_ASSERT((uint16_t)EC_RES_UNAVAILABLE == (uint16_t)EC_HOST_CMD_UNAVAILABLE);
BUILD_ASSERT((uint16_t)EC_RES_TIMEOUT == (uint16_t)EC_HOST_CMD_TIMEOUT);
BUILD_ASSERT((uint16_t)EC_RES_OVERFLOW == (uint16_t)EC_HOST_CMD_OVERFLOW);
BUILD_ASSERT((uint16_t)EC_RES_INVALID_HEADER ==
	     (uint16_t)EC_HOST_CMD_INVALID_HEADER);
BUILD_ASSERT((uint16_t)EC_RES_REQUEST_TRUNCATED ==
	     (uint16_t)EC_HOST_CMD_REQUEST_TRUNCATED);
BUILD_ASSERT((uint16_t)EC_RES_RESPONSE_TOO_BIG ==
	     (uint16_t)EC_HOST_CMD_RESPONSE_TOO_BIG);
BUILD_ASSERT((uint16_t)EC_RES_BUS_ERROR == (uint16_t)EC_HOST_CMD_BUS_ERROR);
BUILD_ASSERT((uint16_t)EC_RES_BUSY == (uint16_t)EC_HOST_CMD_BUSY);
BUILD_ASSERT((uint16_t)EC_RES_INVALID_HEADER_VERSION ==
	     (uint16_t)EC_HOST_CMD_INVALID_HEADER_VERSION);
BUILD_ASSERT((uint16_t)EC_RES_INVALID_HEADER_CRC ==
	     (uint16_t)EC_HOST_CMD_INVALID_HEADER_CRC);
BUILD_ASSERT((uint16_t)EC_RES_INVALID_DATA_CRC ==
	     (uint16_t)EC_HOST_CMD_INVALID_DATA_CRC);
BUILD_ASSERT((uint16_t)EC_RES_DUP_UNAVAILABLE ==
	     (uint16_t)EC_HOST_CMD_DUP_UNAVAILABLE);
BUILD_ASSERT((uint16_t)EC_RES_MAX == (uint16_t)EC_HOST_CMD_MAX);

#endif

/* clang-format off */
#define EC_STATUS_TEXT                                                        \
	{                                                                     \
	EC_MAP_ITEM(EC_RES_SUCCESS, SUCCESS),                                 \
	EC_MAP_ITEM(EC_RES_INVALID_COMMAND, INVALID_COMMAND),                 \
	EC_MAP_ITEM(EC_RES_ERROR, ERROR),                                     \
	EC_MAP_ITEM(EC_RES_INVALID_PARAM, INVALID_PARAM),                     \
	EC_MAP_ITEM(EC_RES_ACCESS_DENIED, ACCESS_DENIED),                     \
	EC_MAP_ITEM(EC_RES_INVALID_RESPONSE, INVALID_RESPONSE),               \
	EC_MAP_ITEM(EC_RES_INVALID_VERSION, INVALID_VERSION),                 \
	EC_MAP_ITEM(EC_RES_INVALID_CHECKSUM, INVALID_CHECKSUM),               \
	EC_MAP_ITEM(EC_RES_IN_PROGRESS, IN_PROGRESS),                         \
	EC_MAP_ITEM(EC_RES_UNAVAILABLE, UNAVAILABLE),                         \
	EC_MAP_ITEM(EC_RES_TIMEOUT, TIMEOUT),                                 \
	EC_MAP_ITEM(EC_RES_OVERFLOW, OVERFLOW),                               \
	EC_MAP_ITEM(EC_RES_INVALID_HEADER, INVALID_HEADER),                   \
	EC_MAP_ITEM(EC_RES_REQUEST_TRUNCATED, REQUEST_TRUNCATED),             \
	EC_MAP_ITEM(EC_RES_RESPONSE_TOO_BIG, RESPONSE_TOO_BIG),               \
	EC_MAP_ITEM(EC_RES_BUS_ERROR, BUS_ERROR),                             \
	EC_MAP_ITEM(EC_RES_BUSY, BUSY),                                       \
	EC_MAP_ITEM(EC_RES_INVALID_HEADER_VERSION, INVALID_HEADER_VERSION),   \
	EC_MAP_ITEM(EC_RES_INVALID_HEADER_CRC, INVALID_HEADER_CRC),           \
	EC_MAP_ITEM(EC_RES_INVALID_DATA_CRC, INVALID_DATA_CRC),               \
	EC_MAP_ITEM(EC_RES_DUP_UNAVAILABLE, DUP_UNAVAILABLE),                 \
	}
/* clang-format on */

#ifndef __cplusplus
#define EC_MAP_ITEM(k, v) [k] = #v
BUILD_ASSERT(ARRAY_SIZE(((const char *[])EC_STATUS_TEXT)) == EC_RES_COUNT);
#undef EC_MAP_ITEM
#endif

/*
 * Host event codes. ACPI query EC command uses code 0 to mean "no event
 * pending".  We explicitly specify each value in the enum listing so they won't
 * change if we delete/insert an item or rearrange the list (it needs to be
 * stable across platforms, not just within a single compiled instance).
 */
enum host_event_code {
	EC_HOST_EVENT_NONE = 0,
	EC_HOST_EVENT_LID_CLOSED = 1,
	EC_HOST_EVENT_LID_OPEN = 2,
	EC_HOST_EVENT_POWER_BUTTON = 3,
	EC_HOST_EVENT_AC_CONNECTED = 4,
	EC_HOST_EVENT_AC_DISCONNECTED = 5,
	EC_HOST_EVENT_BATTERY_LOW = 6,
	EC_HOST_EVENT_BATTERY_CRITICAL = 7,
	EC_HOST_EVENT_BATTERY = 8,
	EC_HOST_EVENT_THERMAL_THRESHOLD = 9,
	/* Event generated by a device attached to the EC */
	EC_HOST_EVENT_DEVICE = 10,
	EC_HOST_EVENT_THERMAL = 11,
	/* GPU related event. Formerly named EC_HOST_EVENT_USB_CHARGER. */
	EC_HOST_EVENT_GPU = 12,
	EC_HOST_EVENT_KEY_PRESSED = 13,
	/*
	 * EC has finished initializing the host interface.  The host can check
	 * for this event following sending a EC_CMD_REBOOT_EC command to
	 * determine when the EC is ready to accept subsequent commands.
	 */
	EC_HOST_EVENT_INTERFACE_READY = 14,
	/* Keyboard recovery combo has been pressed */
	EC_HOST_EVENT_KEYBOARD_RECOVERY = 15,

	/* Shutdown due to thermal overload */
	EC_HOST_EVENT_THERMAL_SHUTDOWN = 16,
	/* Shutdown due to battery level too low */
	EC_HOST_EVENT_BATTERY_SHUTDOWN = 17,

	/* Suggest that the AP throttle itself */
	EC_HOST_EVENT_THROTTLE_START = 18,
	/* Suggest that the AP resume normal speed */
	EC_HOST_EVENT_THROTTLE_STOP = 19,

	/* Hang detect logic detected a hang and host event timeout expired */
	EC_HOST_EVENT_HANG_DETECT = 20,
	/* Hang detect logic detected a hang and warm rebooted the AP */
	EC_HOST_EVENT_HANG_REBOOT = 21,

	/* PD MCU triggering host event */
	EC_HOST_EVENT_PD_MCU = 22,

	/* Battery Status flags have changed */
	EC_HOST_EVENT_BATTERY_STATUS = 23,

	/* EC encountered a panic, triggering a reset */
	EC_HOST_EVENT_PANIC = 24,

	/* Keyboard fastboot combo has been pressed */
	EC_HOST_EVENT_KEYBOARD_FASTBOOT = 25,

	/* EC RTC event occurred */
	EC_HOST_EVENT_RTC = 26,

	/* Emulate MKBP event */
	EC_HOST_EVENT_MKBP = 27,

	/* EC desires to change state of host-controlled USB mux */
	EC_HOST_EVENT_USB_MUX = 28,

	/*
	 * The device has changed "modes". This can be one of the following:
	 *
	 * - TABLET/LAPTOP mode
	 * - detachable base attach/detach event
	 * - on body/off body transition event
	 */
	EC_HOST_EVENT_MODE_CHANGE = 29,

	/* Keyboard recovery combo with hardware reinitialization */
	EC_HOST_EVENT_KEYBOARD_RECOVERY_HW_REINIT = 30,

	/* WoV */
	EC_HOST_EVENT_WOV = 31,

	/*
	 * The high bit of the event mask is not used as a host event code.  If
	 * it reads back as set, then the entire event mask should be
	 * considered invalid by the host.  This can happen when reading the
	 * raw event status via EC_MEMMAP_HOST_EVENTS but the LPC interface is
	 * not initialized on the EC, or improperly configured on the host.
	 */
	EC_HOST_EVENT_INVALID = 32,

	/* Body detect (lap/desk) change event */
	EC_HOST_EVENT_BODY_DETECT_CHANGE = 33,

	/* New console logs since last snapshot */
	EC_HOST_EVENT_CONSOLE_LOGS = 34,

	/*
	 * Only 64 host events are supported. This enum uses 1-based counting so
	 * it can skip 0 (NONE), so the last legal host event number is 64.
	 */
};

/* Host event mask */
#define EC_HOST_EVENT_MASK(event_code) BIT_ULL((event_code) - 1)

/* clang-format off */
#define HOST_EVENT_TEXT                                                        \
	{                                                                      \
		[EC_HOST_EVENT_NONE] = "NONE",                                 \
		[EC_HOST_EVENT_LID_CLOSED] = "LID_CLOSED",                     \
		[EC_HOST_EVENT_LID_OPEN] = "LID_OPEN",                         \
		[EC_HOST_EVENT_POWER_BUTTON] = "POWER_BUTTON",                 \
		[EC_HOST_EVENT_AC_CONNECTED] = "AC_CONNECTED",                 \
		[EC_HOST_EVENT_AC_DISCONNECTED] = "AC_DISCONNECTED",           \
		[EC_HOST_EVENT_BATTERY_LOW] = "BATTERY_LOW",                   \
		[EC_HOST_EVENT_BATTERY_CRITICAL] = "BATTERY_CRITICAL",         \
		[EC_HOST_EVENT_BATTERY] = "BATTERY",                           \
		[EC_HOST_EVENT_THERMAL_THRESHOLD] = "THERMAL_THRESHOLD",       \
		[EC_HOST_EVENT_DEVICE] = "DEVICE",                             \
		[EC_HOST_EVENT_THERMAL] = "THERMAL",                           \
		[EC_HOST_EVENT_GPU] = "GPU",                                   \
		[EC_HOST_EVENT_KEY_PRESSED] = "KEY_PRESSED",                   \
		[EC_HOST_EVENT_INTERFACE_READY] = "INTERFACE_READY",           \
		[EC_HOST_EVENT_KEYBOARD_RECOVERY] = "KEYBOARD_RECOVERY",       \
		[EC_HOST_EVENT_THERMAL_SHUTDOWN] = "THERMAL_SHUTDOWN",         \
		[EC_HOST_EVENT_BATTERY_SHUTDOWN] = "BATTERY_SHUTDOWN",         \
		[EC_HOST_EVENT_THROTTLE_START] = "THROTTLE_START",             \
		[EC_HOST_EVENT_THROTTLE_STOP] = "THROTTLE_STOP",               \
		[EC_HOST_EVENT_HANG_DETECT] = "HANG_DETECT",                   \
		[EC_HOST_EVENT_HANG_REBOOT] = "HANG_REBOOT",                   \
		[EC_HOST_EVENT_PD_MCU] = "PD_MCU",                             \
		[EC_HOST_EVENT_BATTERY_STATUS] = "BATTERY_STATUS",             \
		[EC_HOST_EVENT_PANIC] = "PANIC",                               \
		[EC_HOST_EVENT_KEYBOARD_FASTBOOT] = "KEYBOARD_FASTBOOT",       \
		[EC_HOST_EVENT_RTC] = "RTC",                                   \
		[EC_HOST_EVENT_MKBP] = "MKBP",                                 \
		[EC_HOST_EVENT_USB_MUX] = "USB_MUX",                           \
		[EC_HOST_EVENT_MODE_CHANGE] = "MODE_CHANGE",                   \
		[EC_HOST_EVENT_KEYBOARD_RECOVERY_HW_REINIT] =                  \
			"KEYBOARD_RECOVERY_HW_REINIT",                         \
		[EC_HOST_EVENT_WOV] = "WOV",                                   \
		[EC_HOST_EVENT_INVALID] = "INVALID",                           \
		[EC_HOST_EVENT_BODY_DETECT_CHANGE] = "BODY_DETECT_CHANGE",     \
		[EC_HOST_EVENT_CONSOLE_LOGS] = "CONSOLE_LOGS",                 \
	}
/* clang-format on */

/**
 * struct ec_lpc_host_args - Arguments at EC_LPC_ADDR_HOST_ARGS
 * @flags: The host argument flags.
 * @command_version: Command version.
 * @data_size: The length of data.
 * @checksum: Checksum; sum of command + flags + command_version + data_size +
 *            all params/response data bytes.
 */
struct ec_lpc_host_args {
	uint8_t flags;
	uint8_t command_version;
	uint8_t data_size;
	uint8_t checksum;
} __ec_align4;

/* Flags for ec_lpc_host_args.flags */
/*
 * Args are from host.  Data area at EC_LPC_ADDR_HOST_PARAM contains command
 * params.
 *
 * If EC gets a command and this flag is not set, this is an old-style command.
 * Command version is 0 and params from host are at EC_LPC_ADDR_OLD_PARAM with
 * unknown length.  EC must respond with an old-style response (that is,
 * without setting EC_HOST_ARGS_FLAG_TO_HOST).
 */
#define EC_HOST_ARGS_FLAG_FROM_HOST 0x01
/*
 * Args are from EC.  Data area at EC_LPC_ADDR_HOST_PARAM contains response.
 *
 * If EC responds to a command and this flag is not set, this is an old-style
 * response.  Command version is 0 and response data from EC is at
 * EC_LPC_ADDR_OLD_PARAM with unknown length.
 */
#define EC_HOST_ARGS_FLAG_TO_HOST 0x02

/*****************************************************************************/
/*
 * Byte codes returned by EC over SPI interface.
 *
 * These can be used by the AP to debug the EC interface, and to determine
 * when the EC is not in a state where it will ever get around to responding
 * to the AP.
 *
 * Example of sequence of bytes read from EC for a current good transfer:
 *   1. -                  - AP asserts chip select (CS#)
 *   2. EC_SPI_OLD_READY   - AP sends first byte(s) of request
 *   3. -                  - EC starts handling CS# interrupt
 *   4. EC_SPI_RECEIVING   - AP sends remaining byte(s) of request
 *   5. EC_SPI_PROCESSING  - EC starts processing request; AP is clocking in
 *                           bytes looking for EC_SPI_FRAME_START
 *   6. -                  - EC finishes processing and sets up response
 *   7. EC_SPI_FRAME_START - AP reads frame byte
 *   8. (response packet)  - AP reads response packet
 *   9. EC_SPI_PAST_END    - Any additional bytes read by AP
 *   10 -                  - AP deasserts chip select
 *   11 -                  - EC processes CS# interrupt and sets up DMA for
 *                           next request
 *
 * If the AP is waiting for EC_SPI_FRAME_START and sees any value other than
 * the following byte values:
 *   EC_SPI_OLD_READY
 *   EC_SPI_RX_READY
 *   EC_SPI_RECEIVING
 *   EC_SPI_PROCESSING
 *
 * Then the EC found an error in the request, or was not ready for the request
 * and lost data.  The AP should give up waiting for EC_SPI_FRAME_START,
 * because the EC is unable to tell when the AP is done sending its request.
 */

/*
 * Framing byte which precedes a response packet from the EC.  After sending a
 * request, the AP will clock in bytes until it sees the framing byte, then
 * clock in the response packet.
 */
#define EC_SPI_FRAME_START 0xec

/*
 * Padding bytes which are clocked out after the end of a response packet.
 */
#define EC_SPI_PAST_END 0xed

/*
 * EC is ready to receive, and has ignored the byte sent by the AP. EC expects
 * that the AP will send a valid packet header (starting with
 * EC_COMMAND_PROTOCOL_3) in the next 32 bytes.
 *
 * NOTE: Some SPI configurations place the Most Significant Bit on SDO when
 *	 CS goes low. This macro has the Most Significant Bit set to zero,
 *	 so SDO will not be driven high when CS goes low.
 */
#define EC_SPI_RX_READY 0x78

/*
 * EC has started receiving the request from the AP, but hasn't started
 * processing it yet.
 */
#define EC_SPI_RECEIVING 0xf9

/* EC has received the entire request from the AP and is processing it. */
#define EC_SPI_PROCESSING 0xfa

/*
 * EC received bad data from the AP, such as a packet header with an invalid
 * length.  EC will ignore all data until chip select deasserts.
 */
#define EC_SPI_RX_BAD_DATA 0xfb

/*
 * EC received data from the AP before it was ready.  That is, the AP asserted
 * chip select and started clocking data before the EC was ready to receive it.
 * EC will ignore all data until chip select deasserts.
 */
#define EC_SPI_NOT_READY 0xfc

/*
 * EC was ready to receive a request from the AP.  EC has treated the byte sent
 * by the AP as part of a request packet, or (for old-style ECs) is processing
 * a fully received packet but is not ready to respond yet.
 */
#define EC_SPI_OLD_READY 0xfd

/*****************************************************************************/

/*
 * Protocol version 2 for I2C and SPI send a request this way:
 *
 *	0	EC_CMD_VERSION0 + (command version)
 *	1	Command number
 *	2	Length of params = N
 *	3..N+2	Params, if any
 *	N+3	8-bit checksum of bytes 0..N+2
 *
 * The corresponding response is:
 *
 *	0	Result code (EC_RES_*)
 *	1	Length of params = M
 *	2..M+1	Params, if any
 *	M+2	8-bit checksum of bytes 0..M+1
 */
#define EC_PROTO2_REQUEST_HEADER_BYTES 3
#define EC_PROTO2_REQUEST_TRAILER_BYTES 1
#define EC_PROTO2_REQUEST_OVERHEAD \
	(EC_PROTO2_REQUEST_HEADER_BYTES + EC_PROTO2_REQUEST_TRAILER_BYTES)

#define EC_PROTO2_RESPONSE_HEADER_BYTES 2
#define EC_PROTO2_RESPONSE_TRAILER_BYTES 1
#define EC_PROTO2_RESPONSE_OVERHEAD \
	(EC_PROTO2_RESPONSE_HEADER_BYTES + EC_PROTO2_RESPONSE_TRAILER_BYTES)

/* Parameter length was limited by the LPC interface */
#define EC_PROTO2_MAX_PARAM_SIZE 0xfc

/* Maximum request and response packet sizes for protocol version 2 */
#define EC_PROTO2_MAX_REQUEST_SIZE \
	(EC_PROTO2_REQUEST_OVERHEAD + EC_PROTO2_MAX_PARAM_SIZE)
#define EC_PROTO2_MAX_RESPONSE_SIZE \
	(EC_PROTO2_RESPONSE_OVERHEAD + EC_PROTO2_MAX_PARAM_SIZE)

/*****************************************************************************/

/*
 * Value written to legacy command port / prefix byte to indicate protocol
 * 3+ structs are being used.  Usage is bus-dependent.
 */
#define EC_COMMAND_PROTOCOL_3 0xda

#define EC_HOST_REQUEST_VERSION 3

/**
 * struct ec_host_request - Version 3 request from host.
 * @struct_version: Should be 3. The EC will return EC_RES_INVALID_HEADER if it
 *                  receives a header with a version it doesn't know how to
 *                  parse.
 * @checksum: Checksum of request and data; sum of all bytes including checksum
 *            should total to 0.
 * @command: Command to send (EC_CMD_...)
 * @command_version: Command version.
 * @reserved: Unused byte in current protocol version; set to 0.
 * @data_len: Length of data which follows this header.
 */
struct ec_host_request {
	uint8_t struct_version;
	uint8_t checksum;
	uint16_t command;
	uint8_t command_version;
	uint8_t reserved;
	uint16_t data_len;
} __ec_align4;

#define EC_HOST_RESPONSE_VERSION 3

/**
 * struct ec_host_response - Version 3 response from EC.
 * @struct_version: Struct version (=3).
 * @checksum: Checksum of response and data; sum of all bytes including
 *            checksum should total to 0.
 * @result: EC's response to the command (separate from communication failure)
 * @data_len: Length of data which follows this header.
 * @reserved: Unused bytes in current protocol version; set to 0.
 */
struct ec_host_response {
	uint8_t struct_version;
	uint8_t checksum;
	uint16_t result;
	uint16_t data_len;
	uint16_t reserved;
} __ec_align4;

/*****************************************************************************/

/*
 * Host command protocol V4.
 *
 * Packets always start with a request or response header.  They are followed
 * by data_len bytes of data.  If the data_crc_present flag is set, the data
 * bytes are followed by a CRC-8 of that data, using x^8 + x^2 + x + 1
 * polynomial.
 *
 * Host algorithm when sending a request q:
 *
 * 101) tries_left=(some value, e.g. 3);
 * 102) q.seq_num++
 * 103) q.seq_dup=0
 * 104) Calculate q.header_crc.
 * 105) Send request q to EC.
 * 106) Wait for response r.  Go to 201 if received or 301 if timeout.
 *
 * 201) If r.struct_version != 4, go to 301.
 * 202) If r.header_crc mismatches calculated CRC for r header, go to 301.
 * 203) If r.data_crc_present and r.data_crc mismatches, go to 301.
 * 204) If r.seq_num != q.seq_num, go to 301.
 * 205) If r.seq_dup == q.seq_dup, return success.
 * 207) If r.seq_dup == 1, go to 301.
 * 208) Return error.
 *
 * 301) If --tries_left <= 0, return error.
 * 302) If q.seq_dup == 1, go to 105.
 * 303) q.seq_dup = 1
 * 304) Go to 104.
 *
 * EC algorithm when receiving a request q.
 * EC has response buffer r, error buffer e.
 *
 * 101) If q.struct_version != 4, set e.result = EC_RES_INVALID_HEADER_VERSION
 *      and go to 301
 * 102) If q.header_crc mismatches calculated CRC, set e.result =
 *      EC_RES_INVALID_HEADER_CRC and go to 301
 * 103) If q.data_crc_present, calculate data CRC.  If that mismatches the CRC
 *      byte at the end of the packet, set e.result = EC_RES_INVALID_DATA_CRC
 *      and go to 301.
 * 104) If q.seq_dup == 0, go to 201.
 * 105) If q.seq_num != r.seq_num, go to 201.
 * 106) If q.seq_dup == r.seq_dup, go to 205, else go to 203.
 *
 * 201) Process request q into response r.
 * 202) r.seq_num = q.seq_num
 * 203) r.seq_dup = q.seq_dup
 * 204) Calculate r.header_crc
 * 205) If r.data_len > 0 and data is no longer available, set e.result =
 *      EC_RES_DUP_UNAVAILABLE and go to 301.
 * 206) Send response r.
 *
 * 301) e.seq_num = q.seq_num
 * 302) e.seq_dup = q.seq_dup
 * 303) Calculate e.header_crc.
 * 304) Send error response e.
 */

/* Version 4 request from host */
struct ec_host_request4 {
	/*
	 * bits 0-3: struct_version: Structure version (=4)
	 * bit    4: is_response: Is response (=0)
	 * bits 5-6: seq_num: Sequence number
	 * bit    7: seq_dup: Sequence duplicate flag
	 */
	uint8_t fields0;

	/*
	 * bits 0-4: command_version: Command version
	 * bits 5-6: Reserved (set 0, ignore on read)
	 * bit    7: data_crc_present: Is data CRC present after data
	 */
	uint8_t fields1;

	/* Command code (EC_CMD_*) */
	uint16_t command;

	/* Length of data which follows this header (not including data CRC) */
	uint16_t data_len;

	/* Reserved (set 0, ignore on read) */
	uint8_t reserved;

	/* CRC-8 of above fields, using x^8 + x^2 + x + 1 polynomial */
	uint8_t header_crc;
} __ec_align4;

/* Version 4 response from EC */
struct ec_host_response4 {
	/*
	 * bits 0-3: struct_version: Structure version (=4)
	 * bit    4: is_response: Is response (=1)
	 * bits 5-6: seq_num: Sequence number
	 * bit    7: seq_dup: Sequence duplicate flag
	 */
	uint8_t fields0;

	/*
	 * bits 0-6: Reserved (set 0, ignore on read)
	 * bit    7: data_crc_present: Is data CRC present after data
	 */
	uint8_t fields1;

	/* Result code (EC_RES_*) */
	uint16_t result;

	/* Length of data which follows this header (not including data CRC) */
	uint16_t data_len;

	/* Reserved (set 0, ignore on read) */
	uint8_t reserved;

	/* CRC-8 of above fields, using x^8 + x^2 + x + 1 polynomial */
	uint8_t header_crc;
} __ec_align4;

/* Fields in fields0 byte */
#define EC_PACKET4_0_STRUCT_VERSION_MASK 0x0f
#define EC_PACKET4_0_IS_RESPONSE_MASK 0x10
#define EC_PACKET4_0_SEQ_NUM_SHIFT 5
#define EC_PACKET4_0_SEQ_NUM_MASK 0x60
#define EC_PACKET4_0_SEQ_DUP_MASK 0x80

/* Fields in fields1 byte */
#define EC_PACKET4_1_COMMAND_VERSION_MASK 0x1f /* (request only) */
#define EC_PACKET4_1_DATA_CRC_PRESENT_MASK 0x80

/*****************************************************************************/
/*
 * Notes on commands:
 *
 * Each command is an 16-bit command value.  Commands which take params or
 * return response data specify structures for that data.  If no structure is
 * specified, the command does not input or output data, respectively.
 * Parameter/response length is implicit in the structs.  Some underlying
 * communication protocols (I2C, SPI) may add length or checksum headers, but
 * those are implementation-dependent and not defined here.
 *
 * All commands MUST be #defined to be 4-digit UPPER CASE hex values
 * (e.g., 0x00AB, not 0xab) for CONFIG_HOSTCMD_SECTION_SORTED to work.
 */

/*****************************************************************************/
/* General / test commands */

/*
 * Get protocol version, used to deal with non-backward compatible protocol
 * changes.
 */
#define EC_CMD_PROTO_VERSION 0x0000

/**
 * struct ec_response_proto_version - Response to the proto version command.
 * @version: The protocol version.
 */
struct ec_response_proto_version {
	uint32_t version;
} __ec_align4;

/*
 * Hello.  This is a simple command to test the EC is responsive to
 * commands.
 */
#define EC_CMD_HELLO 0x0001

/**
 * struct ec_params_hello - Parameters to the hello command.
 * @in_data: Pass anything here.
 */
struct ec_params_hello {
	uint32_t in_data;
} __ec_align4;

/**
 * struct ec_response_hello - Response to the hello command.
 * @out_data: Output will be in_data + 0x01020304.
 */
struct ec_response_hello {
	uint32_t out_data;
} __ec_align4;

/* Get version number */
#define EC_CMD_GET_VERSION 0x0002

enum ec_image {
	EC_IMAGE_UNKNOWN = 0,
	EC_IMAGE_RO,
	EC_IMAGE_RW,
	EC_IMAGE_RW_A = EC_IMAGE_RW,
	EC_IMAGE_RO_B,
	EC_IMAGE_RW_B,
};

/**
 * struct ec_response_get_version - Response to the v0 get version command.
 * @version_string_ro: Null-terminated RO firmware version string.
 * @version_string_rw: Null-terminated RW firmware version string.
 * @reserved: Unused bytes; was previously RW-B firmware version string.
 * @current_image: One of ec_image.
 */
struct ec_response_get_version {
	char version_string_ro[32];
	char version_string_rw[32];
	char reserved[32]; /* Changed to cros_fwid_ro in version 1 */
	uint32_t current_image;
} __ec_align4;

/**
 * struct ec_response_get_version_v1 - Response to the v1 get version command.
 *
 * ec_response_get_version_v1 is a strict superset of ec_response_get_version.
 * The v1 response changes the semantics of one field (reserved to cros_fwid_ro)
 * and adds one additional field (cros_fwid_rw).
 *
 * @version_string_ro: Null-terminated RO firmware version string.
 * @version_string_rw: Null-terminated RW firmware version string.
 * @cros_fwid_ro: Null-terminated RO CrOS FWID string.
 * @current_image: One of ec_image.
 * @cros_fwid_rw: Null-terminated RW CrOS FWID string.
 */
struct ec_response_get_version_v1 {
	char version_string_ro[32];
	char version_string_rw[32];
	char cros_fwid_ro[32]; /* Added in version 1 (Used to be reserved) */
	uint32_t current_image;
	char cros_fwid_rw[32]; /* Added in version 1 */
} __ec_align4;

/* Read test - OBSOLETE */
#define EC_CMD_READ_TEST 0x0003

/*
 * Get build information
 *
 * Response is null-terminated string.
 */
#define EC_CMD_GET_BUILD_INFO 0x0004

/* Get chip info */
#define EC_CMD_GET_CHIP_INFO 0x0005

/**
 * struct ec_response_get_chip_info - Response to the get chip info command.
 * @vendor: Null-terminated string for chip vendor.
 * @name: Null-terminated string for chip name.
 * @revision: Null-terminated string for chip mask version.
 */
struct ec_response_get_chip_info {
	char vendor[32];
	char name[32];
	char revision[32];
} __ec_align4;

/* Get board HW version */
#define EC_CMD_GET_BOARD_VERSION 0x0006

/**
 * struct ec_response_board_version - Response to the board version command.
 * @board_version: A monotonously incrementing number.
 */
struct ec_response_board_version {
	uint16_t board_version;
} __ec_align2;

/*
 * Read memory-mapped data.
 *
 * This is an alternate interface to memory-mapped data for bus protocols
 * which don't support direct-mapped memory - I2C, SPI, etc.
 *
 * Response is params.size bytes of data.
 */
#define EC_CMD_READ_MEMMAP 0x0007

/**
 * struct ec_params_read_memmap - Parameters for the read memory map command.
 * @offset: Offset in memmap (EC_MEMMAP_*).
 * @size: Size to read in bytes.
 */
struct ec_params_read_memmap {
	uint8_t offset;
	uint8_t size;
} __ec_align1;

/* Read versions supported for a command */
#define EC_CMD_GET_CMD_VERSIONS 0x0008

/**
 * struct ec_params_get_cmd_versions - Parameters for the get command versions.
 * @cmd: Command to check.
 */
struct ec_params_get_cmd_versions {
	uint8_t cmd;
} __ec_align1;

/**
 * struct ec_params_get_cmd_versions_v1 - Parameters for the get command
 *         versions (v1)
 * @cmd: Command to check.
 */
struct ec_params_get_cmd_versions_v1 {
	uint16_t cmd;
} __ec_align2;

/**
 * struct ec_response_get_cmd_version - Response to the get command versions.
 * @version_mask: Mask of supported versions; use EC_VER_MASK() to compare with
 *                a desired version.
 */
struct ec_response_get_cmd_versions {
	uint32_t version_mask;
} __ec_align4;

/*
 * Check EC communications status (busy). This is needed on i2c/spi but not
 * on lpc since it has its own out-of-band busy indicator.
 *
 * lpc must read the status from the command register. Attempting this on
 * lpc will overwrite the args/parameter space and corrupt its data.
 */
#define EC_CMD_GET_COMMS_STATUS 0x0009

/* Avoid using ec_status which is for return values */
enum ec_comms_status {
	EC_COMMS_STATUS_PROCESSING = BIT(0), /* Processing cmd */
};

/**
 * struct ec_response_get_comms_status - Response to the get comms status
 *         command.
 * @flags: Mask of enum ec_comms_status.
 */
struct ec_response_get_comms_status {
	uint32_t flags; /* Mask of enum ec_comms_status */
} __ec_align4;

/* Fake a variety of responses, purely for testing purposes. */
#define EC_CMD_TEST_PROTOCOL 0x000A

/* Tell the EC what to send back to us. */
struct ec_params_test_protocol {
	uint32_t ec_result;
	uint32_t ret_len;
	uint8_t buf[32];
} __ec_align4;

/* Here it comes... */
struct ec_response_test_protocol {
	uint8_t buf[32];
} __ec_align4;

/* Get protocol information */
#define EC_CMD_GET_PROTOCOL_INFO 0x000B

/* Flags for ec_response_get_protocol_info.flags */
/* EC_RES_IN_PROGRESS may be returned if a command is slow */
#define EC_PROTOCOL_INFO_IN_PROGRESS_SUPPORTED BIT(0)

/**
 * struct ec_response_get_protocol_info - Response to the get protocol info.
 * @protocol_versions: Bitmask of protocol versions supported (1 << n means
 *                     version n).
 * @max_request_packet_size: Maximum request packet size in bytes.
 * @max_response_packet_size: Maximum response packet size in bytes.
 * @flags: see EC_PROTOCOL_INFO_*
 */
struct ec_response_get_protocol_info {
	/* Fields which exist if at least protocol version 3 supported */
	uint32_t protocol_versions;
	uint16_t max_request_packet_size;
	uint16_t max_response_packet_size;
	uint32_t flags;
} __ec_align4;

/*****************************************************************************/
/* Get/Set miscellaneous values */

/* The upper byte of .flags tells what to do (nothing means "get") */
#define EC_GSV_SET 0x80000000

/*
 * The lower three bytes of .flags identifies the parameter, if that has
 * meaning for an individual command.
 */
#define EC_GSV_PARAM_MASK 0x00ffffff

struct ec_params_get_set_value {
	uint32_t flags;
	uint32_t value;
} __ec_align4;

struct ec_response_get_set_value {
	uint32_t flags;
	uint32_t value;
} __ec_align4;

/* More than one command can use these structs to get/set parameters. */
#define EC_CMD_GSV_PAUSE_IN_S5 0x000C

/*****************************************************************************/
/* List the features supported by the firmware */
#define EC_CMD_GET_FEATURES 0x000D

/* Supported features */
enum ec_feature_code {
	/*
	 * This image contains a limited set of features. Another image
	 * in RW partition may support more features.
	 */
	EC_FEATURE_LIMITED = 0,
	/*
	 * Commands for probing/reading/writing/erasing the flash in the
	 * EC are present.
	 */
	EC_FEATURE_FLASH = 1,
	/*
	 * Can control the fan speed directly.
	 */
	EC_FEATURE_PWM_FAN = 2,
	/*
	 * Can control the intensity of the keyboard backlight.
	 */
	EC_FEATURE_PWM_KEYB = 3,
	/*
	 * Support Google lightbar, introduced on Pixel.
	 */
	EC_FEATURE_LIGHTBAR = 4,
	/* Control of LEDs  */
	EC_FEATURE_LED = 5,
	/* Exposes an interface to control gyro and sensors.
	 * The host goes through the EC to access these sensors.
	 * In addition, the EC may provide composite sensors, like lid angle.
	 */
	EC_FEATURE_MOTION_SENSE = 6,
	/* The keyboard is controlled by the EC */
	EC_FEATURE_KEYB = 7,
	/* The AP can use part of the EC flash as persistent storage. */
	EC_FEATURE_PSTORE = 8,
	/* The EC monitors BIOS port 80h, and can return POST codes. */
	EC_FEATURE_PORT80 = 9,
	/*
	 * Thermal management: include TMP specific commands.
	 * Higher level than direct fan control.
	 */
	EC_FEATURE_THERMAL = 10,
	/* Can switch the screen backlight on/off */
	EC_FEATURE_BKLIGHT_SWITCH = 11,
	/* Can switch the wifi module on/off */
	EC_FEATURE_WIFI_SWITCH = 12,
	/* Monitor host events, through for example SMI or SCI */
	EC_FEATURE_HOST_EVENTS = 13,
	/* The EC exposes GPIO commands to control/monitor connected devices. */
	EC_FEATURE_GPIO = 14,
	/* The EC can send i2c messages to downstream devices. */
	EC_FEATURE_I2C = 15,
	/* Command to control charger are included */
	EC_FEATURE_CHARGER = 16,
	/* Simple battery support. */
	EC_FEATURE_BATTERY = 17,
	/*
	 * Support Smart battery protocol
	 * (Common Smart Battery System Interface Specification)
	 */
	EC_FEATURE_SMART_BATTERY = 18,
	/* EC can detect when the host hangs. */
	EC_FEATURE_HANG_DETECT = 19,
	/* Report power information, for pit only */
	EC_FEATURE_PMU = 20,
	/* Another Cros EC device is present downstream of this one */
	EC_FEATURE_SUB_MCU = 21,
	/* Support USB Power delivery (PD) commands */
	EC_FEATURE_USB_PD = 22,
	/* Control USB multiplexer, for audio through USB port for instance. */
	EC_FEATURE_USB_MUX = 23,
	/* Motion Sensor code has an internal software FIFO */
	EC_FEATURE_MOTION_SENSE_FIFO = 24,
	/* Support temporary secure vstore */
	EC_FEATURE_VSTORE = 25,
	/* EC decides on USB-C SS mux state, muxes configured by host */
	EC_FEATURE_USBC_SS_MUX_VIRTUAL = 26,
	/* EC has RTC feature that can be controlled by host commands */
	EC_FEATURE_RTC = 27,
	/* The MCU exposes a Fingerprint sensor */
	EC_FEATURE_FINGERPRINT = 28,
	/* The MCU exposes a Touchpad */
	EC_FEATURE_TOUCHPAD = 29,
	/* The MCU has RWSIG task enabled */
	EC_FEATURE_RWSIG = 30,
	/* EC has device events support */
	EC_FEATURE_DEVICE_EVENT = 31,
	/* EC supports the unified wake masks for LPC/eSPI systems */
	EC_FEATURE_UNIFIED_WAKE_MASKS = 32,
	/* EC supports 64-bit host events */
	EC_FEATURE_HOST_EVENT64 = 33,
	/* EC runs code in RAM (not in place, a.k.a. XIP) */
	EC_FEATURE_EXEC_IN_RAM = 34,
	/* EC supports CEC commands */
	EC_FEATURE_CEC = 35,
	/* EC supports tight sensor timestamping. */
	EC_FEATURE_MOTION_SENSE_TIGHT_TIMESTAMPS = 36,
	/*
	 * EC supports tablet mode detection aligned to Chrome and allows
	 * setting of threshold by host command using
	 * MOTIONSENSE_CMD_TABLET_MODE_LID_ANGLE.
	 */
	EC_FEATURE_REFINED_TABLET_MODE_HYSTERESIS = 37,
	/*
	 * Early Firmware Selection ver.2. Enabled by CONFIG_VBOOT_EFS2.
	 * Note this is a RO feature. So, a query (EC_CMD_GET_FEATURES) should
	 * be sent to RO to be precise.
	 */
	EC_FEATURE_EFS2 = 38,
	/* The MCU is a System Companion Processor (SCP). */
	EC_FEATURE_SCP = 39,
	/* The MCU is an Integrated Sensor Hub */
	EC_FEATURE_ISH = 40,
	/* New TCPMv2 TYPEC_ prefaced commands supported */
	EC_FEATURE_TYPEC_CMD = 41,
	/*
	 * The EC will wait for direction from the AP to enter Type-C alternate
	 * modes or USB4.
	 */
	EC_FEATURE_TYPEC_REQUIRE_AP_MODE_ENTRY = 42,
	/*
	 * The EC will wait for an acknowledge from the AP after setting the
	 * mux.
	 */
	EC_FEATURE_TYPEC_MUX_REQUIRE_AP_ACK = 43,
	/*
	 * The EC supports entering and residing in S4.
	 */
	EC_FEATURE_S4_RESIDENCY = 44,
	/*
	 * The EC supports the AP directing mux sets for the board.
	 */
	EC_FEATURE_TYPEC_AP_MUX_SET = 45,
	/*
	 * The EC supports the AP composing VDMs for us to send.
	 */
	EC_FEATURE_TYPEC_AP_VDM_SEND = 46,
	/*
	 * The EC supports system safe mode panic recovery.
	 */
	EC_FEATURE_SYSTEM_SAFE_MODE = 47,
	/*
	 * The EC will reboot on runtime assertion failures.
	 */
	EC_FEATURE_ASSERT_REBOOTS = 48,
	/*
	 * The EC image is built with tokenized logging enabled.
	 */
	EC_FEATURE_TOKENIZED_LOGGING = 49,
	/*
	 * The EC supports triggering an STB dump.
	 */
	EC_FEATURE_AMD_STB_DUMP = 50,
	/*
	 * The EC supports memory dump commands.
	 */
	EC_FEATURE_MEMORY_DUMP = 51,
	/*
	 * The EC supports DP2.1 capability
	 */
	EC_FEATURE_TYPEC_DP2_1 = 52,
	/*
	 * The MCU is System Companion Processor Core 1
	 */
	EC_FEATURE_SCP_C1 = 53,
	/*
	 * The EC supports UCSI PPM.
	 */
	EC_FEATURE_UCSI_PPM = 54,
	/*
	 * The EC supports Strauss keyboard.
	 */
	EC_FEATURE_STRAUSS = 55,
	/*
	 * The EC supports PoE.
	 */
	EC_FEATURE_POE = 56,
	/*
	 * The EC supports a hybrid boost charger
	 */
	EC_FEATURE_CHARGER_HYBRID_POWER_BOOST = 57,
	/*
	 * Support signaling new console logs via host event
	 */
	EC_FEATURE_CONSOLE_LOG_EVENT = 58,
};

#define EC_FEATURE_MASK_0(event_code) BIT(event_code % 32)
#define EC_FEATURE_MASK_1(event_code) BIT(event_code - 32)

struct ec_response_get_features {
	uint32_t flags[2];
} __ec_align4;

/*****************************************************************************/
/* Get the board's SKU ID from EC */
#define EC_CMD_GET_SKU_ID 0x000E

/* Set SKU ID from AP */
#define EC_CMD_SET_SKU_ID 0x000F

struct ec_sku_id_info {
	uint32_t sku_id;
} __ec_align4;

/*****************************************************************************/
/* Flash commands */

/* Get flash info */
#define EC_CMD_FLASH_INFO 0x0010
#define EC_VER_FLASH_INFO 2

/**
 * struct ec_response_flash_info - Response to the flash info command.
 * @flash_size: Usable flash size in bytes.
 * @write_block_size: Write block size. Write offset and size must be a
 *                    multiple of this.
 * @erase_block_size: Erase block size. Erase offset and size must be a
 *                    multiple of this.
 * @protect_block_size: Protection block size. Protection offset and size
 *                      must be a multiple of this.
 *
 * Version 0 returns these fields.
 */
struct ec_response_flash_info {
	uint32_t flash_size;
	uint32_t write_block_size;
	uint32_t erase_block_size;
	uint32_t protect_block_size;
} __ec_align4;

/*
 * Flags for version 1+ flash info command
 * EC flash erases bits to 0 instead of 1.
 */
#define EC_FLASH_INFO_ERASE_TO_0 BIT(0)

/*
 * Flash must be selected for read/write/erase operations to succeed.  This may
 * be necessary on a chip where write/erase can be corrupted by other board
 * activity, or where the chip needs to enable some sort of programming voltage,
 * or where the read/write/erase operations require cleanly suspending other
 * chip functionality.
 */
#define EC_FLASH_INFO_SELECT_REQUIRED BIT(1)

/**
 * struct ec_response_flash_info_1 - Response to the flash info v1 command.
 * @flash_size: Usable flash size in bytes.
 * @write_block_size: Write block size. Write offset and size must be a
 *                    multiple of this.
 * @erase_block_size: Erase block size. Erase offset and size must be a
 *                    multiple of this.
 * @protect_block_size: Protection block size. Protection offset and size
 *                      must be a multiple of this.
 * @write_ideal_size: Ideal write size in bytes.  Writes will be fastest if
 *                    size is exactly this and offset is a multiple of this.
 *                    For example, an EC may have a write buffer which can do
 *                    half-page operations if data is aligned, and a slower
 *                    word-at-a-time write mode.
 * @flags: Flags; see EC_FLASH_INFO_*
 *
 * Version 1 returns the same initial fields as version 0, with additional
 * fields following.
 *
 * gcc anonymous structs don't seem to get along with the __packed directive;
 * if they did we'd define the version 0 structure as a sub-structure of this
 * one.
 *
 * Version 2 supports flash banks of different sizes:
 * The caller specified the number of banks it has preallocated
 * (num_banks_desc)
 * The EC returns the number of banks describing the flash memory.
 * It adds banks descriptions up to num_banks_desc.
 */
struct ec_response_flash_info_1 {
	/* Version 0 fields; see above for description */
	uint32_t flash_size;
	uint32_t write_block_size;
	uint32_t erase_block_size;
	uint32_t protect_block_size;

	/* Version 1 adds these fields: */
	uint32_t write_ideal_size;
	uint32_t flags;
} __ec_align4;

struct ec_params_flash_info_2 {
	/* Number of banks to describe */
	uint16_t num_banks_desc;
	/* Reserved; set 0; ignore on read */
	uint8_t reserved[2];
} __ec_align4;

struct ec_flash_bank {
	/* Number of sector is in this bank. */
	uint16_t count;
	/* Size in power of 2 of each sector (8 --> 256 bytes) */
	uint8_t size_exp;
	/* Minimal write size for the sectors in this bank */
	uint8_t write_size_exp;
	/* Erase size for the sectors in this bank */
	uint8_t erase_size_exp;
	/* Size for write protection, usually identical to erase size. */
	uint8_t protect_size_exp;
	/* Reserved; set 0; ignore on read */
	uint8_t reserved[2];
};

struct ec_response_flash_info_2 {
	/* Total flash in the EC. */
	uint32_t flash_size;
	/* Flags; see EC_FLASH_INFO_* */
	uint32_t flags;
	/* Maximum size to use to send data to write to the EC. */
	uint32_t write_ideal_size;
	/* Number of banks present in the EC. */
	uint16_t num_banks_total;
	/* Number of banks described in banks array. */
	uint16_t num_banks_desc;
	struct ec_flash_bank banks[FLEXIBLE_ARRAY_MEMBER_SIZE];
} __ec_align4;

/*
 * Read flash
 *
 * Response is params.size bytes of data.
 */
#define EC_CMD_FLASH_READ 0x0011

/**
 * struct ec_params_flash_read - Parameters for the flash read command.
 * @offset: Byte offset to read.
 * @size: Size to read in bytes.
 */
struct ec_params_flash_read {
	uint32_t offset;
	uint32_t size;
} __ec_align4;

/* Write flash */
#define EC_CMD_FLASH_WRITE 0x0012
#define EC_VER_FLASH_WRITE 1

/* Version 0 of the flash command supported only 64 bytes of data */
#define EC_FLASH_WRITE_VER0_SIZE 64

/**
 * struct ec_params_flash_write - Parameters for the flash write command.
 * @offset: Byte offset to write.
 * @size: Size to write in bytes.
 * @data: Data to write.
 * @data.words32: uint32_t data to write.
 * @data.bytes: uint8_t data to write.
 */
struct ec_params_flash_write {
	uint32_t offset;
	uint32_t size;
	/* Followed by data to write. This union allows accessing an
	 * underlying buffer as uint32s or uint8s for convenience.
	 */
	union {
		uint32_t words32[FLEXIBLE_ARRAY_MEMBER_SIZE];
		uint8_t bytes[FLEXIBLE_ARRAY_MEMBER_SIZE];
	} data;
} __ec_align4;
BUILD_ASSERT(member_size(struct ec_params_flash_write, data) == 0);

/* Erase flash */
#define EC_CMD_FLASH_ERASE 0x0013

/**
 * struct ec_params_flash_erase - Parameters for the flash erase command, v0.
 * @offset: Byte offset to erase.
 * @size: Size to erase in bytes.
 */
struct ec_params_flash_erase {
	uint32_t offset;
	uint32_t size;
} __ec_align4;

/*
 * v1 add async erase:
 * subcommands can returns:
 * EC_RES_SUCCESS : erased (see ERASE_SECTOR_ASYNC case below).
 * EC_RES_INVALID_PARAM : offset/size are not aligned on a erase boundary.
 * EC_RES_ERROR : other errors.
 * EC_RES_BUSY : an existing erase operation is in progress.
 * EC_RES_ACCESS_DENIED: Trying to erase running image.
 *
 * When ERASE_SECTOR_ASYNC returns EC_RES_SUCCESS, the operation is just
 * properly queued. The user must call ERASE_GET_RESULT subcommand to get
 * the proper result.
 * When ERASE_GET_RESULT returns EC_RES_BUSY, the caller must wait and send
 * ERASE_GET_RESULT again to get the result of ERASE_SECTOR_ASYNC.
 * ERASE_GET_RESULT command may timeout on EC where flash access is not
 * permitted while erasing. (For instance, STM32F4).
 */
enum ec_flash_erase_cmd {
	FLASH_ERASE_SECTOR, /* Erase and wait for result */
	FLASH_ERASE_SECTOR_ASYNC, /* Erase and return immediately. */
	FLASH_ERASE_GET_RESULT, /* Ask for last erase result */
};

/**
 * struct ec_params_flash_erase_v1 - Parameters for the flash erase command, v1.
 * @cmd: One of ec_flash_erase_cmd.
 * @reserved: Pad byte; currently always contains 0.
 * @flag: No flags defined yet; set to 0.
 * @params: Same as v0 parameters.
 */
struct ec_params_flash_erase_v1 {
	uint8_t cmd;
	uint8_t reserved;
	uint16_t flag;
	struct ec_params_flash_erase params;
} __ec_align4;

/*
 * Get/set flash protection.
 *
 * If mask!=0, sets/clear the requested bits of flags.  Depending on the
 * firmware write protect GPIO, not all flags will take effect immediately;
 * some flags require a subsequent hard reset to take effect.  Check the
 * returned flags bits to see what actually happened.
 *
 * If mask=0, simply returns the current flags state.
 */
#define EC_CMD_FLASH_PROTECT 0x0015
#define EC_VER_FLASH_PROTECT 1 /* Command version 1 */

/* Flags for flash protection */
/* RO flash code protected when the EC boots */
#define EC_FLASH_PROTECT_RO_AT_BOOT BIT(0)
/*
 * RO flash code protected now.  If this bit is set, at-boot status cannot
 * be changed.
 */
#define EC_FLASH_PROTECT_RO_NOW BIT(1)
/* Entire flash code protected now, until reboot. */
#define EC_FLASH_PROTECT_ALL_NOW BIT(2)
/* Flash write protect GPIO is asserted now */
#define EC_FLASH_PROTECT_GPIO_ASSERTED BIT(3)
/* Error - at least one bank of flash is stuck locked, and cannot be unlocked */
#define EC_FLASH_PROTECT_ERROR_STUCK BIT(4)
/*
 * Error - flash protection is in inconsistent state.  At least one bank of
 * flash which should be protected is not protected.  Usually fixed by
 * re-requesting the desired flags, or by a hard reset if that fails.
 */
#define EC_FLASH_PROTECT_ERROR_INCONSISTENT BIT(5)
/* Entire flash code protected when the EC boots */
#define EC_FLASH_PROTECT_ALL_AT_BOOT BIT(6)
/* RW flash code protected when the EC boots */
#define EC_FLASH_PROTECT_RW_AT_BOOT BIT(7)
/* RW flash code protected now. */
#define EC_FLASH_PROTECT_RW_NOW BIT(8)
/* Rollback information flash region protected when the EC boots */
#define EC_FLASH_PROTECT_ROLLBACK_AT_BOOT BIT(9)
/* Rollback information flash region protected now */
#define EC_FLASH_PROTECT_ROLLBACK_NOW BIT(10)
/* Error - Unknown error */
#define EC_FLASH_PROTECT_ERROR_UNKNOWN BIT(11)

/**
 * struct ec_params_flash_protect - Parameters for the flash protect command.
 * @mask: Bits in flags to apply.
 * @flags: New flags to apply.
 */
struct ec_params_flash_protect {
	uint32_t mask;
	uint32_t flags;
} __ec_align4;

enum flash_protect_action {
	FLASH_PROTECT_ASYNC = 0,
	FLASH_PROTECT_GET_RESULT = 1,
};

/* Version 2 of the command is "asynchronous". */
struct ec_params_flash_protect_v2 {
	uint8_t action; /**< enum flash_protect_action */
	uint8_t reserved[3]; /**< padding for alignment */
	uint32_t mask;
	uint32_t flags;
} __ec_align4;

/**
 * struct ec_response_flash_protect - Response to the flash protect command.
 * @flags: Current value of flash protect flags.
 * @valid_flags: Flags which are valid on this platform. This allows the
 *               caller to distinguish between flags which aren't set vs. flags
 *               which can't be set on this platform.
 * @writable_flags: Flags which can be changed given the current protection
 *                  state.
 */
struct ec_response_flash_protect {
	uint32_t flags;
	uint32_t valid_flags;
	uint32_t writable_flags;
} __ec_align4;

/*
 * Note: commands 0x14 - 0x19 version 0 were old commands to get/set flash
 * write protect.  These commands may be reused with version > 0.
 */

/* Get the region offset/size */
#define EC_CMD_FLASH_REGION_INFO 0x0016
#define EC_VER_FLASH_REGION_INFO 1

enum ec_flash_region {
	/* Region which holds read-only EC image */
	EC_FLASH_REGION_RO = 0,
	/*
	 * Region which holds active RW image. 'Active' is different from
	 * 'running'. Active means 'scheduled-to-run'. Since RO image always
	 * scheduled to run, active/non-active applies only to RW images (for
	 * the same reason 'update' applies only to RW images. It's a state of
	 * an image on a flash. Running image can be RO, RW_A, RW_B but active
	 * image can only be RW_A or RW_B. In recovery mode, an active RW image
	 * doesn't enter 'running' state but it's still active on a flash.
	 */
	EC_FLASH_REGION_ACTIVE,
	/*
	 * Region which should be write-protected in the factory (a superset of
	 * EC_FLASH_REGION_RO)
	 */
	EC_FLASH_REGION_WP_RO,
	/* Region which holds updatable (non-active) RW image */
	EC_FLASH_REGION_UPDATE,
	/* Number of regions */
	EC_FLASH_REGION_COUNT,
};
/*
 * 'RW' is vague if there are multiple RW images; we mean the active one,
 * so the old constant is deprecated.
 */
#define EC_FLASH_REGION_RW EC_FLASH_REGION_ACTIVE

/**
 * struct ec_params_flash_region_info - Parameters for the flash region info
 *         command.
 * @region: Flash region; see EC_FLASH_REGION_*
 */
struct ec_params_flash_region_info {
	uint32_t region;
} __ec_align4;

struct ec_response_flash_region_info {
	uint32_t offset;
	uint32_t size;
} __ec_align4;

/* Get SPI flash information */
#define EC_CMD_FLASH_SPI_INFO 0x0018

struct ec_response_flash_spi_info {
	/* JEDEC info from command 0x9F (manufacturer, memory type, size) */
	uint8_t jedec[3];

	/* Pad byte; currently always contains 0 */
	uint8_t reserved0;

	/* Manufacturer / device ID from command 0x90 */
	uint8_t mfr_dev_id[2];

	/* Status registers from command 0x05 and 0x35 */
	uint8_t sr1, sr2;
} __ec_align1;

/* Select flash during flash operations */
#define EC_CMD_FLASH_SELECT 0x0019

/**
 * struct ec_params_flash_select - Parameters for the flash select command.
 * @select: 1 to select flash, 0 to deselect flash
 */
struct ec_params_flash_select {
	uint8_t select;
} __ec_align4;

/**
 * Request random numbers to be generated and returned.
 * Can be used to test the random number generator is truly random.
 * See https://csrc.nist.gov/publications/detail/sp/800-22/rev-1a/final and
 * https://webhome.phy.duke.edu/~rgb/General/dieharder.php.
 */
#define EC_CMD_RAND_NUM 0x001A
#define EC_VER_RAND_NUM 0

struct ec_params_rand_num {
	uint16_t num_rand_bytes; /**< num random bytes to generate */
} __ec_align4;

struct ec_response_rand_num {
	/**
	 * generated random numbers in the range of 1 to EC_MAX_INSIZE. The true
	 * size of rand is determined by ec_params_rand_num's num_rand_bytes.
	 */
	uint8_t rand[FLEXIBLE_ARRAY_MEMBER_SIZE];
} __ec_align1;
BUILD_ASSERT(sizeof(struct ec_response_rand_num) == 0);

/**
 * Get information about the key used to sign the RW firmware.
 * For more details on the fields, see "struct vb21_packed_key".
 */
#define EC_CMD_RWSIG_INFO 0x001B
#define EC_VER_RWSIG_INFO 0

#define VBOOT2_KEY_ID_BYTES 20

#ifdef CHROMIUM_EC
/* Don't force external projects to depend on the vboot headers. */
#include "vb21_struct.h"
BUILD_ASSERT(sizeof(struct vb2_id) == VBOOT2_KEY_ID_BYTES);
#endif

struct ec_response_rwsig_info {
	/**
	 * Signature algorithm used by the key
	 * (enum vb2_signature_algorithm).
	 */
	uint16_t sig_alg;

	/**
	 * Hash digest algorithm used with the key
	 * (enum vb2_hash_algorithm).
	 */
	uint16_t hash_alg;

	/** Key version. */
	uint32_t key_version;

	/** Key ID (struct vb2_id). */
	uint8_t key_id[VBOOT2_KEY_ID_BYTES];

	uint8_t key_is_valid;

	/** Alignment padding. */
	uint8_t reserved[3];
} __ec_align4;

BUILD_ASSERT(sizeof(struct ec_response_rwsig_info) == 32);

/**
 * Get information about the system, such as reset flags, locked state, etc.
 */
#define EC_CMD_SYSINFO 0x001C
#define EC_VER_SYSINFO 0

enum sysinfo_flags {
	SYSTEM_IS_LOCKED = BIT(0),
	SYSTEM_IS_FORCE_LOCKED = BIT(1),
	SYSTEM_JUMP_ENABLED = BIT(2),
	SYSTEM_JUMPED_TO_CURRENT_IMAGE = BIT(3),
	SYSTEM_REBOOT_AT_SHUTDOWN = BIT(4),
	/*
	 * Used internally. It's set when EC_HOST_EVENT_KEYBOARD_RECOVERY is
	 * set and cleared when the system shuts down (not when the host event
	 * flag is cleared).
	 */
	SYSTEM_IN_MANUAL_RECOVERY = BIT(5),
};

struct ec_response_sysinfo {
	uint32_t reset_flags; /**< EC_RESET_FLAG_* flags */
	uint32_t current_image; /**< enum ec_image */
	uint32_t flags; /**< enum sysinfo_flags */
} __ec_align4;

/*****************************************************************************/
/* PWM commands */

/* Get fan target RPM */
#define EC_CMD_PWM_GET_FAN_TARGET_RPM 0x0020

struct ec_response_pwm_get_fan_rpm {
	uint32_t rpm;
} __ec_align4;

/* Set target fan RPM */
#define EC_CMD_PWM_SET_FAN_TARGET_RPM 0x0021

/* Version 0 of input params */
struct ec_params_pwm_set_fan_target_rpm_v0 {
	uint32_t rpm;
} __ec_align4;

/* Version 1 of input params */
struct ec_params_pwm_set_fan_target_rpm_v1 {
	uint32_t rpm;
	uint8_t fan_idx;
} __ec_align_size1;

/* Get keyboard backlight */
/* OBSOLETE - Use EC_CMD_PWM_SET_DUTY */
#define EC_CMD_PWM_GET_KEYBOARD_BACKLIGHT 0x0022

struct ec_response_pwm_get_keyboard_backlight {
	uint8_t percent;
	uint8_t enabled;
} __ec_align1;

/* Set keyboard backlight */
/* OBSOLETE - Use EC_CMD_PWM_SET_DUTY */
#define EC_CMD_PWM_SET_KEYBOARD_BACKLIGHT 0x0023

struct ec_params_pwm_set_keyboard_backlight {
	uint8_t percent;
} __ec_align1;

/* Set target fan PWM duty cycle */
#define EC_CMD_PWM_SET_FAN_DUTY 0x0024

/* Version 0 of input params */
struct ec_params_pwm_set_fan_duty_v0 {
	uint32_t percent;
} __ec_align4;

/* Version 1 of input params */
struct ec_params_pwm_set_fan_duty_v1 {
	uint32_t percent;
	uint8_t fan_idx;
} __ec_align_size1;

#define EC_CMD_PWM_SET_DUTY 0x0025
/* 16 bit duty cycle, 0xffff = 100% */
#define EC_PWM_MAX_DUTY 0xffff

enum ec_pwm_type {
	/* All types, indexed by board-specific enum pwm_channel */
	EC_PWM_TYPE_GENERIC = 0,
	/* Keyboard backlight */
	EC_PWM_TYPE_KB_LIGHT,
	/* Display backlight */
	EC_PWM_TYPE_DISPLAY_LIGHT,
	EC_PWM_TYPE_COUNT,
};

struct ec_params_pwm_set_duty {
	uint16_t duty; /* Duty cycle, EC_PWM_MAX_DUTY = 100% */
	uint8_t pwm_type; /* ec_pwm_type */
	uint8_t index; /* Type-specific index, or 0 if unique */
} __ec_align4;

#define EC_CMD_PWM_GET_DUTY 0x0026

struct ec_params_pwm_get_duty {
	uint8_t pwm_type; /* ec_pwm_type */
	uint8_t index; /* Type-specific index, or 0 if unique */
} __ec_align1;

struct ec_response_pwm_get_duty {
	uint16_t duty; /* Duty cycle, EC_PWM_MAX_DUTY = 100% */
} __ec_align2;

#define EC_CMD_PWM_GET_FAN_DUTY 0x0027

struct ec_params_pwm_get_fan_duty {
	uint8_t fan_idx;
} __ec_align1;

struct ec_response_pwm_get_fan_duty {
	uint32_t percent; /* Percentage of duty cycle, ranging from 0 ~ 100 */
} __ec_align4;

/*****************************************************************************/
/*
 * Lightbar commands. This looks worse than it is. Since we only use one HOST
 * command to say "talk to the lightbar", we put the "and tell it to do X" part
 * into a subcommand. We'll make separate structs for subcommands with
 * different input args, so that we know how much to expect.
 */
#define EC_CMD_LIGHTBAR_CMD 0x0028

struct rgb_s {
	uint8_t r, g, b;
} __ec_todo_unpacked;

#define LB_BATTERY_LEVELS 4

/*
 * List of tweakable parameters. NOTE: It's __packed so it can be sent in a
 * host command, but the alignment is the same regardless. Keep it that way.
 */
struct lightbar_params_v0 {
	/* Timing */
	int32_t google_ramp_up;
	int32_t google_ramp_down;
	int32_t s3s0_ramp_up;
	int32_t s0_tick_delay[2]; /* AC=0/1 */
	int32_t s0a_tick_delay[2]; /* AC=0/1 */
	int32_t s0s3_ramp_down;
	int32_t s3_sleep_for;
	int32_t s3_ramp_up;
	int32_t s3_ramp_down;

	/* Oscillation */
	uint8_t new_s0;
	uint8_t osc_min[2]; /* AC=0/1 */
	uint8_t osc_max[2]; /* AC=0/1 */
	uint8_t w_ofs[2]; /* AC=0/1 */

	/* Brightness limits based on the backlight and AC. */
	uint8_t bright_bl_off_fixed[2]; /* AC=0/1 */
	uint8_t bright_bl_on_min[2]; /* AC=0/1 */
	uint8_t bright_bl_on_max[2]; /* AC=0/1 */

	/* Battery level thresholds */
	uint8_t battery_threshold[LB_BATTERY_LEVELS - 1];

	/* Map [AC][battery_level] to color index */
	uint8_t s0_idx[2][LB_BATTERY_LEVELS]; /* AP is running */
	uint8_t s3_idx[2][LB_BATTERY_LEVELS]; /* AP is sleeping */

	/* Color palette */
	struct rgb_s color[8]; /* 0-3 are Google colors */
} __ec_todo_packed;

struct lightbar_params_v1 {
	/* Timing */
	int32_t google_ramp_up;
	int32_t google_ramp_down;
	int32_t s3s0_ramp_up;
	int32_t s0_tick_delay[2]; /* AC=0/1 */
	int32_t s0a_tick_delay[2]; /* AC=0/1 */
	int32_t s0s3_ramp_down;
	int32_t s3_sleep_for;
	int32_t s3_ramp_up;
	int32_t s3_ramp_down;
	int32_t s5_ramp_up;
	int32_t s5_ramp_down;
	int32_t tap_tick_delay;
	int32_t tap_gate_delay;
	int32_t tap_display_time;

	/* Tap-for-battery params */
	uint8_t tap_pct_red;
	uint8_t tap_pct_green;
	uint8_t tap_seg_min_on;
	uint8_t tap_seg_max_on;
	uint8_t tap_seg_osc;
	uint8_t tap_idx[3];

	/* Oscillation */
	uint8_t osc_min[2]; /* AC=0/1 */
	uint8_t osc_max[2]; /* AC=0/1 */
	uint8_t w_ofs[2]; /* AC=0/1 */

	/* Brightness limits based on the backlight and AC. */
	uint8_t bright_bl_off_fixed[2]; /* AC=0/1 */
	uint8_t bright_bl_on_min[2]; /* AC=0/1 */
	uint8_t bright_bl_on_max[2]; /* AC=0/1 */

	/* Battery level thresholds */
	uint8_t battery_threshold[LB_BATTERY_LEVELS - 1];

	/* Map [AC][battery_level] to color index */
	uint8_t s0_idx[2][LB_BATTERY_LEVELS]; /* AP is running */
	uint8_t s3_idx[2][LB_BATTERY_LEVELS]; /* AP is sleeping */

	/* s5: single color pulse on inhibited power-up */
	uint8_t s5_idx;

	/* Color palette */
	struct rgb_s color[8]; /* 0-3 are Google colors */
} __ec_todo_packed;

/* Lightbar command params v2
 * crbug.com/467716
 *
 * lightbar_parms_v1 was too big for i2c, therefore in v2, we split them up by
 * logical groups to make it more manageable ( < 120 bytes).
 *
 * NOTE: Each of these groups must be less than 120 bytes.
 */

struct lightbar_params_v2_timing {
	/* Timing */
	int32_t google_ramp_up;
	int32_t google_ramp_down;
	int32_t s3s0_ramp_up;
	int32_t s0_tick_delay[2]; /* AC=0/1 */
	int32_t s0a_tick_delay[2]; /* AC=0/1 */
	int32_t s0s3_ramp_down;
	int32_t s3_sleep_for;
	int32_t s3_ramp_up;
	int32_t s3_ramp_down;
	int32_t s5_ramp_up;
	int32_t s5_ramp_down;
	int32_t tap_tick_delay;
	int32_t tap_gate_delay;
	int32_t tap_display_time;
} __ec_todo_packed;

struct lightbar_params_v2_tap {
	/* Tap-for-battery params */
	uint8_t tap_pct_red;
	uint8_t tap_pct_green;
	uint8_t tap_seg_min_on;
	uint8_t tap_seg_max_on;
	uint8_t tap_seg_osc;
	uint8_t tap_idx[3];
} __ec_todo_packed;

struct lightbar_params_v2_oscillation {
	/* Oscillation */
	uint8_t osc_min[2]; /* AC=0/1 */
	uint8_t osc_max[2]; /* AC=0/1 */
	uint8_t w_ofs[2]; /* AC=0/1 */
} __ec_todo_packed;

struct lightbar_params_v2_brightness {
	/* Brightness limits based on the backlight and AC. */
	uint8_t bright_bl_off_fixed[2]; /* AC=0/1 */
	uint8_t bright_bl_on_min[2]; /* AC=0/1 */
	uint8_t bright_bl_on_max[2]; /* AC=0/1 */
} __ec_todo_packed;

struct lightbar_params_v2_thresholds {
	/* Battery level thresholds */
	uint8_t battery_threshold[LB_BATTERY_LEVELS - 1];
} __ec_todo_packed;

struct lightbar_params_v2_colors {
	/* Map [AC][battery_level] to color index */
	uint8_t s0_idx[2][LB_BATTERY_LEVELS]; /* AP is running */
	uint8_t s3_idx[2][LB_BATTERY_LEVELS]; /* AP is sleeping */

	/* s5: single color pulse on inhibited power-up */
	uint8_t s5_idx;

	/* Color palette */
	struct rgb_s color[8]; /* 0-3 are Google colors */
} __ec_todo_packed;

struct lightbar_params_v3 {
	/*
	 *  Number of LEDs reported by the EC.
	 *  May be less than the actual number of LEDs in the lightbar.
	 */
	uint8_t reported_led_num;
} __ec_todo_packed;

/* Lightbar program. */
#define EC_LB_PROG_LEN 192
struct lightbar_program {
	uint8_t size;
	uint8_t data[EC_LB_PROG_LEN];
} __ec_todo_unpacked;

/*
 * Lightbar program for large sequences. Sequences are sent in pieces, with
 * increasing offset. The sequences are still limited by the amount reserved in
 * EC RAM.
 */
struct lightbar_program_ex {
	uint8_t size;
	uint16_t offset;
	uint8_t data[0];
} __ec_todo_packed;

struct ec_params_lightbar {
	uint8_t cmd; /* Command (see enum lightbar_command) */
	union {
		/*
		 * The following commands have no args:
		 *
		 * dump, off, on, init, get_seq, get_params_v0, get_params_v1,
		 * version, get_brightness, get_demo, suspend, resume,
		 * get_params_v2_timing, get_params_v2_tap, get_params_v2_osc,
		 * get_params_v2_bright, get_params_v2_thlds,
		 * get_params_v2_colors
		 *
		 * Don't use an empty struct, because C++ hates that.
		 */

		struct __ec_todo_unpacked {
			uint8_t num;
		} set_brightness, seq, demo;

		struct __ec_todo_unpacked {
			uint8_t ctrl, reg, value;
		} reg;

		struct __ec_todo_unpacked {
			uint8_t led, red, green, blue;
		} set_rgb;

		struct __ec_todo_unpacked {
			uint8_t led;
		} get_rgb;

		struct __ec_todo_unpacked {
			uint8_t enable;
		} manual_suspend_ctrl;

		struct lightbar_params_v0 set_params_v0;
		struct lightbar_params_v1 set_params_v1;

		struct lightbar_params_v2_timing set_v2par_timing;
		struct lightbar_params_v2_tap set_v2par_tap;
		struct lightbar_params_v2_oscillation set_v2par_osc;
		struct lightbar_params_v2_brightness set_v2par_bright;
		struct lightbar_params_v2_thresholds set_v2par_thlds;
		struct lightbar_params_v2_colors set_v2par_colors;

		struct lightbar_program set_program;
		struct lightbar_program_ex set_program_ex;
	};
} __ec_todo_packed;

struct ec_response_lightbar {
	union {
		struct __ec_todo_unpacked {
			struct __ec_todo_unpacked {
				uint8_t reg;
				uint8_t ic0;
				uint8_t ic1;
			} vals[23];
		} dump;

		struct __ec_todo_unpacked {
			uint8_t num;
		} get_seq, get_brightness, get_demo;

		struct lightbar_params_v0 get_params_v0;
		struct lightbar_params_v1 get_params_v1;

		struct lightbar_params_v2_timing get_params_v2_timing;
		struct lightbar_params_v2_tap get_params_v2_tap;
		struct lightbar_params_v2_oscillation get_params_v2_osc;
		struct lightbar_params_v2_brightness get_params_v2_bright;
		struct lightbar_params_v2_thresholds get_params_v2_thlds;
		struct lightbar_params_v2_colors get_params_v2_colors;

		struct lightbar_params_v3 get_params_v3;

		struct __ec_todo_unpacked {
			uint32_t num;
			uint32_t flags;
		} version;

		struct __ec_todo_unpacked {
			uint8_t red, green, blue;
		} get_rgb;

		/*
		 * The following commands have no response:
		 *
		 * off, on, init, set_brightness, seq, reg, set_rgb, demo,
		 * set_params_v0, set_params_v1, set_program,
		 * manual_suspend_ctrl, suspend, resume, set_v2par_timing,
		 * set_v2par_tap, set_v2par_osc, set_v2par_bright,
		 * set_v2par_thlds, set_v2par_colors
		 */
	};
} __ec_todo_packed;

/* Lightbar commands */
enum lightbar_command {
	LIGHTBAR_CMD_DUMP = 0,
	LIGHTBAR_CMD_OFF = 1,
	LIGHTBAR_CMD_ON = 2,
	LIGHTBAR_CMD_INIT = 3,
	LIGHTBAR_CMD_SET_BRIGHTNESS = 4,
	LIGHTBAR_CMD_SEQ = 5,
	LIGHTBAR_CMD_REG = 6,
	LIGHTBAR_CMD_SET_RGB = 7,
	LIGHTBAR_CMD_GET_SEQ = 8,
	LIGHTBAR_CMD_DEMO = 9,
	LIGHTBAR_CMD_GET_PARAMS_V0 = 10,
	LIGHTBAR_CMD_SET_PARAMS_V0 = 11,
	LIGHTBAR_CMD_VERSION = 12,
	LIGHTBAR_CMD_GET_BRIGHTNESS = 13,
	LIGHTBAR_CMD_GET_RGB = 14,
	LIGHTBAR_CMD_GET_DEMO = 15,
	LIGHTBAR_CMD_GET_PARAMS_V1 = 16,
	LIGHTBAR_CMD_SET_PARAMS_V1 = 17,
	LIGHTBAR_CMD_SET_PROGRAM = 18,
	LIGHTBAR_CMD_MANUAL_SUSPEND_CTRL = 19,
	LIGHTBAR_CMD_SUSPEND = 20,
	LIGHTBAR_CMD_RESUME = 21,
	LIGHTBAR_CMD_GET_PARAMS_V2_TIMING = 22,
	LIGHTBAR_CMD_SET_PARAMS_V2_TIMING = 23,
	LIGHTBAR_CMD_GET_PARAMS_V2_TAP = 24,
	LIGHTBAR_CMD_SET_PARAMS_V2_TAP = 25,
	LIGHTBAR_CMD_GET_PARAMS_V2_OSCILLATION = 26,
	LIGHTBAR_CMD_SET_PARAMS_V2_OSCILLATION = 27,
	LIGHTBAR_CMD_GET_PARAMS_V2_BRIGHTNESS = 28,
	LIGHTBAR_CMD_SET_PARAMS_V2_BRIGHTNESS = 29,
	LIGHTBAR_CMD_GET_PARAMS_V2_THRESHOLDS = 30,
	LIGHTBAR_CMD_SET_PARAMS_V2_THRESHOLDS = 31,
	LIGHTBAR_CMD_GET_PARAMS_V2_COLORS = 32,
	LIGHTBAR_CMD_SET_PARAMS_V2_COLORS = 33,
	LIGHTBAR_CMD_GET_PARAMS_V3 = 34,
	LIGHTBAR_CMD_SET_PROGRAM_EX = 35,
	LIGHTBAR_NUM_CMDS,
};

/*****************************************************************************/
/* LED control commands */

#define EC_CMD_LED_CONTROL 0x0029

enum ec_led_id {
	/* LED to indicate battery state of charge */
	EC_LED_ID_BATTERY_LED = 0,
	/*
	 * LED to indicate system power state (on or in suspend).
	 * May be on power button or on C-panel.
	 */
	EC_LED_ID_POWER_LED,
	/* LED on power adapter or its plug */
	EC_LED_ID_ADAPTER_LED,
	/* LED to indicate left side */
	EC_LED_ID_LEFT_LED,
	/* LED to indicate right side */
	EC_LED_ID_RIGHT_LED,
	/* LED to indicate recovery mode with HW_REINIT */
	EC_LED_ID_RECOVERY_HW_REINIT_LED,
	/* LED to indicate sysrq debug mode. */
	EC_LED_ID_SYSRQ_DEBUG_LED,
	/* LED strip for advanced patterns. */
	EC_LED_ID_LIGHTBAR_LED,

	EC_LED_ID_COUNT,
};

/* LED control flags */
#define EC_LED_FLAGS_QUERY BIT(0) /* Query LED capability only */
#define EC_LED_FLAGS_AUTO BIT(1) /* Switch LED back to automatic control */

enum ec_led_colors {
	EC_LED_COLOR_INVALID = -1,
	EC_LED_COLOR_RED = 0,
	EC_LED_COLOR_GREEN,
	EC_LED_COLOR_BLUE,
	EC_LED_COLOR_YELLOW,
	EC_LED_COLOR_WHITE,
	EC_LED_COLOR_AMBER,
	EC_LED_COLOR_MAGENTA,

	EC_LED_COLOR_COUNT,
};

struct ec_params_led_control {
	uint8_t led_id; /* Which LED to control */
	uint8_t flags; /* Control flags */

	uint8_t brightness[EC_LED_COLOR_COUNT];
} __ec_align1;

struct ec_response_led_control {
	/*
	 * Available brightness value range.
	 *
	 * Range 0 means color channel not present.
	 * Range 1 means on/off control.
	 * Other values means the LED is control by PWM.
	 */
	uint8_t brightness_range[EC_LED_COLOR_COUNT];
} __ec_align1;

/*****************************************************************************/
/* Verified boot commands */

/*
 * Note: command code 0x29 version 0 was VBOOT_CMD in Link EVT; it may be
 * reused for other purposes with version > 0.
 */

/* Verified boot hash command */
#define EC_CMD_VBOOT_HASH 0x002A

struct ec_params_vboot_hash {
	uint8_t cmd; /* enum ec_vboot_hash_cmd */
	uint8_t hash_type; /* enum ec_vboot_hash_type */
	uint8_t nonce_size; /* Nonce size; may be 0 */
	uint8_t reserved0; /* Reserved; set 0 */
	uint32_t offset; /* Offset in flash to hash */
	uint32_t size; /* Number of bytes to hash */
	uint8_t nonce_data[64]; /* Nonce data; ignored if nonce_size=0 */
} __ec_align4;

struct ec_response_vboot_hash {
	uint8_t status; /* enum ec_vboot_hash_status */
	uint8_t hash_type; /* enum ec_vboot_hash_type */
	uint8_t digest_size; /* Size of hash digest in bytes */
	uint8_t reserved0; /* Ignore; will be 0 */
	uint32_t offset; /* Offset in flash which was hashed */
	uint32_t size; /* Number of bytes hashed */
	uint8_t hash_digest[64]; /* Hash digest data */
} __ec_align4;

enum ec_vboot_hash_cmd {
	EC_VBOOT_HASH_GET = 0, /* Get current hash status */
	EC_VBOOT_HASH_ABORT = 1, /* Abort calculating current hash */
	EC_VBOOT_HASH_START = 2, /* Start computing a new hash */
	EC_VBOOT_HASH_RECALC = 3, /* Synchronously compute a new hash */
};

enum ec_vboot_hash_type {
	EC_VBOOT_HASH_TYPE_SHA256 = 0, /* SHA-256 */
};

enum ec_vboot_hash_status {
	EC_VBOOT_HASH_STATUS_NONE = 0, /* No hash (not started, or aborted) */
	EC_VBOOT_HASH_STATUS_DONE = 1, /* Finished computing a hash */
	EC_VBOOT_HASH_STATUS_BUSY = 2, /* Busy computing a hash */
};

/*
 * Special values for offset for EC_VBOOT_HASH_START and EC_VBOOT_HASH_RECALC.
 * If one of these is specified, the EC will automatically update offset and
 * size to the correct values for the specified image (RO or RW).
 */
#define EC_VBOOT_HASH_OFFSET_RO 0xfffffffe
#define EC_VBOOT_HASH_OFFSET_ACTIVE 0xfffffffd
#define EC_VBOOT_HASH_OFFSET_UPDATE 0xfffffffc

/*
 * 'RW' is vague if there are multiple RW images; we mean the active one,
 * so the old constant is deprecated.
 */
#define EC_VBOOT_HASH_OFFSET_RW EC_VBOOT_HASH_OFFSET_ACTIVE

/*****************************************************************************/
/*
 * Motion sense commands. We'll make separate structs for sub-commands with
 * different input args, so that we know how much to expect.
 */
#define EC_CMD_MOTION_SENSE_CMD 0x002B

/* Motion sense commands */
enum motionsense_command {
	/*
	 * Dump command returns all motion sensor data including motion sense
	 * module flags and individual sensor flags.
	 */
	MOTIONSENSE_CMD_DUMP = 0,

	/*
	 * Info command returns data describing the details of a given sensor,
	 * including enum motionsensor_type, enum motionsensor_location, and
	 * enum motionsensor_chip.
	 */
	MOTIONSENSE_CMD_INFO = 1,

	/*
	 * EC Rate command is a setter/getter command for the EC sampling rate
	 * in milliseconds.
	 * It is per sensor, the EC run sample task  at the minimum of all
	 * sensors EC_RATE.
	 * For sensors without hardware FIFO, EC_RATE should be equals to 1/ODR
	 * to collect all the sensor samples.
	 * For sensor with hardware FIFO, EC_RATE is used as the maximal delay
	 * to process of all motion sensors in milliseconds.
	 */
	MOTIONSENSE_CMD_EC_RATE = 2,

	/*
	 * Sensor ODR command is a setter/getter command for the output data
	 * rate of a specific motion sensor in millihertz.
	 */
	MOTIONSENSE_CMD_SENSOR_ODR = 3,

	/*
	 * Sensor range command is a setter/getter command for the range of
	 * a specified motion sensor in +/-G's or +/- deg/s.
	 */
	MOTIONSENSE_CMD_SENSOR_RANGE = 4,

	/*
	 * Setter/getter command for the keyboard wake angle. When the lid
	 * angle is greater than this value, keyboard wake is disabled in S3,
	 * and when the lid angle goes less than this value, keyboard wake is
	 * enabled. Note, the lid angle measurement is an approximate,
	 * un-calibrated value, hence the wake angle isn't exact.
	 */
	MOTIONSENSE_CMD_KB_WAKE_ANGLE = 5,

	/*
	 * Returns a single sensor data.
	 */
	MOTIONSENSE_CMD_DATA = 6,

	/*
	 * Return sensor fifo info.
	 */
	MOTIONSENSE_CMD_FIFO_INFO = 7,

	/*
	 * Insert a flush element in the fifo and return sensor fifo info.
	 * The host can use that element to synchronize its operation.
	 */
	MOTIONSENSE_CMD_FIFO_FLUSH = 8,

	/*
	 * Return a portion of the fifo.
	 */
	MOTIONSENSE_CMD_FIFO_READ = 9,

	/*
	 * Perform low level calibration.
	 * On sensors that support it, ask to do offset calibration.
	 */
	MOTIONSENSE_CMD_PERFORM_CALIB = 10,

	/*
	 * Sensor Offset command is a setter/getter command for the offset
	 * used for factory calibration.
	 * The offsets can be calculated by the host, or via
	 * PERFORM_CALIB command.
	 */
	MOTIONSENSE_CMD_SENSOR_OFFSET = 11,

	/*
	 * List available activities for a MOTION sensor.
	 * Indicates if they are enabled or disabled.
	 */
	MOTIONSENSE_CMD_LIST_ACTIVITIES = 12,

	/*
	 * Activity management
	 * Enable/Disable activity recognition.
	 */
	MOTIONSENSE_CMD_SET_ACTIVITY = 13,

	/*
	 * Lid Angle
	 */
	MOTIONSENSE_CMD_LID_ANGLE = 14,

	/*
	 * Allow the FIFO to trigger interrupt via MKBP events.
	 * By default the FIFO does not send interrupt to process the FIFO
	 * until the AP is ready or it is coming from a wakeup sensor.
	 */
	MOTIONSENSE_CMD_FIFO_INT_ENABLE = 15,

	/*
	 * Spoof the readings of the sensors.  The spoofed readings can be set
	 * to arbitrary values, or will lock to the last read actual values.
	 */
	MOTIONSENSE_CMD_SPOOF = 16,

	/* Set lid angle for tablet mode detection. */
	MOTIONSENSE_CMD_TABLET_MODE_LID_ANGLE = 17,

	/*
	 * Sensor Scale command is a setter/getter command for the calibration
	 * scale.
	 */
	MOTIONSENSE_CMD_SENSOR_SCALE = 18,

	/*
	 * Read the current online calibration values (if available).
	 */
	MOTIONSENSE_CMD_ONLINE_CALIB_READ = 19,

	/*
	 * Activity management
	 * Retrieve current status of given activity.
	 */
	MOTIONSENSE_CMD_GET_ACTIVITY = 20,

	/* Number of motionsense sub-commands. */
	MOTIONSENSE_NUM_CMDS,
};

/* List of motion sensor types. */
enum motionsensor_type {
	MOTIONSENSE_TYPE_ACCEL = 0,
	MOTIONSENSE_TYPE_GYRO = 1,
	MOTIONSENSE_TYPE_MAG = 2,
	MOTIONSENSE_TYPE_PROX = 3,
	MOTIONSENSE_TYPE_LIGHT = 4,
	MOTIONSENSE_TYPE_ACTIVITY = 5,
	MOTIONSENSE_TYPE_BARO = 6,
	MOTIONSENSE_TYPE_SYNC = 7,
	MOTIONSENSE_TYPE_LIGHT_RGB = 8,
	MOTIONSENSE_TYPE_MAX,
};

/* List of motion sensor locations. */
enum motionsensor_location {
	MOTIONSENSE_LOC_BASE = 0,
	MOTIONSENSE_LOC_LID = 1,
	MOTIONSENSE_LOC_CAMERA = 2,
	MOTIONSENSE_LOC_MAX,
};

/* List of motion sensor chips. */
enum motionsensor_chip {
	MOTIONSENSE_CHIP_KXCJ9 = 0,
	MOTIONSENSE_CHIP_LSM6DS0 = 1,
	MOTIONSENSE_CHIP_BMI160 = 2,
	MOTIONSENSE_CHIP_SI1141 = 3,
	MOTIONSENSE_CHIP_SI1142 = 4,
	MOTIONSENSE_CHIP_SI1143 = 5,
	MOTIONSENSE_CHIP_KX022 = 6,
	MOTIONSENSE_CHIP_L3GD20H = 7,
	MOTIONSENSE_CHIP_BMA255 = 8,
	MOTIONSENSE_CHIP_BMP280 = 9,
	MOTIONSENSE_CHIP_OPT3001 = 10,
	MOTIONSENSE_CHIP_BH1730 = 11,
	MOTIONSENSE_CHIP_GPIO = 12,
	MOTIONSENSE_CHIP_LIS2DH = 13,
	MOTIONSENSE_CHIP_LSM6DSM = 14,
	MOTIONSENSE_CHIP_LIS2DE = 15,
	MOTIONSENSE_CHIP_LIS2MDL = 16,
	MOTIONSENSE_CHIP_LSM6DS3 = 17,
	MOTIONSENSE_CHIP_LSM6DSO = 18,
	MOTIONSENSE_CHIP_LNG2DM = 19,
	MOTIONSENSE_CHIP_TCS3400 = 20,
	MOTIONSENSE_CHIP_LIS2DW12 = 21,
	MOTIONSENSE_CHIP_LIS2DWL = 22,
	MOTIONSENSE_CHIP_LIS2DS = 23,
	MOTIONSENSE_CHIP_BMI260 = 24,
	MOTIONSENSE_CHIP_ICM426XX = 25,
	MOTIONSENSE_CHIP_ICM42607 = 26,
	MOTIONSENSE_CHIP_BMA422 = 27,
	MOTIONSENSE_CHIP_BMI323 = 28,
	MOTIONSENSE_CHIP_BMI220 = 29,
	MOTIONSENSE_CHIP_CM32183 = 30,
	MOTIONSENSE_CHIP_VEML3328 = 31,
	MOTIONSENSE_CHIP_CM36781 = 32,
	MOTIONSENSE_CHIP_MAX,
};

/* List of orientation positions */
enum motionsensor_orientation {
	MOTIONSENSE_ORIENTATION_LANDSCAPE = 0,
	MOTIONSENSE_ORIENTATION_PORTRAIT = 1,
	MOTIONSENSE_ORIENTATION_UPSIDE_DOWN_PORTRAIT = 2,
	MOTIONSENSE_ORIENTATION_UPSIDE_DOWN_LANDSCAPE = 3,
	MOTIONSENSE_ORIENTATION_UNKNOWN = 4,
};

struct ec_response_activity_data {
	uint8_t activity; /* motionsensor_activity */
	uint8_t state;
} __ec_todo_packed;

struct ec_response_motion_sensor_data {
	/* Flags for each sensor. */
	uint8_t flags;
	/* Sensor number the data comes from. */
	uint8_t sensor_num;
	/* Each sensor is up to 3-axis. */
	union {
		int16_t data[3];
		/* for sensors using unsigned data */
		uint16_t udata[3];
		struct __ec_todo_packed {
			uint16_t reserved;
			uint32_t timestamp;
		};
		struct __ec_todo_unpacked {
			struct ec_response_activity_data activity_data;
			int16_t add_info[2];
		};
	};
} __ec_todo_packed;

/* Response to AP reporting calibration data for a given sensor. */
struct ec_response_online_calibration_data {
	/** The calibration values. */
	int16_t data[3];
};

/* Note: used in ec_response_get_next_data */
struct ec_response_motion_sense_fifo_info {
	/* Size of the fifo */
	uint16_t size;
	/* Amount of space used in the fifo */
	uint16_t count;
	/* Timestamp recorded in us.
	 * aka accurate timestamp when host event was triggered.
	 */
	uint32_t timestamp;
	/* Total amount of vector lost */
	uint16_t total_lost;
	/* Lost events since the last fifo_info, per sensors */
	uint16_t lost[0];
} __ec_todo_packed;

struct ec_response_motion_sense_fifo_data {
	uint32_t number_data;
	struct ec_response_motion_sensor_data data[0];
} __ec_todo_packed;

/* List supported activity recognition */
enum motionsensor_activity {
	MOTIONSENSE_ACTIVITY_RESERVED = 0,
	MOTIONSENSE_ACTIVITY_SIG_MOTION = 1,
	MOTIONSENSE_ACTIVITY_DOUBLE_TAP = 2,
	MOTIONSENSE_ACTIVITY_ORIENTATION = 3,
	MOTIONSENSE_ACTIVITY_BODY_DETECTION = 4,
};

struct ec_motion_sense_activity {
	uint8_t sensor_num;
	uint8_t activity; /* one of enum motionsensor_activity */
	uint8_t enable; /* 1: enable, 0: disable */
	uint8_t reserved;
	uint16_t parameters[4]; /* activity dependent parameters */
} __ec_todo_packed;

/* Module flag masks used for the dump sub-command. */
#define MOTIONSENSE_MODULE_FLAG_ACTIVE BIT(0)

/* Sensor flag masks used for the dump sub-command. */
#define MOTIONSENSE_SENSOR_FLAG_PRESENT BIT(0)

/*
 * Flush entry for synchronization.
 * data contains time stamp
 */
#define MOTIONSENSE_SENSOR_FLAG_FLUSH BIT(0)
#define MOTIONSENSE_SENSOR_FLAG_TIMESTAMP BIT(1)
#define MOTIONSENSE_SENSOR_FLAG_WAKEUP BIT(2)
#define MOTIONSENSE_SENSOR_FLAG_TABLET_MODE BIT(3)
#define MOTIONSENSE_SENSOR_FLAG_ODR BIT(4)

#define MOTIONSENSE_SENSOR_FLAG_BYPASS_FIFO BIT(7)

/*
 * Send this value for the data element to only perform a read. If you
 * send any other value, the EC will interpret it as data to set and will
 * return the actual value set.
 */
#define EC_MOTION_SENSE_NO_VALUE -1

#define EC_MOTION_SENSE_INVALID_CALIB_TEMP INT16_MIN

/* MOTIONSENSE_CMD_SENSOR_OFFSET subcommand flag */
/* Set Calibration information */
#define MOTION_SENSE_SET_OFFSET BIT(0)

/* Default Scale value, factor 1. */
#define MOTION_SENSE_DEFAULT_SCALE BIT(15)

#define LID_ANGLE_UNRELIABLE 500

enum motionsense_spoof_mode {
	/* Disable spoof mode. */
	MOTIONSENSE_SPOOF_MODE_DISABLE = 0,

	/* Enable spoof mode, but use provided component values. */
	MOTIONSENSE_SPOOF_MODE_CUSTOM,

	/* Enable spoof mode, but use the current sensor values. */
	MOTIONSENSE_SPOOF_MODE_LOCK_CURRENT,

	/* Query the current spoof mode status for the sensor. */
	MOTIONSENSE_SPOOF_MODE_QUERY,
};

struct ec_params_motion_sense {
	uint8_t cmd;
	union {
		/* Used for MOTIONSENSE_CMD_DUMP. */
		struct __ec_todo_unpacked {
			/*
			 * Maximal number of sensor the host is expecting.
			 * 0 means the host is only interested in the number
			 * of sensors controlled by the EC.
			 */
			uint8_t max_sensor_count;
		} dump;

		/*
		 * Used for MOTIONSENSE_CMD_KB_WAKE_ANGLE.
		 */
		struct __ec_todo_unpacked {
			/* Data to set or EC_MOTION_SENSE_NO_VALUE to read.
			 * kb_wake_angle: angle to wakeup AP.
			 */
			int16_t data;
		} kb_wake_angle;

		/*
		 * Used for MOTIONSENSE_CMD_INFO, MOTIONSENSE_CMD_DATA
		 */
		struct __ec_todo_unpacked {
			uint8_t sensor_num;
		} info, info_3, info_4, data, fifo_flush, list_activities;

		/*
		 * Used for MOTIONSENSE_CMD_PERFORM_CALIB:
		 * Allow entering/exiting the calibration mode.
		 */
		struct __ec_todo_unpacked {
			uint8_t sensor_num;
			uint8_t enable;
		} perform_calib;

		/*
		 * Used for MOTIONSENSE_CMD_EC_RATE, MOTIONSENSE_CMD_SENSOR_ODR
		 * and MOTIONSENSE_CMD_SENSOR_RANGE.
		 */
		struct __ec_todo_unpacked {
			uint8_t sensor_num;

			/* Rounding flag, true for round-up, false for down. */
			uint8_t roundup;

			uint16_t reserved;

			/* Data to set or EC_MOTION_SENSE_NO_VALUE to read. */
			int32_t data;
		} ec_rate, sensor_odr, sensor_range;

		/* Used for MOTIONSENSE_CMD_SENSOR_OFFSET */
		struct __ec_todo_packed {
			uint8_t sensor_num;

			/*
			 * bit 0: If set (MOTION_SENSE_SET_OFFSET), set
			 * the calibration information in the EC.
			 * If unset, just retrieve calibration information.
			 */
			uint16_t flags;

			/*
			 * Temperature at calibration, in units of 0.01 C
			 * 0x8000: invalid / unknown.
			 * 0x0: 0C
			 * 0x7fff: +327.67C
			 */
			int16_t temp;

			/*
			 * Offset for calibration.
			 * Unit:
			 * Accelerometer: 1/1024 g
			 * Gyro:          1/1024 deg/s
			 * Compass:       1/16 uT
			 */
			int16_t offset[3];
		} sensor_offset;

		/* Used for MOTIONSENSE_CMD_SENSOR_SCALE */
		struct __ec_todo_packed {
			uint8_t sensor_num;

			/*
			 * bit 0: If set (MOTION_SENSE_SET_OFFSET), set
			 * the calibration information in the EC.
			 * If unset, just retrieve calibration information.
			 */
			uint16_t flags;

			/*
			 * Temperature at calibration, in units of 0.01 C
			 * 0x8000: invalid / unknown.
			 * 0x0: 0C
			 * 0x7fff: +327.67C
			 */
			int16_t temp;

			/*
			 * Scale for calibration:
			 * By default scale is 1, it is encoded on 16bits:
			 * 1 = BIT(15)
			 * ~2 = 0xFFFF
			 * ~0 = 0.
			 */
			uint16_t scale[3];
		} sensor_scale;

		/* Used for MOTIONSENSE_CMD_FIFO_INFO */
		/* (no params) */

		/* Used for MOTIONSENSE_CMD_FIFO_READ */
		struct __ec_todo_unpacked {
			/*
			 * Number of expected vector to return.
			 * EC may return less or 0 if none available.
			 */
			uint32_t max_data_vector;
		} fifo_read;

		/* Used for MOTIONSENSE_CMD_SET_ACTIVITY */
		struct ec_motion_sense_activity set_activity;

		/* Used for MOTIONSENSE_CMD_LID_ANGLE */
		/* (no params) */

		/* Used for MOTIONSENSE_CMD_FIFO_INT_ENABLE */
		struct __ec_todo_unpacked {
			/*
			 * 1: enable, 0 disable fifo,
			 * EC_MOTION_SENSE_NO_VALUE return value.
			 */
			int8_t enable;
		} fifo_int_enable;

		/* Used for MOTIONSENSE_CMD_SPOOF */
		struct __ec_todo_packed {
			uint8_t sensor_id;

			/* See enum motionsense_spoof_mode. */
			uint8_t spoof_enable;

			/* Ignored, used for alignment. */
			uint8_t reserved;

			union {
				/* Individual component values to spoof. */
				int16_t components[3];

				/* Used when spoofing an activity */
				struct {
					/* enum motionsensor_activity */
					uint8_t activity_num;

					/* spoof activity state */
					uint8_t activity_state;
				};
			} __ec_todo_packed;
		} spoof;

		/* Used for MOTIONSENSE_CMD_TABLET_MODE_LID_ANGLE. */
		struct __ec_todo_unpacked {
			/*
			 * Lid angle threshold for switching between tablet and
			 * clamshell mode.
			 */
			int16_t lid_angle;

			/*
			 * Hysteresis degree to prevent fluctuations between
			 * clamshell and tablet mode if lid angle keeps
			 * changing around the threshold. Lid motion driver will
			 * use lid_angle + hys_degree to trigger tablet mode and
			 * lid_angle - hys_degree to trigger clamshell mode.
			 */
			int16_t hys_degree;
		} tablet_mode_threshold;

		/*
		 * Used for MOTIONSENSE_CMD_ONLINE_CALIB_READ:
		 * Allow reading a single sensor's online calibration value.
		 */
		struct __ec_todo_unpacked {
			uint8_t sensor_num;
		} online_calib_read;

		/*
		 * Used for MOTIONSENSE_CMD_GET_ACTIVITY.
		 */
		struct __ec_todo_unpacked {
			uint8_t sensor_num;
			uint8_t activity; /* enum motionsensor_activity */
		} get_activity;
	} __ec_todo_packed;
} __ec_todo_packed;

enum motion_sense_cmd_info_flags {
	/* The sensor supports online calibration */
	MOTION_SENSE_CMD_INFO_FLAG_ONLINE_CALIB = BIT(0),
};

struct ec_response_motion_sense {
	union {
		/* Used for MOTIONSENSE_CMD_DUMP */
		struct __ec_todo_unpacked {
			/* Flags representing the motion sensor module. */
			uint8_t module_flags;

			/* Number of sensors managed directly by the EC. */
			uint8_t sensor_count;

			/*
			 * Sensor data is truncated if response_max is too small
			 * for holding all the data.
			 */
			struct ec_response_motion_sensor_data sensor[0];
		} dump;

		/* Used for MOTIONSENSE_CMD_INFO. */
		struct __ec_todo_unpacked {
			/* Should be element of enum motionsensor_type. */
			uint8_t type;

			/* Should be element of enum motionsensor_location. */
			uint8_t location;

			/* Should be element of enum motionsensor_chip. */
			uint8_t chip;
		} info;

		/* Used for MOTIONSENSE_CMD_INFO version 3 */
		struct __ec_todo_unpacked {
			/* Should be element of enum motionsensor_type. */
			uint8_t type;

			/* Should be element of enum motionsensor_location. */
			uint8_t location;

			/* Should be element of enum motionsensor_chip. */
			uint8_t chip;

			/* Minimum sensor sampling frequency */
			uint32_t min_frequency;

			/* Maximum sensor sampling frequency */
			uint32_t max_frequency;

			/* Max number of sensor events that could be in fifo */
			uint32_t fifo_max_event_count;
		} info_3;

		/* Used for MOTIONSENSE_CMD_INFO version 4 */
		struct __ec_align4 {
			/* Should be element of enum motionsensor_type. */
			uint8_t type;

			/* Should be element of enum motionsensor_location. */
			uint8_t location;

			/* Should be element of enum motionsensor_chip. */
			uint8_t chip;

			/* Minimum sensor sampling frequency */
			uint32_t min_frequency;

			/* Maximum sensor sampling frequency */
			uint32_t max_frequency;

			/* Max number of sensor events that could be in fifo */
			uint32_t fifo_max_event_count;

			/*
			 * Should be elements of
			 * enum motion_sense_cmd_info_flags
			 */
			uint32_t flags;
		} info_4;

		/* Used for MOTIONSENSE_CMD_DATA */
		struct ec_response_motion_sensor_data data;

		/*
		 * Used for MOTIONSENSE_CMD_EC_RATE, MOTIONSENSE_CMD_SENSOR_ODR,
		 * MOTIONSENSE_CMD_SENSOR_RANGE,
		 * MOTIONSENSE_CMD_KB_WAKE_ANGLE,
		 * MOTIONSENSE_CMD_FIFO_INT_ENABLE and
		 * MOTIONSENSE_CMD_SPOOF.
		 */
		struct __ec_todo_unpacked {
			/* Current value of the parameter queried. */
			int32_t ret;
		} ec_rate, sensor_odr, sensor_range, kb_wake_angle,
			fifo_int_enable, spoof;

		/*
		 * Used for MOTIONSENSE_CMD_SENSOR_OFFSET,
		 * PERFORM_CALIB.
		 */
		struct __ec_todo_unpacked {
			int16_t temp;
			int16_t offset[3];
		} sensor_offset, perform_calib;

		/* Used for MOTIONSENSE_CMD_SENSOR_SCALE */
		struct __ec_todo_unpacked {
			int16_t temp;
			uint16_t scale[3];
		} sensor_scale;

		struct ec_response_motion_sense_fifo_info fifo_info, fifo_flush;

		struct ec_response_motion_sense_fifo_data fifo_read;

		struct ec_response_online_calibration_data online_calib_read;

		struct __ec_todo_packed {
			uint16_t reserved;
			uint32_t enabled;
			uint32_t disabled;
		} list_activities;

		/* No params for set activity */

		/* Used for MOTIONSENSE_CMD_LID_ANGLE */
		struct __ec_todo_unpacked {
			/*
			 * Angle between 0 and 360 degree if available,
			 * LID_ANGLE_UNRELIABLE otherwise.
			 */
			uint16_t value;
		} lid_angle;

		/* Used for MOTIONSENSE_CMD_TABLET_MODE_LID_ANGLE. */
		struct __ec_todo_unpacked {
			/*
			 * Lid angle threshold for switching between tablet and
			 * clamshell mode.
			 */
			uint16_t lid_angle;

			/* Hysteresis degree. */
			uint16_t hys_degree;
		} tablet_mode_threshold;

		/* USED for MOTIONSENSE_CMD_GET_ACTIVITY. */
		struct __ec_todo_unpacked {
			uint8_t state;
		} get_activity;
	};
} __ec_todo_packed;

/*****************************************************************************/
/* Force lid open command */

/* Make lid event always open */
#define EC_CMD_FORCE_LID_OPEN 0x002C

struct ec_params_force_lid_open {
	uint8_t enabled;
} __ec_align1;

/*****************************************************************************/
/* Configure the behavior of the power button */
#define EC_CMD_CONFIG_POWER_BUTTON 0x002D

enum ec_config_power_button_flags {
	/* Enable/Disable power button pulses for x86 devices */
	EC_POWER_BUTTON_ENABLE_PULSE = BIT(0),
};

struct ec_params_config_power_button {
	/* See enum ec_config_power_button_flags */
	uint8_t flags;
} __ec_align1;

/*****************************************************************************/
/* USB charging control commands */

/* Set USB port charging mode */
#define EC_CMD_USB_CHARGE_SET_MODE 0x0030

enum usb_charge_mode {
	/* Disable USB port. */
	USB_CHARGE_MODE_DISABLED,
	/* Set USB port to Standard Downstream Port, USB 2.0 mode. */
	USB_CHARGE_MODE_SDP2,
	/* Set USB port to Charging Downstream Port, BC 1.2. */
	USB_CHARGE_MODE_CDP,
	/* Set USB port to Dedicated Charging Port, BC 1.2. */
	USB_CHARGE_MODE_DCP_SHORT,
	/* Enable USB port (for dumb ports). */
	USB_CHARGE_MODE_ENABLED,
	/* Set USB port to CONFIG_USB_PORT_POWER_SMART_DEFAULT_MODE. */
	USB_CHARGE_MODE_DEFAULT,

	USB_CHARGE_MODE_COUNT,
};

enum usb_suspend_charge {
	/* Enable charging in suspend */
	USB_ALLOW_SUSPEND_CHARGE,
	/* Disable charging in suspend */
	USB_DISALLOW_SUSPEND_CHARGE,
};

struct ec_params_usb_charge_set_mode {
	uint8_t usb_port_id;
	uint8_t mode : 7; /* enum usb_charge_mode */
	uint8_t inhibit_charge : 1; /* enum usb_suspend_charge */
} __ec_align1;

/*****************************************************************************/
/* Tablet mode commands */

/* Set tablet mode */
#define EC_CMD_SET_TABLET_MODE 0x0031

enum tablet_mode_override {
	TABLET_MODE_DEFAULT,
	TABLET_MODE_FORCE_TABLET,
	TABLET_MODE_FORCE_CLAMSHELL,
};

struct ec_params_set_tablet_mode {
	uint8_t tablet_mode; /* enum tablet_mode_override */
} __ec_align1;

/*****************************************************************************/
/* Persistent storage for host */

/* Maximum bytes that can be read/written in a single command */
#define EC_PSTORE_SIZE_MAX 64

/* Get persistent storage info */
#define EC_CMD_PSTORE_INFO 0x0040

struct ec_response_pstore_info {
	/* Persistent storage size, in bytes */
	uint32_t pstore_size;
	/* Access size; read/write offset and size must be a multiple of this */
	uint32_t access_size;
} __ec_align4;

/*
 * Read persistent storage
 *
 * Response is params.size bytes of data.
 */
#define EC_CMD_PSTORE_READ 0x0041

struct ec_params_pstore_read {
	uint32_t offset; /* Byte offset to read */
	uint32_t size; /* Size to read in bytes */
} __ec_align4;

/* Write persistent storage */
#define EC_CMD_PSTORE_WRITE 0x0042

struct ec_params_pstore_write {
	uint32_t offset; /* Byte offset to write */
	uint32_t size; /* Size to write in bytes */
	uint8_t data[EC_PSTORE_SIZE_MAX];
} __ec_align4;

/*****************************************************************************/
/* Real-time clock */

/* RTC params and response structures */
struct ec_params_rtc {
	uint32_t time;
} __ec_align4;

struct ec_response_rtc {
	uint32_t time;
} __ec_align4;

/* These use ec_response_rtc */
#define EC_CMD_RTC_GET_VALUE 0x0044
#define EC_CMD_RTC_GET_ALARM 0x0045

/* These all use ec_params_rtc */
#define EC_CMD_RTC_SET_VALUE 0x0046
#define EC_CMD_RTC_SET_ALARM 0x0047

/* Pass as time param to SET_ALARM to clear the current alarm */
#define EC_RTC_ALARM_CLEAR 0

/*****************************************************************************/
/* Port80 log access */

/* Maximum entries that can be read/written in a single command */
#define EC_PORT80_SIZE_MAX 32

/* Get last port80 code from previous boot */
#define EC_CMD_PORT80_LAST_BOOT 0x0048
#define EC_CMD_PORT80_READ 0x0048

enum ec_port80_subcmd {
	EC_PORT80_GET_INFO = 0,
	EC_PORT80_READ_BUFFER,
};

struct ec_params_port80_read {
	uint16_t subcmd;
	union {
		struct __ec_todo_unpacked {
			uint32_t offset;
			uint32_t num_entries;
		} read_buffer;
	} __ec_todo_packed;
} __ec_todo_packed;

struct ec_response_port80_read {
	union {
		struct __ec_todo_unpacked {
			uint32_t writes;
			uint32_t history_size;
			uint32_t last_boot;
		} get_info;
		struct __ec_todo_unpacked {
			uint16_t codes[EC_PORT80_SIZE_MAX];
		} data;
	};
} __ec_todo_packed;

struct ec_response_port80_last_boot {
	uint16_t code;
} __ec_align2;

/*****************************************************************************/
/* Temporary secure storage for host verified boot use */

/* Number of bytes in a vstore slot */
#define EC_VSTORE_SLOT_SIZE 64

/* Maximum number of vstore slots */
#define EC_VSTORE_SLOT_MAX 32

/* Get persistent storage info */
#define EC_CMD_VSTORE_INFO 0x0049
struct ec_response_vstore_info {
	/* Indicates which slots are locked */
	uint32_t slot_locked;
	/* Total number of slots available */
	uint8_t slot_count;
} __ec_align_size1;

/*
 * Read temporary secure storage
 *
 * Response is EC_VSTORE_SLOT_SIZE bytes of data.
 */
#define EC_CMD_VSTORE_READ 0x004A

struct ec_params_vstore_read {
	uint8_t slot; /* Slot to read from */
} __ec_align1;

struct ec_response_vstore_read {
	uint8_t data[EC_VSTORE_SLOT_SIZE];
} __ec_align1;

/*
 * Write temporary secure storage and lock it.
 */
#define EC_CMD_VSTORE_WRITE 0x004B

struct ec_params_vstore_write {
	uint8_t slot; /* Slot to write to */
	uint8_t data[EC_VSTORE_SLOT_SIZE];
} __ec_align1;

/*****************************************************************************/
/* Thermal engine commands. Note that there are two implementations. We'll
 * reuse the command number, but the data and behavior is incompatible.
 * Version 0 is what originally shipped on Link.
 * Version 1 separates the CPU thermal limits from the fan control.
 */

#define EC_CMD_THERMAL_SET_THRESHOLD 0x0050
#define EC_CMD_THERMAL_GET_THRESHOLD 0x0051

/* The version 0 structs are opaque. You have to know what they are for
 * the get/set commands to make any sense.
 */

/* Version 0 - set */
struct ec_params_thermal_set_threshold {
	uint8_t sensor_type;
	uint8_t threshold_id;
	uint16_t value;
} __ec_align2;

/* Version 0 - get */
struct ec_params_thermal_get_threshold {
	uint8_t sensor_type;
	uint8_t threshold_id;
} __ec_align1;

struct ec_response_thermal_get_threshold {
	uint16_t value;
} __ec_align2;

/* The version 1 structs are visible. */
enum ec_temp_thresholds {
	EC_TEMP_THRESH_WARN = 0,
	EC_TEMP_THRESH_HIGH,
	EC_TEMP_THRESH_HALT,

	EC_TEMP_THRESH_COUNT,
};

/*
 * Thermal configuration for one temperature sensor. Temps are in degrees K.
 * Zero values will be silently ignored by the thermal task.
 *
 * Set 'temp_host' value allows thermal task to trigger some event with 1 degree
 * hysteresis.
 * For example,
 *	temp_host[EC_TEMP_THRESH_HIGH] = 300 K
 *	temp_host_release[EC_TEMP_THRESH_HIGH] = 0 K
 * EC will throttle ap when temperature >= 301 K, and release throttling when
 * temperature <= 299 K.
 *
 * Set 'temp_host_release' value allows thermal task has a custom hysteresis.
 * For example,
 *	temp_host[EC_TEMP_THRESH_HIGH] = 300 K
 *	temp_host_release[EC_TEMP_THRESH_HIGH] = 295 K
 * EC will throttle ap when temperature >= 301 K, and release throttling when
 * temperature <= 294 K.
 *
 * Note that this structure is a sub-structure of
 * ec_params_thermal_set_threshold_v1, but maintains its alignment there.
 */
struct ec_thermal_config {
	uint32_t temp_host[EC_TEMP_THRESH_COUNT]; /* levels of hotness */
	uint32_t temp_host_release[EC_TEMP_THRESH_COUNT]; /* release levels */
	uint32_t temp_fan_off; /* no active cooling needed */
	uint32_t temp_fan_max; /* max active cooling needed */
} __ec_align4;

/* Version 1 - get config for one sensor. */
struct ec_params_thermal_get_threshold_v1 {
	uint32_t sensor_num;
} __ec_align4;
/* This returns a struct ec_thermal_config */

/*
 * Version 1 - set config for one sensor.
 * Use read-modify-write for best results!
 */
struct ec_params_thermal_set_threshold_v1 {
	uint32_t sensor_num;
	struct ec_thermal_config cfg;
} __ec_align4;
/* This returns no data */

/****************************************************************************/

/* Set or get fan control mode */
#define EC_CMD_THERMAL_AUTO_FAN_CTRL 0x0052

enum ec_auto_fan_ctrl_cmd {
	EC_AUTO_FAN_CONTROL_CMD_SET = 0,
	EC_AUTO_FAN_CONTROL_CMD_GET,
};

/* Version 1 of input params */
struct ec_params_auto_fan_ctrl_v1 {
	uint8_t fan_idx;
} __ec_align1;

/* Version 2 of input params */
struct ec_params_auto_fan_ctrl_v2 {
	uint8_t fan_idx;
	uint8_t cmd; /* enum ec_auto_fan_ctrl_cmd */
	uint8_t set_auto; /* only used with EC_AUTO_FAN_CONTROL_CMD_SET - bool
			   */
} __ec_align4;

struct ec_response_auto_fan_control {
	uint8_t is_auto; /* bool */
} __ec_align1;

/* Get/Set TMP006 calibration data */
#define EC_CMD_TMP006_GET_CALIBRATION 0x0053
#define EC_CMD_TMP006_SET_CALIBRATION 0x0054

/*
 * The original TMP006 calibration only needed four params, but now we need
 * more. Since the algorithm is nothing but magic numbers anyway, we'll leave
 * the params opaque. The v1 "get" response will include the algorithm number
 * and how many params it requires. That way we can change the EC code without
 * needing to update this file. We can also use a different algorithm on each
 * sensor.
 */

/* This is the same struct for both v0 and v1. */
struct ec_params_tmp006_get_calibration {
	uint8_t index;
} __ec_align1;

/* Version 0 */
struct ec_response_tmp006_get_calibration_v0 {
	float s0;
	float b0;
	float b1;
	float b2;
} __ec_align4;

struct ec_params_tmp006_set_calibration_v0 {
	uint8_t index;
	uint8_t reserved[3];
	float s0;
	float b0;
	float b1;
	float b2;
} __ec_align4;

/* Version 1 */
struct ec_response_tmp006_get_calibration_v1 {
	uint8_t algorithm;
	uint8_t num_params;
	uint8_t reserved[2];
	float val[FLEXIBLE_ARRAY_MEMBER_SIZE];
} __ec_align4;

struct ec_params_tmp006_set_calibration_v1 {
	uint8_t index;
	uint8_t algorithm;
	uint8_t num_params;
	uint8_t reserved;
	float val[FLEXIBLE_ARRAY_MEMBER_SIZE];
} __ec_align4;

/* Read raw TMP006 data */
#define EC_CMD_TMP006_GET_RAW 0x0055

struct ec_params_tmp006_get_raw {
	uint8_t index;
} __ec_align1;

struct ec_response_tmp006_get_raw {
	int32_t t; /* In 1/100 K */
	int32_t v; /* In nV */
} __ec_align4;

/*****************************************************************************/
/* MKBP - Matrix KeyBoard Protocol */

/*
 * Read key state
 *
 * Returns raw data for keyboard cols; see ec_response_mkbp_info.cols for
 * expected response size.
 *
 * NOTE: This has been superseded by EC_CMD_MKBP_GET_NEXT_EVENT.  If you wish
 * to obtain the instantaneous state, use EC_CMD_MKBP_INFO with the type
 * EC_MKBP_INFO_CURRENT and event EC_MKBP_EVENT_KEY_MATRIX.
 */
#define EC_CMD_MKBP_STATE 0x0060

/*
 * Provide information about various MKBP things.  See enum ec_mkbp_info_type.
 */
#define EC_CMD_MKBP_INFO 0x0061

struct ec_response_mkbp_info {
	uint32_t rows;
	uint32_t cols;
	/* Formerly "switches", which was 0. */
	uint8_t reserved;
} __ec_align_size1;

struct ec_params_mkbp_info {
	uint8_t info_type;
	uint8_t event_type;
} __ec_align1;

enum ec_mkbp_info_type {
	/*
	 * Info about the keyboard matrix: number of rows and columns.
	 *
	 * Returns struct ec_response_mkbp_info.
	 */
	EC_MKBP_INFO_KBD = 0,

	/*
	 * For buttons and switches, info about which specifically are
	 * supported.  event_type must be set to one of the values in enum
	 * ec_mkbp_event.
	 *
	 * For EC_MKBP_EVENT_BUTTON and EC_MKBP_EVENT_SWITCH, returns a 4 byte
	 * bitmask indicating which buttons or switches are present.  See the
	 * bit inidices below.
	 */
	EC_MKBP_INFO_SUPPORTED = 1,

	/*
	 * Instantaneous state of buttons and switches.
	 *
	 * event_type must be set to one of the values in enum ec_mkbp_event.
	 *
	 * For EC_MKBP_EVENT_KEY_MATRIX, returns uint8_t key_matrix[13]
	 * indicating the current state of the keyboard matrix.
	 *
	 * For EC_MKBP_EVENT_HOST_EVENT, return uint32_t host_event, the raw
	 * event state.
	 *
	 * For EC_MKBP_EVENT_BUTTON, returns uint32_t buttons, indicating the
	 * state of supported buttons.
	 *
	 * For EC_MKBP_EVENT_SWITCH, returns uint32_t switches, indicating the
	 * state of supported switches.
	 */
	EC_MKBP_INFO_CURRENT = 2,
};

/* Simulate key press */
#define EC_CMD_MKBP_SIMULATE_KEY 0x0062

struct ec_params_mkbp_simulate_key {
	uint8_t col;
	uint8_t row;
	uint8_t pressed;
} __ec_align1;

/* Configure keyboard scanning */
#define EC_CMD_MKBP_SET_CONFIG 0x0064
#define EC_CMD_MKBP_GET_CONFIG 0x0065

/* flags */
enum mkbp_config_flags {
	EC_MKBP_FLAGS_ENABLE = 1, /* Enable keyboard scanning */
};

enum mkbp_config_valid {
	EC_MKBP_VALID_SCAN_PERIOD = BIT(0),
	EC_MKBP_VALID_POLL_TIMEOUT = BIT(1),
	EC_MKBP_VALID_MIN_POST_SCAN_DELAY = BIT(3),
	EC_MKBP_VALID_OUTPUT_SETTLE = BIT(4),
	EC_MKBP_VALID_DEBOUNCE_DOWN = BIT(5),
	EC_MKBP_VALID_DEBOUNCE_UP = BIT(6),
	EC_MKBP_VALID_FIFO_MAX_DEPTH = BIT(7),
};

/*
 * Configuration for our key scanning algorithm.
 *
 * Note that this is used as a sub-structure of
 * ec_{params/response}_mkbp_get_config.
 */
struct ec_mkbp_config {
	uint32_t valid_mask; /* valid fields */
	uint8_t flags; /* some flags (enum mkbp_config_flags) */
	uint8_t valid_flags; /* which flags are valid */
	uint16_t scan_period_us; /* period between start of scans */
	/* revert to interrupt mode after no activity for this long */
	uint32_t poll_timeout_us;
	/*
	 * minimum post-scan relax time. Once we finish a scan we check
	 * the time until we are due to start the next one. If this time is
	 * shorter this field, we use this instead.
	 */
	uint16_t min_post_scan_delay_us;
	/* delay between setting up output and waiting for it to settle */
	uint16_t output_settle_us;
	uint16_t debounce_down_us; /* time for debounce on key down */
	uint16_t debounce_up_us; /* time for debounce on key up */
	/* maximum depth to allow for fifo (0 = no keyscan output) */
	uint8_t fifo_max_depth;
} __ec_align_size1;

struct ec_params_mkbp_set_config {
	struct ec_mkbp_config config;
} __ec_align_size1;

struct ec_response_mkbp_get_config {
	struct ec_mkbp_config config;
} __ec_align_size1;

/* Run the key scan emulation */
#define EC_CMD_KEYSCAN_SEQ_CTRL 0x0066

enum ec_keyscan_seq_cmd {
	EC_KEYSCAN_SEQ_STATUS = 0, /* Get status information */
	EC_KEYSCAN_SEQ_CLEAR = 1, /* Clear sequence */
	EC_KEYSCAN_SEQ_ADD = 2, /* Add item to sequence */
	EC_KEYSCAN_SEQ_START = 3, /* Start running sequence */
	EC_KEYSCAN_SEQ_COLLECT = 4, /* Collect sequence summary data */
};

enum ec_collect_flags {
	/*
	 * Indicates this scan was processed by the EC. Due to timing, some
	 * scans may be skipped.
	 */
	EC_KEYSCAN_SEQ_FLAG_DONE = BIT(0),
};

struct ec_collect_item {
	uint8_t flags; /* some flags (enum ec_collect_flags) */
} __ec_align1;

struct ec_params_keyscan_seq_ctrl {
	uint8_t cmd; /* Command to send (enum ec_keyscan_seq_cmd) */
	union {
		struct __ec_align1 {
			uint8_t active; /* still active */
			uint8_t num_items; /* number of items */
			/* Current item being presented */
			uint8_t cur_item;
		} status;
		struct __ec_todo_unpacked {
			/*
			 * Absolute time for this scan, measured from the
			 * start of the sequence.
			 */
			uint32_t time_us;
			/* keyscan data */
			uint8_t scan[FLEXIBLE_ARRAY_MEMBER_SIZE];
		} add;
		struct __ec_align1 {
			uint8_t start_item; /* First item to return */
			uint8_t num_items; /* Number of items to return */
		} collect;
	};
} __ec_todo_packed;

struct ec_result_keyscan_seq_ctrl {
	union {
		struct __ec_todo_unpacked {
			uint8_t num_items; /* Number of items */
			/* Data for each item */
			struct ec_collect_item item[FLEXIBLE_ARRAY_MEMBER_SIZE];
		} collect;
	};
} __ec_todo_packed;

/*
 * Get the next pending MKBP event.
 *
 * Returns EC_RES_UNAVAILABLE if there is no event pending.
 *
 * V0: ec_response_get_next_data
 * V1: ec_response_get_next_data_v1. Increased key_matrix size from 13 -> 16.
 * V2: Added EC_MKBP_HAS_MORE_EVENTS.
 * V3: ec_response_get_next_data_v3. Increased key_matrix size from 16 -> 18.
 */
#define EC_CMD_GET_NEXT_EVENT 0x0067

#define EC_MKBP_HAS_MORE_EVENTS_SHIFT 7

/*
 * We use the most significant bit of the event type to indicate to the host
 * that the EC has more MKBP events available to provide.
 */
#define EC_MKBP_HAS_MORE_EVENTS BIT(EC_MKBP_HAS_MORE_EVENTS_SHIFT)

/* The mask to apply to get the raw event type */
#define EC_MKBP_EVENT_TYPE_MASK (BIT(EC_MKBP_HAS_MORE_EVENTS_SHIFT) - 1)

enum ec_mkbp_event {
	/* Keyboard matrix changed. The event data is the new matrix state. */
	EC_MKBP_EVENT_KEY_MATRIX = 0,

	/* New host event. The event data is 4 bytes of host event flags. */
	EC_MKBP_EVENT_HOST_EVENT = 1,

	/* New Sensor FIFO data. The event data is fifo_info structure. */
	EC_MKBP_EVENT_SENSOR_FIFO = 2,

	/* The state of the non-matrixed buttons have changed. */
	EC_MKBP_EVENT_BUTTON = 3,

	/* The state of the switches have changed. */
	EC_MKBP_EVENT_SWITCH = 4,

	/* New Fingerprint sensor event, the event data is fp_events bitmap. */
	EC_MKBP_EVENT_FINGERPRINT = 5,

	/*
	 * Sysrq event: send emulated sysrq. The event data is sysrq,
	 * corresponding to the key to be pressed.
	 */
	EC_MKBP_EVENT_SYSRQ = 6,

	/*
	 * New 64-bit host event.
	 * The event data is 8 bytes of host event flags.
	 */
	EC_MKBP_EVENT_HOST_EVENT64 = 7,

	/* Notify the AP that something happened on CEC */
	EC_MKBP_EVENT_CEC_EVENT = 8,

	/* Send an incoming CEC message to the AP */
	EC_MKBP_EVENT_CEC_MESSAGE = 9,

	/* We have entered DisplayPort Alternate Mode on a Type-C port. */
	EC_MKBP_EVENT_DP_ALT_MODE_ENTERED = 10,

	/* New online calibration values are available. */
	EC_MKBP_EVENT_ONLINE_CALIBRATION = 11,

	/* Peripheral device charger event */
	EC_MKBP_EVENT_PCHG = 12,

	/* Number of MKBP events */
	EC_MKBP_EVENT_COUNT,
};
BUILD_ASSERT(EC_MKBP_EVENT_COUNT <= EC_MKBP_EVENT_TYPE_MASK);

/* clang-format off */
#define EC_MKBP_EVENT_TEXT                                                     \
	{                                                                      \
		[EC_MKBP_EVENT_KEY_MATRIX] = "KEY_MATRIX",                     \
		[EC_MKBP_EVENT_HOST_EVENT] = "HOST_EVENT",                     \
		[EC_MKBP_EVENT_SENSOR_FIFO] = "SENSOR_FIFO",                   \
		[EC_MKBP_EVENT_BUTTON] = "BUTTON",                             \
		[EC_MKBP_EVENT_SWITCH] = "SWITCH",                             \
		[EC_MKBP_EVENT_FINGERPRINT] = "FINGERPRINT",                   \
		[EC_MKBP_EVENT_SYSRQ] = "SYSRQ",                               \
		[EC_MKBP_EVENT_HOST_EVENT64] = "HOST_EVENT64",                 \
		[EC_MKBP_EVENT_CEC_EVENT] = "CEC_EVENT",                       \
		[EC_MKBP_EVENT_CEC_MESSAGE] = "CEC_MESSAGE",                   \
		[EC_MKBP_EVENT_DP_ALT_MODE_ENTERED] = "DP_ALT_MODE_ENTERED",   \
		[EC_MKBP_EVENT_ONLINE_CALIBRATION] = "ONLINE_CALIBRATION",     \
		[EC_MKBP_EVENT_PCHG] = "PCHG",                                 \
	}
/* clang-format on */

union __ec_align_offset1 ec_response_get_next_data {
	uint8_t key_matrix[13];

	/* Unaligned */
	uint32_t host_event;
	uint64_t host_event64;

	struct __ec_todo_unpacked {
		/* For aligning the fifo_info */
		uint8_t reserved[3];
		struct ec_response_motion_sense_fifo_info info;
	} sensor_fifo;

	uint32_t buttons;

	uint32_t switches;

	uint32_t fp_events;

	uint32_t sysrq;

	/* CEC events from enum mkbp_cec_event */
	uint32_t cec_events;
};

union __ec_align_offset1 ec_response_get_next_data_v1 {
	uint8_t key_matrix[16];

	/* Unaligned */
	uint32_t host_event;
	uint64_t host_event64;

	struct __ec_todo_unpacked {
		/* For aligning the fifo_info */
		uint8_t reserved[3];
		struct ec_response_motion_sense_fifo_info info;
	} sensor_fifo;

	uint32_t buttons;

	uint32_t switches;

	uint32_t fp_events;

	uint32_t sysrq;

	/* CEC events from enum mkbp_cec_event */
	uint32_t cec_events;

	uint8_t cec_message[16];
};
BUILD_ASSERT(sizeof(union ec_response_get_next_data_v1) == 16);

union __ec_align_offset1 ec_response_get_next_data_v3 {
	uint8_t key_matrix[18];

	/* Unaligned */
	uint32_t host_event;
	uint64_t host_event64;

	struct __ec_todo_unpacked {
		/* For aligning the fifo_info */
		uint8_t reserved[3];
		struct ec_response_motion_sense_fifo_info info;
	} sensor_fifo;

	uint32_t buttons;

	uint32_t switches;

	uint32_t fp_events;

	uint32_t sysrq;

	/* CEC events from enum mkbp_cec_event */
	uint32_t cec_events;

	uint8_t cec_message[16];
};
BUILD_ASSERT(sizeof(union ec_response_get_next_data_v3) == 18);

struct ec_response_get_next_event {
	uint8_t event_type;
	/* Followed by event data if any */
	union ec_response_get_next_data data;
} __ec_align1;

struct ec_response_get_next_event_v1 {
	uint8_t event_type;
	/* Followed by event data if any */
	union ec_response_get_next_data_v1 data;
} __ec_align1;

struct ec_response_get_next_event_v3 {
	uint8_t event_type;
	/* Followed by event data if any */
	union ec_response_get_next_data_v3 data;
} __ec_align1;

/* Bit indices for buttons and switches.*/
/* Buttons */
#define EC_MKBP_POWER_BUTTON 0
#define EC_MKBP_VOL_UP 1
#define EC_MKBP_VOL_DOWN 2
#define EC_MKBP_RECOVERY 3

/* Switches */
#define EC_MKBP_LID_OPEN 0
#define EC_MKBP_TABLET_MODE 1
#define EC_MKBP_BASE_ATTACHED 2
#define EC_MKBP_FRONT_PROXIMITY 3

/* Run keyboard factory test scanning */
#define EC_CMD_KEYBOARD_FACTORY_TEST 0x0068

struct ec_response_keyboard_factory_test {
	uint16_t shorted; /* Keyboard pins are shorted */
} __ec_align2;

/* Fingerprint events in 'fp_events' for EC_MKBP_EVENT_FINGERPRINT */
#define EC_MKBP_FP_RAW_EVENT(fp_events) ((fp_events) & 0x00FFFFFF)
#define EC_MKBP_FP_ERRCODE(fp_events) ((fp_events) & 0x0000000F)
#define EC_MKBP_FP_ENROLL_PROGRESS_OFFSET 4
#define EC_MKBP_FP_ENROLL_PROGRESS(fpe) \
	(((fpe) & 0x00000FF0) >> EC_MKBP_FP_ENROLL_PROGRESS_OFFSET)
#define EC_MKBP_FP_MATCH_IDX_OFFSET 12
#define EC_MKBP_FP_MATCH_IDX_MASK 0x0000F000
#define EC_MKBP_FP_MATCH_IDX(fpe) \
	(((fpe) & EC_MKBP_FP_MATCH_IDX_MASK) >> EC_MKBP_FP_MATCH_IDX_OFFSET)
#define EC_MKBP_FP_ENROLL BIT(27)
#define EC_MKBP_FP_MATCH BIT(28)
#define EC_MKBP_FP_FINGER_DOWN BIT(29)
#define EC_MKBP_FP_FINGER_UP BIT(30)
#define EC_MKBP_FP_IMAGE_READY BIT(31)
/* code given by EC_MKBP_FP_ERRCODE() when EC_MKBP_FP_ENROLL is set */
#define EC_MKBP_FP_ERR_ENROLL_OK 0
#define EC_MKBP_FP_ERR_ENROLL_LOW_QUALITY 1
#define EC_MKBP_FP_ERR_ENROLL_IMMOBILE 2
#define EC_MKBP_FP_ERR_ENROLL_LOW_COVERAGE 3
#define EC_MKBP_FP_ERR_ENROLL_INTERNAL 5
/* Can be used to detect if image was usable for enrollment or not. */
#define EC_MKBP_FP_ERR_ENROLL_PROBLEM_MASK 1
/* code given by EC_MKBP_FP_ERRCODE() when EC_MKBP_FP_MATCH is set */
#define EC_MKBP_FP_ERR_MATCH_NO 0
#define EC_MKBP_FP_ERR_MATCH_NO_INTERNAL 6
#define EC_MKBP_FP_ERR_MATCH_NO_TEMPLATES 7
#define EC_MKBP_FP_ERR_MATCH_NO_AUTH_FAIL 8
#define EC_MKBP_FP_ERR_MATCH_NO_LOW_QUALITY 2
#define EC_MKBP_FP_ERR_MATCH_NO_LOW_COVERAGE 4
#define EC_MKBP_FP_ERR_MATCH_YES 1
#define EC_MKBP_FP_ERR_MATCH_YES_UPDATED 3
#define EC_MKBP_FP_ERR_MATCH_YES_UPDATE_FAILED 5

#define EC_CMD_MKBP_WAKE_MASK 0x0069
enum ec_mkbp_event_mask_action {
	/* Retrieve the value of a wake mask. */
	GET_WAKE_MASK = 0,

	/* Set the value of a wake mask. */
	SET_WAKE_MASK,
};

enum ec_mkbp_mask_type {
	/*
	 * These are host events sent via MKBP.
	 *
	 * Some examples are:
	 *    EC_HOST_EVENT_MASK(EC_HOST_EVENT_LID_OPEN)
	 *    EC_HOST_EVENT_MASK(EC_HOST_EVENT_KEY_PRESSED)
	 *
	 * The only things that should be in this mask are:
	 *    EC_HOST_EVENT_MASK(EC_HOST_EVENT_*)
	 */
	EC_MKBP_HOST_EVENT_WAKE_MASK = 0,

	/*
	 * These are MKBP events. Some examples are:
	 *
	 *    EC_MKBP_EVENT_KEY_MATRIX
	 *    EC_MKBP_EVENT_SWITCH
	 *
	 * The only things that should be in this mask are EC_MKBP_EVENT_*.
	 */
	EC_MKBP_EVENT_WAKE_MASK,
};

struct ec_params_mkbp_event_wake_mask {
	/* One of enum ec_mkbp_event_mask_action */
	uint8_t action;

	/*
	 * Which MKBP mask are you interested in acting upon?  This is one of
	 * ec_mkbp_mask_type.
	 */
	uint8_t mask_type;

	/* If setting a new wake mask, this contains the mask to set. */
	uint32_t new_wake_mask;
};

struct ec_response_mkbp_event_wake_mask {
	uint32_t wake_mask;
};

/*****************************************************************************/
/* Temperature sensor commands */

/* Read temperature sensor info */
#define EC_CMD_TEMP_SENSOR_GET_INFO 0x0070

struct ec_params_temp_sensor_get_info {
	uint8_t id;
} __ec_align1;

struct ec_response_temp_sensor_get_info {
	char sensor_name[32];
	uint8_t sensor_type;
} __ec_align1;

/*****************************************************************************/

/*
 * Note: host commands 0x80 - 0x87 are reserved to avoid conflict with ACPI
 * commands accidentally sent to the wrong interface.  See the ACPI section
 * below.
 */

/*****************************************************************************/
/* Host event commands */

/* Obsolete. New implementation should use EC_CMD_HOST_EVENT instead */
/*
 * Host event mask params and response structures, shared by all of the host
 * event commands below.
 */
struct ec_params_host_event_mask {
	uint32_t mask;
} __ec_align4;

struct ec_response_host_event_mask {
	uint32_t mask;
} __ec_align4;

/* These all use ec_response_host_event_mask */
#define EC_CMD_HOST_EVENT_GET_B 0x0087
#define EC_CMD_HOST_EVENT_GET_SMI_MASK 0x0088
#define EC_CMD_HOST_EVENT_GET_SCI_MASK 0x0089
#define EC_CMD_HOST_EVENT_GET_WAKE_MASK 0x008D

/* These all use ec_params_host_event_mask */
#define EC_CMD_HOST_EVENT_SET_SMI_MASK 0x008A
#define EC_CMD_HOST_EVENT_SET_SCI_MASK 0x008B
#define EC_CMD_HOST_EVENT_CLEAR 0x008C
#define EC_CMD_HOST_EVENT_SET_WAKE_MASK 0x008E
#define EC_CMD_HOST_EVENT_CLEAR_B 0x008F

/*
 * Unified host event programming interface - Should be used by newer versions
 * of BIOS/OS to program host events and masks
 *
 * EC returns:
 * - EC_RES_INVALID_PARAM: Action or mask type is unknown.
 * - EC_RES_ACCESS_DENIED: Action is prohibited for specified mask type.
 */

struct ec_params_host_event {
	/* Action requested by host - one of enum ec_host_event_action. */
	uint8_t action;

	/*
	 * Mask type that the host requested the action on - one of
	 * enum ec_host_event_mask_type.
	 */
	uint8_t mask_type;

	/* Set to 0, ignore on read */
	uint16_t reserved;

	/* Value to be used in case of set operations. */
	uint64_t value;
} __ec_align4;

/*
 * Response structure returned by EC_CMD_HOST_EVENT.
 * Update the value on a GET request. Set to 0 on GET/CLEAR
 */

struct ec_response_host_event {
	/* Mask value in case of get operation */
	uint64_t value;
} __ec_align4;

enum ec_host_event_action {
	/*
	 * params.value is ignored. Value of mask_type populated
	 * in response.value
	 */
	EC_HOST_EVENT_GET,

	/* Bits in params.value are set */
	EC_HOST_EVENT_SET,

	/* Bits in params.value are cleared */
	EC_HOST_EVENT_CLEAR,
};

enum ec_host_event_mask_type {

	/* Main host event copy */
	EC_HOST_EVENT_MAIN,

	/* Copy B of host events */
	EC_HOST_EVENT_B,

	/* SCI Mask */
	EC_HOST_EVENT_SCI_MASK,

	/* SMI Mask */
	EC_HOST_EVENT_SMI_MASK,

	/* Mask of events that should be always reported in hostevents */
	EC_HOST_EVENT_ALWAYS_REPORT_MASK,

	/* Active wake mask */
	EC_HOST_EVENT_ACTIVE_WAKE_MASK,

	/* Lazy wake mask for S0ix */
	EC_HOST_EVENT_LAZY_WAKE_MASK_S0IX,

	/* Lazy wake mask for S3 */
	EC_HOST_EVENT_LAZY_WAKE_MASK_S3,

	/* Lazy wake mask for S5 */
	EC_HOST_EVENT_LAZY_WAKE_MASK_S5,
};

#define EC_CMD_HOST_EVENT 0x00A4

/*****************************************************************************/
/* Switch commands */

/* Enable/disable LCD backlight */
#define EC_CMD_SWITCH_ENABLE_BKLIGHT 0x0090

struct ec_params_switch_enable_backlight {
	uint8_t enabled;
} __ec_align1;

/* Enable/disable WLAN/Bluetooth */
#define EC_CMD_SWITCH_ENABLE_WIRELESS 0x0091
#define EC_VER_SWITCH_ENABLE_WIRELESS 1

/* Version 0 params; no response */
struct ec_params_switch_enable_wireless_v0 {
	uint8_t enabled;
} __ec_align1;

/* Version 1 params */
struct ec_params_switch_enable_wireless_v1 {
	/* Flags to enable now */
	uint8_t now_flags;

	/* Which flags to copy from now_flags */
	uint8_t now_mask;

	/*
	 * Flags to leave enabled in S3, if they're on at the S0->S3
	 * transition.  (Other flags will be disabled by the S0->S3
	 * transition.)
	 */
	uint8_t suspend_flags;

	/* Which flags to copy from suspend_flags */
	uint8_t suspend_mask;
} __ec_align1;

/* Version 1 response */
struct ec_response_switch_enable_wireless_v1 {
	/* Flags to enable now */
	uint8_t now_flags;

	/* Flags to leave enabled in S3 */
	uint8_t suspend_flags;
} __ec_align1;

/*****************************************************************************/
/* GPIO commands. Only available on EC if write protect has been disabled. */

/* Set GPIO output value */
#define EC_CMD_GPIO_SET 0x0092

struct ec_params_gpio_set {
	char name[32];
	uint8_t val;
} __ec_align1;

/* Get GPIO value */
#define EC_CMD_GPIO_GET 0x0093

/* Version 0 of input params and response */
struct ec_params_gpio_get {
	char name[32];
} __ec_align1;

struct ec_response_gpio_get {
	uint8_t val;
} __ec_align1;

/* Version 1 of input params and response */
struct ec_params_gpio_get_v1 {
	uint8_t subcmd;
	union {
		struct __ec_align1 {
			char name[32];
		} get_value_by_name;
		struct __ec_align1 {
			uint8_t index;
		} get_info;
	};
} __ec_align1;

struct ec_response_gpio_get_v1 {
	union {
		struct __ec_align1 {
			uint8_t val;
		} get_value_by_name, get_count;
		struct __ec_todo_unpacked {
			uint8_t val;
			char name[32];
			uint32_t flags;
		} get_info;
	};
} __ec_todo_packed;

enum gpio_get_subcmd {
	EC_GPIO_GET_BY_NAME = 0,
	EC_GPIO_GET_COUNT = 1,
	EC_GPIO_GET_INFO = 2,
};

/*****************************************************************************/
/* I2C commands. Only available when flash write protect is unlocked. */

/*
 * CAUTION: These commands are deprecated, and are not supported anymore in EC
 * builds >= 8398.0.0 (see crosbug.com/p/23570).
 *
 * Use EC_CMD_I2C_PASSTHRU instead.
 */

/* Read I2C bus */
#define EC_CMD_I2C_READ 0x0094

struct ec_params_i2c_read {
	uint16_t addr; /* 8-bit address (7-bit shifted << 1) */
	uint8_t read_size; /* Either 8 or 16. */
	uint8_t port;
	uint8_t offset;
} __ec_align_size1;

struct ec_response_i2c_read {
	uint16_t data;
} __ec_align2;

/* Write I2C bus */
#define EC_CMD_I2C_WRITE 0x0095

struct ec_params_i2c_write {
	uint16_t data;
	uint16_t addr; /* 8-bit address (7-bit shifted << 1) */
	uint8_t write_size; /* Either 8 or 16. */
	uint8_t port;
	uint8_t offset;
} __ec_align_size1;

/*****************************************************************************/
/* Charge state commands. Only available when flash write protect unlocked. */

/* Force charge state machine to stop charging the battery or force it to
 * discharge the battery.
 */
#define EC_CMD_CHARGE_CONTROL 0x0096
#define EC_VER_CHARGE_CONTROL 3

enum ec_charge_control_mode {
	CHARGE_CONTROL_NORMAL = 0,
	CHARGE_CONTROL_IDLE,
	CHARGE_CONTROL_DISCHARGE,
	/* Add no more entry below. */
	CHARGE_CONTROL_COUNT,
};

#define EC_CHARGE_MODE_TEXT                               \
	{                                                 \
		[CHARGE_CONTROL_NORMAL] = "NORMAL",       \
		[CHARGE_CONTROL_IDLE] = "IDLE",           \
		[CHARGE_CONTROL_DISCHARGE] = "DISCHARGE", \
	}

enum ec_charge_control_cmd {
	EC_CHARGE_CONTROL_CMD_SET = 0,
	EC_CHARGE_CONTROL_CMD_GET,
};

enum ec_charge_control_flag {
	EC_CHARGE_CONTROL_FLAG_NO_IDLE = BIT(0),
};

struct ec_params_charge_control {
	uint32_t mode; /* enum charge_control_mode */

	/* Below are the fields added in V2. */
	uint8_t cmd; /* enum ec_charge_control_cmd. */
	uint8_t flags; /* enum ec_charge_control_flag (v3+) */
	/*
	 * Lower and upper thresholds for battery sustainer. This struct isn't
	 * named to avoid tainting foreign projects' name spaces.
	 *
	 * If charge mode is explicitly set (e.g. DISCHARGE), battery sustainer
	 * will be disabled. To disable battery sustainer, set mode=NORMAL,
	 * lower=-1, upper=-1.
	 */
	struct {
		int8_t lower; /* Display SoC in percentage. */
		int8_t upper; /* Display SoC in percentage. */
	} sustain_soc;
} __ec_align4;

/* Added in v2 */
struct ec_response_charge_control {
	uint32_t mode; /* enum charge_control_mode */
	struct { /* Battery sustainer thresholds */
		int8_t lower;
		int8_t upper;
	} sustain_soc;
	uint8_t flags; /* enum ec_charge_control_flag (v3+) */
	uint8_t reserved;
} __ec_align4;

/*****************************************************************************/

/* Snapshot console output buffer for use by EC_CMD_CONSOLE_READ. */
#define EC_CMD_CONSOLE_SNAPSHOT 0x0097

/*
 * Read data from the saved snapshot. If the subcmd parameter is
 * CONSOLE_READ_NEXT, this will return data starting from the beginning of
 * the latest snapshot. If it is CONSOLE_READ_RECENT, it will start from the
 * end of the previous snapshot.
 *
 * The params are only looked at in version >= 1 of this command. Prior
 * versions will just default to CONSOLE_READ_NEXT behavior.
 *
 * Response is null-terminated string.  Empty string, if there is no more
 * remaining output.
 */
#define EC_CMD_CONSOLE_READ 0x0098

enum ec_console_read_subcmd {
	CONSOLE_READ_NEXT = 0,
	CONSOLE_READ_RECENT,
};

struct ec_params_console_read_v1 {
	uint8_t subcmd; /* enum ec_console_read_subcmd */
} __ec_align1;

/* Print directly to EC console from host. */
#define EC_CMD_CONSOLE_PRINT 0x00AC

/*****************************************************************************/

/*
 * Cut off battery power immediately or after the host has shut down.
 *
 * return EC_RES_INVALID_COMMAND if unsupported by a board/battery.
 *	  EC_RES_SUCCESS if the command was successful.
 *	  EC_RES_ERROR if the cut off command failed.
 */
#define EC_CMD_BATTERY_CUT_OFF 0x0099

#define EC_BATTERY_CUTOFF_FLAG_AT_SHUTDOWN BIT(0)

struct ec_params_battery_cutoff {
	uint8_t flags;
} __ec_align1;

/*****************************************************************************/
/* USB port mux control. */

/*
 * Switch USB mux or return to automatic switching.
 */
#define EC_CMD_USB_MUX 0x009A

struct ec_params_usb_mux {
	uint8_t mux;
} __ec_align1;

/*****************************************************************************/
/* LDOs / FETs control. */

enum ec_ldo_state {
	EC_LDO_STATE_OFF = 0, /* the LDO / FET is shut down */
	EC_LDO_STATE_ON = 1, /* the LDO / FET is ON / providing power */
};

/*
 * Switch on/off a LDO.
 */
#define EC_CMD_LDO_SET 0x009B

struct ec_params_ldo_set {
	uint8_t index;
	uint8_t state;
} __ec_align1;

/*
 * Get LDO state.
 */
#define EC_CMD_LDO_GET 0x009C

struct ec_params_ldo_get {
	uint8_t index;
} __ec_align1;

struct ec_response_ldo_get {
	uint8_t state;
} __ec_align1;

/*****************************************************************************/
/* Power info. */

/*
 * Get power info.
 *
 * Note: v0 of this command is deprecated
 */
#define EC_CMD_POWER_INFO 0x009D

/*
 * v1 of EC_CMD_POWER_INFO
 */
enum system_power_source {
	/*
	 * Haven't established which power source is used yet,
	 * or no presence signals are available
	 */
	POWER_SOURCE_UNKNOWN = 0,
	/* System is running on battery alone */
	POWER_SOURCE_BATTERY = 1,
	/* System is running on A/C alone */
	POWER_SOURCE_AC = 2,
	/* System is running on A/C and battery */
	POWER_SOURCE_AC_BATTERY = 3,
};

struct ec_response_power_info_v1 {
	/* enum system_power_source */
	uint8_t system_power_source;
	/* Battery state-of-charge, 0-100, 0 if not present */
	uint8_t battery_soc;
	/* AC Adapter 100% rating, Watts */
	uint8_t ac_adapter_100pct;
	/* AC Adapter 10ms rating, Watts */
	uint8_t ac_adapter_10ms;
	/* Battery 1C rating, derated */
	uint8_t battery_1cd;
	/* Rest of Platform average, Watts */
	uint8_t rop_avg;
	/* Rest of Platform peak, Watts */
	uint8_t rop_peak;
	/* Nominal charger efficiency, % */
	uint8_t nominal_charger_eff;
	/* Rest of Platform VR Average Efficiency, % */
	uint8_t rop_avg_eff;
	/* Rest of Platform VR Peak Efficiency, % */
	uint8_t rop_peak_eff;
	/* SoC VR Efficiency at Average level, % */
	uint8_t soc_avg_eff;
	/* SoC VR Efficiency at Peak level, % */
	uint8_t soc_peak_eff;
	/* Intel-specific items */
	struct {
		/* Battery's level of DBPT support: 0, 2 */
		uint8_t batt_dbpt_support_level;
		/*
		 * Maximum peak power from battery (10ms), Watts
		 * If DBPT is not supported, this is 0
		 */
		uint8_t batt_dbpt_max_peak_power;
		/*
		 * Sustained peak power from battery, Watts
		 * If DBPT is not supported, this is 0
		 */
		uint8_t batt_dbpt_sus_peak_power;
	} intel;
} __ec_align1;

/*****************************************************************************/
/* I2C passthru command */

#define EC_CMD_I2C_PASSTHRU 0x009E

/* Read data; if not present, message is a write */
#define EC_I2C_FLAG_READ BIT(15)

/* Mask for address */
#define EC_I2C_ADDR_MASK 0x3ff

#define EC_I2C_STATUS_NAK BIT(0) /* Transfer was not acknowledged */
#define EC_I2C_STATUS_TIMEOUT BIT(1) /* Timeout during transfer */

/* Any error */
#define EC_I2C_STATUS_ERROR (EC_I2C_STATUS_NAK | EC_I2C_STATUS_TIMEOUT)

struct ec_params_i2c_passthru_msg {
	uint16_t addr_flags; /* I2C peripheral address and flags */
	uint16_t len; /* Number of bytes to read or write */
} __ec_align2;

struct ec_params_i2c_passthru {
	uint8_t port; /* I2C port number */
	uint8_t num_msgs; /* Number of messages */
	struct ec_params_i2c_passthru_msg msg[FLEXIBLE_ARRAY_MEMBER_SIZE];
	/* Data to write for all messages is concatenated here */
} __ec_align2;

struct ec_response_i2c_passthru {
	uint8_t i2c_status; /* Status flags (EC_I2C_STATUS_...) */
	uint8_t num_msgs; /* Number of messages processed */
	/* Data read by messages concatenated here */
	uint8_t data[FLEXIBLE_ARRAY_MEMBER_SIZE];
} __ec_align1;

/*****************************************************************************/
/* AP hang detect */
#define EC_CMD_HANG_DETECT 0x009F

#define EC_HANG_DETECT_MIN_TIMEOUT 5

/* EC hang detect commands */
enum ec_hang_detect_cmds {
	/* Reload AP hang detect timer. */
	EC_HANG_DETECT_CMD_RELOAD = 0x0,

	/* Stop AP hang detect timer. */
	EC_HANG_DETECT_CMD_CANCEL = 0x1,

	/* Configure watchdog with given reboot timeout and
	 * cancel currently running AP hand detect timer.
	 */
	EC_HANG_DETECT_CMD_SET_TIMEOUT = 0x2,

	/* Get last hang status - whether the AP boot was clear or not */
	EC_HANG_DETECT_CMD_GET_STATUS = 0x3,

	/* Clear last hang status. Called when AP is rebooting/shutting down
	 * gracefully.
	 */
	EC_HANG_DETECT_CMD_CLEAR_STATUS = 0x4
};

struct ec_params_hang_detect {
	uint16_t command; /* enum ec_hang_detect_cmds */
	/* Timeout in seconds before generating reboot */
	uint16_t reboot_timeout_sec;
} __ec_align2;

/* Status codes that describe whether AP has boot normally or the hang has been
 * detected and EC has reset AP
 */
enum ec_hang_detect_status {
	EC_HANG_DETECT_AP_BOOT_NORMAL = 0x0,
	EC_HANG_DETECT_AP_BOOT_EC_WDT = 0x1,
	EC_HANG_DETECT_AP_BOOT_COUNT,
};
struct ec_response_hang_detect {
	uint8_t status; /* enum ec_hang_detect_status */
} __ec_align1;
/*****************************************************************************/
/* Commands for battery charging */

/*
 * This is the single catch-all host command to exchange data regarding the
 * charge state machine (v2 and up).
 */
#define EC_CMD_CHARGE_STATE 0x00A0

/* Subcommands for this host command */
enum charge_state_command {
	CHARGE_STATE_CMD_GET_STATE,
	CHARGE_STATE_CMD_GET_PARAM,
	CHARGE_STATE_CMD_SET_PARAM,
	CHARGE_STATE_NUM_CMDS,
};

/*
 * Known param numbers are defined here. Ranges are reserved for board-specific
 * params, which are handled by the particular implementations.
 */
enum charge_state_params {
	/* charger voltage limit */
	CS_PARAM_CHG_VOLTAGE,

	/* charger current limit */
	CS_PARAM_CHG_CURRENT,

	/* charger input current limit */
	CS_PARAM_CHG_INPUT_CURRENT,

	/* charger-specific status */
	CS_PARAM_CHG_STATUS,

	/* charger-specific options */
	CS_PARAM_CHG_OPTION,

	/*
	 * Check if power is limited due to low battery and / or a
	 * weak external charger. READ ONLY.
	 */
	CS_PARAM_LIMIT_POWER,

	/* min value of charger voltage limit (READ ONLY) */
	CS_PARAM_CHG_VOLTAGE_MIN,

	/* max value of charger voltage limit (READ ONLY) */
	CS_PARAM_CHG_VOLTAGE_MAX,

	/* step value of charger voltage limit (READ ONLY) */
	CS_PARAM_CHG_VOLTAGE_STEP,

	/* min value of charger current limit (READ ONLY) */
	CS_PARAM_CHG_CURRENT_MIN,

	/* max value of charger current limit (READ ONLY) */
	CS_PARAM_CHG_CURRENT_MAX,

	/* step value of charger current limit (READ ONLY) */
	CS_PARAM_CHG_CURRENT_STEP,

	/* min value of charger input current limit (READ ONLY) */
	CS_PARAM_CHG_INPUT_CURRENT_MIN,

	/* max value of charger input current limit (READ ONLY) */
	CS_PARAM_CHG_INPUT_CURRENT_MAX,

	/* step value of charger input current limit (READ ONLY) */
	CS_PARAM_CHG_INPUT_CURRENT_STEP,

	/* Minimum required voltage for hybrid boost chargers (READ ONLY) */
	CS_PARAM_CHG_MIN_REQUIRED_MV,

	/* For hybrid boost chargers returns !=0 when attached charger is
	 * capable of charging the battery
	 */
	CS_PARAM_CHG_IS_ADAPTER_SUFFICIENT,

	/* How many so far? */
	CS_NUM_BASE_PARAMS,

	/* Range for CONFIG_CHARGER_PROFILE_OVERRIDE params */
	CS_PARAM_CUSTOM_PROFILE_MIN = 0x10000,
	CS_PARAM_CUSTOM_PROFILE_MAX = 0x1ffff,

	/* Range for CONFIG_CHARGE_STATE_DEBUG params */
	CS_PARAM_DEBUG_MIN = 0x20000,
	CS_PARAM_DEBUG_CTL_MODE = 0x20000,
	CS_PARAM_DEBUG_MANUAL_MODE,
	CS_PARAM_DEBUG_SEEMS_DEAD,
	CS_PARAM_DEBUG_SEEMS_DISCONNECTED,
	CS_PARAM_DEBUG_BATT_REMOVED, /* Deprecated */
	CS_PARAM_DEBUG_MANUAL_CURRENT,
	CS_PARAM_DEBUG_MANUAL_VOLTAGE,
	CS_PARAM_DEBUG_MAX = 0x2ffff,

	/* Other custom param ranges go here... */
};

struct ec_params_charge_state {
	uint8_t cmd; /* enum charge_state_command */
	union {
		/* get_state has no args */

		struct __ec_todo_unpacked {
			uint32_t param; /* enum charge_state_param */
		} get_param;

		struct __ec_todo_unpacked {
			uint32_t param; /* param to set */
			uint32_t value; /* value to set */
		} set_param;
	} __ec_todo_packed;
	uint8_t chgnum; /* Version 1 supports chgnum */
} __ec_todo_packed;

struct ec_response_charge_state {
	union {
		struct __ec_align4 {
			int ac;
			int chg_voltage;
			int chg_current;
			int chg_input_current;
			int batt_state_of_charge;
		} get_state;

		struct __ec_align4 {
			uint32_t value;
		} get_param;

		/* set_param returns no args */
	};
} __ec_align4;

/*
 * Set maximum battery charging current.
 */
#define EC_CMD_CHARGE_CURRENT_LIMIT 0x00A1
#define EC_VER_CHARGE_CURRENT_LIMIT 1

struct ec_params_current_limit {
	uint32_t limit; /* in mA */
} __ec_align4;

struct ec_params_current_limit_v1 {
	uint32_t limit; /* in mA */
	/*
	 * Battery state of charge is the minimum charge percentage at which
	 * the battery charge current limit will apply.
	 * When not set, the limit will apply regardless of state of charge.
	 */
	uint8_t battery_soc; /* battery state of charge, 0-100 */
} __ec_align4;

/*
 * Set maximum external voltage / current.
 */
#define EC_CMD_EXTERNAL_POWER_LIMIT 0x00A2

/* Command v0 is used only on Spring and is obsolete + unsupported */
struct ec_params_external_power_limit_v1 {
	uint16_t current_lim; /* in mA, or EC_POWER_LIMIT_NONE to clear limit */
	uint16_t voltage_lim; /* in mV, or EC_POWER_LIMIT_NONE to clear limit */
} __ec_align2;

#define EC_POWER_LIMIT_NONE 0xffff

/*
 * Set maximum voltage & current of a dedicated charge port
 */
#define EC_CMD_OVERRIDE_DEDICATED_CHARGER_LIMIT 0x00A3

struct ec_params_dedicated_charger_limit {
	uint16_t current_lim; /* in mA */
	uint16_t voltage_lim; /* in mV */
} __ec_align2;

/*
 * Get and set charging splashscreen variables
 */
#define EC_CMD_CHARGESPLASH 0x00A5

enum ec_chargesplash_cmd {
	/* Get the current state variables */
	EC_CHARGESPLASH_GET_STATE = 0,

	/* Indicate initialization of the display loop */
	EC_CHARGESPLASH_DISPLAY_READY,

	/* Manually put the EC into the requested state */
	EC_CHARGESPLASH_REQUEST,

	/* Reset all state variables */
	EC_CHARGESPLASH_RESET,

	/* Manually trigger a lockout */
	EC_CHARGESPLASH_LOCKOUT,
};

struct __ec_align1 ec_params_chargesplash {
	/* enum ec_chargesplash_cmd */
	uint8_t cmd;
};

struct __ec_align1 ec_response_chargesplash {
	uint8_t requested;
	uint8_t display_initialized;
	uint8_t locked_out;
};

/*****************************************************************************/
/* Hibernate/Deep Sleep Commands */

/* Set the delay before going into hibernation. */
#define EC_CMD_HIBERNATION_DELAY 0x00A8

struct ec_params_hibernation_delay {
	/*
	 * Seconds to wait in G3 before hibernate.  Pass in 0 to read the
	 * current settings without changing them.
	 */
	uint32_t seconds;
} __ec_align4;

struct ec_response_hibernation_delay {
	/*
	 * The current time in seconds in which the system has been in the G3
	 * state.  This value is reset if the EC transitions out of G3.
	 */
	uint32_t time_g3;

	/*
	 * The current time remaining in seconds until the EC should hibernate.
	 * This value is also reset if the EC transitions out of G3.
	 */
	uint32_t time_remaining;

	/*
	 * The current time in seconds that the EC should wait in G3 before
	 * hibernating.
	 */
	uint32_t hibernate_delay;
} __ec_align4;

/* Inform the EC when entering a sleep state */
#define EC_CMD_HOST_SLEEP_EVENT 0x00A9

enum host_sleep_event {
	HOST_SLEEP_EVENT_S3_SUSPEND = 1,
	HOST_SLEEP_EVENT_S3_RESUME = 2,
	HOST_SLEEP_EVENT_S0IX_SUSPEND = 3,
	HOST_SLEEP_EVENT_S0IX_RESUME = 4,
	/* S3 suspend with additional enabled wake sources */
	HOST_SLEEP_EVENT_S3_WAKEABLE_SUSPEND = 5,
};

struct ec_params_host_sleep_event {
	uint8_t sleep_event;
} __ec_align1;

/*
 * Use a default timeout value (CONFIG_SLEEP_TIMEOUT_MS) for detecting sleep
 * transition failures
 */
#define EC_HOST_SLEEP_TIMEOUT_DEFAULT 0

/* Disable timeout detection for this sleep transition */
#define EC_HOST_SLEEP_TIMEOUT_INFINITE 0xFFFF

struct ec_params_host_sleep_event_v1 {
	/* The type of sleep being entered or exited. */
	uint8_t sleep_event;

	/* Padding */
	uint8_t reserved;
	union {
		/* Parameters that apply for suspend messages. */
		struct {
			/*
			 * The timeout in milliseconds between when this message
			 * is received and when the EC will declare sleep
			 * transition failure if the sleep signal is not
			 * asserted.
			 */
			uint16_t sleep_timeout_ms;
		} suspend_params;

		/* No parameters for non-suspend messages. */
	};
} __ec_align2;

/* A timeout occurred when this bit is set */
#define EC_HOST_RESUME_SLEEP_TIMEOUT 0x80000000

/*
 * The mask defining which bits correspond to the number of sleep transitions,
 * as well as the maximum number of suspend line transitions that will be
 * reported back to the host.
 */
#define EC_HOST_RESUME_SLEEP_TRANSITIONS_MASK 0x7FFFFFFF

struct ec_response_host_sleep_event_v1 {
	union {
		/* Response fields that apply for resume messages. */
		struct {
			/*
			 * The number of sleep power signal transitions that
			 * occurred since the suspend message. The high bit
			 * indicates a timeout occurred.
			 */
			uint32_t sleep_transitions;
		} resume_response;

		/* No response fields for non-resume messages. */
	};
} __ec_align4;

/*****************************************************************************/
/* Device events */
#define EC_CMD_DEVICE_EVENT 0x00AA

enum ec_device_event {
	EC_DEVICE_EVENT_TRACKPAD,
	EC_DEVICE_EVENT_DSP,
	EC_DEVICE_EVENT_WIFI,
	EC_DEVICE_EVENT_WLC,
};

enum ec_device_event_param {
	/* Get and clear pending device events */
	EC_DEVICE_EVENT_PARAM_GET_CURRENT_EVENTS,
	/* Get device event mask */
	EC_DEVICE_EVENT_PARAM_GET_ENABLED_EVENTS,
	/* Set device event mask */
	EC_DEVICE_EVENT_PARAM_SET_ENABLED_EVENTS,
};

#define EC_DEVICE_EVENT_MASK(event_code) BIT(event_code % 32)

struct ec_params_device_event {
	uint32_t event_mask;
	uint8_t param;
} __ec_align_size1;

struct ec_response_device_event {
	uint32_t event_mask;
} __ec_align4;

/*****************************************************************************/
/* Get s0ix counter */
#define EC_CMD_GET_S0IX_COUNTER 0x00AB

/* Flag use to reset the counter */
#define EC_S0IX_COUNTER_RESET 0x1

struct ec_params_s0ix_cnt {
	/* If EC_S0IX_COUNTER_RESET then reset otherwise get the counter */
	uint32_t flags;
} __ec_align4;

struct ec_response_s0ix_cnt {
	/* Value of the s0ix_counter */
	uint32_t s0ix_counter;
} __ec_align4;

/*****************************************************************************/
/* Ask the EC for sleep_signal_transitions without needing to send a
 * HOST_SLEEP_EVENT command, which this command is related to.
 * Note: EC_CMD_CONSOLE_PRINT has value 0x00AC, so skip over it.
 */
#define EC_CMD_HOST_SLEEP_SIGNAL_TRANSITIONS 0x00AD

struct ec_response_host_sleep_signal_transitions {
	uint32_t sleep_signal_transitions;
} __ec_align4;

/*****************************************************************************/
/* Smart battery pass-through */

/* Get / Set 16-bit smart battery registers  - OBSOLETE */
#define EC_CMD_SB_READ_WORD 0x00B0
#define EC_CMD_SB_WRITE_WORD 0x00B1

/* Get / Set string smart battery parameters
 * formatted as SMBUS "block". - OBSOLETE
 */
#define EC_CMD_SB_READ_BLOCK 0x00B2
#define EC_CMD_SB_WRITE_BLOCK 0x00B3

/*****************************************************************************/
/* Battery vendor parameters
 *
 * Get or set vendor-specific parameters in the battery. Implementations may
 * differ between boards or batteries. On a set operation, the response
 * contains the actual value set, which may be rounded or clipped from the
 * requested value.
 */

#define EC_CMD_BATTERY_VENDOR_PARAM 0x00B4

enum ec_battery_vendor_param_mode {
	BATTERY_VENDOR_PARAM_MODE_GET = 0,
	BATTERY_VENDOR_PARAM_MODE_SET,
};

struct ec_params_battery_vendor_param {
	uint32_t param;
	uint32_t value;
	uint8_t mode;
} __ec_align_size1;

struct ec_response_battery_vendor_param {
	uint32_t value;
} __ec_align4;

/*****************************************************************************/
/*
 * Smart Battery Firmware Update Command - OBSOLETE
 */
#define EC_CMD_SB_FW_UPDATE 0x00B5

/*
 * Entering Verified Boot Mode Command
 * Default mode is VBOOT_MODE_NORMAL if EC did not receive this command.
 * Valid Modes are: normal, developer, and recovery.
 *
 * EC no longer needs to know what mode vboot has entered,
 * so this command is deprecated.  (See chromium:1014379.)
 */
#define EC_CMD_ENTERING_MODE 0x00B6

struct ec_params_entering_mode {
	int vboot_mode;
} __ec_align4;

#define VBOOT_MODE_NORMAL 0
#define VBOOT_MODE_DEVELOPER 1
#define VBOOT_MODE_RECOVERY 2

/*****************************************************************************/
/*
 * I2C passthru protection command: Protects I2C tunnels against access on
 * certain addresses (board-specific).
 */
#define EC_CMD_I2C_PASSTHRU_PROTECT 0x00B7

enum ec_i2c_passthru_protect_subcmd {
	EC_CMD_I2C_PASSTHRU_PROTECT_STATUS = 0,
	EC_CMD_I2C_PASSTHRU_PROTECT_ENABLE = 1,
	EC_CMD_I2C_PASSTHRU_PROTECT_ENABLE_TCPCS = 2,
};

struct ec_params_i2c_passthru_protect {
	uint8_t subcmd;
	uint8_t port; /* I2C port number */
} __ec_align1;

struct ec_response_i2c_passthru_protect {
	uint8_t status; /* Status flags (0: unlocked, 1: locked) */
} __ec_align1;

/*****************************************************************************/
/*
 * HDMI CEC commands
 *
 * These commands are for sending and receiving message via HDMI CEC
 */

#define EC_CEC_MAX_PORTS 16

#define MAX_CEC_MSG_LEN 16

/*
 * Helper macros for packing/unpacking cec_events.
 * bits[27:0] : bitmask of events from enum mkbp_cec_event
 * bits[31:28]: port number
 */
#define EC_MKBP_EVENT_CEC_PACK(events, port) \
	(((events) & GENMASK(27, 0)) | (((port) & 0xf) << 28))
#define EC_MKBP_EVENT_CEC_GET_EVENTS(event) ((event) & GENMASK(27, 0))
#define EC_MKBP_EVENT_CEC_GET_PORT(event) (((event) >> 28) & 0xf)

/* CEC message from the AP to be written on the CEC bus */
#define EC_CMD_CEC_WRITE_MSG 0x00B8

/**
 * struct ec_params_cec_write - Message to write to the CEC bus
 * @msg: message content to write to the CEC bus
 */
struct ec_params_cec_write {
	uint8_t msg[MAX_CEC_MSG_LEN];
} __ec_align1;

/**
 * struct ec_params_cec_write_v1 - Message to write to the CEC bus
 * @port: CEC port to write the message on
 * @msg_len: length of msg in bytes
 * @msg: message content to write to the CEC bus
 */
struct ec_params_cec_write_v1 {
	uint8_t port;
	uint8_t msg_len;
	uint8_t msg[MAX_CEC_MSG_LEN];
} __ec_align1;

/* CEC message read from a CEC bus reported back to the AP */
#define EC_CMD_CEC_READ_MSG 0x00B9

/**
 * struct ec_params_cec_read - Read a message from the CEC bus
 * @port: CEC port to read a message on
 */
struct ec_params_cec_read {
	uint8_t port;
} __ec_align1;

/**
 * struct ec_response_cec_read - Message read from the CEC bus
 * @msg_len: length of msg in bytes
 * @msg: message content read from the CEC bus
 */
struct ec_response_cec_read {
	uint8_t msg_len;
	uint8_t msg[MAX_CEC_MSG_LEN];
} __ec_align1;

/* Set various CEC parameters */
#define EC_CMD_CEC_SET 0x00BA

/**
 * struct ec_params_cec_set - CEC parameters set
 * @cmd: parameter type, can be CEC_CMD_ENABLE or CEC_CMD_LOGICAL_ADDRESS
 * @port: CEC port to set the parameter on
 * @val: in case cmd is CEC_CMD_ENABLE, this field can be 0 to disable CEC
 *	or 1 to enable CEC functionality, in case cmd is
 *	CEC_CMD_LOGICAL_ADDRESS, this field encodes the requested logical
 *	address between 0 and 15 or 0xff to unregister
 */
struct ec_params_cec_set {
	uint8_t cmd : 4; /* enum cec_command */
	uint8_t port : 4;
	uint8_t val;
} __ec_align1;

/* Read various CEC parameters */
#define EC_CMD_CEC_GET 0x00BB

/**
 * struct ec_params_cec_get - CEC parameters get
 * @cmd: parameter type, can be CEC_CMD_ENABLE or CEC_CMD_LOGICAL_ADDRESS
 * @port: CEC port to get the parameter on
 */
struct ec_params_cec_get {
	uint8_t cmd : 4; /* enum cec_command */
	uint8_t port : 4;
} __ec_align1;

/**
 * struct ec_response_cec_get - CEC parameters get response
 * @val: in case cmd was CEC_CMD_ENABLE, this field will 0 if CEC is
 *	disabled or 1 if CEC functionality is enabled,
 *	in case cmd was CEC_CMD_LOGICAL_ADDRESS, this will encode the
 *	configured logical address between 0 and 15 or 0xff if unregistered
 */
struct ec_response_cec_get {
	uint8_t val;
} __ec_align1;

/* Get the number of CEC ports */
#define EC_CMD_CEC_PORT_COUNT 0x00C1

/**
 * struct ec_response_cec_port_count - CEC port count response
 * @port_count: number of CEC ports
 */
struct ec_response_cec_port_count {
	uint8_t port_count;
} __ec_align1;

/* CEC parameters command */
enum cec_command {
	/* CEC reading, writing and events enable */
	CEC_CMD_ENABLE,
	/* CEC logical address  */
	CEC_CMD_LOGICAL_ADDRESS,
};

/* Events from CEC to AP */
enum mkbp_cec_event {
	/* Outgoing message was acknowledged by a follower */
	EC_MKBP_CEC_SEND_OK = BIT(0),
	/* Outgoing message was not acknowledged */
	EC_MKBP_CEC_SEND_FAILED = BIT(1),
	/* Incoming message can be read out by AP */
	EC_MKBP_CEC_HAVE_DATA = BIT(2),
};

/*****************************************************************************/

/* Commands for audio codec. */
#define EC_CMD_EC_CODEC 0x00BC

enum ec_codec_subcmd {
	EC_CODEC_GET_CAPABILITIES = 0x0,
	EC_CODEC_GET_SHM_ADDR = 0x1,
	EC_CODEC_SET_SHM_ADDR = 0x2,
	EC_CODEC_SUBCMD_COUNT,
};

enum ec_codec_cap {
	EC_CODEC_CAP_WOV_AUDIO_SHM = 0,
	EC_CODEC_CAP_WOV_LANG_SHM = 1,
	EC_CODEC_CAP_LAST = 32,
};

enum ec_codec_shm_id {
	EC_CODEC_SHM_ID_WOV_AUDIO = 0x0,
	EC_CODEC_SHM_ID_WOV_LANG = 0x1,
	EC_CODEC_SHM_ID_LAST,
};

enum ec_codec_shm_type {
	EC_CODEC_SHM_TYPE_EC_RAM = 0x0,
	EC_CODEC_SHM_TYPE_SYSTEM_RAM = 0x1,
};

struct __ec_align1 ec_param_ec_codec_get_shm_addr {
	uint8_t shm_id;
	uint8_t reserved[3];
};

struct __ec_align4 ec_param_ec_codec_set_shm_addr {
	uint64_t phys_addr;
	uint32_t len;
	uint8_t shm_id;
	uint8_t reserved[3];
};

struct __ec_align4 ec_param_ec_codec {
	uint8_t cmd; /* enum ec_codec_subcmd */
	uint8_t reserved[3];

	union {
		struct ec_param_ec_codec_get_shm_addr get_shm_addr_param;
		struct ec_param_ec_codec_set_shm_addr set_shm_addr_param;
	};
};

struct __ec_align4 ec_response_ec_codec_get_capabilities {
	uint32_t capabilities;
};

struct __ec_align4 ec_response_ec_codec_get_shm_addr {
	uint64_t phys_addr;
	uint32_t len;
	uint8_t type;
	uint8_t reserved[3];
};

/*****************************************************************************/

/* Commands for DMIC on audio codec. */
#define EC_CMD_EC_CODEC_DMIC 0x00BD

enum ec_codec_dmic_subcmd {
	EC_CODEC_DMIC_GET_MAX_GAIN = 0x0,
	EC_CODEC_DMIC_SET_GAIN_IDX = 0x1,
	EC_CODEC_DMIC_GET_GAIN_IDX = 0x2,
	EC_CODEC_DMIC_SUBCMD_COUNT,
};

enum ec_codec_dmic_channel {
	EC_CODEC_DMIC_CHANNEL_0 = 0x0,
	EC_CODEC_DMIC_CHANNEL_1 = 0x1,
	EC_CODEC_DMIC_CHANNEL_2 = 0x2,
	EC_CODEC_DMIC_CHANNEL_3 = 0x3,
	EC_CODEC_DMIC_CHANNEL_4 = 0x4,
	EC_CODEC_DMIC_CHANNEL_5 = 0x5,
	EC_CODEC_DMIC_CHANNEL_6 = 0x6,
	EC_CODEC_DMIC_CHANNEL_7 = 0x7,
	EC_CODEC_DMIC_CHANNEL_COUNT,
};

struct __ec_align1 ec_param_ec_codec_dmic_set_gain_idx {
	uint8_t channel; /* enum ec_codec_dmic_channel */
	uint8_t gain;
	uint8_t reserved[2];
};

struct __ec_align1 ec_param_ec_codec_dmic_get_gain_idx {
	uint8_t channel; /* enum ec_codec_dmic_channel */
	uint8_t reserved[3];
};

struct __ec_align4 ec_param_ec_codec_dmic {
	uint8_t cmd; /* enum ec_codec_dmic_subcmd */
	uint8_t reserved[3];

	union {
		struct ec_param_ec_codec_dmic_set_gain_idx set_gain_idx_param;
		struct ec_param_ec_codec_dmic_get_gain_idx get_gain_idx_param;
	};
};

struct __ec_align1 ec_response_ec_codec_dmic_get_max_gain {
	uint8_t max_gain;
};

struct __ec_align1 ec_response_ec_codec_dmic_get_gain_idx {
	uint8_t gain;
};

/*****************************************************************************/

/* Commands for I2S RX on audio codec. */

#define EC_CMD_EC_CODEC_I2S_RX 0x00BE

enum ec_codec_i2s_rx_subcmd {
	EC_CODEC_I2S_RX_ENABLE = 0x0,
	EC_CODEC_I2S_RX_DISABLE = 0x1,
	EC_CODEC_I2S_RX_SET_SAMPLE_DEPTH = 0x2,
	EC_CODEC_I2S_RX_SET_DAIFMT = 0x3,
	EC_CODEC_I2S_RX_SET_BCLK = 0x4,
	EC_CODEC_I2S_RX_RESET = 0x5,
	EC_CODEC_I2S_RX_SUBCMD_COUNT,
};

enum ec_codec_i2s_rx_sample_depth {
	EC_CODEC_I2S_RX_SAMPLE_DEPTH_16 = 0x0,
	EC_CODEC_I2S_RX_SAMPLE_DEPTH_24 = 0x1,
	EC_CODEC_I2S_RX_SAMPLE_DEPTH_COUNT,
};

enum ec_codec_i2s_rx_daifmt {
	EC_CODEC_I2S_RX_DAIFMT_I2S = 0x0,
	EC_CODEC_I2S_RX_DAIFMT_RIGHT_J = 0x1,
	EC_CODEC_I2S_RX_DAIFMT_LEFT_J = 0x2,
	EC_CODEC_I2S_RX_DAIFMT_COUNT,
};

struct __ec_align1 ec_param_ec_codec_i2s_rx_set_sample_depth {
	uint8_t depth;
	uint8_t reserved[3];
};

struct __ec_align1 ec_param_ec_codec_i2s_rx_set_gain {
	uint8_t left;
	uint8_t right;
	uint8_t reserved[2];
};

struct __ec_align1 ec_param_ec_codec_i2s_rx_set_daifmt {
	uint8_t daifmt;
	uint8_t reserved[3];
};

struct __ec_align4 ec_param_ec_codec_i2s_rx_set_bclk {
	uint32_t bclk;
};

struct __ec_align4 ec_param_ec_codec_i2s_rx {
	uint8_t cmd; /* enum ec_codec_i2s_rx_subcmd */
	uint8_t reserved[3];

	union {
		struct ec_param_ec_codec_i2s_rx_set_sample_depth
			set_sample_depth_param;
		struct ec_param_ec_codec_i2s_rx_set_daifmt set_daifmt_param;
		struct ec_param_ec_codec_i2s_rx_set_bclk set_bclk_param;
	};
};

/*****************************************************************************/
/* Commands for WoV on audio codec. */

#define EC_CMD_EC_CODEC_WOV 0x00BF

enum ec_codec_wov_subcmd {
	EC_CODEC_WOV_SET_LANG = 0x0,
	EC_CODEC_WOV_SET_LANG_SHM = 0x1,
	EC_CODEC_WOV_GET_LANG = 0x2,
	EC_CODEC_WOV_ENABLE = 0x3,
	EC_CODEC_WOV_DISABLE = 0x4,
	EC_CODEC_WOV_READ_AUDIO = 0x5,
	EC_CODEC_WOV_READ_AUDIO_SHM = 0x6,
	EC_CODEC_WOV_SUBCMD_COUNT,
};

/*
 * @hash is SHA256 of the whole language model.
 * @total_len indicates the length of whole language model.
 * @offset is the cursor from the beginning of the model.
 * @buf is the packet buffer.
 * @len denotes how many bytes in the buf.
 */
struct __ec_align4 ec_param_ec_codec_wov_set_lang {
	uint8_t hash[32];
	uint32_t total_len;
	uint32_t offset;
	uint8_t buf[128];
	uint32_t len;
};

struct __ec_align4 ec_param_ec_codec_wov_set_lang_shm {
	uint8_t hash[32];
	uint32_t total_len;
};

struct __ec_align4 ec_param_ec_codec_wov {
	uint8_t cmd; /* enum ec_codec_wov_subcmd */
	uint8_t reserved[3];

	union {
		struct ec_param_ec_codec_wov_set_lang set_lang_param;
		struct ec_param_ec_codec_wov_set_lang_shm set_lang_shm_param;
	};
};

struct __ec_align4 ec_response_ec_codec_wov_get_lang {
	uint8_t hash[32];
};

struct __ec_align4 ec_response_ec_codec_wov_read_audio {
	uint8_t buf[128];
	uint32_t len;
};

struct __ec_align4 ec_response_ec_codec_wov_read_audio_shm {
	uint32_t offset;
	uint32_t len;
};

/*****************************************************************************/
/* Commands for PoE PSE controller */

#define EC_CMD_PSE 0x00C0

enum ec_pse_subcmd {
	EC_PSE_STATUS = 0x0,
	EC_PSE_ENABLE = 0x1,
	EC_PSE_DISABLE = 0x2,
	EC_PSE_SUBCMD_COUNT,
};

struct __ec_align1 ec_params_pse {
	uint8_t cmd; /* enum ec_pse_subcmd */
	uint8_t port; /* PSE port */
};

enum ec_pse_status {
	EC_PSE_STATUS_DISABLED = 0x0,
	EC_PSE_STATUS_ENABLED = 0x1,
	EC_PSE_STATUS_POWERED = 0x2,
};

struct __ec_align1 ec_response_pse_status {
	uint8_t status; /* enum ec_pse_status */
};

/*****************************************************************************/
/* System commands */

/*
 * TODO(crosbug.com/p/23747): This is a confusing name, since it doesn't
 * necessarily reboot the EC.  Rename to "image" or something similar?
 */
#define EC_CMD_REBOOT_EC 0x00D2

/* Command */
enum ec_reboot_cmd {
	EC_REBOOT_CANCEL = 0, /* Cancel a pending reboot */
	EC_REBOOT_JUMP_RO = 1, /* Jump to RO without rebooting */
	EC_REBOOT_JUMP_RW = 2, /* Jump to active RW without rebooting */
	/* (command 3 was jump to RW-B) */
	EC_REBOOT_COLD = 4, /* Cold-reboot */
	EC_REBOOT_DISABLE_JUMP = 5, /* Disable jump until next reboot */
	EC_REBOOT_HIBERNATE = 6, /* Hibernate EC */
	/*
	 * DEPRECATED: Hibernate EC and clears AP_IDLE flag.
	 * Use EC_REBOOT_HIBERNATE and EC_REBOOT_FLAG_CLEAR_AP_IDLE, instead.
	 */
	EC_REBOOT_HIBERNATE_CLEAR_AP_OFF = 7,
	EC_REBOOT_COLD_AP_OFF = 8, /* Cold-reboot and don't boot AP */
	EC_REBOOT_NO_OP = 9, /* Do nothing but apply the flags. */
};

/* Flags for ec_params_reboot_ec.reboot_flags */
#define EC_REBOOT_FLAG_IMMEDIATE 0 /* Trigger Cold Reset */
#define EC_REBOOT_FLAG_RESERVED0 BIT(0) /* Was recovery request */
#define EC_REBOOT_FLAG_ON_AP_SHUTDOWN BIT(1) /* Reboot after AP shutdown */
#define EC_REBOOT_FLAG_SWITCH_RW_SLOT BIT(2) /* Switch RW slot */
#define EC_REBOOT_FLAG_CLEAR_AP_IDLE BIT(3) /* Clear AP_IDLE flag */

struct ec_params_reboot_ec {
	uint8_t cmd; /* enum ec_reboot_cmd */
	uint8_t flags; /* See EC_REBOOT_FLAG_* */
} __ec_align1;

/*
 * Get information on last EC panic.
 *
 * Returns variable-length platform-dependent panic information.  See panic.h
 * for details.
 */
#define EC_CMD_GET_PANIC_INFO 0x00D3

struct ec_params_get_panic_info_v1 {
	/* Do not modify PANIC_DATA_FLAG_OLD_HOSTCMD when reading panic info */
	uint8_t preserve_old_hostcmd_flag;
} __ec_align1;

struct ec_params_get_panic_info_v2 {
	/* Do not modify PANIC_DATA_FLAG_OLD_HOSTCMD when reading panic info */
	uint8_t preserve_old_hostcmd_flag;

	/* Read panic_data struct from this offset.
	 * Signal end of data with empty success.
	 */
	uint16_t read_offset;
} __ec_align1;

/*****************************************************************************/
/*
 * Special commands
 *
 * These do not follow the normal rules for commands.  See each command for
 * details.
 */

/*
 * Reboot NOW
 *
 * This command will work even when the EC LPC interface is busy, because the
 * reboot command is processed at interrupt level.  Note that when the EC
 * reboots, the host will reboot too, so there is no response to this command.
 *
 * Use EC_CMD_REBOOT_EC to reboot the EC more politely.
 */
#define EC_CMD_REBOOT 0x00D1 /* Think "die" */

/*
 * Resend last response (not supported on LPC).
 *
 * Returns EC_RES_UNAVAILABLE if there is no response available - for example,
 * there was no previous command, or the previous command's response was too
 * big to save.
 */
#define EC_CMD_RESEND_RESPONSE 0x00DB

/*
 * This header byte on a command indicate version 0. Any header byte less
 * than this means that we are talking to an old EC which doesn't support
 * versioning. In that case, we assume version 0.
 *
 * Header bytes greater than this indicate a later version. For example,
 * EC_CMD_VERSION0 + 1 means we are using version 1.
 *
 * The old EC interface must not use commands 0xdc or higher.
 */
#define EC_CMD_VERSION0 0x00DC

/*
 * Memory Dump Commands
 *
 * Since the HOSTCMD response size is limited, depending on the
 * protocol, retrieving a memory dump is split into 3 commands.
 *
 * 1. EC_CMD_MEMORY_DUMP_GET_METADATA returns the number of memory dump entries,
 *    and the total dump size.
 * 2. EC_CMD_MEMORY_DUMP_GET_ENTRY_INFO returns the address and size for a given
 *    memory dump entry index.
 * 3. EC_CMD_MEMORY_DUMP_READ_MEMORY returns the actual memory at a given
 *    address. The address and size must be within the bounds of the given
 *    memory dump entry index. Each response is limited to the max response size
 *    of the host protocol, so this may need to be called repeatedly to retrieve
 *    the entire memory dump entry.
 *
 * Memory entries may overlap and may be out of order.
 * The host should check for overlaps to optimize transfer rate.
 */
#define EC_CMD_MEMORY_DUMP_GET_METADATA 0x00DD
struct ec_response_memory_dump_get_metadata {
	uint16_t memory_dump_entry_count;
	uint32_t memory_dump_total_size;
} __ec_align4;

#define EC_CMD_MEMORY_DUMP_GET_ENTRY_INFO 0x00DE
struct ec_params_memory_dump_get_entry_info {
	uint16_t memory_dump_entry_index;
} __ec_align4;

struct ec_response_memory_dump_get_entry_info {
	uint32_t address;
	uint32_t size;
} __ec_align4;

#define EC_CMD_MEMORY_DUMP_READ_MEMORY 0x00DF

struct ec_params_memory_dump_read_memory {
	uint16_t memory_dump_entry_index;
	uint32_t address;
	uint32_t size;
} __ec_align4;

#define EC_CMD_PANIC_LOG_INFO 0x00E0

/*
 * Parameters for configuring the panic log.
 * Freeze and unfreeze are mutually exclusive.
 */
struct ec_params_panic_log_info {
	/* Reset panic log */
	uint8_t reset;
	/* Freeze panic log */
	uint8_t freeze;
	/* Unfreeze panic log */
	uint8_t unfreeze;
} __ec_align1;

/*
 * Returns the panic log info before applying the configuration
 * in ec_params_panic_log_info.
 */
struct ec_response_panic_log_info {
	uint32_t version;
	uint32_t capacity;
	uint32_t length;
	uint8_t valid;
	uint8_t frozen;
} __ec_align4;

#define EC_CMD_PANIC_LOG_READ 0x00E1

/*
 * Read from panic log at given byte offset. Will read up to the end of the
 * panic log or response max. Use EC_CMD_PANIC_LOG_INFO command to freeze
 * the log and get the length before reading.
 */
struct ec_params_panic_log_read {
	uint32_t offset;
} __ec_align4;

/*
 * EC_CMD_MEMORY_DUMP_READ_MEMORY response buffer is written directly into
 * host_cmd_handler_args.response and host_cmd_handler_args.response_size.
 */

/*
 * Enter bootloader mode
 *
 * This command requests EC to enter bootloader mode.
 */
#define EC_CMD_ENTER_BOOTLOADER 0x00E2

struct ec_params_enter_bootloader {
	/* Mode to enter bootloader. Chip specific value. Can be unused. */
	uint8_t mode;
} __ec_align1;

#define EC_CMD_HOSTCMD_WATCHDOG_INFO 0x00E3

struct ec_params_hostcmd_watchdog_info {
	uint8_t reset_stats;
} __ec_align1;

struct ec_response_hostcmd_watchdog_info {
	/* Static watchdog info */
	int32_t watchdog_period_ms;
	int32_t watchdog_warning_period_ms;
	int32_t watchdog_reload_period_nominal_ms;
	/* Dynamic watchdog stats */
	int32_t watchdog_reload_period_max_ms;
	int64_t watchdog_reload_period_max_ts_ms;
	uint32_t watchdog_reload_count;
	int64_t watchdog_stats_elapsed_ms;
} __ec_align4;

/*****************************************************************************/
/*
 * PD commands
 *
 * These commands are for PD MCU communication.
 */

/* EC to PD MCU exchange status command */
#define EC_CMD_PD_EXCHANGE_STATUS 0x0100
#define EC_VER_PD_EXCHANGE_STATUS 2

enum pd_charge_state {
	/* Don't change charge state */
	PD_CHARGE_NO_CHANGE = 0,

	/* No charging allowed */
	PD_CHARGE_NONE,

	/* 5V charging only */
	PD_CHARGE_5V,

	/* Charge at max voltage */
	PD_CHARGE_MAX,
};

/* Status of EC being sent to PD */
#define EC_STATUS_HIBERNATING BIT(0)

struct ec_params_pd_status {
	/* EC status */
	uint8_t status;

	/* battery state of charge */
	int8_t batt_soc;

	/* charging state (from enum pd_charge_state) */
	uint8_t charge_state;
} __ec_align1;

/* Status of PD being sent back to EC */
#define PD_STATUS_HOST_EVENT BIT(0) /* Forward host event to AP */
#define PD_STATUS_IN_RW BIT(1) /* Running RW image */
#define PD_STATUS_JUMPED_TO_IMAGE BIT(2) /* Current image was jumped to */
#define PD_STATUS_TCPC_ALERT_0 BIT(3) /* Alert active in port 0 TCPC */
#define PD_STATUS_TCPC_ALERT_1 BIT(4) /* Alert active in port 1 TCPC */
#define PD_STATUS_TCPC_ALERT_2 BIT(5) /* Alert active in port 2 TCPC */
#define PD_STATUS_TCPC_ALERT_3 BIT(6) /* Alert active in port 3 TCPC */
#define PD_STATUS_EC_INT_ACTIVE \
	(PD_STATUS_TCPC_ALERT_0 | PD_STATUS_TCPC_ALERT_1 | PD_STATUS_HOST_EVENT)
struct ec_response_pd_status {
	/* input current limit */
	uint32_t curr_lim_ma;

	/* PD MCU status */
	uint16_t status;

	/* active charging port */
	int8_t active_charge_port;
} __ec_align_size1;

/* AP to PD MCU host event status command, cleared on read */
#define EC_CMD_PD_HOST_EVENT_STATUS 0x0104

/* PD MCU host event status bits */
#define PD_EVENT_UPDATE_DEVICE BIT(0)
#define PD_EVENT_POWER_CHANGE BIT(1)
#define PD_EVENT_IDENTITY_RECEIVED BIT(2)
#define PD_EVENT_DATA_SWAP BIT(3)
#define PD_EVENT_TYPEC BIT(4)
#define PD_EVENT_PPM BIT(5)
#define PD_EVENT_INIT BIT(6)

struct ec_response_host_event_status {
	uint32_t status; /* PD MCU host event status */
} __ec_align4;

/*
 * Set USB type-C port role and muxes
 *
 * Deprecated in favor of TYPEC_STATUS and TYPEC_CONTROL commands.
 *
 * TODO(b/169771803): TCPMv2: Remove EC_CMD_USB_PD_CONTROL
 */
#define EC_CMD_USB_PD_CONTROL 0x0101

enum usb_pd_control_role {
	USB_PD_CTRL_ROLE_NO_CHANGE = 0,
	USB_PD_CTRL_ROLE_TOGGLE_ON = 1, /* == AUTO */
	USB_PD_CTRL_ROLE_TOGGLE_OFF = 2,
	USB_PD_CTRL_ROLE_FORCE_SINK = 3,
	USB_PD_CTRL_ROLE_FORCE_SOURCE = 4,
	USB_PD_CTRL_ROLE_FREEZE = 5,
	USB_PD_CTRL_ROLE_COUNT,
};

enum usb_pd_control_mux {
	USB_PD_CTRL_MUX_NO_CHANGE = 0,
	USB_PD_CTRL_MUX_NONE = 1,
	USB_PD_CTRL_MUX_USB = 2,
	USB_PD_CTRL_MUX_DP = 3,
	USB_PD_CTRL_MUX_DOCK = 4,
	USB_PD_CTRL_MUX_AUTO = 5,
	USB_PD_CTRL_MUX_COUNT,
};

enum usb_pd_control_swap {
	USB_PD_CTRL_SWAP_NONE = 0,
	USB_PD_CTRL_SWAP_DATA = 1,
	USB_PD_CTRL_SWAP_POWER = 2,
	USB_PD_CTRL_SWAP_VCONN = 3,
	USB_PD_CTRL_SWAP_COUNT,
};

struct ec_params_usb_pd_control {
	uint8_t port;
	uint8_t role;
	uint8_t mux;
	uint8_t swap;
} __ec_align1;

#define PD_CTRL_RESP_ENABLED_COMMS BIT(0) /* Communication enabled */
#define PD_CTRL_RESP_ENABLED_CONNECTED BIT(1) /* Device connected */
#define PD_CTRL_RESP_ENABLED_PD_CAPABLE BIT(2) /* Partner is PD capable */

#define PD_CTRL_RESP_ROLE_POWER BIT(0) /* 0=SNK/1=SRC */
#define PD_CTRL_RESP_ROLE_DATA BIT(1) /* 0=UFP/1=DFP */
#define PD_CTRL_RESP_ROLE_VCONN BIT(2) /* Vconn status */
#define PD_CTRL_RESP_ROLE_DR_POWER BIT(3) /* Partner is dualrole power */
#define PD_CTRL_RESP_ROLE_DR_DATA BIT(4) /* Partner is dualrole data */
#define PD_CTRL_RESP_ROLE_USB_COMM BIT(5) /* Partner USB comm capable */
/* Partner unconstrained power */
#define PD_CTRL_RESP_ROLE_UNCONSTRAINED BIT(6)

struct ec_response_usb_pd_control {
	uint8_t enabled;
	uint8_t role;
	uint8_t polarity;
	uint8_t state;
} __ec_align1;

struct ec_response_usb_pd_control_v1 {
	uint8_t enabled;
	uint8_t role;
	uint8_t polarity;
	char state[32];
} __ec_align1;

/* Possible port partner connections based on CC line states */
enum pd_cc_states {
	PD_CC_NONE = 0, /* No port partner attached */

	/* From DFP perspective */
	PD_CC_UFP_NONE = 1, /* No UFP accessory connected */
	PD_CC_UFP_AUDIO_ACC = 2, /* UFP Audio accessory connected */
	PD_CC_UFP_DEBUG_ACC = 3, /* UFP Debug accessory connected */
	PD_CC_UFP_ATTACHED = 4, /* Plain UFP attached */

	/* From UFP perspective */
	PD_CC_DFP_ATTACHED = 5, /* Plain DFP attached */
	PD_CC_DFP_DEBUG_ACC = 6, /* DFP debug accessory connected */
};

/* Active/Passive Cable */
#define USB_PD_CTRL_ACTIVE_CABLE BIT(0)
/* Optical/Non-optical cable */
#define USB_PD_CTRL_OPTICAL_CABLE BIT(1)
/* 3rd Gen TBT device (or AMA)/2nd gen tbt Adapter */
#define USB_PD_CTRL_TBT_LEGACY_ADAPTER BIT(2)
/* Active Link Uni-Direction */
#define USB_PD_CTRL_ACTIVE_LINK_UNIDIR BIT(3)
/* Retimer/Redriver cable */
#define USB_PD_CTRL_RETIMER_CABLE BIT(4)

struct ec_response_usb_pd_control_v2 {
	uint8_t enabled;
	uint8_t role;
	uint8_t polarity;
	char state[32];
	uint8_t cc_state; /* enum pd_cc_states representing cc state */
	uint8_t dp_mode; /* Current DP pin mode (MODE_DP_PIN_[A-E]) */
	uint8_t reserved; /* Reserved for future use */
	uint8_t control_flags; /* USB_PD_CTRL_*flags */
	uint8_t cable_speed; /* TBT_SS_* cable speed */
	uint8_t cable_gen; /* TBT_GEN3_* cable rounded support */
} __ec_align1;

#define EC_CMD_USB_PD_PORTS 0x0102

/* Maximum number of PD ports on a device, num_ports will be <= this */
#define EC_USB_PD_MAX_PORTS 8

struct ec_response_usb_pd_ports {
	uint8_t num_ports;
} __ec_align1;

#define EC_CMD_USB_PD_POWER_INFO 0x0103

#define PD_POWER_CHARGING_PORT 0xff
struct ec_params_usb_pd_power_info {
	uint8_t port;
} __ec_align1;

enum usb_chg_type {
	USB_CHG_TYPE_NONE,
	USB_CHG_TYPE_PD,
	USB_CHG_TYPE_C,
	USB_CHG_TYPE_PROPRIETARY,
	USB_CHG_TYPE_BC12_DCP,
	USB_CHG_TYPE_BC12_CDP,
	USB_CHG_TYPE_BC12_SDP,
	USB_CHG_TYPE_OTHER,
	USB_CHG_TYPE_VBUS,
	USB_CHG_TYPE_UNKNOWN,
	USB_CHG_TYPE_DEDICATED,
};
enum usb_power_roles {
	USB_PD_PORT_POWER_DISCONNECTED,
	USB_PD_PORT_POWER_SOURCE,
	USB_PD_PORT_POWER_SINK,
	USB_PD_PORT_POWER_SINK_NOT_CHARGING,
};

struct usb_chg_measures {
	uint16_t voltage_max;
	uint16_t voltage_now;
	uint16_t current_max;
	uint16_t current_lim;
} __ec_align2;

struct ec_response_usb_pd_power_info {
	uint8_t role;
	uint8_t type;
	uint8_t dualrole;
	uint8_t reserved1;
	struct usb_chg_measures meas;
	uint32_t max_power;
} __ec_align4;

/*
 * This command will return the number of USB PD charge port + the number
 * of dedicated port present.
 * EC_CMD_USB_PD_PORTS does NOT include the dedicated ports
 */
#define EC_CMD_CHARGE_PORT_COUNT 0x0105
struct ec_response_charge_port_count {
	uint8_t port_count;
} __ec_align1;

/*
 * This command enable/disable dynamic PDO selection.
 */
#define EC_CMD_USB_PD_DPS_CONTROL 0x0106

struct ec_params_usb_pd_dps_control {
	uint8_t enable;
} __ec_align1;

/*
 * This command return the status of dynamic PDO selection.
 */
#define EC_CMD_USB_PD_DPS_STATUS 0x0107

struct ec_response_usb_pd_dps_status {
	int32_t is_enabled;
	int32_t port;
	int32_t requested_voltage;
	int32_t requested_current;
	int32_t input_power;
	int32_t input_voltage;
	int32_t input_current;
	int32_t efficient_voltage;
	int32_t battery_voltage;
	int32_t max_voltage;
} __ec_align4;

/* Write USB-PD device FW */
#define EC_CMD_USB_PD_FW_UPDATE 0x0110

enum usb_pd_fw_update_cmds {
	USB_PD_FW_REBOOT,
	USB_PD_FW_FLASH_ERASE,
	USB_PD_FW_FLASH_WRITE,
	USB_PD_FW_ERASE_SIG,
};

struct ec_params_usb_pd_fw_update {
	uint16_t dev_id;
	uint8_t cmd;
	uint8_t port;

	/* Size to write in bytes */
	uint32_t size;

	/* Followed by data to write */
} __ec_align4;

/* Write USB-PD Accessory RW_HASH table entry */
#define EC_CMD_USB_PD_RW_HASH_ENTRY 0x0111
/* RW hash is first 20 bytes of SHA-256 of RW section */
#define PD_RW_HASH_SIZE 20
struct ec_params_usb_pd_rw_hash_entry {
	uint16_t dev_id;
	uint8_t dev_rw_hash[PD_RW_HASH_SIZE];

	/*
	 * Reserved for alignment of current_image
	 * TODO(rspangler) but it's not aligned!
	 * Should have been reserved[2].
	 */
	uint8_t reserved;

	/* One of ec_image */
	uint32_t current_image;
} __ec_align1;

/* Read USB-PD Accessory info */
#define EC_CMD_USB_PD_DEV_INFO 0x0112

struct ec_params_usb_pd_info_request {
	uint8_t port;
} __ec_align1;

/* Read USB-PD Device discovery info */
#define EC_CMD_USB_PD_DISCOVERY 0x0113
struct ec_params_usb_pd_discovery_entry {
	uint16_t vid; /* USB-IF VID */
	uint16_t pid; /* USB-IF PID */
	uint8_t ptype; /* product type (hub,periph,cable,ama) */
} __ec_align_size1;

/* Override default charge behavior */
#define EC_CMD_PD_CHARGE_PORT_OVERRIDE 0x0114

/* Negative port parameters have special meaning */
enum usb_pd_override_ports {
	/*
	 * DONT_CHARGE is for all ports. Thus it's persistent across plug-in
	 * or plug-out.
	 */
	OVERRIDE_DONT_CHARGE = -2,
	OVERRIDE_OFF = -1,
	/* [0, CONFIG_USB_PD_PORT_MAX_COUNT): Port# */
};

struct ec_params_charge_port_override {
	int16_t override_port; /* Override port# */
} __ec_align2;

/*
 * Read (and delete) one entry of PD event log.
 * TODO(crbug.com/751742): Make this host command more generic to accommodate
 * future non-PD logs that use the same internal EC event_log.
 */
#define EC_CMD_PD_GET_LOG_ENTRY 0x0115

struct ec_response_pd_log {
	uint32_t timestamp; /* relative timestamp in milliseconds */
	uint8_t type; /* event type : see PD_EVENT_xx below */
	uint8_t size_port; /* [7:5] port number [4:0] payload size in bytes */
	uint16_t data; /* type-defined data payload */
	/* optional additional data payload: 0..16 bytes */
	uint8_t payload[FLEXIBLE_ARRAY_MEMBER_SIZE];
} __ec_align4;

/* The timestamp is the microsecond counter shifted to get about a ms. */
#define PD_LOG_TIMESTAMP_SHIFT 10 /* 1 LSB = 1024us */

#define PD_LOG_SIZE_MASK 0x1f
#define PD_LOG_PORT_MASK 0xe0
#define PD_LOG_PORT_SHIFT 5
#define PD_LOG_PORT_SIZE(port, size) \
	(((port) << PD_LOG_PORT_SHIFT) | ((size) & PD_LOG_SIZE_MASK))
#define PD_LOG_PORT(size_port) ((size_port) >> PD_LOG_PORT_SHIFT)
#define PD_LOG_SIZE(size_port) ((size_port) & PD_LOG_SIZE_MASK)

/* PD event log : entry types */
/* PD MCU events */
#define PD_EVENT_MCU_BASE 0x00
#define PD_EVENT_MCU_CHARGE (PD_EVENT_MCU_BASE + 0)
#define PD_EVENT_MCU_CONNECT (PD_EVENT_MCU_BASE + 1)
/* Reserved for custom board event */
#define PD_EVENT_MCU_BOARD_CUSTOM (PD_EVENT_MCU_BASE + 2)
/* PD generic accessory events */
#define PD_EVENT_ACC_BASE 0x20
#define PD_EVENT_ACC_RW_FAIL (PD_EVENT_ACC_BASE + 0)
#define PD_EVENT_ACC_RW_ERASE (PD_EVENT_ACC_BASE + 1)
/* PD power supply events */
#define PD_EVENT_PS_BASE 0x40
#define PD_EVENT_PS_FAULT (PD_EVENT_PS_BASE + 0)
/* PD video dongles events */
#define PD_EVENT_VIDEO_BASE 0x60
#define PD_EVENT_VIDEO_DP_MODE (PD_EVENT_VIDEO_BASE + 0)
#define PD_EVENT_VIDEO_CODEC (PD_EVENT_VIDEO_BASE + 1)
/* Returned in the "type" field, when there is no entry available */
#define PD_EVENT_NO_ENTRY 0xff

/*
 * PD_EVENT_MCU_CHARGE event definition :
 * the payload is "struct usb_chg_measures"
 * the data field contains the port state flags as defined below :
 */
/* Port partner is a dual role device */
#define CHARGE_FLAGS_DUAL_ROLE BIT(15)
/* Port is the pending override port */
#define CHARGE_FLAGS_DELAYED_OVERRIDE BIT(14)
/* Port is the override port */
#define CHARGE_FLAGS_OVERRIDE BIT(13)
/* Charger type */
#define CHARGE_FLAGS_TYPE_SHIFT 3
#define CHARGE_FLAGS_TYPE_MASK (0xf << CHARGE_FLAGS_TYPE_SHIFT)
/* Power delivery role */
#define CHARGE_FLAGS_ROLE_MASK (7 << 0)

/*
 * PD_EVENT_PS_FAULT data field flags definition :
 */
#define PS_FAULT_OCP 1
#define PS_FAULT_FAST_OCP 2
#define PS_FAULT_OVP 3
#define PS_FAULT_DISCH 4

/*
 * PD_EVENT_VIDEO_CODEC payload is "struct mcdp_info".
 */
struct mcdp_version {
	uint8_t major;
	uint8_t minor;
	uint16_t build;
} __ec_align4;

struct mcdp_info {
	uint8_t family[2];
	uint8_t chipid[2];
	struct mcdp_version irom;
	struct mcdp_version fw;
} __ec_align4;

/* struct mcdp_info field decoding */
#define MCDP_CHIPID(chipid) ((chipid[0] << 8) | chipid[1])
#define MCDP_FAMILY(family) ((family[0] << 8) | family[1])

/* Get/Set USB-PD Alternate mode info */
#define EC_CMD_USB_PD_GET_AMODE 0x0116
struct ec_params_usb_pd_get_mode_request {
	uint16_t svid_idx; /* SVID index to get */
	uint8_t port; /* port */
} __ec_align_size1;

#define VDO_MAX_SIZE 7
/* Max number of VDM data objects without VDM header */
#define VDO_MAX_OBJECTS (VDO_MAX_SIZE - 1)

struct ec_params_usb_pd_get_mode_response {
	uint16_t svid; /* SVID */
	uint16_t opos; /* Object Position */
	uint32_t vdo[VDO_MAX_OBJECTS]; /* Mode VDOs */
} __ec_align4;

#define EC_CMD_USB_PD_SET_AMODE 0x0117

enum pd_mode_cmd {
	PD_EXIT_MODE = 0,
	PD_ENTER_MODE = 1,
	/* Not a command.  Do NOT remove. */
	PD_MODE_CMD_COUNT,
};

struct ec_params_usb_pd_set_mode_request {
	uint32_t cmd; /* enum pd_mode_cmd */
	uint16_t svid; /* SVID to set */
	uint8_t opos; /* Object Position */
	uint8_t port; /* port */
} __ec_align4;

/* Ask the PD MCU to record a log of a requested type */
#define EC_CMD_PD_WRITE_LOG_ENTRY 0x0118

struct ec_params_pd_write_log_entry {
	uint8_t type; /* event type : see PD_EVENT_xx above */
	uint8_t port; /* port#, or 0 for events unrelated to a given port */
} __ec_align1;

/* Control USB-PD chip */
#define EC_CMD_PD_CONTROL 0x0119

enum ec_pd_control_cmd {
	PD_SUSPEND = 0, /* Suspend the PD chip (EC: stop talking to PD) */
	PD_RESUME, /* Resume the PD chip (EC: start talking to PD) */
	PD_RESET, /* Force reset the PD chip */
	PD_CONTROL_DISABLE, /* Disable further calls to this command */
	PD_CHIP_ON, /* Power on the PD chip */
};

struct ec_params_pd_control {
	uint8_t chip; /* chip id */
	uint8_t subcmd;
} __ec_align1;

/* Get info about USB-C SS muxes */
#define EC_CMD_USB_PD_MUX_INFO 0x011A

struct ec_params_usb_pd_mux_info {
	uint8_t port; /* USB-C port number */
} __ec_align1;

/* Flags representing mux state */
#define USB_PD_MUX_NONE 0 /* Open switch */
#define USB_PD_MUX_USB_ENABLED BIT(0) /* USB connected */
#define USB_PD_MUX_DP_ENABLED BIT(1) /* DP connected */
#define USB_PD_MUX_POLARITY_INVERTED BIT(2) /* CC line Polarity inverted */
#define USB_PD_MUX_HPD_IRQ BIT(3) /* HPD IRQ is asserted */
#define USB_PD_MUX_HPD_IRQ_DEASSERTED 0 /* HPD IRQ is deasserted */
#define USB_PD_MUX_HPD_LVL BIT(4) /* HPD level is asserted */
#define USB_PD_MUX_HPD_LVL_DEASSERTED 0 /* HPD level is deasserted */
#define USB_PD_MUX_SAFE_MODE BIT(5) /* DP is in safe mode */
#define USB_PD_MUX_TBT_COMPAT_ENABLED BIT(6) /* TBT compat enabled */
#define USB_PD_MUX_USB4_ENABLED BIT(7) /* USB4 enabled */

/* USB-C Dock connected */
#define USB_PD_MUX_DOCK (USB_PD_MUX_USB_ENABLED | USB_PD_MUX_DP_ENABLED)

struct ec_response_usb_pd_mux_info {
	uint8_t flags; /* USB_PD_MUX_*-encoded USB mux state */
} __ec_align1;

#define EC_CMD_PD_CHIP_INFO 0x011B

struct ec_params_pd_chip_info {
	uint8_t port; /* USB-C port number */
	/*
	 * Fetch the live chip info or hard-coded + cached chip info
	 * 0: hardcoded value for VID/PID, cached value for FW version
	 * 1: live chip value for VID/PID/FW Version
	 */
	uint8_t live;
} __ec_align1;

struct ec_response_pd_chip_info {
	uint16_t vendor_id;
	uint16_t product_id;
	uint16_t device_id;
	union {
		uint8_t fw_version_string[8];
		uint64_t fw_version_number;
	} __ec_align2;
} __ec_align2;

struct ec_response_pd_chip_info_v1 {
	uint16_t vendor_id;
	uint16_t product_id;
	uint16_t device_id;
	union {
		uint8_t fw_version_string[8];
		uint64_t fw_version_number;
	} __ec_align2;
	union {
		uint8_t min_req_fw_version_string[8];
		uint64_t min_req_fw_version_number;
	} __ec_align2;
} __ec_align2;

/** Indicates the chip should NOT receive a firmware update, if set. This is
 *  useful when multiple ports are serviced by a single chip, to avoid
 *  performing redundant updates. The host command implementation shall ensure
 *  only one port out of each physical chip has FW updates active.
 */
#define USB_PD_CHIP_INFO_FWUP_FLAG_NO_UPDATE BIT(0)

/** Maximum length of a project name embedded in a PDC FW image. This length
 *  does NOT include a NUL-terminator.
 */
#define USB_PD_CHIP_INFO_PROJECT_NAME_LEN 12

struct ec_response_pd_chip_info_v2 {
	uint16_t vendor_id;
	uint16_t product_id;
	uint16_t device_id;
	union {
		uint8_t fw_version_string[8];
		uint64_t fw_version_number;
	} __ec_align2;
	union {
		uint8_t min_req_fw_version_string[8];
		uint64_t min_req_fw_version_number;
	} __ec_align2;
	/** Flag to control the FW update process for this chip. */
	uint16_t fw_update_flags;
	/** Project name string associated with the chip's FW. Add an extra
	 *  byte for a NUL-terminator.
	 */
	char fw_name_str[USB_PD_CHIP_INFO_PROJECT_NAME_LEN + 1];
} __ec_align2;

/** Maximum length of a driver/chip name reported in the pd_chip_info
 *  response
 */
#define USB_PD_CHIP_INFO_DRIVER_NAME_LEN 24

struct ec_response_pd_chip_info_v3 {
	uint16_t vendor_id;
	uint16_t product_id;
	uint16_t device_id;
	union {
		uint8_t fw_version_string[8];
		uint64_t fw_version_number;
	} __ec_align2;
	union {
		uint8_t min_req_fw_version_string[8];
		uint64_t min_req_fw_version_number;
	} __ec_align2;
	/** Flag to control the FW update process for this chip. */
	uint16_t fw_update_flags;
	/** Project name string associated with the chip's FW. Add an extra
	 *  byte for a NUL-terminator.
	 */
	char fw_name_str[USB_PD_CHIP_INFO_PROJECT_NAME_LEN + 1];
	/** Driver/chip string, plus room for a NUL-terminator */
	char driver_name[USB_PD_CHIP_INFO_DRIVER_NAME_LEN + 1];
} __ec_align2;

/* Run RW signature verification and get status */
#define EC_CMD_RWSIG_CHECK_STATUS 0x011C

struct ec_response_rwsig_check_status {
	uint32_t status;
} __ec_align4;

/* For controlling RWSIG task */
#define EC_CMD_RWSIG_ACTION 0x011D

enum rwsig_action {
	RWSIG_ACTION_ABORT = 0, /* Abort RWSIG and prevent jumping */
	RWSIG_ACTION_CONTINUE = 1, /* Jump to RW immediately */
};

struct ec_params_rwsig_action {
	uint32_t action;
} __ec_align4;

/* Run verification on a slot */
#define EC_CMD_EFS_VERIFY 0x011E

struct ec_params_efs_verify {
	uint8_t region; /* enum ec_flash_region */
} __ec_align1;

/*
 * Retrieve info from Cros Board Info store. Response is based on the data
 * type. Integers return a uint32. Strings return a string, using the response
 * size to determine how big it is.
 */
#define EC_CMD_GET_CROS_BOARD_INFO 0x011F
/*
 * Write info into Cros Board Info on EEPROM. Write fails if the board has
 * hardware write-protect enabled.
 */
#define EC_CMD_SET_CROS_BOARD_INFO 0x0120

enum cbi_data_tag {
	CBI_TAG_BOARD_VERSION = 0, /* uint32_t or smaller */
	CBI_TAG_OEM_ID = 1, /* uint32_t or smaller */
	CBI_TAG_SKU_ID = 2, /* uint32_t or smaller */
	CBI_TAG_DRAM_PART_NUM = 3, /* variable length ascii, nul terminated. */
	CBI_TAG_OEM_NAME = 4, /* variable length ascii, nul terminated. */
	CBI_TAG_MODEL_ID = 5, /* uint32_t or smaller */
	CBI_TAG_FW_CONFIG = 6, /* uint32_t bit field */
	CBI_TAG_PCB_SUPPLIER = 7, /* uint32_t or smaller */
	/* Second Source Factory Cache */
	CBI_TAG_SSFC = 8, /* uint32_t bit field */
	CBI_TAG_REWORK_ID = 9, /* uint64_t or smaller */
	CBI_TAG_FACTORY_CALIBRATION_DATA = 10, /* Deprecated */
	CBI_TAG_COMMON_CONTROL = 11, /* Deprecated */
	/* struct board_batt_params */
	CBI_TAG_BATTERY_CONFIG = 12,
	/* CBI_TAG_BATTERY_CONFIG_1 ~ 15 will use 13 ~ 27. */
	CBI_TAG_BATTERY_CONFIG_15 = 27,

	/* CBI_TAG_PROVISION_MATRIX_VERSION
	 * Version of the current provision matrix
	 */
	CBI_TAG_PROVISION_MATRIX_VERSION = 28, /* uint32_t bit field */

	/* Unified Firmware and Second-source Config:
	 * A fixed-size array of 4 uint32_t values.
	 */
	CBI_TAG_UFSC = 29,

	/* Last entry */
	CBI_TAG_COUNT,
};

#define CBI_UFSC_DATA_COUNT 4

/* Unified Firmware and Second-source Config (UFSC) data structure */
struct cbi_ufsc {
	uint32_t data[CBI_UFSC_DATA_COUNT];
};

/*
 * Flags to control read operation
 *
 * RELOAD:  Invalidate cache and read data from EEPROM. Useful to verify
 *          write was successful without reboot.
 */
#define CBI_GET_RELOAD BIT(0)

struct ec_params_get_cbi {
	uint32_t tag; /* enum cbi_data_tag */
	uint32_t flag; /* CBI_GET_* */
} __ec_align4;

/*
 * Flags to control write behavior.
 *
 * NO_SYNC: Makes EC update data in RAM but skip writing to EEPROM. It's
 *          useful when writing multiple fields in a row.
 * INIT:    Need to be set when creating a new CBI from scratch. All fields
 *          will be initialized to zero first.
 */
#define CBI_SET_NO_SYNC BIT(0)
#define CBI_SET_INIT BIT(1)

struct ec_params_set_cbi {
	uint32_t tag; /* enum cbi_data_tag */
	uint32_t flag; /* CBI_SET_* */
	uint32_t size; /* Data size */
	uint8_t data[FLEXIBLE_ARRAY_MEMBER_SIZE]; /* For string and raw data */
} __ec_align1;

/*
 * Retrieve binary from CrOS Board Info primary memory source.
 */
#define EC_CMD_CBI_BIN_READ 0x0504
/*
 * Write binary into CrOS Board Info temporary buffer and then commit it to
 * permanent storage once complete. Write fails if the board has hardware
 * write-protect enabled.
 */
#define EC_CMD_CBI_BIN_WRITE 0x0505

/*
 * CBI binary read/write flags
 * The default write behavior is to always append any data to the buffer.
 * If 'CLEAR' flag is set, buffer is cleared then data is appended.
 * If 'WRITE' flag is set, data is appended then buffer is written to memory.
 */
#define EC_CBI_BIN_BUFFER_CLEAR BIT(0)
#define EC_CBI_BIN_BUFFER_WRITE BIT(1)

struct ec_params_get_cbi_bin {
	uint32_t offset; /* Data offset */
	uint32_t size; /* Data size */
} __ec_align4;

struct ec_params_set_cbi_bin {
	uint32_t offset; /* Data offset */
	uint32_t size; /* Data size */
	uint8_t flags; /* bit field for EC_CBI_BIN_COMMIT_FLAG_* */
	uint8_t data[FLEXIBLE_ARRAY_MEMBER_SIZE]; /* For string and raw data */
} __ec_align1;

/*
 * Information about resets of the AP by the EC and the EC's own uptime.
 */
#define EC_CMD_GET_UPTIME_INFO 0x0121

/* EC reset causes */
#define EC_RESET_FLAG_OTHER BIT(0) /* Other known reason */
#define EC_RESET_FLAG_RESET_PIN BIT(1) /* Reset pin asserted */
#define EC_RESET_FLAG_BROWNOUT BIT(2) /* Brownout */
#define EC_RESET_FLAG_POWER_ON BIT(3) /* Power-on reset */
#define EC_RESET_FLAG_WATCHDOG BIT(4) /* Watchdog timer reset */
#define EC_RESET_FLAG_SOFT BIT(5) /* Soft reset trigger by core */
#define EC_RESET_FLAG_HIBERNATE BIT(6) /* Wake from hibernate */
#define EC_RESET_FLAG_RTC_ALARM BIT(7) /* RTC alarm wake */
#define EC_RESET_FLAG_WAKE_PIN BIT(8) /* Wake pin triggered wake */
#define EC_RESET_FLAG_LOW_BATTERY BIT(9) /* Low battery triggered wake */
#define EC_RESET_FLAG_SYSJUMP BIT(10) /* Jumped directly to this image */
#define EC_RESET_FLAG_HARD BIT(11) /* Hard reset from software */
#define EC_RESET_FLAG_AP_OFF BIT(12) /* Do not power on AP */
/* Some reset flags preserved from previous boot */
#define EC_RESET_FLAG_PRESERVED BIT(13)
#define EC_RESET_FLAG_USB_RESUME BIT(14) /* USB resume triggered wake */
#define EC_RESET_FLAG_RDD BIT(15) /* USB Type-C debug cable */
#define EC_RESET_FLAG_RBOX BIT(16) /* Fixed Reset Functionality */
#define EC_RESET_FLAG_SECURITY BIT(17) /* Security threat */
/* AP experienced a watchdog reset */
#define EC_RESET_FLAG_AP_WATCHDOG BIT(18)
/* Do not select RW in EFS. This enables PD in RO for Chromebox. */
#define EC_RESET_FLAG_STAY_IN_RO BIT(19)
#define EC_RESET_FLAG_EFS BIT(20) /* Jumped to this image by EFS */
#define EC_RESET_FLAG_AP_IDLE BIT(21) /* Leave alone AP */
#define EC_RESET_FLAG_INITIAL_PWR BIT(22) /* EC had power, then was reset */

/*
 * Reason codes used by the AP after a shutdown to figure out why it was reset
 * by the EC.  These are sent in EC commands.  Therefore, to maintain protocol
 * compatibility:
 * - New entries must be inserted prior to the _COUNT field
 * - If an existing entry is no longer in service, it must be replaced with a
 *   RESERVED entry instead.
 * - The semantic meaning of an entry should not change.
 * - Do not exceed 2^15 - 1 for reset reasons or 2^16 - 1 for shutdown reasons.
 */
enum chipset_shutdown_reason {
	/*
	 * Beginning of reset reasons.
	 */
	CHIPSET_RESET_BEGIN = 0,
	CHIPSET_RESET_UNKNOWN = CHIPSET_RESET_BEGIN,
	/* Custom reason defined by a board.c or baseboard.c file */
	CHIPSET_RESET_BOARD_CUSTOM,
	/* Believe that the AP has hung */
	CHIPSET_RESET_HANG_REBOOT,
	/* Reset by EC console command */
	CHIPSET_RESET_CONSOLE_CMD,
	/* Reset by EC host command */
	CHIPSET_RESET_HOST_CMD,
	/* Keyboard module reset key combination */
	CHIPSET_RESET_KB_SYSRESET,
	/* Keyboard module warm reboot */
	CHIPSET_RESET_KB_WARM_REBOOT,
	/* Debug module warm reboot */
	CHIPSET_RESET_DBG_WARM_REBOOT,
	/* I cannot self-terminate.  You must lower me into the steel. */
	CHIPSET_RESET_AP_REQ,
	/* Reset as side-effect of startup sequence */
	CHIPSET_RESET_INIT,
	/* EC detected an AP watchdog event. */
	CHIPSET_RESET_AP_WATCHDOG,

	CHIPSET_RESET_COUNT, /* End of reset reasons. */

	/*
	 * Beginning of shutdown reasons.
	 */
	CHIPSET_SHUTDOWN_BEGIN = BIT(15),
	CHIPSET_SHUTDOWN_POWERFAIL = CHIPSET_SHUTDOWN_BEGIN,
	/* Forcing a shutdown as part of EC initialization */
	CHIPSET_SHUTDOWN_INIT,
	/* Custom reason on a per-board basis. */
	CHIPSET_SHUTDOWN_BOARD_CUSTOM,
	/* This is a reason to inhibit startup, not cause shut down. */
	CHIPSET_SHUTDOWN_BATTERY_INHIBIT,
	/* A power_wait_signal is being asserted */
	CHIPSET_SHUTDOWN_WAIT,
	/* Critical battery level. */
	CHIPSET_SHUTDOWN_BATTERY_CRIT,
	/* Because you told me to. */
	CHIPSET_SHUTDOWN_CONSOLE_CMD,
	/* Forcing a shutdown to effect entry to G3. */
	CHIPSET_SHUTDOWN_G3,
	/* Force shutdown due to over-temperature. */
	CHIPSET_SHUTDOWN_THERMAL,
	/* Force a chipset shutdown from the power button through EC */
	CHIPSET_SHUTDOWN_BUTTON,
	/* Force a chipset shutdown, because the AP wants to. */
	CHIPSET_SHUTDOWN_HOST_CMD,

	CHIPSET_SHUTDOWN_COUNT, /* End of shutdown reasons. */
};

struct ec_response_uptime_info {
	/*
	 * Number of milliseconds since the last EC boot. Sysjump resets
	 * typically do not restart the EC's time_since_boot epoch.
	 *
	 * WARNING: The EC's sense of time is much less accurate than the AP's
	 * sense of time, in both phase and frequency.  This timebase is similar
	 * to CLOCK_MONOTONIC_RAW, but with 1% or more frequency error.
	 */
	uint32_t time_since_ec_boot_ms;

	/*
	 * Number of times the AP was reset by the EC since the last EC boot.
	 * Note that the AP may be held in reset by the EC during the initial
	 * boot sequence, such that the very first AP boot may count as more
	 * than one here.
	 */
	uint32_t ap_resets_since_ec_boot;

	/*
	 * The set of flags which describe the EC's most recent reset.
	 * See EC_RESET_FLAG_* for details.
	 */
	uint32_t ec_reset_flags;

	/* Empty log entries have both the cause and timestamp set to zero. */
	struct ap_reset_log_entry {
		/* See enum chipset_{reset,shutdown}_reason for details. */
		uint16_t reset_cause;

		/* Reserved for protocol growth. */
		uint16_t reserved;

		/*
		 * The time of the reset's assertion, in milliseconds since the
		 * last EC boot, in the same epoch as time_since_ec_boot_ms.
		 * Set to zero if the log entry is empty.
		 */
		uint32_t reset_time_ms;
	} recent_ap_reset[4];
} __ec_align4;

/*
 * Add entropy to the device secret (stored in the rollback region).
 *
 * Depending on the chip, the operation may take a long time (e.g. to erase
 * flash), so the commands are asynchronous.
 */
#define EC_CMD_ADD_ENTROPY 0x0122

enum add_entropy_action {
	/* Add entropy to the current secret. */
	ADD_ENTROPY_ASYNC = 0,
	/*
	 * Add entropy, and also make sure that the previous secret is erased.
	 * (this can be implemented by adding entropy multiple times until
	 * all rolback blocks have been overwritten).
	 */
	ADD_ENTROPY_RESET_ASYNC = 1,
	/* Read back result from the previous operation. */
	ADD_ENTROPY_GET_RESULT = 2,
};

struct ec_params_rollback_add_entropy {
	uint8_t action;
} __ec_align1;

/*
 * Perform a single read of a given ADC channel.
 */
#define EC_CMD_ADC_READ 0x0123

struct ec_params_adc_read {
	uint8_t adc_channel;
} __ec_align1;

struct ec_response_adc_read {
	int32_t adc_value;
} __ec_align4;

/*
 * Read back rollback info
 */
#define EC_CMD_ROLLBACK_INFO 0x0124

struct ec_response_rollback_info {
	int32_t id; /* Incrementing number to indicate which region to use. */
	int32_t rollback_min_version;
	int32_t rw_rollback_version;
} __ec_align4;

struct ec_response_rollback_info_v1 {
	int32_t id; /* Incrementing number to indicate which region to use. */
	int32_t rollback_min_version;
	int32_t rw_rollback_version;
	uint8_t is_secret_inited;
	uint8_t reserved[3];
} __ec_align4;

/* Issue AP reset */
#define EC_CMD_AP_RESET 0x0125

/*****************************************************************************/
/* Locate peripheral chips
 *
 * Return values:
 * EC_RES_UNAVAILABLE: The chip type is supported but not found on system.
 * EC_RES_INVALID_PARAM: The chip type was unrecognized.
 * EC_RES_OVERFLOW: The index number exceeded the number of chip instances.
 */
#define EC_CMD_LOCATE_CHIP 0x0126

enum ec_chip_type {
	EC_CHIP_TYPE_CBI_EEPROM = 0,
	EC_CHIP_TYPE_TCPC = 1,
	EC_CHIP_TYPE_PDC = 2,
	EC_CHIP_TYPE_COUNT,
	EC_CHIP_TYPE_MAX = 0xFF,
};

enum ec_bus_type {
	EC_BUS_TYPE_I2C = 0,
	EC_BUS_TYPE_EMBEDDED = 1,
	EC_BUS_TYPE_COUNT,
	EC_BUS_TYPE_MAX = 0xFF,
};

struct ec_i2c_info {
	uint16_t port; /* Physical port for device */
	uint16_t addr_flags; /* 7-bit (or 10-bit) address */
};

struct ec_params_locate_chip {
	uint8_t type; /* enum ec_chip_type */
	uint8_t index; /* Specifies one instance of chip type */
	/* Used for type specific parameters in future */
	union {
		uint16_t reserved;
	};
} __ec_align2;

struct ec_response_locate_chip {
	uint8_t bus_type; /* enum ec_bus_type */
	uint8_t reserved; /* Aligning the following union to 2 bytes */
	union {
		struct ec_i2c_info i2c_info;
	};
} __ec_align2;

/*
 * Reboot AP on G3
 *
 * This command is used for validation purpose, where the AP needs to be
 * returned back to S0 state from G3 state without using the servo to trigger
 * wake events.
 * - With command version 0:
 * AP reboots immediately from G3
 * command usage: ectool reboot_ap_on_g3 && shutdown -h now
 * - With command version 1:
 * AP reboots after the user specified delay
 * command usage: ectool reboot_ap_on_g3 [<delay>] && shutdown -h now
 */
#define EC_CMD_REBOOT_AP_ON_G3 0x0127

struct ec_params_reboot_ap_on_g3_v1 {
	/* configurable delay in seconds in G3 state */
	uint32_t reboot_ap_at_g3_delay;
} __ec_align4;

/*****************************************************************************/
/* Get PD port capabilities
 *
 * Returns the following static *capabilities* of the given port:
 * 1) Power role: source, sink, or dual. It is not anticipated that
 *    future CrOS devices would ever be only a source, so the options are
 *    sink or dual.
 * 2) Try-power role: source, sink, or none (practically speaking, I don't
 *    believe any CrOS device would support Try.SNK, so this would be source
 *    or none).
 * 3) Data role: dfp, ufp, or dual. This will probably only be DFP or dual
 *    for CrOS devices.
 */
#define EC_CMD_GET_PD_PORT_CAPS 0x0128

enum ec_pd_power_role_caps {
	EC_PD_POWER_ROLE_SOURCE = 0,
	EC_PD_POWER_ROLE_SINK = 1,
	EC_PD_POWER_ROLE_DUAL = 2,
};

enum ec_pd_try_power_role_caps {
	EC_PD_TRY_POWER_ROLE_NONE = 0,
	EC_PD_TRY_POWER_ROLE_SINK = 1,
	EC_PD_TRY_POWER_ROLE_SOURCE = 2,
};

enum ec_pd_data_role_caps {
	EC_PD_DATA_ROLE_DFP = 0,
	EC_PD_DATA_ROLE_UFP = 1,
	EC_PD_DATA_ROLE_DUAL = 2,
};

/* From: power_manager/power_supply_properties.proto */
enum ec_pd_port_location {
	/* The location of the port is unknown, or there's only one port. */
	EC_PD_PORT_LOCATION_UNKNOWN = 0,

	/*
	 * Various positions on the device. The first word describes the side of
	 * the device where the port is located while the second clarifies the
	 * position. For example, LEFT_BACK means the farthest-back port on the
	 * left side, while BACK_LEFT means the leftmost port on the back of the
	 * device.
	 */
	EC_PD_PORT_LOCATION_LEFT = 1,
	EC_PD_PORT_LOCATION_RIGHT = 2,
	EC_PD_PORT_LOCATION_BACK = 3,
	EC_PD_PORT_LOCATION_FRONT = 4,
	EC_PD_PORT_LOCATION_LEFT_FRONT = 5,
	EC_PD_PORT_LOCATION_LEFT_BACK = 6,
	EC_PD_PORT_LOCATION_RIGHT_FRONT = 7,
	EC_PD_PORT_LOCATION_RIGHT_BACK = 8,
	EC_PD_PORT_LOCATION_BACK_LEFT = 9,
	EC_PD_PORT_LOCATION_BACK_RIGHT = 10,
};

struct ec_params_get_pd_port_caps {
	uint8_t port; /* Which port to interrogate */
} __ec_align1;

struct ec_response_get_pd_port_caps {
	uint8_t pd_power_role_cap; /* enum ec_pd_power_role_caps */
	uint8_t pd_try_power_role_cap; /* enum ec_pd_try_power_role_caps */
	uint8_t pd_data_role_cap; /* enum ec_pd_data_role_caps */
	uint8_t pd_port_location; /* enum ec_pd_port_location */
} __ec_align1;

/*****************************************************************************/
/*
 * Button press simulation
 *
 * This command is used to simulate a button press.
 * Supported commands are vup(volume up) vdown(volume down) & rec(recovery)
 * Time duration for which button needs to be pressed is an optional parameter.
 *
 * NOTE: This is only available on unlocked devices for testing purposes only.
 */
#define EC_CMD_BUTTON 0x0129

struct ec_params_button {
	/* Button mask aligned to enum keyboard_button_type */
	uint32_t btn_mask;

	/* Duration in milliseconds button needs to be pressed */
	uint32_t press_ms;
} __ec_align1;

enum keyboard_button_type {
	KEYBOARD_BUTTON_POWER = 0,
	KEYBOARD_BUTTON_VOLUME_DOWN = 1,
	KEYBOARD_BUTTON_VOLUME_UP = 2,
	KEYBOARD_BUTTON_RECOVERY = 3,
	KEYBOARD_BUTTON_CAPSENSE_1 = 4,
	KEYBOARD_BUTTON_CAPSENSE_2 = 5,
	KEYBOARD_BUTTON_CAPSENSE_3 = 6,
	KEYBOARD_BUTTON_CAPSENSE_4 = 7,
	KEYBOARD_BUTTON_CAPSENSE_5 = 8,
	KEYBOARD_BUTTON_CAPSENSE_6 = 9,
	KEYBOARD_BUTTON_CAPSENSE_7 = 10,
	KEYBOARD_BUTTON_CAPSENSE_8 = 11,

	KEYBOARD_BUTTON_COUNT,
};

/*****************************************************************************/
/*
 *  "Get the Keyboard Config". An EC implementing this command is expected to be
 *  vivaldi capable, i.e. can send action codes for the top row keys.
 *  Additionally, capability to send function codes for the same keys is
 *  optional and acceptable.
 *
 *  Note: If the top row can generate both function and action codes by
 *  using a dedicated Fn key, it does not matter whether the key sends
 *  "function" or "action" codes by default. In both cases, the response
 *  for this command will look the same.
 */
#define EC_CMD_GET_KEYBD_CONFIG 0x012A

/* Possible values for the top row keys */
enum action_key {
	TK_ABSENT = 0,
	TK_BACK = 1,
	TK_FORWARD = 2,
	TK_REFRESH = 3,
	TK_FULLSCREEN = 4,
	TK_OVERVIEW = 5,
	TK_BRIGHTNESS_DOWN = 6,
	TK_BRIGHTNESS_UP = 7,
	TK_VOL_MUTE = 8,
	TK_VOL_DOWN = 9,
	TK_VOL_UP = 10,
	TK_SNAPSHOT = 11,
	TK_PRIVACY_SCRN_TOGGLE = 12,
	TK_KBD_BKLIGHT_DOWN = 13,
	TK_KBD_BKLIGHT_UP = 14,
	TK_PLAY_PAUSE = 15,
	TK_NEXT_TRACK = 16,
	TK_PREV_TRACK = 17,
	TK_KBD_BKLIGHT_TOGGLE = 18,
	TK_MICMUTE = 19,
	TK_MENU = 20,
	TK_DICTATE = 21,
	TK_ACCESSIBILITY = 22,
	TK_DONOTDISTURB = 23,
	TK_HOME = 24,

	TK_COUNT
};

/*
 * Max & Min number of top row keys, excluding Esc and Screenlock keys.
 * If this needs to change, please create a new version of the command.
 */
#define MAX_TOP_ROW_KEYS 15
#define MIN_TOP_ROW_KEYS 10

/*
 * Is the keyboard capable of sending function keys *in addition to*
 * action keys. This is possible for e.g. if the keyboard has a
 * dedicated Fn key.
 */
#define KEYBD_CAP_FUNCTION_KEYS BIT(0)
/*
 * Whether the keyboard has a dedicated numeric keyboard.
 */
#define KEYBD_CAP_NUMERIC_KEYPAD BIT(1)
/*
 * Whether the keyboard has a screenlock key.
 */
#define KEYBD_CAP_SCRNLOCK_KEY BIT(2)

/*
 * Whether the keyboard has an assistant key.
 */
#define KEYBD_CAP_ASSISTANT_KEY BIT(3)

struct ec_response_keybd_config {
	/*
	 *  Number of top row keys, excluding Esc and Screenlock.
	 *  If this is 0, all Vivaldi keyboard code is disabled.
	 *  (i.e. does not expose any tables to the kernel).
	 */
	uint8_t num_top_row_keys;

	/*
	 *  The action keys in the top row, in order from left to right.
	 *  The values are filled from enum action_key. Esc and Screenlock
	 *  keys are not considered part of top row keys.
	 */
	uint8_t action_keys[MAX_TOP_ROW_KEYS];

	/* Capability flags */
	uint8_t capabilities;

} __ec_align1;

/*
 * Configure smart discharge
 */
#define EC_CMD_SMART_DISCHARGE 0x012B

#define EC_SMART_DISCHARGE_FLAGS_SET BIT(0)

/* Discharge rates when the system is in cutoff or hibernation. */
struct discharge_rate {
	uint16_t cutoff; /* Discharge rate (uA) in cutoff */
	uint16_t hibern; /* Discharge rate (uA) in hibernation */
};

struct smart_discharge_zone {
	/* When the capacity (mAh) goes below this, EC cuts off the battery. */
	int cutoff;
	/* When the capacity (mAh) is below this, EC stays up. */
	int stayup;
};

struct ec_params_smart_discharge {
	uint8_t flags; /* EC_SMART_DISCHARGE_FLAGS_* */
	/*
	 * Desired hours for the battery to survive before reaching 0%. Set to
	 * zero to disable smart discharging. That is, the system hibernates as
	 * soon as the G3 idle timer expires.
	 */
	uint16_t hours_to_zero;
	/* Set both to zero to keep the current rates. */
	struct discharge_rate drate;
};

struct ec_response_smart_discharge {
	uint16_t hours_to_zero;
	struct discharge_rate drate;
	struct smart_discharge_zone dzone;
};

/*****************************************************************************/
/* Voltage regulator controls */

/*
 * Get basic info of voltage regulator for given index.
 *
 * Returns the regulator name and supported voltage list in mV.
 */
#define EC_CMD_REGULATOR_GET_INFO 0x012C

/* Maximum length of regulator name */
#define EC_REGULATOR_NAME_MAX_LEN 16

/* Maximum length of the supported voltage list. */
#define EC_REGULATOR_VOLTAGE_MAX_COUNT 16

struct ec_params_regulator_get_info {
	uint32_t index;
} __ec_align4;

struct ec_response_regulator_get_info {
	char name[EC_REGULATOR_NAME_MAX_LEN];
	uint16_t num_voltages;
	uint16_t voltages_mv[EC_REGULATOR_VOLTAGE_MAX_COUNT];
} __ec_align2;

/*
 * Configure the regulator as enabled / disabled.
 */
#define EC_CMD_REGULATOR_ENABLE 0x012D

struct ec_params_regulator_enable {
	uint32_t index;
	uint8_t enable;
} __ec_align4;

/*
 * Query if the regulator is enabled.
 *
 * Returns 1 if the regulator is enabled, 0 if not.
 */
#define EC_CMD_REGULATOR_IS_ENABLED 0x012E

struct ec_params_regulator_is_enabled {
	uint32_t index;
} __ec_align4;

struct ec_response_regulator_is_enabled {
	uint8_t enabled;
} __ec_align1;

/*
 * Set voltage for the voltage regulator within the range specified.
 *
 * The driver should select the voltage in range closest to min_mv.
 *
 * Also note that this might be called before the regulator is enabled, and the
 * setting should be in effect after the regulator is enabled.
 */
#define EC_CMD_REGULATOR_SET_VOLTAGE 0x012F

struct ec_params_regulator_set_voltage {
	uint32_t index;
	uint32_t min_mv;
	uint32_t max_mv;
} __ec_align4;

/*
 * Get the currently configured voltage for the voltage regulator.
 *
 * Note that this might be called before the regulator is enabled, and this
 * should return the configured output voltage if the regulator is enabled.
 */
#define EC_CMD_REGULATOR_GET_VOLTAGE 0x0130

struct ec_params_regulator_get_voltage {
	uint32_t index;
} __ec_align4;

struct ec_response_regulator_get_voltage {
	uint32_t voltage_mv;
} __ec_align4;

/*
 * Gather all discovery information for the given port and partner type.
 *
 * Note that if discovery has not yet completed, only the currently completed
 * responses will be filled in.   If the discovery data structures are changed
 * in the process of the command running, BUSY will be returned.
 *
 * VDO field sizes are set to the maximum possible number of VDOs a VDM may
 * contain, while the number of SVIDs here is selected to fit within the PROTO2
 * maximum parameter size.
 */
#define EC_CMD_TYPEC_DISCOVERY 0x0131

enum typec_partner_type {
	TYPEC_PARTNER_SOP = 0,
	TYPEC_PARTNER_SOP_PRIME = 1,
	TYPEC_PARTNER_SOP_PRIME_PRIME = 2,
};

struct ec_params_typec_discovery {
	uint8_t port;
	uint8_t partner_type; /* enum typec_partner_type */
} __ec_align1;

struct svid_mode_info {
	uint16_t svid;
	uint16_t mode_count; /* Number of modes partner sent */
	uint32_t mode_vdo[VDO_MAX_OBJECTS];
};

struct ec_response_typec_discovery {
	uint8_t identity_count; /* Number of identity VDOs partner sent */
	uint8_t svid_count; /* Number of SVIDs partner sent */
	uint16_t reserved;
	uint32_t discovery_vdo[VDO_MAX_OBJECTS];
	struct svid_mode_info svids[FLEXIBLE_ARRAY_MEMBER_SIZE];
} __ec_align1;

/* USB Type-C commands for AP-controlled device policy. */
#define EC_CMD_TYPEC_CONTROL 0x0132

enum typec_control_command {
	TYPEC_CONTROL_COMMAND_EXIT_MODES,
	TYPEC_CONTROL_COMMAND_CLEAR_EVENTS,
	TYPEC_CONTROL_COMMAND_ENTER_MODE,
	TYPEC_CONTROL_COMMAND_TBT_UFP_REPLY,
	TYPEC_CONTROL_COMMAND_USB_MUX_SET,
	TYPEC_CONTROL_COMMAND_BIST_SHARE_MODE,
	TYPEC_CONTROL_COMMAND_SEND_VDM_REQ,
};

/* Modes (USB or alternate) that a type-C port may enter. */
enum typec_mode {
	TYPEC_MODE_DP,
	TYPEC_MODE_TBT,
	TYPEC_MODE_USB4,
};

/* Replies the AP may specify to the TBT EnterMode command as a UFP */
enum typec_tbt_ufp_reply {
	TYPEC_TBT_UFP_REPLY_NAK,
	TYPEC_TBT_UFP_REPLY_ACK,
};

#define TYPEC_USB_MUX_SET_ALL_CHIPS 0xFF

struct typec_usb_mux_set {
	/* Index of the mux to set in the chain */
	uint8_t mux_index;

	/* USB_PD_MUX_*-encoded USB mux state to set */
	uint8_t mux_flags;
} __ec_align1;

struct typec_vdm_req {
	/* VDM data, including VDM header */
	uint32_t vdm_data[VDO_MAX_SIZE];
	/* Number of 32-bit fields filled in */
	uint8_t vdm_data_objects;
	/* Partner to address - see enum typec_partner_type */
	uint8_t partner_type;
} __ec_align1;

struct ec_params_typec_control {
	uint8_t port;
	uint8_t command; /* enum typec_control_command */
	uint16_t reserved;

	/*
	 * This section will be interpreted based on |command|. Define a
	 * placeholder structure to avoid having to increase the size and bump
	 * the command version when adding new sub-commands.
	 */
	union {
		/* Used for CLEAR_EVENTS */
		uint32_t clear_events_mask;
		/* Used for ENTER_MODE - enum typec_mode */
		uint8_t mode_to_enter;
		/* Used for TBT_UFP_REPLY - enum typec_tbt_ufp_reply */
		uint8_t tbt_ufp_reply;
		/* Used for USB_MUX_SET */
		struct typec_usb_mux_set mux_params;
		/* Used for BIST_SHARE_MODE */
		uint8_t bist_share_mode;
		/* Used for VMD_REQ */
		struct typec_vdm_req vdm_req_params;
		uint8_t placeholder[128];
	};
} __ec_align1;

/*
 * Gather all status information for a port.
 *
 * Note: this covers many of the return fields from the deprecated
 * EC_CMD_USB_PD_CONTROL command, except those that are redundant with the
 * discovery data.  The "enum pd_cc_states" is defined with the deprecated
 * EC_CMD_USB_PD_CONTROL command.
 *
 * This also combines in the EC_CMD_USB_PD_MUX_INFO flags.
 */
#define EC_CMD_TYPEC_STATUS 0x0133

/*
 * Power role.
 *
 * Note this is also used for PD header creation, and values align to those in
 * the Power Delivery Specification Revision 3.0 (See
 * 6.2.1.1.4 Port Power Role).
 */
enum pd_power_role {
	PD_ROLE_SINK = 0,
	PD_ROLE_SOURCE = 1,
};

/*
 * Data role.
 *
 * Note this is also used for PD header creation, and the first two values
 * align to those in the Power Delivery Specification Revision 3.0 (See
 * 6.2.1.1.6 Port Data Role).
 */
enum pd_data_role {
	PD_ROLE_UFP = 0,
	PD_ROLE_DFP = 1,
	PD_ROLE_DISCONNECTED = 2,
};

enum pd_vconn_role {
	PD_ROLE_VCONN_OFF = 0,
	PD_ROLE_VCONN_SRC = 1,
};

/*
 * Note: BIT(0) may be used to determine whether the polarity is CC1 or CC2,
 * regardless of whether a debug accessory is connected.
 */
enum tcpc_cc_polarity {
	/*
	 * _CCx: is used to indicate the polarity while not connected to
	 * a Debug Accessory.  Only one CC line will assert a resistor and
	 * the other will be open.
	 */
	POLARITY_CC1 = 0,
	POLARITY_CC2 = 1,

	/*
	 * _CCx_DTS is used to indicate the polarity while connected to a
	 * SRC Debug Accessory.  Assert resistors on both lines.
	 */
	POLARITY_CC1_DTS = 2,
	POLARITY_CC2_DTS = 3,

	/*
	 * The current TCPC code relies on these specific POLARITY values.
	 * Adding in a check to verify if the list grows for any reason
	 * that this will give a hint that other places need to be
	 * adjusted.
	 */
	POLARITY_COUNT,
};

#define MODE_DP_PIN_A BIT(0)
#define MODE_DP_PIN_B BIT(1)
#define MODE_DP_PIN_C BIT(2)
#define MODE_DP_PIN_D BIT(3)
#define MODE_DP_PIN_E BIT(4)
#define MODE_DP_PIN_F BIT(5)
#define MODE_DP_PIN_ALL GENMASK(5, 0)

#define PD_STATUS_EVENT_SOP_DISC_DONE BIT(0)
#define PD_STATUS_EVENT_SOP_PRIME_DISC_DONE BIT(1)
#define PD_STATUS_EVENT_HARD_RESET BIT(2)
#define PD_STATUS_EVENT_DISCONNECTED BIT(3)
#define PD_STATUS_EVENT_MUX_0_SET_DONE BIT(4)
#define PD_STATUS_EVENT_MUX_1_SET_DONE BIT(5)
#define PD_STATUS_EVENT_VDM_REQ_REPLY BIT(6)
#define PD_STATUS_EVENT_VDM_REQ_FAILED BIT(7)
#define PD_STATUS_EVENT_VDM_ATTENTION BIT(8)
#define PD_STATUS_EVENT_COUNT 9

/*
 * Encode and decode for BCD revision response
 *
 * Note: the major revision set is written assuming that the value given is the
 * Specification Revision from the PD header, which currently only maps to PD
 * 1.0-3.0 with the major revision being one greater than the binary value.
 */
#define PD_STATUS_REV_SET_MAJOR(r) ((r + 1) << 12)
#define PD_STATUS_REV_GET_MAJOR(r) ((r >> 12) & 0xF)
#define PD_STATUS_REV_GET_MINOR(r) ((r >> 8) & 0xF)

/*
 * Encode revision from partner RMDO
 *
 * Unlike the specification revision given in the PD header, specification and
 * version information returned in the revision message data object (RMDO) is
 * not offset.
 */
#define PD_STATUS_RMDO_REV_SET_MAJOR(r) (r << 12)
#define PD_STATUS_RMDO_REV_SET_MINOR(r) (r << 8)
#define PD_STATUS_RMDO_VER_SET_MAJOR(r) (r << 4)
#define PD_STATUS_RMDO_VER_SET_MINOR(r) (r)

/*
 * Decode helpers for Source and Sink Capability PDOs
 *
 * Note: The Power Delivery Specification should be considered the ultimate
 * source of truth on the decoding of these PDOs
 */
#define PDO_TYPE_FIXED (0 << 30)
#define PDO_TYPE_BATTERY (1 << 30)
#define PDO_TYPE_VARIABLE (2 << 30)
#define PDO_TYPE_AUGMENTED (3 << 30)
#define PDO_TYPE_MASK (3 << 30)

/*
 * From Table 6-9 and Table 6-14 PD Rev 3.0 Ver 2.0
 *
 * <31:30> : Fixed Supply
 * <29>    : Dual-Role Power
 * <28>    : SNK/SRC dependent
 * <27>    : Unconstrained Power
 * <26>    : USB Communications Capable
 * <25>    : Dual-Role Data
 * <24:20> : SNK/SRC dependent
 * <19:10> : Voltage in 50mV Units
 * <9:0>   : Maximum Current in 10mA units
 */
#define PDO_FIXED_DUAL_ROLE BIT(29)
#define PDO_FIXED_UNCONSTRAINED BIT(27)
#define PDO_FIXED_COMM_CAP BIT(26)
#define PDO_FIXED_DATA_SWAP BIT(25)
#define PDO_FIXED_FRS_CURR_MASK GENMASK(24, 23) /* Sink Cap only */
#define PDO_FIXED_VOLTAGE(p) ((p >> 10 & 0x3FF) * 50)
#define PDO_FIXED_CURRENT(p) ((p & 0x3FF) * 10)

/*
 * From Table 6-12 and Table 6-16 PD Rev 3.0 Ver 2.0
 *
 * <31:30> : Battery
 * <29:20> : Maximum Voltage in 50mV units
 * <19:10> : Minimum Voltage in 50mV units
 * <9:0>   : Maximum Allowable Power in 250mW units
 */
#define PDO_BATT_MAX_VOLTAGE(p) ((p >> 20 & 0x3FF) * 50)
#define PDO_BATT_MIN_VOLTAGE(p) ((p >> 10 & 0x3FF) * 50)
#define PDO_BATT_MAX_POWER(p) ((p & 0x3FF) * 250)

/*
 * From Table 6-11 and Table 6-15 PD Rev 3.0 Ver 2.0
 *
 * <31:30> : Variable Supply (non-Battery)
 * <29:20> : Maximum Voltage in 50mV units
 * <19:10> : Minimum Voltage in 50mV units
 * <9:0>   : Operational Current in 10mA units
 */
#define PDO_VAR_MAX_VOLTAGE(p) ((p >> 20 & 0x3FF) * 50)
#define PDO_VAR_MIN_VOLTAGE(p) ((p >> 10 & 0x3FF) * 50)
#define PDO_VAR_MAX_CURRENT(p) ((p & 0x3FF) * 10)

/*
 * From Table 6-13 and Table 6-17 PD Rev 3.0 Ver 2.0
 *
 * Note this type is reserved in PD 2.0, and only one type of APDO is
 * supported as of the cited version.
 *
 * <31:30> : Augmented Power Data Object
 * <29:28> : Programmable Power Supply
 * <27>    : PPS Power Limited
 * <26:25> : Reserved
 * <24:17> : Maximum Voltage in 100mV increments
 * <16>    : Reserved
 * <15:8>  : Minimum Voltage in 100mV increments
 * <7>     : Reserved
 * <6:0>   : Maximum Current in 50mA increments
 */
#define PDO_AUG_MAX_VOLTAGE(p) ((p >> 17 & 0xFF) * 100)
#define PDO_AUG_MIN_VOLTAGE(p) ((p >> 8 & 0xFF) * 100)
#define PDO_AUG_MAX_CURRENT(p) ((p & 0x7F) * 50)

struct ec_params_typec_status {
	uint8_t port;
} __ec_align1;

/*
 * ec_response_typec_status is deprecated. Use ec_response_typec_status_v1.
 * If you need to support old ECs who speak only v0, use
 * ec_response_typec_status_v0 instead. They're binary-compatible.
 */
struct ec_response_typec_status /* DEPRECATED */ {
	uint8_t pd_enabled; /* PD communication enabled - bool */
	uint8_t dev_connected; /* Device connected - bool */
	uint8_t sop_connected; /* Device is SOP PD capable - bool */
	uint8_t source_cap_count; /* Number of Source Cap PDOs */

	uint8_t power_role; /* enum pd_power_role */
	uint8_t data_role; /* enum pd_data_role */
	uint8_t vconn_role; /* enum pd_vconn_role */
	uint8_t sink_cap_count; /* Number of Sink Cap PDOs */

	uint8_t polarity; /* enum tcpc_cc_polarity */
	uint8_t cc_state; /* enum pd_cc_states */
	uint8_t dp_pin; /* DP pin mode (MODE_DP_IN_[A-E]) */
	uint8_t mux_state; /* USB_PD_MUX* - encoded mux state */

	char tc_state[32]; /* TC state name */

	uint32_t events; /* PD_STATUS_EVENT bitmask */

	/*
	 * BCD PD revisions for partners
	 *
	 * The format has the PD major revision in the upper nibble, and the PD
	 * minor revision in the next nibble. The following two nibbles hold the
	 * major and minor specification version. If a partner does not support
	 * the Revision message, only the major revision will be given.
	 * ex. PD Revision 3.2 Version 1.9 would map to 0x3219
	 *
	 * PD revision/version will be 0 if no PD device is connected.
	 */
	uint16_t sop_revision;
	uint16_t sop_prime_revision;

	uint32_t source_cap_pdos[7]; /* Max 7 PDOs can be present */

	uint32_t sink_cap_pdos[7]; /* Max 7 PDOs can be present */
} __ec_align1;

struct cros_ec_typec_status {
	uint8_t pd_enabled; /* PD communication enabled - bool */
	uint8_t dev_connected; /* Device connected - bool */
	uint8_t sop_connected; /* Device is SOP PD capable - bool */
	uint8_t source_cap_count; /* Number of Source Cap PDOs */

	uint8_t power_role; /* enum pd_power_role */
	uint8_t data_role; /* enum pd_data_role */
	uint8_t vconn_role; /* enum pd_vconn_role */
	uint8_t sink_cap_count; /* Number of Sink Cap PDOs */

	uint8_t polarity; /* enum tcpc_cc_polarity */
	uint8_t cc_state; /* enum pd_cc_states */
	uint8_t dp_pin; /* DP pin mode (MODE_DP_IN_[A-E]) */
	uint8_t mux_state; /* USB_PD_MUX* - encoded mux state */

	char tc_state[32]; /* TC state name */

	uint32_t events; /* PD_STATUS_EVENT bitmask */

	/*
	 * BCD PD revisions for partners
	 *
	 * The format has the PD major revision in the upper nibble, and the PD
	 * minor revision in the next nibble. The following two nibbles hold the
	 * major and minor specification version. If a partner does not support
	 * the Revision message, only the major revision will be given.
	 * ex. PD Revision 3.2 Version 1.9 would map to 0x3219
	 *
	 * PD revision/version will be 0 if no PD device is connected.
	 */
	uint16_t sop_revision;
	uint16_t sop_prime_revision;
} __ec_align1;

struct ec_response_typec_status_v0 {
	struct cros_ec_typec_status typec_status;
	uint32_t source_cap_pdos[7]; /* Max 7 PDOs can be present */
	uint32_t sink_cap_pdos[7]; /* Max 7 PDOs can be present */
} __ec_align1;

struct ec_response_typec_status_v1 {
	struct cros_ec_typec_status typec_status;
	uint32_t source_cap_pdos[11]; /* Max 11 PDOs can be present */
	uint32_t sink_cap_pdos[11]; /* Max 11 PDOs can be present */
} __ec_align1;

/**
 * Get the number of peripheral charge ports
 */
#define EC_CMD_PCHG_COUNT 0x0134

#define EC_PCHG_MAX_PORTS 8

struct ec_response_pchg_count {
	uint8_t port_count;
} __ec_align1;

/**
 * Get the status of a peripheral charge port
 */
#define EC_CMD_PCHG 0x0135

/* For v1 and v2 */
struct ec_params_pchg {
	uint8_t port;
} __ec_align1;

struct ec_params_pchg_v3 {
	uint8_t port;
	/* Below are new in v3. */
	uint8_t reserved1;
	uint8_t reserved2;
	uint8_t reserved3;
	/* Errors acked by the host (thus to be cleared) */
	uint32_t error;
} __ec_align1;

/* For v1 */
struct ec_response_pchg {
	uint32_t error; /* enum pchg_error */
	uint8_t state; /* enum pchg_state state */
	uint8_t battery_percentage;
	uint8_t unused0;
	uint8_t unused1;
	/* Fields added in version 1 */
	uint32_t fw_version;
	uint32_t dropped_event_count;
} __ec_align4;

/* For v2 and v3 */
struct ec_response_pchg_v2 {
	uint32_t error; /* enum pchg_error */
	uint8_t state; /* enum pchg_state state */
	uint8_t battery_percentage;
	uint8_t unused0;
	uint8_t unused1;
	/* Fields added in version 1 */
	uint32_t fw_version;
	uint32_t dropped_event_count;
	/* Fields added in version 2 */
	uint32_t dropped_host_event_count;
} __ec_align4;

enum pchg_state {
	/* Charger is reset and not initialized. */
	PCHG_STATE_RESET = 0,
	/* Charger is initialized or disabled. */
	PCHG_STATE_INITIALIZED,
	/* Charger is enabled and ready to detect a device. */
	PCHG_STATE_ENABLED,
	/* Device is in proximity. */
	PCHG_STATE_DETECTED,
	/* Device is being charged. */
	PCHG_STATE_CHARGING,
	/* Device is fully charged. It implies DETECTED (& not charging). */
	PCHG_STATE_FULL,
	/* In download (a.k.a. firmware update) mode */
	PCHG_STATE_DOWNLOAD,
	/* In download mode. Ready for receiving data. */
	PCHG_STATE_DOWNLOADING,
	/* Device is ready for data communication. */
	PCHG_STATE_CONNECTED,
	/* Charger is in Built-In Self Test mode. */
	PCHG_STATE_BIST,
	/* Put no more entry below */
	PCHG_STATE_COUNT,
};

/* clang-format off */
#define EC_PCHG_STATE_TEXT                                \
	{                                                 \
		[PCHG_STATE_RESET] = "RESET",             \
		[PCHG_STATE_INITIALIZED] = "INITIALIZED", \
		[PCHG_STATE_ENABLED] = "ENABLED",         \
		[PCHG_STATE_DETECTED] = "DETECTED",       \
		[PCHG_STATE_CHARGING] = "CHARGING",       \
		[PCHG_STATE_FULL] = "FULL",               \
		[PCHG_STATE_DOWNLOAD] = "DOWNLOAD",       \
		[PCHG_STATE_DOWNLOADING] = "DOWNLOADING", \
		[PCHG_STATE_CONNECTED] = "CONNECTED",     \
		[PCHG_STATE_BIST] = "BIST",               \
	}
/* clang-format on */

/**
 * Update firmware of peripheral chip
 */
#define EC_CMD_PCHG_UPDATE 0x0136

/* Port number is encoded in bit[28:31]. */
#define EC_MKBP_PCHG_PORT_SHIFT 28
/* Utility macros for converting MKBP event <-> port number. */
#define EC_MKBP_PCHG_EVENT_TO_PORT(e) (((e) >> EC_MKBP_PCHG_PORT_SHIFT) & 0xf)
#define EC_MKBP_PCHG_PORT_TO_EVENT(p) ((p) << EC_MKBP_PCHG_PORT_SHIFT)
/* Utility macro for extracting event bits. */
#define EC_MKBP_PCHG_EVENT_MASK(e) \
	((e) & GENMASK(EC_MKBP_PCHG_PORT_SHIFT - 1, 0))

#define EC_MKBP_PCHG_UPDATE_OPENED BIT(0)
#define EC_MKBP_PCHG_WRITE_COMPLETE BIT(1)
#define EC_MKBP_PCHG_UPDATE_CLOSED BIT(2)
#define EC_MKBP_PCHG_UPDATE_ERROR BIT(3)
#define EC_MKBP_PCHG_DEVICE_EVENT BIT(4)

enum ec_pchg_update_cmd {
	/* Reset chip to normal mode. */
	EC_PCHG_UPDATE_CMD_RESET_TO_NORMAL = 0,
	/* Reset and put a chip in update (a.k.a. download) mode. */
	EC_PCHG_UPDATE_CMD_OPEN,
	/* Write a block of data containing FW image. */
	EC_PCHG_UPDATE_CMD_WRITE,
	/* Close update session. */
	EC_PCHG_UPDATE_CMD_CLOSE,
	/* Reset chip (without mode change). */
	EC_PCHG_UPDATE_CMD_RESET,
	/* Enable pass-through mode. */
	EC_PCHG_UPDATE_CMD_ENABLE_PASSTHRU,
	/* End of commands */
	EC_PCHG_UPDATE_CMD_COUNT,
};

struct ec_params_pchg_update {
	/* PCHG port number */
	uint8_t port;
	/* enum ec_pchg_update_cmd */
	uint8_t cmd;
	/* Padding */
	uint8_t reserved0;
	uint8_t reserved1;
	/* Version of new firmware */
	uint32_t version;
	/* CRC32 of new firmware */
	uint32_t crc32;
	/* Address in chip memory where <data> is written to */
	uint32_t addr;
	/* Size of <data> */
	uint32_t size;
	/* Partial data of new firmware */
	uint8_t data[FLEXIBLE_ARRAY_MEMBER_SIZE];
} __ec_align4;

BUILD_ASSERT(EC_PCHG_UPDATE_CMD_COUNT <
	     BIT(sizeof(((struct ec_params_pchg_update *)0)->cmd) * 8));

struct ec_response_pchg_update {
	/* Block size */
	uint32_t block_size;
} __ec_align4;

#define EC_CMD_DISPLAY_SOC 0x0137

struct ec_response_display_soc {
	/* Display charge in 10ths of a % (1000=100.0%) */
	int16_t display_soc;
	/* Full factor in 10ths of a % (1000=100.0%) */
	int16_t full_factor;
	/* Shutdown SoC in 10ths of a % (1000=100.0%) */
	int16_t shutdown_soc;
} __ec_align2;

#define EC_CMD_SET_BASE_STATE 0x0138

struct ec_params_set_base_state {
	uint8_t cmd; /* enum ec_set_base_state_cmd */
} __ec_align1;

enum ec_set_base_state_cmd {
	EC_SET_BASE_STATE_DETACH = 0,
	EC_SET_BASE_STATE_ATTACH,
	EC_SET_BASE_STATE_RESET,
};

#define EC_CMD_I2C_CONTROL 0x0139

/* Subcommands for I2C control */

enum ec_i2c_control_command {
	EC_I2C_CONTROL_GET_SPEED,
	EC_I2C_CONTROL_SET_SPEED,
};

#define EC_I2C_CONTROL_SPEED_UNKNOWN 0

struct ec_params_i2c_control {
	uint8_t port; /* I2C port number */
	uint8_t cmd; /* enum ec_i2c_control_command */
	union {
		uint16_t speed_khz;
	} cmd_params;
} __ec_align_size1;

struct ec_response_i2c_control {
	union {
		uint16_t speed_khz;
	} cmd_response;
} __ec_align_size1;

#define EC_CMD_RGBKBD_SET_COLOR 0x013A
#define EC_CMD_RGBKBD 0x013B

#define EC_RGBKBD_MAX_KEY_COUNT 128
#define EC_RGBKBD_MAX_RGB_COLOR 0xFFFFFF
#define EC_RGBKBD_MAX_SCALE 0xFF

enum rgbkbd_state {
	/* RGB keyboard is reset and not initialized. */
	RGBKBD_STATE_RESET = 0,
	/* RGB keyboard is initialized but not enabled. */
	RGBKBD_STATE_INITIALIZED,
	/* RGB keyboard is disabled. */
	RGBKBD_STATE_DISABLED,
	/* RGB keyboard is enabled and ready to receive a command. */
	RGBKBD_STATE_ENABLED,

	/* Put no more entry below */
	RGBKBD_STATE_COUNT,
};

enum ec_rgbkbd_subcmd {
	EC_RGBKBD_SUBCMD_CLEAR = 1,
	EC_RGBKBD_SUBCMD_DEMO = 2,
	EC_RGBKBD_SUBCMD_SET_SCALE = 3,
	EC_RGBKBD_SUBCMD_GET_CONFIG = 4,
	EC_RGBKBD_SUBCMD_COUNT
};

enum ec_rgbkbd_demo {
	EC_RGBKBD_DEMO_OFF = 0,
	EC_RGBKBD_DEMO_FLOW = 1,
	EC_RGBKBD_DEMO_DOT = 2,
	EC_RGBKBD_DEMO_COUNT,
};

BUILD_ASSERT(EC_RGBKBD_DEMO_COUNT <= 255);

enum ec_rgbkbd_type {
	EC_RGBKBD_TYPE_UNKNOWN = 0,
	EC_RGBKBD_TYPE_PER_KEY = 1, /* e.g. Vell */
	EC_RGBKBD_TYPE_FOUR_ZONES_40_LEDS = 2, /* e.g. Taniks */
	EC_RGBKBD_TYPE_FOUR_ZONES_12_LEDS = 3, /* e.g. Osiris */
	EC_RGBKBD_TYPE_FOUR_ZONES_4_LEDS = 4, /* e.g. Mithrax */
	EC_RGBKBD_TYPE_COUNT,
};

struct ec_rgbkbd_set_scale {
	uint8_t key;
	struct rgb_s scale;
};

struct ec_params_rgbkbd {
	uint8_t subcmd; /* Sub-command (enum ec_rgbkbd_subcmd) */
	union {
		struct rgb_s color; /* EC_RGBKBD_SUBCMD_CLEAR */
		uint8_t demo; /* EC_RGBKBD_SUBCMD_DEMO */
		struct ec_rgbkbd_set_scale set_scale;
	};
} __ec_align1;

struct ec_response_rgbkbd {
	/*
	 * RGBKBD type supported by the device.
	 */

	uint8_t rgbkbd_type; /* enum ec_rgbkbd_type */
} __ec_align1;

struct ec_params_rgbkbd_set_color {
	/* Specifies the starting key ID whose color is being changed. */
	uint8_t start_key;
	/* Specifies # of elements in <color>. */
	uint8_t length;
	/* RGB color data array of length up to MAX_KEY_COUNT. */
	struct rgb_s color[FLEXIBLE_ARRAY_MEMBER_SIZE];
} __ec_align1;

/*
 * Gather the response to the most recent VDM REQ from the AP, as well
 * as popping the oldest VDM:Attention from the DPM queue
 */
#define EC_CMD_TYPEC_VDM_RESPONSE 0x013C

struct ec_params_typec_vdm_response {
	uint8_t port;
} __ec_align1;

struct ec_response_typec_vdm_response {
	/* Number of 32-bit fields filled in */
	uint8_t vdm_data_objects;
	/* Partner to address - see enum typec_partner_type */
	uint8_t partner_type;
	/* enum ec_status describing VDM response */
	uint16_t vdm_response_err;
	/* VDM data, including VDM header */
	uint32_t vdm_response[VDO_MAX_SIZE];
	/* Number of 32-bit Attention fields filled in */
	uint8_t vdm_attention_objects;
	/* Number of remaining messages to consume */
	uint8_t vdm_attention_left;
	/* Reserved */
	uint16_t reserved1;
	/* VDM:Attention contents */
	uint32_t vdm_attention[2];
} __ec_align1;

/*
 * Get an active battery config from the EC.
 */
#define EC_CMD_BATTERY_CONFIG 0x013D

/* Version of struct batt_conf_header and its internals. */
#define EC_BATTERY_CONFIG_STRUCT_VERSION 0x00

/* Number of writes needed to invoke battery cutoff command */
#define SHIP_MODE_WRITES 2

struct ship_mode_info {
	uint8_t reg_addr;
	uint8_t reserved;
	uint16_t reg_data[SHIP_MODE_WRITES];
} __packed __aligned(2);

struct sleep_mode_info {
	uint8_t reg_addr;
	uint8_t reserved;
	uint16_t reg_data;
} __packed __aligned(2);

struct fet_info {
	uint8_t reg_addr;
	uint8_t reserved;
	uint16_t reg_mask;
	uint16_t disconnect_val;
	uint16_t cfet_mask; /* CHG FET status mask */
	uint16_t cfet_off_val;
} __packed __aligned(2);

enum fuel_gauge_flags {
	/*
	 * Write Block Support. If enabled, we use a i2c write block command
	 * instead of a 16-bit write. The effective difference is the i2c
	 * transaction will prefix the length (2).
	 */
	FUEL_GAUGE_FLAG_WRITE_BLOCK = BIT(0),
	/* Sleep command support. fuel_gauge_info.sleep_mode must be defined. */
	FUEL_GAUGE_FLAG_SLEEP_MODE = BIT(1),
	/*
	 * Manufacturer access command support. If enabled, FET status is read
	 * from the OperationStatus (0x54) register using the
	 * ManufacturerBlockAccess (0x44).
	 */
	FUEL_GAUGE_FLAG_MFGACC = BIT(2),
	/*
	 * SMB block protocol support in manufacturer access command. If
	 * enabled, FET status is read from the OperationStatus (0x54) register
	 * using the ManufacturerBlockAccess (0x44).
	 */
	FUEL_GAUGE_FLAG_MFGACC_SMB_BLOCK = BIT(3),
};

struct fuel_gauge_info {
	uint32_t flags;
	uint32_t board_flags;
	struct ship_mode_info ship_mode;
	struct sleep_mode_info sleep_mode;
	struct fet_info fet;
} __packed __aligned(4);

/* Battery constants */
struct battery_info {
	/* Operation voltage in mV */
	uint16_t voltage_max;
	uint16_t voltage_normal;
	uint16_t voltage_min;
	/* (TODO(chromium:756700): add desired_charging_current */
	/**
	 * Pre-charge to fast charge threshold in mV,
	 * default to voltage_min if not specified.
	 * This option is only available on isl923x and rt946x.
	 */
	uint16_t precharge_voltage;
	/* Pre-charge current in mA */
	uint16_t precharge_current;
	/* Working temperature ranges in degrees C */
	int8_t start_charging_min_c;
	int8_t start_charging_max_c;
	int8_t charging_min_c;
	int8_t charging_max_c;
	int8_t discharging_min_c;
	int8_t discharging_max_c;
	/* Used only if CONFIG_BATTERY_VENDOR_PARAM is defined. */
	uint8_t vendor_param_start;
	uint8_t reserved;
} __packed __aligned(2);

/*
 * The 'config' of a battery.
 */
struct board_batt_params {
	struct fuel_gauge_info fuel_gauge;
	struct battery_info batt_info;
} __packed __aligned(4);

/*
 * The SBS defines a string object as a block of chars, 32 byte maximum, where
 * the first byte indicates the number of chars in the block (excluding the
 * first byte).
 *
 * Thus, the actual string length (i.e. the value strlen returns) is limited to
 * 31 (=SBS_MAX_STR_SIZE).
 *
 * SBS_MAX_STR_OBJ_SIZE can be used as the size of a buffer for an SBS string
 * object but also as a buffer for a c-lang string because the null terminating
 * char also takes one byte.
 */
#define SBS_MAX_STR_SIZE 31
#define SBS_MAX_STR_OBJ_SIZE (SBS_MAX_STR_SIZE + 1)

/*
 * Header describing a battery config stored in CBI. Only struct_version has
 * size and position independent of struct_version. The rest varies as
 * struct_version changes.
 *
 * Version 0
 * Layout:
 *  +-------------+
 *  | header      |
 *  +-------------+
 *  |             | ^
 *  | manuf_name  | | manuf_name_size
 *  |             | v
 *  +-------------+
 *  | device_name | ^
 *  |             | | device_name_size
 *  |             | v
 *  +-------------+
 *  | config      | ^
 *  |             | |
 *  |             | | cbi data size
 *  |             | |    - (header_size+manuf_name_size+device_name_size)
 *  |             | |
 *  |             | v
 *  +-------------+
 * Note:
 * - manuf_name and device_name are not null-terminated.
 * - The config isn't aligned. It should be copied to struct board_batt_params
 *   before its contents are accessed.
 */
struct batt_conf_header {
	/* Version independent field. It's always here as a uint8_t. */
	uint8_t struct_version;
	/* Version 0 members */
	uint8_t manuf_name_size;
	uint8_t device_name_size;
	uint8_t reserved;
	/* manuf_name, device_name, board_batt_params follow after this. */
} __packed;

#define BATT_CONF_MAX_SIZE                                            \
	(sizeof(struct batt_conf_header) + SBS_MAX_STR_OBJ_SIZE * 2 + \
	 sizeof(struct board_batt_params))

/*
 * Record the current AP firmware state. This is used to help testing, such as
 * with FAFT (Fully-Automated Firmware Test), which likes to know which firmware
 * screen is currently displayed.
 */

#define EC_CMD_AP_FW_STATE 0x013E

struct ec_params_ap_fw_state {
	/*
	 * Value which indicates the state. This is not decoded by the EC, so
	 * its meaning is entirely outside this code base.
	 */
	uint32_t state;
} __ec_align1;

/*
 * UCSI OPM-PPM commands
 *
 * These commands are used for communication between OPM and PPM.
 * Only UCSI3.0 is tested.
 */

#define EC_CMD_UCSI_PPM_SET 0x0140

/* The data size is stored in the host command protocol header. */
struct ec_params_ucsi_ppm_set {
	uint16_t offset;
	uint8_t data[FLEXIBLE_ARRAY_MEMBER_SIZE];
} __ec_align2;

#define EC_CMD_UCSI_PPM_GET 0x0141

/* For 'GET' sub-commands, data will be returned as a raw payload. */
struct ec_params_ucsi_ppm_get {
	uint16_t offset;
	uint8_t size;
} __ec_align2;

#define EC_CMD_SET_ALARM_SLP_S0_DBG 0x0142

/* RTC params and response structures */
struct ec_params_set_alarm_slp_s0_dbg {
	uint32_t time;
} __ec_align2;

/*
 * Control PDC tracing.
 *   EC_PDC_TRACE_MSG_PORT_NONE disable tracing
 *   EC_PDC_TRACE_MSG_PORT_ALL enable tracing on all ports
 *   else, enable tracing on a specific port.
 */

#define EC_CMD_PDC_TRACE_MSG_ENABLE 0x0143

#define EC_PDC_TRACE_MSG_PORT_NONE 0xff
#define EC_PDC_TRACE_MSG_PORT_ALL 0xfe

struct ec_params_pdc_trace_msg_enable {
	uint8_t port;
};

struct ec_response_pdc_trace_msg_enable {
	/* Previous port value. */
	uint8_t port;
	uint8_t reserved;
	/* Number of free bytes in FIFO. */
	uint16_t fifo_free;
	/* Running total of dropped messages (may wrap). */
	uint32_t dropped_count;
} __ec_align4;

/*
 * Fetch multiple PDC trace entries.
 *
 * If no entries are available, pl_size is 0.
 * At most MAX_HC_PDC_TRACE_MSG_GET_PAYLOAD bytes worth of entries
 * are returned. Only whole entries are returned.
 */

#define EC_CMD_PDC_TRACE_MSG_GET_ENTRIES 0x0144
#define MAX_HC_PDC_TRACE_MSG_GET_PAYLOAD 240

struct ec_response_pdc_trace_msg_get_entries {
	/* Total bytes of payload. */
	uint16_t pl_size;
	/* Packed array of pdc_trace_msg_entry structs. */
	uint8_t payload[FLEXIBLE_ARRAY_MEMBER_SIZE];
};

enum pdc_trace_msg_direction {
	PDC_TRACE_MSG_DIR_IN = 0,
	PDC_TRACE_MSG_DIR_OUT = 1,
};

struct pdc_trace_msg_entry {
	/*
	 * Timestamp - least significant 32 bits of EC epoch time
	 * (microseconds, will wrap around).
	 */
	uint32_t time32_us;
	/* Entry sequence number (wraps around). */
	uint16_t seq_num;
	/* Port number associated with this entry. */
	uint8_t port_num;
	/* Direction of message (enum pdc_trace_msg_direction) */
	uint8_t direction;
	/* Format of pdc_data (PDC chip identifier). */
	uint8_t msg_type;
	/* Bytes in pdc_data. */
	uint8_t pdc_data_size;
	/* Captured PDC message. */
	uint8_t pdc_data[FLEXIBLE_ARRAY_MEMBER_SIZE];
} __ec_align1;

/* Enable/disable Ethernet POE power */
#define EC_CMD_SWITCH_ENABLE_POE 0x0145

struct ec_params_switch_enable_poe {
	uint8_t enabled;
} __ec_align1;

/*****************************************************************************/
/* The command range 0x200-0x2FF is reserved for Rotor. */

/*****************************************************************************/
/*
 * Reserve a range of host commands for the CR51 firmware.
 */
#define EC_CMD_CR51_BASE 0x0300
#define EC_CMD_CR51_LAST 0x03FF

/*****************************************************************************/
/* Fingerprint MCU commands: range 0x0400-0x040x */

/*
 * Fingerprint SPI sensor passthru command
 *
 * This command was used for prototyping and it's now deprecated.
 */
#define EC_CMD_FP_PASSTHRU 0x0400

#define EC_FP_FLAG_NOT_COMPLETE 0x1

struct ec_params_fp_passthru {
	uint16_t len; /* Number of bytes to write then read */
	uint16_t flags; /* EC_FP_FLAG_xxx */
	uint8_t data[FLEXIBLE_ARRAY_MEMBER_SIZE]; /* Data to send */
} __ec_align2;

/* Configure the Fingerprint MCU behavior */
#define EC_CMD_FP_MODE 0x0402

/* Put the sensor in its lowest power mode */
#define FP_MODE_DEEPSLEEP BIT(0)
/* Wait to see a finger on the sensor */
#define FP_MODE_FINGER_DOWN BIT(1)
/* Poll until the finger has left the sensor */
#define FP_MODE_FINGER_UP BIT(2)
/* Capture the current finger image */
#define FP_MODE_CAPTURE BIT(3)
/* Finger enrollment session on-going */
#define FP_MODE_ENROLL_SESSION BIT(4)
/* Enroll the current finger image */
#define FP_MODE_ENROLL_IMAGE BIT(5)
/* Try to match the current finger image */
#define FP_MODE_MATCH BIT(6)
/* Reset and re-initialize the sensor. */
#define FP_MODE_RESET_SENSOR BIT(7)
/* Sensor maintenance for dead pixels. */
#define FP_MODE_SENSOR_MAINTENANCE BIT(8)
/* Encrypt template. */
#define FP_MODE_ENCRYPT_TEMPLATE BIT(9)
/* Decrypt template. */
#define FP_MODE_DECRYPT_TEMPLATE BIT(10)
/* Disable template update. */
#define FP_MODE_MATCH_NO_TEMPLATE_UPDATE BIT(11)
/* special value: don't change anything just read back current mode */
#define FP_MODE_DONT_CHANGE BIT(31)

#define FP_VALID_MODES                                                       \
	(FP_MODE_DEEPSLEEP | FP_MODE_FINGER_DOWN | FP_MODE_FINGER_UP |       \
	 FP_MODE_CAPTURE | FP_MODE_ENROLL_SESSION | FP_MODE_ENROLL_IMAGE |   \
	 FP_MODE_MATCH | FP_MODE_RESET_SENSOR | FP_MODE_SENSOR_MAINTENANCE | \
	 FP_MODE_ENCRYPT_TEMPLATE | FP_MODE_DECRYPT_TEMPLATE |               \
	 FP_MODE_MATCH_NO_TEMPLATE_UPDATE | FP_MODE_DONT_CHANGE)

#define FP_MODES_WITH_AUTHENTICATION (FP_MODE_ENROLL_SESSION | FP_MODE_MATCH)
#define FP_MODES_CRYPTO_IN_PROGRESS \
	(FP_MODE_ENCRYPT_TEMPLATE | FP_MODE_DECRYPT_TEMPLATE)
#define FP_MODES_TEMPLATE_OPERATION                                      \
	(FP_MODE_ENROLL_SESSION | FP_MODE_ENROLL_IMAGE | FP_MODE_MATCH | \
	 FP_MODE_RESET_SENSOR | FP_MODE_ENCRYPT_TEMPLATE |               \
	 FP_MODE_DECRYPT_TEMPLATE)

/* Capture types defined in bits [30..26] */
#define FP_MODE_CAPTURE_TYPE_SHIFT 26
#define FP_MODE_CAPTURE_TYPE_MASK (0x1F << FP_MODE_CAPTURE_TYPE_SHIFT)
/**
 * enum fp_capture_type - Specifies the "mode" when capturing images.
 *
 * @FP_CAPTURE_TYPE_INVALID: an invalid capture type
 * @FP_CAPTURE_VENDOR_FORMAT: Capture 1-3 images and choose the best quality
 * image (produces 'frame_size' bytes)
 * @FP_CAPTURE_SIMPLE_IMAGE: Simple raw image capture (produces width x height x
 * bpp bits)
 * @FP_CAPTURE_DEFECT_PXL_TEST: Capture for check defect pixel test
 * @FP_CAPTURE_ABNORMAL_TEST: Capture for check abnormal pixel test
 * @FP_CAPTURE_NOISE_TEST: Capture for check noise test
 * @FP_CAPTURE_PATTERN0: Self test pattern (e.g. checkerboard)
 * @FP_CAPTURE_PATTERN1: Self test pattern (e.g. inverted checkerboard)
 * @FP_CAPTURE_QUALITY_TEST: Capture for Quality test with fixed contrast
 * @FP_CAPTURE_RESET_TEST: Capture for pixel reset value test
 * @FP_CAPTURE_TYPE_MAX: End of enum
 *
 * @note This enum must remain ordered, if you add new values you must ensure
 * that FP_CAPTURE_TYPE_MAX is still the last one.
 */
/* LINT.IfChange */
enum fp_capture_type {
	FP_CAPTURE_TYPE_INVALID = -1,
	FP_CAPTURE_VENDOR_FORMAT = 0,
	FP_CAPTURE_DEFECT_PXL_TEST = 1,
	FP_CAPTURE_ABNORMAL_TEST = 2,
	FP_CAPTURE_NOISE_TEST = 3,
	FP_CAPTURE_SIMPLE_IMAGE = 4,
	FP_CAPTURE_PATTERN0 = 8,
	FP_CAPTURE_PATTERN1 = 12,
	FP_CAPTURE_QUALITY_TEST = 16,
	FP_CAPTURE_RESET_TEST = 20,
	FP_CAPTURE_TYPE_MAX,
};
/* LINT.ThenChange(/test/fpsensor_utils.cc,
 * /zephyr/test/fingerprint/task/src/fpsensor_debug.cc)
 */

/* The maximum number of capture types in enum fp_capture_type */
#define FP_MAX_CAPTURE_TYPES 9

/* Extracts the capture type from the sensor 'mode' word */
#define FP_CAPTURE_TYPE(mode)                                          \
	(enum fp_capture_type)(((mode) & FP_MODE_CAPTURE_TYPE_MASK) >> \
			       FP_MODE_CAPTURE_TYPE_SHIFT)

#define FP_MAC_LENGTH 32

struct ec_params_fp_mode {
	uint32_t mode; /* as defined by FP_MODE_ constants */
} __ec_align4;

struct ec_params_fp_mode_v1 {
	uint32_t mode; /* as defined by FP_MODE_ constants */
	uint8_t mac[FP_MAC_LENGTH];
} __ec_align4;

struct ec_response_fp_mode {
	uint32_t mode; /* as defined by FP_MODE_ constants */
} __ec_align4;

/* Retrieve Fingerprint sensor information */
#define EC_CMD_FP_INFO 0x0403

/* Mask for dead pixels */
#define FP_ERROR_DEAD_PIXELS_MASK 0x3FF
/* Maximum number of dead pixels */
#define FP_ERROR_DEAD_PIXELS_MAX (FP_ERROR_DEAD_PIXELS_MASK - 1)
/* Number of dead pixels detected on the last maintenance */
#define FP_ERROR_DEAD_PIXELS(errors) ((errors) & FP_ERROR_DEAD_PIXELS_MASK)
/* Unknown number of dead pixels detected on the last maintenance */
#define FP_ERROR_DEAD_PIXELS_UNKNOWN (FP_ERROR_DEAD_PIXELS_MASK)
/* No interrupt from the sensor */
#define FP_ERROR_NO_IRQ BIT(12)
/* SPI communication error */
#define FP_ERROR_SPI_COMM BIT(13)
/* Invalid sensor Hardware ID */
#define FP_ERROR_BAD_HWID BIT(14)
/* Sensor initialization failed */
#define FP_ERROR_INIT_FAIL BIT(15)

struct ec_response_fp_info_v0 {
	/* Sensor identification */
	uint32_t vendor_id;
	uint32_t product_id;
	uint32_t model_id;
	uint32_t version;
	/* Image frame characteristics */
	uint32_t frame_size;
	uint32_t pixel_format; /* using V4L2_PIX_FMT_ */
	uint16_t width;
	uint16_t height;
	uint16_t bpp;
	uint16_t errors; /* see FP_ERROR_ flags above */
} __ec_align4;

struct ec_response_fp_info {
	/* Sensor identification */
	uint32_t vendor_id;
	uint32_t product_id;
	uint32_t model_id;
	uint32_t version;
	/* Image frame characteristics */
	uint32_t frame_size;
	uint32_t pixel_format; /* using V4L2_PIX_FMT_ */
	uint16_t width;
	uint16_t height;
	uint16_t bpp;
	uint16_t errors; /* see FP_ERROR_ flags above */
	/* Template/finger current information */
	uint32_t template_size; /* max template size in bytes */
	uint16_t template_max; /* maximum number of fingers/templates */
	uint16_t template_valid; /* number of valid fingers/templates */
	uint32_t template_dirty; /* bitmap of templates with MCU side changes */
	uint32_t template_version; /* version of the template format */
} __ec_align4;

struct fp_sensor_info {
	/* Sensor identification */
	uint32_t vendor_id;
	uint32_t product_id;
	uint32_t model_id;
	uint32_t version;
	uint16_t num_capture_types; /* number of image capture types */
	uint16_t errors; /* see FP_ERROR_ flags above */
} __ec_align4;
BUILD_ASSERT(sizeof(struct fp_sensor_info) == 20);

struct fp_template_info {
	/* Template/finger current information */
	uint32_t template_size; /* max template size in bytes */
	uint16_t template_max; /* maximum number of fingers/templates */
	uint16_t template_valid; /* number of valid fingers/templates */
	uint32_t template_dirty; /* bitmap of templates with MCU side changes */
	uint32_t template_version; /* version of the template format */
} __ec_align4;
BUILD_ASSERT(sizeof(struct fp_template_info) == 16);

struct fp_image_frame_params {
	/* Image frame characteristics */
	uint32_t frame_size;
	uint32_t pixel_format; /* using V4L2_PIX_FMT_ */
	uint16_t width;
	uint16_t height;
	uint16_t bpp;
	/** Type of image capture from enum fp_capture_type. */
	uint8_t fp_capture_type;
	uint8_t reserved; /**< padding for alignment */
} __ec_align4;
BUILD_ASSERT(sizeof(struct fp_image_frame_params) == 16);

struct ec_response_fp_info_v2 {
	/* Sensor identification */
	struct fp_sensor_info sensor_info;
	/* Template/finger current information */
	struct fp_template_info template_info;
	/* fingerprint image frame parameters */
	struct fp_image_frame_params
		image_frame_params[FLEXIBLE_ARRAY_MEMBER_SIZE];
} __ec_align4;
BUILD_ASSERT(sizeof(struct ec_response_fp_info_v2) == 36);

struct fp_image_frame_params_v2 {
	/* Image frame characteristics */
	uint32_t frame_size;
	uint32_t image_data_offset_bytes; /**< Byte offset of image buffer */
	uint32_t pixel_format; /* using V4L2_PIX_FMT_ */
	uint16_t width;
	uint16_t height;
	uint16_t bpp;
	/** Type of image capture from enum fp_capture_type. */
	uint8_t fp_capture_type;
	uint8_t reserved; /**< padding for alignment */
} __ec_align4;
BUILD_ASSERT(sizeof(struct fp_image_frame_params_v2) == 20);

struct ec_response_fp_info_v3 {
	/* Sensor identification */
	struct fp_sensor_info sensor_info;
	/* Template/finger current information */
	struct fp_template_info template_info;
	/* fingerprint image frame parameters */
	struct fp_image_frame_params_v2
		image_frame_params[FLEXIBLE_ARRAY_MEMBER_SIZE];
} __ec_align4;
BUILD_ASSERT(sizeof(struct ec_response_fp_info_v3) == 36);

/* Get the last captured finger frame or a template content */
#define EC_CMD_FP_FRAME 0x0404

/* constants defining the 'offset' field which also contains the frame index */
#define FP_FRAME_INDEX_SHIFT 28
/* Frame buffer where the captured image is stored */
#define FP_FRAME_INDEX_RAW_IMAGE 0
/* First frame buffer holding a template */
#define FP_FRAME_INDEX_TEMPLATE 1
#define FP_FRAME_GET_BUFFER_INDEX(offset) ((offset) >> FP_FRAME_INDEX_SHIFT)
#define FP_FRAME_OFFSET_MASK 0x0FFFFFFF

/* Version of the format of the encrypted templates. */
#define FP_TEMPLATE_FORMAT_VERSION 4

/* Constants for encryption parameters */
#define FP_CONTEXT_NONCE_BYTES 12
#define FP_CONTEXT_USERID_BYTES 32
#define FP_CONTEXT_USERID_WORDS (FP_CONTEXT_USERID_BYTES / sizeof(uint32_t))
#define FP_CONTEXT_TAG_BYTES 16
#define FP_CONTEXT_ENCRYPTION_SALT_BYTES 16
#define FP_CONTEXT_TPM_BYTES 32

/* Constants for positive match parameters. */
#define FP_POSITIVE_MATCH_SALT_BYTES 16

struct ec_fp_template_encryption_metadata {
	/*
	 * Version of the structure format (N=3).
	 */
	uint16_t struct_version;
	/* Reserved bytes, set to 0. */
	uint16_t reserved;
	/*
	 * The salt is *only* ever used for key derivation. The nonce is unique,
	 * a different one is used for every message.
	 */
	uint8_t nonce[FP_CONTEXT_NONCE_BYTES];
	uint8_t encryption_salt[FP_CONTEXT_ENCRYPTION_SALT_BYTES];
	uint8_t tag[FP_CONTEXT_TAG_BYTES];
};

struct ec_params_fp_frame {
	/*
	 * The offset contains the template index or FP_FRAME_INDEX_RAW_IMAGE
	 * in the high nibble, and the real offset within the frame in
	 * FP_FRAME_OFFSET_MASK.
	 */
	uint32_t offset;
	uint32_t size;
} __ec_align4;

/*
 * FP_FRAME commands:
 *
 * - FP_FRAME_GET_RAW_IMAGE command can be used to get raw image from sensor.
 *   This command works only when the system is not locked. The template index
 *   is ignored.
 * - FP_FRAME_ENCRYPT_TEMPLATE command is used to request encryption of the
 *   template with provided template index. Offset and size are ignored.
 *   The encryption process is considered as started only after EC_SUCCESS
 *   was returned.
 * - FP_FRAME_GET_ENCRYPTED_TEMPLATE command is used to obtain the encrypted
 *   template.
 */
enum fp_frame_cmd {
	FP_FRAME_GET_RAW_IMAGE = 0,
	FP_FRAME_ENCRYPT_TEMPLATE = 1,
	FP_FRAME_GET_ENCRYPTED_TEMPLATE = 2,
};

struct ec_params_fp_frame_v1 {
	uint8_t cmd;
	uint8_t reserved;
	uint16_t index;
	uint32_t offset;
	uint32_t size;
} __ec_align4;

/* Load a template into the MCU */
#define EC_CMD_FP_TEMPLATE 0x0405

/* Flag in the 'size' field indicating that the full template has been sent */
#define FP_TEMPLATE_COMMIT 0x80000000

struct ec_params_fp_template {
	uint32_t offset;
	uint32_t size;
	uint8_t data[FLEXIBLE_ARRAY_MEMBER_SIZE];
} __ec_align4;

/*
 * FP_TEMPLATE commands:
 *
 * - FP_TEMPLATE_LOAD command is used to copy part of the template to FPMCU
 *   buffer.
 * - FP_TEMPLATE_DECRYPT command starts template decryption.
 * - FP_TEMPLATE_GET_RESULT command is used to check decryption result.
 */
enum fp_template_cmd {
	FP_TEMPLATE_LOAD = 0,
	FP_TEMPLATE_DECRYPT = 1,
	FP_TEMPLATE_GET_RESULT = 2,
};

struct ec_params_fp_template_v1 {
	uint32_t offset;
	uint32_t size;
	uint8_t cmd;
	uint8_t data[FLEXIBLE_ARRAY_MEMBER_SIZE];
} __ec_align4;

/* Clear the current fingerprint user context and set a new one */
#define EC_CMD_FP_CONTEXT 0x0406

struct ec_params_fp_context {
	uint32_t userid[FP_CONTEXT_USERID_WORDS];
} __ec_align4;

enum fp_context_action {
	FP_CONTEXT_ASYNC = 0,
	FP_CONTEXT_GET_RESULT = 1,
};

/* Version 1 of the command is "asynchronous". */
struct ec_params_fp_context_v1 {
	uint8_t action; /**< enum fp_context_action */
	uint8_t reserved[3]; /**< padding for alignment */
	uint32_t userid[FP_CONTEXT_USERID_WORDS];
} __ec_align4;

#define EC_CMD_FP_STATS 0x0407

#define FPSTATS_CAPTURE_INV BIT(0)
#define FPSTATS_MATCHING_INV BIT(1)

struct ec_response_fp_stats {
	uint32_t capture_time_us;
	uint32_t matching_time_us;
	uint32_t overall_time_us;
	struct {
		uint32_t lo;
		uint32_t hi;
	} overall_t0;
	uint8_t timestamps_invalid;
	int8_t template_matched;
} __ec_align2;

#define EC_CMD_FP_SEED 0x0408
struct ec_params_fp_seed {
	/*
	 * Version of the structure format (N=3).
	 */
	uint16_t struct_version;
	/* Reserved bytes, set to 0. */
	uint16_t reserved;
	/* Seed from the TPM. */
	uint8_t seed[FP_CONTEXT_TPM_BYTES];
} __ec_align4;

#define EC_CMD_FP_ENC_STATUS 0x0409

/* FP TPM seed has been set or not */
#define FP_ENC_STATUS_SEED_SET BIT(0)
/* Session was established or not */
#define FP_CONTEXT_STATUS_SESSION_ESTABLISHED BIT(1)
/* FP session_nonce had been set or not*/
#define FP_CONTEXT_SESSION_NONCE_SET BIT(2)
/* FP user_id had been set or not*/
#define FP_CONTEXT_USER_ID_SET BIT(3)
/* The operation authentication challenge was generated */
#define FP_AUTH_CHALLENGE_SET BIT(4)
/* Encrypted template is available */
#define FP_ENCRYPTED_TEMPLATE_READY BIT(5)

struct ec_response_fp_encryption_status {
	/* Used bits in encryption engine status */
	uint32_t valid_flags;
	/* Encryption engine status */
	uint32_t status;
} __ec_align4;

#define EC_CMD_FP_READ_MATCH_SECRET 0x040A
struct ec_params_fp_read_match_secret {
	uint16_t fgr;
} __ec_align4;

/*
 * Fingerprint vendor defined command.
 *
 * A custom per fingerprint vendor host command. It can be used to fetch some
 * custom data during testing, manufacturing etc.
 *
 * This command should be handled only if the system is unlocked.
 */
#define EC_CMD_FP_VENDOR 0x040B
struct ec_params_fp_vendor {
	/* Parameter to be used by FP vendors. */
	uint32_t param1;
} __ec_align4;

/* The positive match secret has the length of the SHA256 digest. */
#define FP_POSITIVE_MATCH_SECRET_BYTES 32
struct ec_response_fp_read_match_secret {
	uint8_t positive_match_secret[FP_POSITIVE_MATCH_SECRET_BYTES];
} __ec_align4;

#define FP_ELLIPTIC_CURVE_PUBLIC_KEY_POINT_LEN 32

struct fp_elliptic_curve_public_key {
	uint8_t x[FP_ELLIPTIC_CURVE_PUBLIC_KEY_POINT_LEN];
	uint8_t y[FP_ELLIPTIC_CURVE_PUBLIC_KEY_POINT_LEN];
} __ec_align4;

#define FP_AES_KEY_ENC_METADATA_VERSION 1
#define FP_AES_KEY_NONCE_BYTES 12
#define FP_AES_KEY_ENCRYPTION_SALT_BYTES 16
#define FP_AES_KEY_TAG_BYTES 16

struct fp_auth_command_encryption_metadata {
	/* Version of the structure format */
	uint16_t struct_version;
	/* Reserved bytes, set to 0. */
	uint16_t reserved;
	/*
	 * The salt is *only* ever used for key derivation. The nonce is unique,
	 * a different one is used for every message.
	 */
	uint8_t nonce[FP_AES_KEY_NONCE_BYTES];
	uint8_t encryption_salt[FP_AES_KEY_ENCRYPTION_SALT_BYTES];
	uint8_t tag[FP_AES_KEY_TAG_BYTES];
} __ec_align4;

#define FP_ELLIPTIC_CURVE_PRIVATE_KEY_LEN 32
#define FP_ELLIPTIC_CURVE_PUBLIC_KEY_IV_LEN 16

struct fp_encrypted_private_key {
	struct fp_auth_command_encryption_metadata info;
	uint8_t data[FP_ELLIPTIC_CURVE_PRIVATE_KEY_LEN];
} __ec_align4;

#define EC_CMD_FP_ESTABLISH_PAIRING_KEY_KEYGEN 0x0410

struct ec_response_fp_establish_pairing_key_keygen {
	struct fp_elliptic_curve_public_key pubkey;
} __ec_align4;

#define FP_PAIRING_KEY_LEN 32

struct ec_fp_encrypted_pairing_key {
	struct fp_auth_command_encryption_metadata info;
	uint8_t data[FP_PAIRING_KEY_LEN];
} __ec_align4;

#define EC_CMD_FP_ESTABLISH_PAIRING_KEY_WRAP 0x0411

struct ec_params_fp_establish_pairing_key_wrap {
	struct fp_elliptic_curve_public_key peers_pubkey;
} __ec_align4;

struct ec_response_fp_establish_pairing_key_wrap {
	struct ec_fp_encrypted_pairing_key encrypted_pairing_key;
} __ec_align4;

#define EC_CMD_FP_LOAD_PAIRING_KEY 0x0412

typedef struct ec_response_fp_establish_pairing_key_wrap
	ec_params_fp_load_pairing_key;

#define FP_CK_SESSION_NONCE_LEN 32

#define EC_CMD_FP_GENERATE_NONCE 0x0413
struct ec_response_fp_generate_nonce {
	uint8_t nonce[FP_CK_SESSION_NONCE_LEN];
} __ec_align4;

#define EC_CMD_FP_ESTABLISH_SESSION 0x0414
struct ec_params_fp_establish_session {
	uint8_t peer_nonce[FP_CK_SESSION_NONCE_LEN];
	uint8_t enc_tpm_seed[FP_CONTEXT_TPM_BYTES];
	uint8_t nonce[FP_AES_KEY_NONCE_BYTES];
	uint8_t tag[FP_AES_KEY_TAG_BYTES];
} __ec_align4;

#define FP_CHALLENGE_SIZE 32

#define EC_CMD_FP_GENERATE_CHALLENGE 0x0415
struct ec_response_fp_generate_challenge {
	uint8_t challenge[FP_CHALLENGE_SIZE];
} __ec_align4;

#define EC_CMD_FP_CONFIRM_TEMPLATE 0x0416
struct ec_params_fp_confirm_template {
	uint8_t mac[FP_MAC_LENGTH];
} __ec_align4;

#define EC_CMD_FP_SIGN_MATCH 0x0417
struct ec_params_fp_sign_match {
	uint8_t challenge[FP_CHALLENGE_SIZE];
} __ec_align4;

struct ec_response_fp_sign_match {
	uint8_t signature[FP_MAC_LENGTH];
} __ec_align4;

/*
 * Fingerprint ASCP claim command.
 *
 */
#define EC_CMD_FP_ASCP_CLAIM 0x0420

/*
 * ECC public key with no point compression as defined in
 * ANSI X9.62 section 4.3.6 (0x04||x||y), P256v1 curve.
 */
#define FP_ASCP_KEY_SIZE 65
/* ECC signature, P256v1 curve, P1363 encoding (r||s) */
#define FP_ASCP_SIGNATURE_SIZE 64
/* SHA256 */
#define FP_ASCP_HASH_SIZE 32

struct ec_response_fp_ascp_claim {
	/* Model public key. */
	uint8_t pk_m[FP_ASCP_KEY_SIZE];
	/* Model public key signature. */
	uint8_t s_goog[FP_ASCP_SIGNATURE_SIZE];
	/* Device public key. */
	uint8_t pk_d[FP_ASCP_KEY_SIZE];
	/* Device public key signature (signed using model key). */
	uint8_t s_m[FP_ASCP_SIGNATURE_SIZE];
	/* Ephemeral public key used in ECDH procedure. */
	uint8_t pk_f[FP_ASCP_KEY_SIZE];
	/* SHA256 hash of the firmware. */
	uint8_t h_f[FP_ASCP_HASH_SIZE];
	/* Signature of the SHA256( 0xC001 || h_f || pk_f) using device key. */
	uint8_t s_d[FP_ASCP_SIGNATURE_SIZE];
} __ec_align4;

/*
 * Fingerprint ASCP establish command.
 *
 */
#define EC_CMD_FP_ASCP_ESTABLISH 0x0421

struct ec_params_fp_ascp_establish {
	/* TA's ephemeral public key. */
	uint8_t pk_g[FP_ASCP_KEY_SIZE];
} __ec_align4;

/*****************************************************************************/
/* Touchpad MCU commands: range 0x0500-0x05FF */

/* Perform touchpad self test */
#define EC_CMD_TP_SELF_TEST 0x0500

/* Get number of frame types, and the size of each type */
#define EC_CMD_TP_FRAME_INFO 0x0501

struct ec_response_tp_frame_info {
	uint32_t n_frames;
	uint32_t frame_sizes[FLEXIBLE_ARRAY_MEMBER_SIZE];
} __ec_align4;

/* Create a snapshot of current frame readings */
#define EC_CMD_TP_FRAME_SNAPSHOT 0x0502

/* Read the frame */
#define EC_CMD_TP_FRAME_GET 0x0503

struct ec_params_tp_frame_get {
	uint32_t frame_index;
	uint32_t offset;
	uint32_t size;
} __ec_align4;

/*****************************************************************************/
/* EC-EC communication commands: range 0x0600-0x06FF */

#define EC_COMM_TEXT_MAX 8

/*
 * Get battery static information, i.e. information that never changes, or
 * very infrequently.
 */
#define EC_CMD_BATTERY_GET_STATIC 0x0600

/**
 * struct ec_params_battery_static_info - Battery static info parameters
 * @index: Battery index.
 */
struct ec_params_battery_static_info {
	uint8_t index;
} __ec_align_size1;

/**
 * struct ec_response_battery_static_info - Battery static info response
 * @design_capacity: Battery Design Capacity (mAh)
 * @design_voltage: Battery Design Voltage (mV)
 * @manufacturer: Battery Manufacturer String
 * @model: Battery Model Number String
 * @serial: Battery Serial Number String
 * @type: Battery Type String
 * @cycle_count: Battery Cycle Count
 */
struct ec_response_battery_static_info {
	uint16_t design_capacity;
	uint16_t design_voltage;
	char manufacturer[EC_COMM_TEXT_MAX];
	char model[EC_COMM_TEXT_MAX];
	char serial[EC_COMM_TEXT_MAX];
	char type[EC_COMM_TEXT_MAX];
	/* TODO(crbug.com/795991): Consider moving to dynamic structure. */
	uint32_t cycle_count;
} __ec_align4;

/**
 * struct ec_response_battery_static_info_v1 - hostcmd v1 battery static info
 * Equivalent to struct ec_response_battery_static_info, but with longer
 * strings.
 * @design_capacity: battery design capacity (in mAh)
 * @design_voltage: battery design voltage (in mV)
 * @cycle_count: battery cycle count
 * @manufacturer_ext: battery manufacturer string
 * @model_ext: battery model string
 * @serial_ext: battery serial number string
 * @type_ext: battery type string
 */
struct ec_response_battery_static_info_v1 {
	uint16_t design_capacity;
	uint16_t design_voltage;
	uint32_t cycle_count;
	char manufacturer_ext[12];
	char model_ext[12];
	char serial_ext[12];
	char type_ext[12];
} __ec_align4;

/**
 * struct ec_response_battery_static_info_v2 - hostcmd v2 battery static info
 *
 * Equivalent to struct ec_response_battery_static_info, but with strings
 * further lengthened (relative to v1) to accommodate the maximum string length
 * permitted by the Smart Battery Data Specification revision 1.1 and fields
 * renamed to better match that specification.
 *
 * @design_capacity: battery design capacity (in mAh)
 * @design_voltage: battery design voltage (in mV)
 * @cycle_count: battery cycle count
 * @manufacturer: battery manufacturer string
 * @device_name: battery model string
 * @serial: battery serial number string
 * @chemistry: battery type string
 */
struct ec_response_battery_static_info_v2 {
	uint16_t design_capacity;
	uint16_t design_voltage;
	uint32_t cycle_count;
	char manufacturer[SBS_MAX_STR_OBJ_SIZE];
	char device_name[SBS_MAX_STR_OBJ_SIZE];
	char serial[SBS_MAX_STR_OBJ_SIZE];
	char chemistry[SBS_MAX_STR_OBJ_SIZE];
} __ec_align4;

/**
 * struct ec_response_battery_static_info_v3 - hostcmd v3 battery static info
 *
 * Extends struct ec_response_battery_static_info_v2 with
 * manuf_info.
 *
 * @design_capacity: battery design capacity (in mAh)
 * @design_voltage: battery design voltage (in mV)
 * @cycle_count: battery cycle count
 * @manufacturer: battery manufacturer string
 * @device_name: battery model string
 * @serial: battery serial number string
 * @chemistry: battery type string
 * @manuf_info: battery manufacture info string (vendor specific)
 * @manuf_year: battery manufacture year
 * @manuf_month: battery manufacture month
 * @manuf_day: battery manufacture day
 */
struct ec_response_battery_static_info_v3 {
	uint16_t design_capacity;
	uint16_t design_voltage;
	uint32_t cycle_count;
	char manufacturer[SBS_MAX_STR_OBJ_SIZE];
	char device_name[SBS_MAX_STR_OBJ_SIZE];
	char serial[SBS_MAX_STR_OBJ_SIZE];
	char chemistry[SBS_MAX_STR_OBJ_SIZE];
	char manuf_info[SBS_MAX_STR_OBJ_SIZE];
	uint16_t manuf_year;
	uint8_t manuf_month;
	uint8_t manuf_day;
} __ec_align4;

/*
 * Get battery dynamic information, i.e. information that is likely to change
 * every time it is read.
 */
#define EC_CMD_BATTERY_GET_DYNAMIC 0x0601

/**
 * struct ec_params_battery_dynamic_info - Battery dynamic info parameters
 * @index: Battery index.
 */
struct ec_params_battery_dynamic_info {
	uint8_t index;
} __ec_align_size1;

/**
 * struct ec_response_battery_dynamic_info - Battery dynamic info response
 * @actual_voltage: Battery voltage (mV)
 * @actual_current: Battery current (mA); negative=discharging
 * @remaining_capacity: Remaining capacity (mAh)
 * @full_capacity: Capacity (mAh, might change occasionally)
 * @flags: Flags, see EC_BATT_FLAG_*
 * @desired_voltage: Charging voltage desired by battery (mV)
 * @desired_current: Charging current desired by battery (mA)
 */
struct ec_response_battery_dynamic_info {
	int16_t actual_voltage;
	int16_t actual_current;
	int16_t remaining_capacity;
	int16_t full_capacity;
	int16_t flags;
	int16_t desired_voltage;
	int16_t desired_current;
} __ec_align2;

/**
 * struct ec_response_battery_dynamic_info_v1 - Battery dynamic info response
 * (v1)
 * @actual_voltage: Battery voltage (mV)
 * @actual_current: Battery current (mA); negative=discharging
 * @remaining_capacity: Remaining capacity (mAh)
 * @full_capacity: Capacity (mAh, might change occasionally)
 * @flags: Flags, see EC_BATT_FLAG_*
 * @desired_voltage: Charging voltage desired by battery (mV)
 * @desired_current: Charging current desired by battery (mA)
 * @temperature: Battery temperature (dK)
 */
struct ec_response_battery_dynamic_info_v1 {
	int16_t actual_voltage;
	int16_t actual_current;
	int16_t remaining_capacity;
	int16_t full_capacity;
	int16_t flags;
	int16_t desired_voltage;
	int16_t desired_current;
	uint16_t temperature;
} __ec_align2;

/*
 * Control charger chip. Used to control charger chip on the peripheral.
 */
#define EC_CMD_CHARGER_CONTROL 0x0602

/**
 * struct ec_params_charger_control - Charger control parameters
 * @max_current: Charger current (mA). Positive to allow base to draw up to
 *     max_current and (possibly) charge battery, negative to request current
 *     from base (OTG).
 * @otg_voltage: Voltage (mV) to use in OTG mode, ignored if max_current is
 *     >= 0.
 * @allow_charging: Allow base battery charging (only makes sense if
 *     max_current > 0).
 */
struct ec_params_charger_control {
	int16_t max_current;
	uint16_t otg_voltage;
	uint8_t allow_charging;
} __ec_align_size1;

/* Get ACK from the USB-C SS muxes */
#define EC_CMD_USB_PD_MUX_ACK 0x0603

struct ec_params_usb_pd_mux_ack {
	uint8_t port; /* USB-C port number */
} __ec_align1;

/* Get boot time */
#define EC_CMD_GET_BOOT_TIME 0x0604

enum boot_time_param {
	ARAIL = 0,
	RSMRST,
	ESPIRST,
	PLTRST_LOW,
	PLTRST_HIGH,
	EC_CUR_TIME,
	RESET_CNT,
};

struct ec_response_get_boot_time {
	uint64_t timestamp[RESET_CNT];
	uint16_t cnt;
} __ec_align4;

/* Issue AP shutdown */
#define EC_CMD_AP_SHUTDOWN 0x0605

/**
 * Issue AP shutdown using heartbeat wake.
 * The AP calls this to enter the low-power G3 state for off-mode charging.
 * The EC then monitors battery SoC and wakes the AP when discharged by a
 * configured threshold.
 */
#define EC_CMD_ENABLE_OFFMODE_HEARTBEAT 0x0606

/*****************************************************************************/
/*
 * Reserve a range of host commands for board-specific, experimental, or
 * special purpose features. These can be (re)used without updating this file.
 *
 * CAUTION: Don't go nuts with this. Shipping products should document ALL
 * their EC commands for easier development, testing, debugging, and support.
 *
 * All commands MUST be #defined to be 4-digit UPPER CASE hex values
 * (e.g., 0x00AB, not 0xab) for CONFIG_HOSTCMD_SECTION_SORTED to work.
 *
 * In your experimental code, you may want to do something like this:
 *
 *   #define EC_CMD_MAGIC_FOO 0x0000
 *   #define EC_CMD_MAGIC_BAR 0x0001
 *   #define EC_CMD_MAGIC_HEY 0x0002
 *
 *   DECLARE_PRIVATE_HOST_COMMAND(EC_CMD_MAGIC_FOO, magic_foo_handler,
 *      EC_VER_MASK(0);
 *
 *   DECLARE_PRIVATE_HOST_COMMAND(EC_CMD_MAGIC_BAR, magic_bar_handler,
 *      EC_VER_MASK(0);
 *
 *   DECLARE_PRIVATE_HOST_COMMAND(EC_CMD_MAGIC_HEY, magic_hey_handler,
 *      EC_VER_MASK(0);
 */
#define EC_CMD_BOARD_SPECIFIC_BASE 0x3E00
#define EC_CMD_BOARD_SPECIFIC_LAST 0x3FFF

/*
 * Given the private host command offset, calculate the true private host
 * command value.
 */
#define EC_PRIVATE_HOST_COMMAND_VALUE(command) \
	(EC_CMD_BOARD_SPECIFIC_BASE + (command))

/*****************************************************************************/
/*
 * Passthru commands
 *
 * Some platforms have sub-processors chained to each other.  For example.
 *
 *     AP <--> EC <--> PD MCU
 *
 * The top 2 bits of the command number are used to indicate which device the
 * command is intended for.  Device 0 is always the device receiving the
 * command; other device mapping is board-specific.
 *
 * When a device receives a command to be passed to a sub-processor, it passes
 * it on with the device number set back to 0.  This allows the sub-processor
 * to remain blissfully unaware of whether the command originated on the next
 * device up the chain, or was passed through from the AP.
 *
 * In the above example, if the AP wants to send command 0x0002 to the PD MCU,
 *     AP sends command 0x4002 to the EC
 *     EC sends command 0x0002 to the PD MCU
 *     EC forwards PD MCU response back to the AP
 */

/* Offset and max command number for sub-device n */
#define EC_CMD_PASSTHRU_OFFSET(n) (0x4000 * (n))
#define EC_CMD_PASSTHRU_MAX(n) (EC_CMD_PASSTHRU_OFFSET(n) + 0x3fff)

/*****************************************************************************/
/*
 * Deprecated constants. These constants have been renamed for clarity. The
 * meaning and size has not changed. Programs that use the old names should
 * switch to the new names soon, as the old names may not be carried forward
 * forever.
 */
#define EC_HOST_PARAM_SIZE EC_PROTO2_MAX_PARAM_SIZE
#define EC_LPC_ADDR_OLD_PARAM EC_HOST_CMD_REGION1
#define EC_OLD_PARAM_SIZE EC_HOST_CMD_REGION_SIZE

#endif /* !__ACPI__ */

#ifdef __cplusplus
}
#endif

#endif /* __CROS_EC_EC_COMMANDS_H */

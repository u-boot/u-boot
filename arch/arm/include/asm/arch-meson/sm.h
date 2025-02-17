/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2016 - Beniamino Galvani <b.galvani@gmail.com>
 */

#ifndef __MESON_SM_H__
#define __MESON_SM_H__

#include <asm/types.h>

/**
 * meson_sm_read_efuse - read efuse memory into buffer
 *
 * @offset: offset from the start efuse memory
 * @buffer: pointer to buffer
 * @size: number of bytes to read
 * @return: number of bytes read
 */
ssize_t meson_sm_read_efuse(uintptr_t offset, void *buffer, size_t size);

/**
 * meson_sm_write_efuse - write into efuse memory from buffer
 *
 * @offset: offset from the start efuse memory
 * @buffer: pointer to buffer
 * @size: number of bytes to write
 * @return: number of bytes written
 */
ssize_t meson_sm_write_efuse(uintptr_t offset, void *buffer, size_t size);

#define SM_SERIAL_SIZE	12
#define MESON_CPU_ID_SZ	4
#define MESON_CHIP_ID_SZ 16

/**
 * union meson_cpu_id - Amlogic cpu_id.
 * @raw: buffer to hold the cpu_id value as sequential bytes.
 * @val: cpu_id represented as 32 bit value.
 */
union meson_cpu_id {
	u8 raw[MESON_CPU_ID_SZ];
	u32 val;
};

/**
 * struct meson_sm_chip_id - Amlogic chip_id.
 * @cpu_id: cpu_id value, which is distinct from socinfo in that the order of
 *          PACK & MINOR bytes are swapped according to Amlogic chip_id format.
 * @serial: 12 byte unique SoC number, identifying particular die, read
 *          usually from efuse OTP storage. Serial comes in little-endian
 *          order.
 */
struct meson_sm_chip_id {
	union meson_cpu_id cpu_id;
	u8 serial[SM_SERIAL_SIZE];
};

/**
 * meson_sm_get_serial - read chip unique serial (OTP data) into buffer
 *
 * @buffer: pointer to buffer
 * @size: buffer size.
 *
 * Serial is returned in big-endian order.
 *
 * @return: zero on success or -errno on failure
 */
int meson_sm_get_serial(void *buffer, size_t size);

/**
 * meson_sm_get_chip_id - read Amlogic chip_id
 *
 * @chip_id: pointer to buffer capable to hold the struct meson_sm_chip_id
 *
 * Amlogic SoCs support 2 versions of chip_id. Function requests the newest
 * one (v2), but if chip_id v2 is not supported, then secure monitor returns
 * v1. All differences between v1 and v2 versions are handled by this function
 * and chip_id is returned in unified format.
 *
 * chip_id contains serial, which is returned here in little-endian order.
 *
 * @return: 0 on success or -errno on failure
 */
int meson_sm_get_chip_id(struct meson_sm_chip_id *chip_id);

enum {
	REBOOT_REASON_COLD = 0,
	REBOOT_REASON_NORMAL = 1,
	REBOOT_REASON_RECOVERY = 2,
	REBOOT_REASON_UPDATE = 3,
	REBOOT_REASON_FASTBOOT = 4,
	REBOOT_REASON_SUSPEND_OFF = 5,
	REBOOT_REASON_HIBERNATE = 6,
	REBOOT_REASON_BOOTLOADER = 7,
	REBOOT_REASON_SHUTDOWN_REBOOT = 8,
	REBOOT_REASON_RPMBP = 9,
	REBOOT_REASON_CRASH_DUMP = 11,
	REBOOT_REASON_KERNEL_PANIC = 12,
	REBOOT_REASON_WATCHDOG_REBOOT = 13,
};

/**
 * meson_sm_get_reboot_reason - get reboot reason
 */
int meson_sm_get_reboot_reason(void);

#define PWRDM_OFF 0
#define PWRDM_ON 1

/**
 * meson_sm_pwrdm_set - do command at specified power domain.
 *
 * @index: power domain index.
 * @cmd: command index.
 * @return: zero on success or error code on failure.
 */
int meson_sm_pwrdm_set(size_t index, int cmd);

/**
 * meson_sm_pwrdm_off - disable specified power domain.
 *
 * @index: power domain index.
 * @return: zero on success or error code on failure.
 */
#define meson_sm_pwrdm_off(index) \
	meson_sm_pwrdm_set(index, PWRDM_OFF)

/**
 * meson_sm_pwrdm_on - enable specified power domain.
 *
 * @index: power domain index.
 * @return: zero on success or error code on failure.
 */
#define meson_sm_pwrdm_on(index) \
	meson_sm_pwrdm_set(index, PWRDM_ON)

#endif /* __MESON_SM_H__ */

/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2019 Intel Corporation
 */

#ifndef __RSU_H__
#define __RSU_H__

#include <asm/types.h>
#include <linux/bitops.h>
#include <linux/bitfield.h>

/* RSU Error Codes */
#define EINTF		1
#define ECFG		2
#define ESLOTNUM	3
#define EFORMAT		4
#define EERASE		5
#define EPROGRAM	6
#define ECMP		7
#define ESIZE		8
#define ENAME		9
#define EFILEIO		10
#define ECALLBACK	11
#define ELOWLEVEL	12
#define EWRPROT		13
#define EARGS		14
#define ECORRUPTED_CPB	15
#define ECORRUPTED_SPT	16

#define STATE_DCIO_CORRUPTED		0xF004D00F
#define STATE_CPB0_CORRUPTED		0xF004D010
#define STATE_CPB0_CPB1_CORRUPTED	0xF004D011

/* RSU Version Bitmasks */
#define RSU_VERSION_CRT_IDX_MASK	GENMASK(31, 28)
#define RSU_VERSION_ERR_MASK		GENMASK(27, 16)
#define RSU_VERSION_DCMF_MASK		GENMASK(7, 0)
#define RSU_VERSION_ACMF_MASK		GENMASK(15, 8)

/* Macros for extracting RSU version fields */
#define RSU_VERSION_CRT_DCMF_IDX(v)	FIELD_GET(RSU_VERSION_CRT_IDX_MASK, (v))
#define RSU_VERSION_ERROR_SOURCE(v)	FIELD_GET(RSU_VERSION_ERR_MASK, (v))
#define RSU_VERSION_ACMF_VERSION(v)	FIELD_GET(RSU_VERSION_ACMF_MASK, (v))
#define RSU_VERSION_DCMF_VERSION(v)	FIELD_GET(RSU_VERSION_DCMF_MASK, (v))

/* DCMF Version Bitmasks */
#define DCMF_VERSION_MAJOR_MASK		GENMASK(31, 24)
#define DCMF_VERSION_MINOR_MASK		GENMASK(23, 16)
#define DCMF_VERSION_UPDATE_MASK	GENMASK(15, 8)

/* Macros for extracting DCMF version fields */
#define DCMF_VERSION_MAJOR(v)		FIELD_GET(DCMF_VERSION_MAJOR_MASK, (v))
#define DCMF_VERSION_MINOR(v)		FIELD_GET(DCMF_VERSION_MINOR_MASK, (v))
#define DCMF_VERSION_UPDATE(v)		FIELD_GET(DCMF_VERSION_UPDATE_MASK, (v))

/**
 * struct rsu_status_info - firmware status log info structure
 * @current_image:address of image currently running in flash
 * @fail_image: address of failed image in flash
 * @state: the state of RSU system
 * @version: the version number of RSU firmware
 * @error_location: the error offset inside the failed image
 * @error_details: error code
 * @retry_counter: current image retry counter
 *
 * This structure is used to capture firmware status log information
 */
struct rsu_status_info {
	u64 current_image;
	u64 fail_image;
	u32 state;
	u32 version;
	u32 error_location;
	u32 error_details;
	u32 retry_counter;
};

/**
 * struct rsu_slot_info - slot information structure
 * @name: a slot name
 * @offset: a slot offset
 * @size: the size of a slot
 * @priority: the priority of a slot
 *
 * This structure is used to capture the slot information details
 */
struct rsu_slot_info {
	char name[16];
	u64 offset;
	u32 size;
	int priority;
};

/**
 * rsu_init() - initialize flash driver, SPT and CPB data
 * @filename: NULL for qspi
 *
 * Returns: 0 on success, or error code
 */
int rsu_init(char *filename);

/**
 * rsu_exit() - free flash driver, clean SPT and CPB data
 */
void rsu_exit(void);

/**
 * rsu_slot_count() - get the number of slots defined
 *
 * Returns: the number of defined slots
 */
int rsu_slot_count(void);

/**
 * rsu_slot_by_name() - get slot number based on name
 * @name: name of slot
 *
 * Return:slot number on success, or error code
 */
int rsu_slot_by_name(char *name);

/**
 * rsu_slot_get_info() - get the attributes of a slot
 * @slot: slot number
 * @info: pointer to info structure to be filled in
 *
 * Returns: 0 on success, or error code
 */
int rsu_slot_get_info(int slot, struct rsu_slot_info *info);

/**
 * rsu_slot_size() - get the size of a slot
 * @slot: slot number
 *
 * Returns: the size of the slot in bytes, or error code
 */
int rsu_slot_size(int slot);

/**
 * rsu_slot_priority() - get the Decision CMF load priority of a slot
 * @slot: slot number
 *
 * Priority of zero means the slot has no priority and is disabled.
 * The slot with priority of one has the highest priority.
 *
 * Returns: the priority of the slot, or error code
 */
int rsu_slot_priority(int slot);

/**
 * rsu_slot_erase() - erase all data in a slot
 * @slot: slot number
 *
 * Erase all data in a slot to prepare for programming. Remove the slot
 * if it is in the CMF pointer block.
 *
 * Returns: 0 on success, or error code
 */
int rsu_slot_erase(int slot);

/**
 * rsu_slot_program_buf() - program a slot from FPGA buffer data
 * @slot: slot number
 * @buf: pointer to data buffer
 * @size: bytes to read from buffer, in hex value
 *
 * This function is used to program a slot using FPGA config data from a
 * buffer and then enter the slot into CPB.
 *
 * Returns: 0 on success, or error code
 */
int rsu_slot_program_buf(int slot, void *buf, int size);

/**
 * rsu_slot_program_factory_update_buf() - program a slot using factory update
 *                                         data from a buffer
 * @slot: slot number
 * @buf: pointer to data buffer
 * @size: bytes to read from buffer
 *
 * This function is used to program a slot using factory update data from a
 * buffer and then enter the slot into CPB.
 *
 * Returns 0 on success, or error code
 */
int rsu_slot_program_factory_update_buf(int slot, void *buf, int size);

/**
 * rsu_slot_program_buf_raw() - program a slot from raw buffer data
 * @slot: slot number
 * @buf: pointer to data buffer
 * @size: bytes to read from buffer, in hex value
 *
 * This function is used to program a slot using raw data from a buffer,
 * and then enter this slot into CPB.
 *
 * Returns: 0 on success, or error code
 */
int rsu_slot_program_buf_raw(int slot, void *buf, int size);

/**
 * rsu_slot_verify_buf() - verify FPGA config data in a slot against a
 * buffer
 * @slot: slot number
 * @buf: pointer to data buffer
 * @size: bytes to read from buffer, in hex value
 *
 * Returns: 0 on success, or error code
 */
int rsu_slot_verify_buf(int slot, void *buf, int size);

/**
 * rsu_slot_verify_buf_raw() - verify raw data in a slot against a buffer
 * @slot: slot number
 * @buf: pointer to data buffer
 * @size: bytes to read from buffer, in hex value
 *
 * Returns: 0 on success, or error code
 */
int rsu_slot_verify_buf_raw(int slot, void *buf, int size);

/**
 * rsu_slot_enable() - enable the selected slot
 * @slot: slot number
 *
 * Set the selected slot as the highest priority. It will be the first
 * slot to be tried after a power-on reset.
 *
 * Returns: 0 on success, or error code
 */
int rsu_slot_enable(int slot);

/**
 * rsu_slot_disable() - disable the selected slot
 * @slot: slot number
 *
 * Remove the selected slot from the priority scheme, but don't erase the
 * slot data so that it can be re-enabled.
 *
 * Returns: 0 on success, or error code
 */
int rsu_slot_disable(int slot);

/**
 * rsu_slot_load() - load the selected slot
 * @slot: slot number
 *
 * This function is used to request the selected slot to be loaded
 * immediately. On success, after a small delay, the system is rebooted.
 *
 * Returns: 0 on success, or error code
 */
int rsu_slot_load(int slot);

/**
 * rsu_slot_load_factory() - load the factory image
 *
 * This function is used to request the factory image to be loaded
 * immediately. On success, after a small delay, the system is rebooted.
 *
 * Returns: 0 on success, or error code
 */
int rsu_slot_load_factory(void);

/**
 * rsu_slot_rename() - Rename the selected slot.
 * @slot: slot number
 * @name: new name for slot
 *
 * Returns: 0 on success, or error code
 */
int rsu_slot_rename(int slot, char *name);

/**
 * rsu_slot_delete() - Delete the selected slot.
 * @slot: slot number
 *
 * Returns 0 on success, or Error Code
 */
int rsu_slot_delete(int slot);

/**
 * rsu_slot_create() - Create a new slot.
 * @name: slot name
 * @address: slot start address
 * @size: slot size
 *
 * Returns 0 on success, or Error Code
 */
int rsu_slot_create(char *name, u64 address, unsigned int size);

/**
 * rsu_status_log() - Copy firmware status log to info struct
 * @info: pointer to info struct to fill in
 *
 * Return 0 on success, or error code
 */
int rsu_status_log(struct rsu_status_info *info);

/**
 * rsu_notify() - report HPS software execution stage as a 16bit number
 * @stage: software execution stage
 *
 * Returns: 0 on success, or error code
 */
int rsu_notify(int stage);

/**
 * rsu_clear_error_status() - clear errors from the current RSU status log
 *
 * Returns: 0 on success, or error code
 */
int rsu_clear_error_status(void);

/**
 * rsu_reset_retry_counter() - reset the retry counter
 *
 * This function is used to request the retry counter to be set to zero, so that
 * the currently running image may be tried again after the next watchdog
 * timeout.
 *
 * Returns: 0 on success, or error code
 */
int rsu_reset_retry_counter(void);

/**
 * rsu_dcmf_version() - retrieve the decision firmware version
 * @versions: pointer to where the four DCMF versions will be stored
 *
 * This function is used to retrieve the version of each of the four DCMF copies
 * in flash.
 *
 * Returns: 0 on success, or error code
 */
int rsu_dcmf_version(u32 *versions);

/**
 * rsu_max_retry() - retrieve the max_retry parameter
 * @value: pointer to where the max_retry will be stored
 *
 * This function is used to retrieve the max_retry parameter from the decision
 * firmware section in flash
 *
 * Returns: 0 on success, or error code
 */
int rsu_max_retry(u8 *value);

/**
 * rsu_dcmf_status() - retrieve the decision firmware status
 * @status: pointer to where the status will be stored
 *
 * This function is used to determine whether decision firmware copies are
 * corrupted in flash, with the currently used decision firmware being used as
 * reference. The status is an array of 4 values, one for each decision
 * firmware copy. A 0 means the copy is fine, anything else means the copy is
 * corrupted.
 *
 * Returns: 0 on success, or error code
 */
int rsu_dcmf_status(u16 *status);

/**
 * rsu_create_empty_cpb() - create a cpb with header field only
 * This function is used to create a empty configuration pointer block
 * (CPB) with the header field only.
 *
 * Returns: 0 on success, or error code
 */
int rsu_create_empty_cpb(void);

/**
 * rsu_restore_cpb() - restore the cpb from an address
 * @address: the address which cpb is restored from.
 *
 * This function is used to restore a saved CPB from an address.
 *
 * Returns: 0 on success, or error code
 */
int rsu_restore_cpb(u64 address);

/**
 * rsu_save_cpb() - save cpb to the address
 * @address: the address which cpb is saved to.
 *
 * This function is used to save CPB to an address.
 *
 * Returns: 0 on success, or error code
 */
int rsu_save_cpb(u64 address);

/**
 * rsu_restore_spt() - restore the spt from an address
 * @address: the address which spt is restored from.
 *
 * This function is used to restore a saved SPT from an address
 *
 * Returns: 0 on success, or error code
 */
int rsu_restore_spt(u64 address);

/**
 * rsu_save_spt() - save spt to the address
 * @address: the address which spt is saved to.
 *
 * This function is used to save SPT to an address.
 *
 * Returns: 0 on success, or error code
 */
int rsu_save_spt(u64 address);

/**
 * rsu_running_factory() - determine if current running image is factory image
 * @factory: set to non-zero value when running factory image, zero otherwise
 *
 * Returns: 0 on success, or error code
 */
int rsu_running_factory(int *factory);
#endif

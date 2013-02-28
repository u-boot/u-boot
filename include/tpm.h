/*
 * Copyright (c) 2013 The Chromium OS Authors.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __TPM_H
#define __TPM_H

#include <tis.h>

/*
 * Here is a partial implementation of TPM commands.  Please consult TCG Main
 * Specification for definitions of TPM commands.
 */

enum tpm_startup_type {
	TPM_ST_CLEAR		= 0x0001,
	TPM_ST_STATE		= 0x0002,
	TPM_ST_DEACTIVATED	= 0x0003,
};

enum tpm_physical_presence {
	TPM_PHYSICAL_PRESENCE_HW_DISABLE	= 0x0200,
	TPM_PHYSICAL_PRESENCE_CMD_DISABLE	= 0x0100,
	TPM_PHYSICAL_PRESENCE_LIFETIME_LOCK	= 0x0080,
	TPM_PHYSICAL_PRESENCE_HW_ENABLE		= 0x0040,
	TPM_PHYSICAL_PRESENCE_CMD_ENABLE	= 0x0020,
	TPM_PHYSICAL_PRESENCE_NOTPRESENT	= 0x0010,
	TPM_PHYSICAL_PRESENCE_PRESENT		= 0x0008,
	TPM_PHYSICAL_PRESENCE_LOCK		= 0x0004,
};

enum tpm_nv_index {
	TPM_NV_INDEX_LOCK	= 0xffffffff,
	TPM_NV_INDEX_0		= 0x00000000,
	TPM_NV_INDEX_DIR	= 0x10000001,
};

/**
 * Initialize TPM device.  It must be called before any TPM commands.
 *
 * @return 0 on success, non-0 on error.
 */
uint32_t tpm_init(void);

/**
 * Issue a TPM_Startup command.
 *
 * @param mode		TPM startup mode
 * @return return code of the operation
 */
uint32_t tpm_startup(enum tpm_startup_type mode);

/**
 * Issue a TPM_SelfTestFull command.
 *
 * @return return code of the operation
 */
uint32_t tpm_self_test_full(void);

/**
 * Issue a TPM_ContinueSelfTest command.
 *
 * @return return code of the operation
 */
uint32_t tpm_continue_self_test(void);

/**
 * Issue a TPM_NV_DefineSpace command.  The implementation is limited
 * to specify TPM_NV_ATTRIBUTES and size of the area.  The area index
 * could be one of the special value listed in enum tpm_nv_index.
 *
 * @param index		index of the area
 * @param perm		TPM_NV_ATTRIBUTES of the area
 * @param size		size of the area
 * @return return code of the operation
 */
uint32_t tpm_nv_define_space(uint32_t index, uint32_t perm, uint32_t size);

/**
 * Issue a TPM_NV_ReadValue command.  This implementation is limited
 * to read the area from offset 0.  The area index could be one of
 * the special value listed in enum tpm_nv_index.
 *
 * @param index		index of the area
 * @param data		output buffer of the area contents
 * @param count		size of output buffer
 * @return return code of the operation
 */
uint32_t tpm_nv_read_value(uint32_t index, void *data, uint32_t count);

/**
 * Issue a TPM_NV_WriteValue command.  This implementation is limited
 * to write the area from offset 0.  The area index could be one of
 * the special value listed in enum tpm_nv_index.
 *
 * @param index		index of the area
 * @param data		input buffer to be wrote to the area
 * @param length	length of data bytes of input buffer
 * @return return code of the operation
 */
uint32_t tpm_nv_write_value(uint32_t index, const void *data, uint32_t length);

/**
 * Issue a TPM_Extend command.
 *
 * @param index		index of the PCR
 * @param in_digest	160-bit value representing the event to be
 *			recorded
 * @param out_digest	160-bit PCR value after execution of the
 *			command
 * @return return code of the operation
 */
uint32_t tpm_extend(uint32_t index, const void *in_digest, void *out_digest);

/**
 * Issue a TPM_PCRRead command.
 *
 * @param index		index of the PCR
 * @param data		output buffer for contents of the named PCR
 * @param count		size of output buffer
 * @return return code of the operation
 */
uint32_t tpm_pcr_read(uint32_t index, void *data, size_t count);

/**
 * Issue a TSC_PhysicalPresence command.  TPM physical presence flag
 * is bit-wise OR'ed of flags listed in enum tpm_physical_presence.
 *
 * @param presence	TPM physical presence flag
 * @return return code of the operation
 */
uint32_t tpm_tsc_physical_presence(uint16_t presence);

/**
 * Issue a TPM_ReadPubek command.
 *
 * @param data		output buffer for the public endorsement key
 * @param count		size of ouput buffer
 * @return return code of the operation
 */
uint32_t tpm_read_pubek(void *data, size_t count);

/**
 * Issue a TPM_ForceClear command.
 *
 * @return return code of the operation
 */
uint32_t tpm_force_clear(void);

/**
 * Issue a TPM_PhysicalEnable command.
 *
 * @return return code of the operation
 */
uint32_t tpm_physical_enable(void);

/**
 * Issue a TPM_PhysicalDisable command.
 *
 * @return return code of the operation
 */
uint32_t tpm_physical_disable(void);

/**
 * Issue a TPM_PhysicalSetDeactivated command.
 *
 * @param state		boolean state of the deactivated flag
 * @return return code of the operation
 */
uint32_t tpm_physical_set_deactivated(uint8_t state);

/**
 * Issue a TPM_GetCapability command.  This implementation is limited
 * to query sub_cap index that is 4-byte wide.
 *
 * @param cap_area	partition of capabilities
 * @param sub_cap	further definition of capability, which is
 *			limited to be 4-byte wide
 * @param cap		output buffer for capability information
 * @param count		size of ouput buffer
 * @return return code of the operation
 */
uint32_t tpm_get_capability(uint32_t cap_area, uint32_t sub_cap,
		void *cap, size_t count);

#endif /* __TPM_H */

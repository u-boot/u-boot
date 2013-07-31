/*
 * Copyright (c) 2013 The Chromium OS Authors.
 * Coypright (c) 2013 Guntermann & Drunck GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
 * TPM return codes as defined in the TCG Main specification
 * (TPM Main Part 2 Structures; Specification version 1.2)
 */
enum tpm_return_code {
	TPM_BASE	= 0x00000000,
	TPM_NON_FATAL	= 0x00000800,
	TPM_SUCCESS	= TPM_BASE,
	/* TPM-defined fatal error codes */
	TPM_AUTHFAIL			= TPM_BASE +  1,
	TPM_BADINDEX			= TPM_BASE +  2,
	TPM_BAD_PARAMETER		= TPM_BASE +  3,
	TPM_AUDITFAILURE		= TPM_BASE +  4,
	TPM_CLEAR_DISABLED		= TPM_BASE +  5,
	TPM_DEACTIVATED			= TPM_BASE +  6,
	TPM_DISABLED			= TPM_BASE +  7,
	TPM_DISABLED_CMD		= TPM_BASE +  8,
	TPM_FAIL			= TPM_BASE +  9,
	TPM_BAD_ORDINAL			= TPM_BASE + 10,
	TPM_INSTALL_DISABLED		= TPM_BASE + 11,
	TPM_INVALID_KEYHANDLE		= TPM_BASE + 12,
	TPM_KEYNOTFOUND			= TPM_BASE + 13,
	TPM_INAPPROPRIATE_ENC		= TPM_BASE + 14,
	TPM_MIGRATE_FAIL		= TPM_BASE + 15,
	TPM_INVALID_PCR_INFO		= TPM_BASE + 16,
	TPM_NOSPACE			= TPM_BASE + 17,
	TPM_NOSRK			= TPM_BASE + 18,
	TPM_NOTSEALED_BLOB		= TPM_BASE + 19,
	TPM_OWNER_SET			= TPM_BASE + 20,
	TPM_RESOURCES			= TPM_BASE + 21,
	TPM_SHORTRANDOM			= TPM_BASE + 22,
	TPM_SIZE			= TPM_BASE + 23,
	TPM_WRONGPCRVAL			= TPM_BASE + 24,
	TPM_BAD_PARAM_SIZE		= TPM_BASE + 25,
	TPM_SHA_THREAD			= TPM_BASE + 26,
	TPM_SHA_ERROR			= TPM_BASE + 27,
	TPM_FAILEDSELFTEST		= TPM_BASE + 28,
	TPM_AUTH2FAIL			= TPM_BASE + 29,
	TPM_BADTAG			= TPM_BASE + 30,
	TPM_IOERROR			= TPM_BASE + 31,
	TPM_ENCRYPT_ERROR		= TPM_BASE + 32,
	TPM_DECRYPT_ERROR		= TPM_BASE + 33,
	TPM_INVALID_AUTHHANDLE		= TPM_BASE + 34,
	TPM_NO_ENDORSEMENT		= TPM_BASE + 35,
	TPM_INVALID_KEYUSAGE		= TPM_BASE + 36,
	TPM_WRONG_ENTITYTYPE		= TPM_BASE + 37,
	TPM_INVALID_POSTINIT		= TPM_BASE + 38,
	TPM_INAPPROPRIATE_SIG		= TPM_BASE + 39,
	TPM_BAD_KEY_PROPERTY		= TPM_BASE + 40,
	TPM_BAD_MIGRATION		= TPM_BASE + 41,
	TPM_BAD_SCHEME			= TPM_BASE + 42,
	TPM_BAD_DATASIZE		= TPM_BASE + 43,
	TPM_BAD_MODE			= TPM_BASE + 44,
	TPM_BAD_PRESENCE		= TPM_BASE + 45,
	TPM_BAD_VERSION			= TPM_BASE + 46,
	TPM_NO_WRAP_TRANSPORT		= TPM_BASE + 47,
	TPM_AUDITFAIL_UNSUCCESSFUL	= TPM_BASE + 48,
	TPM_AUDITFAIL_SUCCESSFUL	= TPM_BASE + 49,
	TPM_NOTRESETABLE		= TPM_BASE + 50,
	TPM_NOTLOCAL			= TPM_BASE + 51,
	TPM_BAD_TYPE			= TPM_BASE + 52,
	TPM_INVALID_RESOURCE		= TPM_BASE + 53,
	TPM_NOTFIPS			= TPM_BASE + 54,
	TPM_INVALID_FAMILY		= TPM_BASE + 55,
	TPM_NO_NV_PERMISSION		= TPM_BASE + 56,
	TPM_REQUIRES_SIGN		= TPM_BASE + 57,
	TPM_KEY_NOTSUPPORTED		= TPM_BASE + 58,
	TPM_AUTH_CONFLICT		= TPM_BASE + 59,
	TPM_AREA_LOCKED			= TPM_BASE + 60,
	TPM_BAD_LOCALITY		= TPM_BASE + 61,
	TPM_READ_ONLY			= TPM_BASE + 62,
	TPM_PER_NOWRITE			= TPM_BASE + 63,
	TPM_FAMILY_COUNT		= TPM_BASE + 64,
	TPM_WRITE_LOCKED		= TPM_BASE + 65,
	TPM_BAD_ATTRIBUTES		= TPM_BASE + 66,
	TPM_INVALID_STRUCTURE		= TPM_BASE + 67,
	TPM_KEY_OWNER_CONTROL		= TPM_BASE + 68,
	TPM_BAD_COUNTER			= TPM_BASE + 69,
	TPM_NOT_FULLWRITE		= TPM_BASE + 70,
	TPM_CONTEXT_GAP			= TPM_BASE + 71,
	TPM_MAXNVWRITES			= TPM_BASE + 72,
	TPM_NOOPERATOR			= TPM_BASE + 73,
	TPM_RESOURCEMISSING		= TPM_BASE + 74,
	TPM_DELEGATE_LOCK		= TPM_BASE + 75,
	TPM_DELEGATE_FAMILY		= TPM_BASE + 76,
	TPM_DELEGATE_ADMIN		= TPM_BASE + 77,
	TPM_TRANSPORT_NOTEXCLUSIVE	= TPM_BASE + 78,
	TPM_OWNER_CONTROL		= TPM_BASE + 79,
	TPM_DAA_RESOURCES		= TPM_BASE + 80,
	TPM_DAA_INPUT_DATA0		= TPM_BASE + 81,
	TPM_DAA_INPUT_DATA1		= TPM_BASE + 82,
	TPM_DAA_ISSUER_SETTINGS		= TPM_BASE + 83,
	TPM_DAA_TPM_SETTINGS		= TPM_BASE + 84,
	TPM_DAA_STAGE			= TPM_BASE + 85,
	TPM_DAA_ISSUER_VALIDITY		= TPM_BASE + 86,
	TPM_DAA_WRONG_W			= TPM_BASE + 87,
	TPM_BAD_HANDLE			= TPM_BASE + 88,
	TPM_BAD_DELEGATE		= TPM_BASE + 89,
	TPM_BADCONTEXT			= TPM_BASE + 90,
	TPM_TOOMANYCONTEXTS		= TPM_BASE + 91,
	TPM_MA_TICKET_SIGNATURE		= TPM_BASE + 92,
	TPM_MA_DESTINATION		= TPM_BASE + 93,
	TPM_MA_SOURCE			= TPM_BASE + 94,
	TPM_MA_AUTHORITY		= TPM_BASE + 95,
	TPM_PERMANENTEK			= TPM_BASE + 97,
	TPM_BAD_SIGNATURE		= TPM_BASE + 98,
	TPM_NOCONTEXTSPACE		= TPM_BASE + 99,
	/* TPM-defined non-fatal errors */
	TPM_RETRY		= TPM_BASE + TPM_NON_FATAL,
	TPM_NEEDS_SELFTEST	= TPM_BASE + TPM_NON_FATAL + 1,
	TPM_DOING_SELFTEST	= TPM_BASE + TPM_NON_FATAL + 2,
	TPM_DEFEND_LOCK_RUNNING	= TPM_BASE + TPM_NON_FATAL + 3,
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

/**
 * Issue a TPM_FlushSpecific command for a AUTH ressource.
 *
 * @param auth_handle	handle of the auth session
 * @return return code of the operation
 */
uint32_t tpm_terminate_auth_session(uint32_t auth_handle);

/**
 * Issue a TPM_OIAP command to setup an object independant authorization
 * session.
 * Information about the session is stored internally.
 * If there was already an OIAP session active it is terminated and a new
 * session is set up.
 *
 * @param auth_handle	pointer to the (new) auth handle or NULL.
 * @return return code of the operation
 */
uint32_t tpm_oiap(uint32_t *auth_handle);

/**
 * Ends an active OIAP session.
 *
 * @return return code of the operation
 */
uint32_t tpm_end_oiap(void);

/**
 * Issue a TPM_LoadKey2 (Auth1) command using an OIAP session for authenticating
 * the usage of the parent key.
 *
 * @param parent_handle	handle of the parent key.
 * @param key		pointer to the key structure (TPM_KEY or TPM_KEY12).
 * @param key_length	size of the key structure
 * @param parent_key_usage_auth	usage auth for the parent key
 * @param key_handle	pointer to the key handle
 * @return return code of the operation
 */
uint32_t tpm_load_key2_oiap(uint32_t parent_handle,
		const void *key, size_t key_length,
		const void *parent_key_usage_auth,
		uint32_t *key_handle);

/**
 * Issue a TPM_GetPubKey (Auth1) command using an OIAP session for
 * authenticating the usage of the key.
 *
 * @param key_handle	handle of the key
 * @param usage_auth	usage auth for the key
 * @param pubkey	pointer to the pub key buffer; may be NULL if the pubkey
 *			should not be stored.
 * @param pubkey_len	pointer to the pub key buffer len. On entry: the size of
 *			the provided pubkey buffer. On successful exit: the size
 *			of the stored TPM_PUBKEY structure (iff pubkey != NULL).
 * @return return code of the operation
 */
uint32_t tpm_get_pub_key_oiap(uint32_t key_handle, const void *usage_auth,
		void *pubkey, size_t *pubkey_len);

#endif /* __TPM_H */

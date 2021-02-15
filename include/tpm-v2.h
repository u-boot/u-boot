/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Defines APIs and structures that allow software to interact with a
 * TPM2 device
 *
 * Copyright (c) 2020 Linaro
 * Copyright (c) 2018 Bootlin
 *
 * https://trustedcomputinggroup.org/resource/tss-overview-common-structures-specification/
 *
 * Author: Miquel Raynal <miquel.raynal@bootlin.com>
 */

#ifndef __TPM_V2_H
#define __TPM_V2_H

#include <tpm-common.h>

struct udevice;

#define TPM2_DIGEST_LEN		32

#define TPM2_SHA1_DIGEST_SIZE 20
#define TPM2_SHA256_DIGEST_SIZE	32
#define TPM2_SHA384_DIGEST_SIZE	48
#define TPM2_SHA512_DIGEST_SIZE	64
#define TPM2_SM3_256_DIGEST_SIZE 32

#define TPM2_MAX_PCRS 32
#define TPM2_PCR_SELECT_MAX ((TPM2_MAX_PCRS + 7) / 8)
#define TPM2_MAX_CAP_BUFFER 1024
#define TPM2_MAX_TPM_PROPERTIES ((TPM2_MAX_CAP_BUFFER - sizeof(u32) /* TPM2_CAP */ - \
				 sizeof(u32)) / sizeof(struct tpms_tagged_property))

/*
 *  We deviate from this draft of the specification by increasing the value of
 *  TPM2_NUM_PCR_BANKS from 3 to 16 to ensure compatibility with TPM2
 *  implementations that have enabled a larger than typical number of PCR
 *  banks. This larger value for TPM2_NUM_PCR_BANKS is expected to be included
 *  in a future revision of the specification.
 */
#define TPM2_NUM_PCR_BANKS 16

/* Definition of (UINT32) TPM2_CAP Constants */
#define TPM2_CAP_PCRS 0x00000005U
#define TPM2_CAP_TPM_PROPERTIES 0x00000006U

/* Definition of (UINT32) TPM2_PT Constants */
#define TPM2_PT_GROUP			(u32)(0x00000100)
#define TPM2_PT_FIXED			(u32)(TPM2_PT_GROUP * 1)
#define TPM2_PT_MANUFACTURER		(u32)(TPM2_PT_FIXED + 5)
#define TPM2_PT_PCR_COUNT		(u32)(TPM2_PT_FIXED + 18)
#define TPM2_PT_MAX_COMMAND_SIZE	(u32)(TPM2_PT_FIXED + 30)
#define TPM2_PT_MAX_RESPONSE_SIZE	(u32)(TPM2_PT_FIXED + 31)

/* event types */
#define EV_POST_CODE		((u32)0x00000001)
#define EV_NO_ACTION		((u32)0x00000003)
#define EV_SEPARATOR		((u32)0x00000004)
#define EV_S_CRTM_CONTENTS	((u32)0x00000007)
#define EV_S_CRTM_VERSION	((u32)0x00000008)
#define EV_CPU_MICROCODE	((u32)0x00000009)
#define EV_TABLE_OF_DEVICES	((u32)0x0000000B)

/* TPMS_TAGGED_PROPERTY Structure */
struct tpms_tagged_property {
	u32 property;
	u32 value;
} __packed;

/* TPMS_PCR_SELECTION Structure */
struct tpms_pcr_selection {
	u16 hash;
	u8 size_of_select;
	u8 pcr_select[TPM2_PCR_SELECT_MAX];
} __packed;

/* TPML_PCR_SELECTION Structure */
struct tpml_pcr_selection {
	u32 count;
	struct tpms_pcr_selection selection[TPM2_NUM_PCR_BANKS];
} __packed;

/* TPML_TAGGED_TPM_PROPERTY Structure */
struct tpml_tagged_tpm_property {
	u32 count;
	struct tpms_tagged_property tpm_property[TPM2_MAX_TPM_PROPERTIES];
} __packed;

/* TPMU_CAPABILITIES Union */
union tpmu_capabilities {
	/*
	 * Non exhaustive. Only added the structs needed for our
	 * current code
	 */
	struct tpml_pcr_selection assigned_pcr;
	struct tpml_tagged_tpm_property tpm_properties;
} __packed;

/* TPMS_CAPABILITY_DATA Structure */
struct tpms_capability_data {
	u32 capability;
	union tpmu_capabilities data;
} __packed;

/**
 * SHA1 Event Log Entry Format
 *
 * @pcr_index:	PCRIndex event extended to
 * @event_type:	Type of event (see EFI specs)
 * @digest:	Value extended into PCR index
 * @event_size:	Size of event
 * @event:	Event data
 */
struct tcg_pcr_event {
	u32 pcr_index;
	u32 event_type;
	u8 digest[TPM2_SHA1_DIGEST_SIZE];
	u32 event_size;
	u8 event[];
} __packed;

/**
 * Definition of TPMU_HA Union
 */
union tmpu_ha {
	u8 sha1[TPM2_SHA1_DIGEST_SIZE];
	u8 sha256[TPM2_SHA256_DIGEST_SIZE];
	u8 sm3_256[TPM2_SM3_256_DIGEST_SIZE];
	u8 sha384[TPM2_SHA384_DIGEST_SIZE];
	u8 sha512[TPM2_SHA512_DIGEST_SIZE];
} __packed;

/**
 * Definition of TPMT_HA Structure
 *
 * @hash_alg:	Hash algorithm defined in enum tpm2_algorithms
 * @digest:	Digest value for a given algorithm
 */
struct tpmt_ha {
	u16 hash_alg;
	union tmpu_ha digest;
} __packed;

/**
 * Definition of TPML_DIGEST_VALUES Structure
 *
 * @count:	Number of algorithms supported by hardware
 * @digests:	struct for algorithm id and hash value
 */
struct tpml_digest_values {
	u32 count;
	struct tpmt_ha digests[TPM2_NUM_PCR_BANKS];
} __packed;

/**
 * Crypto Agile Log Entry Format
 *
 * @pcr_index:	PCRIndex event extended to
 * @event_type:	Type of event
 * @digests:	List of digestsextended to PCR index
 * @event_size: Size of the event data
 * @event:	Event data
 */
struct tcg_pcr_event2 {
	u32 pcr_index;
	u32 event_type;
	struct tpml_digest_values digests;
	u32 event_size;
	u8 event[];
} __packed;

/**
 * TPM2 Structure Tags for command/response buffers.
 *
 * @TPM2_ST_NO_SESSIONS: the command does not need an authentication.
 * @TPM2_ST_SESSIONS: the command needs an authentication.
 */
enum tpm2_structures {
	TPM2_ST_NO_SESSIONS	= 0x8001,
	TPM2_ST_SESSIONS	= 0x8002,
};

/**
 * TPM2 type of boolean.
 */
enum tpm2_yes_no {
	TPMI_YES		= 1,
	TPMI_NO			= 0,
};

/**
 * TPM2 startup values.
 *
 * @TPM2_SU_CLEAR: reset the internal state.
 * @TPM2_SU_STATE: restore saved state (if any).
 */
enum tpm2_startup_types {
	TPM2_SU_CLEAR		= 0x0000,
	TPM2_SU_STATE		= 0x0001,
};

/**
 * TPM2 permanent handles.
 *
 * @TPM2_RH_OWNER: refers to the 'owner' hierarchy.
 * @TPM2_RS_PW: indicates a password.
 * @TPM2_RH_LOCKOUT: refers to the 'lockout' hierarchy.
 * @TPM2_RH_ENDORSEMENT: refers to the 'endorsement' hierarchy.
 * @TPM2_RH_PLATFORM: refers to the 'platform' hierarchy.
 */
enum tpm2_handles {
	TPM2_RH_OWNER		= 0x40000001,
	TPM2_RS_PW		= 0x40000009,
	TPM2_RH_LOCKOUT		= 0x4000000A,
	TPM2_RH_ENDORSEMENT	= 0x4000000B,
	TPM2_RH_PLATFORM	= 0x4000000C,
};

/**
 * TPM2 command codes used at the beginning of a buffer, gives the command.
 *
 * @TPM2_CC_STARTUP: TPM2_Startup().
 * @TPM2_CC_SELF_TEST: TPM2_SelfTest().
 * @TPM2_CC_CLEAR: TPM2_Clear().
 * @TPM2_CC_CLEARCONTROL: TPM2_ClearControl().
 * @TPM2_CC_HIERCHANGEAUTH: TPM2_HierarchyChangeAuth().
 * @TPM2_CC_PCR_SETAUTHPOL: TPM2_PCR_SetAuthPolicy().
 * @TPM2_CC_DAM_RESET: TPM2_DictionaryAttackLockReset().
 * @TPM2_CC_DAM_PARAMETERS: TPM2_DictionaryAttackParameters().
 * @TPM2_CC_GET_CAPABILITY: TPM2_GetCapibility().
 * @TPM2_CC_GET_RANDOM: TPM2_GetRandom().
 * @TPM2_CC_PCR_READ: TPM2_PCR_Read().
 * @TPM2_CC_PCR_EXTEND: TPM2_PCR_Extend().
 * @TPM2_CC_PCR_SETAUTHVAL: TPM2_PCR_SetAuthValue().
 */
enum tpm2_command_codes {
	TPM2_CC_STARTUP		= 0x0144,
	TPM2_CC_SELF_TEST	= 0x0143,
	TPM2_CC_CLEAR		= 0x0126,
	TPM2_CC_CLEARCONTROL	= 0x0127,
	TPM2_CC_HIERCHANGEAUTH	= 0x0129,
	TPM2_CC_PCR_SETAUTHPOL	= 0x012C,
	TPM2_CC_DAM_RESET	= 0x0139,
	TPM2_CC_DAM_PARAMETERS	= 0x013A,
	TPM2_CC_NV_READ         = 0x014E,
	TPM2_CC_GET_CAPABILITY	= 0x017A,
	TPM2_CC_GET_RANDOM      = 0x017B,
	TPM2_CC_PCR_READ	= 0x017E,
	TPM2_CC_PCR_EXTEND	= 0x0182,
	TPM2_CC_PCR_SETAUTHVAL	= 0x0183,
};

/**
 * TPM2 return codes.
 */
enum tpm2_return_codes {
	TPM2_RC_SUCCESS		= 0x0000,
	TPM2_RC_BAD_TAG		= 0x001E,
	TPM2_RC_FMT1		= 0x0080,
	TPM2_RC_HASH		= TPM2_RC_FMT1 + 0x0003,
	TPM2_RC_VALUE		= TPM2_RC_FMT1 + 0x0004,
	TPM2_RC_SIZE		= TPM2_RC_FMT1 + 0x0015,
	TPM2_RC_BAD_AUTH	= TPM2_RC_FMT1 + 0x0022,
	TPM2_RC_HANDLE		= TPM2_RC_FMT1 + 0x000B,
	TPM2_RC_VER1		= 0x0100,
	TPM2_RC_INITIALIZE	= TPM2_RC_VER1 + 0x0000,
	TPM2_RC_FAILURE		= TPM2_RC_VER1 + 0x0001,
	TPM2_RC_DISABLED	= TPM2_RC_VER1 + 0x0020,
	TPM2_RC_AUTH_MISSING	= TPM2_RC_VER1 + 0x0025,
	TPM2_RC_COMMAND_CODE	= TPM2_RC_VER1 + 0x0043,
	TPM2_RC_AUTHSIZE	= TPM2_RC_VER1 + 0x0044,
	TPM2_RC_AUTH_CONTEXT	= TPM2_RC_VER1 + 0x0045,
	TPM2_RC_NEEDS_TEST	= TPM2_RC_VER1 + 0x0053,
	TPM2_RC_WARN		= 0x0900,
	TPM2_RC_TESTING		= TPM2_RC_WARN + 0x000A,
	TPM2_RC_REFERENCE_H0	= TPM2_RC_WARN + 0x0010,
	TPM2_RC_LOCKOUT		= TPM2_RC_WARN + 0x0021,
};

/**
 * TPM2 algorithms.
 */
enum tpm2_algorithms {
	TPM2_ALG_SHA1		= 0x04,
	TPM2_ALG_XOR		= 0x0A,
	TPM2_ALG_SHA256		= 0x0B,
	TPM2_ALG_SHA384		= 0x0C,
	TPM2_ALG_SHA512		= 0x0D,
	TPM2_ALG_NULL		= 0x10,
	TPM2_ALG_SM3_256	= 0x12,
};

/* NV index attributes */
enum tpm_index_attrs {
	TPMA_NV_PPWRITE		= 1UL << 0,
	TPMA_NV_OWNERWRITE	= 1UL << 1,
	TPMA_NV_AUTHWRITE	= 1UL << 2,
	TPMA_NV_POLICYWRITE	= 1UL << 3,
	TPMA_NV_COUNTER		= 1UL << 4,
	TPMA_NV_BITS		= 1UL << 5,
	TPMA_NV_EXTEND		= 1UL << 6,
	TPMA_NV_POLICY_DELETE	= 1UL << 10,
	TPMA_NV_WRITELOCKED	= 1UL << 11,
	TPMA_NV_WRITEALL	= 1UL << 12,
	TPMA_NV_WRITEDEFINE	= 1UL << 13,
	TPMA_NV_WRITE_STCLEAR	= 1UL << 14,
	TPMA_NV_GLOBALLOCK	= 1UL << 15,
	TPMA_NV_PPREAD		= 1UL << 16,
	TPMA_NV_OWNERREAD	= 1UL << 17,
	TPMA_NV_AUTHREAD	= 1UL << 18,
	TPMA_NV_POLICYREAD	= 1UL << 19,
	TPMA_NV_NO_DA		= 1UL << 25,
	TPMA_NV_ORDERLY		= 1UL << 26,
	TPMA_NV_CLEAR_STCLEAR	= 1UL << 27,
	TPMA_NV_READLOCKED	= 1UL << 28,
	TPMA_NV_WRITTEN		= 1UL << 29,
	TPMA_NV_PLATFORMCREATE	= 1UL << 30,
	TPMA_NV_READ_STCLEAR	= 1UL << 31,

	TPMA_NV_MASK_READ	= TPMA_NV_PPREAD | TPMA_NV_OWNERREAD |
				TPMA_NV_AUTHREAD | TPMA_NV_POLICYREAD,
	TPMA_NV_MASK_WRITE	= TPMA_NV_PPWRITE | TPMA_NV_OWNERWRITE |
					TPMA_NV_AUTHWRITE | TPMA_NV_POLICYWRITE,
};

enum {
	TPM_ACCESS_VALID		= 1 << 7,
	TPM_ACCESS_ACTIVE_LOCALITY	= 1 << 5,
	TPM_ACCESS_REQUEST_PENDING	= 1 << 2,
	TPM_ACCESS_REQUEST_USE		= 1 << 1,
	TPM_ACCESS_ESTABLISHMENT	= 1 << 0,
};

enum {
	TPM_STS_FAMILY_SHIFT		= 26,
	TPM_STS_FAMILY_MASK		= 0x3 << TPM_STS_FAMILY_SHIFT,
	TPM_STS_FAMILY_TPM2		= 1 << TPM_STS_FAMILY_SHIFT,
	TPM_STS_RESE_TESTABLISMENT_BIT	= 1 << 25,
	TPM_STS_COMMAND_CANCEL		= 1 << 24,
	TPM_STS_BURST_COUNT_SHIFT	= 8,
	TPM_STS_BURST_COUNT_MASK	= 0xffff << TPM_STS_BURST_COUNT_SHIFT,
	TPM_STS_VALID			= 1 << 7,
	TPM_STS_COMMAND_READY		= 1 << 6,
	TPM_STS_GO			= 1 << 5,
	TPM_STS_DATA_AVAIL		= 1 << 4,
	TPM_STS_DATA_EXPECT		= 1 << 3,
	TPM_STS_SELF_TEST_DONE		= 1 << 2,
	TPM_STS_RESPONSE_RETRY		= 1 << 1,
};

enum {
	TPM_CMD_COUNT_OFFSET	= 2,
	TPM_CMD_ORDINAL_OFFSET	= 6,
	TPM_MAX_BUF_SIZE	= 1260,
};

/**
 * Issue a TPM2_Startup command.
 *
 * @dev		TPM device
 * @mode	TPM startup mode
 *
 * @return code of the operation
 */
u32 tpm2_startup(struct udevice *dev, enum tpm2_startup_types mode);

/**
 * Issue a TPM2_SelfTest command.
 *
 * @dev		TPM device
 * @full_test	Asking to perform all tests or only the untested ones
 *
 * @return code of the operation
 */
u32 tpm2_self_test(struct udevice *dev, enum tpm2_yes_no full_test);

/**
 * Issue a TPM2_Clear command.
 *
 * @dev		TPM device
 * @handle	Handle
 * @pw		Password
 * @pw_sz	Length of the password
 *
 * @return code of the operation
 */
u32 tpm2_clear(struct udevice *dev, u32 handle, const char *pw,
	       const ssize_t pw_sz);

/**
 * Issue a TPM2_PCR_Extend command.
 *
 * @dev		TPM device
 * @index	Index of the PCR
 * @algorithm	Algorithm used, defined in 'enum tpm2_algorithms'
 * @digest	Value representing the event to be recorded
 * @digest_len  len of the hash
 *
 * @return code of the operation
 */
u32 tpm2_pcr_extend(struct udevice *dev, u32 index, u32 algorithm,
		    const u8 *digest, u32 digest_len);

/**
 * Issue a TPM2_PCR_Read command.
 *
 * @dev		TPM device
 * @idx		Index of the PCR
 * @idx_min_sz	Minimum size in bytes of the pcrSelect array
 * @data	Output buffer for contents of the named PCR
 * @updates	Optional out parameter: number of updates for this PCR
 *
 * @return code of the operation
 */
u32 tpm2_pcr_read(struct udevice *dev, u32 idx, unsigned int idx_min_sz,
		  void *data, unsigned int *updates);

/**
 * Issue a TPM2_GetCapability command.  This implementation is limited
 * to query property index that is 4-byte wide.
 *
 * @dev		TPM device
 * @capability	Partition of capabilities
 * @property	Further definition of capability, limited to be 4 bytes wide
 * @buf		Output buffer for capability information
 * @prop_count	Size of output buffer
 *
 * @return code of the operation
 */
u32 tpm2_get_capability(struct udevice *dev, u32 capability, u32 property,
			void *buf, size_t prop_count);

/**
 * Issue a TPM2_DictionaryAttackLockReset command.
 *
 * @dev		TPM device
 * @pw		Password
 * @pw_sz	Length of the password
 *
 * @return code of the operation
 */
u32 tpm2_dam_reset(struct udevice *dev, const char *pw, const ssize_t pw_sz);

/**
 * Issue a TPM2_DictionaryAttackParameters command.
 *
 * @dev		TPM device
 * @pw		Password
 * @pw_sz	Length of the password
 * @max_tries	Count of authorizations before lockout
 * @recovery_time Time before decrementation of the failure count
 * @lockout_recovery Time to wait after a lockout
 *
 * @return code of the operation
 */
u32 tpm2_dam_parameters(struct udevice *dev, const char *pw,
			const ssize_t pw_sz, unsigned int max_tries,
			unsigned int recovery_time,
			unsigned int lockout_recovery);

/**
 * Issue a TPM2_HierarchyChangeAuth command.
 *
 * @dev		TPM device
 * @handle	Handle
 * @newpw	New password
 * @newpw_sz	Length of the new password
 * @oldpw	Old password
 * @oldpw_sz	Length of the old password
 *
 * @return code of the operation
 */
int tpm2_change_auth(struct udevice *dev, u32 handle, const char *newpw,
		     const ssize_t newpw_sz, const char *oldpw,
		     const ssize_t oldpw_sz);

/**
 * Issue a TPM_PCR_SetAuthPolicy command.
 *
 * @dev		TPM device
 * @pw		Platform password
 * @pw_sz	Length of the password
 * @index	Index of the PCR
 * @digest	New key to access the PCR
 *
 * @return code of the operation
 */
u32 tpm2_pcr_setauthpolicy(struct udevice *dev, const char *pw,
			   const ssize_t pw_sz, u32 index, const char *key);

/**
 * Issue a TPM_PCR_SetAuthValue command.
 *
 * @dev		TPM device
 * @pw		Platform password
 * @pw_sz	Length of the password
 * @index	Index of the PCR
 * @digest	New key to access the PCR
 * @key_sz	Length of the new key
 *
 * @return code of the operation
 */
u32 tpm2_pcr_setauthvalue(struct udevice *dev, const char *pw,
			  const ssize_t pw_sz, u32 index, const char *key,
			  const ssize_t key_sz);

/**
 * Issue a TPM2_GetRandom command.
 *
 * @dev		TPM device
 * @param data		output buffer for the random bytes
 * @param count		size of output buffer
 *
 * @return return code of the operation
 */
u32 tpm2_get_random(struct udevice *dev, void *data, u32 count);

#endif /* __TPM_V2_H */

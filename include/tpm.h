/*
 * Copyright (c) 2013 The Chromium OS Authors.
 * Coypright (c) 2013 Guntermann & Drunck GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __TPM_H
#define __TPM_H

/*
 * Here is a partial implementation of TPM commands.  Please consult TCG Main
 * Specification for definitions of TPM commands.
 */

#define TPM_HEADER_SIZE		10

enum tpm_duration {
	TPM_SHORT = 0,
	TPM_MEDIUM = 1,
	TPM_LONG = 2,
	TPM_UNDEFINED,

	TPM_DURATION_COUNT,
};

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

#define TPM_NV_PER_GLOBALLOCK		(1U << 15)
#define TPM_NV_PER_PPWRITE		(1U << 0)
#define TPM_NV_PER_READ_STCLEAR		(1U << 31)
#define TPM_NV_PER_WRITE_STCLEAR	(1U << 14)

enum {
	TPM_PUBEK_SIZE			= 256,
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

struct tpm_permanent_flags {
	__be16	tag;
	u8	disable;
	u8	ownership;
	u8	deactivated;
	u8	read_pubek;
	u8	disable_owner_clear;
	u8	allow_maintenance;
	u8	physical_presence_lifetime_lock;
	u8	physical_presence_hw_enable;
	u8	physical_presence_cmd_enable;
	u8	cekp_used;
	u8	tpm_post;
	u8	tpm_post_lock;
	u8	fips;
	u8	operator;
	u8	enable_revoke_ek;
	u8	nv_locked;
	u8	read_srk_pub;
	u8	tpm_established;
	u8	maintenance_done;
	u8	disable_full_da_logic_info;
} __packed;

/* Max buffer size supported by our tpm */
#define TPM_DEV_BUFSIZE		1260

/**
 * struct tpm_chip_priv - Information about a TPM, stored by the uclass
 *
 * These values must be set up by the device's probe() method before
 * communcation is attempted. If the device has an xfer() method, this is
 * not needed. There is no need to set up @buf.
 *
 * @duration_ms:	Length of each duration type in milliseconds
 * @retry_time_ms:	Time to wait before retrying receive
 */
struct tpm_chip_priv {
	uint duration_ms[TPM_DURATION_COUNT];
	uint retry_time_ms;
	u8 buf[TPM_DEV_BUFSIZE + sizeof(u8)];  /* Max buffer size + addr */
};

/**
 * struct tpm_ops - low-level TPM operations
 *
 * These are designed to avoid loops and delays in the driver itself. These
 * should be handled in the uclass.
 *
 * In gneral you should implement everything except xfer(). Where you need
 * complete control of the transfer, then xfer() can be provided and will
 * override the other methods.
 *
 * This interface is for low-level TPM access. It does not understand the
 * concept of localities or the various TPM messages. That interface is
 * defined in the functions later on in this file, but they all translate
 * to bytes which are sent and received.
 */
struct tpm_ops {
	/**
	 * open() - Request access to locality 0 for the caller
	 *
	 * After all commands have been completed the caller should call
	 * close().
	 *
	 * @dev:	Device to close
	 * @return 0 ok OK, -ve on error
	 */
	int (*open)(struct udevice *dev);

	/**
	 * close() - Close the current session
	 *
	 * Releasing the locked locality. Returns 0 on success, -ve 1 on
	 * failure (in case lock removal did not succeed).
	 *
	 * @dev:	Device to close
	 * @return 0 ok OK, -ve on error
	 */
	int (*close)(struct udevice *dev);

	/**
	 * get_desc() - Get a text description of the TPM
	 *
	 * @dev:	Device to check
	 * @buf:	Buffer to put the string
	 * @size:	Maximum size of buffer
	 * @return length of string, or -ENOSPC it no space
	 */
	int (*get_desc)(struct udevice *dev, char *buf, int size);

	/**
	 * send() - send data to the TPM
	 *
	 * @dev:	Device to talk to
	 * @sendbuf:	Buffer of the data to send
	 * @send_size:	Size of the data to send
	 *
	 * Returns 0 on success or -ve on failure.
	 */
	int (*send)(struct udevice *dev, const uint8_t *sendbuf,
		    size_t send_size);

	/**
	 * recv() - receive a response from the TPM
	 *
	 * @dev:	Device to talk to
	 * @recvbuf:	Buffer to save the response to
	 * @max_size:	Maximum number of bytes to receive
	 *
	 * Returns number of bytes received on success, -EAGAIN if the TPM
	 * response is not ready, -EINTR if cancelled, or other -ve value on
	 * failure.
	 */
	int (*recv)(struct udevice *dev, uint8_t *recvbuf, size_t max_size);

	/**
	 * cleanup() - clean up after an operation in progress
	 *
	 * This is called if receiving times out. The TPM may need to abort
	 * the current transaction if it did not complete, and make itself
	 * ready for another.
	 *
	 * @dev:	Device to talk to
	 */
	int (*cleanup)(struct udevice *dev);

	/**
	 * xfer() - send data to the TPM and get response
	 *
	 * This method is optional. If it exists it is used in preference
	 * to send(), recv() and cleanup(). It should handle all aspects of
	 * TPM communication for a single transfer.
	 *
	 * @dev:	Device to talk to
	 * @sendbuf:	Buffer of the data to send
	 * @send_size:	Size of the data to send
	 * @recvbuf:	Buffer to save the response to
	 * @recv_size:	Pointer to the size of the response buffer
	 *
	 * Returns 0 on success (and places the number of response bytes at
	 * recv_size) or -ve on failure.
	 */
	int (*xfer)(struct udevice *dev, const uint8_t *sendbuf,
		    size_t send_size, uint8_t *recvbuf, size_t *recv_size);
};

#define tpm_get_ops(dev)        ((struct tpm_ops *)device_get_ops(dev))

/**
 * tpm_open() - Request access to locality 0 for the caller
 *
 * After all commands have been completed the caller is supposed to
 * call tpm_close().
 *
 * Returns 0 on success, -ve on failure.
 */
int tpm_open(struct udevice *dev);

/**
 * tpm_close() - Close the current session
 *
 * Releasing the locked locality. Returns 0 on success, -ve 1 on
 * failure (in case lock removal did not succeed).
 */
int tpm_close(struct udevice *dev);

/**
 * tpm_get_desc() - Get a text description of the TPM
 *
 * @dev:	Device to check
 * @buf:	Buffer to put the string
 * @size:	Maximum size of buffer
 * @return length of string, or -ENOSPC it no space
 */
int tpm_get_desc(struct udevice *dev, char *buf, int size);

/**
 * tpm_xfer() - send data to the TPM and get response
 *
 * This first uses the device's send() method to send the bytes. Then it calls
 * recv() to get the reply. If recv() returns -EAGAIN then it will delay a
 * short time and then call recv() again.
 *
 * Regardless of whether recv() completes successfully, it will then call
 * cleanup() to finish the transaction.
 *
 * Note that the outgoing data is inspected to determine command type
 * (ordinal) and a timeout is used for that command type.
 *
 * @sendbuf - buffer of the data to send
 * @send_size size of the data to send
 * @recvbuf - memory to save the response to
 * @recv_len - pointer to the size of the response buffer
 *
 * Returns 0 on success (and places the number of response bytes at
 * recv_len) or -ve on failure.
 */
int tpm_xfer(struct udevice *dev, const uint8_t *sendbuf, size_t send_size,
	     uint8_t *recvbuf, size_t *recv_size);

/**
 * Initialize TPM device.  It must be called before any TPM commands.
 *
 * @return 0 on success, non-0 on error.
 */
int tpm_init(void);

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

/**
 * Get the TPM permanent flags value
 *
 * @param pflags	Place to put permanent flags
 * @return return code of the operation
 */
uint32_t tpm_get_permanent_flags(struct tpm_permanent_flags *pflags);

/**
 * Get the TPM permissions
 *
 * @param perm		Returns permissions value
 * @return return code of the operation
 */
uint32_t tpm_get_permissions(uint32_t index, uint32_t *perm);

#endif /* __TPM_H */

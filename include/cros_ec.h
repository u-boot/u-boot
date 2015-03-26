/*
 * Chromium OS cros_ec driver
 *
 * Copyright (c) 2012 The Chromium OS Authors.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _CROS_EC_H
#define _CROS_EC_H

#include <linux/compiler.h>
#include <ec_commands.h>
#include <fdtdec.h>
#include <cros_ec_message.h>
#include <asm/gpio.h>

/* Our configuration information */
struct cros_ec_dev {
	struct udevice *dev;		/* Transport device */
	struct gpio_desc ec_int;	/* GPIO used as EC interrupt line */
	int protocol_version;           /* Protocol version to use */
	int optimise_flash_write;	/* Don't write erased flash blocks */

	/*
	 * These two buffers will always be dword-aligned and include enough
	 * space for up to 7 word-alignment bytes also, so we can ensure that
	 * the body of the message is always dword-aligned (64-bit).
	 *
	 * We use this alignment to keep ARM and x86 happy. Probably word
	 * alignment would be OK, there might be a small performance advantage
	 * to using dword.
	 */
	uint8_t din[ALIGN(MSG_BYTES + sizeof(int64_t), sizeof(int64_t))]
		__aligned(sizeof(int64_t));
	uint8_t dout[ALIGN(MSG_BYTES + sizeof(int64_t), sizeof(int64_t))]
		__aligned(sizeof(int64_t));
};

/*
 * Hard-code the number of columns we happen to know we have right now.  It
 * would be more correct to call cros_ec_info() at startup and determine the
 * actual number of keyboard cols from there.
 */
#define CROS_EC_KEYSCAN_COLS 13

/* Information returned by a key scan */
struct mbkp_keyscan {
	uint8_t data[CROS_EC_KEYSCAN_COLS];
};

/* Holds information about the Chrome EC */
struct fdt_cros_ec {
	struct fmap_entry flash;	/* Address and size of EC flash */
	/*
	 * Byte value of erased flash, or -1 if not known. It is normally
	 * 0xff but some flash devices use 0 (e.g. STM32Lxxx)
	 */
	int flash_erase_value;
	struct fmap_entry region[EC_FLASH_REGION_COUNT];
};

/**
 * Read the ID of the CROS-EC device
 *
 * The ID is a string identifying the CROS-EC device.
 *
 * @param dev		CROS-EC device
 * @param id		Place to put the ID
 * @param maxlen	Maximum length of the ID field
 * @return 0 if ok, -1 on error
 */
int cros_ec_read_id(struct cros_ec_dev *dev, char *id, int maxlen);

/**
 * Read a keyboard scan from the CROS-EC device
 *
 * Send a message requesting a keyboard scan and return the result
 *
 * @param dev		CROS-EC device
 * @param scan		Place to put the scan results
 * @return 0 if ok, -1 on error
 */
int cros_ec_scan_keyboard(struct cros_ec_dev *dev, struct mbkp_keyscan *scan);

/**
 * Read which image is currently running on the CROS-EC device.
 *
 * @param dev		CROS-EC device
 * @param image		Destination for image identifier
 * @return 0 if ok, <0 on error
 */
int cros_ec_read_current_image(struct cros_ec_dev *dev,
		enum ec_current_image *image);

/**
 * Read the hash of the CROS-EC device firmware.
 *
 * @param dev		CROS-EC device
 * @param hash		Destination for hash information
 * @return 0 if ok, <0 on error
 */
int cros_ec_read_hash(struct cros_ec_dev *dev,
		struct ec_response_vboot_hash *hash);

/**
 * Send a reboot command to the CROS-EC device.
 *
 * Note that some reboot commands (such as EC_REBOOT_COLD) also reboot the AP.
 *
 * @param dev		CROS-EC device
 * @param cmd		Reboot command
 * @param flags         Flags for reboot command (EC_REBOOT_FLAG_*)
 * @return 0 if ok, <0 on error
 */
int cros_ec_reboot(struct cros_ec_dev *dev, enum ec_reboot_cmd cmd,
		uint8_t flags);

/**
 * Check if the CROS-EC device has an interrupt pending.
 *
 * Read the status of the external interrupt connected to the CROS-EC device.
 * If no external interrupt is configured, this always returns 1.
 *
 * @param dev		CROS-EC device
 * @return 0 if no interrupt is pending
 */
int cros_ec_interrupt_pending(struct cros_ec_dev *dev);

enum {
	CROS_EC_OK,
	CROS_EC_ERR = 1,
	CROS_EC_ERR_FDT_DECODE,
	CROS_EC_ERR_CHECK_VERSION,
	CROS_EC_ERR_READ_ID,
	CROS_EC_ERR_DEV_INIT,
};

/**
 * Initialise the Chromium OS EC driver
 *
 * @param blob		Device tree blob containing setup information
 * @param cros_ecp        Returns pointer to the cros_ec device, or NULL if none
 * @return 0 if we got an cros_ec device and all is well (or no cros_ec is
 *	expected), -ve if we should have an cros_ec device but failed to find
 *	one, or init failed (-CROS_EC_ERR_...).
 */
int cros_ec_init(const void *blob, struct cros_ec_dev **cros_ecp);

/**
 * Read information about the keyboard matrix
 *
 * @param dev		CROS-EC device
 * @param info		Place to put the info structure
 */
int cros_ec_info(struct cros_ec_dev *dev,
		struct ec_response_mkbp_info *info);

/**
 * Read the host event flags
 *
 * @param dev		CROS-EC device
 * @param events_ptr	Destination for event flags.  Not changed on error.
 * @return 0 if ok, <0 on error
 */
int cros_ec_get_host_events(struct cros_ec_dev *dev, uint32_t *events_ptr);

/**
 * Clear the specified host event flags
 *
 * @param dev		CROS-EC device
 * @param events	Event flags to clear
 * @return 0 if ok, <0 on error
 */
int cros_ec_clear_host_events(struct cros_ec_dev *dev, uint32_t events);

/**
 * Get/set flash protection
 *
 * @param dev		CROS-EC device
 * @param set_mask	Mask of flags to set; if 0, just retrieves existing
 *                      protection state without changing it.
 * @param set_flags	New flag values; only bits in set_mask are applied;
 *                      ignored if set_mask=0.
 * @param prot          Destination for updated protection state from EC.
 * @return 0 if ok, <0 on error
 */
int cros_ec_flash_protect(struct cros_ec_dev *dev,
		       uint32_t set_mask, uint32_t set_flags,
		       struct ec_response_flash_protect *resp);


/**
 * Run internal tests on the cros_ec interface.
 *
 * @param dev		CROS-EC device
 * @return 0 if ok, <0 if the test failed
 */
int cros_ec_test(struct cros_ec_dev *dev);

/**
 * Update the EC RW copy.
 *
 * @param dev		CROS-EC device
 * @param image		the content to write
 * @param imafge_size	content length
 * @return 0 if ok, <0 if the test failed
 */
int cros_ec_flash_update_rw(struct cros_ec_dev *dev,
			 const uint8_t  *image, int image_size);

/**
 * Return a pointer to the board's CROS-EC device
 *
 * This should be implemented by board files.
 *
 * @return pointer to CROS-EC device, or NULL if none is available
 */
struct cros_ec_dev *board_get_cros_ec_dev(void);

struct dm_cros_ec_ops {
	int (*check_version)(struct udevice *dev);
	int (*command)(struct udevice *dev, uint8_t cmd, int cmd_version,
		       const uint8_t *dout, int dout_len,
		       uint8_t **dinp, int din_len);
	int (*packet)(struct udevice *dev, int out_bytes, int in_bytes);
};

#define dm_cros_ec_get_ops(dev) \
		((struct dm_cros_ec_ops *)(dev)->driver->ops)

int cros_ec_register(struct udevice *dev);

/**
 * Dump a block of data for a command.
 *
 * @param name	Name for data (e.g. 'in', 'out')
 * @param cmd	Command number associated with data, or -1 for none
 * @param data	Data block to dump
 * @param len	Length of data block to dump
 */
void cros_ec_dump_data(const char *name, int cmd, const uint8_t *data, int len);

/**
 * Calculate a simple 8-bit checksum of a data block
 *
 * @param data	Data block to checksum
 * @param size	Size of data block in bytes
 * @return checksum value (0 to 255)
 */
int cros_ec_calc_checksum(const uint8_t *data, int size);

/**
 * Decode a flash region parameter
 *
 * @param argc	Number of params remaining
 * @param argv	List of remaining parameters
 * @return flash region (EC_FLASH_REGION_...) or -1 on error
 */
int cros_ec_decode_region(int argc, char * const argv[]);

int cros_ec_flash_erase(struct cros_ec_dev *dev, uint32_t offset,
		uint32_t size);

/**
 * Read data from the flash
 *
 * Read an arbitrary amount of data from the EC flash, by repeatedly reading
 * small blocks.
 *
 * The offset starts at 0. You can obtain the region information from
 * cros_ec_flash_offset() to find out where to read for a particular region.
 *
 * @param dev		CROS-EC device
 * @param data		Pointer to data buffer to read into
 * @param offset	Offset within flash to read from
 * @param size		Number of bytes to read
 * @return 0 if ok, -1 on error
 */
int cros_ec_flash_read(struct cros_ec_dev *dev, uint8_t *data, uint32_t offset,
		    uint32_t size);

/**
 * Write data to the flash
 *
 * Write an arbitrary amount of data to the EC flash, by repeatedly writing
 * small blocks.
 *
 * The offset starts at 0. You can obtain the region information from
 * cros_ec_flash_offset() to find out where to write for a particular region.
 *
 * Attempting to write to the region where the EC is currently running from
 * will result in an error.
 *
 * @param dev		CROS-EC device
 * @param data		Pointer to data buffer to write
 * @param offset	Offset within flash to write to.
 * @param size		Number of bytes to write
 * @return 0 if ok, -1 on error
 */
int cros_ec_flash_write(struct cros_ec_dev *dev, const uint8_t *data,
		     uint32_t offset, uint32_t size);

/**
 * Obtain position and size of a flash region
 *
 * @param dev		CROS-EC device
 * @param region	Flash region to query
 * @param offset	Returns offset of flash region in EC flash
 * @param size		Returns size of flash region
 * @return 0 if ok, -1 on error
 */
int cros_ec_flash_offset(struct cros_ec_dev *dev, enum ec_flash_region region,
		      uint32_t *offset, uint32_t *size);

/**
 * Read/write VbNvContext from/to a CROS-EC device.
 *
 * @param dev		CROS-EC device
 * @param block		Buffer of VbNvContext to be read/write
 * @return 0 if ok, -1 on error
 */
int cros_ec_read_vbnvcontext(struct cros_ec_dev *dev, uint8_t *block);
int cros_ec_write_vbnvcontext(struct cros_ec_dev *dev, const uint8_t *block);

/**
 * Read the version information for the EC images
 *
 * @param dev		CROS-EC device
 * @param versionp	This is set to point to the version information
 * @return 0 if ok, -1 on error
 */
int cros_ec_read_version(struct cros_ec_dev *dev,
		       struct ec_response_get_version **versionp);

/**
 * Read the build information for the EC
 *
 * @param dev		CROS-EC device
 * @param versionp	This is set to point to the build string
 * @return 0 if ok, -1 on error
 */
int cros_ec_read_build_info(struct cros_ec_dev *dev, char **strp);

/**
 * Switch on/off a LDO / FET.
 *
 * @param dev		CROS-EC device
 * @param index		index of the LDO/FET to switch
 * @param state		new state of the LDO/FET : EC_LDO_STATE_ON|OFF
 * @return 0 if ok, -1 on error
 */
int cros_ec_set_ldo(struct cros_ec_dev *dev, uint8_t index, uint8_t state);

/**
 * Read back a LDO / FET current state.
 *
 * @param dev		CROS-EC device
 * @param index		index of the LDO/FET to switch
 * @param state		current state of the LDO/FET : EC_LDO_STATE_ON|OFF
 * @return 0 if ok, -1 on error
 */
int cros_ec_get_ldo(struct cros_ec_dev *dev, uint8_t index, uint8_t *state);

/**
 * Get access to the error reported when cros_ec_board_init() was called
 *
 * This permits delayed reporting of the EC error if it failed during
 * early init.
 *
 * @return error (0 if there was no error, -ve if there was an error)
 */
int cros_ec_get_error(void);

/**
 * Returns information from the FDT about the Chrome EC flash
 *
 * @param blob		FDT blob to use
 * @param node		Node offset to read from
 * @param config	Structure to use to return information
 */
int cros_ec_decode_ec_flash(const void *blob, int node,
			    struct fdt_cros_ec *config);

/**
 * Check the current keyboard state, in case recovery mode is requested.
 * This function is for sandbox only.
 *
 * @param ec		CROS-EC device
 */
void cros_ec_check_keyboard(struct cros_ec_dev *dev);

/*
 * Tunnel an I2C transfer to the EC
 *
 * @param dev		CROS-EC device
 * @param chip		Chip address (7-bit I2C address)
 * @param addr		Register address to read/write
 * @param alen		Length of register address in bytes
 * @param buffer	Buffer containing data to read/write
 * @param len		Length of buffer
 * @param is_read	1 if this is a read, 0 if this is a write
 */
int cros_ec_i2c_xfer(struct cros_ec_dev *dev, uchar chip, uint addr,
		     int alen, uchar *buffer, int len, int is_read);

#endif

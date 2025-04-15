/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2017
 * Mario Six,  Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#ifndef __SYSINFO_H__
#define __SYSINFO_H__

#include <linux/errno.h>

struct udevice;

#define SYSINFO_CACHE_LVL_MAX 3

/*
 * This uclass encapsulates hardware methods to gather information about a
 * sysinfo or a specific device such as hard-wired GPIOs on GPIO expanders,
 * read-only data in flash ICs, or similar.
 *
 * The interface offers functions to read the usual standard data types (bool,
 * int, string) from the device, each of which is identified by a static
 * numeric ID (which will usually be defined as a enum in a header file).
 *
 * If for example the sysinfo had a read-only serial number flash IC, we could
 * call
 *
 * ret = sysinfo_detect(dev);
 * if (ret) {
 *	debug("sysinfo device not found.");
 *	return ret;
 * }
 *
 * ret = sysinfo_get_int(dev, ID_SERIAL_NUMBER, &serial);
 * if (ret) {
 *	debug("Error when reading serial number from device.");
 *	return ret;
 * }
 *
 * to read the serial number.
 */

/** enum sysinfo_id - Standard IDs defined by U-Boot */
enum sysinfo_id {
	SYSID_NONE,

	/* BIOS Information (Type 0) */
	SYSID_SM_BIOS_VENDOR,
	SYSID_SM_BIOS_VER,
	SYSID_SM_BIOS_REL_DATE,

	/* System Information (Type 1) */
	SYSID_SM_SYSTEM_MANUFACTURER,
	SYSID_SM_SYSTEM_PRODUCT,
	SYSID_SM_SYSTEM_VERSION,
	SYSID_SM_SYSTEM_SERIAL,
	SYSID_SM_SYSTEM_WAKEUP,
	SYSID_SM_SYSTEM_SKU,
	SYSID_SM_SYSTEM_FAMILY,
	SYSID_SM_SYSTEM_UUID,

	/* Baseboard (or Module) Information (Type 2) */
	SYSID_SM_BASEBOARD_MANUFACTURER,
	SYSID_SM_BASEBOARD_PRODUCT,
	SYSID_SM_BASEBOARD_VERSION,
	SYSID_SM_BASEBOARD_SERIAL,
	SYSID_SM_BASEBOARD_ASSET_TAG,
	SYSID_SM_BASEBOARD_FEATURE,
	SYSID_SM_BASEBOARD_CHASSIS_LOCAT,
	SYSID_SM_BASEBOARD_TYPE,
	SYSID_SM_BASEBOARD_OBJS_NUM,
	SYSID_SM_BASEBOARD_OBJS_HANDLE,

	/* System Enclosure or Chassis (Type 3) */
	SYSID_SM_ENCLOSURE_MANUFACTURER,
	SYSID_SM_ENCLOSURE_VERSION,
	SYSID_SM_ENCLOSURE_SERIAL,
	SYSID_SM_ENCLOSURE_ASSET_TAG,
	SYSID_SM_ENCLOSURE_TYPE,
	SYSID_SM_ENCLOSURE_BOOTUP,
	SYSID_SM_ENCLOSURE_POW,
	SYSID_SM_ENCLOSURE_THERMAL,
	SYSID_SM_ENCLOSURE_SECURITY,
	SYSID_SM_ENCLOSURE_OEM,
	SYSID_SM_ENCLOSURE_HEIGHT,
	SYSID_SM_ENCLOSURE_POWCORE_NUM,
	SYSID_SM_ENCLOSURE_ELEMENT_CNT,
	SYSID_SM_ENCLOSURE_ELEMENT_LEN,
	SYSID_SM_ENCLOSURE_ELEMENTS,
	SYSID_SM_ENCLOSURE_SKU,

	/* Processor Information (Type 4) */
	SYSID_SM_PROCESSOR_SOCKET,
	SYSID_SM_PROCESSOR_TYPE,
	SYSID_SM_PROCESSOR_MANUFACT,
	SYSID_SM_PROCESSOR_ID,
	SYSID_SM_PROCESSOR_VERSION,
	SYSID_SM_PROCESSOR_VOLTAGE,
	SYSID_SM_PROCESSOR_EXT_CLOCK,
	SYSID_SM_PROCESSOR_MAX_SPEED,
	SYSID_SM_PROCESSOR_CUR_SPEED,
	SYSID_SM_PROCESSOR_STATUS,
	SYSID_SM_PROCESSOR_UPGRADE,
	SYSID_SM_PROCESSOR_SN,
	SYSID_SM_PROCESSOR_ASSET_TAG,
	SYSID_SM_PROCESSOR_PN,
	SYSID_SM_PROCESSOR_CORE_CNT,
	SYSID_SM_PROCESSOR_CORE_EN,
	SYSID_SM_PROCESSOR_THREAD_CNT,
	SYSID_SM_PROCESSOR_CHARA,
	SYSID_SM_PROCESSOR_FAMILY,
	SYSID_SM_PROCESSOR_FAMILY2,
	SYSID_SM_PROCESSOR_CORE_CNT2,
	SYSID_SM_PROCESSOR_CORE_EN2,
	SYSID_SM_PROCESSOR_THREAD_CNT2,
	SYSID_SM_PROCESSOR_THREAD_EN,

	/*
	 * Cache Information (Type 7)
	 * Each of the id should reserve space for up to
	 * SYSINFO_CACHE_LVL_MAX levels of cache
	 */
	SYSID_SM_CACHE_LEVEL,
	SYSID_SM_CACHE_HANDLE,
	SYSID_SM_CACHE_INFO_START,
	SYSID_SM_CACHE_SOCKET = SYSID_SM_CACHE_INFO_START,
	SYSID_SM_CACHE_CONFIG =
		SYSID_SM_CACHE_SOCKET + SYSINFO_CACHE_LVL_MAX,
	SYSID_SM_CACHE_MAX_SIZE =
		SYSID_SM_CACHE_CONFIG + SYSINFO_CACHE_LVL_MAX,
	SYSID_SM_CACHE_INST_SIZE =
		SYSID_SM_CACHE_MAX_SIZE + SYSINFO_CACHE_LVL_MAX,
	SYSID_SM_CACHE_SUPSRAM_TYPE =
		SYSID_SM_CACHE_INST_SIZE + SYSINFO_CACHE_LVL_MAX,
	SYSID_SM_CACHE_CURSRAM_TYPE =
		SYSID_SM_CACHE_SUPSRAM_TYPE + SYSINFO_CACHE_LVL_MAX,
	SYSID_SM_CACHE_SPEED =
		SYSID_SM_CACHE_CURSRAM_TYPE + SYSINFO_CACHE_LVL_MAX,
	SYSID_SM_CACHE_ERRCOR_TYPE =
		SYSID_SM_CACHE_SPEED + SYSINFO_CACHE_LVL_MAX,
	SYSID_SM_CACHE_SCACHE_TYPE =
		SYSID_SM_CACHE_ERRCOR_TYPE + SYSINFO_CACHE_LVL_MAX,
	SYSID_SM_CACHE_ASSOC =
		SYSID_SM_CACHE_SCACHE_TYPE + SYSINFO_CACHE_LVL_MAX,
	SYSID_SM_CACHE_MAX_SIZE2 =
		SYSID_SM_CACHE_ASSOC + SYSINFO_CACHE_LVL_MAX,
	SYSID_SM_CACHE_INST_SIZE2 =
		SYSID_SM_CACHE_MAX_SIZE2 + SYSINFO_CACHE_LVL_MAX,
	SYSID_SM_CACHE_INFO_END =
		SYSID_SM_CACHE_INST_SIZE2 + SYSINFO_CACHE_LVL_MAX - 1,

	/* For show_board_info() */
	SYSID_BOARD_MODEL,
	SYSID_BOARD_MANUFACTURER,
	SYSID_BOARD_MAC_ADDR,
	SYSID_BOARD_RAM_SIZE_MB,
	SYSID_PRIOR_STAGE_VERSION,
	SYSID_PRIOR_STAGE_DATE,

	/* First value available for downstream/board used */
	SYSID_USER = 0x1000,
};

struct sysinfo_ops {
	/**
	 * detect() - Run the hardware info detection procedure for this
	 *	      device.
	 * @dev:      The device containing the information
	 *
	 * This operation might take a long time (e.g. read from EEPROM,
	 * check the presence of a device on a bus etc.), hence this is not
	 * done in the probe() method, but later during operation in this
	 * dedicated method. This method will be called before any other
	 * methods.
	 *
	 * Return: 0 if OK, -ve on error.
	 */
	int (*detect)(struct udevice *dev);

	/**
	 * get_bool() - Read a specific bool data value that describes the
	 *		hardware setup.
	 * @dev:	The sysinfo instance to gather the data.
	 * @id:		A unique identifier for the bool value to be read.
	 * @val:	Pointer to a buffer that receives the value read.
	 *
	 * Return: 0 if OK, -ve on error.
	 */
	int (*get_bool)(struct udevice *dev, int id, bool *val);

	/**
	 * get_int() - Read a specific int data value that describes the
	 *	       hardware setup.
	 * @dev:       The sysinfo instance to gather the data.
	 * @id:        A unique identifier for the int value to be read.
	 * @val:       Pointer to a buffer that receives the value read.
	 *
	 * Return: 0 if OK, -ve on error.
	 */
	int (*get_int)(struct udevice *dev, int id, int *val);

	/**
	 * get_str() - Read a specific string data value that describes the
	 *	       hardware setup.
	 * @dev:	The sysinfo instance to gather the data.
	 * @id:		A unique identifier for the string value to be read.
	 * @size:	The size of the buffer to receive the string data.
	 * @val:	Pointer to a buffer that receives the value read.
	 *
	 * Return: 0 if OK, -ve on error.
	 */
	int (*get_str)(struct udevice *dev, int id, size_t size, char *val);

	/**
	 * get_data() - Read a specific string data value that describes the
	 *	       hardware setup.
	 * @dev:	The sysinfo instance to gather the data.
	 * @id:		A unique identifier for the data area to be get.
	 * @data:	Pointer to the address of the data area.
	 * @size:	Pointer to the size of the data area.
	 *
	 * Return: 0 if OK, -ve on error.
	 */
	int (*get_data)(struct udevice *dev, int id, void **data, size_t *size);

	/**
	 * get_item_count() - Get the item count of the specific data area that
	 *		describes the hardware setup.
	 * @dev:	The sysinfo instance to gather the data.
	 * @id:		A unique identifier for the data area to be get.
	 *
	 * Return: non-negative item count if OK, -ve on error.
	 */
	int (*get_item_count)(struct udevice *dev, int id);

	/**
	 * get_data_by_index() - Get a data value by index from the platform.
	 *
	 * @dev:	The sysinfo instance to gather the data.
	 * @id:		A unique identifier for the data area to be get.
	 * @index:	The item index, starting from 0.
	 * @data:	Pointer to the address of the data area.
	 * @size:	Pointer to the size of the data area.
	 *
	 * Return: 0 if OK, -ve on error.
	 */
	int (*get_data_by_index)(struct udevice *dev, int id, int index,
				 void **data, size_t *size);

	/**
	 * get_fit_loadable - Get the name of an image to load from FIT
	 * This function can be used to provide the image names based on runtime
	 * detection. A classic use-case would when DTBOs are used to describe
	 * additional daughter cards.
	 *
	 * @dev:	The sysinfo instance to gather the data.
	 * @index:	Index of the image. Starts at 0 and gets incremented
	 *		after each call to this function.
	 * @type:	The type of image. For example, "fdt" for DTBs
	 * @strp:	A pointer to string. Untouched if the function fails
	 *
	 * Return: 0 if OK, -ENOENT if no loadable is available else -ve on
	 * error.
	 */
	int (*get_fit_loadable)(struct udevice *dev, int index,
				const char *type, const char **strp);
};

#define sysinfo_get_ops(dev)	((struct sysinfo_ops *)(dev)->driver->ops)

#if CONFIG_IS_ENABLED(SYSINFO)
/**
 * sysinfo_detect() - Run the hardware info detection procedure for this device.
 *
 * @dev:	The device containing the information
 *
 * This function must be called before any other accessor function for this
 * device.
 *
 * Return: 0 if OK, -ve on error.
 */
int sysinfo_detect(struct udevice *dev);

/**
 * sysinfo_get_bool() - Read a specific bool data value that describes the
 *		      hardware setup.
 * @dev:	The sysinfo instance to gather the data.
 * @id:		A unique identifier for the bool value to be read.
 * @val:	Pointer to a buffer that receives the value read.
 *
 * Return: 0 if OK, -EPERM if called before sysinfo_detect(), else -ve on
 * error.
 */
int sysinfo_get_bool(struct udevice *dev, int id, bool *val);

/**
 * sysinfo_get_int() - Read a specific int data value that describes the
 *		     hardware setup.
 * @dev:	The sysinfo instance to gather the data.
 * @id:		A unique identifier for the int value to be read.
 * @val:	Pointer to a buffer that receives the value read.
 *
 * Return: 0 if OK, -EPERM if called before sysinfo_detect(), else -ve on
 * error.
 */
int sysinfo_get_int(struct udevice *dev, int id, int *val);

/**
 * sysinfo_get_str() - Read a specific string data value that describes the
 *		     hardware setup.
 * @dev:	The sysinfo instance to gather the data.
 * @id:		A unique identifier for the string value to be read.
 * @size:	The size of the buffer to receive the string data.
 * @val:	Pointer to a buffer that receives the value read.
 *
 * Return: 0 if OK, -EPERM if called before sysinfo_detect(), else -ve on
 * error.
 */
int sysinfo_get_str(struct udevice *dev, int id, size_t size, char *val);

/**
 * sysinfo_get_data() - Get a data area from the platform.
 * @dev:	The sysinfo instance to gather the data.
 * @id:		A unique identifier for the data area to be get.
 * @data:	Pointer to the address of the data area.
 * @size:	Pointer to the size of the data area.
 *
 * Return: 0 if OK, -EPERM if called before sysinfo_detect(), else -ve on
 * error.
 */
int sysinfo_get_data(struct udevice *dev, int id, void **data, size_t *size);

/**
 * sysinfo_get_item_count() - Get the item count of the specific data area that
 *			    describes the hardware setup.
 * @dev:	The sysinfo instance to gather the data.
 * @id:		A unique identifier for the data area to be get.
 *
 * Return: non-negative item count if OK, -EPERM if called before
 * sysinfo_detect(), else -ve on error.
 */
int sysinfo_get_item_count(struct udevice *dev, int id);

/**
 * sysinfo_get_data_by_index() - Get a data value by index from the platform.
 *
 * @dev:	The sysinfo instance to gather the data.
 * @id:		A unique identifier for the data area to be get.
 * @index:	The item index, starting from 0.
 * @data:	Pointer to the address of the data area.
 * @size:	Pointer to the size of the data area.
 *
 * Return: 0 if OK, -EPERM if called before sysinfo_detect(), else -ve on
 * error.
 */
int sysinfo_get_data_by_index(struct udevice *dev, int id, int index,
			      void **data, size_t *size);

/**
 * sysinfo_get() - Return the sysinfo device for the sysinfo in question.
 * @devp: Pointer to structure to receive the sysinfo device.
 *
 * Since there can only be at most one sysinfo instance, the API can supply a
 * function that returns the unique device. This is especially useful for use
 * in sysinfo files.
 *
 * Return: 0 if OK, -EPERM if called before sysinfo_detect(), else -ve on
 * error.
 */
int sysinfo_get(struct udevice **devp);

/**
 * sysinfo_get_fit_loadable - Get the name of an image to load from FIT
 * This function can be used to provide the image names based on runtime
 * detection. A classic use-case would when DTBOs are used to describe
 * additional daughter cards.
 *
 * @dev:	The sysinfo instance to gather the data.
 * @index:	Index of the image. Starts at 0 and gets incremented
 *		after each call to this function.
 * @type:	The type of image. For example, "fdt" for DTBs
 * @strp:	A pointer to string. Untouched if the function fails
 *
 *
 * Return: 0 if OK, -EPERM if called before sysinfo_detect(), -ENOENT if no
 * loadable is available else -ve on error.
 */
int sysinfo_get_fit_loadable(struct udevice *dev, int index, const char *type,
			     const char **strp);

#else

static inline int sysinfo_detect(struct udevice *dev)
{
	return -ENOSYS;
}

static inline int sysinfo_get_bool(struct udevice *dev, int id, bool *val)
{
	return -ENOSYS;
}

static inline int sysinfo_get_int(struct udevice *dev, int id, int *val)
{
	return -ENOSYS;
}

static inline int sysinfo_get_str(struct udevice *dev, int id, size_t size,
				  char *val)
{
	return -ENOSYS;
}

static inline int sysinfo_get_data(struct udevice *dev, int id, void **data,
				   size_t *size)
{
	return -ENOSYS;
}

static inline int sysinfo_get_item_count(struct udevice *dev, int id)
{
	return -ENOSYS;
}

static inline int sysinfo_get_data_by_index(struct udevice *dev, int id,
					    int index, void **data,
					    size_t *size)
{
	return -ENOSYS;
}

static inline int sysinfo_get(struct udevice **devp)
{
	return -ENOSYS;
}

static inline int sysinfo_get_fit_loadable(struct udevice *dev, int index,
					   const char *type, const char **strp)
{
	return -ENOSYS;
}

#endif
#endif

/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2017
 * Mario Six,  Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#ifndef __SYSINFO_H__
#define __SYSINFO_H__

struct udevice;

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
	SYSINFO_ID_NONE,

	/* For SMBIOS tables */
	SYSINFO_ID_SMBIOS_SYSTEM_VERSION,
	SYSINFO_ID_SMBIOS_BASEBOARD_VERSION,

	/* For show_board_info() */
	SYSINFO_ID_BOARD_MODEL,

	/* First value available for downstream/board used */
	SYSINFO_ID_USER = 0x1000,
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
	 *		If the buffer is not large enough to contain the whole
	 *		string, the string must be trimmed to fit in the
	 *		buffer including the terminating NULL-byte.
	 * @val:	Pointer to a buffer that receives the value read.
	 *
	 * Return: Actual length of the string excluding the terminating
	 *	   NULL-byte if OK, -ve on error.
	 */

	int (*get_str)(struct udevice *dev, int id, size_t size, char *val);

	/**
	 * get_str_list() - Read a specific string data value from a string list
	 *		    that describes hardware setup.
	 * @dev:	The sysinfo instance to gather the data.
	 * @id:		A unique identifier for the string list to read from.
	 * @idx:	The index of the string in the string list.
	 * @size:	The size of the buffer to receive the string data.
	 *		If the buffer is not large enough to contain the whole
	 *		string, the string must be trimmed to fit in the
	 *		buffer including the terminating NULL-byte.
	 * @val:	Pointer to a buffer that receives the value read.
	 *
	 * Return: Actual length of the string excluding the terminating
	 *	   NULL-byte if OK, -ENOENT if list with ID @id does not exist,
	 *	   -ERANGE if @idx is invalid or -ve on error.
	 */
	int (*get_str_list)(struct udevice *dev, int id, unsigned idx,
			    size_t size, char *val);

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
 * @size:	The size of the buffer to receive the string data. If the buffer
 *		is not large enough to contain the whole string, the string will
 *		be trimmed to fit in the buffer including the terminating
 *		NULL-byte.
 * @val:	Pointer to a buffer that receives the value read.
 *
 * Return: Actual length of the string excluding the terminating NULL-byte if
 *	   OK, -EPERM if called before sysinfo_detect(), else -ve on error.
 */
int sysinfo_get_str(struct udevice *dev, int id, size_t size, char *val);

/**
 * sysinfo_get_str_list() - Read a specific string data value from a string list
 *			    that describes hardware setup.
 * @dev:	The sysinfo instance to gather the data.
 * @id:		A unique identifier for the string list to read from.
 * @idx:	The index of the string in the string list.
 * @size:	The size of the buffer to receive the string data. If the buffer
 *		is not large enough to contain the whole string, the string will
 *		be trimmed to fit in the buffer including the terminating
 *		NULL-byte.
 * @val:	Pointer to a buffer that receives the value read.
 *
 * Return: Actual length of the string excluding the terminating NULL-byte if
 *	   OK, -ENOENT if list with ID @id does not exist, -ERANGE if @idx is
 *	   invalid, -EPERM if called before sysinfo_detect(), else -ve on error.
 */
int sysinfo_get_str_list(struct udevice *dev, int id, unsigned idx, size_t size,
			 char *val);

/**
 * sysinfo_get_str_list_max_len() - Get length of longest string in a string
 *				    list that describes hardware setup.
 * @dev:	The sysinfo instance to gather the data.
 * @id:		A unique identifier for the string list to read from.
 *
 * Return: Length (excluding the terminating NULL-byte) of the longest string in
 *	   the string list, or -ve on error.
 */
int sysinfo_get_str_list_max_len(struct udevice *dev, int id);

/**
 * sysinfo_str_list_first() - Start iterating a string list.
 * @dev:	The sysinfo instance to gather the data.
 * @id:		A unique identifier for the string list to read from.
 * @_iter:	Pointer where iterator data will be stored.
 *
 * Pass a reference to a void * pointer as @_iter, i.e.
 *	void *iter;
 *	first = sysinfo_str_list_first(dev, id, &iter);
 *
 * The function will allocate space for the value. You need to iterate all
 * elements with sysinfo_str_list_next() for the space to be freed, or free
 * the pointer stored in @_iter, i.e.
 *	void *iter;
 *	first = sysinfo_str_list_first(dev, id, &iter);
 *	if (first)
 *		free(iter);
 *
 * Return: First string in the string list, or NULL on error.
 */
char *sysinfo_str_list_first(struct udevice *dev, int id, void *_iter);

/**
 * sysinfo_str_list_next() - Get next string in the string string list.
 * @_iter:	Pointer to iterator, filled in by sysinfo_str_list_first().
 *
 * Pass a reference to a void * pointer as @_iter, i.e.
 *	void *iter;
 *	first = sysinfo_str_list_first(dev, id, &iter);
 *	next = sysinfo_str_list_next(&iter);
 *
 * All elements must be iterated until the function returns NULL for the
 * resources allocated for the iteration to be freed, or pointer stored in
 * @_iter must be freed, i.e.:
 *	void *iter;
 *	first = sysinfo_str_list_first(dev, id, &iter);
 *	next = sysinfo_str_list_next(&iter);
 *	if (next)
 *		free(iter);
 *
 * Return: Next string in the string list, NULL on end of list or NULL on error.
 */
char *sysinfo_str_list_next(void *_iter);

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

static inline int sysinfo_get_str_list(struct udevice *dev, int id,
				       unsigned idx, size_t size, char *val)
{
	return -ENOSYS;
}

static inline int sysinfo_get_str_list_max_len(struct udevice *dev, int id)
{
	return -ENOSYS;
}

static inline char *sysinfo_str_list_first(struct udevice *dev, int id,
					   void *_iter)
{
	return NULL;
}

static inline char *sysinfo_str_list_next(void *_iter)
{
	return NULL;
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

/**
 * for_each_sysinfo_str_list - Iterate a sysinfo string list
 * @dev:	The sysinfo instance to gather the data.
 * @id:		A unique identifier for the string list to read from.
 * @val:	String pointer for the value.
 * @iter:	Pointer where iteration data are stored.
 *
 * Important: all elements of the list must be iterated for the iterator
 * resources to be freed automatically. If you need to break from the for cycle,
 * you need to free the iterator.
 *
 * Example:
 *	char *value;
 *	void *iter;
 *	for_each_sysinfo_str_list(dev, MY_STR_LIST, value, iter) {
 *		printf("Value: %s\n", value);
 *		if (!strcmp(value, "needle")) {
 *			free(iter);
 *			break;
 *		}
 *	}
 */
#define for_each_sysinfo_str_list(dev, id, val, iter)			\
	for ((val) = sysinfo_str_list_first((dev), (id), &(iter));	\
	     (val);							\
	     (val) = sysinfo_str_list_next(&(iter)))

#endif

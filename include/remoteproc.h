/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015
 * Texas Instruments Incorporated - http://www.ti.com/
 */

#ifndef _RPROC_H_
#define _RPROC_H_

/*
 * Note: The platform data support is not meant for use with newer
 * platforms. This is meant only for legacy devices. This mode of
 * initialization *will* be eventually removed once all necessary
 * platforms have moved to dm/fdt.
 */
#include <dm/platdata.h>	/* For platform data support - non dt world */

/**
 * enum rproc_mem_type - What type of memory model does the rproc use
 * @RPROC_INTERNAL_MEMORY_MAPPED: Remote processor uses own memory and is memory
 *	mapped to the host processor over an address range.
 *
 * Please note that this is an enumeration of memory model of different types
 * of remote processors. Few of the remote processors do have own internal
 * memories, while others use external memory for instruction and data.
 */
enum rproc_mem_type {
	RPROC_INTERNAL_MEMORY_MAPPED	= 0,
};

/**
 * struct dm_rproc_uclass_pdata - platform data for a CPU
 * @name: Platform-specific way of naming the Remote proc
 * @mem_type: one of 'enum rproc_mem_type'
 * @driver_plat_data: driver specific platform data that may be needed.
 *
 * This can be accessed with dev_get_uclass_plat() for any UCLASS_REMOTEPROC
 * device.
 *
 */
struct dm_rproc_uclass_pdata {
	const char *name;
	enum rproc_mem_type mem_type;
	void *driver_plat_data;
};

/**
 * struct dm_rproc_ops - Driver model remote proc operations.
 *
 * This defines the operations provided by remote proc driver.
 */
struct dm_rproc_ops {
	/**
	 * init() - Initialize the remoteproc device (optional)
	 *
	 * This is called after the probe is completed allowing the remote
	 * processor drivers to split up the initializations between probe and
	 * init if needed.
	 *
	 * @dev:	Remote proc device
	 * @return 0 if all ok, else appropriate error value.
	 */
	int (*init)(struct udevice *dev);

	/**
	 * load() - Load the remoteproc device using data provided (mandatory)
	 *
	 * Load the remoteproc device with an image, do not start the device.
	 *
	 * @dev:	Remote proc device
	 * @addr:	Address of the image to be loaded
	 * @size:	Size of the image to be loaded
	 * @return 0 if all ok, else appropriate error value.
	 */
	int (*load)(struct udevice *dev, ulong addr, ulong size);

	/**
	 * start() - Start the remoteproc device (mandatory)
	 *
	 * @dev:	Remote proc device
	 * @return 0 if all ok, else appropriate error value.
	 */
	int (*start)(struct udevice *dev);

	/**
	 * stop() - Stop the remoteproc device (optional)
	 *
	 * @dev:	Remote proc device
	 * @return 0 if all ok, else appropriate error value.
	 */
	int (*stop)(struct udevice *dev);

	/**
	 * reset() - Reset the remoteproc device (optional)
	 *
	 * @dev:	Remote proc device
	 * @return 0 if all ok, else appropriate error value.
	 */
	int (*reset)(struct udevice *dev);

	/**
	 * is_running() - Check if the remote processor is running (optional)
	 *
	 * @dev:	Remote proc device
	 * @return 0 if running, 1 if not running, -ve on error.
	 */
	int (*is_running)(struct udevice *dev);

	/**
	 * ping() - Ping the remote device for basic communication (optional)
	 *
	 * @dev:	Remote proc device
	 * @return 0 on success, 1 if not responding, -ve on other errors.
	 */
	int (*ping)(struct udevice *dev);

	/**
	 * device_to_virt() - Return translated virtual address (optional)
	 *
	 * Translate a device address (remote processor view) to virtual
	 * address (main processor view).
	 *
	 * @dev:	Remote proc device
	 * @da:		Device address
	 * @size:	Size of the memory region @da is pointing to
	 * @return virtual address.
	 */
	void * (*device_to_virt)(struct udevice *dev, ulong da, ulong size);
};

/* Accessor */
#define rproc_get_ops(dev) ((struct dm_rproc_ops *)(dev)->driver->ops)

#if CONFIG_IS_ENABLED(REMOTEPROC)
/**
 * rproc_init() - Initialize all bound remote proc devices
 * @return 0 if all ok, else appropriate error value.
 */
int rproc_init(void);

/**
 * rproc_dev_init() - Initialize a remote proc device based on id
 * @id:		id of the remote processor
 * @return 0 if all ok, else appropriate error value.
 */
int rproc_dev_init(int id);

/**
 * rproc_is_initialized() - check to see if remoteproc devices are initialized
 * @return true if all devices are initialized, false otherwise.
 */
bool rproc_is_initialized(void);

/**
 * rproc_load() - load binary or elf to a remote processor
 * @id:		id of the remote processor
 * @addr:	address in memory where the image is located
 * @size:	size of the image
 * @return 0 if all ok, else appropriate error value.
 */
int rproc_load(int id, ulong addr, ulong size);

/**
 * rproc_start() - Start a remote processor
 * @id:		id of the remote processor
 * @return 0 if all ok, else appropriate error value.
 */
int rproc_start(int id);

/**
 * rproc_stop() - Stop a remote processor
 * @id:		id of the remote processor
 * @return 0 if all ok, else appropriate error value.
 */
int rproc_stop(int id);

/**
 * rproc_reset() - reset a remote processor
 * @id:		id of the remote processor
 * @return 0 if all ok, else appropriate error value.
 */
int rproc_reset(int id);

/**
 * rproc_ping() - ping a remote processor to check if it can communicate
 * @id:		id of the remote processor
 * @return 0 if all ok, else appropriate error value.
 *
 * NOTE: this might need communication path available, which is not implemented
 * as part of remoteproc framework - hook on to appropriate bus architecture to
 * do the same
 */
int rproc_ping(int id);

/**
 * rproc_is_running() - check to see if remote processor is running
 * @id:		id of the remote processor
 * @return 0 if running, 1 if not running, -ve on error.
 *
 * NOTE: this may not involve actual communication capability of the remote
 * processor, but just ensures that it is out of reset and executing code.
 */
int rproc_is_running(int id);

/**
 * rproc_elf32_sanity_check() - Verify if an image is a valid ELF32 one
 *
 * Check if a valid ELF32 image exists at the given memory location. Verify
 * basic ELF32 format requirements like magic number and sections size.
 *
 * @addr:	address of the image to verify
 * @size:	size of the image
 * @return 0 if the image looks good, else appropriate error value.
 */
int rproc_elf32_sanity_check(ulong addr, ulong size);

/**
 * rproc_elf64_sanity_check() - Verify if an image is a valid ELF32 one
 *
 * Check if a valid ELF64 image exists at the given memory location. Verify
 * basic ELF64 format requirements like magic number and sections size.
 *
 * @addr:	address of the image to verify
 * @size:	size of the image
 * @return 0 if the image looks good, else appropriate error value.
 */
int rproc_elf64_sanity_check(ulong addr, ulong size);

/**
 * rproc_elf32_load_image() - load an ELF32 image
 * @dev:	device loading the ELF32 image
 * @addr:	valid ELF32 image address
 * @size:	size of the image
 * @return 0 if the image is successfully loaded, else appropriate error value.
 */
int rproc_elf32_load_image(struct udevice *dev, unsigned long addr, ulong size);

/**
 * rproc_elf64_load_image() - load an ELF64 image
 * @dev:	device loading the ELF64 image
 * @addr:	valid ELF64 image address
 * @size:	size of the image
 * @return 0 if the image is successfully loaded, else appropriate error value.
 */
int rproc_elf64_load_image(struct udevice *dev, ulong addr, ulong size);

/**
 * rproc_elf_load_image() - load an ELF image
 * @dev:	device loading the ELF image
 * @addr:	valid ELF image address
 * @size:	size of the image
 *
 * Auto detects if the image is ELF32 or ELF64 image and load accordingly.
 * @return 0 if the image is successfully loaded, else appropriate error value.
 */
int rproc_elf_load_image(struct udevice *dev, unsigned long addr, ulong size);

/**
 * rproc_elf_get_boot_addr() - Get rproc's boot address.
 * @dev:	device loading the ELF image
 * @addr:	valid ELF image address
 *
 * This function returns the entry point address of the ELF
 * image.
 */
ulong rproc_elf_get_boot_addr(struct udevice *dev, ulong addr);

/**
 * rproc_elf32_load_rsc_table() - load the resource table from an ELF32 image
 *
 * Search for the resource table in an ELF32 image, and if found, copy it to
 * device memory.
 *
 * @dev:	device loading the resource table
 * @fw_addr:	ELF image address
 * @fw_size:	size of the ELF image
 * @rsc_addr:	pointer to the found resource table address. Updated on
 *		operation success
 * @rsc_size:	pointer to the found resource table size. Updated on operation
 *		success
 *
 * @return 0 if a valid resource table is successfully loaded, -ENODATA if there
 * is no resource table (which is optional), or another appropriate error value.
 */
int rproc_elf32_load_rsc_table(struct udevice *dev, ulong fw_addr,
			       ulong fw_size, ulong *rsc_addr, ulong *rsc_size);
/**
 * rproc_elf64_load_rsc_table() - load the resource table from an ELF64 image
 *
 * Search for the resource table in an ELF64 image, and if found, copy it to
 * device memory.
 *
 * @dev:	device loading the resource table
 * @fw_addr:	ELF image address
 * @fw_size:	size of the ELF image
 * @rsc_addr:	pointer to the found resource table address. Updated on
 *		operation success
 * @rsc_size:	pointer to the found resource table size. Updated on operation
 *		success
 *
 * @return 0 if a valid resource table is successfully loaded, -ENODATA if there
 * is no resource table (which is optional), or another appropriate error value.
 */
int rproc_elf64_load_rsc_table(struct udevice *dev, ulong fw_addr,
			       ulong fw_size, ulong *rsc_addr, ulong *rsc_size);
/**
 * rproc_elf_load_rsc_table() - load the resource table from an ELF image
 *
 * Auto detects if the image is ELF32 or ELF64 image and search accordingly for
 * the resource table, and if found, copy it to device memory.
 *
 * @dev:	device loading the resource table
 * @fw_addr:	ELF image address
 * @fw_size:	size of the ELF image
 * @rsc_addr:	pointer to the found resource table address. Updated on
 *		operation success
 * @rsc_size:	pointer to the found resource table size. Updated on operation
 *		success
 *
 * @return 0 if a valid resource table is successfully loaded, -ENODATA if there
 * is no resource table (which is optional), or another appropriate error value.
 */
int rproc_elf_load_rsc_table(struct udevice *dev, ulong fw_addr,
			     ulong fw_size, ulong *rsc_addr, ulong *rsc_size);
#else
static inline int rproc_init(void) { return -ENOSYS; }
static inline int rproc_dev_init(int id) { return -ENOSYS; }
static inline bool rproc_is_initialized(void) { return false; }
static inline int rproc_load(int id, ulong addr, ulong size) { return -ENOSYS; }
static inline int rproc_start(int id) { return -ENOSYS; }
static inline int rproc_stop(int id) { return -ENOSYS; }
static inline int rproc_reset(int id) { return -ENOSYS; }
static inline int rproc_ping(int id) { return -ENOSYS; }
static inline int rproc_is_running(int id) { return -ENOSYS; }
static inline int rproc_elf32_sanity_check(ulong addr,
					   ulong size) { return -ENOSYS; }
static inline int rproc_elf64_sanity_check(ulong addr,
					   ulong size) { return -ENOSYS; }
static inline int rproc_elf_sanity_check(ulong addr,
					 ulong size) { return -ENOSYS; }
static inline int rproc_elf32_load_image(struct udevice *dev,
					 unsigned long addr, ulong size)
{ return -ENOSYS; }
static inline int rproc_elf64_load_image(struct udevice *dev, ulong addr,
					 ulong size)
{ return -ENOSYS; }
static inline int rproc_elf_load_image(struct udevice *dev, ulong addr,
				       ulong size)
{ return -ENOSYS; }
static inline ulong rproc_elf_get_boot_addr(struct udevice *dev, ulong addr)
{ return 0; }
static inline int rproc_elf32_load_rsc_table(struct udevice *dev, ulong fw_addr,
					     ulong fw_size, ulong *rsc_addr,
					     ulong *rsc_size)
{ return -ENOSYS; }
static inline int rproc_elf64_load_rsc_table(struct udevice *dev, ulong fw_addr,
					     ulong fw_size, ulong *rsc_addr,
					     ulong *rsc_size)
{ return -ENOSYS; }
static inline int rproc_elf_load_rsc_table(struct udevice *dev, ulong fw_addr,
					   ulong fw_size, ulong *rsc_addr,
					   ulong *rsc_size)
{ return -ENOSYS; }
#endif

#endif	/* _RPROC_H_ */

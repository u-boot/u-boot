/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * EFI device path functions
 *
 * (C) Copyright 2017 Rob Clark
 */

#ifndef EFI_DEVICE_PATH_H
#define EFI_DEVICE_PATH_H

#include <efi.h>

struct blk_desc;
struct efi_load_option;
struct udevice;

/*
 * EFI_DP_END - Template end node for EFI device paths.
 *
 * Represents the terminating node of an EFI device path.
 * It has a type of DEVICE_PATH_TYPE_END and sub_type DEVICE_PATH_SUB_TYPE_END
 */
extern const struct efi_device_path EFI_DP_END;

/**
 * efi_dp_next() - Iterate to next block in device-path
 *
 * Advance to the next node in an EFI device path.
 *
 * @dp: Pointer to the current device path node.
 * Return: Pointer to the next device path node, or NULL if at the end
 * or if input is NULL.
 */
struct efi_device_path *efi_dp_next(const struct efi_device_path *dp);

/**
 * efi_dp_match() - Compare two device-paths
 *
 * Compare two device paths node by node. The comparison stops when an End
 * node is reached in the shorter of the two paths. This is useful, for example,
 * to compare a device-path representing a device with one representing a file
 * on that device, or a device with a parent device.
 *
 * @a: Pointer to the first device path.
 * @b: Pointer to the second device path.
 * Return: An integer less than, equal to, or greater than zero if the first
 * differing node in 'a' is found, respectively, to be less than,
 * to match, or be greater than the corresponding node in 'b'. Returns 0
 * if they match up to the end of the shorter path. Compares length first,
 * then content.
 */
int efi_dp_match(const struct efi_device_path *a,
		 const struct efi_device_path *b);

/**
 * efi_dp_shorten() - shorten device-path
 *
 * When creating a short-boot option we want to use a device-path that is
 * independent of the location where the block device is plugged in.
 *
 * UsbWwi() nodes contain a serial number, hard drive paths a partition
 * UUID. Both should be unique.
 *
 * See UEFI spec, section 3.1.2 for "short-form device path".
 *
 * @dp:     original device-path
 * Return:  shortened device-path or NULL
 */
struct efi_device_path *efi_dp_shorten(struct efi_device_path *dp);

/**
 * efi_dp_find_obj() - find handle by device path
 *
 * If @rem is provided, the handle with the longest partial match is returned.
 *
 * @dp:     device path to search
 * @guid:   GUID of protocol that must be installed on path or NULL
 * @rem:    pointer to receive remaining device path
 * Return:  matching handle
 */
efi_handle_t efi_dp_find_obj(struct efi_device_path *dp, const efi_guid_t *guid,
			     struct efi_device_path **rem);

/**
 * efi_dp_last_node() - Determine the last device path node before the end node
 *
 * Iterate through the device path to find the very last node before
 * the terminating EFI_DP_END node.
 *
 * @dp: Pointer to the device path.
 * Return: Pointer to the last actual data node before the end node if it exists
 * otherwise NULL (e.g., if dp is NULL or only an EFI_DP_END node).
 */
const struct efi_device_path *efi_dp_last_node(const struct efi_device_path *dp);

/**
 * efi_dp_instance_size() - Get size of the first device path instance
 *
 * Calculate the total length of all nodes in the first instance of a
 * (potentially multi-instance) device path. The size of the instance-specific
 * end node (if any) or the final device path. The end node is not included.
 *
 * @dp: Pointer to the device path.
 * Return: Size in bytes of the first instance, or 0 if dp is NULL or an
 * EFI_DP_END node
 */
efi_uintn_t efi_dp_instance_size(const struct efi_device_path *dp);

/**
 * efi_dp_size() - Get size of multi-instance device path excluding end node
 *
 * Calculate the total size of the entire device path structure, traversing
 * through all instances, up to but not including the final
 * END_ENTIRE_DEVICE_PATH node.
 *
 * @dp: Pointer to the device path.
 * Return: Total size in bytes of all nodes in the device path (excluding the
 * final EFI_DP_END node), or 0 if dp is NULL.
 */
efi_uintn_t efi_dp_size(const struct efi_device_path *dp);

/**
 * efi_dp_dup() - Copy multi-instance device path
 *
 * Duplicate the given device path, including its end node(s).
 * The caller is responsible for freeing the allocated memory (e.g.,
 * using efi_free()).
 *
 * @dp: Pointer to the device path to duplicate.
 * Return: Pointer to the newly allocated and copied device path, or NULL on
 * allocation failure or if dp is NULL.
 */
struct efi_device_path *efi_dp_dup(const struct efi_device_path *dp);

/**
 * efi_dp_concat() - Concatenate two device paths and terminate the result
 *
 * @dp1:        First device path
 * @dp2:        Second device path
 * @split_end_node:
 * - 0 to concatenate (dp1 is assumed not to have an end node or it's ignored,
 *   dp2 is appended, then one EFI_DP_END node)
 * - 1 to concatenate with end node added as separator (dp1, END_THIS_INSTANCE,
 *   dp2, END_ENTIRE)
 *
 * Size of dp1 excluding last end node to concatenate with end node as
 * separator in case dp1 contains an end node (dp1 (partial), END_THIS_INSTANCE,
 * dp2, END_ENTIRE)
 *
 * Return:
 * concatenated device path or NULL. Caller must free the returned value.
 */
struct efi_device_path *efi_dp_concat(const struct efi_device_path *dp1,
				      const struct efi_device_path *dp2,
				      size_t split_end_node);

/**
 * efi_dp_append_node() - Append a single node to a device path
 *
 * Create a new device path by appending a given node to an existing
 * device path.
 * If the original device path @dp is NULL, a new path is created
 * with the given @node followed by an EFI_DP_END node.
 * If the @node is NULL and @dp is not NULL, the original path @dp is
 * duplicated.
 * If both @dp and @node are NULL, a path with only an EFI_DP_END node is
 * returned.
 * The caller must free the returned path (e.g., using efi_free()).
 *
 * @dp:   Original device path (can be NULL).
 * @node: Node to append (can be NULL).
 * Return: New device path with the node appended, or NULL on allocation
 * failure.
 */
struct efi_device_path *efi_dp_append_node(const struct efi_device_path *dp,
					   const struct efi_device_path *node);

/**
 * efi_dp_create_device_node() - Create a new device path node
 *
 * Allocate and initialise the header of a new EFI device path node with the
 * given type, sub-type, and length. The content of the node beyond the basic
 * efi_device_path header is zeroed by efi_alloc.
 *
 * @type:     Device path type.
 * @sub_type: Device path sub-type.
 * @length:   Length of the node (must be >= sizeof(struct efi_device_path)).
 * Return: Pointer to the new device path node, or NULL on allocation failure
 * or if length is invalid.
 */
struct efi_device_path *efi_dp_create_device_node(const u8 type,
						  const u8 sub_type,
						  const u16 length);

/**
 * efi_dp_append_instance() - Append a device path instance to another
 *
 * Concatenate two device paths, treating the second path (@dpi) as a new
 * instance appended to the first path (@dp). An END_THIS_INSTANCE node is
 * inserted between @dp and @dpi if @dp is not NULL.
 * If @dp is NULL, @dpi is duplicated (and terminated appropriately).
 * @dpi must not be NULL.
 * The caller is responsible for freeing the returned path (e.g., using
 * efi_free()).
 *
 * @dp:  The base device path. If NULL, @dpi is duplicated.
 * @dpi: The device path instance to append. Must not be NULL.
 * Return: A new device path with @dpi appended as a new instance, or NULL on
 * error  (e.g. allocation failure, @dpi is NULL).
 */
struct efi_device_path *
efi_dp_append_instance(const struct efi_device_path *dp,
		       const struct efi_device_path *dpi);

/**
 * efi_dp_get_next_instance() - Extract the next dp instance
 *
 * Given a pointer to a pointer to a device path (@dp), this function extracts
 * the first instance from the path. It allocates a new path for this extracted
 * instance (including its instance-specific EFI_DP_END node). The input pointer
 * (*@dp) is then updated to point to the start of the next instance in the
 * original path, or set to NULL if no more instances remain.
 * The caller is responsible for freeing the returned instance path (e.g.,
 * using efi_free()).
 *
 * @dp:  On input, a pointer to a pointer to the multi-instance device path.
 * On output, *@dp is updated to point to the start of the next instance,
 * or NULL if no more instances.
 * @size: Optional pointer to an efi_uintn_t variable that will receive the size
 * of the extracted instance path (including its EFI_DP_END node).
 * Return: Pointer to a newly allocated device path for the extracted instance,
 * or NULL if no instance could be extracted or an error occurred (e.g.,
 * allocation failure).
 */
struct efi_device_path *efi_dp_get_next_instance(struct efi_device_path **dp,
						 efi_uintn_t *size);

/**
 * efi_dp_is_multi_instance() - Check if a device path is multi-instance
 *
 * Traverse the device path to its end. It is considered multi-instance if an
 * END_THIS_INSTANCE_DEVICE_PATH node (type DEVICE_PATH_TYPE_END, sub-type
 * DEVICE_PATH_SUB_TYPE_INSTANCE_END) is encountered before the final
 * END_ENTIRE_DEVICE_PATH node.
 *
 * @dp: The device path to check.
 * Return: True if the device path contains multiple instances, false otherwise
 * (including if @dp is NULL).
 */
bool efi_dp_is_multi_instance(const struct efi_device_path *dp);

/**
 * efi_dp_from_part() - Construct a dp from a partition on a block device
 *
 * Create a full device path for a specified partition on a given block device.
 * If the partition number @part is 0, the path is for the block device itself.
 * The caller is responsible for freeing the allocated memory (e.g., using
 * efi_free()).
 *
 * @desc: Pointer to the block device descriptor.
 * @part: Partition number (0 for the whole device, >0 for a specific
 * partition).
 * Return: Pointer to the newly created device path, or NULL on allocation
 * failure or if the device/partition is not found or invalid.
 */
struct efi_device_path *efi_dp_from_part(struct blk_desc *desc, int part);

/**
 * efi_dp_part_node() - Create a device node for a block device partition
 *
 * Creates a single device path node representing a specific partition
 * (e.g., HardDrivePath or CDROMPath, depending on desc->part_type).
 * It does not create the full path from the root, only the partition-specific
 * node. The caller is responsible for freeing the allocated memory (e.g.,
 * using efi_free()).
 *
 * @desc: Pointer to the block device descriptor.
 * @part: Partition number (must be > 0 and correspond to a valid partition on
 * the device).
 * Return: Pointer to the new device path node for the partition, or NULL on
 * allocation * failure or error in getting partition information.
 */
struct efi_device_path *efi_dp_part_node(struct blk_desc *desc, int part);

/**
 * efi_dp_from_file() - append file path node to device path.
 *
 * @dp:     device path or NULL
 * @path:   file path or NULL
 * Return:  device path or NULL in case of an error
 */
struct efi_device_path *efi_dp_from_file(const struct efi_device_path *dp,
					 const char *path);

/**
 * efi_dp_from_uart() - Create a device path for a UART device.
 *
 * Construct a device path representing the system's default UART,
 * typically based on the U-Boot device model root and a UART messaging node.
 * The caller is responsible for freeing the allocated memory (e.g., using
 * efi_free()).
 *
 * Return: Pointer to the new UART device path, or NULL on allocation failure.
 */
struct efi_device_path *efi_dp_from_uart(void);

/**
 * efi_dp_from_eth() - Create a device path for an Ethernet device
 *
 * Construct a device path representing the given device. The caller is
 * responsible for freeing the allocated memory (e.g. using efi_free())
 *
 * @dev: UCLASS_ETH device to process
 *
 * Return: Pointer to the new Ethernet device path, or NULL on allocation
 * failure
 */
struct efi_device_path *efi_dp_from_eth(struct udevice *dev);

/**
 * efi_dp_from_mem() - Construct a device-path for a memory-mapped region
 *
 * Create an EFI device path representing a specific memory region, defined
 * by its type, start address, and size.
 * The caller is responsible for freeing the allocated memory (e.g.,
 * using efi_free()).
 *
 * @memory_type: EFI memory type (e.g., EFI_RESERVED_MEMORY_TYPE).
 * @start_address: Starting address of the memory region.
 * @size: Size of the memory region in bytes.
 * Return: Pointer to the new memory device path, or NULL on allocation failure
 */
struct efi_device_path *efi_dp_from_mem(u32 memory_type, u64 start_address,
					size_t size);

/**
 * efi_dp_split_file_path() - split of relative file path from device path
 *
 * Given a device path indicating a file on a device, separate the device
 * path in two: the device path of the actual device and the file path
 * relative to this device.
 *
 * @full_path:      device path including device and file path
 * @device_path:    path of the device
 * @file_path:      relative path of the file or NULL if there is none
 * Return:      status code
 */
efi_status_t efi_dp_split_file_path(struct efi_device_path *full_path,
				    struct efi_device_path **device_path,
				    struct efi_device_path **file_path);

/**
 * efi_dp_from_name() - convert U-Boot device and file path to device path
 *
 * @dev:    U-Boot device, e.g. 'mmc'
 * @devnr:  U-Boot device number, e.g. 1 for 'mmc:1'
 * @path:   file path relative to U-Boot device, may be NULL
 * @device: pointer to receive device path of the device
 * @file:   pointer to receive device path for the file
 * Return:  status code
 */
efi_status_t efi_dp_from_name(const char *dev, const char *devnr,
			      const char *path, struct efi_device_path **device,
			      struct efi_device_path **file);

/**
 * efi_dp_check_length() - check length of a device path
 *
 * @dp:     pointer to device path
 * @maxlen: maximum length of the device path
 * Return:
 * * length of the device path if it is less or equal @maxlen
 * * -1 if the device path is longer then @maxlen
 * * -1 if a device path node has a length of less than 4
 * * -EINVAL if maxlen exceeds SSIZE_MAX
 */
ssize_t efi_dp_check_length(const struct efi_device_path *dp,
			    const size_t maxlen);

/**
 * efi_dp_from_lo() - get device-path from load option
 *
 * The load options in U-Boot may contain multiple concatenated device-paths.
 * The first device-path indicates the EFI binary to execute. Subsequent
 * device-paths start with a VenMedia node where the GUID identifies the
 * function (initrd or fdt).
 *
 * @lo:     EFI load option containing a valid device path
 * @guid:   GUID identifying device-path or NULL for the EFI binary
 *
 * Return:
 * device path excluding the matched VenMedia node or NULL.
 * Caller must free the returned value.
 */
struct efi_device_path *efi_dp_from_lo(struct efi_load_option *lo,
				       const efi_guid_t *guid);

/**
 * search_gpt_dp_node() - search gpt device path node
 *
 * @device_path:    device path
 *
 * Return:  pointer to the gpt device path node
 */
struct efi_device_path *search_gpt_dp_node(struct efi_device_path *device_path);

/**
 * efi_dp_from_http() - set device path from http
 *
 * Set the device path to an IPv4 path as provided by efi_dp_from_ipv4
 * concatenated with a device path of subtype DEVICE_PATH_SUB_TYPE_MSG_URI,
 * and an EFI_DP_END node.
 *
 * @server:	URI of remote server
 * @dev:	net udevice
 * Return:	pointer to HTTP device path, NULL on error
 */
struct efi_device_path *efi_dp_from_http(const char *server,
					 struct udevice *dev);

#endif /* EFI_DEVICE_PATH_H */

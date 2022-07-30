/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __bootflow_h
#define __bootflow_h

#include <linux/list.h>

/**
 * enum bootflow_state_t - states that a particular bootflow can be in
 *
 * Only bootflows in state BOOTFLOWST_READY can be used to boot.
 *
 * See bootflow_state[] for the names for each of these
 */
enum bootflow_state_t {
	BOOTFLOWST_BASE,	/**< Nothing known yet */
	BOOTFLOWST_MEDIA,	/**< Media exists */
	BOOTFLOWST_PART,	/**< Partition exists */
	BOOTFLOWST_FS,		/**< Filesystem exists */
	BOOTFLOWST_FILE,	/**< Bootflow file exists */
	BOOTFLOWST_READY,	/**< Bootflow file loaded */

	BOOTFLOWST_COUNT
};

/**
 * struct bootflow - information about a bootflow
 *
 * This is connected into two separate linked lists:
 *
 *   bm_sibling - links all bootflows in the same bootdev
 *   glob_sibling - links all bootflows in all bootdevs
 *
 * @bm_node: Points to siblings in the same bootdev
 * @glob_node: Points to siblings in the global list (all bootdev)
 * @dev: Bootdevice device which produced this bootflow
 * @blk: Block device which contains this bootflow, NULL if this is a network
 *	device
 * @part: Partition number (0 for whole device)
 * @fs_type: Filesystem type (FS_TYPE...) if this is fixed by the media, else 0.
 *	For example, the sandbox host-filesystem bootdev sets this to
 *	FS_TYPE_SANDBOX
 * @method: Bootmethod device used to perform the boot and read files
 * @name: Name of bootflow (allocated)
 * @state: Current state (enum bootflow_state_t)
 * @subdir: Subdirectory to fetch files from (with trailing /), or NULL if none
 * @fname: Filename of bootflow file (allocated)
 * @buf: Bootflow file contents (allocated)
 * @size: Size of bootflow file in bytes
 * @err: Error number received (0 if OK)
 */
struct bootflow {
	struct list_head bm_node;
	struct list_head glob_node;
	struct udevice *dev;
	struct udevice *blk;
	int part;
	int fs_type;
	struct udevice *method;
	char *name;
	enum bootflow_state_t state;
	char *subdir;
	char *fname;
	char *buf;
	int size;
	int err;
};

/**
 * enum bootflow_flags_t - flags for the bootflow iterator
 *
 * @BOOTFLOWF_FIXED: Only used fixed/internal media
 * @BOOTFLOWF_SHOW: Show each bootdev before scanning it
 * @BOOTFLOWF_ALL: Return bootflows with errors as well
 * @BOOTFLOWF_SINGLE_DEV: Just scan one bootmeth
 * @BOOTFLOWF_SKIP_GLOBAL: Don't scan global bootmeths
 */
enum bootflow_flags_t {
	BOOTFLOWF_FIXED		= 1 << 0,
	BOOTFLOWF_SHOW		= 1 << 1,
	BOOTFLOWF_ALL		= 1 << 2,
	BOOTFLOWF_SINGLE_DEV	= 1 << 3,
	BOOTFLOWF_SKIP_GLOBAL	= 1 << 4,
};

/**
 * struct bootflow_iter - state for iterating through bootflows
 *
 * This starts at with the first bootdev/partition/bootmeth and can be used to
 * iterate through all of them.
 *
 * Iteration starts with the bootdev. The first partition (0, i.e. whole device)
 * is scanned first. For partition 0, it iterates through all the available
 * bootmeths to see which one(s) can provide a bootflow. Then it moves to
 * parition 1 (if there is one) and the process continues. Once all partitions
 * are examined, it moves to the next bootdev.
 *
 * Initially @max_part is 0, meaning that only the whole device (@part=0) can be
 * used. During scanning, if a partition table is found, then @max_part is
 * updated to a larger value, no less than the number of available partitions.
 * This ensures that iteration works through all partitions on the bootdev.
 *
 * @flags: Flags to use (see enum bootflow_flags_t). If BOOTFLOWF_GLOBAL_FIRST is
 *	enabled then the global bootmeths are being scanned, otherwise we have
 *	moved onto the bootdevs
 * @dev: Current bootdev, NULL if none
 * @part: Current partition number (0 for whole device)
 * @method: Current bootmeth
 * @max_part: Maximum hardware partition number in @dev, 0 if there is no
 *	partition table
 * @err: Error obtained from checking the last iteration. This is used to skip
 *	forward (e.g. to skip the current partition because it is not valid)
 *	-ESHUTDOWN: try next bootdev
 * @num_devs: Number of bootdevs in @dev_order
 * @cur_dev: Current bootdev number, an index into @dev_order[]
 * @dev_order: List of bootdevs to scan, in order of priority. The scan starts
 *	with the first one on the list
 * @num_methods: Number of bootmeth devices in @method_order
 * @cur_method: Current method number, an index into @method_order
 * @first_glob_method: First global method, if any, else -1
 * @method_order: List of bootmeth devices to use, in order. The normal methods
 *	appear first, then the global ones, if any
 * @doing_global: true if we are iterating through the global bootmeths (which
 *	happens before the normal ones)
 */
struct bootflow_iter {
	int flags;
	struct udevice *dev;
	int part;
	struct udevice *method;
	int max_part;
	int err;
	int num_devs;
	int cur_dev;
	struct udevice **dev_order;
	int num_methods;
	int cur_method;
	int first_glob_method;
	struct udevice **method_order;
	bool doing_global;
};

/**
 * bootflow_iter_init() - Reset a bootflow iterator
 *
 * This sets everything to the starting point, ready for use.
 *
 * @iter: Place to store private info (inited by this call)
 * @flags: Flags to use (see enum bootflow_flags_t)
 */
void bootflow_iter_init(struct bootflow_iter *iter, int flags);

/**
 * bootflow_iter_uninit() - Free memory used by an interator
 *
 * @iter:	Iterator to free
 */
void bootflow_iter_uninit(struct bootflow_iter *iter);

/**
 * bootflow_iter_drop_bootmeth() - Remove a bootmeth from an iterator
 *
 * Update the iterator so that the bootmeth will not be used again while this
 * iterator is in use
 *
 * @iter: Iterator to update
 * @bmeth: Boot method to remove
 */
int bootflow_iter_drop_bootmeth(struct bootflow_iter *iter,
				const struct udevice *bmeth);

/**
 * bootflow_scan_bootdev() - find the first bootflow in a bootdev
 *
 * If @flags includes BOOTFLOWF_ALL then bootflows with errors are returned too
 *
 * @dev:	Boot device to scan, NULL to work through all of them until it
 *	finds one that can supply a bootflow
 * @iter:	Place to store private info (inited by this call)
 * @flags:	Flags for iterator (enum bootflow_flags_t)
 * @bflow:	Place to put the bootflow if found
 * Return: 0 if found,  -ENODEV if no device, other -ve on other error
 *	(iteration can continue)
 */
int bootflow_scan_bootdev(struct udevice *dev, struct bootflow_iter *iter,
			  int flags, struct bootflow *bflow);

/**
 * bootflow_scan_first() - find the first bootflow
 *
 * This works through the available bootdev devices until it finds one that
 * can supply a bootflow. It then returns that
 *
 * If @flags includes BOOTFLOWF_ALL then bootflows with errors are returned too
 *
 * @iter:	Place to store private info (inited by this call), with
 * @flags:	Flags for bootdev (enum bootflow_flags_t)
 * @bflow:	Place to put the bootflow if found
 * Return: 0 if found, -ENODEV if no device, other -ve on other error (iteration
 *	can continue)
 */
int bootflow_scan_first(struct bootflow_iter *iter, int flags,
			struct bootflow *bflow);

/**
 * bootflow_scan_next() - find the next bootflow
 *
 * This works through the available bootdev devices until it finds one that
 * can supply a bootflow. It then returns that bootflow
 *
 * @iter:	Private info (as set up by bootflow_scan_first())
 * @bflow:	Place to put the bootflow if found
 * Return: 0 if found, -ENODEV if no device, -ESHUTDOWN if no more bootflows,
 *	other -ve on other error (iteration can continue)
 */
int bootflow_scan_next(struct bootflow_iter *iter, struct bootflow *bflow);

/**
 * bootflow_first_glob() - Get the first bootflow from the global list
 *
 * Returns the first bootflow in the global list, no matter what bootflow it is
 * attached to
 *
 * @bflowp: Returns a pointer to the bootflow
 * Return: 0 if found, -ENOENT if there are no bootflows
 */
int bootflow_first_glob(struct bootflow **bflowp);

/**
 * bootflow_next_glob() - Get the next bootflow from the global list
 *
 * Returns the next bootflow in the global list, no matter what bootflow it is
 * attached to
 *
 * @bflowp: On entry, the last bootflow returned , e.g. from
 *	bootflow_first_glob()
 * Return: 0 if found, -ENOENT if there are no more bootflows
 */
int bootflow_next_glob(struct bootflow **bflowp);

/**
 * bootflow_free() - Free memory used by a bootflow
 *
 * This frees fields within @bflow, but not the @bflow pointer itself
 */
void bootflow_free(struct bootflow *bflow);

/**
 * bootflow_boot() - boot a bootflow
 *
 * @bflow: Bootflow to boot
 * Return: -EPROTO if bootflow has not been loaded, -ENOSYS if the bootflow
 *	type is not supported, -EFAULT if the boot returned without an error
 *	when we are expecting it to boot, -ENOTSUPP if trying method resulted in
 *	finding out that is not actually supported for this boot and should not
 *	be tried again unless something changes
 */
int bootflow_boot(struct bootflow *bflow);

/**
 * bootflow_run_boot() - Try to boot a bootflow
 *
 * @iter: Current iteration (or NULL if none). Used to disable a bootmeth if the
 *	boot returns -ENOTSUPP
 * @bflow: Bootflow to boot
 * Return: result of trying to boot
 */
int bootflow_run_boot(struct bootflow_iter *iter, struct bootflow *bflow);

/**
 * bootflow_state_get_name() - Get the name of a bootflow state
 *
 * @state: State to check
 * Return: name, or "?" if invalid
 */
const char *bootflow_state_get_name(enum bootflow_state_t state);

/**
 * bootflow_remove() - Remove a bootflow and free its memory
 *
 * This updates the linked lists containing the bootflow then frees it.
 *
 * @bflow: Bootflow to remove
 */
void bootflow_remove(struct bootflow *bflow);

/**
 * bootflow_iter_uses_blk_dev() - Check that a bootflow uses a block device
 *
 * This checks the bootdev in the bootflow to make sure it uses a block device
 *
 * Return: 0 if OK, -ENOTSUPP if some other device is used (e.g. ethernet)
 */
int bootflow_iter_uses_blk_dev(const struct bootflow_iter *iter);

/**
 * bootflow_iter_uses_network() - Check that a bootflow uses a network device
 *
 * This checks the bootdev in the bootflow to make sure it uses a network
 * device
 *
 * Return: 0 if OK, -ENOTSUPP if some other device is used (e.g. MMC)
 */
int bootflow_iter_uses_network(const struct bootflow_iter *iter);

/**
 * bootflow_iter_uses_system() - Check that a bootflow uses the bootstd device
 *
 * This checks the bootdev in the bootflow to make sure it uses the bootstd
 * device
 *
 * Return: 0 if OK, -ENOTSUPP if some other device is used (e.g. MMC)
 */
int bootflow_iter_uses_system(const struct bootflow_iter *iter);

#endif

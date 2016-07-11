/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef BLK_H
#define BLK_H

#ifdef CONFIG_SYS_64BIT_LBA
typedef uint64_t lbaint_t;
#define LBAFlength "ll"
#else
typedef ulong lbaint_t;
#define LBAFlength "l"
#endif
#define LBAF "%" LBAFlength "x"
#define LBAFU "%" LBAFlength "u"

/* Interface types: */
enum if_type {
	IF_TYPE_UNKNOWN = 0,
	IF_TYPE_IDE,
	IF_TYPE_SCSI,
	IF_TYPE_ATAPI,
	IF_TYPE_USB,
	IF_TYPE_DOC,
	IF_TYPE_MMC,
	IF_TYPE_SD,
	IF_TYPE_SATA,
	IF_TYPE_HOST,

	IF_TYPE_COUNT,			/* Number of interface types */
};

/*
 * With driver model (CONFIG_BLK) this is uclass platform data, accessible
 * with dev_get_uclass_platdata(dev)
 */
struct blk_desc {
	/*
	 * TODO: With driver model we should be able to use the parent
	 * device's uclass instead.
	 */
	enum if_type	if_type;	/* type of the interface */
	int		devnum;		/* device number */
	unsigned char	part_type;	/* partition type */
	unsigned char	target;		/* target SCSI ID */
	unsigned char	lun;		/* target LUN */
	unsigned char	hwpart;		/* HW partition, e.g. for eMMC */
	unsigned char	type;		/* device type */
	unsigned char	removable;	/* removable device */
#ifdef CONFIG_LBA48
	/* device can use 48bit addr (ATA/ATAPI v7) */
	unsigned char	lba48;
#endif
	lbaint_t	lba;		/* number of blocks */
	unsigned long	blksz;		/* block size */
	int		log2blksz;	/* for convenience: log2(blksz) */
	char		vendor[40+1];	/* IDE model, SCSI Vendor */
	char		product[20+1];	/* IDE Serial no, SCSI product */
	char		revision[8+1];	/* firmware revision */
#ifdef CONFIG_BLK
	struct udevice *bdev;
#else
	unsigned long	(*block_read)(struct blk_desc *block_dev,
				      lbaint_t start,
				      lbaint_t blkcnt,
				      void *buffer);
	unsigned long	(*block_write)(struct blk_desc *block_dev,
				       lbaint_t start,
				       lbaint_t blkcnt,
				       const void *buffer);
	unsigned long	(*block_erase)(struct blk_desc *block_dev,
				       lbaint_t start,
				       lbaint_t blkcnt);
	void		*priv;		/* driver private struct pointer */
#endif
};

#define BLOCK_CNT(size, blk_desc) (PAD_COUNT(size, blk_desc->blksz))
#define PAD_TO_BLOCKSIZE(size, blk_desc) \
	(PAD_SIZE(size, blk_desc->blksz))

#ifdef CONFIG_BLOCK_CACHE
/**
 * blkcache_read() - attempt to read a set of blocks from cache
 *
 * @param iftype - IF_TYPE_x for type of device
 * @param dev - device index of particular type
 * @param start - starting block number
 * @param blkcnt - number of blocks to read
 * @param blksz - size in bytes of each block
 * @param buf - buffer to contain cached data
 *
 * @return - '1' if block returned from cache, '0' otherwise.
 */
int blkcache_read(int iftype, int dev,
		  lbaint_t start, lbaint_t blkcnt,
		  unsigned long blksz, void *buffer);

/**
 * blkcache_fill() - make data read from a block device available
 * to the block cache
 *
 * @param iftype - IF_TYPE_x for type of device
 * @param dev - device index of particular type
 * @param start - starting block number
 * @param blkcnt - number of blocks available
 * @param blksz - size in bytes of each block
 * @param buf - buffer containing data to cache
 *
 */
void blkcache_fill(int iftype, int dev,
		   lbaint_t start, lbaint_t blkcnt,
		   unsigned long blksz, void const *buffer);

/**
 * blkcache_invalidate() - discard the cache for a set of blocks
 * because of a write or device (re)initialization.
 *
 * @param iftype - IF_TYPE_x for type of device
 * @param dev - device index of particular type
 */
void blkcache_invalidate(int iftype, int dev);

/**
 * blkcache_configure() - configure block cache
 *
 * @param blocks - maximum blocks per entry
 * @param entries - maximum entries in cache
 */
void blkcache_configure(unsigned blocks, unsigned entries);

/*
 * statistics of the block cache
 */
struct block_cache_stats {
	unsigned hits;
	unsigned misses;
	unsigned entries; /* current entry count */
	unsigned max_blocks_per_entry;
	unsigned max_entries;
};

/**
 * get_blkcache_stats() - return statistics and reset
 *
 * @param stats - statistics are copied here
 */
void blkcache_stats(struct block_cache_stats *stats);

#else

static inline int blkcache_read(int iftype, int dev,
				lbaint_t start, lbaint_t blkcnt,
				unsigned long blksz, void *buffer)
{
	return 0;
}

static inline void blkcache_fill(int iftype, int dev,
				 lbaint_t start, lbaint_t blkcnt,
				 unsigned long blksz, void const *buffer) {}

static inline void blkcache_invalidate(int iftype, int dev) {}

#endif

#ifdef CONFIG_BLK
struct udevice;

/* Operations on block devices */
struct blk_ops {
	/**
	 * read() - read from a block device
	 *
	 * @dev:	Device to read from
	 * @start:	Start block number to read (0=first)
	 * @blkcnt:	Number of blocks to read
	 * @buffer:	Destination buffer for data read
	 * @return number of blocks read, or -ve error number (see the
	 * IS_ERR_VALUE() macro
	 */
	unsigned long (*read)(struct udevice *dev, lbaint_t start,
			      lbaint_t blkcnt, void *buffer);

	/**
	 * write() - write to a block device
	 *
	 * @dev:	Device to write to
	 * @start:	Start block number to write (0=first)
	 * @blkcnt:	Number of blocks to write
	 * @buffer:	Source buffer for data to write
	 * @return number of blocks written, or -ve error number (see the
	 * IS_ERR_VALUE() macro
	 */
	unsigned long (*write)(struct udevice *dev, lbaint_t start,
			       lbaint_t blkcnt, const void *buffer);

	/**
	 * erase() - erase a section of a block device
	 *
	 * @dev:	Device to (partially) erase
	 * @start:	Start block number to erase (0=first)
	 * @blkcnt:	Number of blocks to erase
	 * @return number of blocks erased, or -ve error number (see the
	 * IS_ERR_VALUE() macro
	 */
	unsigned long (*erase)(struct udevice *dev, lbaint_t start,
			       lbaint_t blkcnt);
};

#define blk_get_ops(dev)	((struct blk_ops *)(dev)->driver->ops)

/*
 * These functions should take struct udevice instead of struct blk_desc,
 * but this is convenient for migration to driver model. Add a 'd' prefix
 * to the function operations, so that blk_read(), etc. can be reserved for
 * functions with the correct arguments.
 */
unsigned long blk_dread(struct blk_desc *block_dev, lbaint_t start,
			lbaint_t blkcnt, void *buffer);
unsigned long blk_dwrite(struct blk_desc *block_dev, lbaint_t start,
			 lbaint_t blkcnt, const void *buffer);
unsigned long blk_derase(struct blk_desc *block_dev, lbaint_t start,
			 lbaint_t blkcnt);

/**
 * blk_get_device() - Find and probe a block device ready for use
 *
 * @if_type:	Interface type (enum if_type_t)
 * @devnum:	Device number (specific to each interface type)
 * @devp:	the device, if found
 * @return - if found, -ENODEV if no device found, or other -ve error value
 */
int blk_get_device(int if_type, int devnum, struct udevice **devp);

/**
 * blk_first_device() - Find the first device for a given interface
 *
 * The device is probed ready for use
 *
 * @devnum:	Device number (specific to each interface type)
 * @devp:	the device, if found
 * @return 0 if found, -ENODEV if no device, or other -ve error value
 */
int blk_first_device(int if_type, struct udevice **devp);

/**
 * blk_next_device() - Find the next device for a given interface
 *
 * This can be called repeatedly after blk_first_device() to iterate through
 * all devices of the given interface type.
 *
 * The device is probed ready for use
 *
 * @devp:	On entry, the previous device returned. On exit, the next
 *		device, if found
 * @return 0 if found, -ENODEV if no device, or other -ve error value
 */
int blk_next_device(struct udevice **devp);

/**
 * blk_create_device() - Create a new block device
 *
 * @parent:	Parent of the new device
 * @drv_name:	Driver name to use for the block device
 * @name:	Name for the device
 * @if_type:	Interface type (enum if_type_t)
 * @devnum:	Device number, specific to the interface type
 * @blksz:	Block size of the device in bytes (typically 512)
 * @size:	Total size of the device in bytes
 * @devp:	the new device (which has not been probed)
 */
int blk_create_device(struct udevice *parent, const char *drv_name,
		      const char *name, int if_type, int devnum, int blksz,
		      lbaint_t size, struct udevice **devp);

/**
 * blk_prepare_device() - Prepare a block device for use
 *
 * This reads partition information from the device if supported.
 *
 * @dev:	Device to prepare
 * @return 0 if ok, -ve on error
 */
int blk_prepare_device(struct udevice *dev);

/**
 * blk_unbind_all() - Unbind all device of the given interface type
 *
 * The devices are removed and then unbound.
 *
 * @if_type:	Interface type to unbind
 * @return 0 if OK, -ve on error
 */
int blk_unbind_all(int if_type);

#else
#include <errno.h>
/*
 * These functions should take struct udevice instead of struct blk_desc,
 * but this is convenient for migration to driver model. Add a 'd' prefix
 * to the function operations, so that blk_read(), etc. can be reserved for
 * functions with the correct arguments.
 */
static inline ulong blk_dread(struct blk_desc *block_dev, lbaint_t start,
			      lbaint_t blkcnt, void *buffer)
{
	ulong blks_read;
	if (blkcache_read(block_dev->if_type, block_dev->devnum,
			  start, blkcnt, block_dev->blksz, buffer))
		return blkcnt;

	/*
	 * We could check if block_read is NULL and return -ENOSYS. But this
	 * bloats the code slightly (cause some board to fail to build), and
	 * it would be an error to try an operation that does not exist.
	 */
	blks_read = block_dev->block_read(block_dev, start, blkcnt, buffer);
	if (blks_read == blkcnt)
		blkcache_fill(block_dev->if_type, block_dev->devnum,
			      start, blkcnt, block_dev->blksz, buffer);

	return blks_read;
}

static inline ulong blk_dwrite(struct blk_desc *block_dev, lbaint_t start,
			       lbaint_t blkcnt, const void *buffer)
{
	blkcache_invalidate(block_dev->if_type, block_dev->devnum);
	return block_dev->block_write(block_dev, start, blkcnt, buffer);
}

static inline ulong blk_derase(struct blk_desc *block_dev, lbaint_t start,
			       lbaint_t blkcnt)
{
	blkcache_invalidate(block_dev->if_type, block_dev->devnum);
	return block_dev->block_erase(block_dev, start, blkcnt);
}
#endif /* !CONFIG_BLK */

#endif

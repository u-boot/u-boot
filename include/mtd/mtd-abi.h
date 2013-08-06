/*
 * $Id: mtd-abi.h,v 1.13 2005/11/07 11:14:56 gleixner Exp $
 *
 * Portions of MTD ABI definition which are shared by kernel and user space
 */

#ifndef __MTD_ABI_H__
#define __MTD_ABI_H__

#if 1
#include <linux/compat.h>
#endif

#include <linux/compiler.h>

struct erase_info_user {
	uint32_t start;
	uint32_t length;
};

struct mtd_oob_buf {
	uint32_t start;
	uint32_t length;
	unsigned char __user *ptr;
};

/*
 * MTD operation modes
 *
 * @MTD_OPS_PLACE_OOB:	OOB data are placed at the given offset (default)
 * @MTD_OPS_AUTO_OOB:	OOB data are automatically placed at the free areas
 *			which are defined by the internal ecclayout
 * @MTD_OPS_RAW:	data are transferred as-is, with no error correction;
 *			this mode implies %MTD_OPS_PLACE_OOB
 *
 * These modes can be passed to ioctl(MEMWRITE) and are also used internally.
 * See notes on "MTD file modes" for discussion on %MTD_OPS_RAW vs.
 * %MTD_FILE_MODE_RAW.
 */
enum {
	MTD_OPS_PLACE_OOB = 0,
	MTD_OPS_AUTO_OOB = 1,
	MTD_OPS_RAW = 2,
};

#define MTD_ABSENT		0
#define MTD_RAM			1
#define MTD_ROM			2
#define MTD_NORFLASH		3
#define MTD_NANDFLASH		4
#define MTD_DATAFLASH		6
#define MTD_UBIVOLUME		7

#define MTD_WRITEABLE		0x400	/* Device is writeable */
#define MTD_BIT_WRITEABLE	0x800	/* Single bits can be flipped */
#define MTD_NO_ERASE		0x1000	/* No erase necessary */
#define MTD_STUPID_LOCK		0x2000	/* Always locked after reset */

/* Some common devices / combinations of capabilities */
#define MTD_CAP_ROM		0
#define MTD_CAP_RAM		(MTD_WRITEABLE | MTD_BIT_WRITEABLE | MTD_NO_ERASE)
#define MTD_CAP_NORFLASH	(MTD_WRITEABLE | MTD_BIT_WRITEABLE)
#define MTD_CAP_NANDFLASH	(MTD_WRITEABLE)

/* ECC byte placement */
#define MTD_NANDECC_OFF		0	/* Switch off ECC (Not recommended) */
#define MTD_NANDECC_PLACE	1	/* Use the given placement in the structure (YAFFS1 legacy mode) */
#define MTD_NANDECC_AUTOPLACE	2	/* Use the default placement scheme */
#define MTD_NANDECC_PLACEONLY	3	/* Use the given placement in the structure (Do not store ecc result on read) */
#define MTD_NANDECC_AUTOPL_USR	4	/* Use the given autoplacement scheme rather than using the default */

/* OTP mode selection */
#define MTD_OTP_OFF		0
#define MTD_OTP_FACTORY		1
#define MTD_OTP_USER		2

struct mtd_info_user {
	uint8_t type;
	uint32_t flags;
	uint32_t size;			/* Total size of the MTD */
	uint32_t erasesize;
	uint32_t writesize;
	uint32_t oobsize;		/* Amount of OOB data per block (e.g. 16) */
	/* The below two fields are obsolete and broken, do not use them
	 * (TODO: remove at some point) */
	uint32_t ecctype;
	uint32_t eccsize;
};

struct region_info_user {
	uint32_t offset;		/* At which this region starts,
					 * from the beginning of the MTD */
	uint32_t erasesize;		/* For this region */
	uint32_t numblocks;		/* Number of blocks in this region */
	uint32_t regionindex;
};

struct otp_info {
	uint32_t start;
	uint32_t length;
	uint32_t locked;
};

/* Get basic MTD characteristics info (better to use sysfs) */
#define MEMGETINFO		_IOR('M', 1, struct mtd_info_user)
/* Erase segment of MTD */
#define MEMERASE		_IOW('M', 2, struct erase_info_user)
/* Write out-of-band data from MTD */
#define MEMWRITEOOB		_IOWR('M', 3, struct mtd_oob_buf)
/* Read out-of-band data from MTD */
#define MEMREADOOB		_IOWR('M', 4, struct mtd_oob_buf)
/* Lock a chip (for MTD that supports it) */
#define MEMLOCK			_IOW('M', 5, struct erase_info_user)
/* Unlock a chip (for MTD that supports it) */
#define MEMUNLOCK		_IOW('M', 6, struct erase_info_user)
/* Get the number of different erase regions */
#define MEMGETREGIONCOUNT	_IOR('M', 7, int)
/* Get information about the erase region for a specific index */
#define MEMGETREGIONINFO	_IOWR('M', 8, struct region_info_user)
/* Get info about OOB modes (e.g., RAW, PLACE, AUTO) - legacy interface */
#define MEMSETOOBSEL		_IOW('M', 9, struct nand_oobinfo)
#define MEMGETOOBSEL		_IOR('M', 10, struct nand_oobinfo)
/* Check if an eraseblock is bad */
#define MEMGETBADBLOCK		_IOW('M', 11, loff_t)
/* Mark an eraseblock as bad */
#define MEMSETBADBLOCK		_IOW('M', 12, loff_t)
/* Set OTP (One-Time Programmable) mode (factory vs. user) */
#define OTPSELECT		_IOR('M', 13, int)
/* Get number of OTP (One-Time Programmable) regions */
#define OTPGETREGIONCOUNT	_IOW('M', 14, int)
/* Get all OTP (One-Time Programmable) info about MTD */
#define OTPGETREGIONINFO	_IOW('M', 15, struct otp_info)
/* Lock a given range of user data (must be in mode %MTD_FILE_MODE_OTP_USER) */
#define OTPLOCK			_IOR('M', 16, struct otp_info)
/* Get ECC layout (deprecated) */
#define ECCGETLAYOUT		_IOR('M', 17, struct nand_ecclayout)
/* Get statistics about corrected/uncorrected errors */
#define ECCGETSTATS		_IOR('M', 18, struct mtd_ecc_stats)
/* Set MTD mode on a per-file-descriptor basis (see "MTD file modes") */
#define MTDFILEMODE		_IO('M', 19)

/*
 * Obsolete legacy interface. Keep it in order not to break userspace
 * interfaces
 */
struct nand_oobinfo {
	uint32_t useecc;
	uint32_t eccbytes;
	uint32_t oobfree[8][2];
	uint32_t eccpos[48];
};

struct nand_oobfree {
	uint32_t offset;
	uint32_t length;
};

#define MTD_MAX_OOBFREE_ENTRIES	8
/*
 * ECC layout control structure. Exported to userspace for
 * diagnosis and to allow creation of raw images
 */
struct nand_ecclayout {
	uint32_t eccbytes;
	uint32_t eccpos[128];
	uint32_t oobavail;
	struct nand_oobfree oobfree[MTD_MAX_OOBFREE_ENTRIES];
};

/**
 * struct mtd_ecc_stats - error correction stats
 *
 * @corrected:	number of corrected bits
 * @failed:	number of uncorrectable errors
 * @badblocks:	number of bad blocks in this partition
 * @bbtblocks:	number of blocks reserved for bad block tables
 */
struct mtd_ecc_stats {
	uint32_t corrected;
	uint32_t failed;
	uint32_t badblocks;
	uint32_t bbtblocks;
};

/*
 * MTD file modes - for read/write access to MTD
 *
 * @MTD_FILE_MODE_NORMAL:	OTP disabled, ECC enabled
 * @MTD_FILE_MODE_OTP_FACTORY:	OTP enabled in factory mode
 * @MTD_FILE_MODE_OTP_USER:	OTP enabled in user mode
 * @MTD_FILE_MODE_RAW:		OTP disabled, ECC disabled
 *
 * These modes can be set via ioctl(MTDFILEMODE). The mode mode will be retained
 * separately for each open file descriptor.
 *
 * Note: %MTD_FILE_MODE_RAW provides the same functionality as %MTD_OPS_RAW -
 * raw access to the flash, without error correction or autoplacement schemes.
 * Wherever possible, the MTD_OPS_* mode will override the MTD_FILE_MODE_* mode
 * (e.g., when using ioctl(MEMWRITE)), but in some cases, the MTD_FILE_MODE is
 * used out of necessity (e.g., `write()', ioctl(MEMWRITEOOB64)).
 */
enum mtd_file_modes {
	MTD_MODE_NORMAL = MTD_OTP_OFF,
	MTD_MODE_OTP_FACTORY = MTD_OTP_FACTORY,
	MTD_MODE_OTP_USER = MTD_OTP_USER,
	MTD_MODE_RAW,
};

#endif /* __MTD_ABI_H__ */

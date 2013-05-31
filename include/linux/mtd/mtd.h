/*
 * Copyright (C) 1999-2003 David Woodhouse <dwmw2@infradead.org> et al.
 *
 * Released under GPL
 */

#ifndef __MTD_MTD_H__
#define __MTD_MTD_H__

#include <linux/types.h>
#include <div64.h>
#include <mtd/mtd-abi.h>
#include <asm/errno.h>

#define MTD_CHAR_MAJOR 90
#define MTD_BLOCK_MAJOR 31
#define MAX_MTD_DEVICES 32

#define MTD_ERASE_PENDING	0x01
#define MTD_ERASING		0x02
#define MTD_ERASE_SUSPEND	0x04
#define MTD_ERASE_DONE          0x08
#define MTD_ERASE_FAILED        0x10

#define MTD_FAIL_ADDR_UNKNOWN	-1LL

/*
 * Enumeration for NAND/OneNAND flash chip state
 */
enum {
	FL_READY,
	FL_READING,
	FL_WRITING,
	FL_ERASING,
	FL_SYNCING,
	FL_CACHEDPRG,
	FL_RESETING,
	FL_UNLOCKING,
	FL_LOCKING,
	FL_PM_SUSPENDED,
};

/* If the erase fails, fail_addr might indicate exactly which block failed.  If
   fail_addr = MTD_FAIL_ADDR_UNKNOWN, the failure was not at the device level or was not
   specific to any particular block. */
struct erase_info {
	struct mtd_info *mtd;
	uint64_t addr;
	uint64_t len;
	uint64_t fail_addr;
	u_long time;
	u_long retries;
	u_int dev;
	u_int cell;
	void (*callback) (struct erase_info *self);
	u_long priv;
	u_char state;
	struct erase_info *next;
	int scrub;
};

struct mtd_erase_region_info {
	uint64_t offset;			/* At which this region starts, from the beginning of the MTD */
	u_int32_t erasesize;		/* For this region */
	u_int32_t numblocks;		/* Number of blocks of erasesize in this region */
	unsigned long *lockmap;		/* If keeping bitmap of locks */
};

/**
 * struct mtd_oob_ops - oob operation operands
 * @mode:	operation mode
 *
 * @len:	number of data bytes to write/read
 *
 * @retlen:	number of data bytes written/read
 *
 * @ooblen:	number of oob bytes to write/read
 * @oobretlen:	number of oob bytes written/read
 * @ooboffs:	offset of oob data in the oob area (only relevant when
 *		mode = MTD_OPS_PLACE_OOB or MTD_OPS_RAW)
 * @datbuf:	data buffer - if NULL only oob data are read/written
 * @oobbuf:	oob data buffer
 *
 * Note, it is allowed to read more then one OOB area at one go, but not write.
 * The interface assumes that the OOB write requests program only one page's
 * OOB area.
 */
struct mtd_oob_ops {
	unsigned int	mode;
	size_t		len;
	size_t		retlen;
	size_t		ooblen;
	size_t		oobretlen;
	uint32_t	ooboffs;
	uint8_t		*datbuf;
	uint8_t		*oobbuf;
};

struct mtd_info {
	u_char type;
	u_int32_t flags;
	uint64_t size;	 /* Total size of the MTD */

	/* "Major" erase size for the device. Naïve users may take this
	 * to be the only erase size available, or may use the more detailed
	 * information below if they desire
	 */
	u_int32_t erasesize;
	/* Minimal writable flash unit size. In case of NOR flash it is 1 (even
	 * though individual bits can be cleared), in case of NAND flash it is
	 * one NAND page (or half, or one-fourths of it), in case of ECC-ed NOR
	 * it is of ECC block size, etc. It is illegal to have writesize = 0.
	 * Any driver registering a struct mtd_info must ensure a writesize of
	 * 1 or larger.
	 */
	u_int32_t writesize;

	u_int32_t oobsize;   /* Amount of OOB data per block (e.g. 16) */
	u_int32_t oobavail;  /* Available OOB bytes per block */

	/*
	 * read ops return -EUCLEAN if max number of bitflips corrected on any
	 * one region comprising an ecc step equals or exceeds this value.
	 * Settable by driver, else defaults to ecc_strength.  User can override
	 * in sysfs.  N.B. The meaning of the -EUCLEAN return code has changed;
	 * see Documentation/ABI/testing/sysfs-class-mtd for more detail.
	 */
	unsigned int bitflip_threshold;

	/* Kernel-only stuff starts here. */
	const char *name;
	int index;

	/* ECC layout structure pointer - read only! */
	struct nand_ecclayout *ecclayout;

	/* max number of correctible bit errors per ecc step */
	unsigned int ecc_strength;

	/* Data for variable erase regions. If numeraseregions is zero,
	 * it means that the whole device has erasesize as given above.
	 */
	int numeraseregions;
	struct mtd_erase_region_info *eraseregions;

	/*
	 * Do not call via these pointers, use corresponding mtd_*()
	 * wrappers instead.
	 */
	int (*_erase) (struct mtd_info *mtd, struct erase_info *instr);
	int (*_point) (struct mtd_info *mtd, loff_t from, size_t len,
			size_t *retlen, void **virt, phys_addr_t *phys);
	void (*_unpoint) (struct mtd_info *mtd, loff_t from, size_t len);
	int (*_read) (struct mtd_info *mtd, loff_t from, size_t len,
		     size_t *retlen, u_char *buf);
	int (*_write) (struct mtd_info *mtd, loff_t to, size_t len,
		      size_t *retlen, const u_char *buf);

	/* In blackbox flight recorder like scenarios we want to make successful
	   writes in interrupt context. panic_write() is only intended to be
	   called when its known the kernel is about to panic and we need the
	   write to succeed. Since the kernel is not going to be running for much
	   longer, this function can break locks and delay to ensure the write
	   succeeds (but not sleep). */

	int (*_panic_write) (struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf);

	int (*_read_oob) (struct mtd_info *mtd, loff_t from,
			 struct mtd_oob_ops *ops);
	int (*_write_oob) (struct mtd_info *mtd, loff_t to,
			 struct mtd_oob_ops *ops);
	int (*_get_fact_prot_info) (struct mtd_info *mtd, struct otp_info *buf,
				   size_t len);
	int (*_read_fact_prot_reg) (struct mtd_info *mtd, loff_t from,
				   size_t len, size_t *retlen, u_char *buf);
	int (*_get_user_prot_info) (struct mtd_info *mtd, struct otp_info *buf,
				   size_t len);
	int (*_read_user_prot_reg) (struct mtd_info *mtd, loff_t from,
				   size_t len, size_t *retlen, u_char *buf);
	int (*_write_user_prot_reg) (struct mtd_info *mtd, loff_t to, size_t len,
				    size_t *retlen, u_char *buf);
	int (*_lock_user_prot_reg) (struct mtd_info *mtd, loff_t from,
				   size_t len);
	void (*_sync) (struct mtd_info *mtd);
	int (*_lock) (struct mtd_info *mtd, loff_t ofs, uint64_t len);
	int (*_unlock) (struct mtd_info *mtd, loff_t ofs, uint64_t len);
	int (*_block_isbad) (struct mtd_info *mtd, loff_t ofs);
	int (*_block_markbad) (struct mtd_info *mtd, loff_t ofs);
	/*
	 * If the driver is something smart, like UBI, it may need to maintain
	 * its own reference counting. The below functions are only for driver.
	 */
	int (*_get_device) (struct mtd_info *mtd);
	void (*_put_device) (struct mtd_info *mtd);

/* XXX U-BOOT XXX */
#if 0
	/* kvec-based read/write methods.
	   NB: The 'count' parameter is the number of _vectors_, each of
	   which contains an (ofs, len) tuple.
	*/
	int (*writev) (struct mtd_info *mtd, const struct kvec *vecs, unsigned long count, loff_t to, size_t *retlen);
#endif
/* XXX U-BOOT XXX */
#if 0
	struct notifier_block reboot_notifier;  /* default mode before reboot */
#endif

	/* ECC status information */
	struct mtd_ecc_stats ecc_stats;
	/* Subpage shift (NAND) */
	int subpage_sft;

	void *priv;

	struct module *owner;
	int usecount;
};

int mtd_erase(struct mtd_info *mtd, struct erase_info *instr);
int mtd_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen,
	     u_char *buf);
int mtd_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen,
	      const u_char *buf);
int mtd_panic_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen,
		    const u_char *buf);

int mtd_read_oob(struct mtd_info *mtd, loff_t from, struct mtd_oob_ops *ops);

static inline int mtd_write_oob(struct mtd_info *mtd, loff_t to,
				struct mtd_oob_ops *ops)
{
	ops->retlen = ops->oobretlen = 0;
	if (!mtd->_write_oob)
		return -EOPNOTSUPP;
	if (!(mtd->flags & MTD_WRITEABLE))
		return -EROFS;
	return mtd->_write_oob(mtd, to, ops);
}

int mtd_get_fact_prot_info(struct mtd_info *mtd, struct otp_info *buf,
			   size_t len);
int mtd_read_fact_prot_reg(struct mtd_info *mtd, loff_t from, size_t len,
			   size_t *retlen, u_char *buf);
int mtd_get_user_prot_info(struct mtd_info *mtd, struct otp_info *buf,
			   size_t len);
int mtd_read_user_prot_reg(struct mtd_info *mtd, loff_t from, size_t len,
			   size_t *retlen, u_char *buf);
int mtd_write_user_prot_reg(struct mtd_info *mtd, loff_t to, size_t len,
			    size_t *retlen, u_char *buf);
int mtd_lock_user_prot_reg(struct mtd_info *mtd, loff_t from, size_t len);

/* XXX U-BOOT XXX */
#if 0
int mtd_writev(struct mtd_info *mtd, const struct kvec *vecs,
	       unsigned long count, loff_t to, size_t *retlen);
#endif

static inline void mtd_sync(struct mtd_info *mtd)
{
	if (mtd->_sync)
		mtd->_sync(mtd);
}

int mtd_lock(struct mtd_info *mtd, loff_t ofs, uint64_t len);
int mtd_unlock(struct mtd_info *mtd, loff_t ofs, uint64_t len);
int mtd_is_locked(struct mtd_info *mtd, loff_t ofs, uint64_t len);
int mtd_block_isbad(struct mtd_info *mtd, loff_t ofs);
int mtd_block_markbad(struct mtd_info *mtd, loff_t ofs);

static inline uint32_t mtd_div_by_eb(uint64_t sz, struct mtd_info *mtd)
{
	do_div(sz, mtd->erasesize);
	return sz;
}

static inline uint32_t mtd_mod_by_eb(uint64_t sz, struct mtd_info *mtd)
{
	return do_div(sz, mtd->erasesize);
}

static inline int mtd_has_oob(const struct mtd_info *mtd)
{
	return mtd->_read_oob && mtd->_write_oob;
}

static inline int mtd_can_have_bb(const struct mtd_info *mtd)
{
	return !!mtd->_block_isbad;
}

	/* Kernel-side ioctl definitions */

extern int add_mtd_device(struct mtd_info *mtd);
extern int del_mtd_device (struct mtd_info *mtd);

extern struct mtd_info *get_mtd_device(struct mtd_info *mtd, int num);
extern struct mtd_info *get_mtd_device_nm(const char *name);

extern void put_mtd_device(struct mtd_info *mtd);
extern void mtd_get_len_incl_bad(struct mtd_info *mtd, uint64_t offset,
				 const uint64_t length, uint64_t *len_incl_bad,
				 int *truncated);
/* XXX U-BOOT XXX */
#if 0
struct mtd_notifier {
	void (*add)(struct mtd_info *mtd);
	void (*remove)(struct mtd_info *mtd);
	struct list_head list;
};

extern void register_mtd_user (struct mtd_notifier *new);
extern int unregister_mtd_user (struct mtd_notifier *old);
#endif

#ifdef CONFIG_MTD_PARTITIONS
void mtd_erase_callback(struct erase_info *instr);
#else
static inline void mtd_erase_callback(struct erase_info *instr)
{
	if (instr->callback)
		instr->callback(instr);
}
#endif

/*
 * Debugging macro and defines
 */
#define MTD_DEBUG_LEVEL0	(0)	/* Quiet   */
#define MTD_DEBUG_LEVEL1	(1)	/* Audible */
#define MTD_DEBUG_LEVEL2	(2)	/* Loud    */
#define MTD_DEBUG_LEVEL3	(3)	/* Noisy   */

#ifdef CONFIG_MTD_DEBUG
#define pr_debug(args...)	MTDDEBUG(MTD_DEBUG_LEVEL0, args)
#define MTDDEBUG(n, args...)				\
	do {						\
		if (n <= CONFIG_MTD_DEBUG_VERBOSE)	\
			printk(KERN_INFO args);		\
	} while(0)
#else /* CONFIG_MTD_DEBUG */
#define pr_debug(args...)
#define MTDDEBUG(n, args...)				\
	do {						\
		if (0)					\
			printk(KERN_INFO args);		\
	} while(0)
#endif /* CONFIG_MTD_DEBUG */
#define pr_info(args...)	MTDDEBUG(MTD_DEBUG_LEVEL0, args)
#define pr_warn(args...)	MTDDEBUG(MTD_DEBUG_LEVEL0, args)
#define pr_err(args...)		MTDDEBUG(MTD_DEBUG_LEVEL0, args)

static inline int mtd_is_bitflip(int err) {
	return err == -EUCLEAN;
}

static inline int mtd_is_eccerr(int err) {
	return err == -EBADMSG;
}

static inline int mtd_is_bitflip_or_eccerr(int err) {
	return mtd_is_bitflip(err) || mtd_is_eccerr(err);
}

#endif /* __MTD_MTD_H__ */

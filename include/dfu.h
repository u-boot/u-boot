/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * dfu.h - DFU flashable area description
 *
 * Copyright (C) 2012 Samsung Electronics
 * authors: Andrzej Pietrasiewicz <andrzej.p@samsung.com>
 *	    Lukasz Majewski <l.majewski@samsung.com>
 */

#ifndef __DFU_ENTITY_H_
#define __DFU_ENTITY_H_

#include <common.h>
#include <linux/list.h>
#include <mmc.h>
#include <spi_flash.h>
#include <linux/usb/composite.h>

enum dfu_device_type {
	DFU_DEV_MMC = 1,
	DFU_DEV_ONENAND,
	DFU_DEV_NAND,
	DFU_DEV_RAM,
	DFU_DEV_SF,
	DFU_DEV_MTD,
	DFU_DEV_VIRT,
};

enum dfu_layout {
	DFU_RAW_ADDR = 1,
	DFU_FS_FAT,
	DFU_FS_EXT2,
	DFU_FS_EXT3,
	DFU_FS_EXT4,
	DFU_RAM_ADDR,
};

enum dfu_op {
	DFU_OP_READ = 1,
	DFU_OP_WRITE,
	DFU_OP_SIZE,
};

struct mmc_internal_data {
	int dev_num;

	/* RAW programming */
	unsigned int lba_start;
	unsigned int lba_size;
	unsigned int lba_blk_size;

	/* eMMC HW partition access */
	int hw_partition;

	/* FAT/EXT */
	unsigned int dev;
	unsigned int part;
};

struct mtd_internal_data {
	struct mtd_info *info;

	/* RAW programming */
	u64 start;
	u64 size;
	/* for ubi partition */
	unsigned int ubi;
};

struct nand_internal_data {
	/* RAW programming */
	u64 start;
	u64 size;

	unsigned int dev;
	unsigned int part;
	/* for nand/ubi use */
	unsigned int ubi;
};

struct ram_internal_data {
	void		*start;
	unsigned int	size;
};

struct sf_internal_data {
	struct spi_flash *dev;

	/* RAW programming */
	u64 start;
	u64 size;
	/* for sf/ubi use */
	unsigned int ubi;
};

struct virt_internal_data {
	int dev_num;
};

#define DFU_NAME_SIZE			32
#ifndef CONFIG_SYS_DFU_DATA_BUF_SIZE
#define CONFIG_SYS_DFU_DATA_BUF_SIZE		(1024*1024*8)	/* 8 MiB */
#endif
#ifndef CONFIG_SYS_DFU_MAX_FILE_SIZE
#define CONFIG_SYS_DFU_MAX_FILE_SIZE CONFIG_SYS_DFU_DATA_BUF_SIZE
#endif
#ifndef DFU_DEFAULT_POLL_TIMEOUT
#define DFU_DEFAULT_POLL_TIMEOUT 0
#endif
#ifndef DFU_MANIFEST_POLL_TIMEOUT
#define DFU_MANIFEST_POLL_TIMEOUT	DFU_DEFAULT_POLL_TIMEOUT
#endif

struct dfu_entity {
	char			name[DFU_NAME_SIZE];
	int                     alt;
	void                    *dev_private;
	enum dfu_device_type    dev_type;
	enum dfu_layout         layout;
	unsigned long           max_buf_size;

	union {
		struct mmc_internal_data mmc;
		struct mtd_internal_data mtd;
		struct nand_internal_data nand;
		struct ram_internal_data ram;
		struct sf_internal_data sf;
		struct virt_internal_data virt;
	} data;

	int (*get_medium_size)(struct dfu_entity *dfu, u64 *size);

	int (*read_medium)(struct dfu_entity *dfu,
			u64 offset, void *buf, long *len);

	int (*write_medium)(struct dfu_entity *dfu,
			u64 offset, void *buf, long *len);

	int (*flush_medium)(struct dfu_entity *dfu);
	unsigned int (*poll_timeout)(struct dfu_entity *dfu);

	void (*free_entity)(struct dfu_entity *dfu);

	struct list_head list;

	/* on the fly state */
	u32 crc;
	u64 offset;
	int i_blk_seq_num;
	u8 *i_buf;
	u8 *i_buf_start;
	u8 *i_buf_end;
	u64 r_left;
	long b_left;

	u32 bad_skip;	/* for nand use */

	unsigned int inited:1;
};

#ifdef CONFIG_SET_DFU_ALT_INFO
void set_dfu_alt_info(char *interface, char *devstr);
#endif
int dfu_alt_init(int num, struct dfu_entity **dfu);
int dfu_alt_add(struct dfu_entity *dfu, char *interface, char *devstr, char *s);
int dfu_config_entities(char *s, char *interface, char *devstr);
void dfu_free_entities(void);
void dfu_show_entities(void);
int dfu_get_alt_number(void);
const char *dfu_get_dev_type(enum dfu_device_type t);
const char *dfu_get_layout(enum dfu_layout l);
struct dfu_entity *dfu_get_entity(int alt);
char *dfu_extract_token(char** e, int *n);
int dfu_get_alt(char *name);
int dfu_init_env_entities(char *interface, char *devstr);
unsigned char *dfu_get_buf(struct dfu_entity *dfu);
unsigned char *dfu_free_buf(void);
unsigned long dfu_get_buf_size(void);
bool dfu_usb_get_reset(void);

#ifdef CONFIG_DFU_TIMEOUT
unsigned long dfu_get_timeout(void);
void dfu_set_timeout(unsigned long);
#endif

int dfu_read(struct dfu_entity *de, void *buf, int size, int blk_seq_num);
int dfu_write(struct dfu_entity *de, void *buf, int size, int blk_seq_num);
int dfu_flush(struct dfu_entity *de, void *buf, int size, int blk_seq_num);

/**
 * dfu_initiated_callback - weak callback called on DFU transaction start
 *
 * It is a callback function called by DFU stack when a DFU transaction is
 * initiated. This function allows to manage some board specific behavior on
 * DFU targets.
 *
 * @param dfu - pointer to the dfu_entity, which should be initialized
 *
 */
void dfu_initiated_callback(struct dfu_entity *dfu);
/**
 * dfu_flush_callback - weak callback called at the end of the DFU write
 *
 * It is a callback function called by DFU stack after DFU manifestation.
 * This function allows to manage some board specific behavior on DFU targets
 *
 * @param dfu - pointer to the dfu_entity, which should be flushed
 *
 */
void dfu_flush_callback(struct dfu_entity *dfu);

/*
 * dfu_defer_flush - pointer to store dfu_entity for deferred flashing.
 *		     It should be NULL when not used.
 */
extern struct dfu_entity *dfu_defer_flush;
/**
 * dfu_get_defer_flush - get current value of dfu_defer_flush pointer
 *
 * @return - value of the dfu_defer_flush pointer
 */
static inline struct dfu_entity *dfu_get_defer_flush(void)
{
	return dfu_defer_flush;
}

/**
 * dfu_set_defer_flush - set the dfu_defer_flush pointer
 *
 * @param dfu - pointer to the dfu_entity, which should be written
 */
static inline void dfu_set_defer_flush(struct dfu_entity *dfu)
{
	dfu_defer_flush = dfu;
}

/**
 * dfu_write_from_mem_addr - write data from memory to DFU managed medium
 *
 * This function adds support for writing data starting from fixed memory
 * address (like $loadaddr) to dfu managed medium (e.g. NAND, MMC, file system)
 *
 * @param dfu - dfu entity to which we want to store data
 * @param buf - fixed memory addres from where data starts
 * @param size - number of bytes to write
 *
 * @return - 0 on success, other value on failure
 */
int dfu_write_from_mem_addr(struct dfu_entity *dfu, void *buf, int size);

/* Device specific */
#if CONFIG_IS_ENABLED(DFU_MMC)
extern int dfu_fill_entity_mmc(struct dfu_entity *dfu, char *devstr, char *s);
#else
static inline int dfu_fill_entity_mmc(struct dfu_entity *dfu, char *devstr,
				      char *s)
{
	puts("MMC support not available!\n");
	return -1;
}
#endif

#if CONFIG_IS_ENABLED(DFU_NAND)
extern int dfu_fill_entity_nand(struct dfu_entity *dfu, char *devstr, char *s);
#else
static inline int dfu_fill_entity_nand(struct dfu_entity *dfu, char *devstr,
				       char *s)
{
	puts("NAND support not available!\n");
	return -1;
}
#endif

#if CONFIG_IS_ENABLED(DFU_RAM)
extern int dfu_fill_entity_ram(struct dfu_entity *dfu, char *devstr, char *s);
#else
static inline int dfu_fill_entity_ram(struct dfu_entity *dfu, char *devstr,
				      char *s)
{
	puts("RAM support not available!\n");
	return -1;
}
#endif

#if CONFIG_IS_ENABLED(DFU_SF)
extern int dfu_fill_entity_sf(struct dfu_entity *dfu, char *devstr, char *s);
#else
static inline int dfu_fill_entity_sf(struct dfu_entity *dfu, char *devstr,
				     char *s)
{
	puts("SF support not available!\n");
	return -1;
}
#endif

#if CONFIG_IS_ENABLED(DFU_MTD)
int dfu_fill_entity_mtd(struct dfu_entity *dfu, char *devstr, char *s);
#else
static inline int dfu_fill_entity_mtd(struct dfu_entity *dfu, char *devstr,
				      char *s)
{
	puts("MTD support not available!\n");
	return -1;
}
#endif

#ifdef CONFIG_DFU_VIRT
int dfu_fill_entity_virt(struct dfu_entity *dfu, char *devstr, char *s);
int dfu_write_medium_virt(struct dfu_entity *dfu, u64 offset,
			  void *buf, long *len);
int dfu_get_medium_size_virt(struct dfu_entity *dfu, u64 *size);
int dfu_read_medium_virt(struct dfu_entity *dfu, u64 offset,
			 void *buf, long *len);
#else
static inline int dfu_fill_entity_virt(struct dfu_entity *dfu, char *devstr,
				       char *s)
{
	puts("VIRT support not available!\n");
	return -1;
}
#endif

/**
 * dfu_tftp_write - Write TFTP data to DFU medium
 *
 * This function is storing data received via TFTP on DFU supported medium.
 *
 * @param dfu_entity_name - name of DFU entity to write
 * @param addr - address of data buffer to write
 * @param len - number of bytes
 * @param interface - destination DFU medium (e.g. "mmc")
 * @param devstring - instance number of destination DFU medium (e.g. "1")
 *
 * @return 0 on success, otherwise error code
 */
#if CONFIG_IS_ENABLED(DFU_TFTP)
int dfu_tftp_write(char *dfu_entity_name, unsigned int addr, unsigned int len,
		   char *interface, char *devstring);
#else
static inline int dfu_tftp_write(char *dfu_entity_name, unsigned int addr,
				 unsigned int len, char *interface,
				 char *devstring)
{
	puts("TFTP write support for DFU not available!\n");
	return -ENOSYS;
}
#endif

int dfu_add(struct usb_configuration *c);
#endif /* __DFU_ENTITY_H_ */

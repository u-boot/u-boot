// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#include <common.h>
#include <div64.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <malloc.h>
#include <mapmem.h>
#include <memalign.h>
#include <mmc.h>
#include <spi.h>
#include <spi_flash.h>
#include <jffs2/jffs2.h>
#include <asm/io.h>
#include <linux/mtd/mtd.h>

/* Offsets and sizes to various structures in the image */
#define SCR_MAX_SIZE			0x40000
#define MAX_FW_SIZE			0x5C0000
#define MIN_SIZE			0x500000
#define INFO_SIZE			0x100
#define SIG_SIZE			0x100
#define AP_NB1FW_INFO			0x10000
#define SCP_NB1FW_INFO			0x10400
#define SCP_NB1FW_INFO_BLK		(SCP_NB1FW_INFO / 512)
#define SCP_TBL1FW_INFO			0x10600
#define SCP_TBL1FW_INFO_BLK		(SCP_TBL1FW_INFO / 512)
#define AP_NBL1FW_OPAQUE		0x20000
#define AP_NBL1FW_OPAQUE_SIZE		0x30000
#define AP_TBL1FW_OPAQUE		0x50000
#define AP_TBL1FW_OPAQUE_SIZE		0x30000
#define INFO_MAGIC			"CVM_CLIB"
#define INFO_MAGIC_SIZE			8
#define INFO_MAX_OFFSET			0x1000000
#define INFO_VERSION			0
#define AP_NBL1FW_HEADER		"OCTEONTX"
#define ATF_BL2_STAGE1			0x400000	/** uses bdk_header */
#define ATF_BL2_STAGE1_SIZE		0x40000
#define ATF_FIP_ADDRESS			0x440000
#define ATF_FIP_NAME			0xAA640001

/**
 * The BDK is located inAP_NBL1FW
 */
struct bdk_header {
	u32	skip_insn;
	u32	length;		/** Length, including header */
	char	magic[8];	/** OCTEONTX */
	u32	crc32;		/** CRC32 */
	u32	zero;
	char	name[64];	/** Zero terminated name */
	char	version[32];	/** Zero terminated version */
	char	pad[136];
};

/** ROM Code Load Information-Block Structure, all fields are little-endian */
struct rom_clib {
	u8	magic[8];	/** CVM_CLIB */
	u8	pad[7];		/** 0 */
	u8	ver;		/** 0 */
	u64	offset;
	u64	size;
	u32	rom_script_size;
	u32	rom_script_offset;
	u32	rom_script_chksum;
	u32	pad2[5];
	u64	csib_sig[8];
	u64	pad3[12];
};

/**
 * Information saved to later restore the buffer when preserving the ROM
 * script area
 *
 * @param	desc		block descriptor if mmc is used
 * @param	flash		SPI flash descriptor if flash is used
 * @param	bak_nb1fw_scr	Pointer to backup of script in new image,
 *				NULL if not present.
 * @param	nb1fw_scr_size	Size of SCP NB1FW script, 0 if not present
 * @param	nb1fw_scr_offset Offset of SCP NB1FW script in image
 * @param	bak_tbl1fw_scr	Pointer to backup of script in new image,
 *				NULL if not present.
 * @param	tbl1fw_scr_size	Size of SCP TBL1FW script, 0 if not present
 * @param	tbl1fw_scr_offset	Offset of SCP TBL1FW script in image
 * @param	bak_scp_nb1fw_info	SCP NB1FW INFO from new image
 * @param	bak_scp_tbl1fw_info	SCP TBL1FW INFO from new image
 */
struct rom_scr_info {
	struct blk_desc		*desc;
	struct spi_flash	*flash;
	u8			*bak_nb1fw_scr;
	u32			nb1fw_scr_size;
	u32			nb1fw_scr_offset;
	u8			*bak_tbl1fw_scr;
	u32			tbl1fw_scr_size;
	u32			tbl1fw_scr_offset;
	struct rom_clib		bak_scp_nb1fw_info;
	struct rom_clib		bak_scp_tbl1fw_info;
};

struct dos_partition {
	unsigned char boot_ind;		/* 0x80 - active			*/
	unsigned char head;		/* starting head			*/
	unsigned char sector;		/* starting sector			*/
	unsigned char cyl;		/* starting cylinder			*/
	unsigned char sys_ind;		/* What partition type			*/
	unsigned char end_head;		/* end head				*/
	unsigned char end_sector;	/* end sector				*/
	unsigned char end_cyl;		/* end cylinder				*/
	unsigned int  start4;		/* starting sector counting from 0	*/
	unsigned int  size4;		/* nr of sectors in partition		*/
};

static struct spi_flash *flash;

/**
 * Validates the BDK headers
 *
 * @param	addr	address of BDK image
 *
 * @return	0 for success or !0 for error
 */
static int validate_bdk(unsigned long addr, size_t size)
{
	struct bdk_header *bhdr = (struct bdk_header *)addr;
	u32 crc;
	const u32 zero = 0;
	u32 len = le32_to_cpu(bhdr->length);

	if (len > size) {
		printf("Invalid header length %#x for BDK type image at %#lx\n",
		       len, addr);
		return 1;
	}

	if (strncmp(bhdr->magic, AP_NBL1FW_HEADER, 8)) {
		printf("%s: Invalid header \"%8s\"\n", __func__, bhdr->magic);
		return 1;
	}
	crc = crc32(0, (u8 *)bhdr, 0x10);
	crc = crc32(crc, (u8 *)&zero, sizeof(zero));
	crc = crc32(crc, (u8 *)&bhdr->zero, bhdr->length - 0x14);
	debug("%s: calculated CRC: %#x, CRC: %#x, length: %#x\n",
	      __func__, crc, bhdr->crc32, bhdr->length);
	return crc != bhdr->crc32;
}

/**
 * Validates a CLIB header
 *
 * @param	addr	address of CLIB header
 *
 * @return	0 for success, !0 for error
 */
static int validate_clib(unsigned long addr)
{
	struct rom_clib *clib = (struct rom_clib *)addr;

	if (memcmp(clib->magic, INFO_MAGIC, sizeof(clib->magic))) {
		printf("Invalid CLIB magic value for CLIB at %#lx\n", addr);
		return 1;
	}
	if (clib->ver != INFO_VERSION) {
		printf("CLIB version %#x invalid, should be %#x\n",
		       clib->ver, INFO_VERSION);
		return 1;
	}
	if (le32_to_cpu(clib->offset) + le32_to_cpu(clib->size) >
							INFO_MAX_OFFSET) {
		printf("Image offset %#llx and size %#llx out of range for CLIB at %#lx\n",
		       le64_to_cpu(clib->offset), le64_to_cpu(clib->size),
		       addr);
		return 1;
	}
	if ((le32_to_cpu(clib->rom_script_offset) +
	     le32_to_cpu(clib->rom_script_size) > INFO_MAX_OFFSET) ||
	    (le32_to_cpu(clib->rom_script_offset) % 512)) {
		printf("Invalid CLIB ROM script offset %#x, size %#x for CLIP at %#lx\n",
		       le32_to_cpu(clib->rom_script_offset),
		       le32_to_cpu(clib->rom_script_size), addr);
		return 1;
	}
	return 0;
}

/**
 * Validate a number of headers in the image to make sure it's sane
 *
 * @param	addr	Base address of the image
 *
 * @return	0 for success, !0 for error
 */
static int validate_bootimg_header(unsigned long addr)
{
	u32  fip_toc_header = *(u32 *)(addr + ATF_FIP_ADDRESS);

	if (validate_clib(addr + AP_NB1FW_INFO)) {
		printf("%s: AP NB1FW INFO CLIB bad\n", __func__);
		return 1;
	}
	if (validate_clib(addr + SCP_NB1FW_INFO)) {
		printf("%s: SCP NB1FW INFO CLIB bad\n", __func__);
		return 1;
	}
	if (validate_clib(addr + SCP_TBL1FW_INFO)) {
		printf("%s: SCP TBL1FW INFO CLIB bad\n", __func__);
		return 1;
	}

	if (validate_bdk(addr + AP_NBL1FW_OPAQUE, AP_NBL1FW_OPAQUE_SIZE)) {
		printf("Invalid BDK image at %#lx\n", addr + AP_NBL1FW_OPAQUE);
		return 1;
	}
	if (validate_bdk(addr + AP_TBL1FW_OPAQUE, AP_TBL1FW_OPAQUE_SIZE)) {
		printf("Invalid BDK image at %#lx\n", addr + AP_TBL1FW_OPAQUE);
		return 1;
	}
	if (le32_to_cpu(fip_toc_header) != ATF_FIP_NAME) {
		printf("Invalid FIP TOC header\n");
		return 1;
	}
	return 0;
}

#if !CONFIG_IS_ENABLED(ARCH_OCTEONTX)
/**
 * Calculate checksum for ROM script
 *
 * @param[in]	rom_script	pointer to ROM script
 * @param	len		length of script in bytes
 *
 * @return	checksum of ROM script
 */
static u32 scr_chksum(const void *rom_script, u32 len)
{
	u32 csum = 0;
	const u32 *ptr;

	debug("%s(%p, %u)\n", __func__, rom_script, len);
	for (ptr = (u32 *)rom_script;
	     ptr < (u32 *)(rom_script + len);
	     ptr++) {
		csum += le32_to_cpu(*ptr);
		/* Wrap overflow. */
		csum += (csum < le32_to_cpu(*ptr)) ? 1 : 0;
	}
	debug("%s: csum: %#x, ptr: %p\n", __func__, csum, ptr);
	return csum;
}
#endif

/**
 * This function takes a byte length and a delta unit of time to compute the
 * approximate bytes per second
 *
 * @param len		amount of bytes currently processed
 * @param start_ms	start time of processing in ms
 * @return bytes per second if OK, 0 on error
 */
static ulong bytes_per_second(unsigned int len, ulong start_ms)
{
	/* less accurate but avoids overflow */
	if (len >= ((unsigned int)-1) / 1024)
		return len / (max(get_timer(start_ms) / 1024, 1UL));
	else
		return 1024 * len / max(get_timer(start_ms), 1UL);
}

/**
 * Write a block of data to SPI flash, first checking if it is different from
 * what is already there.
 *
 * If the data being written is the same, then *skipped is incremented by len.
 *
 * @param flash		flash context pointer
 * @param offset	flash offset to write
 * @param len		number of bytes to write
 * @param buf		buffer to write from
 * @return NULL if OK, else a string containing the stage which failed
 */
static const char *spi_flash_update_block(struct spi_flash *flash, u32 offset,
					  size_t len, const char *buf,
					  char *cmp_buf)
{
	char *ret = NULL;
	char *ptr = (char *)buf;
	char *rbuf = cmp_buf;

	/* Read the entire sector so to allow for rewriting */
	if (spi_flash_read(flash, offset, flash->sector_size, rbuf)) {
		ret = "read";
		debug("%s: Read at offset %#x, len: %#lx\n",
		      __func__, offset, len);
		goto error;
	}
	/* Compare only what is meaningful (len) */
	if (memcmp(rbuf, buf, len) == 0) {
		return ret;
	}

	/* Erase the entire sector */
	if (spi_flash_erase(flash, offset, flash->sector_size)) {
		ret = "erase";
		goto error;
	}

	/* Write one complete sector */
	if (spi_flash_write(flash, offset, len, ptr)) {
		ret = "write";
		goto error;
	}

	if (spi_flash_read(flash, offset, len, rbuf)) {
		ret = "read";
		goto error;
	}

	if (memcmp(ptr, rbuf, len)) {
		ret = "compare";
		debug("%s: Comparison at offset %#x, len: %#lx\n",
		      __func__, offset, len);
#ifdef DEBUG
		debug("Written values:\n");
		print_buffer(offset, ptr, 1, min(len, 1024UL), 0);
		debug("Read values:\n");
		print_buffer(offset, rbuf, 1, min(len, 1024UL), 0);
#endif
	}

error:
	return ret;
}

/**
 * Update an area of SPI flash by erasing and writing any blocks which need
 * to change. Existing blocks with the correct data are left unchanged.
 *
 * @param flash		flash context pointer
 * @param offset	flash offset to write
 * @param len		number of bytes to write
 * @param buf		buffer to write from
 * @return 0 if ok, 1 on error
 */
static int spi_flash_update(struct spi_flash *flash, u32 offset,
			    size_t len, const char *buf)
{
	const char *err_oper = NULL;
	const char *end = buf + len;
	size_t todo;		/* number of bytes to do in this pass */
	const ulong start_time = get_timer(0);
	size_t scale = 1;
	const char *start_buf = buf;
	char *cmp_buf;
	ulong delta;

	if (end - buf >= 200)
		scale = (end - buf) / 100;
	cmp_buf = memalign(ARCH_DMA_MINALIGN, flash->sector_size);
	if (!cmp_buf) {
		printf("%s: Out of memory\n", __func__);
		return 1;
	}
	ulong last_update = get_timer(0);

	for (; (buf < end) && (!err_oper); buf += todo, offset += todo) {
		todo = min_t(size_t, end - buf, flash->sector_size);
		if (get_timer(last_update) > 100) {
			printf("   \rUpdating, %zu%% %lu B/s",
			       100 - (end - buf) / scale,
				bytes_per_second(buf - start_buf,
						 start_time));
			last_update = get_timer(0);
		}
		err_oper = spi_flash_update_block(flash, offset, todo, buf,
						  cmp_buf);
		if (err_oper)
			break;
	}
	free(cmp_buf);
	putc('\r');
	if (err_oper) {
		printf("SPI flash failed in %s step\n", err_oper);
		return 1;
	}

	delta = get_timer(start_time);
	printf("%zu bytes written", len);
	printf(" in %ld.%lds, speed %ld B/s\n",
	       delta / 1000, delta % 1000, bytes_per_second(len, start_time));

	return 0;
}

static int do_spi_flash_probe(unsigned int bus, unsigned int cs)
{
	unsigned int speed = CONFIG_SF_DEFAULT_SPEED;
	unsigned int mode = CONFIG_SF_DEFAULT_MODE;
	struct udevice *new;
	int ret;

	ret = spi_flash_probe_bus_cs(bus, cs, speed, mode, &new);
	if (ret) {
		printf("Failed to initialize SPI flash at %u:%u (error %d)\n",
		       bus, cs, ret);
		return 1;
	}

	flash = dev_get_uclass_priv(new);

	return flash ? 0 : 1;
}

/**
 * Converts a length into the number of mmc blocks
 */
static inline size_t get_num_blocks(size_t len)
{
	return DIV_ROUND_UP(len, 512);
}

#if !CONFIG_IS_ENABLED(ARCH_OCTEONTX)
/**
 * Extract any ROM scripts present for the SCP NB1FW and SCP TBL1FW blocks
 *
 * @param[in]	buf			pointer to input buffer
 * @param[out]	scp_nb1fw_info		data structure with the NB1FW CLIB
 * @param[out]	scp_nb1fw		Pointer to buffer with ROM script
 *					or NULL if no NB1FW ROM script.
 * @param[out]	scp_nb1fw_size		size of firmware, 0 if no firmware
 * @param[out]	scp_nb1fw_offset	offset of NB1FW ROM script
 * @param[out]	scp_tbl1fw_info		data structure with the SBL1FW CLIB
 * @param[out]	scp_tbl1fw		Pointer to buffer with ROM script
 *					or NULL if no SBL1FW ROM script.
 * @param[out]	scp_tbl1fw_size		size of firmware, 0 if no firmware
 * @param[out]	scp_tbl1fw_offset	offset of SBL1FW ROM script
 *
 * @return	0 for success or -ENOMEM or -EINVAL on error
 *
 * NOTE: The allocated script data buffer will be rounded up to the nearest
 * 512 bytes for eMMC support.  No data is copied into the sec_data field.
 */
static int get_rom_script(struct rom_clib *clib,
			  u8 **scr_data,
			  u32 *scr_size,
			  u32 *scr_offset,
			  u32 *scr_chksum)
{
	*scr_size = le32_to_cpu(clib->rom_script_size);
	*scr_offset = le32_to_cpu(clib->rom_script_offset);
	*scr_chksum = le32_to_cpu(clib->rom_script_chksum);
	if (*scr_size > SCR_MAX_SIZE) {
		printf("%s: CLIB at %p script size %#x exceeds maximum size %#x\n",
		       __func__, clib, *scr_size, SCR_MAX_SIZE);
		return -EINVAL;
	}
	if (*scr_size) {
		/* Round size up for eMMC */
		*scr_data = malloc(roundup(*scr_size, 512));
		if (!(*scr_data))
			return -ENOMEM;
	} else {
		*scr_data = NULL;
	}
	return 0;
}

/**
 * Preserve the original ROM script for updating
 *
 * @param	si	script information pointer
 * @param	buf	pointer to file buffer
 *
 * @return	0 for success, !0 for error
 *
 * NOTE: This will modify the contents of the buffer.  Use finish_rom_scr()
 * when finished to restore the buffer.  On error the buffer may be
 * modified and may not be able to be restored.
 */
static int prepare_rom_scr(struct rom_scr_info *si, u8 *buf)
{
	bool is_mmc = !!si->desc;
	u8 buffer[512];
	u32 chksum;
	int n;
	int nb;
	int ret = 1;
	u8 *nb1fw_scr = NULL;
	u8 *tbl1fw_scr = NULL;
	u32 nb1fw_scr_chksum;
	u32 tbl1fw_scr_chksum;
	struct rom_clib old_scp_nb1fw_info;
	struct rom_clib *new_scp_nb1fw_info;
	struct rom_clib old_scp_tbl1fw_info;
	struct rom_clib *new_scp_tbl1fw_info;

	si->bak_nb1fw_scr = NULL;
	si->bak_tbl1fw_scr = NULL;
	memset(&si->bak_scp_nb1fw_info, 0, sizeof(si->bak_scp_nb1fw_info));
	memset(&si->bak_scp_tbl1fw_info, 0, sizeof(si->bak_scp_tbl1fw_info));

	if ((!si->flash && !si->desc) || (si->flash && si->desc)) {
		printf("%s: Error: either SPI flash or MMC must be passed\n",
		       __func__);
		return -1;
	}

	/*
	 * If we're preserving the SCP ROM data then we first read
	 * this section from the SPI NOR and copy it to the buffer
	 * after preserving the original contents of the buffer
	 * before writing it.
	 */
	/* Read the NB1FW CLIB descriptor */
	if (is_mmc) {
		n = blk_dread(si->desc, SCP_NB1FW_INFO_BLK, 1, buffer);
		ret = n != 1;
		memcpy(&old_scp_nb1fw_info, buffer,
		       sizeof(old_scp_nb1fw_info));
	} else {
		ret = spi_flash_read(si->flash, SCP_NB1FW_INFO,
				     sizeof(old_scp_nb1fw_info),
				     &old_scp_nb1fw_info);
	}
	if (ret) {
		printf("%s: Error reading SCP NB1FW info descriptor\n",
		       __func__);
		goto error;
	}

	/* Parse it for ROM scripts */
	ret = get_rom_script(&old_scp_nb1fw_info,
			     &nb1fw_scr, &si->nb1fw_scr_size,
			     &si->nb1fw_scr_offset,
			     &nb1fw_scr_chksum);
	if (ret) {
		printf("%s: Error parsing SCP NB1FW info descriptor\n",
		       __func__);
		goto error;
	}

	debug("%s: old rom SCP NB1FW script size: %u, offset: %#x, checksum: %#x\n",
	      __func__, si->nb1fw_scr_size, si->nb1fw_scr_offset,
	      nb1fw_scr_chksum);

	/* Backup the info field */
	memcpy(&si->bak_scp_nb1fw_info, buf + SCP_NB1FW_INFO,
	       sizeof(si->bak_scp_nb1fw_info));

	/* Read the TBL1FW CLIB descriptor */
	if (is_mmc) {
		n = blk_dread(si->desc, SCP_TBL1FW_INFO_BLK, 1, buffer);
		ret = n != 1;
		memcpy(&old_scp_tbl1fw_info, buffer,
		       sizeof(old_scp_tbl1fw_info));
	} else {
		ret = spi_flash_read(si->flash, SCP_TBL1FW_INFO,
				     sizeof(old_scp_tbl1fw_info),
				     &old_scp_tbl1fw_info);
	}
	if (ret) {
		printf("%s: Error reading SFP TBL1FW info descriptor\n",
		       __func__);
		goto error;
	}
	/* Parse it for ROM scripts */
	ret = get_rom_script(&old_scp_tbl1fw_info,
			     &tbl1fw_scr, &si->tbl1fw_scr_size,
			     &si->tbl1fw_scr_offset, &tbl1fw_scr_chksum);
	if (ret) {
		printf("%s: Error parsing SCP TBL1FW info descriptor\n",
		       __func__);
		goto error;
	}

	debug("%s: old rom SCP TBL1FW script size: %u, offset: %#x, checksum: %#x\n",
	      __func__, si->tbl1fw_scr_size, si->tbl1fw_scr_offset,
	      tbl1fw_scr_chksum);

	/* Back up the info field */
	memcpy(&si->bak_scp_tbl1fw_info, buf + SCP_TBL1FW_INFO,
	       sizeof(si->bak_scp_tbl1fw_info));

	if (si->nb1fw_scr_size) {
		/* Save area for NB1FW ROM script to restore later */
		si->bak_nb1fw_scr = malloc(si->nb1fw_scr_size);
		debug("%s: Allocated bak_nb1fw_scr at %p, size: %#x\n",
		      __func__, si->bak_nb1fw_scr, si->nb1fw_scr_size);

		if (!si->bak_nb1fw_scr) {
			printf("%s: Could not allocate %u bytes for SCP NB1FW ROM script\n",
			       __func__, si->nb1fw_scr_size);
			ret = -ENOMEM;
			goto error;
		}
		memcpy(si->bak_nb1fw_scr, buf + si->nb1fw_scr_offset,
		       si->nb1fw_scr_size);
#ifdef DEBUG
		print_buffer(0, si->bak_nb1fw_scr, 4,
			     si->nb1fw_scr_size / 4, 0);
#endif
	}

	if (si->tbl1fw_scr_size) {
		/* Save area for NB1FW ROM script to restore later */
		si->bak_tbl1fw_scr = malloc(si->tbl1fw_scr_size);
		debug("%s: Allocated bak_tbl1fw_scr at %p, size: %#x\n",
		      __func__, si->bak_tbl1fw_scr, si->tbl1fw_scr_size);
		if (!si->bak_tbl1fw_scr) {
			printf("%s: Could not allocate %u bytes for SCP TBL1FW ROM script\n",
			       __func__, si->tbl1fw_scr_size);
			if (si->bak_nb1fw_scr) {
				/* Don't later restore this */
				free(si->bak_nb1fw_scr);
				si->bak_nb1fw_scr = NULL;
			}
			ret = -ENOMEM;
			goto error;
		}
		memcpy(si->bak_tbl1fw_scr, buf + si->tbl1fw_scr_offset,
		       si->tbl1fw_scr_size);
#ifdef DEBUG
		print_buffer(0, si->bak_tbl1fw_scr, 4,
			     si->tbl1fw_scr_size / 4, 0);
#endif
	}

	/* Read NB1FW ROM script if present */
	if (si->nb1fw_scr_size) {
		debug("%s: Reading nb1fw script at offset: %#x, size: %u\n",
		      __func__, si->nb1fw_scr_offset, si->nb1fw_scr_size);
		if (is_mmc) {
			nb = get_num_blocks(si->nb1fw_scr_size);
			n = blk_dread(si->desc, si->nb1fw_scr_offset / 512,
				      nb, nb1fw_scr);
			ret = n != nb;
		} else {
			ret = spi_flash_read(si->flash, si->nb1fw_scr_offset,
					     si->nb1fw_scr_size, nb1fw_scr);
		}
		if (ret) {
			printf("%s: Error reading %u bytes for NB1FW script at %#x\n",
			       __func__, si->nb1fw_scr_size,
			       si->nb1fw_scr_offset);
			goto error;
		}
		chksum = scr_chksum(nb1fw_scr, si->nb1fw_scr_size);
		if (chksum != nb1fw_scr_chksum) {
			printf("Error: invalid checksum in SCP NB1FW ROM script, old checksum: %#x, calculated: %#x\n",
			       nb1fw_scr_chksum, chksum);
			ret = -EINVAL;
			goto error;
		}

		/* Copy old ROM script into buffer */
		memcpy(buf + si->nb1fw_scr_offset, nb1fw_scr,
		       si->nb1fw_scr_size);

#ifdef DEBUG
		print_buffer(0, nb1fw_scr, 4, si->nb1fw_scr_size / 4, 0);
#endif
		new_scp_nb1fw_info =
				(struct rom_clib *)(buf + SCP_NB1FW_INFO);
		/* Update ROM CLIB to old ROM script info */
		new_scp_nb1fw_info->rom_script_offset =
					cpu_to_le32(si->nb1fw_scr_offset);
		new_scp_nb1fw_info->rom_script_size =
					cpu_to_le32(si->nb1fw_scr_size);
		new_scp_nb1fw_info->rom_script_chksum =
					cpu_to_le32(nb1fw_scr_chksum);
		printf("Preserving SCP NB1FW ROM script at offset: %#x, size: %u, checksum: %#x\n",
		       si->nb1fw_scr_offset, si->nb1fw_scr_size,
		       nb1fw_scr_chksum);
	}

	/* Read TBL1FW ROM script if present */
	if (si->tbl1fw_scr_size) {
		if (is_mmc) {
			nb = get_num_blocks(si->tbl1fw_scr_size);
			n = blk_dread(si->desc, si->tbl1fw_scr_offset / 512,
				      nb, tbl1fw_scr);
			ret = n != nb;
		} else {
			ret = spi_flash_read(flash, si->tbl1fw_scr_offset,
					     si->tbl1fw_scr_size,
					     tbl1fw_scr);
		}
		if (ret) {
			printf("%s: Error reading %u bytes for TBL1FW script at %#x\n",
			       __func__, si->tbl1fw_scr_size,
			       si->tbl1fw_scr_offset);
			goto error;
		}
		chksum = scr_chksum(tbl1fw_scr, si->tbl1fw_scr_size);
		if (chksum != tbl1fw_scr_chksum) {
			printf("Error: invalid checksum in SCP TBL1FW ROM script.  Old checksum: %#x, calculated: %#x\n",
			       tbl1fw_scr_chksum, chksum);
			ret = -EINVAL;
			goto error;
		}

		/* Copy old ROM script to buffer */
		memcpy(buf + si->tbl1fw_scr_offset, tbl1fw_scr,
		       si->tbl1fw_scr_size);
#ifdef DEBUG
		print_buffer(0, nb1fw_scr, 4, si->nb1fw_scr_size / 4, 0);
#endif
		new_scp_tbl1fw_info =
				(struct rom_clib *)(buf + SCP_TBL1FW_INFO);
		debug("%s: scp tbl1fw info: %p\n", __func__,
		      new_scp_tbl1fw_info);
		/* Update ROM CLIB to old ROM script info */
		new_scp_tbl1fw_info->rom_script_offset =
					cpu_to_le32(si->tbl1fw_scr_offset);
		new_scp_tbl1fw_info->rom_script_size =
					cpu_to_le32(si->tbl1fw_scr_size);
		new_scp_tbl1fw_info->rom_script_chksum =
					cpu_to_le32(tbl1fw_scr_chksum);
		printf("Preserving SCP TBL1FW ROM script at offset: %#x, size: %u, checksum: %#x\n",
		       si->tbl1fw_scr_offset, si->tbl1fw_scr_size,
		       tbl1fw_scr_chksum);
	}

error:
	/* Free up any memory */
	if (nb1fw_scr)
		free(nb1fw_scr);
	if (tbl1fw_scr)
		free(tbl1fw_scr);

	if (ret) {
		/* Attempt cleanup */
		if (si->bak_nb1fw_scr) {
			if (!memcmp(si->bak_scp_nb1fw_info.magic, INFO_MAGIC,
				    INFO_MAGIC_SIZE))
				memcpy(buf + SCP_NB1FW_INFO,
				       &si->bak_scp_nb1fw_info,
				       INFO_SIZE);
			memcpy(buf + si->nb1fw_scr_offset, si->bak_nb1fw_scr,
			       si->nb1fw_scr_size);
			free(si->bak_nb1fw_scr);
			si->bak_nb1fw_scr = NULL;
		}
		if (si->bak_tbl1fw_scr) {
			if (!memcmp(si->bak_scp_tbl1fw_info.magic, INFO_MAGIC,
				    INFO_MAGIC_SIZE))
				memcpy(buf + SCP_TBL1FW_INFO,
				       &si->bak_scp_tbl1fw_info,
				       INFO_SIZE);
			memcpy(buf + SCP_NB1FW_INFO, &si->bak_scp_nb1fw_info,
			       INFO_SIZE);
			memcpy(buf + si->tbl1fw_scr_offset, si->bak_tbl1fw_scr,
			       si->tbl1fw_scr_size);
			free(si->bak_tbl1fw_scr);
			si->bak_tbl1fw_scr = NULL;
		}
	}
	return ret;
}

/**
 * Restores the file buffer to its original state
 *
 * @param	si	script info pointer
 * @param	buf	pointer to buffer file was loaded to
 *
 * @return	0 for success
 */
static int finish_rom_scr(struct rom_scr_info *si, u8 *buf)
{
	if (si->bak_nb1fw_scr) {
		memcpy(buf + si->nb1fw_scr_offset,
		       si->bak_nb1fw_scr, si->nb1fw_scr_size);
		memcpy(buf + SCP_NB1FW_INFO, &si->bak_scp_nb1fw_info,
		       sizeof(si->bak_scp_nb1fw_info));
		free(si->bak_nb1fw_scr);
	}
	if (si->bak_tbl1fw_scr) {
		memcpy(buf + si->tbl1fw_scr_offset, si->bak_tbl1fw_scr,
		       si->tbl1fw_scr_size);
		memcpy(buf + SCP_TBL1FW_INFO, &si->bak_scp_tbl1fw_info,
		       sizeof(si->bak_scp_tbl1fw_info));
		free(si->bak_tbl1fw_scr);
	}
	return 0;
}
#endif

static int do_bootu_spi(int argc, char * const argv[], bool update_scr)
{
	unsigned long addr, offset, len;
	void *buf;
	char *env1, *env2;
	char *endp;
	int ret = 1;
	unsigned int bus = 0, cs;
#if !CONFIG_IS_ENABLED(ARCH_OCTEONTX)
	struct rom_scr_info si;
#endif

	if ((argc < 1) || (argc > 4))
		return -1;

	if (argc == 1) {
		bus = cs = 0;
		env1 = env_get("fileaddr");
		env2 = env_get("filesize");
		if (!env1 || !env2) {
			printf("Missing env variables fileaddr/filesize\n");
			return CMD_RET_USAGE;
		}
	} else if (argc == 2) {
		cs = simple_strtoul(argv[1], &endp, 0);
		if (*argv[1] == 0 || (*endp != 0 && *endp != ':'))
			return -1;
		if (*endp == ':') {
			if (endp[1] == 0)
				return CMD_RET_USAGE;

			bus = cs;
			cs = simple_strtoul(endp + 1, &endp, 0);
			if (*endp != 0)
				return CMD_RET_USAGE;
		}
		env1 = env_get("fileaddr");
		env2 = env_get("filesize");
		if (!env1 || !env2) {
			printf("Missing env variables fileaddr/filesize\n");
			return CMD_RET_USAGE;
		}
	} else if (argc == 4) {
		cs = simple_strtoul(argv[1], &endp, 0);
		if (*argv[1] == 0 || (*endp != 0 && *endp != ':'))
			return -1;
		if (*endp == ':') {
			if (endp[1] == 0)
				return CMD_RET_USAGE;

			bus = cs;
			cs = simple_strtoul(endp + 1, &endp, 0);
			if (*endp != 0)
				return CMD_RET_USAGE;
		}
		debug("%s argv0 %s argv1 %s\n", __func__, argv[0], argv[1]);
		debug("%s argv2 %s argv3 %s\n", __func__, argv[2], argv[3]);
		env1 = argv[2];
		env2 = argv[3];
	} else {
		printf("Missing args\n");
		return CMD_RET_USAGE;
	}
	debug("%s update SCP: %s\n", __func__, update_scr ? "yes" : "no");
	debug("%s fileaddr %s filesize %s\n", __func__, env1, env2);
	debug("%s bus %d cs %d\n", __func__, bus, cs);

	offset = 0;
	ret = strict_strtoul(env1, 16, &addr);
	if (ret)
		return CMD_RET_USAGE;
	debug("%s addr %#lx\n", __func__, addr);

	ret = strict_strtoul(env2, 16, &len);
	if (ret)
		return CMD_RET_USAGE;
	debug("%s len %#lx\n", __func__, len);

	if( !addr || !len) {
		printf("image address or length is 0\n");
		return CMD_RET_USAGE;
	}
	buf = (u8 *)addr;

	if (validate_bootimg_header(addr)) {
		printf("\n No valid boot image header found \n");
		return CMD_RET_FAILURE;
	}

	ret = do_spi_flash_probe(bus, cs);
	if (ret) {
		printf("Could not probe SPI flash %d:%d\n", bus, cs);
		return CMD_RET_FAILURE;
	}

	/* Consistency checking */
	if (offset + len > flash->size) {
		printf("ERROR: attempting %s past flash size (%#x)\n",
		       argv[0], flash->size);
		return CMD_RET_FAILURE;
	}

#if !CONFIG_IS_ENABLED(ARCH_OCTEONTX)
	if (!update_scr) {
		si.desc = NULL;
		si.flash = flash;
		ret = prepare_rom_scr(&si, buf);
		if (ret) {
			printf("%s: Error preparing ROM script info.\n",
			       __func__);
			goto error;
		}
	}
#endif

	ret = spi_flash_update(flash, offset, len, buf);

	printf("bootu SPI : %zu bytes @ %#x Written ",
	       (size_t)len, (u32)offset);
	if (ret)
		printf("ERROR %d\n", ret);
	else
		printf("OK\n");

#if !CONFIG_IS_ENABLED(ARCH_OCTEONTX)
	if (!update_scr)
		ret = finish_rom_scr(&si, buf);
error:
#endif
	if (ret)
		debug("%s: Return code: %d\n", __func__, ret);

	return ret == 0 ? CMD_RET_SUCCESS : CMD_RET_FAILURE;
}

/**
 * Make sure the partition table is sane
 *
 * @param[in]	buf	pointer to buffer with partition table
 *
 * @return	0 for success, !0 for error
 */
static int validate_partition_table(const unsigned char *buf)
{
	struct dos_partition *p;

	if ((buf[510] != 0x55) || (buf[511] != 0xaa)) {
		printf("No valid MBR signature in first sector\n");
		return 1; /* no DOS Signature at all */
	}

	/* checks for FAT12 as partition 1 */
	p = (struct dos_partition *)&buf[446];
	if (p->sys_ind != 0x01) {
		printf("%s Invalid first partition type %x"
			 " expected FAT12 \n", __func__, p->sys_ind);
		return 1;
	}
	/* check for second partition start <16MB */
	p = (struct dos_partition *)&buf[446 + 16];
	if (p->sys_ind != 0)
		if (p->start4 < 0x8000) {
			printf("%s partition type %x start sector at %d "
				"below 16MB(reserved for boot image)\n",
				 __func__, p->sys_ind, p->start4);
			return 1;
		}

	// FIXME below checks really needed?
	/* check for third partition start <16MB */
	p = (struct dos_partition *)&buf[446 + 16 * 2];
	if (p->sys_ind != 0)
		if (p->start4 < 0x8000) {
			printf("%s partition type %x start sector at %d "
				"below 16MB(reserved for boot image)\n",
				 __func__, p->sys_ind, p->start4);
			return 1;
		}
	/* check for fourth partition start <16MB */
	p = (struct dos_partition *)&buf[446 + 16 * 3];
	if (p->sys_ind != 0)
		if (p->start4 < 0x8000) {
			printf("%s partition type %x start sector at %d "
				"below 16MB(reserved for boot image)\n",
				 __func__, p->sys_ind, p->start4);
			return 1;
		}
	return 0;
}

static struct mmc *init_mmc_device(int dev, bool force_init)
{
	struct mmc *mmc;

	mmc = find_mmc_device(dev);
	if (!mmc) {
		printf("no mmc device at slot %x\n", dev);
		return NULL;
	}
	if (force_init)
		mmc->has_init = 0;
	if (mmc_init(mmc))
		return NULL;
	return mmc;
}

static int do_bootu_mmc(int argc, char * const argv[],
			bool update_scp, bool overwrite_part)
{
	static int curr_device = -1;
	struct mmc *mmc;
	struct blk_desc *desc;
	char *env1, *env2, *endp;
	unsigned long blk, len, n, blk_cnt = 0;
	unsigned long addr;
	u8 *buf;
	int ret = 0;
	u32 num_blks;
	u8 buffer[512];
#if !CONFIG_IS_ENABLED(ARCH_OCTEONTX)
	struct rom_scr_info si;
#endif

	if ((argc < 1) || (argc > 4)) {
		printf("Invalid # args \n");
		return CMD_RET_USAGE;
	}
	if (argc == 1) {
		curr_device = 0;
		env1 = env_get("fileaddr");
		env2 = env_get("filesize");
		if (!env1 || !env2) {
			printf("Missing env variables fileaddr/filesize\n");
			return CMD_RET_USAGE;
		}
	} else if (argc == 2) {
		curr_device = simple_strtoul(argv[1], &endp, 0);
		env1 = env_get("fileaddr");
		env2 = env_get("filesize");
		if (!env1 || !env2) {
			printf("Missing env variables fileaddr/filesize\n");
			return CMD_RET_USAGE;
		}
	} else if (argc == 3) {
			printf("Missing args - image addr or image size\n");
			return CMD_RET_USAGE;
	} else if (argc == 4) {
		curr_device = simple_strtoul(argv[1], &endp, 0);
		debug("%s argv0 %s argv1 %s\n", __func__, argv[0], argv[1]);
		env1 = argv[2];
		env2 = argv[3];
	}
	debug("%s update scp: %s, overwrite partition table: %s\n",
	      __func__, update_scp ? "yes" : "no",
	      overwrite_part ? "yes" : "no");
	debug("%s loadaddr %s filesize %s\n", __func__, env1, env2);
	debug("%s curr_device %d\n", __func__, curr_device);
	blk = 0;

	ret = strict_strtoul(env1, 16, &addr);
	if (ret)
		return -1;
	debug("%s addr %ld\n", __func__, addr);

	ret = strict_strtoul(env2, 16, &len);
	if (ret)
		return -1;
	debug("%s len %ld\n", __func__, len);
	if( !addr || !len) {
		printf("image address or length is 0\n");
		return CMD_RET_USAGE;
	}
	len = DIV_ROUND_UP(len, 512);
	debug("%s len %ld\n", __func__, len);

	if ((blk + 512 * len) > 0x1000000) {
		printf("\nBoot Image size exceeding 16MB\n");
		return CMD_RET_FAILURE;
	}

	if (validate_bootimg_header(addr)) {
		printf("\nNo valid boot image header found\n");
		return CMD_RET_FAILURE;
	}

	if (get_mmc_num() < curr_device) {
		puts("No MMC device available\n");
		return CMD_RET_FAILURE;
	}

	mmc = init_mmc_device(curr_device, false);
	if (!mmc)
		return CMD_RET_FAILURE;

	desc = mmc_get_blk_desc(mmc);
	if (!desc)
		return CMD_RET_FAILURE;

	buf = (u8 *)addr;

	n = blk_dread(desc, 0, 1, buffer);
	if (n != 1) {
		printf("ERROR: read partition table failed\n");
		return CMD_RET_FAILURE;
	}

	if (!overwrite_part && validate_partition_table(buffer)) {
		printf("Invalid partition setup, can't write bootimg\n");
		return CMD_RET_FAILURE;
	}

	if (mmc_getwp(mmc) == 1) {
		printf("Error: card is write protected!\n");
		return CMD_RET_FAILURE;
	}

	if (len <= get_num_blocks(MIN_SIZE)) {
		printf("\nError: Image size is too small, missing SCP section\n");
		return CMD_RET_FAILURE;
	}

#if !CONFIG_IS_ENABLED(ARCH_OCTEONTX)
	if (!update_scp) {
		si.flash = NULL;
		si.desc = desc;
		ret = prepare_rom_scr(&si, buf);
		if (ret)
			goto error;
	}
#endif

	printf("\nMMC write: dev # %d, block # %ld, count %ld ... ",
	       curr_device, blk, len);

	num_blks = len - blk;
	n = blk_dwrite(desc, blk, num_blks, buf);
	blk_cnt += n;
	if (n != num_blks)
		goto error;

#if !CONFIG_IS_ENABLED(ARCH_OCTEONTX)
	if (!update_scp)
		ret = finish_rom_scr(&si, buf);
#endif

error:

	printf("%lu blocks written: %s\n", blk_cnt,
	       (blk_cnt == len && !ret) ? "OK" : "ERROR");

	if (blk_cnt != len || ret)
		return CMD_RET_FAILURE;

	if (!overwrite_part) {
		/* Update partition table with FAT entry of boot image */
		memcpy(&buffer[446], (void *)(addr + 446), 16);

		/* Update partition table with read boot sector */
		n = blk_dwrite(desc, 0, 1, (void *)buffer);
		printf("%lu blocks written: %s\n", n,
		       (n == 1) ? "OK" : "ERROR");
		if (n != 1)
			return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

static int do_bootimgup(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	const char *cmd;
	int ret;
	int i;
	bool overwrite_part = false;
#if CONFIG_IS_ENABLED(ARCH_OCTEONTX)
	/* OcteonTX does not have a SCP section so it is always updated */
	const bool update_scp = true;
#else
	bool update_scp = false;
#endif

	/* Check flags at the beginning */
	if (argc > 1) {
		for (i = 1; i < min(argc, 3); i++) {
#if !CONFIG_IS_ENABLED(ARCH_OCTEONTX)
			if (!strcmp(argv[1], "-s")) {
				update_scp = true;
				argv++;
				argc--;
			}
#endif
			if (!strcmp(argv[1], "-p")) {
				overwrite_part = true;
				argv++;
				argc--;
			}
		}
	}

	/* need at least two arguments */
	if (argc < 2)
		goto usage;

	cmd = argv[1];
	--argc;
	++argv;

	if (strcmp(cmd, "spi") == 0) {
		if (overwrite_part)
			puts("-p is ignored for SPI\n");
		ret = do_bootu_spi(argc, argv, update_scp);
	}
	else if (strcmp(cmd, "mmc") == 0)
		ret = do_bootu_mmc(argc, argv, update_scp, overwrite_part);
	else
		ret = -1;

	if (ret != -1)
		return ret;

usage:
	return CMD_RET_USAGE;
}

U_BOOT_CMD(
#if !CONFIG_IS_ENABLED(ARCH_OCTEONTX)
	bootimgup, 7, 1, do_bootimgup, "Updates Boot Image",
	" <[-s]> <[-p]> <mmc | spi> <[devid] | [bus:cs]> [image_address] [image_size]\n"
	" where: \n"
	" -s - overwrite SCP ROM area\n"
#else
	bootimgup, 6, 1, do_bootimgup, "Updates Boot Image",
	" <[-p]> <mmc | spi> <[devid] | [bus:cs]> [image_address] [image_size]\n"
#endif
	" -p - (MMC only) overwrite the partition table\n"
	" spi - updates boot image on spi flash \n"
	" bus and cs should be passed together, passing only one \n"
	" of them treated as invalid. If [bus:cs] not given, 0:0 is used \n"
	" image_address - address at which image is located in RAM \n"
	" image_size    - size of image in hex \n"
	" eg. to load on spi0 chipselect 0 \n"
	" bootimgup spi 0:0 $loadaddr $filesize \n"
	" eg. to load on spi1 chipselect 1 \n"
	" bootimgup spi 1:1 $loadaddr $filesize \n"
	" \n"
	" mmc - updates boot image on mmc card/chip \n"
	" eg. to load on device 0 \n"
	" bootimgup mmc 0 $loadaddr $filesize \n"
	" eg. to load on device 1. If device id not given, 0 is used \n"
	" bootimgup mmc 1 $loadaddr $filesize \n"
	" image_address - address at which image is located in RAM \n"
	" image_size    - size of image in hex \n"
	" image_address, image_size should be passed together, \n"
	" passing only one of them treated as invalid. \n"
	" \n"
	" If not given, then $loadaddr and $filesize values in \n"
	" environment are used, otherwise fail to update. \n"
);

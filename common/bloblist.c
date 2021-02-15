// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <bloblist.h>
#include <log.h>
#include <mapmem.h>
#include <spl.h>
#include <asm/global_data.h>
#include <u-boot/crc.h>

/*
 * A bloblist is a single contiguous chunk of memory with a header
 * (struct bloblist_hdr) and a number of blobs in it.
 *
 * Each blob starts on a BLOBLIST_ALIGN boundary relative to the start of the
 * bloblist and consists of a struct bloblist_rec, some padding to the required
 * alignment for the blog and then the actual data. The padding ensures that the
 * start address of the data in each blob is aligned as required. Note that
 * each blob's *data* is aligned to BLOBLIST_ALIGN regardless of the alignment
 * of the bloblist itself or the blob header.
 *
 * So far, only BLOBLIST_ALIGN alignment is supported.
 */

DECLARE_GLOBAL_DATA_PTR;

static const char *const tag_name[] = {
	[BLOBLISTT_NONE]		= "(none)",
	[BLOBLISTT_EC_HOSTEVENT]	= "EC host event",
	[BLOBLISTT_SPL_HANDOFF]		= "SPL hand-off",
	[BLOBLISTT_VBOOT_CTX]		= "Chrome OS vboot context",
	[BLOBLISTT_VBOOT_HANDOFF]	= "Chrome OS vboot hand-off",
	[BLOBLISTT_ACPI_GNVS]		= "ACPI GNVS",
	[BLOBLISTT_INTEL_VBT]		= "Intel Video-BIOS table",
	[BLOBLISTT_TPM2_TCG_LOG]	= "TPM v2 log space",
	[BLOBLISTT_TCPA_LOG]		= "TPM log space",
	[BLOBLISTT_ACPI_TABLES]		= "ACPI tables for x86",
	[BLOBLISTT_SMBIOS_TABLES]	= "SMBIOS tables for x86",
};

const char *bloblist_tag_name(enum bloblist_tag_t tag)
{
	if (tag < 0 || tag >= BLOBLISTT_COUNT)
		return "invalid";

	return tag_name[tag];
}

static struct bloblist_rec *bloblist_first_blob(struct bloblist_hdr *hdr)
{
	if (hdr->alloced <= hdr->hdr_size)
		return NULL;
	return (struct bloblist_rec *)((void *)hdr + hdr->hdr_size);
}

static struct bloblist_rec *bloblist_next_blob(struct bloblist_hdr *hdr,
					       struct bloblist_rec *rec)
{
	ulong offset;

	offset = (void *)rec - (void *)hdr;
	offset += rec->hdr_size + ALIGN(rec->size, BLOBLIST_ALIGN);
	if (offset >= hdr->alloced)
		return NULL;
	return (struct bloblist_rec *)((void *)hdr + offset);
}

#define foreach_rec(_rec, _hdr) \
	for (_rec = bloblist_first_blob(_hdr); \
	     _rec; \
	     _rec = bloblist_next_blob(_hdr, _rec))

static struct bloblist_rec *bloblist_findrec(uint tag)
{
	struct bloblist_hdr *hdr = gd->bloblist;
	struct bloblist_rec *rec;

	if (!hdr)
		return NULL;

	foreach_rec(rec, hdr) {
		if (rec->tag == tag)
			return rec;
	}

	return NULL;
}

static int bloblist_addrec(uint tag, int size, int align,
			   struct bloblist_rec **recp)
{
	struct bloblist_hdr *hdr = gd->bloblist;
	struct bloblist_rec *rec;
	int data_start, new_alloced;

	if (!align)
		align = BLOBLIST_ALIGN;

	/* Figure out where the new data will start */
	data_start = map_to_sysmem(hdr) + hdr->alloced + sizeof(*rec);

	/* Align the address and then calculate the offset from ->alloced */
	data_start = ALIGN(data_start, align) - map_to_sysmem(hdr);

	/* Calculate the new allocated total */
	new_alloced = data_start + ALIGN(size, align);

	if (new_alloced >= hdr->size) {
		log(LOGC_BLOBLIST, LOGL_ERR,
		    "Failed to allocate %x bytes size=%x, need size=%x\n",
		    size, hdr->size, new_alloced);
		return log_msg_ret("bloblist add", -ENOSPC);
	}
	rec = (void *)hdr + hdr->alloced;

	rec->tag = tag;
	rec->hdr_size = data_start - hdr->alloced;
	rec->size = size;
	rec->spare = 0;

	/* Zero the record data */
	memset((void *)rec + rec->hdr_size, '\0', rec->size);

	hdr->alloced = new_alloced;
	*recp = rec;

	return 0;
}

static int bloblist_ensurerec(uint tag, struct bloblist_rec **recp, int size,
			      int align)
{
	struct bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	if (rec) {
		if (size && size != rec->size) {
			*recp = rec;
			return -ESPIPE;
		}
	} else {
		int ret;

		ret = bloblist_addrec(tag, size, align, &rec);
		if (ret)
			return ret;
	}
	*recp = rec;

	return 0;
}

void *bloblist_find(uint tag, int size)
{
	struct bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	if (!rec)
		return NULL;
	if (size && size != rec->size)
		return NULL;

	return (void *)rec + rec->hdr_size;
}

void *bloblist_add(uint tag, int size, int align)
{
	struct bloblist_rec *rec;

	if (bloblist_addrec(tag, size, align, &rec))
		return NULL;

	return (void *)rec + rec->hdr_size;
}

int bloblist_ensure_size(uint tag, int size, int align, void **blobp)
{
	struct bloblist_rec *rec;
	int ret;

	ret = bloblist_ensurerec(tag, &rec, size, align);
	if (ret)
		return ret;
	*blobp = (void *)rec + rec->hdr_size;

	return 0;
}

void *bloblist_ensure(uint tag, int size)
{
	struct bloblist_rec *rec;

	if (bloblist_ensurerec(tag, &rec, size, 0))
		return NULL;

	return (void *)rec + rec->hdr_size;
}

int bloblist_ensure_size_ret(uint tag, int *sizep, void **blobp)
{
	struct bloblist_rec *rec;
	int ret;

	ret = bloblist_ensurerec(tag, &rec, *sizep, 0);
	if (ret == -ESPIPE)
		*sizep = rec->size;
	else if (ret)
		return ret;
	*blobp = (void *)rec + rec->hdr_size;

	return 0;
}

static u32 bloblist_calc_chksum(struct bloblist_hdr *hdr)
{
	struct bloblist_rec *rec;
	u32 chksum;

	chksum = crc32(0, (unsigned char *)hdr,
		       offsetof(struct bloblist_hdr, chksum));
	foreach_rec(rec, hdr) {
		chksum = crc32(chksum, (void *)rec, rec->hdr_size);
		chksum = crc32(chksum, (void *)rec + rec->hdr_size, rec->size);
	}

	return chksum;
}

int bloblist_new(ulong addr, uint size, uint flags)
{
	struct bloblist_hdr *hdr;

	if (size < sizeof(*hdr))
		return log_ret(-ENOSPC);
	if (addr & (BLOBLIST_ALIGN - 1))
		return log_ret(-EFAULT);
	hdr = map_sysmem(addr, size);
	memset(hdr, '\0', sizeof(*hdr));
	hdr->version = BLOBLIST_VERSION;
	hdr->hdr_size = sizeof(*hdr);
	hdr->flags = flags;
	hdr->magic = BLOBLIST_MAGIC;
	hdr->size = size;
	hdr->alloced = hdr->hdr_size;
	hdr->chksum = 0;
	gd->bloblist = hdr;

	return 0;
}

int bloblist_check(ulong addr, uint size)
{
	struct bloblist_hdr *hdr;
	u32 chksum;

	hdr = map_sysmem(addr, sizeof(*hdr));
	if (hdr->magic != BLOBLIST_MAGIC)
		return log_msg_ret("Bad magic", -ENOENT);
	if (hdr->version != BLOBLIST_VERSION)
		return log_msg_ret("Bad version", -EPROTONOSUPPORT);
	if (size && hdr->size != size)
		return log_msg_ret("Bad size", -EFBIG);
	chksum = bloblist_calc_chksum(hdr);
	if (hdr->chksum != chksum) {
		log(LOGC_BLOBLIST, LOGL_ERR, "Checksum %x != %x\n", hdr->chksum,
		    chksum);
		return log_msg_ret("Bad checksum", -EIO);
	}
	gd->bloblist = hdr;

	return 0;
}

int bloblist_finish(void)
{
	struct bloblist_hdr *hdr = gd->bloblist;

	hdr->chksum = bloblist_calc_chksum(hdr);

	return 0;
}

void bloblist_get_stats(ulong *basep, ulong *sizep, ulong *allocedp)
{
	struct bloblist_hdr *hdr = gd->bloblist;

	*basep = map_to_sysmem(gd->bloblist);
	*sizep = hdr->size;
	*allocedp = hdr->alloced;
}

static void show_value(const char *prompt, ulong value)
{
	printf("%s:%*s %-5lx  ", prompt, 8 - (int)strlen(prompt), "", value);
	print_size(value, "\n");
}

void bloblist_show_stats(void)
{
	ulong base, size, alloced;

	bloblist_get_stats(&base, &size, &alloced);
	printf("base:     %lx\n", base);
	show_value("size", size);
	show_value("alloced", alloced);
	show_value("free", size - alloced);
}

void bloblist_show_list(void)
{
	struct bloblist_hdr *hdr = gd->bloblist;
	struct bloblist_rec *rec;

	printf("%-8s  %8s  Tag Name\n", "Address", "Size");
	for (rec = bloblist_first_blob(hdr); rec;
	     rec = bloblist_next_blob(hdr, rec)) {
		printf("%08lx  %8x  %3d %s\n",
		       (ulong)map_to_sysmem((void *)rec + rec->hdr_size),
		       rec->size, rec->tag, bloblist_tag_name(rec->tag));
	}
}

void bloblist_reloc(void *to, uint to_size, void *from, uint from_size)
{
	struct bloblist_hdr *hdr;

	memcpy(to, from, from_size);
	hdr = to;
	hdr->size = to_size;
}

int bloblist_init(void)
{
	bool expected;
	int ret = -ENOENT;

	/**
	 * Wed expect to find an existing bloblist in the first phase of U-Boot
	 * that runs
	 */
	expected = !u_boot_first_phase();
	if (spl_prev_phase() == PHASE_TPL && !IS_ENABLED(CONFIG_TPL_BLOBLIST))
		expected = false;
	if (expected)
		ret = bloblist_check(CONFIG_BLOBLIST_ADDR,
				     CONFIG_BLOBLIST_SIZE);
	if (ret) {
		log(LOGC_BLOBLIST, expected ? LOGL_WARNING : LOGL_DEBUG,
		    "Existing bloblist not found: creating new bloblist\n");
		ret = bloblist_new(CONFIG_BLOBLIST_ADDR, CONFIG_BLOBLIST_SIZE,
				   0);
	} else {
		log(LOGC_BLOBLIST, LOGL_DEBUG, "Found existing bloblist\n");
	}

	return ret;
}

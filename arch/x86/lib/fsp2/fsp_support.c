// SPDX-License-Identifier: Intel
/*
 * Copyright 2019 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <spi_flash.h>
#include <asm/fsp/fsp_support.h>
#include <asm/fsp2/fsp_internal.h>

/* The amount of the FSP header to probe to obtain what we need */
#define PROBE_BUF_SIZE 0x180

int fsp_get_header(ulong offset, ulong size, bool use_spi_flash,
		   struct fsp_header **fspp)
{
	static efi_guid_t guid = FSP_HEADER_GUID;
	struct fv_ext_header *exhdr;
	struct fsp_header *fsp;
	struct ffs_file_header *file_hdr;
	struct fv_header *fv;
	struct raw_section *raw;
	void *ptr, *base;
	u8 buf[PROBE_BUF_SIZE];
	struct udevice *dev;
	int ret;

	/*
	 * There are quite a very steps to work through all the headers in this
	 * file and the structs have similar names. Turn on debugging if needed
	 * to understand what is going wrong.
	 *
	 * You are in a maze of twisty little headers all alike.
	 */
	debug("offset=%x buf=%x\n", (uint)offset, (uint)buf);
	if (use_spi_flash) {
		ret = uclass_first_device_err(UCLASS_SPI_FLASH, &dev);
		if (ret)
			return log_msg_ret("Cannot find flash device", ret);
		ret = spi_flash_read_dm(dev, offset, PROBE_BUF_SIZE, buf);
		if (ret)
			return log_msg_ret("Cannot read flash", ret);
	} else {
		memcpy(buf, (void *)offset, PROBE_BUF_SIZE);
	}

	/* Initalise the FSP base */
	ptr = buf;
	fv = ptr;

	/* Check the FV signature, _FVH */
	debug("offset=%x sign=%x\n", (uint)offset, (uint)fv->sign);
	if (fv->sign != EFI_FVH_SIGNATURE)
		return log_msg_ret("Base FV signature", -EINVAL);

	/* Go to the end of the FV header and align the address */
	debug("fv->ext_hdr_off = %x\n", fv->ext_hdr_off);
	ptr += fv->ext_hdr_off;
	exhdr = ptr;
	ptr += ALIGN(exhdr->ext_hdr_size, 8);
	debug("ptr=%x\n", ptr - (void *)buf);

	/* Check the FFS GUID */
	file_hdr = ptr;
	if (memcmp(&file_hdr->name, &guid, sizeof(guid)))
		return log_msg_ret("Base FFS GUID", -ENXIO);
	/* Add the FFS header size to find the raw section header */
	ptr = file_hdr + 1;

	raw = ptr;
	debug("raw->type = %x\n", raw->type);
	if (raw->type != EFI_SECTION_RAW)
		return log_msg_ret("Section type not RAW", -ENOEXEC);

	/* Add the raw section header size to find the FSP header */
	ptr = raw + 1;
	fsp = ptr;

	/* Check the FSPH header */
	debug("fsp %x\n", (uint)fsp);
	if (fsp->sign != EFI_FSPH_SIGNATURE)
		return log_msg_ret("Base FSPH signature", -EACCES);

	base = (void *)fsp->img_base;
	debug("Image base %x\n", (uint)base);
	debug("Image addr %x\n", (uint)fsp->fsp_mem_init);
	if (use_spi_flash) {
		ret = spi_flash_read_dm(dev, offset, size, base);
		if (ret)
			return log_msg_ret("Could not read FPS-M", ret);
	} else {
		memcpy(base, (void *)offset, size);
	}
	ptr = base + (ptr - (void *)buf);
	*fspp = ptr;

	return 0;
}

u32 fsp_notify(struct fsp_header *fsp_hdr, u32 phase)
{
	fsp_notify_f notify;
	struct fsp_notify_params params;
	struct fsp_notify_params *params_ptr;
	u32 status;

	if (!fsp_hdr)
		fsp_hdr = gd->arch.fsp_s_hdr;

	if (!fsp_hdr)
		return log_msg_ret("no FSP", -ENOENT);

	notify = (fsp_notify_f)(fsp_hdr->img_base + fsp_hdr->fsp_notify);
	params.phase = phase;
	params_ptr = &params;

	/*
	 * Use ASM code to ensure correct parameter is on the stack for
	 * FspNotify as U-Boot is using different ABI from FSP
	 */
	asm volatile (
		"pushl	%1;"		/* push notify phase */
		"call	*%%eax;"	/* call FspNotify */
		"addl	$4, %%esp;"	/* clean up the stack */
		: "=a"(status) : "m"(params_ptr), "a"(notify), "m"(*params_ptr)
	);

	return status;
}

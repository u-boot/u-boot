/*
 *  EFI application disk support
 *
 *  Copyright (c) 2016 Alexander Graf
 *
 *  SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <efi_loader.h>
#include <inttypes.h>
#include <lcd.h>
#include <malloc.h>
#include <video.h>

DECLARE_GLOBAL_DATA_PTR;

static const efi_guid_t efi_gop_guid = EFI_GOP_GUID;

struct efi_gop_obj {
	/* Generic EFI object parent class data */
	struct efi_object parent;
	/* EFI Interface callback struct for gop */
	struct efi_gop ops;
	/* The only mode we support */
	struct efi_gop_mode_info info;
	struct efi_gop_mode mode;
	/* Fields we only have acces to during init */
	u32 bpix;
	void *fb;
};

static efi_status_t EFIAPI gop_query_mode(struct efi_gop *this, u32 mode_number,
					  efi_uintn_t *size_of_info,
					  struct efi_gop_mode_info **info)
{
	struct efi_gop_obj *gopobj;

	EFI_ENTRY("%p, %x, %p, %p", this, mode_number, size_of_info, info);

	gopobj = container_of(this, struct efi_gop_obj, ops);
	*size_of_info = sizeof(gopobj->info);
	*info = &gopobj->info;

	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI gop_set_mode(struct efi_gop *this, u32 mode_number)
{
	EFI_ENTRY("%p, %x", this, mode_number);

	if (mode_number != 0)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	return EFI_EXIT(EFI_SUCCESS);
}

efi_status_t EFIAPI gop_blt(struct efi_gop *this, void *buffer,
			    u32 operation, efi_uintn_t sx,
			    efi_uintn_t sy, efi_uintn_t dx,
			    efi_uintn_t dy, efi_uintn_t width,
			    efi_uintn_t height, efi_uintn_t delta)
{
	struct efi_gop_obj *gopobj = container_of(this, struct efi_gop_obj, ops);
	int i, j, line_len16, line_len32;
	void *fb;

	EFI_ENTRY("%p, %p, %u, %zu, %zu, %zu, %zu, %zu, %zu, %zu", this,
		  buffer, operation, sx, sy, dx, dy, width, height, delta);

	if (operation != EFI_BLT_BUFFER_TO_VIDEO)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	fb = gopobj->fb;
	line_len16 = gopobj->info.width * sizeof(u16);
	line_len32 = gopobj->info.width * sizeof(u32);

	/* Copy the contents line by line */

	switch (gopobj->bpix) {
#ifdef CONFIG_DM_VIDEO
	case VIDEO_BPP32:
#else
	case LCD_COLOR32:
#endif
		for (i = 0; i < height; i++) {
			u32 *dest = fb + ((i + dy)  * line_len32) +
					 (dx * sizeof(u32));
			u32 *src = buffer + ((i + sy)  * line_len32) +
					 (sx * sizeof(u32));

			/* Same color format, just memcpy */
			memcpy(dest, src, width * sizeof(u32));
		}
		break;
#ifdef CONFIG_DM_VIDEO
	case VIDEO_BPP16:
#else
	case LCD_COLOR16:
#endif
		for (i = 0; i < height; i++) {
			u16 *dest = fb + ((i + dy)  * line_len16) +
					 (dx * sizeof(u16));
			u32 *src = buffer + ((i + sy)  * line_len32) +
					 (sx * sizeof(u32));

			/* Convert from rgb888 to rgb565 */
			for (j = 0; j < width; j++) {
				u32 rgb888 = src[j];
				dest[j] = ((((rgb888 >> (16 + 3)) & 0x1f) << 11) |
					   (((rgb888 >> (8 + 2)) & 0x3f) << 5) |
					   (((rgb888 >> (0 + 3)) & 0x1f) << 0));
			}
		}
		break;
	}

#ifdef CONFIG_DM_VIDEO
	video_sync_all();
#else
	lcd_sync();
#endif

	return EFI_EXIT(EFI_SUCCESS);
}

/* This gets called from do_bootefi_exec(). */
int efi_gop_register(void)
{
	struct efi_gop_obj *gopobj;
	u32 bpix, col, row;
	u64 fb_base, fb_size;
	void *fb;
	efi_status_t ret;

#ifdef CONFIG_DM_VIDEO
	struct udevice *vdev;

	/* We only support a single video output device for now */
	if (uclass_first_device(UCLASS_VIDEO, &vdev) || !vdev)
		return -1;

	struct video_priv *priv = dev_get_uclass_priv(vdev);
	bpix = priv->bpix;
	col = video_get_xsize(vdev);
	row = video_get_ysize(vdev);
	fb_base = (uintptr_t)priv->fb;
	fb_size = priv->fb_size;
	fb = priv->fb;
#else
	int line_len;

	bpix = panel_info.vl_bpix;
	col = panel_info.vl_col;
	row = panel_info.vl_row;
	fb_base = gd->fb_base;
	fb_size = lcd_get_size(&line_len);
	fb = (void*)gd->fb_base;
#endif

	switch (bpix) {
#ifdef CONFIG_DM_VIDEO
	case VIDEO_BPP16:
	case VIDEO_BPP32:
#else
	case LCD_COLOR32:
	case LCD_COLOR16:
#endif
		break;
	default:
		/* So far, we only work in 16 or 32 bit mode */
		return -1;
	}

	gopobj = calloc(1, sizeof(*gopobj));
	if (!gopobj) {
		printf("ERROR: Out of memory\n");
		return 1;
	}

	/* Hook up to the device list */
	efi_add_handle(&gopobj->parent);

	/* Fill in object data */
	ret = efi_add_protocol(gopobj->parent.handle, &efi_gop_guid,
			       &gopobj->ops);
	if (ret != EFI_SUCCESS) {
		printf("ERROR: Out of memory\n");
		return 1;
	}
	gopobj->ops.query_mode = gop_query_mode;
	gopobj->ops.set_mode = gop_set_mode;
	gopobj->ops.blt = gop_blt;
	gopobj->ops.mode = &gopobj->mode;

	gopobj->mode.max_mode = 1;
	gopobj->mode.info = &gopobj->info;
	gopobj->mode.info_size = sizeof(gopobj->info);

#ifdef CONFIG_DM_VIDEO
	if (bpix == VIDEO_BPP32) {
#else
	if (bpix == LCD_COLOR32) {
#endif
		/* With 32bit color space we can directly expose the fb */
		gopobj->mode.fb_base = fb_base;
		gopobj->mode.fb_size = fb_size;
	}

	gopobj->info.version = 0;
	gopobj->info.width = col;
	gopobj->info.height = row;
	gopobj->info.pixel_format = EFI_GOT_RGBA8;
	gopobj->info.pixels_per_scanline = col;

	gopobj->bpix = bpix;
	gopobj->fb = fb;

	return 0;
}

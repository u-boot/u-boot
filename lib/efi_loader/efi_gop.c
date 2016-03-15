/*
 *  EFI application disk support
 *
 *  Copyright (c) 2016 Alexander Graf
 *
 *  SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <efi_loader.h>
#include <inttypes.h>
#include <lcd.h>
#include <malloc.h>

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
};

static efi_status_t EFIAPI gop_query_mode(struct efi_gop *this, u32 mode_number,
					  unsigned long *size_of_info,
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

static efi_status_t EFIAPI gop_blt(struct efi_gop *this, void *buffer,
				   unsigned long operation, unsigned long sx,
				   unsigned long sy, unsigned long dx,
				   unsigned long dy, unsigned long width,
				   unsigned long height, unsigned long delta)
{
	int i, j, line_len16, line_len32;
	void *fb;

	EFI_ENTRY("%p, %p, %lx, %lx, %lx, %lx, %lx, %lx, %lx, %lx", this,
		  buffer, operation, sx, sy, dx, dy, width, height, delta);

	if (operation != EFI_BLT_BUFFER_TO_VIDEO)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	fb = (void*)gd->fb_base;
	line_len16 = panel_info.vl_col * sizeof(u16);
	line_len32 = panel_info.vl_col * sizeof(u32);

	/* Copy the contents line by line */

	switch (panel_info.vl_bpix) {
	case LCD_COLOR32:
		for (i = 0; i < height; i++) {
			u32 *dest = fb + ((i + dy)  * line_len32) +
					 (dx * sizeof(u32));
			u32 *src = buffer + ((i + sy)  * line_len32) +
					 (sx * sizeof(u32));

			/* Same color format, just memcpy */
			memcpy(dest, src, width * sizeof(u32));
		}
		break;
	case LCD_COLOR16:
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

	lcd_sync();

	return EFI_EXIT(EFI_SUCCESS);
}

/* This gets called from do_bootefi_exec(). */
int efi_gop_register(void)
{
	struct efi_gop_obj *gopobj;
	int line_len;

	switch (panel_info.vl_bpix) {
	case LCD_COLOR32:
	case LCD_COLOR16:
		break;
	default:
		/* So far, we only work in 16 or 32 bit mode */
		return -1;
	}

	gopobj = calloc(1, sizeof(*gopobj));

	/* Fill in object data */
	gopobj->parent.protocols[0].guid = &efi_gop_guid;
	gopobj->parent.protocols[0].open = efi_return_handle;
	gopobj->parent.handle = &gopobj->ops;
	gopobj->ops.query_mode = gop_query_mode;
	gopobj->ops.set_mode = gop_set_mode;
	gopobj->ops.blt = gop_blt;
	gopobj->ops.mode = &gopobj->mode;

	gopobj->mode.max_mode = 1;
	gopobj->mode.info = &gopobj->info;
	gopobj->mode.info_size = sizeof(gopobj->info);
	gopobj->mode.fb_base = gd->fb_base;
	gopobj->mode.fb_size = lcd_get_size(&line_len);

	gopobj->info.version = 0;
	gopobj->info.width = panel_info.vl_col;
	gopobj->info.height = panel_info.vl_row;
	gopobj->info.pixel_format = EFI_GOT_RGBA8;
	gopobj->info.pixels_per_scanline = panel_info.vl_col;

	/* Hook up to the device list */
	list_add_tail(&gopobj->parent.link, &efi_obj_list);

	return 0;
}

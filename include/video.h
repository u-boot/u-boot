/*
 * Video uclass to support displays (see also vidconsole for text)
 *
 * Copyright (c) 2015 Google, Inc
 */

#ifndef _VIDEO_H_
#define _VIDEO_H_

#include <stdio_dev.h>

struct udevice;

/**
 * struct video_uc_plat - uclass platform data for a video device
 *
 * This holds information that the uclass needs to know about each device. It
 * is accessed using dev_get_uclass_plat(dev). See 'Theory of operation' at
 * the top of video-uclass.c for details on how this information is set.
 *
 * @align: Frame-buffer alignment, indicating the memory boundary the frame
 *	buffer should start on. If 0, 1MB is assumed
 * @size: Frame-buffer size, in bytes
 * @base: Base address of frame buffer, 0 if not yet known. If CONFIG_VIDEO_COPY
 *	is enabled, this is the software copy, so writes to this will not be
 *	visible until vidconsole_sync_copy() is called. If CONFIG_VIDEO_COPY is
 *	disabled, this is the hardware framebuffer.
 * @copy_base: Base address of a hardware copy of the frame buffer. If
 *	CONFIG_VIDEO_COPY is disabled, this is not used.
 * @copy_size: Size of copy framebuffer, used if @size is 0
 * @hide_logo: Hide the logo (used for testing)
 */
struct video_uc_plat {
	uint align;
	uint size;
	ulong base;
	ulong copy_base;
	ulong copy_size;
	bool hide_logo;
};

enum video_polarity {
	VIDEO_ACTIVE_HIGH,	/* Pins are active high */
	VIDEO_ACTIVE_LOW,	/* Pins are active low */
};

/*
 * Bits per pixel selector. Each value n is such that the bits-per-pixel is
 * 2 ^ n
 */
enum video_log2_bpp {
	VIDEO_BPP1	= 0,
	VIDEO_BPP2,
	VIDEO_BPP4,
	VIDEO_BPP8,
	VIDEO_BPP16,
	VIDEO_BPP32,
};

/* Convert enum video_log2_bpp to bytes and bits */
#define VNBYTES(bpix)	((1 << (bpix)) / 8)
#define VNBITS(bpix)	(1 << (bpix))

enum video_format {
	VIDEO_UNKNOWN,
	VIDEO_RGBA8888,
	VIDEO_X8B8G8R8,
	VIDEO_X8R8G8B8,
	VIDEO_X2R10G10B10,
};

/**
 * struct video_priv - Device information used by the video uclass
 *
 * @xsize:	Number of pixel columns (e.g. 1366)
 * @ysize:	Number of pixels rows (e.g.. 768)
 * @rot:	Display rotation (0=none, 1=90 degrees clockwise, etc.). THis
 *		does not affect @xsize and @ysize
 * @bpix:	Encoded bits per pixel (enum video_log2_bpp)
 * @format:	Pixel format (enum video_format)
 * @vidconsole_drv_name:	Driver to use for the text console, NULL to
 *		select automatically
 * @font_size:	Font size in pixels (0 to use a default value)
 * @fb:		Frame buffer
 * @fb_size:	Frame buffer size
 * @copy_fb:	Copy of the frame buffer to keep up to date; see struct
 *		video_uc_plat
 * @damage:	A bounding box of framebuffer regions updated since last sync
 * @damage.xstart:	X start position in pixels from the left
 * @damage.ystart:	Y start position in pixels from the top
 * @damage.xend:	X end position in pixels from the left
 * @damage.xend:	Y end position in pixels from the top
 * @line_length:	Length of each frame buffer line, in bytes. This can be
 *		set by the driver, but if not, the uclass will set it after
 *		probing
 * @colour_fg:	Foreground colour (pixel value)
 * @colour_bg:	Background colour (pixel value)
 * @flush_dcache:	true to enable flushing of the data cache after
 *		the LCD is updated
 * @fg_col_idx:	Foreground color code (bit 3 = bold, bit 0-2 = color)
 * @bg_col_idx:	Background color code (bit 3 = bold, bit 0-2 = color)
 * @last_sync:	Monotonic time of last video sync
 * @white_on_black: Use a black background
 */
struct video_priv {
	/* Things set up by the driver: */
	ushort xsize;
	ushort ysize;
	ushort rot;
	enum video_log2_bpp bpix;
	enum video_format format;
	const char *vidconsole_drv_name;
	int font_size;

	/*
	 * Things that are private to the uclass: don't use these in the
	 * driver
	 */
	void *fb;
	int fb_size;
	void *copy_fb;
	struct {
		int xstart;
		int ystart;
		int xend;
		int yend;
	} damage;
	int line_length;
	u32 colour_fg;
	u32 colour_bg;
	bool flush_dcache;
	u8 fg_col_idx;
	u8 bg_col_idx;
	ulong last_sync;
	bool white_on_black;
};

/**
 * struct video_ops - structure for keeping video operations
 * @video_sync: Synchronize FB with device. Some device like SPI based LCD
 *		displays needs synchronization when data in an FB is available.
 *		For these devices implement video_sync hook to call a sync
 *		function. vid is pointer to video device udevice. Function
 *		should return 0 on success video_sync and error code otherwise
 */
struct video_ops {
	int (*video_sync)(struct udevice *vid);
};

#define video_get_ops(dev)        ((struct video_ops *)(dev)->driver->ops)

/**
 * struct video_handoff - video information passed from SPL
 *
 * This is used when video is set up by SPL, to provide the details to U-Boot
 * proper.
 *
 * @fb: Base address of frame buffer, 0 if not yet known
 * @size: Frame-buffer size, in bytes
 * @xsize:	Number of pixel columns (e.g. 1366)
 * @ysize:	Number of pixels rows (e.g.. 768)
 * @line_length:	Length of each frame buffer line, in bytes. This can be
 *		set by the driver, but if not, the uclass will set it after
 *		probing
 * @bpix:	Encoded bits per pixel (enum video_log2_bpp)
 * @format:	Video format (enum video_format)
 */
struct video_handoff {
	u64 fb;
	u32 size;
	u16 xsize;
	u16 ysize;
	u32 line_length;
	u8 bpix;
	u8 format;
};

/** enum colour_idx - the 16 colors supported by consoles */
enum colour_idx {
	VID_BLACK = 0,
	VID_RED,
	VID_GREEN,
	VID_BROWN,
	VID_BLUE,
	VID_MAGENTA,
	VID_CYAN,
	VID_LIGHT_GRAY,
	VID_GRAY,
	VID_LIGHT_RED,
	VID_LIGHT_GREEN,
	VID_YELLOW,
	VID_LIGHT_BLUE,
	VID_LIGHT_MAGENTA,
	VID_LIGHT_CYAN,
	VID_WHITE,
	VID_DARK_GREY,

	VID_COLOUR_COUNT
};

/**
 * video_index_to_colour() - convert a color code to a pixel's internal
 * representation
 *
 * The caller has to guarantee that the color index is less than
 * VID_COLOR_COUNT.
 *
 * @priv	private data of the video device (UCLASS_VIDEO)
 * @idx		color index (e.g. VID_YELLOW)
 * Return:	color value
 */
u32 video_index_to_colour(struct video_priv *priv, enum colour_idx idx);

/**
 * video_reserve() - Reserve frame-buffer memory for video devices
 *
 * Note: This function is for internal use.
 *
 * This uses the uclass plat's @size and @align members to figure out
 * a size and position for each frame buffer as part of the pre-relocation
 * process of determining the post-relocation memory layout.
 *
 * gd->video_top is set to the initial value of *@addrp and gd->video_bottom
 * is set to the final value.
 *
 * @addrp:	On entry, the top of available memory. On exit, the new top,
 *		after allocating the required memory.
 * Return: 0
 */
int video_reserve(ulong *addrp);

/**
 * video_clear() - Clear a device's frame buffer to background colour.
 *
 * @dev:	Device to clear
 * Return: 0 on success
 */
int video_clear(struct udevice *dev);

/**
 * video_fill() - Fill a device's frame buffer to a colour.
 *
 * @dev:	Device to fill
 * @colour:	Colour to use, in the frame buffer's format
 * Return: 0 on success
 */
int video_fill(struct udevice *dev, u32 colour);

/**
 * video_fill_part() - Erase a region
 *
 * Erase a rectangle on the display within the given bounds
 *
 * @dev:	Device to update
 * @xstart:	X start position in pixels from the left
 * @ystart:	Y start position in pixels from the top
 * @xend:	X end position in pixels from the left
 * @yend:	Y end position  in pixels from the top
 * @colour:	Value to write
 * Return: 0 if OK, -ENOSYS if the display depth is not supported
 */
int video_fill_part(struct udevice *dev, int xstart, int ystart, int xend,
		    int yend, u32 colour);

/**
 * video_draw_box() - Draw a box
 *
 * Draw a rectangle on the display within the given bounds
 *
 * @dev:	Device to update
 * @x0:		X start position in pixels from the left
 * @y0:		Y start position in pixels from the top
 * @x1:		X end position in pixels from the left
 * @y1:		Y end position in pixels from the top
 * @width:	width in pixels
 * @colour:	Value to write
 * Return: 0 if OK, -ENOSYS if the display depth is not supported
 */
int video_draw_box(struct udevice *dev, int x0, int y0, int x1, int y1,
		   int width, u32 colour);

/**
 * video_sync() - Sync a device's frame buffer with its hardware
 *
 * @vid:	Device to sync
 * @force:	True to force a sync even if there was one recently (this is
 *		very expensive on sandbox)
 *
 * @return: 0 on success, error code otherwise
 *
 * Some frame buffers are cached or have a secondary frame buffer. This
 * function syncs the damaged parts of them up so that the current contents
 * of the U-Boot frame buffer are displayed to the user. It clears the damage
 * buffer.
 */
int video_sync(struct udevice *vid, bool force);

/**
 * video_sync_all() - Sync all devices' frame buffers with their hardware
 *
 * This calls video_sync() on all active video devices.
 */
void video_sync_all(void);

/**
 * video_bmp_get_info() - Get information about a bitmap image
 *
 * @bmp_image: Pointer to BMP image to check
 * @widthp: Returns width in pixels
 * @heightp: Returns height in pixels
 * @bpixp: Returns log2 of bits per pixel
 */
void video_bmp_get_info(void *bmp_image, ulong *widthp, ulong *heightp,
			uint *bpixp);

/**
 * video_bmp_display() - Display a BMP file
 *
 * @dev:	Device to display the bitmap on
 * @bmp_image:	Address of bitmap image to display
 * @x:		X position in pixels from the left
 * @y:		Y position in pixels from the top
 * @align:	true to adjust the coordinates to centre the image. If false
 *		the coordinates are used as is. If true:
 *
 *		- if a coordinate is 0x7fff then the image will be centred in
 *		  that direction
 *		- if a coordinate is -ve then it will be offset to the
 *		  left/top of the centre by that many pixels
 *		- if a coordinate is positive it will be used unchanged.
 * Return: 0 if OK, -ve on error
 */
int video_bmp_display(struct udevice *dev, ulong bmp_image, int x, int y,
		      bool align);

/**
 * video_get_xsize() - Get the width of the display in pixels
 *
 * @dev:	Device to check
 * Return: device frame buffer width in pixels
 */
int video_get_xsize(struct udevice *dev);

/**
 * video_get_ysize() - Get the height of the display in pixels
 *
 * @dev:	Device to check
 * Return: device frame buffer height in pixels
 */
int video_get_ysize(struct udevice *dev);

/**
 * Set whether we need to flush the dcache when changing the image. This
 * defaults to off.
 *
 * @param flush		non-zero to flush cache after update, 0 to skip
 */
void video_set_flush_dcache(struct udevice *dev, bool flush);

/**
 * Set default colors and attributes
 *
 * @dev:	video device
 * @invert	true to invert colours
 */
void video_set_default_colors(struct udevice *dev, bool invert);

/**
 * video_set_white_on_black() - Change the setting for white-on-black
 *
 * This does nothing if the setting is already the same.
 *
 * @dev: video device
 * @white_on_black: true to use white-on-black, false for black-on-white
 */
void video_set_white_on_black(struct udevice *dev, bool white_on_black);

/**
 * video_default_font_height() - Get the default font height
 *
 * @dev:	video device
 * Returns: Default font height in pixels, which depends on which console driver
 * is in use
 */
int video_default_font_height(struct udevice *dev);

#ifdef CONFIG_VIDEO_DAMAGE
/**
 * video_damage() - Notify the video subsystem about screen updates.
 *
 * @vid:	Device to sync
 * @x:	        Upper left X coordinate of the damaged rectangle
 * @y:	        Upper left Y coordinate of the damaged rectangle
 * @width:	Width of the damaged rectangle
 * @height:	Height of the damaged rectangle
 *
 * Some frame buffers are cached or have a secondary frame buffer. This
 * function notifies the video subsystem about rectangles that were updated
 * within the frame buffer. They may only get written to the screen on the
 * next call to video_sync().
 */
void video_damage(struct udevice *vid, int x, int y, int width, int height);
#else
static inline void video_damage(struct udevice *vid, int x, int y, int width,
				int height)
{
	return;
}
#endif /* CONFIG_VIDEO_DAMAGE */

/**
 * video_is_active() - Test if one video device it active
 *
 * Return: true if at least one video device is active, else false.
 */
bool video_is_active(void);

/**
 * video_get_u_boot_logo() - Get a pointer to the U-Boot logo
 *
 * Returns: Pointer to logo
 */
void *video_get_u_boot_logo(void);

/*
 * bmp_display() - Display BMP (bitmap) data located in memory
 *
 * @addr: address of the bmp data
 * @x: Position of bitmap from the left side, in pixels
 * @y: Position of bitmap from the top, in pixels
 */
int bmp_display(ulong addr, int x, int y);

/*
 * bmp_info() - Show information about bmp file
 *
 * @addr: address of bmp file
 * Returns: 0 if OK, else 1 if bmp image not found
 */
int bmp_info(ulong addr);

/*
 * video_reserve_from_bloblist()- Reserve frame-buffer memory for video devices
 * using blobs.
 *
 * @ho: video information passed from SPL
 * Returns: 0 (always)
 */
int video_reserve_from_bloblist(struct video_handoff *ho);

/**
 * video_get_fb() - Get the first framebuffer address
 *
 * This function does not probe video devices, so can only be used after a video
 * device has been activated.
 *
 * Return: address of the framebuffer of the first video device found, or 0 if
 * there is no device
 */
ulong video_get_fb(void);

#endif

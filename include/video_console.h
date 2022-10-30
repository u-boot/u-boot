/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2015 Google, Inc
 */

#ifndef __video_console_h
#define __video_console_h

#include <video.h>

struct video_priv;

#define VID_FRAC_DIV	256

#define VID_TO_PIXEL(x)	((x) / VID_FRAC_DIV)
#define VID_TO_POS(x)	((x) * VID_FRAC_DIV)

/**
 * struct vidconsole_priv - uclass-private data about a console device
 *
 * Drivers must set up @rows, @cols, @x_charsize, @y_charsize in their probe()
 * method. Drivers may set up @xstart_frac if desired.
 *
 * @sdev:		stdio device, acting as an output sink
 * @xcur_frac:		Current X position, in fractional units (VID_TO_POS(x))
 * @ycur:		Current Y position in pixels (0=top)
 * @rows:		Number of text rows
 * @cols:		Number of text columns
 * @x_charsize:		Character width in pixels
 * @y_charsize:		Character height in pixels
 * @tab_width_frac:	Tab width in fractional units
 * @xsize_frac:		Width of the display in fractional units
 * @xstart_frac:	Left margin for the text console in fractional units
 * @last_ch:		Last character written to the text console on this line
 * @escape:		TRUE if currently accumulating an ANSI escape sequence
 * @escape_len:		Length of accumulated escape sequence so far
 * @col_saved:		Saved X position, in fractional units (VID_TO_POS(x))
 * @row_saved:		Saved Y position in pixels (0=top)
 * @escape_buf:		Buffer to accumulate escape sequence
 */
struct vidconsole_priv {
	struct stdio_dev sdev;
	int xcur_frac;
	int ycur;
	int rows;
	int cols;
	int x_charsize;
	int y_charsize;
	int tab_width_frac;
	int xsize_frac;
	int xstart_frac;
	int last_ch;
	/*
	 * ANSI escape sequences are accumulated character by character,
	 * starting after the ESC char (0x1b) until the entire sequence
	 * is consumed at which point it is acted upon.
	 */
	int escape;
	int escape_len;
	int row_saved;
	int col_saved;
	char escape_buf[32];
};

/**
 * struct vidconsole_ops - Video console operations
 *
 * These operations work on either an absolute console position (measured
 * in pixels) or a text row number (measured in rows, where each row consists
 * of an entire line of text - typically 16 pixels).
 */
struct vidconsole_ops {
	/**
	 * putc_xy() - write a single character to a position
	 *
	 * @dev:	Device to write to
	 * @x_frac:	Fractional pixel X position (0=left-most pixel) which
	 *		is the X position multipled by VID_FRAC_DIV.
	 * @y:		Pixel Y position (0=top-most pixel)
	 * @ch:		Character to write
	 * @return number of fractional pixels that the cursor should move,
	 * if all is OK, -EAGAIN if we ran out of space on this line, other -ve
	 * on error
	 */
	int (*putc_xy)(struct udevice *dev, uint x_frac, uint y, char ch);

	/**
	 * move_rows() - Move text rows from one place to another
	 *
	 * @dev:	Device to adjust
	 * @rowdst:	Destination text row (0=top)
	 * @rowsrc:	Source start text row
	 * @count:	Number of text rows to move
	 * @return 0 if OK, -ve on error
	 */
	int (*move_rows)(struct udevice *dev, uint rowdst, uint rowsrc,
			  uint count);

	/**
	 * set_row() - Set the colour of a text row
	 *
	 * Every pixel contained within the text row is adjusted
	 *
	 * @dev:	Device to adjust
	 * @row:	Text row to adjust (0=top)
	 * @clr:	Raw colour (pixel value) to write to each pixel
	 * @return 0 if OK, -ve on error
	 */
	int (*set_row)(struct udevice *dev, uint row, int clr);

	/**
	 * entry_start() - Indicate that text entry is starting afresh
	 *
	 * Consoles which use proportional fonts need to track the position of
	 * each character output so that backspace will return to the correct
	 * place. This method signals to the console driver that a new entry
	 * line is being start (e.g. the user pressed return to start a new
	 * command). The driver can use this signal to empty its list of
	 * positions.
	 */
	int (*entry_start)(struct udevice *dev);

	/**
	 * backspace() - Handle erasing the last character
	 *
	 * With proportional fonts the vidconsole uclass cannot itself erase
	 * the previous character. This optional method will be called when
	 * a backspace is needed. The driver should erase the previous
	 * character and update the cursor position (xcur_frac, ycur) to the
	 * start of the previous character.
	 *
	 * If not implement, default behaviour will work for fixed-width
	 * characters.
	 */
	int (*backspace)(struct udevice *dev);
};

/* Get a pointer to the driver operations for a video console device */
#define vidconsole_get_ops(dev)  ((struct vidconsole_ops *)(dev)->driver->ops)

/**
 * vidconsole_putc_xy() - write a single character to a position
 *
 * @dev:	Device to write to
 * @x_frac:	Fractional pixel X position (0=left-most pixel) which
 *		is the X position multipled by VID_FRAC_DIV.
 * @y:		Pixel Y position (0=top-most pixel)
 * @ch:		Character to write
 * Return: number of fractional pixels that the cursor should move,
 * if all is OK, -EAGAIN if we ran out of space on this line, other -ve
 * on error
 */
int vidconsole_putc_xy(struct udevice *dev, uint x, uint y, char ch);

/**
 * vidconsole_move_rows() - Move text rows from one place to another
 *
 * @dev:	Device to adjust
 * @rowdst:	Destination text row (0=top)
 * @rowsrc:	Source start text row
 * @count:	Number of text rows to move
 * Return: 0 if OK, -ve on error
 */
int vidconsole_move_rows(struct udevice *dev, uint rowdst, uint rowsrc,
			 uint count);

/**
 * vidconsole_set_row() - Set the colour of a text row
 *
 * Every pixel contained within the text row is adjusted
 *
 * @dev:	Device to adjust
 * @row:	Text row to adjust (0=top)
 * @clr:	Raw colour (pixel value) to write to each pixel
 * Return: 0 if OK, -ve on error
 */
int vidconsole_set_row(struct udevice *dev, uint row, int clr);

/**
 * vidconsole_put_char() - Output a character to the current console position
 *
 * Outputs a character to the console and advances the cursor. This function
 * handles wrapping to new lines and scrolling the console. Special
 * characters are handled also: \n, \r, \b and \t.
 *
 * The device always starts with the cursor at position 0,0 (top left). It
 * can be adjusted manually using vidconsole_position_cursor().
 *
 * @dev:	Device to adjust
 * @ch:		Character to write
 * Return: 0 if OK, -ve on error
 */
int vidconsole_put_char(struct udevice *dev, char ch);

/**
 * vidconsole_put_string() - Output a string to the current console position
 *
 * Outputs a string to the console and advances the cursor. This function
 * handles wrapping to new lines and scrolling the console. Special
 * characters are handled also: \n, \r, \b and \t.
 *
 * The device always starts with the cursor at position 0,0 (top left). It
 * can be adjusted manually using vidconsole_position_cursor().
 *
 * @dev:	Device to adjust
 * @str:	String to write
 * Return: 0 if OK, -ve on error
 */
int vidconsole_put_string(struct udevice *dev, const char *str);

/**
 * vidconsole_position_cursor() - Move the text cursor
 *
 * @dev:	Device to adjust
 * @col:	New cursor text column
 * @row:	New cursor text row
 * Return: 0 if OK, -ve on error
 */
void vidconsole_position_cursor(struct udevice *dev, unsigned col,
				unsigned row);

/**
 * vidconsole_set_cursor_pos() - set cursor position
 *
 * The cursor is set to the new position and the start-of-line information is
 * updated to the same position, so that a newline will return to @x
 *
 * @dev:	video console device to update
 * @x:		x position from left in pixels
 * @y:		y position from top in pixels
 */
void vidconsole_set_cursor_pos(struct udevice *dev, int x, int y);

/**
 * vidconsole_list_fonts() - List the available fonts
 *
 * This shows a list on the console
 */
void vidconsole_list_fonts(void);

/**
 * vidconsole_select_font() - Select a font to use
 *
 * @dev: vidconsole device
 * @name: Font name
 * @size: Size of the font (norminal pixel height) or 0 for default
 */
int vidconsole_select_font(struct udevice *dev, const char *name, uint size);

/**
 * vidconsole_get_font() - get the current font name and size
 *
 * @dev: vidconsole device
 * @sizep: Place to put the font size (nominal height in pixels)
 * Returns: Current font name
 */
const char *vidconsole_get_font(struct udevice *dev, uint *sizep);

#ifdef CONFIG_VIDEO_COPY
/**
 * vidconsole_sync_copy() - Sync back to the copy framebuffer
 *
 * This ensures that the copy framebuffer has the same data as the framebuffer
 * for a particular region. It should be called after the framebuffer is updated
 *
 * @from and @to can be in either order. The region between them is synced.
 *
 * @dev: Vidconsole device being updated
 * @from: Start/end address within the framebuffer (->fb)
 * @to: Other address within the frame buffer
 * Return: 0 if OK, -EFAULT if the start address is before the start of the
 *	frame buffer start
 */
int vidconsole_sync_copy(struct udevice *dev, void *from, void *to);

/**
 * vidconsole_memmove() - Perform a memmove() within the frame buffer
 *
 * This handles a memmove(), e.g. for scrolling. It also updates the copy
 * framebuffer.
 *
 * @dev: Vidconsole device being updated
 * @dst: Destination address within the framebuffer (->fb)
 * @src: Source address within the framebuffer (->fb)
 * @size: Number of bytes to transfer
 * Return: 0 if OK, -EFAULT if the start address is before the start of the
 *	frame buffer start
 */
int vidconsole_memmove(struct udevice *dev, void *dst, const void *src,
		       int size);
#else
static inline int vidconsole_sync_copy(struct udevice *dev, void *from,
				       void *to)
{
	return 0;
}

static inline int vidconsole_memmove(struct udevice *dev, void *dst,
				     const void *src, int size)
{
	memmove(dst, src, size);

	return 0;
}

#endif

#endif

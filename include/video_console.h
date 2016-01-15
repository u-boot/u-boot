/*
 * Copyright (c) 2015 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __video_console_h
#define __video_console_h

#define VID_FRAC_DIV	256

#define VID_TO_PIXEL(x)	((x) / VID_FRAC_DIV)
#define VID_TO_POS(x)	((x) * VID_FRAC_DIV)

/**
 * struct vidconsole_priv - uclass-private data about a console device
 *
 * Drivers must set up @rows, @cols, @x_charsize, @y_charsize in their probe()
 * method. Drivers may set up @xstart_frac if desired.
 *
 * @sdev:	stdio device, acting as an output sink
 * @xcur_frac:	Current X position, in fractional units (VID_TO_POS(x))
 * @curr_row:	Current Y position in pixels (0=top)
 * @rows:	Number of text rows
 * @cols:	Number of text columns
 * @x_charsize:	Character width in pixels
 * @y_charsize:	Character height in pixels
 * @tab_width_frac:	Tab width in fractional units
 * @xsize_frac:	Width of the display in fractional units
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
 * @return number of fractional pixels that the cursor should move,
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
 * @return 0 if OK, -ve on error
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
 * @return 0 if OK, -ve on error
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
 * @return 0 if OK, -ve on error
 */
int vidconsole_put_char(struct udevice *dev, char ch);

/**
 * vidconsole_position_cursor() - Move the text cursor
 *
 * @dev:	Device to adjust
 * @col:	New cursor text column
 * @row:	New cursor text row
 * @return 0 if OK, -ve on error
 */
void vidconsole_position_cursor(struct udevice *dev, unsigned col,
				unsigned row);

#endif

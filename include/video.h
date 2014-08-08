/*
** MPC823 Video Controller
** =======================
** (C) 2000 by Paolo Scaffardi (arsenio@tin.it)
** AIRVENT SAM s.p.a - RIMINI(ITALY)
**
*/

#ifndef _VIDEO_H_
#define _VIDEO_H_

/* Video functions */

struct stdio_dev;

int	video_init(void *videobase);
void	video_putc(struct stdio_dev *dev, const char c);
void	video_puts(struct stdio_dev *dev, const char *s);

/**
 * Display a BMP format bitmap on the screen
 *
 * @param bmp_image	Address of BMP image
 * @param x		X position to draw image
 * @param y		Y position to draw image
 */
int video_display_bitmap(ulong bmp_image, int x, int y);

/**
 * Get the width of the screen in pixels
 *
 * @return width of screen in pixels
 */
int video_get_pixel_width(void);

/**
 * Get the height of the screen in pixels
 *
 * @return height of screen in pixels
 */
int video_get_pixel_height(void);

/**
 * Get the number of text lines/rows on the screen
 *
 * @return number of rows
 */
int video_get_screen_rows(void);

/**
 * Get the number of text columns on the screen
 *
 * @return number of columns
 */
int video_get_screen_columns(void);

/**
 * Set the position of the text cursor
 *
 * @param col	Column to place cursor (0 = left side)
 * @param row	Row to place cursor (0 = top line)
 */
void video_position_cursor(unsigned col, unsigned row);

/* Clear the display */
void video_clear(void);

#if defined(CONFIG_FORMIKE)
int kwh043st20_f01_spi_startup(unsigned int bus, unsigned int cs,
	unsigned int max_hz, unsigned int spi_mode);
#endif
#endif

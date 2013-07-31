/* (C) Copyright 2002
 * Detlev Zundel, DENX Software Engineering, dzu@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/************************************************************************/
/* ** Layout of a bmp file						*/
/************************************************************************/

#ifndef _BMP_H_
#define _BMP_H_

typedef struct bmp_color_table_entry {
	__u8	blue;
	__u8	green;
	__u8	red;
	__u8	reserved;
} __attribute__ ((packed)) bmp_color_table_entry_t;

/* When accessing these fields, remember that they are stored in little
   endian format, so use linux macros, e.g. le32_to_cpu(width)          */

typedef struct bmp_header {
	/* Header */
	char signature[2];
	__u32	file_size;
	__u32	reserved;
	__u32	data_offset;
	/* InfoHeader */
	__u32	size;
	__u32	width;
	__u32	height;
	__u16	planes;
	__u16	bit_count;
	__u32	compression;
	__u32	image_size;
	__u32	x_pixels_per_m;
	__u32	y_pixels_per_m;
	__u32	colors_used;
	__u32	colors_important;
	/* ColorTable */

} __attribute__ ((packed)) bmp_header_t;

typedef struct bmp_image {
	bmp_header_t header;
	/* We use a zero sized array just as a placeholder for variable
	   sized array */
	bmp_color_table_entry_t color_table[0];
} bmp_image_t;

/* Data in the bmp_image is aligned to this length */
#define BMP_DATA_ALIGN	4

/* Constants for the compression field */
#define BMP_BI_RGB	0
#define BMP_BI_RLE8	1
#define BMP_BI_RLE4	2

#endif							/* _BMP_H_ */

/*
 * (C) Copyright 2002
 * Detlev Zundel, DENX Software Engineering, dzu@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * BMP handling routines
 */

#include <common.h>
#include <bmp_layout.h>
#include <command.h>
#include <asm/byteorder.h>
#include <malloc.h>

static int bmp_info (ulong addr);
static int bmp_display (ulong addr, int x, int y);

int gunzip(void *, int, unsigned char *, unsigned long *);

/*
 * Allocate and decompress a BMP image using gunzip().
 *
 * Returns a pointer to the decompressed image data. Must be freed by
 * the caller after use.
 *
 * Returns NULL if decompression failed, or if the decompressed data
 * didn't contain a valid BMP signature.
 */
#ifdef CONFIG_VIDEO_BMP_GZIP
static bmp_image_t *gunzip_bmp(unsigned long addr, unsigned long *lenp)
{
	void *dst;
	unsigned long len;
	bmp_image_t *bmp;

	/*
	 * Decompress bmp image
	 */
	len = CFG_VIDEO_LOGO_MAX_SIZE;
	dst = malloc(CFG_VIDEO_LOGO_MAX_SIZE);
	if (dst == NULL) {
		puts("Error: malloc in gunzip failed!\n");
		return NULL;
	}
	if (gunzip(dst, CFG_VIDEO_LOGO_MAX_SIZE, (uchar *)addr, &len) != 0) {
		free(dst);
		return NULL;
	}
	if (len == CFG_VIDEO_LOGO_MAX_SIZE)
		puts("Image could be truncated"
				" (increase CFG_VIDEO_LOGO_MAX_SIZE)!\n");

	bmp = dst;

	/*
	 * Check for bmp mark 'BM'
	 */
	if (!((bmp->header.signature[0] == 'B') &&
	      (bmp->header.signature[1] == 'M'))) {
		free(dst);
		return NULL;
	}

	puts("Gzipped BMP image detected!\n");

	return bmp;
}
#else
static bmp_image_t *gunzip_bmp(unsigned long addr, unsigned long *lenp)
{
	return NULL;
}
#endif


/*
 * Subroutine:  do_bmp
 *
 * Description: Handler for 'bmp' command..
 *
 * Inputs:	argv[1] contains the subcommand
 *
 * Return:      None
 *
 */
int do_bmp(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong addr;
	int x = 0, y = 0;

	switch (argc) {
	case 2:		/* use load_addr as default address */
		addr = load_addr;
		break;
	case 3:		/* use argument */
		addr = simple_strtoul(argv[2], NULL, 16);
		break;
	case 5:
		addr = simple_strtoul(argv[2], NULL, 16);
	        x = simple_strtoul(argv[3], NULL, 10);
	        y = simple_strtoul(argv[4], NULL, 10);
	        break;
	default:
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	/* Allow for short names
	 * Adjust length if more sub-commands get added
	 */
	if (strncmp(argv[1],"info",1) == 0) {
		return (bmp_info(addr));
	} else if (strncmp(argv[1],"display",1) == 0) {
	    return (bmp_display(addr, x, y));
	} else {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}
}

U_BOOT_CMD(
	bmp,	5,	1,	do_bmp,
	"bmp     - manipulate BMP image data\n",
	"info <imageAddr>          - display image info\n"
	"bmp display <imageAddr> [x y] - display image at x,y\n"
);

/*
 * Subroutine:  bmp_info
 *
 * Description: Show information about bmp file in memory
 *
 * Inputs:	addr		address of the bmp file
 *
 * Return:      None
 *
 */
static int bmp_info(ulong addr)
{
	bmp_image_t *bmp=(bmp_image_t *)addr;
	unsigned long len;

	if (!((bmp->header.signature[0]=='B') &&
	      (bmp->header.signature[1]=='M')))
		bmp = gunzip_bmp(addr, &len);

	if (bmp == NULL) {
		printf("There is no valid bmp file at the given address\n");
		return 1;
	}

	printf("Image size    : %d x %d\n", le32_to_cpu(bmp->header.width),
	       le32_to_cpu(bmp->header.height));
	printf("Bits per pixel: %d\n", le16_to_cpu(bmp->header.bit_count));
	printf("Compression   : %d\n", le32_to_cpu(bmp->header.compression));

	if ((unsigned long)bmp != addr)
		free(bmp);

	return(0);
}

/*
 * Subroutine:  bmp_display
 *
 * Description: Display bmp file located in memory
 *
 * Inputs:	addr		address of the bmp file
 *
 * Return:      None
 *
 */
static int bmp_display(ulong addr, int x, int y)
{
	int ret;
	bmp_image_t *bmp = (bmp_image_t *)addr;
	unsigned long len;

	if (!((bmp->header.signature[0]=='B') &&
	      (bmp->header.signature[1]=='M')))
		bmp = gunzip_bmp(addr, &len);

	if (!bmp) {
		printf("There is no valid bmp file at the given address\n");
		return 1;
	}

#if defined(CONFIG_LCD)
	extern int lcd_display_bitmap (ulong, int, int);

	ret = lcd_display_bitmap ((unsigned long)bmp, x, y);
#elif defined(CONFIG_VIDEO)
	extern int video_display_bitmap (ulong, int, int);

	ret = video_display_bitmap ((unsigned long)bmp, x, y);
#else
# error bmp_display() requires CONFIG_LCD or CONFIG_VIDEO
#endif

	if ((unsigned long)bmp != addr)
		free(bmp);

	return ret;
}

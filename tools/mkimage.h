/*
 * (C) Copyright 2000-2004
 * DENX Software Engineering
 * Wolfgang Denk, wd@denx.de
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

#ifndef _MKIIMAGE_H_
#define _MKIIMAGE_H_

#include "os_support.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <sha1.h>
#include "fdt_host.h"

#undef MKIMAGE_DEBUG

#ifdef MKIMAGE_DEBUG
#define debug(fmt,args...)	printf (fmt ,##args)
#else
#define debug(fmt,args...)
#endif /* MKIMAGE_DEBUG */

#define MKIMAGE_TMPFILE_SUFFIX		".tmp"
#define MKIMAGE_MAX_TMPFILE_LEN		256
#define MKIMAGE_DEFAULT_DTC_OPTIONS	"-I dts -O dtb -p 500"
#define MKIMAGE_MAX_DTC_CMDLINE_LEN	512
#define MKIMAGE_DTC			"dtc"   /* assume dtc is in $PATH */

/*
 * This structure defines all such variables those are initialized by
 * mkimage main core and need to be referred by image type specific
 * functions
 */
struct mkimage_params {
	int dflag;
	int eflag;
	int fflag;
	int lflag;
	int vflag;
	int xflag;
	int os;
	int arch;
	int type;
	int comp;
	char *dtc;
	unsigned int addr;
	unsigned int ep;
	char *imagename;
	char *datafile;
	char *imagefile;
	char *cmdname;
};

/*
 * image type specific variables and callback functions
 */
struct image_type_params {
	/* name is an identification tag string for added support */
	char *name;
	/*
	 * header size is local to the specific image type to be supported,
	 * mkimage core treats this as number of bytes
	 */
	uint32_t header_size;
	/* Image type header pointer */
	void *hdr;
	/*
	 * There are several arguments that are passed on the command line
	 * and are registered as flags in mkimage_params structure.
	 * This callback function can be used to check the passed arguments
	 * are in-lined with the image type to be supported
	 *
	 * Returns 1 if parameter check is successful
	 */
	int (*check_params) (struct mkimage_params *);
	/*
	 * This function is used by list command (i.e. mkimage -l <filename>)
	 * image type verification code must be put here
	 *
	 * Returns 0 if image header verification is successful
	 * otherwise, returns respective negative error codes
	 */
	int (*verify_header) (unsigned char *, int, struct mkimage_params *);
	/* Prints image information abstracting from image header */
	void (*print_header) (const void *);
	/*
	 * The header or image contents need to be set as per image type to
	 * be generated using this callback function.
	 * further output file post processing (for ex. checksum calculation,
	 * padding bytes etc..) can also be done in this callback function.
	 */
	void (*set_header) (void *, struct stat *, int,
					struct mkimage_params *);
	/*
	 * Some image generation support for ex (default image type) supports
	 * more than one type_ids, this callback function is used to check
	 * whether input (-T <image_type>) is supported by registered image
	 * generation/list low level code
	 */
	int (*check_image_type) (uint8_t);
	/* This callback function will be executed if fflag is defined */
	int (*fflag_handle) (struct mkimage_params *);
	/* pointer to the next registered entry in linked list */
	struct image_type_params *next;
};

/*
 * Exported functions
 */
void mkimage_register (struct image_type_params *tparams);

/*
 * There is a c file associated with supported image type low level code
 * for ex. default_image.c, fit_image.c
 * init is the only function referred by mkimage core.
 * to avoid a single lined header file, you can define them here
 *
 * Supported image types init functions
 */
void init_kwb_image_type (void);
void init_imx_image_type (void);
void init_default_image_type (void);
void init_fit_image_type (void);

#endif /* _MKIIMAGE_H_ */

/*
 * (C) Copyright 2013
 *
 * Written by Guilherme Maciel Ferreira <guilherme.maciel.ferreira@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _IMAGETOOL_H_
#define _IMAGETOOL_H_

#include "os_support.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <u-boot/sha1.h>
#include "fdt_host.h"

#define ARRAY_SIZE(x)		(sizeof(x) / sizeof((x)[0]))

#define IH_ARCH_DEFAULT		IH_ARCH_INVALID

/*
 * This structure defines all such variables those are initialized by
 * mkimage and dumpimage main core and need to be referred by image
 * type specific functions
 */
struct image_tool_params {
	int dflag;
	int eflag;
	int fflag;
	int iflag;
	int lflag;
	int pflag;
	int vflag;
	int xflag;
	int skipcpy;
	int os;
	int arch;
	int type;
	int comp;
	char *dtc;
	unsigned int addr;
	unsigned int ep;
	char *imagename;
	char *imagename2;
	char *datafile;
	char *imagefile;
	char *cmdname;
	const char *outfile;	/* Output filename */
	const char *keydir;	/* Directory holding private keys */
	const char *keydest;	/* Destination .dtb for public key */
	const char *comment;	/* Comment to add to signature node */
	int require_keys;	/* 1 to mark signing keys as 'required' */
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
	 * and are registered as flags in image_tool_params structure.
	 * This callback function can be used to check the passed arguments
	 * are in-lined with the image type to be supported
	 *
	 * Returns 1 if parameter check is successful
	 */
	int (*check_params) (struct image_tool_params *);
	/*
	 * This function is used by list command (i.e. mkimage -l <filename>)
	 * image type verification code must be put here
	 *
	 * Returns 0 if image header verification is successful
	 * otherwise, returns respective negative error codes
	 */
	int (*verify_header) (unsigned char *, int, struct image_tool_params *);
	/* Prints image information abstracting from image header */
	void (*print_header) (const void *);
	/*
	 * The header or image contents need to be set as per image type to
	 * be generated using this callback function.
	 * further output file post processing (for ex. checksum calculation,
	 * padding bytes etc..) can also be done in this callback function.
	 */
	void (*set_header) (void *, struct stat *, int,
					struct image_tool_params *);
	/*
	 * This function is used by the command to retrieve a data file from
	 * the image (i.e. dumpimage -i <image> -p <position> <data_file>).
	 * Thus the code to extract a file from an image must be put here.
	 *
	 * Returns 0 if the file was successfully retrieved from the image,
	 * or a negative value on error.
	 */
	int (*extract_datafile) (void *, struct image_tool_params *);
	/*
	 * Some image generation support for ex (default image type) supports
	 * more than one type_ids, this callback function is used to check
	 * whether input (-T <image_type>) is supported by registered image
	 * generation/list low level code
	 */
	int (*check_image_type) (uint8_t);
	/* This callback function will be executed if fflag is defined */
	int (*fflag_handle) (struct image_tool_params *);
	/*
	 * This callback function will be executed for variable size record
	 * It is expected to build this header in memory and return its length
	 * and a pointer to it by using image_type_params.header_size and
	 * image_type_params.hdr. The return value shall indicate if an
	 * additional padding should be used when copying the data image
	 * by returning the padding length.
	 */
	int (*vrec_header) (struct image_tool_params *,
		struct image_type_params *);
	/* pointer to the next registered entry in linked list */
	struct image_type_params *next;
};

/*
 * Tool registration function.
 */
typedef void (*imagetool_register_t)(struct image_type_params *);

/*
 * Initializes all image types with the given registration callback
 * function.
 * An image tool uses this function to initialize all image types.
 */
void register_image_tool(imagetool_register_t image_register);

/*
 * Register a image type within a tool.
 * An image type uses this function to register itself within
 * all tools.
 */
void register_image_type(struct image_type_params *tparams);

/*
 * There is a c file associated with supported image type low level code
 * for ex. default_image.c, fit_image.c
 * init_xxx_type() is the only function referred by image tool core to avoid
 * a single lined header file, you can define them here
 *
 * Supported image types init functions
 */
void init_default_image_type(void);
void init_atmel_image_type(void);
void init_pbl_image_type(void);
void init_ais_image_type(void);
void init_kwb_image_type(void);
void init_imx_image_type(void);
void init_mxs_image_type(void);
void init_fit_image_type(void);
void init_ubl_image_type(void);
void init_omap_image_type(void);
void init_gpimage_type(void);

void pbl_load_uboot(int fd, struct image_tool_params *mparams);

#endif /* _IMAGETOOL_H_ */

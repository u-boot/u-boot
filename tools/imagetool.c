/*
 * (C) Copyright 2013
 *
 * Written by Guilherme Maciel Ferreira <guilherme.maciel.ferreira@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "imagetool.h"

#include <image.h>

/*
 * Callback function to register a image type within a tool
 */
static imagetool_register_t register_func;

/*
 * register_image_tool -
 *
 * The tool provides its own registration function in order to all image
 * types initialize themselves.
 */
void register_image_tool(imagetool_register_t image_register)
{
	/*
	 * Save the image tool callback function. It will be used to register
	 * image types within that tool
	 */
	register_func = image_register;

	/* Init ATMEL ROM Boot Image generation/list support */
	init_atmel_image_type();
	/* Init Freescale PBL Boot image generation/list support */
	init_pbl_image_type();
	/* Init Kirkwood Boot image generation/list support */
	init_kwb_image_type();
	/* Init Freescale imx Boot image generation/list support */
	init_imx_image_type();
	/* Init Freescale mxs Boot image generation/list support */
	init_mxs_image_type();
	/* Init FIT image generation/list support */
	init_fit_image_type();
	/* Init TI OMAP Boot image generation/list support */
	init_omap_image_type();
	/* Init Default image generation/list support */
	init_default_image_type();
	/* Init Davinci UBL support */
	init_ubl_image_type();
	/* Init Davinci AIS support */
	init_ais_image_type();
	/* Init Altera SOCFPGA support */
	init_socfpga_image_type();
	/* Init TI Keystone boot image generation/list support */
	init_gpimage_type();
}

/*
 * register_image_type -
 *
 * Register a image type within a tool
 */
void register_image_type(struct image_type_params *tparams)
{
	register_func(tparams);
}

struct image_type_params *imagetool_get_type(
	int type,
	struct image_type_params *tparams)
{
	struct image_type_params *curr;

	for (curr = tparams; curr != NULL; curr = curr->next) {
		if (curr->check_image_type) {
			if (!curr->check_image_type(type))
				return curr;
		}
	}
	return NULL;
}

int imagetool_verify_print_header(
	void *ptr,
	struct stat *sbuf,
	struct image_type_params *tparams,
	struct image_tool_params *params)
{
	int retval = -1;
	struct image_type_params *curr;

	for (curr = tparams; curr != NULL; curr = curr->next) {
		if (curr->verify_header) {
			retval = curr->verify_header((unsigned char *)ptr,
						     sbuf->st_size, params);

			if (retval == 0) {
				/*
				 * Print the image information  if verify is
				 * successful
				 */
				if (curr->print_header) {
					curr->print_header(ptr);
				} else {
					fprintf(stderr,
						"%s: print_header undefined for %s\n",
						params->cmdname, curr->name);
				}
				break;
			}
		}
	}

	return retval;
}

int imagetool_save_datafile(
	const char *file_name,
	ulong file_data,
	ulong file_len)
{
	int dfd;

	dfd = open(file_name, O_RDWR | O_CREAT | O_TRUNC | O_BINARY,
		   S_IRUSR | S_IWUSR);
	if (dfd < 0) {
		fprintf(stderr, "Can't open \"%s\": %s\n",
			file_name, strerror(errno));
		return -1;
	}

	if (write(dfd, (void *)file_data, file_len) != (ssize_t)file_len) {
		fprintf(stderr, "Write error on \"%s\": %s\n",
			file_name, strerror(errno));
		close(dfd);
		return -1;
	}

	close(dfd);

	return 0;
}

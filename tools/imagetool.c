/*
 * (C) Copyright 2013
 *
 * Written by Guilherme Maciel Ferreira <guilherme.maciel.ferreira@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "imagetool.h"

#include <image.h>

struct image_type_params *imagetool_get_type(int type)
{
	struct image_type_params **curr;
	INIT_SECTION(image_type);

	struct image_type_params **start = __start_image_type;
	struct image_type_params **end = __stop_image_type;

	for (curr = start; curr != end; curr++) {
		if ((*curr)->check_image_type) {
			if (!(*curr)->check_image_type(type))
				return *curr;
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
	struct image_type_params **curr;
	INIT_SECTION(image_type);

	struct image_type_params **start = __start_image_type;
	struct image_type_params **end = __stop_image_type;

	for (curr = start; curr != end; curr++) {
		if ((*curr)->verify_header) {
			retval = (*curr)->verify_header((unsigned char *)ptr,
						     sbuf->st_size, params);

			if (retval == 0) {
				/*
				 * Print the image information  if verify is
				 * successful
				 */
				if ((*curr)->print_header) {
					(*curr)->print_header(ptr);
				} else {
					fprintf(stderr,
						"%s: print_header undefined for %s\n",
						params->cmdname, (*curr)->name);
				}
				break;
			}
		}
	}

	return retval;
}

int imagetool_save_subimage(
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

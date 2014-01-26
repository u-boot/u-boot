/*
 * (C) Copyright 2013
 *
 * Written by Guilherme Maciel Ferreira <guilherme.maciel.ferreira@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "imagetool.h"

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

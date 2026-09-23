// SPDX-License-Identifier: GPL-2.0+
/*
 * Test for vbe-simple bootmeth. All start with 'vbe_simple'
 *
 * Copyright 2023 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <image.h>
#include <test/ut.h>
#include <linux/libfdt.h>
#include "bootstd_common.h"

/* Test that the default configuration breaks best-match ties */
static int test_fit_conf_find_compat(struct unit_test_state *uts)
{
	char fdt[256], fit[1024];
	int confs, images, node;
	int ret;

	/* control devicetree with a two-entry compatible list */
	ut_assertok(fdt_create_empty_tree(fdt, sizeof(fdt)));
	ut_assertok(fdt_appendprop_string(fdt, 0, "compatible",
					  "test,board-a"));
	ut_assertok(fdt_appendprop_string(fdt, 0, "compatible",
					  "test,fallback"));

	/* FIT with two configurations matching the same compatible */
	ut_assertok(fdt_create_empty_tree(fit, sizeof(fit)));
	images = fdt_add_subnode(fit, 0, "images");
	ut_assert(images >= 0);
	confs = fdt_add_subnode(fit, 0, "configurations");
	ut_assert(confs >= 0);
	ut_assertok(fdt_setprop_string(fit, confs, FIT_DEFAULT_PROP, "conf-2"));
	/*
	 * fdt_add_subnode() inserts before existing subnodes: create conf-2
	 * first so that conf-1 ends up listed first, like an .its compiled
	 * with the configurations in that order
	 */
	node = fdt_add_subnode(fit, confs, "conf-2");
	ut_assert(node >= 0);
	ut_assertok(fdt_setprop_string(fit, node, "compatible",
				       "test,board-a"));
	node = fdt_add_subnode(fit, confs, "conf-1");
	ut_assert(node >= 0);
	ut_assertok(fdt_setprop_string(fit, node, "compatible",
				       "test,board-a"));
	confs = fdt_path_offset(fit, "/configurations");
	node = fdt_first_subnode(fit, confs);
	ut_asserteq_str("conf-1", fdt_get_name(fit, node, NULL));

	/* on a tie, the default configuration wins */
	ret = fit_conf_find_compat(fit, fdt);
	ut_assert(ret > 0);
	ut_asserteq_str("conf-2", fdt_get_name(fit, ret, NULL));

	/* without a default, the first listed configuration wins */
	confs = fdt_path_offset(fit, "/configurations");
	ut_assertok(fdt_delprop(fit, confs, FIT_DEFAULT_PROP));
	confs = fdt_path_offset(fit, "/configurations");
	ut_assertnull((void *)fdt_getprop(fit, confs, FIT_DEFAULT_PROP, NULL));
	ret = fit_conf_find_compat(fit, fdt);
	ut_assert(ret > 0);
	ut_asserteq_str("conf-1", fdt_get_name(fit, ret, NULL));

	/* a strictly better match still beats the default */
	confs = fdt_path_offset(fit, "/configurations");
	ut_assertok(fdt_setprop_string(fit, confs, FIT_DEFAULT_PROP, "conf-2"));
	confs = fdt_path_offset(fit, "/configurations");
	node = fdt_subnode_offset(fit, confs, "conf-2");
	ut_assertok(fdt_setprop_string(fit, node, "compatible",
				       "test,fallback"));
	ret = fit_conf_find_compat(fit, fdt);
	ut_assert(ret > 0);
	ut_asserteq_str("conf-1", fdt_get_name(fit, ret, NULL));

	return 0;
}
BOOTSTD_TEST(test_fit_conf_find_compat, 0);

/* Test of image phase */
static int test_image_phase(struct unit_test_state *uts)
{
	int val;

	ut_asserteq_str("U-Boot phase", genimg_get_phase_name(IH_PHASE_U_BOOT));
	ut_asserteq_str("SPL Phase", genimg_get_phase_name(IH_PHASE_SPL));
	ut_asserteq_str("any", genimg_get_phase_name(IH_PHASE_NONE));
	ut_asserteq_str("Unknown Phase", genimg_get_phase_name(-1));

	ut_asserteq(IH_PHASE_U_BOOT, genimg_get_phase_id("u-boot"));
	ut_asserteq(IH_PHASE_SPL, genimg_get_phase_id("spl"));
	ut_asserteq(IH_PHASE_NONE, genimg_get_phase_id("none"));
	ut_asserteq(-1, genimg_get_phase_id("fred"));

	val = image_ph(IH_PHASE_SPL, IH_TYPE_FIRMWARE);
	ut_asserteq(IH_PHASE_SPL, image_ph_phase(val));
	ut_asserteq(IH_TYPE_FIRMWARE, image_ph_type(val));

	return 0;
}
BOOTSTD_TEST(test_image_phase, 0);

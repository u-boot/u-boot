// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 NXP
 */

#include <common.h>
#include <asm/arch/sci/sci.h>
#include <dm/ofnode.h>
#include <fdt_support.h>

DECLARE_GLOBAL_DATA_PTR;

static bool check_owned_resource(sc_rsrc_t rsrc_id)
{
	bool owned;

	owned = sc_rm_is_resource_owned(-1, rsrc_id);

	return owned;
}

static int disable_fdt_node(void *blob, int nodeoffset)
{
	int rc, ret;
	const char *status = "disabled";

	do {
		rc = fdt_setprop(blob, nodeoffset, "status", status,
				 strlen(status) + 1);
		if (rc) {
			if (rc == -FDT_ERR_NOSPACE) {
				ret = fdt_increase_size(blob, 512);
				if (ret)
					return ret;
			}
		}
	} while (rc == -FDT_ERR_NOSPACE);

	return rc;
}

static void update_fdt_with_owned_resources(void *blob)
{
	/*
	 * Traverses the fdt nodes, check its power domain and use
	 * the resource id in the power domain for checking whether
	 * it is owned by current partition
	 */
	struct fdtdec_phandle_args args;
	int offset = 0, depth = 0;
	u32 rsrc_id;
	int rc, i;

	for (offset = fdt_next_node(blob, offset, &depth); offset > 0;
	     offset = fdt_next_node(blob, offset, &depth)) {
		debug("Node name: %s, depth %d\n",
		      fdt_get_name(blob, offset, NULL), depth);

		if (!fdt_get_property(blob, offset, "power-domains", NULL)) {
			debug("   - ignoring node %s\n",
			      fdt_get_name(blob, offset, NULL));
			continue;
		}

		if (!fdtdec_get_is_enabled(blob, offset)) {
			debug("   - ignoring node %s\n",
			      fdt_get_name(blob, offset, NULL));
			continue;
		}

		i = 0;
		while (true) {
			rc = fdtdec_parse_phandle_with_args(blob, offset,
							    "power-domains",
							    "#power-domain-cells",
							    0, i++, &args);
			if (rc == -ENOENT) {
				break;
			} else if (rc) {
				printf("Parse power-domains of %s wrong: %d\n",
				       fdt_get_name(blob, offset, NULL), rc);
				continue;
			}

			rsrc_id = args.args[0];

			if (!check_owned_resource(rsrc_id)) {
				rc = disable_fdt_node(blob, offset);
				if (!rc) {
					printf("Disable %s rsrc %u not owned\n",
					       fdt_get_name(blob, offset, NULL),
					       rsrc_id);
				} else {
					printf("Unable to disable %s, err=%s\n",
					       fdt_get_name(blob, offset, NULL),
					       fdt_strerror(rc));
				}
			}
		}
	}
}

int ft_system_setup(void *blob, bd_t *bd)
{
	update_fdt_with_owned_resources(blob);

	return 0;
}

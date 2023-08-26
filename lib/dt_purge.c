// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023, Linaro Limited
 */

#include <dt-structs.h>
#include <event.h>
#include <linker_lists.h>

#include <linux/libfdt.h>

/**
 * dt_non_compliant_purge() -	Remove non-upstreamed nodes and properties
 *				from the DT
 * @ctx: Context for event
 * @event: Event to process
 *
 * Iterate through an array of DT nodes and properties, and remove them
 * from the device-tree before the DT gets handed over to the kernel.
 * These are nodes and properties which do not have upstream bindings
 * and need to be purged before being handed over to the kernel.
 *
 * If both the node and property are specified, delete the property. If
 * only the node is specified, delete the entire node, including it's
 * subnodes, if any.
 *
 * Return: 0 if OK, -ve on error
 */
static int dt_non_compliant_purge(void *ctx, struct event *event)
{
	int nodeoff = 0;
	int err = 0;
	void *fdt;
	const struct event_ft_fixup *fixup = &event->data.ft_fixup;
	struct dt_non_compliant_purge *purge_entry;
	struct dt_non_compliant_purge *purge_start =
		ll_entry_start(struct dt_non_compliant_purge, dt_purge);
	int nentries = ll_entry_count(struct dt_non_compliant_purge, dt_purge);

	if (fixup->images)
		return 0;

	fdt = fixup->tree.fdt;
	for (purge_entry = purge_start; purge_entry != purge_start + nentries;
	     purge_entry++) {
		nodeoff = fdt_path_offset(fdt, purge_entry->node_path);
		if (nodeoff < 0) {
			log_debug("Error (%d) getting node offset for %s\n",
				  nodeoff, purge_entry->node_path);
			continue;
		}

		if (purge_entry->prop) {
			err = fdt_delprop(fdt, nodeoff, purge_entry->prop);
			if (err < 0 && err != -FDT_ERR_NOTFOUND) {
				log_debug("Error (%d) deleting %s\n",
					  err, purge_entry->prop);
				goto out;
			}
		} else {
			err = fdt_del_node(fdt, nodeoff);
			if (err) {
				log_debug("Error (%d) trying to delete node %s\n",
					  err, purge_entry->node_path);
				goto out;
			}
		}
	}

out:
	return err;
}
EVENT_SPY(EVT_FT_FIXUP, dt_non_compliant_purge);

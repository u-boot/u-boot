/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2025 Linaro Limited
 */

#ifndef _INTERCONNECT_UCLASS_H
#define _INTERCONNECT_UCLASS_H

#include <interconnect.h>

#define icc_units_to_bps(bw)  ((bw) * 1000ULL)

struct udevice;

/**
 * struct icc_req - constraints that are attached to each node
 *
 * @req_node:	entry in list of requests for the particular @node
 * @node:	the interconnect node to which this constraint applies
 * @enabled:	indicates whether the path with this request is enabled
 * @tag:	path tag (optional)
 * @avg_bw:	an integer describing the average bandwidth in kBps
 * @peak_bw:	an integer describing the peak bandwidth in kBps
 */
struct icc_req {
	struct hlist_node	req_node;
	struct icc_node		*node;
	bool			enabled;
	u32			tag;
	u32			avg_bw;
	u32			peak_bw;
};

/**
 * struct icc_path - An interconnect path
 *
 * @dev:	Device who requested the path
 * @num_nodes:	number of nodes (hops) in the path
 * @reqs:	array of the requests applicable to this path of nodes
 */
struct icc_path {
	struct udevice		*dev;
	size_t			num_nodes;
	struct icc_req		reqs[];
};

/**
 * struct icc_provider - interconnect provider (controller) entity that might
 * provide multiple interconnect controls
 *
 * @inter_set:		whether inter-provider pairs will be configured with @set
 * @xlate_num_nodes:	provider-specific nodes counts for mapping nodes from phandle arguments
 * @xlate_nodes:	provider-specific array for mapping nodes from phandle arguments
 */
struct icc_provider {
	bool			inter_set;
	unsigned int		xlate_num_nodes;
	struct icc_node		**xlate_nodes;
};

/**
 * struct icc_node - entity that is part of the interconnect topology
 *
 * @dev:		points to the interconnect provider of this node
 * @links:		a list of targets pointing to where we can go next when traversing
 * @num_links:		number of links to other interconnect nodes
 * @users:		count of active users
 * @node_list:		the list entry in the parent provider's "nodes" list
 * @search_list:	list used when walking the nodes graph
 * @reverse:		pointer to previous node when walking the nodes graph
 * @is_traversed:	flag that is used when walking the nodes graph
 * @req_list:		a list of QoS constraint requests associated with this node
 * @avg_bw:		aggregated value of average bandwidth requests from all consumers
 * @peak_bw:		aggregated value of peak bandwidth requests from all consumers
 * @data:		pointer to private data
 */
struct icc_node {
	struct udevice		*dev;
	ulong			*links;
	size_t			num_links;
	int			users;

	struct list_head	node_list;
	struct list_head	search_list;
	struct icc_node		*reverse;
	u8			is_traversed:1;
	struct hlist_head	req_list;
	u32			avg_bw;
	u32			peak_bw;
	void			*data;
};

/**
 * struct interconnect_ops - Interconnect uclass operations
 *
 * @of_xlate:		provider-specific callback for mapping nodes from phandle arguments
 * @set:		pointer to device specific set operation function
 * @pre_aggregate:	pointer to device specific function that is called
 *			before the aggregation begins (optional)
 * @aggregate:		pointer to device specific aggregate operation function
 */
struct interconnect_ops {
	struct icc_node *(*of_xlate)(struct udevice *dev,
				     const struct ofnode_phandle_args *args);
	int (*set)(struct icc_node *src, struct icc_node *dst);
	void (*pre_aggregate)(struct icc_node *node);
	int (*aggregate)(struct icc_node *node, u32 tag, u32 avg_bw,
			 u32 peak_bw, u32 *agg_avg, u32 *agg_peak);
};

/**
 * icc_node_create() - create a node
 *
 * @dev:	Provider device
 * @id:		node id, can be a numeric ID or pointer casted to ulong
 * @name:	node name
 *
 * Return: icc_node pointer on success, or ERR_PTR() on error
 */
struct icc_node *icc_node_create(struct udevice *dev,
				 ulong id, const char *name);

/**
 * icc_link_create() - create a link between two nodes
 * @node: source node id
 * @dst_id: destination node id
 *
 * Create a link between two nodes. The nodes might belong to different
 * interconnect providers and the @dst_id node might not exist, the link
 * will be done at runtime in `icc_path_find()`.
 *
 * Return: 0 on success, or an error code otherwise
 */
int icc_link_create(struct icc_node *node, const ulong dst_id);

#endif

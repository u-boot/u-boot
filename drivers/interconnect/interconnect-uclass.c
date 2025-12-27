// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2025 Linaro Limited
 * Based on the Linux Driver:
 * Copyright (c) 2017-2019, Linaro Ltd.
 * Author: Georgi Djakov <georgi.djakov@linaro.org>
 */

#define LOG_CATEGORY UCLASS_INTERCONNECT

#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <linux/err.h>
#include <interconnect.h>
#include <interconnect-uclass.h>
#include <dm/lists.h>
#include <dm/uclass-internal.h>
#include <dm/device-internal.h>
#include <dm/device_compat.h>

static struct icc_node *of_icc_get_from_provider(struct udevice *dev,
						 const struct ofnode_phandle_args *args);
static struct icc_path *icc_path_find(struct udevice *dev,
				      struct icc_node *src, struct icc_node *dst);
static struct icc_node *icc_node_find(const ulong id);

/* Public API */

struct icc_path *of_icc_get(struct udevice *dev, const char *name)
{
	int index = 0;

	if (!dev)
		return ERR_PTR(-ENODEV);

	if (!ofnode_has_property(dev_ofnode(dev), "interconnects"))
		return NULL;

	if (name) {
		index = dev_read_stringlist_search(dev, "interconnect-names", name);
		if (index < 0) {
			debug("fdt_stringlist_search() failed: %d\n", index);
			return ERR_PTR(index);
		}
	}

	return of_icc_get_by_index(dev, index);
}

struct icc_path *of_icc_get_by_index(struct udevice *dev, int index)
{
	struct ofnode_phandle_args src_args, dst_args;
	struct icc_node *src_node, *dst_node;
	struct icc_path *path;
	int ret;

	if (!dev)
		return ERR_PTR(-ENODEV);

	debug("(dev=%p,idx=%d)\n", dev, index);

	if (!ofnode_has_property(dev_ofnode(dev), "interconnects"))
		return NULL;

	ret = dev_read_phandle_with_args(dev, "interconnects",
					 "#interconnect-cells", 0, index * 2,
					 &src_args);
	if (ret) {
		dev_err(dev, "dev_read_phandle_with_args src failed: %d\n", ret);
		return ERR_PTR(ret);
	}

	ret = dev_read_phandle_with_args(dev, "interconnects",
					 "#interconnect-cells", 0, index * 2 + 1,
					 &dst_args);
	if (ret) {
		dev_err(dev, "dev_read_phandle_with_args dst failed: %d\n", ret);
		return ERR_PTR(ret);
	}

	src_node = of_icc_get_from_provider(dev, &src_args);
	if (IS_ERR(src_node)) {
		dev_err(dev, "error finding src node\n");
		return ERR_CAST(src_node);
	}

	dst_node = of_icc_get_from_provider(dev, &dst_args);
	if (IS_ERR(dst_node)) {
		dev_err(dev, "error finding dst node\n");
		return ERR_CAST(dst_node);
	}

	path = icc_path_find(dev, src_node, dst_node);
	if (IS_ERR(path))
		dev_err(dev, "invalid path=%ld\n", PTR_ERR(path));

	debug("(path=%p)\n", path);

	return path;
}

int icc_put(struct icc_path *path)
{
	struct icc_node *node;
	size_t i;
	int ret;

	debug("(path=%p)\n", path);

	if (!path || IS_ERR(path))
		return 0;

	ret = icc_set_bw(path, 0, 0);
	if (ret) {
		dev_err(path->dev, "failed to set bandwidth (%d)\n", ret);
		return ret;
	}

	for (i = 0; i < path->num_nodes; i++) {
		node = path->reqs[i].node;

		if (node->users)
			node->users--;
		if (!node->users)
			device_remove(node->dev, DM_REMOVE_NORMAL);

		hlist_del(&path->reqs[i].req_node);
	}

	kfree(path);

	return 0;
}

static int __icc_enable(struct icc_path *path, bool enable)
{
	int i;

	if (!path)
		return 0;

	if (IS_ERR(path) || !path->num_nodes)
		return -EINVAL;

	for (i = 0; i < path->num_nodes; i++)
		path->reqs[i].enabled = enable;

	return icc_set_bw(path, path->reqs[0].avg_bw,
			  path->reqs[0].peak_bw);
}

int icc_enable(struct icc_path *path)
{
	debug("(path=%p)\n", path);
	return __icc_enable(path, true);
}

int icc_disable(struct icc_path *path)
{
	debug("(path=%p)\n", path);
	return __icc_enable(path, false);
}

static int apply_constraints(struct icc_path *path)
{
	struct icc_node *next, *prev = NULL;
	const struct interconnect_ops *ops;
	struct icc_provider *provider;
	struct udevice *p;
	int ret = -EINVAL;
	int i;

	debug("(path=%p)\n", path);

	for (i = 0; i < path->num_nodes; i++) {
		next = path->reqs[i].node;
		p = next->dev->parent;
		provider = dev_get_uclass_plat(p);

		/* both endpoints should be valid master-slave pairs */
		if (!prev || (p != prev->dev->parent && !provider->inter_set)) {
			prev = next;
			continue;
		}

		debug("(path=%p,req=%d,node=%s,provider=%s)\n",
		      path, i, next->dev->name, p->name);

		ops = device_get_ops(p);

		/* set the constraints */
		if (ops->set) {
			ret = ops->set(prev, next);
			if (ret)
				goto out;
		}

		prev = next;
	}
out:
	return ret;
}

/*
 * We want the path to honor all bandwidth requests, so the average and peak
 * bandwidth requirements from each consumer are aggregated at each node.
 * The aggregation is platform specific, so each platform can customize it by
 * implementing its own aggregate() function.
 */

static int aggregate_requests(struct icc_node *node)
{
	const struct interconnect_ops *ops = device_get_ops(node->dev->parent);
	struct icc_req *r;
	u32 avg_bw, peak_bw;

	debug("(dev=%s)\n", node->dev->name);

	node->avg_bw = 0;
	node->peak_bw = 0;

	if (ops->pre_aggregate)
		ops->pre_aggregate(node);

	hlist_for_each_entry(r, &node->req_list, req_node) {
		if (r->enabled) {
			avg_bw = r->avg_bw;
			peak_bw = r->peak_bw;
		} else {
			avg_bw = 0;
			peak_bw = 0;
		}
		debug("(dev=%s,req=%s,avg=%d,peak=%d)\n",
		      node->dev->name, r->node->dev->name,
		      avg_bw, peak_bw);
		if (ops->aggregate)
			ops->aggregate(node, r->tag, avg_bw, peak_bw,
				       &node->avg_bw, &node->peak_bw);
	}

	return 0;
}

int icc_set_bw(struct icc_path *path, u32 avg_bw, u32 peak_bw)
{
	struct icc_node *node;
	u32 old_avg, old_peak;
	size_t i;
	int ret;

	debug("(path=%p,avg=%d,peak=%d)\n", path, avg_bw, peak_bw);

	if (!path)
		return 0;

	if (IS_ERR(path) || !path->num_nodes)
		return -EINVAL;

	old_avg = path->reqs[0].avg_bw;
	old_peak = path->reqs[0].peak_bw;

	for (i = 0; i < path->num_nodes; i++) {
		node = path->reqs[i].node;

		/* update the consumer request for this path */
		path->reqs[i].avg_bw = avg_bw;
		path->reqs[i].peak_bw = peak_bw;

		/* aggregate requests for this node */
		aggregate_requests(node);
	}

	ret = apply_constraints(path);
	if (ret) {
		dev_err(path->dev, "error applying constraints (%d)\n", ret);

		for (i = 0; i < path->num_nodes; i++) {
			node = path->reqs[i].node;
			path->reqs[i].avg_bw = old_avg;
			path->reqs[i].peak_bw = old_peak;
			aggregate_requests(node);
		}
		apply_constraints(path);
	}

	return ret;
}

/* Provider API */

static struct icc_path *icc_path_init(struct udevice *dev, struct icc_node *dst,
				      ssize_t num_nodes)
{
	struct icc_node *node = dst;
	struct icc_path *path;
	struct udevice *node_dev;
	int i, ret;

	debug("(dev=%s,node=%s)\n", dev->name, node->dev->name);

	path = kzalloc(sizeof(struct icc_path) +
		       sizeof(struct icc_req) * num_nodes,
		       GFP_KERNEL);
	if (!path)
		return ERR_PTR(-ENOMEM);

	path->dev = dev;
	path->num_nodes = num_nodes;

	for (i = num_nodes - 1; i >= 0; i--) {
		debug("(req[%d]=%s)\n", i, node->dev->name);
		hlist_add_head(&path->reqs[i].req_node, &node->req_list);
		path->reqs[i].node = node;
		path->reqs[i].enabled = true;

		/* Probe this node since used in an active path */
		ret = uclass_get_device_tail(node->dev, 0, &node_dev);
		if (ret)
			return ERR_PTR(ret);

		node->users++;

		/* reference to previous node was saved during path traversal */
		node = node->reverse;
	}

	return path;
}

static struct icc_path *icc_path_find(struct udevice *dev, struct icc_node *src,
				      struct icc_node *dst)
{
	struct icc_path *path = ERR_PTR(-EPROBE_DEFER);
	struct icc_node *n, *node = NULL;
	struct list_head traverse_list;
	struct list_head edge_list;
	struct list_head visited_list;
	size_t i, depth = 1;
	bool found = false;

	debug("(dev=%s,src=%s,dest=%p\n",
	      dev->name, src->dev->name, dst->dev->name);

	INIT_LIST_HEAD(&traverse_list);
	INIT_LIST_HEAD(&edge_list);
	INIT_LIST_HEAD(&visited_list);

	list_add(&src->search_list, &traverse_list);
	src->reverse = NULL;

	do {
		list_for_each_entry_safe(node, n, &traverse_list, search_list) {
			if (node == dst) {
				found = true;
				list_splice_init(&edge_list, &visited_list);
				list_splice_init(&traverse_list, &visited_list);
				break;
			}
			for (i = 0; i < node->num_links; i++) {
				struct icc_node *tmp;

				tmp = icc_node_find(node->links[i]);
				if (!tmp) {
					dev_err(dev, "missing link to node id %lx\n",
						node->links[i]);
					path = ERR_PTR(-ENOENT);
					goto out;
				}

				if (tmp->is_traversed)
					continue;

				tmp->is_traversed = true;
				tmp->reverse = node;
				list_add_tail(&tmp->search_list, &edge_list);
			}
		}

		if (found)
			break;

		list_splice_init(&traverse_list, &visited_list);
		list_splice_init(&edge_list, &traverse_list);

		/* count the hops including the source */
		depth++;

	} while (!list_empty(&traverse_list));

out:
	/* reset the traversed state */
	list_for_each_entry_reverse(n, &visited_list, search_list)
		n->is_traversed = false;

	if (found)
		path = icc_path_init(dev, dst, depth);

	return path;
}

static struct icc_node *of_icc_get_from_provider(struct udevice *dev,
						 const struct ofnode_phandle_args *args)
{
	const struct interconnect_ops *ops;
	struct udevice *icc_dev;
	int ret;

	ret = uclass_get_device_by_ofnode(UCLASS_INTERCONNECT, args->node,
					  &icc_dev);
	if (ret) {
		dev_err(dev, "uclass_get_device_by_ofnode failed: %d\n", ret);
		return ERR_PTR(ret);
	}
	ops = device_get_ops(icc_dev);

	return ops->of_xlate(icc_dev, args);
}

static struct icc_node *icc_node_find(const ulong id)
{
	struct udevice *dev;

	for (uclass_find_first_device(UCLASS_ICC_NODE, &dev);
	     dev;
	     uclass_find_next_device(&dev)) {
		if (dev_get_driver_data(dev) == id)
			return dev_get_uclass_plat(dev);
	}

	return NULL;
}

static bool icc_node_busy(struct udevice *dev)
{
	struct icc_node *node = dev_get_uclass_plat(dev);

	debug("(dev=%s,users=%d)\n", dev->name, node->users);

	return !!node->users;
}

struct icc_node *icc_node_create(struct udevice *dev,
				 ulong id, const char *name)
{
	struct udevice *node;
	struct driver *drv;
	int ret;

	drv = lists_driver_lookup_name("icc_node");
	if (!drv)
		return ERR_PTR(-ENOENT);

	ret = device_bind_with_driver_data(dev, drv, strdup(name),
					   id, ofnode_null(), &node);
	if (ret)
		return ERR_PTR(ret);

	device_set_name_alloced(node);

	return dev_get_uclass_plat(node);
}

int icc_link_create(struct icc_node *node, const ulong dst_id)
{
	ulong *new;

	new = realloc(node->links,
		      (node->num_links + 1) * sizeof(*node->links));
	if (!new)
		return -ENOMEM;

	node->links = new;
	node->links[node->num_links++] = dst_id;

	return 0;
}

static int icc_node_bind(struct udevice *dev)
{
	struct icc_node *node = dev_get_uclass_plat(dev);

	debug("(dev=%s)\n", dev->name);

	node->dev = dev;

	return 0;
}

static int icc_node_probe(struct udevice *dev)
{
	struct icc_node *node = dev_get_uclass_plat(dev);

	debug("(dev=%s,parent=%p,id=%lx)\n",
	      dev->name, dev->parent->name, dev_get_driver_data(dev));

	node->avg_bw = 0;
	node->peak_bw = 0;

	return 0;
}

static int icc_node_remove(struct udevice *dev)
{
	debug("(dev=%s,parent=%s,id=%lx)\n",
	      dev->name, dev->parent->name, dev_get_driver_data(dev));

	if (icc_node_busy(dev))
		return -EBUSY;

	return 0;
}

static int icc_node_unbind(struct udevice *dev)
{
	struct icc_node *node = dev_get_uclass_plat(dev);

	debug("(dev=%s,id=%lx)\n",
	      dev->name, dev_get_driver_data(dev));

	kfree(node->links);

	return 0;
}

UCLASS_DRIVER(interconnect) = {
	.id		= UCLASS_INTERCONNECT,
	.name		= "interconnect",
	.per_device_plat_auto = sizeof(struct icc_provider),
};

U_BOOT_DRIVER(icc_node) = {
	.name		= "icc_node",
	.id		= UCLASS_ICC_NODE,
	.bind		= icc_node_bind,
	.probe		= icc_node_probe,
	.remove		= icc_node_remove,
	.unbind		= icc_node_unbind,
};

UCLASS_DRIVER(icc_node) = {
	.id		= UCLASS_ICC_NODE,
	.name		= "icc_node",
	.per_device_plat_auto = sizeof(struct icc_node),
};

// SPDX-License-Identifier: GPL-2.0+
/*
 * Originally from Linux v4.9
 * Paul Mackerras	August 1996.
 * Copyright (C) 1996-2005 Paul Mackerras.
 *
 * Adapted for 64bit PowerPC by Dave Engebretsen and Peter Bergner.
 *   {engebret|bergner}@us.ibm.com
 *
 * Adapted for sparc and sparc64 by David S. Miller davem@davemloft.net
 *
 * Reconsolidated from arch/x/kernel/prom.c by Stephen Rothwell and
 * Grant Likely.
 *
 * Modified for U-Boot
 * Copyright (c) 2017 Google, Inc
 *
 * This file follows drivers/of/base.c with functions in the same order as the
 * Linux version.
 */

#include <log.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <linux/bug.h>
#include <linux/libfdt.h>
#include <dm/of_access.h>
#include <dm/util.h>
#include <linux/ctype.h>
#include <linux/err.h>
#include <linux/ioport.h>

DECLARE_GLOBAL_DATA_PTR;

/* list of struct alias_prop aliases */
static LIST_HEAD(aliases_lookup);

/* "/aliaes" node */
static struct device_node *of_aliases;

/* "/chosen" node */
static struct device_node *of_chosen;

/* node pointed to by the stdout-path alias */
static struct device_node *of_stdout;

/* pointer to options given after the alias (separated by :) or NULL if none */
static const char *of_stdout_options;

/**
 * struct alias_prop - Alias property in 'aliases' node
 *
 * The structure represents one alias property of 'aliases' node as
 * an entry in aliases_lookup list.
 *
 * @link:	List node to link the structure in aliases_lookup list
 * @alias:	Alias property name
 * @np:		Pointer to device_node that the alias stands for
 * @id:		Index value from end of alias name
 * @stem:	Alias string without the index
 */
struct alias_prop {
	struct list_head link;
	const char *alias;
	struct device_node *np;
	int id;
	char stem[0];
};

int of_n_addr_cells(const struct device_node *np)
{
	const __be32 *ip;

	do {
		if (np->parent)
			np = np->parent;
		ip = of_get_property(np, "#address-cells", NULL);
		if (ip)
			return be32_to_cpup(ip);
	} while (np->parent);

	/* No #address-cells property for the root node */
	return OF_ROOT_NODE_ADDR_CELLS_DEFAULT;
}

int of_n_size_cells(const struct device_node *np)
{
	const __be32 *ip;

	do {
		if (np->parent)
			np = np->parent;
		ip = of_get_property(np, "#size-cells", NULL);
		if (ip)
			return be32_to_cpup(ip);
	} while (np->parent);

	/* No #size-cells property for the root node */
	return OF_ROOT_NODE_SIZE_CELLS_DEFAULT;
}

int of_simple_addr_cells(const struct device_node *np)
{
	const __be32 *ip;

	ip = of_get_property(np, "#address-cells", NULL);
	if (ip)
		return be32_to_cpup(ip);

	/* Return a default of 2 to match fdt_address_cells()*/
	return 2;
}

int of_simple_size_cells(const struct device_node *np)
{
	const __be32 *ip;

	ip = of_get_property(np, "#size-cells", NULL);
	if (ip)
		return be32_to_cpup(ip);

	/* Return a default of 2 to match fdt_size_cells()*/
	return 2;
}

struct property *of_find_property(const struct device_node *np,
				  const char *name, int *lenp)
{
	struct property *pp;

	if (!np)
		return NULL;

	for (pp = np->properties; pp; pp = pp->next) {
		if (strcmp(pp->name, name) == 0) {
			if (lenp)
				*lenp = pp->length;
			break;
		}
	}
	if (!pp && lenp)
		*lenp = -FDT_ERR_NOTFOUND;

	return pp;
}

struct device_node *of_find_all_nodes(struct device_node *prev)
{
	struct device_node *np;

	if (!prev) {
		np = gd->of_root;
	} else if (prev->child) {
		np = prev->child;
	} else {
		/*
		 * Walk back up looking for a sibling, or the end of the
		 * structure
		 */
		np = prev;
		while (np->parent && !np->sibling)
			np = np->parent;
		np = np->sibling; /* Might be null at the end of the tree */
	}

	return np;
}

const void *of_get_property(const struct device_node *np, const char *name,
			    int *lenp)
{
	struct property *pp = of_find_property(np, name, lenp);

	return pp ? pp->value : NULL;
}

const struct property *of_get_first_property(const struct device_node *np)
{
	if (!np)
		return NULL;

	return  np->properties;
}

const struct property *of_get_next_property(const struct device_node *np,
					    const struct property *property)
{
	if (!np)
		return NULL;

	return property->next;
}

const void *of_get_property_by_prop(const struct device_node *np,
				    const struct property *property,
				    const char **name,
				    int *lenp)
{
	if (!np || !property)
		return NULL;
	if (name)
		*name = property->name;
	if (lenp)
		*lenp = property->length;

	return property->value;
}

static const char *of_prop_next_string(struct property *prop, const char *cur)
{
	const void *curv = cur;

	if (!prop)
		return NULL;

	if (!cur)
		return prop->value;

	curv += strlen(cur) + 1;
	if (curv >= prop->value + prop->length)
		return NULL;

	return curv;
}

int of_device_is_compatible(const struct device_node *device,
			    const char *compat, const char *type,
			    const char *name)
{
	struct property *prop;
	const char *cp;
	int index = 0, score = 0;

	/* Compatible match has highest priority */
	if (compat && compat[0]) {
		prop = of_find_property(device, "compatible", NULL);
		for (cp = of_prop_next_string(prop, NULL); cp;
		     cp = of_prop_next_string(prop, cp), index++) {
			if (of_compat_cmp(cp, compat, strlen(compat)) == 0) {
				score = INT_MAX/2 - (index << 2);
				break;
			}
		}
		if (!score)
			return 0;
	}

	/* Matching type is better than matching name */
	if (type && type[0]) {
		if (!device->type || of_node_cmp(type, device->type))
			return 0;
		score += 2;
	}

	/* Matching name is a bit better than not */
	if (name && name[0]) {
		if (!device->name || of_node_cmp(name, device->name))
			return 0;
		score++;
	}

	return score;
}

bool of_device_is_available(const struct device_node *device)
{
	const char *status;
	int statlen;

	if (!device)
		return false;

	status = of_get_property(device, "status", &statlen);
	if (status == NULL)
		return true;

	if (statlen > 0) {
		if (!strcmp(status, "okay"))
			return true;
	}

	return false;
}

struct device_node *of_get_parent(const struct device_node *node)
{
	const struct device_node *np;

	if (!node)
		return NULL;

	np = of_node_get(node->parent);

	return (struct device_node *)np;
}

static struct device_node *__of_get_next_child(const struct device_node *node,
					       struct device_node *prev)
{
	struct device_node *next;

	if (!node)
		return NULL;

	next = prev ? prev->sibling : node->child;
	/*
	 * coverity[dead_error_line : FALSE]
	 * Dead code here since our current implementation of of_node_get()
	 * always returns NULL (Coverity CID 163245). But we leave it as is
	 * since we may want to implement get/put later.
	 */
	for (; next; next = next->sibling)
		if (of_node_get(next))
			break;
	of_node_put(prev);
	return next;
}

#define __for_each_child_of_node(parent, child) \
	for (child = __of_get_next_child(parent, NULL); child != NULL; \
	     child = __of_get_next_child(parent, child))

static struct device_node *__of_find_node_by_path(struct device_node *parent,
						  const char *path)
{
	struct device_node *child;
	int len;

	len = strcspn(path, "/:");
	if (!len)
		return NULL;

	__for_each_child_of_node(parent, child) {
		const char *name = strrchr(child->full_name, '/');

		name++;
		if (strncmp(path, name, len) == 0 && (strlen(name) == len))
			return child;
	}
	return NULL;
}

#define for_each_property_of_node(dn, pp) \
	for (pp = dn->properties; pp != NULL; pp = pp->next)

struct device_node *of_find_node_opts_by_path(struct device_node *root,
					      const char *path,
					      const char **opts)
{
	struct device_node *np = NULL;
	struct property *pp;
	const char *separator = strchr(path, ':');

	if (!root)
		root = gd->of_root;
	if (opts)
		*opts = separator ? separator + 1 : NULL;

	if (strcmp(path, "/") == 0)
		return of_node_get(root);

	/* The path could begin with an alias */
	if (*path != '/') {
		int len;
		const char *p = separator;

		/* Only allow alias processing on the control FDT */
		if (root != gd->of_root)
			return NULL;
		if (!p)
			p = strchrnul(path, '/');
		len = p - path;

		/* of_aliases must not be NULL */
		if (!of_aliases)
			return NULL;

		for_each_property_of_node(of_aliases, pp) {
			if (strlen(pp->name) == len && !strncmp(pp->name, path,
								len)) {
				np = of_find_node_by_path(pp->value);
				break;
			}
		}
		if (!np)
			return NULL;
		path = p;
	}

	/* Step down the tree matching path components */
	if (!np)
		np = of_node_get(root);
	while (np && *path == '/') {
		struct device_node *tmp = np;

		path++; /* Increment past '/' delimiter */
		np = __of_find_node_by_path(np, path);
		of_node_put(tmp);
		path = strchrnul(path, '/');
		if (separator && separator < path)
			break;
	}

	return np;
}

struct device_node *of_find_compatible_node(struct device_node *from,
		const char *type, const char *compatible)
{
	struct device_node *np;

	for_each_of_allnodes_from(from, np)
		if (of_device_is_compatible(np, compatible, type, NULL) &&
		    of_node_get(np))
			break;
	of_node_put(from);

	return np;
}

static int of_device_has_prop_value(const struct device_node *device,
				    const char *propname, const void *propval,
				    int proplen)
{
	struct property *prop = of_find_property(device, propname, NULL);

	if (!prop || !prop->value || prop->length != proplen)
		return 0;
	return !memcmp(prop->value, propval, proplen);
}

struct device_node *of_find_node_by_prop_value(struct device_node *from,
					       const char *propname,
					       const void *propval, int proplen)
{
	struct device_node *np;

	for_each_of_allnodes_from(from, np) {
		if (of_device_has_prop_value(np, propname, propval, proplen) &&
		    of_node_get(np))
			break;
	}
	of_node_put(from);

	return np;
}

struct device_node *of_find_node_by_phandle(struct device_node *root,
					    phandle handle)
{
	struct device_node *np;

	if (!handle)
		return NULL;

	for_each_of_allnodes_from(root, np)
		if (np->phandle == handle)
			break;
	(void)of_node_get(np);

	return np;
}

/**
 * of_find_property_value_of_size() - find property of given size
 *
 * Search for a property in a device node and validate the requested size.
 *
 * @np:		device node from which the property value is to be read.
 * @propname:	name of the property to be searched.
 * @len:	requested length of property value
 *
 * Return: the property value on success, -EINVAL if the property does not
 * exist and -EOVERFLOW if the property data isn't large enough.
 */
static void *of_find_property_value_of_size(const struct device_node *np,
					    const char *propname, u32 len)
{
	struct property *prop = of_find_property(np, propname, NULL);

	if (!prop)
		return ERR_PTR(-EINVAL);
	if (len > prop->length)
		return ERR_PTR(-EOVERFLOW);

	return prop->value;
}

int of_read_u8(const struct device_node *np, const char *propname, u8 *outp)
{
	const u8 *val;

	log_debug("%s: %s: ", __func__, propname);
	if (!np)
		return -EINVAL;
	val = of_find_property_value_of_size(np, propname, sizeof(*outp));
	if (IS_ERR(val)) {
		log_debug("(not found)\n");
		return PTR_ERR(val);
	}

	*outp = *val;
	log_debug("%#x (%d)\n", *outp, *outp);

	return 0;
}

int of_read_u16(const struct device_node *np, const char *propname, u16 *outp)
{
	const __be16 *val;

	log_debug("%s: %s: ", __func__, propname);
	if (!np)
		return -EINVAL;
	val = of_find_property_value_of_size(np, propname, sizeof(*outp));
	if (IS_ERR(val)) {
		log_debug("(not found)\n");
		return PTR_ERR(val);
	}

	*outp = be16_to_cpup(val);
	log_debug("%#x (%d)\n", *outp, *outp);

	return 0;
}

int of_read_u32(const struct device_node *np, const char *propname, u32 *outp)
{
	return of_read_u32_index(np, propname, 0, outp);
}

int of_read_u32_array(const struct device_node *np, const char *propname,
		      u32 *out_values, size_t sz)
{
	const __be32 *val;

	log_debug("%s: %s: ", __func__, propname);
	val = of_find_property_value_of_size(np, propname,
					     sz * sizeof(*out_values));

	if (IS_ERR(val))
		return PTR_ERR(val);

	log_debug("size %zd\n", sz);
	while (sz--)
		*out_values++ = be32_to_cpup(val++);

	return 0;
}

int of_read_u32_index(const struct device_node *np, const char *propname,
		      int index, u32 *outp)
{
	const __be32 *val;

	log_debug("%s: %s: ", __func__, propname);
	if (!np)
		return -EINVAL;

	val = of_find_property_value_of_size(np, propname,
					     sizeof(*outp) * (index + 1));
	if (IS_ERR(val)) {
		log_debug("(not found)\n");
		return PTR_ERR(val);
	}

	*outp = be32_to_cpup(val + index);
	log_debug("%#x (%d)\n", *outp, *outp);

	return 0;
}

int of_read_u64_index(const struct device_node *np, const char *propname,
		      int index, u64 *outp)
{
	const __be64 *val;

	log_debug("%s: %s: ", __func__, propname);
	if (!np)
		return -EINVAL;

	val = of_find_property_value_of_size(np, propname,
					     sizeof(*outp) * (index + 1));
	if (IS_ERR(val)) {
		log_debug("(not found)\n");
		return PTR_ERR(val);
	}

	*outp = be64_to_cpup(val + index);
	log_debug("%#llx (%lld)\n", (unsigned long long)*outp,
		  (unsigned long long)*outp);

	return 0;
}

int of_read_u64(const struct device_node *np, const char *propname, u64 *outp)
{
	return of_read_u64_index(np, propname, 0, outp);
}

int of_property_match_string(const struct device_node *np, const char *propname,
			     const char *string)
{
	int len = 0;
	const struct property *prop = of_find_property(np, propname, &len);
	size_t l;
	int i;
	const char *p, *end;

	if (!prop && len == -FDT_ERR_NOTFOUND)
		return -ENOENT;
	if (!prop)
		return -EINVAL;
	if (!prop->value)
		return -ENODATA;

	p = prop->value;
	end = p + prop->length;

	for (i = 0; p < end; i++, p += l) {
		l = strnlen(p, end - p) + 1;
		if (p + l > end)
			return -EILSEQ;
		log_debug("comparing %s with %s\n", string, p);
		if (strcmp(string, p) == 0)
			return i; /* Found it; return index */
	}
	return -ENODATA;
}

/**
 * of_property_read_string_helper() - Utility helper for parsing string properties
 * @np:		device node from which the property value is to be read.
 * @propname:	name of the property to be searched.
 * @out_strs:	output array of string pointers.
 * @sz:		number of array elements to read.
 * @skip:	Number of strings to skip over at beginning of list (cannot be
 *	negative)
 *
 * Don't call this function directly. It is a utility helper for the
 * of_property_read_string*() family of functions.
 */
int of_property_read_string_helper(const struct device_node *np,
				   const char *propname, const char **out_strs,
				   size_t sz, int skip)
{
	const struct property *prop = of_find_property(np, propname, NULL);
	int l = 0, i = 0;
	const char *p, *end;

	if (!prop)
		return -EINVAL;
	if (!prop->value)
		return -ENODATA;
	p = prop->value;
	end = p + prop->length;

	for (i = 0; p < end && (!out_strs || i < skip + sz); i++, p += l) {
		l = strnlen(p, end - p) + 1;
		if (p + l > end)
			return -EILSEQ;
		if (out_strs && i >= skip)
			*out_strs++ = p;
	}
	i -= skip;
	return i <= 0 ? -ENODATA : i;
}

static int __of_root_parse_phandle_with_args(struct device_node *root,
					     const struct device_node *np,
					     const char *list_name,
					     const char *cells_name,
					     int cell_count, int index,
					     struct of_phandle_args *out_args)
{
	const __be32 *list, *list_end;
	int rc = 0, cur_index = 0;
	uint32_t count;
	struct device_node *node = NULL;
	phandle phandle;
	int size;

	/* Retrieve the phandle list property */
	list = of_get_property(np, list_name, &size);
	if (!list)
		return -ENOENT;
	list_end = list + size / sizeof(*list);

	/* Loop over the phandles until all the requested entry is found */
	while (list < list_end) {
		rc = -EINVAL;
		count = 0;

		/*
		 * If phandle is 0, then it is an empty entry with no
		 * arguments.  Skip forward to the next entry.
		 */
		phandle = be32_to_cpup(list++);
		if (phandle) {
			/*
			 * Find the provider node and parse the #*-cells
			 * property to determine the argument length.
			 *
			 * This is not needed if the cell count is hard-coded
			 * (i.e. cells_name not set, but cell_count is set),
			 * except when we're going to return the found node
			 * below.
			 */
			if (cells_name || cur_index == index) {
				node = of_find_node_by_phandle(root, phandle);
				if (!node) {
					dm_warn("%s: could not find phandle\n",
						np->full_name);
					goto err;
				}
			}

			if (cells_name) {
				if (of_read_u32(node, cells_name, &count)) {
					dm_warn("%s: could not get %s for %s\n",
						np->full_name, cells_name,
						node->full_name);
					goto err;
				}
			} else {
				count = cell_count;
			}

			/*
			 * Make sure that the arguments actually fit in the
			 * remaining property data length
			 */
			if (list + count > list_end) {
				dm_warn("%s: arguments longer than property\n",
					np->full_name);
				goto err;
			}
		}

		/*
		 * All of the error cases above bail out of the loop, so at
		 * this point, the parsing is successful. If the requested
		 * index matches, then fill the out_args structure and return,
		 * or return -ENOENT for an empty entry.
		 */
		rc = -ENOENT;
		if (cur_index == index) {
			if (!phandle)
				goto err;

			if (out_args) {
				int i;
				if (WARN_ON(count > OF_MAX_PHANDLE_ARGS))
					count = OF_MAX_PHANDLE_ARGS;
				out_args->np = node;
				out_args->args_count = count;
				for (i = 0; i < count; i++)
					out_args->args[i] =
							be32_to_cpup(list++);
			} else {
				of_node_put(node);
			}

			/* Found it! return success */
			return 0;
		}

		of_node_put(node);
		node = NULL;
		list += count;
		cur_index++;
	}

	/*
	 * Unlock node before returning result; will be one of:
	 * -ENOENT : index is for empty phandle
	 * -EINVAL : parsing error on data
	 * [1..n]  : Number of phandle (count mode; when index = -1)
	 */
	rc = index < 0 ? cur_index : -ENOENT;
 err:
	if (node)
		of_node_put(node);
	return rc;
}

struct device_node *of_root_parse_phandle(struct device_node *root,
					  const struct device_node *np,
					  const char *phandle_name, int index)
{
	struct of_phandle_args args;

	if (index < 0)
		return NULL;

	if (__of_root_parse_phandle_with_args(root, np, phandle_name, NULL, 0,
					      index, &args))
		return NULL;

	return args.np;
}

int of_root_parse_phandle_with_args(struct device_node *root,
				    const struct device_node *np,
				    const char *list_name, const char *cells_name,
				    int cell_count, int index,
				    struct of_phandle_args *out_args)
{
	if (index < 0)
		return -EINVAL;

	return __of_root_parse_phandle_with_args(root, np, list_name, cells_name,
						 cell_count, index, out_args);
}

int of_root_count_phandle_with_args(struct device_node *root,
				    const struct device_node *np,
				    const char *list_name, const char *cells_name,
				    int cell_count)
{
	return __of_root_parse_phandle_with_args(root, np, list_name, cells_name,
						 cell_count, -1, NULL);
}

struct device_node *of_parse_phandle(const struct device_node *np,
				     const char *phandle_name, int index)
{
	return of_root_parse_phandle(NULL, np, phandle_name, index);
}

int of_parse_phandle_with_args(const struct device_node *np,
			       const char *list_name, const char *cells_name,
			       int cell_count, int index,
			       struct of_phandle_args *out_args)
{
	return of_root_parse_phandle_with_args(NULL, np, list_name, cells_name,
					       cell_count, index, out_args);
}

int of_count_phandle_with_args(const struct device_node *np,
			       const char *list_name, const char *cells_name,
			       int cell_count)
{
	return of_root_count_phandle_with_args(NULL, np, list_name, cells_name,
					       cell_count);
}

static void of_alias_add(struct alias_prop *ap, struct device_node *np,
			 int id, const char *stem, int stem_len)
{
	ap->np = np;
	ap->id = id;
	strncpy(ap->stem, stem, stem_len);
	ap->stem[stem_len] = 0;
	list_add_tail(&ap->link, &aliases_lookup);
	log_debug("adding DT alias:%s: stem=%s id=%i node=%s\n",
		  ap->alias, ap->stem, ap->id, of_node_full_name(np));
}

int of_alias_scan(void)
{
	struct property *pp;

	of_aliases = of_find_node_by_path("/aliases");
	of_chosen = of_find_node_by_path("/chosen");
	if (of_chosen == NULL)
		of_chosen = of_find_node_by_path("/chosen@0");

	if (of_chosen) {
		const char *name;

		name = of_get_property(of_chosen, "stdout-path", NULL);
		if (name)
			of_stdout = of_find_node_opts_by_path(NULL, name,
							&of_stdout_options);
	}

	if (!of_aliases)
		return 0;

	for_each_property_of_node(of_aliases, pp) {
		const char *start = pp->name;
		const char *end = start + strlen(start);
		struct device_node *np;
		struct alias_prop *ap;
		ulong id;
		int len;

		/* Skip those we do not want to proceed */
		if (!strcmp(pp->name, "name") ||
		    !strcmp(pp->name, "phandle") ||
		    !strcmp(pp->name, "linux,phandle"))
			continue;

		np = of_find_node_by_path(pp->value);
		if (!np)
			continue;

		/*
		 * walk the alias backwards to extract the id and work out
		 * the 'stem' string
		 */
		while (isdigit(*(end-1)) && end > start)
			end--;
		len = end - start;

		if (strict_strtoul(end, 10, &id) < 0)
			continue;

		/* Allocate an alias_prop with enough space for the stem */
		ap = malloc(sizeof(*ap) + len + 1);
		if (!ap)
			return -ENOMEM;
		memset(ap, 0, sizeof(*ap) + len + 1);
		ap->alias = start;
		of_alias_add(ap, np, id, start, len);
	}

	return 0;
}

int of_alias_get_id(const struct device_node *np, const char *stem)
{
	struct alias_prop *app;
	int id = -ENODEV;

	mutex_lock(&of_mutex);
	list_for_each_entry(app, &aliases_lookup, link) {
		if (strcmp(app->stem, stem) != 0)
			continue;

		if (np == app->np) {
			id = app->id;
			break;
		}
	}
	mutex_unlock(&of_mutex);

	return id;
}

int of_alias_get_highest_id(const char *stem)
{
	struct alias_prop *app;
	int id = -1;

	mutex_lock(&of_mutex);
	list_for_each_entry(app, &aliases_lookup, link) {
		if (strcmp(app->stem, stem) != 0)
			continue;

		if (app->id > id)
			id = app->id;
	}
	mutex_unlock(&of_mutex);

	return id;
}

struct device_node *of_get_stdout(void)
{
	return of_stdout;
}

int of_write_prop(struct device_node *np, const char *propname, int len,
		  const void *value)
{
	struct property *pp;
	struct property *pp_last = NULL;
	struct property *new;

	if (!np)
		return -EINVAL;

	for (pp = np->properties; pp; pp = pp->next) {
		if (strcmp(pp->name, propname) == 0) {
			/* Property exists -> change value */
			pp->value = (void *)value;
			pp->length = len;
			return 0;
		}
		pp_last = pp;
	}

	/* Property does not exist -> append new property */
	new = malloc(sizeof(struct property));
	if (!new)
		return -ENOMEM;

	new->name = strdup(propname);
	if (!new->name) {
		free(new);
		return -ENOMEM;
	}

	new->value = (void *)value;
	new->length = len;
	new->next = NULL;

	if (pp_last)
		pp_last->next = new;
	else
		np->properties = new;

	return 0;
}

int of_add_subnode(struct device_node *parent, const char *name, int len,
		   struct device_node **childp)
{
	struct device_node *child, *new, *last_sibling = NULL;
	char *new_name, *full_name;
	int parent_fnl;

	if (len == -1)
		len = strlen(name);
	__for_each_child_of_node(parent, child) {
		/*
		 * make sure we don't use a child called "trevor" when we are
		 * searching for "trev".
		 */
		if (!strncmp(child->name, name, len) && strlen(name) == len) {
			*childp = child;
			return -EEXIST;
		}
		last_sibling = child;
	}

	/* Subnode does not exist -> append new subnode */
	new = calloc(1, sizeof(struct device_node));
	if (!new)
		return -ENOMEM;

	new_name = memdup(name, len + 1);
	if (!new_name) {
		free(new);
		return -ENOMEM;
	}
	new_name[len] = '\0';

	/*
	 * if the parent is the root node (named "") we don't need to prepend
	 * its full path
	 */
	parent_fnl = *parent->name ? strlen(parent->full_name) : 0;
	full_name = calloc(1, parent_fnl + 1 + len + 1);
	if (!full_name) {
		free(new_name);
		free(new);
		return -ENOMEM;
	}
	new->name = new_name;	/* assign to constant pointer */

	strcpy(full_name, parent->full_name); /* "" for root node */
	full_name[parent_fnl] = '/';
	strlcpy(&full_name[parent_fnl + 1], name, len + 1);
	new->full_name = full_name;

	/* Add as last sibling of the parent */
	if (last_sibling)
		last_sibling->sibling = new;
	if (!parent->child)
		parent->child = new;
	new->parent = parent;

	*childp = new;

	return 0;
}

int __of_remove_property(struct device_node *np, struct property *prop)
{
	struct property **next;

	for (next = &np->properties; *next; next = &(*next)->next) {
		if (*next == prop)
			break;
	}
	if (!*next)
		return -ENODEV;

	/* found the node */
	*next = prop->next;

	return 0;
}

int of_remove_property(struct device_node *np, struct property *prop)
{
	int rc;

	mutex_lock(&of_mutex);

	rc = __of_remove_property(np, prop);

	mutex_unlock(&of_mutex);

	return rc;
}

int of_remove_node(struct device_node *to_remove)
{
	struct device_node *parent = to_remove->parent;
	struct device_node *np, *prev;

	if (!parent)
		return -EPERM;
	prev = NULL;
	__for_each_child_of_node(parent, np) {
		if (np == to_remove)
			break;
		prev = np;
	}
	if (!np)
		return -EFAULT;

	/* if there is a previous node, link it to this one's sibling */
	if (prev)
		prev->sibling = np->sibling;
	else
		parent->child = np->sibling;

	/*
	 * don't free it, since if this is an unflattened tree, all the memory
	 * was alloced in one block; this pointer will be somewhere in the
	 * middle of that
	 *
	 * TODO(sjg@chromium.org): Consider marking nodes as 'allocated'?
	 *
	 * free(np);
	 */

	return 0;
}

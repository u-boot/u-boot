/*
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _DM_OFNODE_H
#define _DM_OFNODE_H

/**
 * ofnode - reference to a device tree node
 *
 * This union can hold either a straightforward pointer to a struct device_node
 * in the live device tree, or an offset within the flat device tree. In the
 * latter case, the pointer value is just the integer offset within the flat DT.
 *
 * Thus we can reference nodes in both the live tree (once available) and the
 * flat tree (until then). Functions are available to translate between an
 * ofnode and either an offset or a struct device_node *.
 *
 * The reference can also hold a null offset, in which case the pointer value
 * here is (void *)-1. This corresponds to a struct device_node * value of
 * NULL, or an offset of -1.
 *
 * There is no ambiguity as to whether ofnode holds an offset or a node
 * pointer: when the live tree is active it holds a node pointer, otherwise it
 * holds an offset. The value itself does not need to be unique and in theory
 * the same value could point to a valid device node or a valid offset. We
 * could arrange for a unique value to be used (e.g. by making the pointer
 * point to an offset within the flat device tree in the case of an offset) but
 * this increases code size slightly due to the subtraction. Since it offers no
 * real benefit, the approach described here seems best.
 *
 * For now these points use constant types, since we don't allow writing
 * the DT.
 *
 * @np: Pointer to device node, used for live tree
 * @flat_ptr: Pointer into flat device tree, used for flat tree. Note that this
 *	is not a really a pointer to a node: it is an offset value. See above.
 */
typedef union ofnode_union {
	const struct device_node *np;	/* will be used for future live tree */
	long of_offset;
} ofnode;

/**
 * ofnode_to_offset() - convert an ofnode to a flat DT offset
 *
 * This cannot be called if the reference contains a node pointer.
 *
 * @node: Reference containing offset (possibly invalid)
 * @return DT offset (can be -1)
 */
static inline int ofnode_to_offset(ofnode node)
{
	return node.of_offset;
}

/**
 * ofnode_valid() - check if an ofnode is valid
 *
 * @return true if the reference contains a valid ofnode, false if it is NULL
 */
static inline bool ofnode_valid(ofnode node)
{
	return node.of_offset != -1;
}

/**
 * offset_to_ofnode() - convert a DT offset to an ofnode
 *
 * @of_offset: DT offset (either valid, or -1)
 * @return reference to the associated DT offset
 */
static inline ofnode offset_to_ofnode(int of_offset)
{
	ofnode node;

	node.of_offset = of_offset;

	return node;
}

/**
 * ofnode_equal() - check if two references are equal
 *
 * @return true if equal, else false
 */
static inline bool ofnode_equal(ofnode ref1, ofnode ref2)
{
	/* We only need to compare the contents */
	return ref1.of_offset == ref2.of_offset;
}

#endif

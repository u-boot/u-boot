// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Sean Anderson <seanga2@gmail.com>
 */

#define LOG_CATEGORY UCLASS_CLK

#include <common.h>
#include <clk.h>
#include <clk-uclass.h>
#include <dm.h>
#include <log.h>
#include <kendryte/bypass.h>
#include <linux/clk-provider.h>
#include <linux/err.h>

#define CLK_K210_BYPASS "k210_clk_bypass"

/*
 * This is a small driver to do a software bypass of a clock if hardware bypass
 * is not working. I have tried to write this in a generic fashion, so that it
 * could be potentially broken out of the kendryte code at some future date.
 *
 * Say you have the following clock configuration
 *
 * +---+ +---+
 * |osc| |pll|
 * +---+ +---+
 *         ^
 *        /|
 *       / |
 *      /  |
 *     /   |
 *    /    |
 * +---+ +---+
 * |clk| |clk|
 * +---+ +---+
 *
 * But the pll does not have a bypass, so when you configure the pll, the
 * configuration needs to change to look like
 *
 * +---+ +---+
 * |osc| |pll|
 * +---+ +---+
 *   ^
 *   |\
 *   | \
 *   |  \
 *   |   \
 *   |    \
 * +---+ +---+
 * |clk| |clk|
 * +---+ +---+
 *
 * To set this up, create a bypass clock with bypassee=pll and alt=osc. When
 * creating the child clocks, set their parent to the bypass clock. After
 * creating all the children, call k210_bypass_setchildren().
 */

static int k210_bypass_dobypass(struct k210_bypass *bypass)
{
	int ret, i;

	/*
	 * If we already have saved parents, then the children are already
	 * bypassed
	 */
	if (bypass->child_count && bypass->saved_parents[0])
		return 0;

	for (i = 0; i < bypass->child_count; i++) {
		struct clk *child = bypass->children[i];
		struct clk *parent = clk_get_parent(child);

		if (IS_ERR(parent)) {
			for (; i; i--)
				bypass->saved_parents[i] = NULL;
			return PTR_ERR(parent);
		}
		bypass->saved_parents[i] = parent;
	}

	for (i = 0; i < bypass->child_count; i++) {
		struct clk *child = bypass->children[i];

		ret = clk_set_parent(child, bypass->alt);
		if (ret) {
			for (; i; i--)
				clk_set_parent(bypass->children[i],
					       bypass->saved_parents[i]);
			for (i = 0; i < bypass->child_count; i++)
				bypass->saved_parents[i] = NULL;
			return ret;
		}
	}

	return 0;
}

static int k210_bypass_unbypass(struct k210_bypass *bypass)
{
	int err, ret, i;

	if (!bypass->child_count && !bypass->saved_parents[0]) {
		log_warning("Cannot unbypass children; dobypass not called first\n");
		return 0;
	}

	ret = 0;
	for (i = 0; i < bypass->child_count; i++) {
		err = clk_set_parent(bypass->children[i],
				     bypass->saved_parents[i]);
		if (err)
			ret = err;
		bypass->saved_parents[i] = NULL;
	}
	return ret;
}

static ulong k210_bypass_get_rate(struct clk *clk)
{
	struct k210_bypass *bypass = to_k210_bypass(clk);
	const struct clk_ops *ops = bypass->bypassee_ops;

	if (ops->get_rate)
		return ops->get_rate(bypass->bypassee);
	else
		return clk_get_parent_rate(bypass->bypassee);
}

static ulong k210_bypass_set_rate(struct clk *clk, unsigned long rate)
{
	int ret;
	struct k210_bypass *bypass = to_k210_bypass(clk);
	const struct clk_ops *ops = bypass->bypassee_ops;

	/* Don't bother bypassing if we aren't going to set the rate */
	if (!ops->set_rate)
		return k210_bypass_get_rate(clk);

	ret = k210_bypass_dobypass(bypass);
	if (ret)
		return ret;

	ret = ops->set_rate(bypass->bypassee, rate);
	if (ret < 0)
		return ret;

	return k210_bypass_unbypass(bypass);
}

static int k210_bypass_set_parent(struct clk *clk, struct clk *parent)
{
	struct k210_bypass *bypass = to_k210_bypass(clk);
	const struct clk_ops *ops = bypass->bypassee_ops;

	if (ops->set_parent)
		return ops->set_parent(bypass->bypassee, parent);
	else
		return -ENOTSUPP;
}

/*
 * For these next two functions, do the bypassing even if there is no
 * en-/-disable function, since the bypassing itself can be observed in between
 * calls.
 */
static int k210_bypass_enable(struct clk *clk)
{
	int ret;
	struct k210_bypass *bypass = to_k210_bypass(clk);
	const struct clk_ops *ops = bypass->bypassee_ops;

	ret = k210_bypass_dobypass(bypass);
	if (ret)
		return ret;

	if (ops->enable)
		ret = ops->enable(bypass->bypassee);
	else
		ret = 0;
	if (ret)
		return ret;

	return k210_bypass_unbypass(bypass);
}

static int k210_bypass_disable(struct clk *clk)
{
	int ret;
	struct k210_bypass *bypass = to_k210_bypass(clk);
	const struct clk_ops *ops = bypass->bypassee_ops;

	ret = k210_bypass_dobypass(bypass);
	if (ret)
		return ret;

	if (ops->disable)
		return ops->disable(bypass->bypassee);
	else
		return 0;
}

static const struct clk_ops k210_bypass_ops = {
	.get_rate = k210_bypass_get_rate,
	.set_rate = k210_bypass_set_rate,
	.set_parent = k210_bypass_set_parent,
	.enable = k210_bypass_enable,
	.disable = k210_bypass_disable,
};

int k210_bypass_set_children(struct clk *clk, struct clk **children,
			     size_t child_count)
{
	struct k210_bypass *bypass = to_k210_bypass(clk);

	kfree(bypass->saved_parents);
	if (child_count) {
		bypass->saved_parents =
			kcalloc(child_count, sizeof(struct clk *), GFP_KERNEL);
		if (!bypass->saved_parents)
			return -ENOMEM;
	}
	bypass->child_count = child_count;
	bypass->children = children;

	return 0;
}

struct clk *k210_register_bypass_struct(const char *name,
					const char *parent_name,
					struct k210_bypass *bypass)
{
	int ret;
	struct clk *clk;

	clk = &bypass->clk;

	ret = clk_register(clk, CLK_K210_BYPASS, name, parent_name);
	if (ret)
		return ERR_PTR(ret);

	bypass->bypassee->dev = clk->dev;
	return clk;
}

struct clk *k210_register_bypass(const char *name, const char *parent_name,
				 struct clk *bypassee,
				 const struct clk_ops *bypassee_ops,
				 struct clk *alt)
{
	struct clk *clk;
	struct k210_bypass *bypass;

	bypass = kzalloc(sizeof(*bypass), GFP_KERNEL);
	if (!bypass)
		return ERR_PTR(-ENOMEM);

	bypass->bypassee = bypassee;
	bypass->bypassee_ops = bypassee_ops;
	bypass->alt = alt;

	clk = k210_register_bypass_struct(name, parent_name, bypass);
	if (IS_ERR(clk))
		kfree(bypass);
	return clk;
}

U_BOOT_DRIVER(k210_bypass) = {
	.name	= CLK_K210_BYPASS,
	.id	= UCLASS_CLK,
	.ops	= &k210_bypass_ops,
};

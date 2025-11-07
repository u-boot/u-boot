// SPDX-License-Identifier: GPL-2.0+
/*
 * Author: Mikhail Kshevetskiy <mikhail.kshevetskiy@iopsys.eu>
 */

#include <dm/ofnode.h>
#include <linux/err.h>
#include <asm/arch/scu-regmap.h>

static struct regmap *airoha_scu_node_regmap_by_index(unsigned int index)
{
	struct regmap *map;
	ofnode node;
	int err;

	node = ofnode_by_compatible(ofnode_null(), "airoha,en7523-scu");
	if (!ofnode_valid(node))
		return ERR_PTR(-EINVAL);

	/* CHIP_SCU (index=0), SCU (index=1) */
	err = regmap_init_mem_index(node, &map, index);
	if (err)
		return ERR_PTR(err);

	return map;
}

struct regmap *airoha_get_scu_regmap(void)
{
	/* CHIP_SCU (index=0), SCU (index=1) */
	return airoha_scu_node_regmap_by_index(1);
}

struct regmap *airoha_get_chip_scu_regmap(void)
{
	/* CHIP_SCU (index=0), SCU (index=1) */
	return airoha_scu_node_regmap_by_index(0);
}

// SPDX-License-Identifier: GPL-2.0+
/*
 * Author: Mikhail Kshevetskiy <mikhail.kshevetskiy@iopsys.eu>
 */

#include <syscon.h>
#include <linux/err.h>
#include <asm/arch/scu-regmap.h>

struct regmap *airoha_get_scu_regmap(void)
{
	ofnode node;

	node = ofnode_by_compatible(ofnode_null(), "airoha,en7581-scu");
	if (!ofnode_valid(node))
		return ERR_PTR(-EINVAL);

	return syscon_node_to_regmap(node);
}

struct regmap *airoha_get_chip_scu_regmap(void)
{
	ofnode node;

	node = ofnode_by_compatible(ofnode_null(), "airoha,en7581-chip-scu");
	if (!ofnode_valid(node))
		return ERR_PTR(-EINVAL);

	return syscon_node_to_regmap(node);
}

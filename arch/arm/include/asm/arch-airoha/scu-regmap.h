/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Author: Mikhail Kshevetskiy <mikhail.kshevetskiy@iopsys.eu>
 */
#ifndef __AIROHA_SCU_REGMAP__
#define __AIROHA_SCU_REGMAP__

#include <regmap.h>

struct regmap *airoha_get_scu_regmap(void);
struct regmap *airoha_get_chip_scu_regmap(void);

#endif

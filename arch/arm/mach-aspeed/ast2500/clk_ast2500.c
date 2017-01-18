/*
 * Copyright (C) 2016 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <asm/arch/scu_ast2500.h>

int ast_get_clk(struct udevice **devp)
{
	return uclass_get_device_by_driver(UCLASS_CLK,
			DM_GET_DRIVER(aspeed_ast2500_scu), devp);
}

void *ast_get_scu(void)
{
	struct ast2500_clk_priv *priv;
	struct udevice *dev;
	int ret;

	ret = ast_get_clk(&dev);
	if (ret)
		return ERR_PTR(ret);

	priv = dev_get_priv(dev);

	return priv->scu;
}

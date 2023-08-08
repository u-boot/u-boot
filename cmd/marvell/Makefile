# Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifdef CONFIG_ARCH_OCTEONTX2
obj-$(CONFIG_CMD_BOOTIMGUP) += bootimgup.o
obj-$(CONFIG_CMD_FAILSAFE) += octeontx2_failsafe_cmds.o
obj-$(CONFIG_CMD_ETH) += eth_cmds.o
obj-$(CONFIG_CMD_MDIO_DBG) += mdio_dbg.o
obj-$(CONFIG_OCTEONTX2_CGX_INTF) += octeontx2_qlm_cmds.o
obj-$(CONFIG_CMD_ATTEST) += octeontx2_attest_cmds.o
endif
ifdef CONFIG_ARCH_OCTEONTX
obj-$(CONFIG_CMD_BOOTIMGUP) += bootimgup.o
endif

# SPDX-License-Identifier: GPL-2.0+

dtb-$(CONFIG_TARGET_AP121) += ap121.dtb
dtb-$(CONFIG_TARGET_AP143) += ap143.dtb
dtb-$(CONFIG_TARGET_BOSTON) += img,boston.dtb
dtb-$(CONFIG_TARGET_MALTA) += mti,malta.dtb
dtb-$(CONFIG_TARGET_PIC32MZDASK) += pic32mzda_sk.dtb
dtb-$(CONFIG_TARGET_XILFPGA) += nexys4ddr.dtb
dtb-$(CONFIG_BOARD_COMTREND_AR5315U) += comtrend,ar-5315u.dtb
dtb-$(CONFIG_BOARD_COMTREND_AR5387UN) += comtrend,ar-5387un.dtb
dtb-$(CONFIG_BOARD_COMTREND_CT5361) += comtrend,ct-5361.dtb
dtb-$(CONFIG_BOARD_COMTREND_VR3032U) += comtrend,vr-3032u.dtb
dtb-$(CONFIG_BOARD_COMTREND_WAP5813N) += comtrend,wap-5813n.dtb
dtb-$(CONFIG_BOARD_HUAWEI_HG556A) += huawei,hg556a.dtb
dtb-$(CONFIG_BOARD_NETGEAR_CG3100D) += netgear,cg3100d.dtb
dtb-$(CONFIG_BOARD_NETGEAR_DGND3700V2) += netgear,dgnd3700v2.dtb
dtb-$(CONFIG_BOARD_SAGEM_FAST1704) += sagem,f@st1704.dtb
dtb-$(CONFIG_BOARD_TPLINK_WDR4300) += tplink_wdr4300.dtb
dtb-$(CONFIG_TARGET_JZ4780_CI20) += ci20.dtb

targets += $(dtb-y)

# Add any required device tree compiler flags here
DTC_FLAGS +=

PHONY += dtbs
dtbs: $(addprefix $(obj)/, $(dtb-y))
	@:

clean-files := *.dtb

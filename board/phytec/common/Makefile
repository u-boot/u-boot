# SPDX-License-Identifier: GPL-2.0+
# Copyright (C) 2023 PHYTEC Messtechnik GmbH
# Author: Teresa Remmet <t.remmet@phytec.de>

ifdef CONFIG_XPL_BUILD
# necessary to create built-in.a
obj- := __dummy__.a
endif

obj-y += phytec_som_detection.o phytec_som_detection_blocks.o
obj-$(CONFIG_ARCH_K3) += am6_som_detection.o k3/
obj-$(CONFIG_ARCH_IMX8M) += imx8m_som_detection.o
obj-$(CONFIG_ARCH_IMX9) += imx93_som_detection.o

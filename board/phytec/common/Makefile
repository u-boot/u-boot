# SPDX-License-Identifier: GPL-2.0+
# Copyright (C) 2023 PHYTEC Messtechnik GmbH
# Author: Teresa Remmet <t.remmet@phytec.de>

ifdef CONFIG_SPL_BUILD
# necessary to create built-in.o
obj- := __dummy__.o
endif

obj-$(CONFIG_PHYTEC_SOM_DETECTION) += phytec_som_detection.o
obj-$(CONFIG_PHYTEC_IMX8M_SOM_DETECTION) += imx8m_som_detection.o

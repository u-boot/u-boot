#
# Copyright (C) 2024 Marek Vasut <marek.vasut+renesas@mailbox.org>
#
# SPDX-License-Identifier: GPL-2.0+
#

# R-Car SoCs
ifndef CONFIG_RZA1
ifndef CONFIG_RZG2L

# 32 bit SoCs
ifdef CONFIG_RCAR_32
ifdef CONFIG_RCAR_GEN2
endif
endif

# 64 bit SoCs
ifdef CONFIG_RCAR_64
ifdef CONFIG_XPL_BUILD
obj-y	+= rcar64-spl.o
else
obj-y	+= rcar64-common.o
endif

ifdef CONFIG_RCAR_GEN3
ifdef CONFIG_XPL_BUILD
obj-y	+= gen3-spl.o
else
obj-y	+= gen3-common.o
ifdef CONFIG_R8A77970
obj-y	+= v3-common.o
endif
ifdef CONFIG_R8A77980
obj-y	+= v3-common.o
endif
endif
endif

ifdef CONFIG_RCAR_GEN4
ifdef CONFIG_XPL_BUILD
obj-y	+= gen4-spl.o
else
obj-y	+= gen4-common.o
endif
endif
endif

endif
endif

# RZ/A1 SoCs
ifdef CONFIG_RZA1
obj-y	+= rza1-common.o
obj-y	+= rza1-lowlevel_init.o
endif

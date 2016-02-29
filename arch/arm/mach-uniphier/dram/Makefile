#
# SPDX-License-Identifier:	GPL-2.0+
#

ifdef CONFIG_SPL_BUILD

obj-$(CONFIG_ARCH_UNIPHIER_PH1_LD4)	+= umc-ph1-ld4.o \
					   ddrphy-training.o ddrphy-ph1-ld4.o
obj-$(CONFIG_ARCH_UNIPHIER_PH1_PRO4)	+= umc-ph1-pro4.o \
					   ddrphy-training.o ddrphy-ph1-ld4.o
obj-$(CONFIG_ARCH_UNIPHIER_PH1_SLD8)	+= umc-ph1-sld8.o \
					   ddrphy-training.o ddrphy-ph1-ld4.o
obj-$(CONFIG_ARCH_UNIPHIER_PROXSTREAM2)	+= umc-proxstream2.o
obj-$(CONFIG_ARCH_UNIPHIER_PH1_LD6B)	+= umc-proxstream2.o

else

obj-$(CONFIG_CMD_DDRPHY_DUMP) += cmd_ddrphy.o
obj-$(CONFIG_CMD_DDRMPHY_DUMP) += cmd_ddrmphy.o

endif

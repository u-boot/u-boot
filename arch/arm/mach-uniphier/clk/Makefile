#
# SPDX-License-Identifier:	GPL-2.0+
#

ifdef CONFIG_SPL_BUILD

obj-$(CONFIG_ARCH_UNIPHIER_SLD3)	+= early-clk-ld4.o dpll-sld3.o
obj-$(CONFIG_ARCH_UNIPHIER_LD4)		+= early-clk-ld4.o dpll-ld4.o
obj-$(CONFIG_ARCH_UNIPHIER_PRO4)	+= early-clk-ld4.o dpll-pro4.o
obj-$(CONFIG_ARCH_UNIPHIER_SLD8)	+= early-clk-ld4.o dpll-sld8.o
obj-$(CONFIG_ARCH_UNIPHIER_PRO5)	+= early-clk-pro5.o
obj-$(CONFIG_ARCH_UNIPHIER_PXS2)	+= early-clk-pxs2.o
obj-$(CONFIG_ARCH_UNIPHIER_LD6B)	+= early-clk-pxs2.o
obj-$(CONFIG_ARCH_UNIPHIER_LD11)	+= early-clk-ld11.o dpll-ld11.o
obj-$(CONFIG_ARCH_UNIPHIER_LD20)	+= early-clk-ld20.o dpll-ld20.o

else

obj-$(CONFIG_ARCH_UNIPHIER_SLD3)	+= clk-ld4.o pll-sld3.o dpll-tail.o
obj-$(CONFIG_ARCH_UNIPHIER_LD4)		+= clk-ld4.o pll-ld4.o dpll-tail.o
obj-$(CONFIG_ARCH_UNIPHIER_PRO4)	+= clk-pro4.o pll-pro4.o dpll-tail.o
obj-$(CONFIG_ARCH_UNIPHIER_SLD8)	+= clk-ld4.o pll-ld4.o dpll-tail.o
obj-$(CONFIG_ARCH_UNIPHIER_PRO5)	+= clk-pro5.o
obj-$(CONFIG_ARCH_UNIPHIER_PXS2)	+= clk-pxs2.o
obj-$(CONFIG_ARCH_UNIPHIER_LD6B)	+= clk-pxs2.o
obj-$(CONFIG_ARCH_UNIPHIER_LD11)	+= clk-ld11.o pll-ld11.o
obj-$(CONFIG_ARCH_UNIPHIER_LD20)	+= clk-ld20.o pll-ld20.o

endif

obj-$(CONFIG_ARCH_UNIPHIER_LD11)	+= pll-base-ld20.o
obj-$(CONFIG_ARCH_UNIPHIER_LD20)	+= pll-base-ld20.o

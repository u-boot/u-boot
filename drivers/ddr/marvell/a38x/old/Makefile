#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-$(CONFIG_SPL_BUILD) += ddr3_a38x.o
obj-$(CONFIG_SPL_BUILD) += ddr3_a38x_training.o
obj-$(CONFIG_SPL_BUILD) += ddr3_debug.o
obj-$(CONFIG_SPL_BUILD) += ddr3_hws_hw_training.o
obj-$(CONFIG_SPL_BUILD) += ddr3_init.o
obj-$(CONFIG_SPL_BUILD) += ddr3_training.o
obj-$(CONFIG_SPL_BUILD) += ddr3_training_bist.o
obj-$(CONFIG_SPL_BUILD) += ddr3_training_centralization.o
obj-$(CONFIG_SPL_BUILD) += ddr3_training_db.o
obj-$(CONFIG_SPL_BUILD) += ddr3_training_hw_algo.o
obj-$(CONFIG_SPL_BUILD) += ddr3_training_ip_engine.o
obj-$(CONFIG_SPL_BUILD) += ddr3_training_leveling.o
obj-$(CONFIG_SPL_BUILD) += ddr3_training_pbs.o
obj-$(CONFIG_SPL_BUILD) += ddr3_training_static.o

define IncludeSymbolRename
  CFLAGS_$(1) = -include $(srctree)/drivers/ddr/marvell/a38x/old/glue_symbol_renames.h
endef

$(foreach obj,$(obj-y),$(eval $(call IncludeSymbolRename,$(obj))))

# The old version of DDR training fails weirdly on some boards if the whole
# driver is compiled with LTO. It seems to work if at least ddr3_init.c is
# compiled without LTO.
CFLAGS_REMOVE_ddr3_init.o := $(LTO_CFLAGS)

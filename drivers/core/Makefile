# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2013 Google, Inc

obj-y	+= device.o fdtaddr.o lists.o root.o uclass.o util.o tag.o
obj-$(CONFIG_$(PHASE_)ACPIGEN) += acpi.o
obj-$(CONFIG_$(PHASE_)DEVRES) += devres.o
obj-$(CONFIG_$(PHASE_)DM_DEVICE_REMOVE)	+= device-remove.o
obj-$(CONFIG_$(PHASE_)SIMPLE_BUS)	+= simple-bus.o
obj-$(CONFIG_SIMPLE_PM_BUS)	+= simple-pm-bus.o
obj-$(CONFIG_DM)	+= dump.o
obj-$(CONFIG_$(PHASE_)REGMAP)	+= regmap.o
obj-$(CONFIG_$(PHASE_)SYSCON)	+= syscon-uclass.o
obj-$(CONFIG_$(PHASE_)OF_LIVE) += of_access.o of_addr.o
ifndef CONFIG_DM_DEV_READ_INLINE
obj-$(CONFIG_OF_CONTROL) += read.o
endif
obj-$(CONFIG_$(PHASE_)OF_PLATDATA) += read.o
obj-$(CONFIG_OF_CONTROL) += of_extra.o ofnode.o read_extra.o ofnode_graph.o

ccflags-$(CONFIG_DM_DEBUG) += -DDEBUG

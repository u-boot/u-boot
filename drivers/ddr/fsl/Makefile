#
# Copyright 2008-2011 Freescale Semiconductor, Inc.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# Version 2 as published by the Free Software Foundation.
#

obj-$(CONFIG_SYS_FSL_DDR1)	+= main.o util.o ctrl_regs.o options.o \
				   lc_common_dimm_params.o

obj-$(CONFIG_SYS_FSL_DDR2)	+= main.o util.o ctrl_regs.o options.o \
				   lc_common_dimm_params.o

obj-$(CONFIG_SYS_FSL_DDR3)	+= main.o util.o ctrl_regs.o options.o \
				   lc_common_dimm_params.o
ifdef CONFIG_DDR_SPD
SPD := y
endif
ifdef CONFIG_SPD_EEPROM
SPD := y
endif
ifdef SPD
obj-$(CONFIG_SYS_FSL_DDR1)	+= ddr1_dimm_params.o
obj-$(CONFIG_SYS_FSL_DDR2)	+= ddr2_dimm_params.o
obj-$(CONFIG_SYS_FSL_DDR3)	+= ddr3_dimm_params.o
endif

obj-$(CONFIG_FSL_DDR_INTERACTIVE)	+= interactive.o
obj-$(CONFIG_SYS_FSL_DDRC_GEN1)	+= mpc85xx_ddr_gen1.o
obj-$(CONFIG_SYS_FSL_DDRC_GEN2)	+= mpc85xx_ddr_gen2.o
obj-$(CONFIG_SYS_FSL_DDRC_GEN3)	+= mpc85xx_ddr_gen3.o
obj-$(CONFIG_SYS_FSL_DDR_86XX)		+= mpc86xx_ddr.o
obj-$(CONFIG_SYS_FSL_DDRC_ARM_GEN3)	+= arm_ddr_gen3.o

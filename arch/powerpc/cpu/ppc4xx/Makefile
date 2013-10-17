#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(CPU).o

START	:= resetvec.o
START	+= start.o

SOBJS	:= cache.o
SOBJS	+= dcr.o
SOBJS	+= kgdb.o

COBJS	:= 40x_spd_sdram.o

ifndef CONFIG_NAND_SPL
ifndef CONFIG_NAND_U_BOOT
COBJS	+= 44x_spd_ddr.o
endif
endif
COBJS-$(CONFIG_SDRAM_PPC4xx_IBM_DDR2) += 44x_spd_ddr2.o
COBJS-$(CONFIG_PPC4xx_DDR_AUTOCALIBRATION) += 4xx_ibm_ddr2_autocalib.o
COBJS	+= 4xx_pci.o
COBJS	+= 4xx_pcie.o
COBJS	+= bedbug_405.o
COBJS-$(CONFIG_CMD_CHIP_CONFIG)	+= cmd_chip_config.o
COBJS	+= cpu.o
COBJS	+= cpu_init.o
COBJS	+= denali_data_eye.o
COBJS	+= denali_spd_ddr2.o
COBJS	+= ecc.o
COBJS-$(CONFIG_CMD_ECCTEST) += cmd_ecctest.o
COBJS	+= fdt.o
COBJS	+= interrupts.o
COBJS-$(CONFIG_CMD_REGINFO) += reginfo.o
COBJS	+= sdram.o
COBJS	+= speed.o
COBJS	+= tlb.o
COBJS	+= traps.o
COBJS	+= usb.o
COBJS	+= usb_ohci.o
COBJS-$(CONFIG_XILINX_440) += xilinx_irq.o
ifndef CONFIG_XILINX_440
COBJS	+= 4xx_uart.o
COBJS	+= gpio.o
COBJS	+= miiphy.o
COBJS	+= uic.o
endif

ifdef CONFIG_SPL_BUILD
COBJS-y += spl_boot.o
endif

SRCS	:= $(START:.o=.S) $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS) $(COBJS-y))
START	:= $(addprefix $(obj),$(START))

all:	$(obj).depend $(START) $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################

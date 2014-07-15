#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

extra-y	:= resetvec.o
extra-y	+= start.o

obj-y	:= cache.o
obj-y	+= dcr.o
obj-y	+= kgdb.o

obj-y	+= 40x_spd_sdram.o

obj-y	+= 44x_spd_ddr.o
obj-$(CONFIG_SDRAM_PPC4xx_IBM_DDR2) += 44x_spd_ddr2.o
obj-$(CONFIG_PPC4xx_DDR_AUTOCALIBRATION) += 4xx_ibm_ddr2_autocalib.o
obj-y	+= 4xx_pci.o
obj-y	+= 4xx_pcie.o
obj-y	+= bedbug_405.o
obj-$(CONFIG_CMD_CHIP_CONFIG)	+= cmd_chip_config.o
obj-y	+= cpu.o
obj-y	+= cpu_init.o
obj-y	+= denali_data_eye.o
obj-y	+= denali_spd_ddr2.o
obj-y	+= ecc.o
obj-$(CONFIG_CMD_ECCTEST) += cmd_ecctest.o
obj-y	+= fdt.o
obj-y	+= interrupts.o
obj-$(CONFIG_CMD_REGINFO) += reginfo.o
obj-y	+= sdram.o
obj-y	+= speed.o
obj-y	+= tlb.o
obj-y	+= traps.o
obj-y	+= usb.o
obj-y	+= usb_ohci.o
obj-$(CONFIG_XILINX_440) += xilinx_irq.o
ifndef CONFIG_XILINX_440
obj-y	+= 4xx_uart.o
obj-y	+= gpio.o
obj-y	+= miiphy.o
obj-y	+= uic.o
endif

ifdef CONFIG_SPL_BUILD
obj-y += spl_boot.o
endif

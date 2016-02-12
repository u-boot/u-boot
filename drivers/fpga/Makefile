#
# (C) Copyright 2008
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y += fpga.o
obj-$(CONFIG_FPGA_SPARTAN2) += spartan2.o
obj-$(CONFIG_FPGA_SPARTAN3) += spartan3.o
obj-$(CONFIG_FPGA_VIRTEX2) += virtex2.o
obj-$(CONFIG_FPGA_ZYNQPL) += zynqpl.o
obj-$(CONFIG_FPGA_XILINX) += xilinx.o
obj-$(CONFIG_FPGA_LATTICE) += ivm_core.o lattice.o
ifdef CONFIG_FPGA_ALTERA
obj-y += altera.o
obj-$(CONFIG_FPGA_ACEX1K) += ACEX1K.o
obj-$(CONFIG_FPGA_CYCLON2) += cyclon2.o
obj-$(CONFIG_FPGA_STRATIX_II) += stratixII.o
obj-$(CONFIG_FPGA_STRATIX_V) += stratixv.o
obj-$(CONFIG_FPGA_SOCFPGA) += socfpga.o
endif

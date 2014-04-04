#
# Copyright (c) 2005-2008 Analog Device Inc.
#
# (C) Copyright 2001
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

# Set some default LDR flags based on boot mode.
LDR_FLAGS-BFIN_BOOT_PARA       := --dma 6
LDR_FLAGS-BFIN_BOOT_FIFO       := --dma 1
LDR_FLAGS-BFIN_BOOT_SPI_MASTER := --dma 1
LDR_FLAGS-BFIN_BOOT_UART       := --dma 1
LDR_FLAGS-BFIN_BOOT_NAND       := --dma 6

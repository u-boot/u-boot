#
# Makefile
#
# (C) Copyright 2016 CompuLab, Ltd. <www.compulab.co.il>
#
# Author: Dmitry Lifshitz <lifshitz@compulab.co.il>
#
# SPDX-License-Identifier:     GPL-2.0+
#

ifdef CONFIG_SPL_BUILD
obj-y	+= spl.o mux.o
else
obj-y	+= cl-som-am57x.o mux.o
endif

obj-$(CONFIG_DRIVER_TI_CPSW)	+= eth.o

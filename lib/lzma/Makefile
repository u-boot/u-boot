# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2007-2008 Industrie Dial Face S.p.A.
# Luigi 'Comio' Mantellini (luigi.mantellini@idf-hit.com)
#
# (C) Copyright 2003-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

ccflags-y += -DZ7_LZMA_PROB32
ccflags-$(CONFIG_$(PHASE_)LZMA_SIZE_OPTIMIZATION) += -DZ7_LZMA_SIZE_OPT

obj-y += LzmaDec.o LzmaTools.o

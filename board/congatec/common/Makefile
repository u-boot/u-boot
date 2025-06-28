#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

MINIMAL=

ifdef CONFIG_XPL_BUILD
ifndef CONFIG_TPL_BUILD
ifdef CONFIG_SPL_INIT_MINIMAL
MINIMAL=y
endif
endif
endif

ifdef MINIMAL
# necessary to create built-in.a
obj- := __dummy__.a
else

obj-y				+= mmc.o

endif

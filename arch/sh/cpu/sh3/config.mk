#
# (C) Copyright 2000-2004
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2007
# Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
#
# (C) Copyright 2007
# Yoshihiro Shimoda <shimoda.yoshihiro@renesas.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#
#
PLATFORM_CPPFLAGS += -DCONFIG_SH3 -m3
PLATFORM_RELFLAGS += -ffixed-r13

# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2025, Kory Maincent <kory.maincent@bootlin.com>

mkfwumdata-objs := fwumdata_src/mkfwumdata.o generated/lib/crc32.o
HOSTLDLIBS_mkfwumdata += -luuid
hostprogs-always-$(CONFIG_TOOLS_MKFWUMDATA) += mkfwumdata

fwumdata-objs := fwumdata_src/fwumdata.o generated/lib/crc32.o
hostprogs-always-$(CONFIG_TOOLS_FWUMDATA) += fwumdata

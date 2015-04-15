#
# Copyright 2014 Freescale Semiconductor, Inc.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# Version 2 as published by the Free Software Foundation.
#

obj-y += sec.o
obj-$(CONFIG_FSL_CAAM) += jr.o fsl_hash.o jobdesc.o error.o
obj-$(CONFIG_CMD_BLOB)$(CONFIG_CMD_DEKBLOB) += fsl_blob.o
obj-$(CONFIG_RSA_FREESCALE_EXP) += fsl_rsa.o

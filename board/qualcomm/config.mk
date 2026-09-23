# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright Linaro Ltd.
#
# Qualcomm specific make target for MBN signed ELF files.
#

# Create Qualcomm signed elf images
CMD_MKMBN = $(srctree)/tools/qcom/mkmbn/mkmbn.py
quiet_cmd_mkmbn = MBN     $@
      cmd_mkmbn = $(CMD_MKMBN) $<

u-boot.mbn: u-boot.bin FORCE
	$(call if_changed,mkmbn)

# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright Linaro Ltd.
#
# Handle xbl_sec.elf
#

xbl_sec.elf: prepare FORCE
	$(Q)if ! test -f $(abs_objtree)/$@; then \
		echo "WARNING: xbl_sec.elf not provided, image will be non-functional"; \
	fi; \
	touch $(abs_objtree)/$@;

INPUTS-y += xbl_sec.elf

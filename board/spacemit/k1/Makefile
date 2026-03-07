# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (c) 2024, Kongyang Liu <seashell11234455@gmail.com>
# Copyright (c) 2025-2026, RISCStar Ltd.

obj-y := board.o
obj-$(CONFIG_SPL_BUILD) += spl.o

DDR_FW_SRC ?= $(DDR_FW_FILE)
FW_TARGET = $(srctree)/lib/vendor/spacemit/ddr_fw.bin

DDR_FW_HEADER = $(objtree)/include/generated/ddr_fw_info.h

$(obj)/spl.o: $(DDR_FW_HEADER)

$(DDR_FW_HEADER): $(FW_TARGET)
	@echo "/* DDR firmware info - $$(date) */" > $@
	@if [ -f "$(FW_TARGET)" ]; then \
		SIZE=$$(stat -c%s "$(FW_TARGET)" 2>/dev/null || echo 0); \
	else \
		SIZE=0; \
	fi; \
	echo "#define DDR_FW_FILE_SIZE	$$SIZE" >> $@
	@echo "/* Note: Update ADDR if binman layout changes */" >> $@

clean-files += $(FW_TARGET) $(DDR_FW_HEADER)

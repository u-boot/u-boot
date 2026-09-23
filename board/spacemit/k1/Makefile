# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (c) 2024, Kongyang Liu <seashell11234455@gmail.com>
# Copyright (c) 2025-2026, RISCstar Ltd.

obj-y := board.o
obj-$(CONFIG_SPL_BUILD) += spl.o

DDR_FW_SRC ?= $(DDR_FW_FILE)
FW_TARGET = $(objtree)/ddr_fw.bin

$(obj)/spl.o: $(FW_TARGET)

$(FW_TARGET): FORCE
	@if [ -n "$(DDR_FW_SRC)" ] && [ -f "$(DDR_FW_SRC)" ]; then \
		if ! cmp -s "$(DDR_FW_SRC)" $@ 2>/dev/null; then \
			echo "  Copying DDR firmware from: $(DDR_FW_SRC)"; \
			cp "$(DDR_FW_SRC)" $@; \
		fi; \
	elif [ -s $@ ]; then \
		: ; \
	else \
		echo "  Note: No DDR firmware found, creating empty placeholder"; \
		echo "  (Set DDR_FW_FILE to specify firmware location)"; \
		touch $@; \
	fi

clean-files += $(FW_TARGET)

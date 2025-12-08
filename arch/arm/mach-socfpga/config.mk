# SPDX-License-Identifier: GPL-2.0+
#
# Brian Sune <briansune@gmail.com>

ifeq ($(CONFIG_TARGET_SOCFPGA_CYCLONE5),y)
archprepare: socfpga_g5_handoff_prepare
else ifeq ($(CONFIG_TARGET_SOCFPGA_ARRIA5),y)
archprepare: socfpga_g5_handoff_prepare
endif

socfpga_g5_handoff_prepare:
	@SOCFAMILY="$(SOCFAMILY)"; \
		if [ -z "$$SOCFAMILY" ]; then \
			exit 0; \
		fi; \
		echo "[INFO] SOC family detected: $$SOCFAMILY";
		@set -- $$(awk -F'"' ' \
		    /^CONFIG_SYS_VENDOR=/ {v=$$2} \
		    /^CONFIG_SYS_BOARD=/ {b=$$2} \
		    END {print v, b}' .config); \
		VENDOR=$$1; \
		BOARD=$$2; \
		if [ -z "$$VENDOR" ] || [ -z "$$BOARD" ]; then \
			exit 0; \
		fi; \
		BOARD_DIR=$(src)/board/$$VENDOR/$$BOARD; \
		if [ "$$HANDOFF_PATH" ]; then \
			echo "[INFO] Using manually specified handoff folder: $$HANDOFF_PATH"; \
		else \
			HANDOFF_BASE=$$BOARD_DIR/hps_isw_handoff; \
			if [ ! -d "$$HANDOFF_BASE" ]; then \
				exit 0; \
			fi; \
			HANDOFF_PATH=$$(ls -d "$$HANDOFF_BASE"/*/ 2>/dev/null | head -n1); \
			if [ -z "$$HANDOFF_PATH" ]; then \
				exit 0; \
			fi; \
			echo "[INFO] Auto-detected handoff folder: $$HANDOFF_PATH"; \
		fi; \
		HIOF_FILE=$$HANDOFF_PATH/$$(basename $$HANDOFF_PATH).hiof; \
		if [ ! -f "$$HIOF_FILE" ]; then \
			echo "[WARN] No .hiof file found in $$HANDOFF_PATH, skipping BSP generation."; \
			exit 0; \
		fi; \
		echo "[INFO] Found hiof file: $$HIOF_FILE"; \
		echo "[INFO] Running BSP generator..."; \
		python3 $(src)/tools/cv_bsp_generator/cv_bsp_generator.py -i "$$HANDOFF_PATH" -o "$$BOARD_DIR/qts" || echo "[WARN] BSP generator failed, continuing..."; \
		echo "[DONE] SoCFPGA QTS handoff conversion complete."

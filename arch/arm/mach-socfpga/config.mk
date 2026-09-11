# SPDX-License-Identifier: GPL-2.0+
#
# Brian Sune <briansune@gmail.com>
#
# HANDOFF_PATH
# ------------
# Unset - Board path where qts locates and "hps_isw_handoff" should be placed.
# Set   - Custom path points to "hps_isw_handoff" folder.
#
# HANDOFF_KEEP
# ------------
# Unset	- Clean header files.
# Set	- HANDOFF_KEEP= , Clean header files.
#	  HANDOFF_KEEP=0, Clean header files.
#	  HANDOFF_KEEP=1, Backup header files and rename to ".h.handoff_backup.<timestamp>".

ifeq ($(CONFIG_ARCH_SOCFPGA_CYCLONE5),y)
archprepare: socfpga_g5_handoff_prepare
else ifeq ($(CONFIG_ARCH_SOCFPGA_ARRIA5),y)
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
		BOARD_DIR=$(srctree)/board/$$VENDOR/$$BOARD; \
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
		echo "[INFO] Try BSP generator..."; \
		TEMP_DIR=$$(mktemp -dp "$$BOARD_DIR/"); \
		trap 'rm -rf "$$TEMP_DIR"' EXIT; \
		if python3 $(srctree)/tools/cv_bsp_generator/cv_bsp_generator.py -i "$$HANDOFF_PATH" -o "$$TEMP_DIR"; then \
			if [ "$${HANDOFF_KEEP:-0}" != "0" ]; then \
				echo "[INFO] Preserving old BSP files..."; \
				TIMESTAMP=$$(date +%Y%m%d_%H%M%S); \
				for f in "$$BOARD_DIR"/qts/*.h; do \
					[ -e "$$f" ] || continue; \
					echo "[INFO] $$f -> $${f%.h}.h.handoff_backup.$$TIMESTAMP"; \
					mv "$$f" "$${f%.h}.h.handoff_backup.$$TIMESTAMP"; \
				done; \
			else \
				echo "[INFO] Clean old BSP files..."; \
				if ls "$$BOARD_DIR/qts"/*.h >/dev/null 2>&1; then \
					rm "$$BOARD_DIR/qts"/*.h; \
					echo "[INFO] Removed old BSP files..."; \
				fi; \
			fi; \
			mv "$$TEMP_DIR"/*.h "$$BOARD_DIR"/qts; \
			echo "[INFO] SoCFPGA QTS handoff conversion complete."; \
		else \
			echo "[WARN] BSP generator failed!"; \
		fi;

# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2016 Alexander Graf
#

# This file only gets included with CONFIG_EFI_LOADER set, so all
# object inclusion implicitly depends on it

asflags-y += -I.

CFLAGS_efi_boottime.o += \
  -DFW_VERSION="0x$(VERSION)" \
  -DFW_PATCHLEVEL="0x$(PATCHLEVEL)"

# These are the apps that are built
apps-$(CONFIG_RISCV) += boothart
apps-$(CONFIG_BOOTEFI_HELLO_COMPILE) += helloworld
apps-$(CONFIG_GENERATE_SMBIOS_TABLE) += smbiosdump
apps-$(CONFIG_EFI_LOAD_FILE2_INITRD) += initrddump
ifeq ($(CONFIG_GENERATE_ACPI_TABLE),)
apps-y += dtbdump
endif
apps-$(CONFIG_BOOTEFI_TESTAPP_COMPILE) += testapp
apps-y += dbginfodump

obj-$(CONFIG_CMD_BOOTEFI_HELLO) += helloworld_efi.o
obj-$(CONFIG_EFI_BOOTMGR) += efi_bootmgr.o
obj-$(CONFIG_EFI_BINARY_EXEC) += efi_bootbin.o
obj-y += efi_boottime.o
obj-y += efi_helper.o
obj-$(CONFIG_EFI_HAVE_CAPSULE_SUPPORT) += efi_capsule.o
obj-$(CONFIG_EFI_CAPSULE_FIRMWARE) += efi_firmware.o
obj-$(CONFIG_EFI_CAPSULE_AUTHENTICATE) += efi_capsule_key.o
obj-y += efi_console.o
obj-y += efi_device_path.o
obj-$(CONFIG_EFI_DEVICE_PATH_TO_TEXT) += efi_device_path_to_text.o
obj-$(CONFIG_EFI_DEVICE_PATH_UTIL) += efi_device_path_utilities.o
obj-y += efi_dt_fixup.o
obj-y += efi_fdt.o
obj-y += efi_file.o
obj-$(CONFIG_EFI_LOADER_HII) += efi_hii.o efi_hii_config.o
obj-y += efi_image_loader.o
obj-y += efi_load_options.o
obj-y += efi_memory.o
obj-y += efi_root_node.o
obj-y += efi_runtime.o
obj-y += efi_setup.o
obj-y += efi_string.o
obj-$(CONFIG_EFI_UNICODE_COLLATION_PROTOCOL2) += efi_unicode_collation.o
obj-y += efi_var_common.o
obj-y += efi_var_mem.o
ifeq ($(CONFIG_EFI_MM_COMM_TEE),y)
obj-y += efi_variable_tee.o
else
obj-y += efi_variable.o
obj-y += efi_var_file.o
obj-$(CONFIG_EFI_VARIABLES_PRESEED) += efi_var_seed.o
endif
obj-y += efi_watchdog.o
obj-$(CONFIG_EFI_ESRT) += efi_esrt.o
obj-$(CONFIG_VIDEO) += efi_gop.o
obj-$(CONFIG_BLK) += efi_disk.o
obj-$(CONFIG_NETDEVICES) += efi_net.o
obj-$(CONFIG_EFI_IP4_CONFIG2_PROTOCOL) += efi_ipconfig.o
obj-$(CONFIG_EFI_HTTP_PROTOCOL) += efi_http.o
obj-$(CONFIG_ACPI) += efi_acpi.o
obj-$(CONFIG_SMBIOS) += efi_smbios.o
obj-$(CONFIG_EFI_RNG_PROTOCOL) += efi_rng.o
obj-$(CONFIG_EFI_TCG2_PROTOCOL) += efi_tcg2.o
obj-$(CONFIG_EFI_RISCV_BOOT_PROTOCOL) += efi_riscv.o
obj-$(CONFIG_EFI_LOAD_FILE2_INITRD) += efi_load_initrd.o
obj-$(CONFIG_EFI_SIGNATURE_SUPPORT) += efi_signature.o
obj-$(CONFIG_EFI_ECPT) += efi_conformance.o
obj-$(CONFIG_EFI_DEBUG_SUPPORT) += efi_debug_support.o

EFI_VAR_SEED_FILE := $(subst $\",,$(CONFIG_EFI_VAR_SEED_FILE))
$(obj)/efi_var_seed.o: $(srctree)/$(EFI_VAR_SEED_FILE)

ifeq ($(CONFIG_EFI_CAPSULE_AUTHENTICATE),y)
capsule_crt_path=($(subst $(quote),,$(CONFIG_EFI_CAPSULE_CRT_FILE)))
capsule_crt_full=$(srctree)/$(subst $(quote),,$(CONFIG_EFI_CAPSULE_CRT_FILE))
quiet_cmd_capsule_esl_gen = CAPSULE_ESL_GEN $@
cmd_capsule_esl_gen = cert-to-efi-sig-list $(capsule_crt_full) $@
$(objtree)/capsule_esl_file: FORCE
	@if [ ! -e "$(capsule_crt_full)" ]; then \
		echo "ERROR: path $(capsule_crt_full) is invalid." >&2; \
		echo "EFI CONFIG_EFI_CAPSULE_CRT_FILE must be specified when CONFIG_EFI_CAPSULE_AUTHENTICATE is enabled." >&2; \
		exit 1; \
	fi
	$(call cmd,capsule_esl_gen)

$(obj)/efi_capsule.o: $(objtree)/capsule_esl_file FORCE
asflags-y += -DCAPSULE_ESL_PATH=\"$(objtree)/capsule_esl_file\"
endif

# Set the C flags to add and remove for each app
$(foreach f,$(apps-y),\
	$(eval CFLAGS_$(f).o := $(CFLAGS_EFI) -Os -ffreestanding)\
	$(eval CFLAGS_REMOVE_$(f).o := $(CFLAGS_NON_EFI)))

always += $(foreach f,$(apps-y),$(f).efi)
targets += $(foreach f,$(apps-y),$(f).o)

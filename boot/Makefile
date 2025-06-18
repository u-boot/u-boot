# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2004-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

ifndef CONFIG_XPL_BUILD

obj-$(CONFIG_BOOT_RETRY) += bootretry.o
obj-$(CONFIG_CMD_BOOTM) += bootm.o bootm_os.o
obj-$(CONFIG_CMD_BOOTZ) += bootm.o bootm_os.o
obj-$(CONFIG_CMD_BOOTI) += bootm.o bootm_os.o

obj-$(CONFIG_PXE_UTILS) += pxe_utils.o

endif

obj-y += image.o image-board.o

obj-$(CONFIG_ANDROID_AB) += android_ab.o
obj-$(CONFIG_ANDROID_BOOT_IMAGE) += image-android.o image-android-dt.o

obj-$(CONFIG_$(PHASE_)BOOTSTD) += bootdev-uclass.o
obj-$(CONFIG_$(PHASE_)BOOTSTD) += bootflow.o
obj-$(CONFIG_$(PHASE_)BOOTSTD) += bootmeth-uclass.o
obj-$(CONFIG_$(PHASE_)BOOTSTD) += bootstd-uclass.o

obj-$(CONFIG_$(PHASE_)BOOTSTD_MENU) += bootflow_menu.o
obj-$(CONFIG_$(PHASE_)BOOTSTD_PROG) += prog_boot.o

obj-$(CONFIG_$(PHASE_)BOOTMETH_EXTLINUX) += bootmeth_extlinux.o
obj-$(CONFIG_$(PHASE_)BOOTMETH_EXTLINUX_PXE) += bootmeth_pxe.o
obj-$(CONFIG_$(PHASE_)BOOTMETH_EFILOADER) += bootmeth_efi.o
obj-$(CONFIG_$(PHASE_)BOOTMETH_CROS) += bootm.o bootm_os.o bootmeth_cros.o
obj-$(CONFIG_$(PHASE_)BOOTMETH_QFW) += bootmeth_qfw.o
obj-$(CONFIG_$(PHASE_)BOOTMETH_RAUC) += bootmeth_rauc.o
obj-$(CONFIG_$(PHASE_)BOOTMETH_SANDBOX) += bootmeth_sandbox.o
obj-$(CONFIG_$(PHASE_)BOOTMETH_SCRIPT) += bootmeth_script.o
obj-$(CONFIG_$(PHASE_)CEDIT) += cedit.o
obj-$(CONFIG_$(PHASE_)BOOTMETH_EFI_BOOTMGR) += bootmeth_efi_mgr.o

obj-$(CONFIG_$(PHASE_)OF_LIBFDT) += fdt_support.o
obj-$(CONFIG_$(PHASE_)FDT_SIMPLEFB) += fdt_simplefb.o

obj-$(CONFIG_$(PHASE_)UPL) += upl_common.o
obj-$(CONFIG_$(PHASE_)UPL_READ) += upl_read.o
obj-$(CONFIG_$(PHASE_)UPL_WRITE) += upl_write.o

obj-$(CONFIG_$(PHASE_)OF_LIBFDT) += image-fdt.o
obj-$(CONFIG_$(PHASE_)FIT_SIGNATURE) += fdt_region.o
obj-$(CONFIG_$(PHASE_)FIT) += image-fit.o
obj-$(CONFIG_$(PHASE_)MULTI_DTB_FIT) += boot_fit.o common_fit.o
obj-$(CONFIG_$(PHASE_)IMAGE_PRE_LOAD) += image-pre-load.o
obj-$(CONFIG_$(PHASE_)IMAGE_SIGN_INFO) += image-sig.o
obj-$(CONFIG_$(PHASE_)FIT_SIGNATURE) += image-fit-sig.o
obj-$(CONFIG_$(PHASE_)FIT_CIPHER) += image-cipher.o

obj-$(CONFIG_CMD_ADTIMG) += image-android-dt.o

obj-$(CONFIG_$(PHASE_)LOAD_FIT) += common_fit.o

obj-$(CONFIG_$(PHASE_)EXPO) += expo.o scene.o expo_build.o
obj-$(CONFIG_$(PHASE_)EXPO) += scene_menu.o scene_textline.o scene_textedit.o
ifdef CONFIG_COREBOOT_SYSINFO
obj-$(CONFIG_$(PHASE_)EXPO) += expo_build_cb.o
endif

obj-$(CONFIG_$(PHASE_)BOOTMETH_VBE) += vbe.o
obj-$(CONFIG_$(PHASE_)BOOTMETH_VBE_REQUEST) += vbe_request.o
obj-$(CONFIG_$(PHASE_)BOOTMETH_VBE_SIMPLE) += vbe_simple.o vbe_common.o
obj-$(CONFIG_$(PHASE_)BOOTMETH_VBE_SIMPLE_FW) += vbe_simple_fw.o
obj-$(CONFIG_$(PHASE_)BOOTMETH_VBE_SIMPLE_OS) += vbe_simple_os.o

obj-$(CONFIG_$(PHASE_)BOOTMETH_ANDROID) += bootmeth_android.o

obj-$(CONFIG_$(PHASE_)BOOTMETH_VBE_ABREC) += vbe_abrec.o vbe_common.o
obj-$(CONFIG_$(PHASE_)BOOTMETH_VBE_ABREC_FW) += vbe_abrec_fw.o
obj-$(CONFIG_$(PHASE_)BOOTMETH_VBE_ABREC_OS) += vbe_abrec_os.o

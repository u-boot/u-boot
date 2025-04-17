# SPDX-License-Identifier: GPL-2.0+

obj-$(CONFIG_$(PHASE_)ADC) += adc/
obj-$(CONFIG_$(PHASE_)BIOSEMU) += bios_emulator/
obj-$(CONFIG_$(PHASE_)BLK) += block/
obj-$(CONFIG_$(PHASE_)BOOTCOUNT_LIMIT) += bootcount/
obj-$(CONFIG_$(PHASE_)BUTTON) += button/
obj-$(CONFIG_$(PHASE_)CACHE) += cache/
obj-$(CONFIG_$(PHASE_)CLK) += clk/
obj-$(CONFIG_$(PHASE_)DM) += core/
obj-$(CONFIG_$(PHASE_)DMA) += dma/
obj-$(CONFIG_$(PHASE_)DMA_LEGACY) += dma/
obj-$(CONFIG_$(PHASE_)DFU) += dfu/
obj-$(CONFIG_$(PHASE_)EXTCON) += extcon/
obj-$(CONFIG_$(PHASE_)GPIO) += gpio/
obj-$(CONFIG_$(PHASE_)DRIVERS_MISC) += misc/
obj-$(CONFIG_$(PHASE_)SYSRESET) += sysreset/
obj-$(CONFIG_$(PHASE_)FIRMWARE) +=firmware/
obj-$(CONFIG_$(PHASE_)I2C) += i2c/
obj-$(CONFIG_$(PHASE_)I3C) += i3c/
obj-$(CONFIG_$(PHASE_)INPUT) += input/
obj-$(CONFIG_$(PHASE_)LED) += led/
obj-$(CONFIG_$(PHASE_)MMC) += mmc/
obj-y += mtd/
obj-$(CONFIG_MULTIPLEXER) += mux/
obj-$(CONFIG_$(PHASE_)ETH) += net/
obj-$(CONFIG_$(PHASE_)PCH) += pch/
obj-$(CONFIG_$(PHASE_)PCI) += pci/
obj-$(CONFIG_$(PHASE_)PHY) += phy/
obj-$(CONFIG_$(PHASE_)PINCTRL) += pinctrl/
obj-$(CONFIG_$(PHASE_)POWER) += power/
obj-$(CONFIG_$(PHASE_)RAM) += ram/
obj-$(CONFIG_$(PHASE_)RTC) += rtc/
obj-$(CONFIG_$(PHASE_)SERIAL) += serial/
obj-$(CONFIG_$(PHASE_)SPI) += spi/
obj-$(CONFIG_$(PHASE_)TIMER) += timer/
obj-$(CONFIG_$(PHASE_)VIRTIO) += virtio/
obj-$(CONFIG_$(PHASE_)DM_MAILBOX) += mailbox/
obj-$(CONFIG_$(PHASE_)REMOTEPROC) += remoteproc/
obj-$(CONFIG_$(PHASE_)SYSINFO) += sysinfo/
obj-$(CONFIG_$(PHASE_)SM) += sm/
obj-$(CONFIG_$(PHASE_)TPM) += tpm/
obj-$(CONFIG_$(PHASE_)NVME) += nvme/
obj-$(CONFIG_XEN) += xen/
obj-$(CONFIG_$(PHASE_)FPGA) += fpga/
obj-$(CONFIG_$(PHASE_)VIDEO) += video/

obj-y += bus/

ifndef CONFIG_TPL_BUILD
ifndef CONFIG_VPL_BUILD
ifdef CONFIG_XPL_BUILD

obj-$(CONFIG_SPL_CPU) += cpu/
obj-$(CONFIG_SPL_CRYPTO) += crypto/
obj-$(CONFIG_SPL_MPC8XXX_INIT_DDR) += ddr/fsl/
obj-$(CONFIG_ARMADA_38X) += ddr/marvell/a38x/
obj-$(CONFIG_ARMADA_XP) += ddr/marvell/axp/
obj-$(CONFIG_$(PHASE_)ALTERA_SDRAM) += ddr/altera/
obj-$(CONFIG_ARCH_IMX8M) += ddr/imx/imx8m/
obj-$(CONFIG_IMX8ULP_DRAM) += ddr/imx/imx8ulp/
obj-$(CONFIG_ARCH_IMX9) += ddr/imx/imx9/
obj-$(CONFIG_DRAM_SUN20I_D1) += ram/
obj-$(CONFIG_SPL_DM_RESET) += reset/
obj-$(CONFIG_SPL_MUSB_NEW) += usb/musb-new/
obj-$(CONFIG_SPL_USB_GADGET) += usb/gadget/
obj-$(CONFIG_SPL_USB_GADGET) += usb/common/
obj-$(CONFIG_SPL_USB_GADGET) += usb/gadget/udc/
obj-$(CONFIG_SPL_WATCHDOG) += watchdog/
obj-$(CONFIG_SPL_USB_HOST) += usb/host/
obj-$(CONFIG_SPL_SATA) += ata/ scsi/
obj-$(CONFIG_SPL_LEGACY_BLOCK) += block/
obj-$(CONFIG_SPL_THERMAL) += thermal/

endif
endif
endif

ifdef CONFIG_TPL_BUILD

obj-$(CONFIG_TPL_MPC8XXX_INIT_DDR) += ddr/fsl/

endif

ifeq ($(CONFIG_XPL_BUILD)$(CONFIG_TPL_BUILD),)

obj-y += ata/
obj-$(CONFIG_DM_DEMO) += demo/
obj-y += block/
obj-y += cache/
obj-$(CONFIG_CPU) += cpu/
obj-y += crypto/
obj-$(CONFIG_FASTBOOT) += fastboot/
obj-$(CONFIG_FWU_MDATA) += fwu-mdata/
obj-y += misc/
obj-$(CONFIG_MMC) += mmc/
obj-$(CONFIG_MULTIPLEXER) += mux/
obj-$(CONFIG_NVME) += nvme/
obj-$(CONFIG_PCI_ENDPOINT) += pci_endpoint/
obj-y += dfu/
obj-$(CONFIG_PCH) += pch/
obj-$(CONFIG_DM_REBOOT_MODE) += reboot-mode/
obj-y += rtc/
obj-y += scsi/
obj-y += sound/
obj-y += spmi/
obj-y += watchdog/
obj-$(CONFIG_QE) += qe/
obj-$(CONFIG_U_QE) += qe/
obj-y += mailbox/
obj-y += memory/
obj-y += mfd/
obj-y += mtd/
obj-y += pwm/
obj-y += reset/
obj-y += input/
obj-y += iommu/
# SOC specific infrastructure drivers.
obj-y += smem/
obj-y += thermal/
obj-$(CONFIG_TEE) += tee/
obj-$(CONFIG_ARM_FFA_TRANSPORT) += firmware/arm-ffa/
obj-y += axi/
obj-y += ufs/
obj-$(CONFIG_W1) += w1/
obj-$(CONFIG_W1_EEPROM) += w1-eeprom/

obj-$(CONFIG_MACH_PIC32) += ddr/microchip/
obj-$(CONFIG_FUZZ) += fuzz/
obj-$(CONFIG_DM_HWSPINLOCK) += hwspinlock/
obj-$(CONFIG_$(PHASE_)DM_RNG) += rng/
endif

obj-y += soc/

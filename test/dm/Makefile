# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2013 Google, Inc

obj-$(CONFIG_UT_DM) += test-dm.o

# Tests for particular subsystems - when enabling driver model for a new
# subsystem you must add sandbox tests here.
ifeq ($(CONFIG_SPL_BUILD),y)
obj-$(CONFIG_SPL_OF_PLATDATA) += of_platdata.o
else
obj-$(CONFIG_UT_DM) += bus.o
obj-$(CONFIG_UT_DM) += test-driver.o
obj-$(CONFIG_UT_DM) += test-fdt.o
obj-$(CONFIG_UT_DM) += test-uclass.o

obj-$(CONFIG_UT_DM) += core.o
obj-$(CONFIG_UT_DM) += read.o
obj-$(CONFIG_UT_DM) += phys2bus.o
ifneq ($(CONFIG_SANDBOX),)
obj-$(CONFIG_ACPIGEN) += acpi.o
obj-$(CONFIG_ACPIGEN) += acpigen.o
obj-$(CONFIG_ACPIGEN) += acpi_dp.o
obj-$(CONFIG_ADC) += adc.o
obj-$(CONFIG_SOUND) += audio.o
obj-$(CONFIG_AXI) += axi.o
obj-$(CONFIG_BLK) += blk.o
obj-$(CONFIG_BUTTON) += button.o
obj-$(CONFIG_DM_BOOTCOUNT) += bootcount.o
obj-$(CONFIG_CLK) += clk.o clk_ccf.o
obj-$(CONFIG_CPU) += cpu.o
obj-$(CONFIG_CROS_EC) += cros_ec.o
obj-$(CONFIG_DEVRES) += devres.o
obj-$(CONFIG_DMA) += dma.o
obj-$(CONFIG_VIDEO_MIPI_DSI) += dsi_host.o
obj-$(CONFIG_DM_DSA) += dsa.o
obj-$(CONFIG_DM_ETH) += eth.o
ifneq ($(CONFIG_EFI_PARTITION),)
obj-$(CONFIG_FASTBOOT_FLASH_MMC) += fastboot.o
endif
obj-$(CONFIG_FIRMWARE) += firmware.o
obj-$(CONFIG_DM_GPIO) += gpio.o
obj-$(CONFIG_DM_HWSPINLOCK) += hwspinlock.o
obj-$(CONFIG_DM_I2C) += i2c.o
obj-$(CONFIG_SOUND) += i2s.o
obj-y += irq.o
obj-$(CONFIG_CLK_K210_SET_RATE) += k210_pll.o
obj-$(CONFIG_LED) += led.o
obj-$(CONFIG_DM_MAILBOX) += mailbox.o
obj-$(CONFIG_DM_MDIO) += mdio.o
obj-$(CONFIG_DM_MDIO_MUX) += mdio_mux.o
obj-$(CONFIG_MISC) += misc.o
obj-$(CONFIG_DM_MMC) += mmc.o
obj-$(CONFIG_CMD_MUX) += mux-cmd.o
obj-$(CONFIG_MULTIPLEXER) += mux-emul.o
obj-$(CONFIG_MUX_MMIO) += mux-mmio.o
obj-y += fdtdec.o
obj-$(CONFIG_UT_DM) += nop.o
obj-y += ofnode.o
obj-y += ofread.o
obj-y += of_extra.o
obj-$(CONFIG_OSD) += osd.o
obj-$(CONFIG_DM_VIDEO) += panel.o
obj-$(CONFIG_EFI_PARTITION) += part.o
obj-$(CONFIG_DM_PCI) += pci.o
obj-$(CONFIG_P2SB) += p2sb.o
obj-$(CONFIG_PCI_ENDPOINT) += pci_ep.o
obj-$(CONFIG_PCH) += pch.o
obj-$(CONFIG_PHY) += phy.o
ifneq ($(CONFIG_PINMUX),)
obj-$(CONFIG_PINCONF) += pinmux.o
endif
obj-$(CONFIG_POWER_DOMAIN) += power-domain.o
obj-$(CONFIG_ACPI_PMC) += pmc.o
obj-$(CONFIG_DM_PMIC) += pmic.o
obj-$(CONFIG_DM_PWM) += pwm.o
obj-$(CONFIG_QFW) += qfw.o
obj-$(CONFIG_RAM) += ram.o
obj-y += regmap.o
obj-$(CONFIG_REMOTEPROC) += remoteproc.o
obj-$(CONFIG_DM_RESET) += reset.o
obj-$(CONFIG_SYSRESET) += sysreset.o
obj-$(CONFIG_DM_REGULATOR) += regulator.o
obj-$(CONFIG_DM_RNG) += rng.o
obj-$(CONFIG_DM_RTC) += rtc.o
obj-$(CONFIG_SCMI_FIRMWARE) += scmi.o
obj-$(CONFIG_DM_SERIAL) += serial.o
obj-$(CONFIG_DM_SPI_FLASH) += sf.o
obj-$(CONFIG_SIMPLE_BUS) += simple-bus.o
obj-$(CONFIG_SIMPLE_PM_BUS) += simple-pm-bus.o
obj-$(CONFIG_SMEM) += smem.o
obj-$(CONFIG_SOC_DEVICE) += soc.o
obj-$(CONFIG_SOUND) += sound.o
obj-$(CONFIG_DM_SPI) += spi.o
obj-$(CONFIG_SPMI) += spmi.o
obj-y += syscon.o
obj-$(CONFIG_RESET_SYSCON) += syscon-reset.o
obj-$(CONFIG_SYSINFO) += sysinfo.o
obj-$(CONFIG_TEE) += tee.o
obj-$(CONFIG_TIMER) += timer.o
obj-$(CONFIG_DM_USB) += usb.o
obj-$(CONFIG_DM_VIDEO) += video.o
obj-$(CONFIG_VIRTIO_SANDBOX) += virtio.o
obj-$(CONFIG_WDT) += wdt.o
endif
endif # !SPL

#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-$(CONFIG_$(SPL_)DM)		+= core/
obj-$(CONFIG_$(SPL_)CLK)	+= clk/
obj-$(CONFIG_$(SPL_)LED)	+= led/
obj-$(CONFIG_$(SPL_)PINCTRL)	+= pinctrl/
obj-$(CONFIG_$(SPL_)RAM)	+= ram/

ifdef CONFIG_SPL_BUILD

obj-$(CONFIG_SPL_CRYPTO_SUPPORT) += crypto/
obj-$(CONFIG_SPL_I2C_SUPPORT) += i2c/
obj-$(CONFIG_SPL_GPIO_SUPPORT) += gpio/
obj-$(CONFIG_SPL_MMC_SUPPORT) += mmc/
obj-$(CONFIG_SPL_MPC8XXX_INIT_DDR_SUPPORT) += ddr/fsl/
obj-$(CONFIG_ARMADA_38X) += ddr/marvell/a38x/
obj-$(CONFIG_ARMADA_XP) += ddr/marvell/axp/
obj-$(CONFIG_ALTERA_SDRAM) += ddr/altera/
obj-$(CONFIG_SPL_SERIAL_SUPPORT) += serial/
obj-$(CONFIG_SPL_SPI_SUPPORT) += spi/
obj-$(CONFIG_SPL_POWER_SUPPORT) += power/ power/pmic/
obj-$(CONFIG_SPL_POWER_SUPPORT) += power/regulator/
obj-$(CONFIG_SPL_DRIVERS_MISC_SUPPORT) += misc/
obj-$(CONFIG_SPL_MTD_SUPPORT) += mtd/
obj-$(CONFIG_SPL_NAND_SUPPORT) += mtd/nand/
obj-$(CONFIG_SPL_ONENAND_SUPPORT) += mtd/onenand/
obj-$(CONFIG_SPL_SPI_FLASH_SUPPORT) += mtd/spi/
obj-$(CONFIG_SPL_UBI) += mtd/ubispl/
obj-$(CONFIG_SPL_DMA_SUPPORT) += dma/
obj-$(CONFIG_SPL_ETH_SUPPORT) += net/
obj-$(CONFIG_SPL_ETH_SUPPORT) += net/phy/
obj-$(CONFIG_SPL_USBETH_SUPPORT) += net/phy/
obj-$(CONFIG_SPL_MUSB_NEW_SUPPORT) += usb/musb-new/
obj-$(CONFIG_SPL_USBETH_SUPPORT) += usb/gadget/
obj-$(CONFIG_SPL_WATCHDOG_SUPPORT) += watchdog/
obj-$(CONFIG_SPL_USB_HOST_SUPPORT) += usb/host/
obj-$(CONFIG_OMAP_USB_PHY) += usb/phy/
obj-$(CONFIG_SPL_SATA_SUPPORT) += block/
obj-$(CONFIG_SPL_USB_HOST_SUPPORT) += block/
obj-$(CONFIG_SPL_MMC_SUPPORT) += block/

else

obj-y += adc/
obj-$(CONFIG_DM_DEMO) += demo/
obj-$(CONFIG_BIOSEMU) += bios_emulator/
obj-y += block/
obj-$(CONFIG_BOOTCOUNT_LIMIT) += bootcount/
obj-$(CONFIG_CPU) += cpu/
obj-y += crypto/
obj-$(CONFIG_FPGA) += fpga/
obj-y += hwmon/
obj-y += misc/
obj-y += pcmcia/
obj-y += dfu/
obj-$(CONFIG_X86) += pch/
obj-y += rtc/
obj-y += sound/
obj-y += spmi/
obj-y += timer/
obj-y += tpm/
obj-y += twserial/
obj-y += video/
obj-y += watchdog/
obj-$(CONFIG_QE) += qe/
obj-$(CONFIG_U_QE) += qe/
obj-y += mailbox/
obj-y += memory/
obj-y += pwm/
obj-y += reset/
obj-y += input/
# SOC specific infrastructure drivers.
obj-y += soc/
obj-$(CONFIG_REMOTEPROC) += remoteproc/
obj-y += thermal/

obj-$(CONFIG_MACH_PIC32) += ddr/microchip/
endif

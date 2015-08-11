ifdef CONFIG_SPL_BUILD

obj-$(CONFIG_SPL_CLK_SUPPORT) += clk/
obj-$(CONFIG_SPL_DM) += core/
obj-$(CONFIG_SPL_I2C_SUPPORT) += i2c/
obj-$(CONFIG_SPL_GPIO_SUPPORT) += gpio/
obj-$(CONFIG_SPL_MMC_SUPPORT) += mmc/
obj-$(CONFIG_SPL_MPC8XXX_INIT_DDR_SUPPORT) += ddr/fsl/
obj-$(CONFIG_SYS_MVEBU_DDR_A38X) += ddr/marvell/a38x/
obj-$(CONFIG_SYS_MVEBU_DDR_AXP) += ddr/marvell/axp/
obj-$(CONFIG_ALTERA_SDRAM) += ddr/altera/
obj-$(CONFIG_SPL_SERIAL_SUPPORT) += serial/
obj-$(CONFIG_SPL_SPI_FLASH_SUPPORT) += mtd/spi/
obj-$(CONFIG_SPL_SPI_SUPPORT) += spi/
obj-$(CONFIG_SPL_LED_SUPPORT) += led/
obj-$(CONFIG_SPL_POWER_SUPPORT) += power/ power/pmic/
obj-$(CONFIG_SPL_POWER_SUPPORT) += power/regulator/
obj-$(CONFIG_SPL_MTD_SUPPORT) += mtd/
obj-$(CONFIG_SPL_NAND_SUPPORT) += mtd/nand/
obj-$(CONFIG_SPL_DRIVERS_MISC_SUPPORT) += misc/
obj-$(CONFIG_SPL_ONENAND_SUPPORT) += mtd/onenand/
obj-$(CONFIG_SPL_DMA_SUPPORT) += dma/
obj-$(CONFIG_SPL_ETH_SUPPORT) += net/
obj-$(CONFIG_SPL_ETH_SUPPORT) += net/phy/
obj-$(CONFIG_SPL_USBETH_SUPPORT) += net/phy/
obj-$(CONFIG_SPL_RAM_SUPPORT) += ram/
obj-$(CONFIG_SPL_MUSB_NEW_SUPPORT) += usb/musb-new/
obj-$(CONFIG_SPL_USBETH_SUPPORT) += usb/gadget/
obj-$(CONFIG_SPL_WATCHDOG_SUPPORT) += watchdog/
obj-$(CONFIG_SPL_USB_HOST_SUPPORT) += usb/host/
obj-$(CONFIG_OMAP_USB_PHY) += usb/phy/
obj-$(CONFIG_SPL_SATA_SUPPORT) += block/

else

obj-$(CONFIG_CLK) += clk/
obj-$(CONFIG_DM) += core/
obj-$(CONFIG_DM_DEMO) += demo/
obj-$(CONFIG_BIOSEMU) += bios_emulator/
obj-y += block/
obj-$(CONFIG_BOOTCOUNT_LIMIT) += bootcount/
obj-$(CONFIG_CPU) += cpu/
obj-y += crypto/
obj-$(CONFIG_FPGA) += fpga/
obj-y += hwmon/
obj-$(CONFIG_LED) += led/
obj-y += misc/
obj-y += pcmcia/
obj-y += dfu/
obj-$(CONFIG_RAM) += ram/
obj-y += rtc/
obj-y += sound/
obj-y += tpm/
obj-y += twserial/
obj-y += video/
obj-y += watchdog/
obj-$(CONFIG_QE) += qe/
obj-$(CONFIG_U_QE) += qe/
obj-y += memory/
obj-y += pwm/
obj-y += input/
# SOC specific infrastructure drivers.
obj-y += soc/
obj-y += thermal/

endif

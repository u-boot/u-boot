#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-$(CONFIG_MISC) += misc-uclass.o
obj-$(CONFIG_ALI152X) += ali512x.o
obj-$(CONFIG_ALTERA_SYSID) += altera_sysid.o
obj-$(CONFIG_DS4510)  += ds4510.o
obj-$(CONFIG_CBMEM_CONSOLE) += cbmem_console.o
ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_CROS_EC) += cros_ec.o
obj-$(CONFIG_CROS_EC_LPC) += cros_ec_lpc.o
obj-$(CONFIG_CROS_EC_I2C) += cros_ec_i2c.o
obj-$(CONFIG_CROS_EC_SANDBOX) += cros_ec_sandbox.o
obj-$(CONFIG_CROS_EC_SPI) += cros_ec_spi.o
obj-$(CONFIG_FSL_DEBUG_SERVER) += fsl_debug_server.o
endif
obj-$(CONFIG_FSL_IIM) += fsl_iim.o
obj-$(CONFIG_GPIO_LED) += gpio_led.o
obj-$(CONFIG_I2C_EEPROM) += i2c_eeprom.o
obj-$(CONFIG_FSL_MC9SDZ60) += mc9sdz60.o
obj-$(CONFIG_MXC_OCOTP) += mxc_ocotp.o
obj-$(CONFIG_MXS_OCOTP) += mxs_ocotp.o
obj-$(CONFIG_NS87308) += ns87308.o
obj-$(CONFIG_PDSP188x) += pdsp188x.o
obj-$(CONFIG_$(SPL_)PWRSEQ) += pwrseq-uclass.o
obj-$(CONFIG_SANDBOX) += sysreset_sandbox.o
ifdef CONFIG_DM_I2C
obj-$(CONFIG_SANDBOX) += i2c_eeprom_emul.o
endif
obj-$(CONFIG_SMSC_LPC47M) += smsc_lpc47m.o
obj-$(CONFIG_SMSC_SIO1007) += smsc_sio1007.o
obj-$(CONFIG_STATUS_LED) += status_led.o
obj-$(CONFIG_SANDBOX) += swap_case.o
obj-$(CONFIG_SANDBOX) += syscon_sandbox.o
obj-$(CONFIG_TWL4030_LED) += twl4030_led.o
obj-$(CONFIG_FSL_IFC) += fsl_ifc.o
obj-$(CONFIG_FSL_SEC_MON) += fsl_sec_mon.o
obj-$(CONFIG_PCA9551_LED) += pca9551_led.o
obj-$(CONFIG_SYSRESET) += sysreset-uclass.o
obj-$(CONFIG_FSL_DEVICE_DISABLE) += fsl_devdis.o
obj-$(CONFIG_WINBOND_W83627) += winbond_w83627.o
obj-$(CONFIG_QFW) += qfw.o

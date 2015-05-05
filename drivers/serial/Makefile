#
# (C) Copyright 2006-2009
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifdef CONFIG_DM_SERIAL
obj-y += serial-uclass.o
obj-$(CONFIG_PL01X_SERIAL) += serial_pl01x.o
obj-$(CONFIG_PPC) += serial_ppc.o
else
obj-y += serial.o
obj-$(CONFIG_PL010_SERIAL) += serial_pl01x.o
obj-$(CONFIG_PL011_SERIAL) += serial_pl01x.o
obj-$(CONFIG_SYS_NS16550_SERIAL) += serial_ns16550.o
endif

obj-$(CONFIG_ALTERA_UART) += altera_uart.o
obj-$(CONFIG_ALTERA_JTAG_UART) += altera_jtag_uart.o
obj-$(CONFIG_ARM_DCC) += arm_dcc.o
obj-$(CONFIG_ATMEL_USART) += atmel_usart.o
obj-$(CONFIG_DW_SERIAL) += serial_dw.o
obj-$(CONFIG_LPC32XX_HSUART) += lpc32xx_hsuart.o
obj-$(CONFIG_MCFUART) += mcfuart.o
obj-$(CONFIG_OPENCORES_YANU) += opencores_yanu.o
obj-$(CONFIG_SYS_NS16550) += ns16550.o
obj-$(CONFIG_S5P) += serial_s5p.o
obj-$(CONFIG_IMX_SERIAL) += serial_imx.o
obj-$(CONFIG_MAX3100_SERIAL) += serial_max3100.o
obj-$(CONFIG_MXC_UART) += serial_mxc.o
obj-$(CONFIG_PXA_SERIAL) += serial_pxa.o
obj-$(CONFIG_SA1100_SERIAL) += serial_sa1100.o
obj-$(CONFIG_S3C24X0_SERIAL) += serial_s3c24x0.o
obj-$(CONFIG_XILINX_UARTLITE) += serial_xuartlite.o
obj-$(CONFIG_SANDBOX_SERIAL) += sandbox.o
obj-$(CONFIG_SCIF_CONSOLE) += serial_sh.o
obj-$(CONFIG_ZYNQ_SERIAL) += serial_zynq.o
obj-$(CONFIG_BFIN_SERIAL) += serial_bfin.o
obj-$(CONFIG_FSL_LPUART) += serial_lpuart.o
obj-$(CONFIG_MXS_AUART) += mxs_auart.o
obj-$(CONFIG_ARC_SERIAL) += serial_arc.o
obj-$(CONFIG_TEGRA_SERIAL) += serial_tegra.o
obj-$(CONFIG_UNIPHIER_SERIAL) += serial_uniphier.o
obj-$(CONFIG_OMAP_SERIAL) += serial_omap.o
obj-$(CONFIG_X86_SERIAL) += serial_x86.o
obj-$(CONFIG_STM32_SERIAL) += serial_stm32.o

ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_USB_TTY) += usbtty.o
endif

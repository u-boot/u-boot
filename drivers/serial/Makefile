#
# (C) Copyright 2006-2009
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y += serial.o

obj-$(CONFIG_ALTERA_UART) += altera_uart.o
obj-$(CONFIG_ALTERA_JTAG_UART) += altera_jtag_uart.o
obj-$(CONFIG_ARM_DCC) += arm_dcc.o
obj-$(CONFIG_ATMEL_USART) += atmel_usart.o
obj-$(CONFIG_LPC32XX_HSUART) += lpc32xx_hsuart.o
obj-$(CONFIG_MCFUART) += mcfuart.o
obj-$(CONFIG_OPENCORES_YANU) += opencores_yanu.o
obj-$(CONFIG_SYS_NS16550) += ns16550.o
obj-$(CONFIG_S5P) += serial_s5p.o
obj-$(CONFIG_SYS_NS16550_SERIAL) += serial_ns16550.o
obj-$(CONFIG_IMX_SERIAL) += serial_imx.o
obj-$(CONFIG_IXP_SERIAL) += serial_ixp.o
obj-$(CONFIG_KS8695_SERIAL) += serial_ks8695.o
obj-$(CONFIG_MAX3100_SERIAL) += serial_max3100.o
obj-$(CONFIG_MXC_UART) += serial_mxc.o
obj-$(CONFIG_PL010_SERIAL) += serial_pl01x.o
obj-$(CONFIG_PL011_SERIAL) += serial_pl01x.o
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

ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_USB_TTY) += usbtty.o
endif

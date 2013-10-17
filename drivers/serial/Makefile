#
# (C) Copyright 2006-2009
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)libserial.o

COBJS-y += serial.o

COBJS-$(CONFIG_ALTERA_UART) += altera_uart.o
COBJS-$(CONFIG_ALTERA_JTAG_UART) += altera_jtag_uart.o
COBJS-$(CONFIG_ARM_DCC) += arm_dcc.o
COBJS-$(CONFIG_ATMEL_USART) += atmel_usart.o
COBJS-$(CONFIG_LPC32XX_HSUART) += lpc32xx_hsuart.o
COBJS-$(CONFIG_MCFUART) += mcfuart.o
COBJS-$(CONFIG_OPENCORES_YANU) += opencores_yanu.o
COBJS-$(CONFIG_SYS_NS16550) += ns16550.o
COBJS-$(CONFIG_S5P) += serial_s5p.o
COBJS-$(CONFIG_SYS_NS16550_SERIAL) += serial_ns16550.o
COBJS-$(CONFIG_IMX_SERIAL) += serial_imx.o
COBJS-$(CONFIG_IXP_SERIAL) += serial_ixp.o
COBJS-$(CONFIG_KS8695_SERIAL) += serial_ks8695.o
COBJS-$(CONFIG_MAX3100_SERIAL) += serial_max3100.o
COBJS-$(CONFIG_MXC_UART) += serial_mxc.o
COBJS-$(CONFIG_PL010_SERIAL) += serial_pl01x.o
COBJS-$(CONFIG_PL011_SERIAL) += serial_pl01x.o
COBJS-$(CONFIG_PXA_SERIAL) += serial_pxa.o
COBJS-$(CONFIG_SA1100_SERIAL) += serial_sa1100.o
COBJS-$(CONFIG_S3C24X0_SERIAL) += serial_s3c24x0.o
COBJS-$(CONFIG_XILINX_UARTLITE) += serial_xuartlite.o
COBJS-$(CONFIG_SANDBOX_SERIAL) += sandbox.o
COBJS-$(CONFIG_SCIF_CONSOLE) += serial_sh.o
COBJS-$(CONFIG_ZYNQ_SERIAL) += serial_zynq.o
COBJS-$(CONFIG_BFIN_SERIAL) += serial_bfin.o
COBJS-$(CONFIG_FSL_LPUART) += serial_lpuart.o
COBJS-$(CONFIG_MXS_AUART) += mxs_auart.o

ifndef CONFIG_SPL_BUILD
COBJS-$(CONFIG_USB_TTY) += usbtty.o
endif

COBJS	:= $(sort $(COBJS-y))
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################

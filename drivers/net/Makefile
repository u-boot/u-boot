#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)libnet.o

COBJS-$(CONFIG_PPC4xx_EMAC) += 4xx_enet.o
COBJS-$(CONFIG_ALTERA_TSE) += altera_tse.o
COBJS-$(CONFIG_ARMADA100_FEC) += armada100_fec.o
COBJS-$(CONFIG_DRIVER_AT91EMAC) += at91_emac.o
COBJS-$(CONFIG_DRIVER_AX88180) += ax88180.o
COBJS-$(CONFIG_BFIN_MAC) += bfin_mac.o
COBJS-$(CONFIG_CALXEDA_XGMAC) += calxedaxgmac.o
COBJS-$(CONFIG_CS8900) += cs8900.o
COBJS-$(CONFIG_TULIP) += dc2114x.o
COBJS-$(CONFIG_DESIGNWARE_ETH) += designware.o
COBJS-$(CONFIG_DRIVER_DM9000) += dm9000x.o
COBJS-$(CONFIG_DNET) += dnet.o
COBJS-$(CONFIG_E1000) += e1000.o
COBJS-$(CONFIG_E1000_SPI) += e1000_spi.o
COBJS-$(CONFIG_EEPRO100) += eepro100.o
COBJS-$(CONFIG_ENC28J60) += enc28j60.o
COBJS-$(CONFIG_EP93XX) += ep93xx_eth.o
COBJS-$(CONFIG_ETHOC) += ethoc.o
COBJS-$(CONFIG_FEC_MXC) += fec_mxc.o
COBJS-$(CONFIG_FSLDMAFEC) += fsl_mcdmafec.o mcfmii.o
COBJS-$(CONFIG_FTGMAC100) += ftgmac100.o
COBJS-$(CONFIG_FTMAC110) += ftmac110.o
COBJS-$(CONFIG_FTMAC100) += ftmac100.o
COBJS-$(CONFIG_GRETH) += greth.o
COBJS-$(CONFIG_INCA_IP_SWITCH) += inca-ip_sw.o
COBJS-$(CONFIG_DRIVER_KS8695ETH) += ks8695eth.o
COBJS-$(CONFIG_KS8851_MLL) += ks8851_mll.o
COBJS-$(CONFIG_LAN91C96) += lan91c96.o
COBJS-$(CONFIG_MACB) += macb.o
COBJS-$(CONFIG_MCFFEC) += mcffec.o mcfmii.o
COBJS-$(CONFIG_MPC5xxx_FEC) += mpc5xxx_fec.o
COBJS-$(CONFIG_MPC512x_FEC) += mpc512x_fec.o
COBJS-$(CONFIG_MVGBE) += mvgbe.o
COBJS-$(CONFIG_NATSEMI) += natsemi.o
COBJS-$(CONFIG_DRIVER_NE2000) += ne2000.o ne2000_base.o
COBJS-$(CONFIG_DRIVER_AX88796L) += ax88796.o ne2000_base.o
COBJS-$(CONFIG_NETCONSOLE) += netconsole.o
COBJS-$(CONFIG_NS8382X) += ns8382x.o
COBJS-$(CONFIG_PCNET) += pcnet.o
COBJS-$(CONFIG_PLB2800_ETHER) += plb2800_eth.o
COBJS-$(CONFIG_RTL8139) += rtl8139.o
COBJS-$(CONFIG_RTL8169) += rtl8169.o
COBJS-$(CONFIG_SH_ETHER) += sh_eth.o
COBJS-$(CONFIG_SMC91111) += smc91111.o
COBJS-$(CONFIG_SMC911X) += smc911x.o
COBJS-$(CONFIG_SUNXI_WEMAC) += sunxi_wemac.o
COBJS-$(CONFIG_DRIVER_TI_EMAC) += davinci_emac.o
COBJS-$(CONFIG_TSEC_ENET) += tsec.o fsl_mdio.o
COBJS-$(CONFIG_DRIVER_TI_CPSW) += cpsw.o
COBJS-$(CONFIG_FMAN_ENET) += fsl_mdio.o
COBJS-$(CONFIG_TSI108_ETH) += tsi108_eth.o
COBJS-$(CONFIG_ULI526X) += uli526x.o
COBJS-$(CONFIG_VSC7385_ENET) += vsc7385.o
COBJS-$(CONFIG_XILINX_AXIEMAC) += xilinx_axi_emac.o
COBJS-$(CONFIG_XILINX_EMACLITE) += xilinx_emaclite.o
COBJS-$(CONFIG_XILINX_LL_TEMAC) += xilinx_ll_temac.o xilinx_ll_temac_mdio.o \
		xilinx_ll_temac_fifo.o xilinx_ll_temac_sdma.o
COBJS-$(CONFIG_ZYNQ_GEM) += zynq_gem.o

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

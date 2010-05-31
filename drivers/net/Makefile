#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)libnet.a

COBJS-$(CONFIG_DRIVER_3C589) += 3c589.o
COBJS-$(CONFIG_PPC4xx_EMAC) += 4xx_enet.o
COBJS-$(CONFIG_ALTERA_TSE) += altera_tse.o
COBJS-$(CONFIG_DRIVER_AT91EMAC) += at91_emac.o
COBJS-$(CONFIG_DRIVER_AX88180) += ax88180.o
COBJS-$(CONFIG_BCM570x) += bcm570x.o bcm570x_autoneg.o 5701rls.o
COBJS-$(CONFIG_BFIN_MAC) += bfin_mac.o
COBJS-$(CONFIG_CS8900) += cs8900.o
COBJS-$(CONFIG_TULIP) += dc2114x.o
COBJS-$(CONFIG_DRIVER_DM9000) += dm9000x.o
COBJS-$(CONFIG_DNET) += dnet.o
COBJS-$(CONFIG_E1000) += e1000.o
COBJS-$(CONFIG_EEPRO100) += eepro100.o
COBJS-$(CONFIG_ENC28J60) += enc28j60.o
COBJS-$(CONFIG_EP93XX) += ep93xx_eth.o
COBJS-$(CONFIG_ETHOC) += ethoc.o
COBJS-$(CONFIG_FEC_MXC) += fec_mxc.o
COBJS-$(CONFIG_FSLDMAFEC) += fsl_mcdmafec.o mcfmii.o
COBJS-$(CONFIG_FTMAC100) += ftmac100.o
COBJS-$(CONFIG_GRETH) += greth.o
COBJS-$(CONFIG_INCA_IP_SWITCH) += inca-ip_sw.o
COBJS-$(CONFIG_KIRKWOOD_EGIGA) += kirkwood_egiga.o
COBJS-$(CONFIG_DRIVER_KS8695ETH) += ks8695eth.o
COBJS-$(CONFIG_LAN91C96) += lan91c96.o
COBJS-$(CONFIG_MACB) += macb.o
COBJS-$(CONFIG_MCFFEC) += mcffec.o mcfmii.o
COBJS-$(CONFIG_MPC5xxx_FEC) += mpc5xxx_fec.o
COBJS-$(CONFIG_MPC512x_FEC) += mpc512x_fec.o
COBJS-$(CONFIG_NATSEMI) += natsemi.o
COBJS-$(CONFIG_DRIVER_NE2000) += ne2000.o ne2000_base.o
COBJS-$(CONFIG_DRIVER_AX88796L) += ax88796.o ne2000_base.o
COBJS-$(CONFIG_DRIVER_NETARMETH) += netarm_eth.o
COBJS-$(CONFIG_NETCONSOLE) += netconsole.o
COBJS-$(CONFIG_DRIVER_NS7520_ETHERNET) += ns7520_eth.o
COBJS-$(CONFIG_NS8382X) += ns8382x.o
COBJS-$(CONFIG_DRIVER_NS9750_ETHERNET) += ns9750_eth.o
COBJS-$(CONFIG_PCNET) += pcnet.o
COBJS-$(CONFIG_PLB2800_ETHER) += plb2800_eth.o
COBJS-$(CONFIG_DRIVER_RTL8019) += rtl8019.o
COBJS-$(CONFIG_RTL8139) += rtl8139.o
COBJS-$(CONFIG_RTL8169) += rtl8169.o
COBJS-$(CONFIG_DRIVER_S3C4510_ETH) += s3c4510b_eth.o
COBJS-$(CONFIG_SH_ETHER) += sh_eth.o
COBJS-$(CONFIG_SMC91111) += smc91111.o
COBJS-$(CONFIG_SMC911X) += smc911x.o
COBJS-$(CONFIG_TIGON3) += tigon3.o bcm570x_autoneg.o 5701rls.o
COBJS-$(CONFIG_DRIVER_TI_EMAC) += davinci_emac.o
COBJS-$(CONFIG_TSEC_ENET) += tsec.o
COBJS-$(CONFIG_TSI108_ETH) += tsi108_eth.o
COBJS-$(CONFIG_ULI526X) += uli526x.o
COBJS-$(CONFIG_VSC7385_ENET) += vsc7385.o
COBJS-$(CONFIG_XILINX_EMACLITE) += xilinx_emaclite.o

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################

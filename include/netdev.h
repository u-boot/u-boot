/*
 * (C) Copyright 2008
 * Benjamin Warren, biggerbadderben@gmail.com
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * netdev.h - definitions an prototypes for network devices
 */

#ifndef _NETDEV_H_
#define _NETDEV_H_

/*
 * Board and CPU-specific initialization functions
 * board_eth_init() has highest priority.  cpu_eth_init() only
 * gets called if board_eth_init() isn't instantiated or fails.
 * Return values:
 *      0: success
 *     -1: failure
 */

int board_eth_init(bd_t *bis);
int cpu_eth_init(bd_t *bis);

/* Driver initialization prototypes */
int altera_tse_initialize(u8 dev_num, int mac_base,
			  int sgdma_rx_base, int sgdma_tx_base,
			  u32 sgdma_desc_base, u32 sgdma_desc_size);
int at91emac_register(bd_t *bis, unsigned long iobase);
int au1x00_enet_initialize(bd_t*);
int ax88180_initialize(bd_t *bis);
int bfin_EMAC_initialize(bd_t *bis);
int calxedaxgmac_initialize(u32 id, ulong base_addr);
int cs8900_initialize(u8 dev_num, int base_addr);
int davinci_emac_initialize(void);
int dc21x4x_initialize(bd_t *bis);
int designware_initialize(u32 id, ulong base_addr, u32 phy_addr, u32 interface);
int dm9000_initialize(bd_t *bis);
int dnet_eth_initialize(int id, void *regs, unsigned int phy_addr);
int e1000_initialize(bd_t *bis);
int eepro100_initialize(bd_t *bis);
int enc28j60_initialize(unsigned int bus, unsigned int cs,
	unsigned int max_hz, unsigned int mode);
int ep93xx_eth_initialize(u8 dev_num, int base_addr);
int eth_3com_initialize (bd_t * bis);
int ethoc_initialize(u8 dev_num, int base_addr);
int fec_initialize (bd_t *bis);
int fecmxc_initialize(bd_t *bis);
int fecmxc_initialize_multi(bd_t *bis, int dev_id, int phy_id, uint32_t addr);
int ftgmac100_initialize(bd_t *bits);
int ftmac100_initialize(bd_t *bits);
int greth_initialize(bd_t *bis);
void gt6426x_eth_initialize(bd_t *bis);
int inca_switch_initialize(bd_t *bis);
int ks8695_eth_initialize(void);
int lan91c96_initialize(u8 dev_num, int base_addr);
int macb_eth_initialize(int id, void *regs, unsigned int phy_addr);
int mcdmafec_initialize(bd_t *bis);
int mcffec_initialize(bd_t *bis);
int mpc512x_fec_initialize(bd_t *bis);
int mpc5xxx_fec_initialize(bd_t *bis);
int mpc8220_fec_initialize(bd_t *bis);
int mpc82xx_scc_enet_initialize(bd_t *bis);
int mvgbe_initialize(bd_t *bis);
int natsemi_initialize(bd_t *bis);
int ne2k_register(void);
int npe_initialize(bd_t *bis);
int ns8382x_initialize(bd_t *bis);
int pcnet_initialize(bd_t *bis);
int plb2800_eth_initialize(bd_t *bis);
int ppc_4xx_eth_initialize (bd_t *bis);
int rtl8139_initialize(bd_t *bis);
int rtl8169_initialize(bd_t *bis);
int scc_initialize(bd_t *bis);
int sh_eth_initialize(bd_t *bis);
int skge_initialize(bd_t *bis);
int smc91111_initialize(u8 dev_num, int base_addr);
int smc911x_initialize(u8 dev_num, int base_addr);
int tsi108_eth_initialize(bd_t *bis);
int uec_standard_init(bd_t *bis);
int uli526x_initialize(bd_t *bis);
int armada100_fec_register(unsigned long base_addr);
int xilinx_axiemac_initialize(bd_t *bis, unsigned long base_addr,
							unsigned long dma_addr);
int xilinx_emaclite_initialize(bd_t *bis, unsigned long base_addr,
							int txpp, int rxpp);
int xilinx_ll_temac_eth_init(bd_t *bis, unsigned long base_addr, int flags,
						unsigned long ctrl_addr);
int zynq_gem_initialize(bd_t *bis, int base_addr);
/*
 * As long as the Xilinx xps_ll_temac ethernet driver has not its own interface
 * exported by a public hader file, we need a global definition at this point.
 */
#if defined(CONFIG_XILINX_LL_TEMAC)
#define XILINX_LL_TEMAC_M_FIFO		0	/* use FIFO Ctrl */
#define XILINX_LL_TEMAC_M_SDMA_PLB	(1 << 0)/* use SDMA Ctrl via PLB */
#define XILINX_LL_TEMAC_M_SDMA_DCR	(1 << 1)/* use SDMA Ctrl via DCR */
#endif

/* Boards with PCI network controllers can call this from their board_eth_init()
 * function to initialize whatever's on board.
 * Return value is total # of devices found */

static inline int pci_eth_init(bd_t *bis)
{
	int num = 0;

#ifdef CONFIG_PCI

#ifdef CONFIG_EEPRO100
	num += eepro100_initialize(bis);
#endif
#ifdef CONFIG_TULIP
	num += dc21x4x_initialize(bis);
#endif
#ifdef CONFIG_E1000
	num += e1000_initialize(bis);
#endif
#ifdef CONFIG_PCNET
	num += pcnet_initialize(bis);
#endif
#ifdef CONFIG_NATSEMI
	num += natsemi_initialize(bis);
#endif
#ifdef CONFIG_NS8382X
	num += ns8382x_initialize(bis);
#endif
#if defined(CONFIG_RTL8139)
	num += rtl8139_initialize(bis);
#endif
#if defined(CONFIG_RTL8169)
	num += rtl8169_initialize(bis);
#endif
#if defined(CONFIG_ULI526X)
	num += uli526x_initialize(bis);
#endif

#endif  /* CONFIG_PCI */
	return num;
}

/*
 * Boards with mv88e61xx switch can use this by defining
 * CONFIG_MV88E61XX_SWITCH in respective board configheader file
 * the stuct and enums here are used to specify switch configuration params
 */
#if defined(CONFIG_MV88E61XX_SWITCH)
enum mv88e61xx_cfg_vlan {
	MV88E61XX_VLANCFG_DEFAULT,
	MV88E61XX_VLANCFG_ROUTER
};

enum mv88e61xx_cfg_mdip {
	MV88E61XX_MDIP_NOCHANGE,
	MV88E61XX_MDIP_REVERSE
};

enum mv88e61xx_cfg_ledinit {
	MV88E61XX_LED_INIT_DIS,
	MV88E61XX_LED_INIT_EN
};

enum mv88e61xx_cfg_rgmiid {
	MV88E61XX_RGMII_DELAY_DIS,
	MV88E61XX_RGMII_DELAY_EN
};

enum mv88e61xx_cfg_prtstt {
	MV88E61XX_PORTSTT_DISABLED,
	MV88E61XX_PORTSTT_BLOCKING,
	MV88E61XX_PORTSTT_LEARNING,
	MV88E61XX_PORTSTT_FORWARDING
};

struct mv88e61xx_config {
	char *name;
	enum mv88e61xx_cfg_vlan vlancfg;
	enum mv88e61xx_cfg_rgmiid rgmii_delay;
	enum mv88e61xx_cfg_prtstt portstate;
	enum mv88e61xx_cfg_ledinit led_init;
	enum mv88e61xx_cfg_mdip mdip;
	u32 ports_enabled;
	u8 cpuport;
};

int mv88e61xx_switch_initialize(struct mv88e61xx_config *swconfig);
#endif /* CONFIG_MV88E61XX_SWITCH */

/*
 * Allow FEC to fine-tune MII configuration on boards which require this.
 */
int fecmxc_register_mii_postcall(struct eth_device *dev, int (*cb)(int));

#endif /* _NETDEV_H_ */

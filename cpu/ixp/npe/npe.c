/*
 * (C) Copyright 2005-2006
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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

#if 0
#define DEBUG		/* define for debug output */
#endif

#include <config.h>
#include <common.h>
#include <net.h>
#include <miiphy.h>
#include <malloc.h>
#include <asm/processor.h>
#include <asm/arch-ixp/ixp425.h>

#include <IxOsal.h>
#include <IxEthAcc.h>
#include <IxEthDB.h>
#include <IxNpeDl.h>
#include <IxQMgr.h>
#include <IxNpeMh.h>
#include <ix_ossl.h>
#include <IxFeatureCtrl.h>

#include <npe.h>

#ifdef CONFIG_IXP4XX_NPE

static IxQMgrDispatcherFuncPtr qDispatcherFunc = NULL;
static int npe_exists[NPE_NUM_PORTS];
static int npe_used[NPE_NUM_PORTS];

/* A little extra so we can align to cacheline. */
static u8 npe_alloc_pool[NPE_MEM_POOL_SIZE + CFG_CACHELINE_SIZE - 1];
static u8 *npe_alloc_end;
static u8 *npe_alloc_free;

static void *npe_alloc(int size)
{
	static int count = 0;
	void *p = NULL;

	size = (size + (CFG_CACHELINE_SIZE-1)) & ~(CFG_CACHELINE_SIZE-1);
	count++;

	if ((npe_alloc_free + size) < npe_alloc_end) {
		p = npe_alloc_free;
		npe_alloc_free += size;
	} else {
		printf("%s: failed (count=%d, size=%d)!\n", count, size);
	}
	return p;
}

/* Not interrupt safe! */
static void mbuf_enqueue(IX_OSAL_MBUF **q, IX_OSAL_MBUF *new)
{
	IX_OSAL_MBUF *m = *q;

	IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR(new) = NULL;

	if (m) {
		while(IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR(m))
			m = IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR(m);
		IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR(m) = new;
	} else
		*q = new;
}

/* Not interrupt safe! */
static IX_OSAL_MBUF *mbuf_dequeue(IX_OSAL_MBUF **q)
{
	IX_OSAL_MBUF *m = *q;
	if (m)
		*q = IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR(m);
	return m;
}

static void reset_tx_mbufs(struct npe* p_npe)
{
	IX_OSAL_MBUF *m;
	int i;

	p_npe->txQHead = NULL;

	for (i = 0; i < CONFIG_DEVS_ETH_INTEL_NPE_MAX_TX_DESCRIPTORS; i++) {
		m = &p_npe->tx_mbufs[i];

		memset(m, 0, sizeof(*m));

		IX_OSAL_MBUF_MDATA(m) = (void *)&p_npe->tx_pkts[i * NPE_PKT_SIZE];
		IX_OSAL_MBUF_MLEN(m) = IX_OSAL_MBUF_PKT_LEN(m) = NPE_PKT_SIZE;
		mbuf_enqueue(&p_npe->txQHead, m);
	}
}

static void reset_rx_mbufs(struct npe* p_npe)
{
	IX_OSAL_MBUF *m;
	int i;

	p_npe->rxQHead = NULL;

	HAL_DCACHE_INVALIDATE(p_npe->rx_pkts, NPE_PKT_SIZE *
			      CONFIG_DEVS_ETH_INTEL_NPE_MAX_RX_DESCRIPTORS);

	for (i = 0; i < CONFIG_DEVS_ETH_INTEL_NPE_MAX_RX_DESCRIPTORS; i++) {
		m = &p_npe->rx_mbufs[i];

		memset(m, 0, sizeof(*m));

		IX_OSAL_MBUF_MDATA(m) = (void *)&p_npe->rx_pkts[i * NPE_PKT_SIZE];
		IX_OSAL_MBUF_MLEN(m) = IX_OSAL_MBUF_PKT_LEN(m) = NPE_PKT_SIZE;

		if(ixEthAccPortRxFreeReplenish(p_npe->eth_id, m) != IX_SUCCESS) {
			printf("ixEthAccPortRxFreeReplenish failed for port %d\n", p_npe->eth_id);
			break;
		}
	}
}

static void init_rx_mbufs(struct npe* p_npe)
{
	p_npe->rxQHead = NULL;

	p_npe->rx_pkts = npe_alloc(NPE_PKT_SIZE *
				   CONFIG_DEVS_ETH_INTEL_NPE_MAX_RX_DESCRIPTORS);
	if (p_npe->rx_pkts == NULL) {
		printf("alloc of packets failed.\n");
		return;
	}

	p_npe->rx_mbufs = (IX_OSAL_MBUF *)
		npe_alloc(sizeof(IX_OSAL_MBUF) *
			  CONFIG_DEVS_ETH_INTEL_NPE_MAX_RX_DESCRIPTORS);
	if (p_npe->rx_mbufs == NULL) {
		printf("alloc of mbufs failed.\n");
		return;
	}

	reset_rx_mbufs(p_npe);
}

static void init_tx_mbufs(struct npe* p_npe)
{
	p_npe->tx_pkts = npe_alloc(NPE_PKT_SIZE *
				   CONFIG_DEVS_ETH_INTEL_NPE_MAX_TX_DESCRIPTORS);
	if (p_npe->tx_pkts == NULL) {
		printf("alloc of packets failed.\n");
		return;
	}

	p_npe->tx_mbufs = (IX_OSAL_MBUF *)
		npe_alloc(sizeof(IX_OSAL_MBUF) *
			  CONFIG_DEVS_ETH_INTEL_NPE_MAX_TX_DESCRIPTORS);
	if (p_npe->tx_mbufs == NULL) {
		printf("alloc of mbufs failed.\n");
		return;
	}

	reset_tx_mbufs(p_npe);
}

/* Convert IX_ETH_PORT_n to IX_NPEMH_NPEID_NPEx */
static int __eth_to_npe(int eth_id)
{
	switch(eth_id) {
	case IX_ETH_PORT_1:
		return IX_NPEMH_NPEID_NPEB;

	case IX_ETH_PORT_2:
		return IX_NPEMH_NPEID_NPEC;

	case IX_ETH_PORT_3:
		return IX_NPEMH_NPEID_NPEA;
	}
	return 0;
}

/* Poll the CSR machinery. */
static void npe_poll(int eth_id)
{
	if (qDispatcherFunc != NULL) {
		ixNpeMhMessagesReceive(__eth_to_npe(eth_id));
		(*qDispatcherFunc)(IX_QMGR_QUELOW_GROUP);
	}
}

/* ethAcc RX callback */
static void npe_rx_callback(u32 cbTag, IX_OSAL_MBUF *m, IxEthAccPortId portid)
{
	struct npe* p_npe = (struct npe *)cbTag;

	if (IX_OSAL_MBUF_MLEN(m) > 0) {
		mbuf_enqueue(&p_npe->rxQHead, m);

		if (p_npe->rx_write == ((p_npe->rx_read-1) & (PKTBUFSRX-1))) {
			debug("Rx overflow: rx_write=%d rx_read=%d\n",
			      p_npe->rx_write, p_npe->rx_read);
		} else {
			debug("Received message #%d (len=%d)\n", p_npe->rx_write,
			      IX_OSAL_MBUF_MLEN(m));
			memcpy((void *)NetRxPackets[p_npe->rx_write], IX_OSAL_MBUF_MDATA(m),
			       IX_OSAL_MBUF_MLEN(m));
			p_npe->rx_len[p_npe->rx_write] = IX_OSAL_MBUF_MLEN(m);
			p_npe->rx_write++;
			if (p_npe->rx_write == PKTBUFSRX)
				p_npe->rx_write = 0;

#ifdef CONFIG_PRINT_RX_FRAMES
			{
				u8 *ptr = IX_OSAL_MBUF_MDATA(m);
				int i;

				for (i=0; i<60; i++) {
					debug("%02x ", *ptr++);
				}
				debug("\n");
			}
#endif
		}

		m = mbuf_dequeue(&p_npe->rxQHead);
	} else {
		debug("Received frame with length 0!!!\n");
		m = mbuf_dequeue(&p_npe->rxQHead);
	}

	/* Now return mbuf to NPE */
	IX_OSAL_MBUF_MLEN(m) = IX_OSAL_MBUF_PKT_LEN(m) = NPE_PKT_SIZE;
	IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(m) = NULL;
	IX_OSAL_MBUF_FLAGS(m) = 0;

	if(ixEthAccPortRxFreeReplenish(p_npe->eth_id, m) != IX_SUCCESS) {
		debug("npe_rx_callback: Error returning mbuf.\n");
	}
}

/* ethAcc TX callback */
static void npe_tx_callback(u32 cbTag, IX_OSAL_MBUF *m)
{
	struct npe* p_npe = (struct npe *)cbTag;

	debug("%s\n", __FUNCTION__);

	IX_OSAL_MBUF_MLEN(m) = IX_OSAL_MBUF_PKT_LEN(m) = NPE_PKT_SIZE;
	IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(m) = NULL;
	IX_OSAL_MBUF_FLAGS(m) = 0;

	mbuf_enqueue(&p_npe->txQHead, m);
}


static int npe_set_mac_address(struct eth_device *dev)
{
	struct npe *p_npe = (struct npe *)dev->priv;
	IxEthAccMacAddr npeMac;

	debug("%s\n", __FUNCTION__);

	/* Set MAC address */
	memcpy(npeMac.macAddress, dev->enetaddr, 6);

	if (ixEthAccPortUnicastMacAddressSet(p_npe->eth_id, &npeMac) != IX_ETH_ACC_SUCCESS) {
		printf("Error setting unicast address! %02x:%02x:%02x:%02x:%02x:%02x\n",
		       npeMac.macAddress[0], npeMac.macAddress[1],
		       npeMac.macAddress[2], npeMac.macAddress[3],
		       npeMac.macAddress[4], npeMac.macAddress[5]);
		return 0;
	}

	return 1;
}

/* Boot-time CSR library initialization. */
static int npe_csr_load(void)
{
	int i;

	if (ixQMgrInit() != IX_SUCCESS) {
		debug("Error initialising queue manager!\n");
		return 0;
	}

	ixQMgrDispatcherLoopGet(&qDispatcherFunc);

	if(ixNpeMhInitialize(IX_NPEMH_NPEINTERRUPTS_YES) != IX_SUCCESS) {
		printf("Error initialising NPE Message handler!\n");
		return 0;
	}

	if (npe_used[IX_ETH_PORT_1] && npe_exists[IX_ETH_PORT_1] &&
	    ixNpeDlNpeInitAndStart(IX_NPEDL_NPEIMAGE_NPEB_ETH_LEARN_FILTER_SPAN_FIREWALL_VLAN_QOS)
	    != IX_SUCCESS) {
		printf("Error downloading firmware to NPE-B!\n");
		return 0;
	}

	if (npe_used[IX_ETH_PORT_2] && npe_exists[IX_ETH_PORT_2] &&
	    ixNpeDlNpeInitAndStart(IX_NPEDL_NPEIMAGE_NPEC_ETH_LEARN_FILTER_SPAN_FIREWALL_VLAN_QOS)
	    != IX_SUCCESS) {
		printf("Error downloading firmware to NPE-C!\n");
		return 0;
	}

	/* don't need this for U-Boot */
	ixFeatureCtrlSwConfigurationWrite(IX_FEATURECTRL_ETH_LEARNING, FALSE);

	if (ixEthAccInit() != IX_ETH_ACC_SUCCESS) {
		printf("Error initialising Ethernet access driver!\n");
		return 0;
	}

	for (i = 0; i < IX_ETH_ACC_NUMBER_OF_PORTS; i++) {
		if (!npe_used[i] || !npe_exists[i])
			continue;
		if (ixEthAccPortInit(i) != IX_ETH_ACC_SUCCESS) {
			printf("Error initialising Ethernet port%d!\n", i);
		}
		if (ixEthAccTxSchedulingDisciplineSet(i, FIFO_NO_PRIORITY) != IX_ETH_ACC_SUCCESS) {
			printf("Error setting scheduling discipline for port %d.\n", i);
		}
		if (ixEthAccPortRxFrameAppendFCSDisable(i) != IX_ETH_ACC_SUCCESS) {
			printf("Error disabling RX FCS for port %d.\n", i);
		}
		if (ixEthAccPortTxFrameAppendFCSEnable(i) != IX_ETH_ACC_SUCCESS) {
			printf("Error enabling TX FCS for port %d.\n", i);
		}
	}

	return 1;
}

static int npe_init(struct eth_device *dev, bd_t * bis)
{
	struct npe *p_npe = (struct npe *)dev->priv;
	int i;
	u16 reg_short;
	int speed;
	int duplex;

	debug("%s: 1\n", __FUNCTION__);

	miiphy_read (dev->name, p_npe->phy_no, PHY_BMSR, &reg_short);

	/*
	 * Wait if PHY is capable of autonegotiation and autonegotiation is not complete
	 */
	if ((reg_short & PHY_BMSR_AUTN_ABLE) && !(reg_short & PHY_BMSR_AUTN_COMP)) {
		puts ("Waiting for PHY auto negotiation to complete");
		i = 0;
		while (!(reg_short & PHY_BMSR_AUTN_COMP)) {
			/*
			 * Timeout reached ?
			 */
			if (i > PHY_AUTONEGOTIATE_TIMEOUT) {
				puts (" TIMEOUT !\n");
				break;
			}

			if ((i++ % 1000) == 0) {
				putc ('.');
				miiphy_read (dev->name, p_npe->phy_no, PHY_BMSR, &reg_short);
			}
			udelay (1000);	/* 1 ms */
		}
		puts (" done\n");
		udelay (500000);	/* another 500 ms (results in faster booting) */
	}

	speed = miiphy_speed (dev->name, p_npe->phy_no);
	duplex = miiphy_duplex (dev->name, p_npe->phy_no);

	if (p_npe->print_speed) {
		p_npe->print_speed = 0;
		printf ("ENET Speed is %d Mbps - %s duplex connection\n",
			(int) speed, (duplex == HALF) ? "HALF" : "FULL");
	}

	npe_alloc_end = npe_alloc_pool + sizeof(npe_alloc_pool);
	npe_alloc_free = (u8 *)(((unsigned)npe_alloc_pool +
				 CFG_CACHELINE_SIZE - 1) & ~(CFG_CACHELINE_SIZE - 1));

	/* initialize mbuf pool */
	init_rx_mbufs(p_npe);
	init_tx_mbufs(p_npe);

	if (ixEthAccPortRxCallbackRegister(p_npe->eth_id, npe_rx_callback,
					   (u32)p_npe) != IX_ETH_ACC_SUCCESS) {
		printf("can't register RX callback!\n");
		return -1;
	}

	if (ixEthAccPortTxDoneCallbackRegister(p_npe->eth_id, npe_tx_callback,
					       (u32)p_npe) != IX_ETH_ACC_SUCCESS) {
		printf("can't register TX callback!\n");
		return -1;
	}

	npe_set_mac_address(dev);

	if (ixEthAccPortEnable(p_npe->eth_id) != IX_ETH_ACC_SUCCESS) {
		printf("can't enable port!\n");
		return -1;
	}

	p_npe->active = 1;

	return 0;
}

#if 0 /* test-only: probably have to deal with it when booting linux (for a clean state) */
/* Uninitialize CSR library. */
static void npe_csr_unload(void)
{
	ixEthAccUnload();
	ixEthDBUnload();
	ixNpeMhUnload();
	ixQMgrUnload();
}

/* callback which is used by ethAcc to recover RX buffers when stopping */
static void npe_rx_stop_callback(u32 cbTag, IX_OSAL_MBUF *m, IxEthAccPortId portid)
{
	debug("%s\n", __FUNCTION__);
}

/* callback which is used by ethAcc to recover TX buffers when stopping */
static void npe_tx_stop_callback(u32 cbTag, IX_OSAL_MBUF *m)
{
	debug("%s\n", __FUNCTION__);
}
#endif

static void npe_halt(struct eth_device *dev)
{
	struct npe *p_npe = (struct npe *)dev->priv;
	int i;

	debug("%s\n", __FUNCTION__);

	/* Delay to give time for recovery of mbufs */
	for (i = 0; i < 100; i++) {
		npe_poll(p_npe->eth_id);
		udelay(100);
	}

#if 0 /* test-only: probably have to deal with it when booting linux (for a clean state) */
	if (ixEthAccPortRxCallbackRegister(p_npe->eth_id, npe_rx_stop_callback,
					   (u32)p_npe) != IX_ETH_ACC_SUCCESS) {
		debug("Error registering rx callback!\n");
	}

	if (ixEthAccPortTxDoneCallbackRegister(p_npe->eth_id, npe_tx_stop_callback,
					       (u32)p_npe) != IX_ETH_ACC_SUCCESS) {
		debug("Error registering tx callback!\n");
	}

	if (ixEthAccPortDisable(p_npe->eth_id) != IX_ETH_ACC_SUCCESS) {
		debug("npe_stop: Error disabling NPEB!\n");
	}

	/* Delay to give time for recovery of mbufs */
	for (i = 0; i < 100; i++) {
		npe_poll(p_npe->eth_id);
		udelay(10000);
	}

	/*
	 * For U-Boot only, we are probably launching Linux or other OS that
	 * needs a clean slate for its NPE library.
	 */
#if 0 /* test-only */
	for (i = 0; i < IX_ETH_ACC_NUMBER_OF_PORTS; i++) {
		if (npe_used[i] && npe_exists[i])
			if (ixNpeDlNpeStopAndReset(__eth_to_npe(i)) != IX_SUCCESS)
				printf("Failed to stop and reset NPE B.\n");
	}
#endif

#endif
	p_npe->active = 0;
}


static int npe_send(struct eth_device *dev, volatile void *packet, int len)
{
	struct npe *p_npe = (struct npe *)dev->priv;
	u8 *dest;
	int err;
	IX_OSAL_MBUF *m;

	debug("%s\n", __FUNCTION__);
	m = mbuf_dequeue(&p_npe->txQHead);
	dest = IX_OSAL_MBUF_MDATA(m);
	IX_OSAL_MBUF_PKT_LEN(m) = IX_OSAL_MBUF_MLEN(m) = len;
	IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR(m) = NULL;

	memcpy(dest, (char *)packet, len);

	if ((err = ixEthAccPortTxFrameSubmit(p_npe->eth_id, m, IX_ETH_ACC_TX_DEFAULT_PRIORITY))
	    != IX_ETH_ACC_SUCCESS) {
		printf("npe_send: Can't submit frame. err[%d]\n", err);
		mbuf_enqueue(&p_npe->txQHead, m);
		return 0;
	}

#ifdef DEBUG_PRINT_TX_FRAMES
	{
		u8 *ptr = IX_OSAL_MBUF_MDATA(m);
		int i;

		for (i=0; i<IX_OSAL_MBUF_MLEN(m); i++) {
			printf("%02x ", *ptr++);
		}
		printf(" (tx-len=%d)\n", IX_OSAL_MBUF_MLEN(m));
	}
#endif

	npe_poll(p_npe->eth_id);

	return len;
}

static int npe_rx(struct eth_device *dev)
{
	struct npe *p_npe = (struct npe *)dev->priv;

	debug("%s\n", __FUNCTION__);
	npe_poll(p_npe->eth_id);

	debug("%s: rx_write=%d rx_read=%d\n", __FUNCTION__, p_npe->rx_write, p_npe->rx_read);
	while (p_npe->rx_write != p_npe->rx_read) {
		debug("Reading message #%d\n", p_npe->rx_read);
		NetReceive(NetRxPackets[p_npe->rx_read], p_npe->rx_len[p_npe->rx_read]);
		p_npe->rx_read++;
		if (p_npe->rx_read == PKTBUFSRX)
			p_npe->rx_read = 0;
	}

	return 0;
}

int npe_initialize(bd_t * bis)
{
	static int virgin = 0;
	struct eth_device *dev;
	int eth_num = 0;
	struct npe *p_npe = NULL;

	for (eth_num = 0; eth_num < CFG_NPE_NUMS; eth_num++) {

		/* See if we can actually bring up the interface, otherwise, skip it */
		switch (eth_num) {
		default:		/* fall through */
		case 0:
			if (memcmp (bis->bi_enetaddr, "\0\0\0\0\0\0", 6) == 0) {
				continue;
			}
			break;
#ifdef CONFIG_HAS_ETH1
		case 1:
			if (memcmp (bis->bi_enet1addr, "\0\0\0\0\0\0", 6) == 0) {
				continue;
			}
			break;
#endif
		}

		/* Allocate device structure */
		dev = (struct eth_device *)malloc(sizeof(*dev));
		if (dev == NULL) {
			printf ("%s: Cannot allocate eth_device %d\n", __FUNCTION__, eth_num);
			return -1;
		}
		memset(dev, 0, sizeof(*dev));

		/* Allocate our private use data */
		p_npe = (struct npe *)malloc(sizeof(struct npe));
		if (p_npe == NULL) {
			printf("%s: Cannot allocate private hw data for eth_device %d",
			       __FUNCTION__, eth_num);
			free(dev);
			return -1;
		}
		memset(p_npe, 0, sizeof(struct npe));

		switch (eth_num) {
		default:		/* fall through */
		case 0:
			memcpy(dev->enetaddr, bis->bi_enetaddr, 6);
			p_npe->eth_id = 0;
			p_npe->phy_no = CONFIG_PHY_ADDR;
			break;

#ifdef CONFIG_HAS_ETH1
		case 1:
			memcpy(dev->enetaddr, bis->bi_enet1addr, 6);
			p_npe->eth_id = 1;
			p_npe->phy_no = CONFIG_PHY1_ADDR;
			break;
#endif
		}

		sprintf(dev->name, "NPE%d", eth_num);
		dev->priv = (void *)p_npe;
		dev->init = npe_init;
		dev->halt = npe_halt;
		dev->send = npe_send;
		dev->recv = npe_rx;

		p_npe->print_speed = 1;

		if (0 == virgin) {
			virgin = 1;

			if (ixFeatureCtrlDeviceRead() == IX_FEATURE_CTRL_DEVICE_TYPE_IXP42X) {
				switch (ixFeatureCtrlProductIdRead() & IX_FEATURE_CTRL_SILICON_STEPPING_MASK) {
				case IX_FEATURE_CTRL_SILICON_TYPE_B0:
					/*
					 * If it is B0 Silicon, we only enable port when its corresponding
					 * Eth Coprocessor is available.
					 */
					if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_ETH0) ==
					    IX_FEATURE_CTRL_COMPONENT_ENABLED)
						npe_exists[IX_ETH_PORT_1] = TRUE;

					if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_ETH1) ==
					    IX_FEATURE_CTRL_COMPONENT_ENABLED)
						npe_exists[IX_ETH_PORT_2] = TRUE;
					break;
				case IX_FEATURE_CTRL_SILICON_TYPE_A0:
					/*
					 * If it is A0 Silicon, we enable both as both Eth Coprocessors
					 * are available.
					 */
					npe_exists[IX_ETH_PORT_1] = TRUE;
					npe_exists[IX_ETH_PORT_2] = TRUE;
					break;
				}
			} else if (ixFeatureCtrlDeviceRead() == IX_FEATURE_CTRL_DEVICE_TYPE_IXP46X) {
				if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_ETH0) ==
				    IX_FEATURE_CTRL_COMPONENT_ENABLED)
					npe_exists[IX_ETH_PORT_1] = TRUE;

				if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_ETH1) ==
				    IX_FEATURE_CTRL_COMPONENT_ENABLED)
					npe_exists[IX_ETH_PORT_2] = TRUE;
			}

			npe_used[IX_ETH_PORT_1] = 1;
			npe_used[IX_ETH_PORT_2] = 1;

			npe_alloc_end = npe_alloc_pool + sizeof(npe_alloc_pool);
			npe_alloc_free = (u8 *)(((unsigned)npe_alloc_pool +
						 CFG_CACHELINE_SIZE - 1)
						& ~(CFG_CACHELINE_SIZE - 1));

			if (!npe_csr_load())
				return 0;
		}

		eth_register(dev);

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
		miiphy_register(dev->name, npe_miiphy_read, npe_miiphy_write);
#endif

	}			/* end for each supported device */

	return 1;
}

#endif /* CONFIG_IXP4XX_NPE */

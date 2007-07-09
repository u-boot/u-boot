/**************************************************************************
Inter Pro 1000 for ppcboot/das-u-boot
Drivers are port from Intel's Linux driver e1000-4.3.15
and from Etherboot pro 1000 driver by mrakes at vivato dot net
tested on both gig copper and gig fiber boards
***************************************************************************/
/*******************************************************************************


  Copyright(c) 1999 - 2002 Intel Corporation. All rights reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 59
  Temple Place - Suite 330, Boston, MA  02111-1307, USA.

  The full GNU General Public License is included in this distribution in the
  file called LICENSE.

  Contact Information:
  Linux NICS <linux.nics@intel.com>
  Intel Corporation, 5200 N.E. Elam Young Parkway, Hillsboro, OR 97124-6497

*******************************************************************************/
/*
 *  Copyright (C) Archway Digital Solutions.
 *
 *  written by Chrsitopher Li <cli at arcyway dot com> or <chrisl at gnuchina dot org>
 *  2/9/2002
 *
 *  Copyright (C) Linux Networx.
 *  Massive upgrade to work with the new intel gigabit NICs.
 *  <ebiederman at lnxi dot com>
 */

#include "e1000.h"

#if defined(CONFIG_CMD_NET) \
	&& defined(CONFIG_NET_MULTI) && defined(CONFIG_E1000)

#define TOUT_LOOP   100000

#undef	virt_to_bus
#define	virt_to_bus(x)	((unsigned long)x)
#define bus_to_phys(devno, a)	pci_mem_to_phys(devno, a)
#define mdelay(n)       udelay((n)*1000)

#define E1000_DEFAULT_PBA    0x00000030

/* NIC specific static variables go here */

static char tx_pool[128 + 16];
static char rx_pool[128 + 16];
static char packet[2096];

static struct e1000_tx_desc *tx_base;
static struct e1000_rx_desc *rx_base;

static int tx_tail;
static int rx_tail, rx_last;

static struct pci_device_id supported[] = {
	{PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82542},
	{PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82543GC_FIBER},
	{PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82543GC_COPPER},
	{PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82544EI_COPPER},
	{PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82544EI_FIBER},
	{PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82544GC_COPPER},
	{PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82544GC_LOM},
	{PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82540EM},
	{PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82545EM_COPPER},
	{PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82546EB_COPPER},
	{PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82545EM_FIBER},
	{PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82546EB_FIBER},
	{PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82540EM_LOM},
};

/* Function forward declarations */
static int e1000_setup_link(struct eth_device *nic);
static int e1000_setup_fiber_link(struct eth_device *nic);
static int e1000_setup_copper_link(struct eth_device *nic);
static int e1000_phy_setup_autoneg(struct e1000_hw *hw);
static void e1000_config_collision_dist(struct e1000_hw *hw);
static int e1000_config_mac_to_phy(struct e1000_hw *hw);
static int e1000_config_fc_after_link_up(struct e1000_hw *hw);
static int e1000_check_for_link(struct eth_device *nic);
static int e1000_wait_autoneg(struct e1000_hw *hw);
static void e1000_get_speed_and_duplex(struct e1000_hw *hw, uint16_t * speed,
				       uint16_t * duplex);
static int e1000_read_phy_reg(struct e1000_hw *hw, uint32_t reg_addr,
			      uint16_t * phy_data);
static int e1000_write_phy_reg(struct e1000_hw *hw, uint32_t reg_addr,
			       uint16_t phy_data);
static void e1000_phy_hw_reset(struct e1000_hw *hw);
static int e1000_phy_reset(struct e1000_hw *hw);
static int e1000_detect_gig_phy(struct e1000_hw *hw);

#define E1000_WRITE_REG(a, reg, value) (writel((value), ((a)->hw_addr + E1000_##reg)))
#define E1000_READ_REG(a, reg) (readl((a)->hw_addr + E1000_##reg))
#define E1000_WRITE_REG_ARRAY(a, reg, offset, value) (\
			writel((value), ((a)->hw_addr + E1000_##reg + ((offset) << 2))))
#define E1000_READ_REG_ARRAY(a, reg, offset) ( \
	readl((a)->hw_addr + E1000_##reg + ((offset) << 2)))
#define E1000_WRITE_FLUSH(a) {uint32_t x; x = E1000_READ_REG(a, STATUS);}

#ifndef CONFIG_AP1000 /* remove for warnings */
/******************************************************************************
 * Raises the EEPROM's clock input.
 *
 * hw - Struct containing variables accessed by shared code
 * eecd - EECD's current value
 *****************************************************************************/
static void
e1000_raise_ee_clk(struct e1000_hw *hw, uint32_t * eecd)
{
	/* Raise the clock input to the EEPROM (by setting the SK bit), and then
	 * wait 50 microseconds.
	 */
	*eecd = *eecd | E1000_EECD_SK;
	E1000_WRITE_REG(hw, EECD, *eecd);
	E1000_WRITE_FLUSH(hw);
	udelay(50);
}

/******************************************************************************
 * Lowers the EEPROM's clock input.
 *
 * hw - Struct containing variables accessed by shared code
 * eecd - EECD's current value
 *****************************************************************************/
static void
e1000_lower_ee_clk(struct e1000_hw *hw, uint32_t * eecd)
{
	/* Lower the clock input to the EEPROM (by clearing the SK bit), and then
	 * wait 50 microseconds.
	 */
	*eecd = *eecd & ~E1000_EECD_SK;
	E1000_WRITE_REG(hw, EECD, *eecd);
	E1000_WRITE_FLUSH(hw);
	udelay(50);
}

/******************************************************************************
 * Shift data bits out to the EEPROM.
 *
 * hw - Struct containing variables accessed by shared code
 * data - data to send to the EEPROM
 * count - number of bits to shift out
 *****************************************************************************/
static void
e1000_shift_out_ee_bits(struct e1000_hw *hw, uint16_t data, uint16_t count)
{
	uint32_t eecd;
	uint32_t mask;

	/* We need to shift "count" bits out to the EEPROM. So, value in the
	 * "data" parameter will be shifted out to the EEPROM one bit at a time.
	 * In order to do this, "data" must be broken down into bits.
	 */
	mask = 0x01 << (count - 1);
	eecd = E1000_READ_REG(hw, EECD);
	eecd &= ~(E1000_EECD_DO | E1000_EECD_DI);
	do {
		/* A "1" is shifted out to the EEPROM by setting bit "DI" to a "1",
		 * and then raising and then lowering the clock (the SK bit controls
		 * the clock input to the EEPROM).  A "0" is shifted out to the EEPROM
		 * by setting "DI" to "0" and then raising and then lowering the clock.
		 */
		eecd &= ~E1000_EECD_DI;

		if (data & mask)
			eecd |= E1000_EECD_DI;

		E1000_WRITE_REG(hw, EECD, eecd);
		E1000_WRITE_FLUSH(hw);

		udelay(50);

		e1000_raise_ee_clk(hw, &eecd);
		e1000_lower_ee_clk(hw, &eecd);

		mask = mask >> 1;

	} while (mask);

	/* We leave the "DI" bit set to "0" when we leave this routine. */
	eecd &= ~E1000_EECD_DI;
	E1000_WRITE_REG(hw, EECD, eecd);
}

/******************************************************************************
 * Shift data bits in from the EEPROM
 *
 * hw - Struct containing variables accessed by shared code
 *****************************************************************************/
static uint16_t
e1000_shift_in_ee_bits(struct e1000_hw *hw)
{
	uint32_t eecd;
	uint32_t i;
	uint16_t data;

	/* In order to read a register from the EEPROM, we need to shift 16 bits
	 * in from the EEPROM. Bits are "shifted in" by raising the clock input to
	 * the EEPROM (setting the SK bit), and then reading the value of the "DO"
	 * bit.  During this "shifting in" process the "DI" bit should always be
	 * clear..
	 */

	eecd = E1000_READ_REG(hw, EECD);

	eecd &= ~(E1000_EECD_DO | E1000_EECD_DI);
	data = 0;

	for (i = 0; i < 16; i++) {
		data = data << 1;
		e1000_raise_ee_clk(hw, &eecd);

		eecd = E1000_READ_REG(hw, EECD);

		eecd &= ~(E1000_EECD_DI);
		if (eecd & E1000_EECD_DO)
			data |= 1;

		e1000_lower_ee_clk(hw, &eecd);
	}

	return data;
}

/******************************************************************************
 * Prepares EEPROM for access
 *
 * hw - Struct containing variables accessed by shared code
 *
 * Lowers EEPROM clock. Clears input pin. Sets the chip select pin. This
 * function should be called before issuing a command to the EEPROM.
 *****************************************************************************/
static void
e1000_setup_eeprom(struct e1000_hw *hw)
{
	uint32_t eecd;

	eecd = E1000_READ_REG(hw, EECD);

	/* Clear SK and DI */
	eecd &= ~(E1000_EECD_SK | E1000_EECD_DI);
	E1000_WRITE_REG(hw, EECD, eecd);

	/* Set CS */
	eecd |= E1000_EECD_CS;
	E1000_WRITE_REG(hw, EECD, eecd);
}

/******************************************************************************
 * Returns EEPROM to a "standby" state
 *
 * hw - Struct containing variables accessed by shared code
 *****************************************************************************/
static void
e1000_standby_eeprom(struct e1000_hw *hw)
{
	uint32_t eecd;

	eecd = E1000_READ_REG(hw, EECD);

	/* Deselct EEPROM */
	eecd &= ~(E1000_EECD_CS | E1000_EECD_SK);
	E1000_WRITE_REG(hw, EECD, eecd);
	E1000_WRITE_FLUSH(hw);
	udelay(50);

	/* Clock high */
	eecd |= E1000_EECD_SK;
	E1000_WRITE_REG(hw, EECD, eecd);
	E1000_WRITE_FLUSH(hw);
	udelay(50);

	/* Select EEPROM */
	eecd |= E1000_EECD_CS;
	E1000_WRITE_REG(hw, EECD, eecd);
	E1000_WRITE_FLUSH(hw);
	udelay(50);

	/* Clock low */
	eecd &= ~E1000_EECD_SK;
	E1000_WRITE_REG(hw, EECD, eecd);
	E1000_WRITE_FLUSH(hw);
	udelay(50);
}

/******************************************************************************
 * Reads a 16 bit word from the EEPROM.
 *
 * hw - Struct containing variables accessed by shared code
 * offset - offset of  word in the EEPROM to read
 * data - word read from the EEPROM
 *****************************************************************************/
static int
e1000_read_eeprom(struct e1000_hw *hw, uint16_t offset, uint16_t * data)
{
	uint32_t eecd;
	uint32_t i = 0;
	int large_eeprom = FALSE;

	/* Request EEPROM Access */
	if (hw->mac_type > e1000_82544) {
		eecd = E1000_READ_REG(hw, EECD);
		if (eecd & E1000_EECD_SIZE)
			large_eeprom = TRUE;
		eecd |= E1000_EECD_REQ;
		E1000_WRITE_REG(hw, EECD, eecd);
		eecd = E1000_READ_REG(hw, EECD);
		while ((!(eecd & E1000_EECD_GNT)) && (i < 100)) {
			i++;
			udelay(10);
			eecd = E1000_READ_REG(hw, EECD);
		}
		if (!(eecd & E1000_EECD_GNT)) {
			eecd &= ~E1000_EECD_REQ;
			E1000_WRITE_REG(hw, EECD, eecd);
			DEBUGOUT("Could not acquire EEPROM grant\n");
			return -E1000_ERR_EEPROM;
		}
	}

	/*  Prepare the EEPROM for reading  */
	e1000_setup_eeprom(hw);

	/*  Send the READ command (opcode + addr)  */
	e1000_shift_out_ee_bits(hw, EEPROM_READ_OPCODE, 3);
	e1000_shift_out_ee_bits(hw, offset, (large_eeprom) ? 8 : 6);

	/* Read the data */
	*data = e1000_shift_in_ee_bits(hw);

	/* End this read operation */
	e1000_standby_eeprom(hw);

	/* Stop requesting EEPROM access */
	if (hw->mac_type > e1000_82544) {
		eecd = E1000_READ_REG(hw, EECD);
		eecd &= ~E1000_EECD_REQ;
		E1000_WRITE_REG(hw, EECD, eecd);
	}

	return 0;
}

#if 0
static void
e1000_eeprom_cleanup(struct e1000_hw *hw)
{
	uint32_t eecd;

	eecd = E1000_READ_REG(hw, EECD);
	eecd &= ~(E1000_EECD_CS | E1000_EECD_DI);
	E1000_WRITE_REG(hw, EECD, eecd);
	e1000_raise_ee_clk(hw, &eecd);
	e1000_lower_ee_clk(hw, &eecd);
}

static uint16_t
e1000_wait_eeprom_done(struct e1000_hw *hw)
{
	uint32_t eecd;
	uint32_t i;

	e1000_standby_eeprom(hw);
	for (i = 0; i < 200; i++) {
		eecd = E1000_READ_REG(hw, EECD);
		if (eecd & E1000_EECD_DO)
			return (TRUE);
		udelay(5);
	}
	return (FALSE);
}

static int
e1000_write_eeprom(struct e1000_hw *hw, uint16_t Reg, uint16_t Data)
{
	uint32_t eecd;
	int large_eeprom = FALSE;
	int i = 0;

	/* Request EEPROM Access */
	if (hw->mac_type > e1000_82544) {
		eecd = E1000_READ_REG(hw, EECD);
		if (eecd & E1000_EECD_SIZE)
			large_eeprom = TRUE;
		eecd |= E1000_EECD_REQ;
		E1000_WRITE_REG(hw, EECD, eecd);
		eecd = E1000_READ_REG(hw, EECD);
		while ((!(eecd & E1000_EECD_GNT)) && (i < 100)) {
			i++;
			udelay(5);
			eecd = E1000_READ_REG(hw, EECD);
		}
		if (!(eecd & E1000_EECD_GNT)) {
			eecd &= ~E1000_EECD_REQ;
			E1000_WRITE_REG(hw, EECD, eecd);
			DEBUGOUT("Could not acquire EEPROM grant\n");
			return FALSE;
		}
	}
	e1000_setup_eeprom(hw);
	e1000_shift_out_ee_bits(hw, EEPROM_EWEN_OPCODE, 5);
	e1000_shift_out_ee_bits(hw, Reg, (large_eeprom) ? 6 : 4);
	e1000_standby_eeprom(hw);
	e1000_shift_out_ee_bits(hw, EEPROM_WRITE_OPCODE, 3);
	e1000_shift_out_ee_bits(hw, Reg, (large_eeprom) ? 8 : 6);
	e1000_shift_out_ee_bits(hw, Data, 16);
	if (!e1000_wait_eeprom_done(hw)) {
		return FALSE;
	}
	e1000_shift_out_ee_bits(hw, EEPROM_EWDS_OPCODE, 5);
	e1000_shift_out_ee_bits(hw, Reg, (large_eeprom) ? 6 : 4);
	e1000_eeprom_cleanup(hw);

	/* Stop requesting EEPROM access */
	if (hw->mac_type > e1000_82544) {
		eecd = E1000_READ_REG(hw, EECD);
		eecd &= ~E1000_EECD_REQ;
		E1000_WRITE_REG(hw, EECD, eecd);
	}
	i = 0;
	eecd = E1000_READ_REG(hw, EECD);
	while (((eecd & E1000_EECD_GNT)) && (i < 500)) {
		i++;
		udelay(10);
		eecd = E1000_READ_REG(hw, EECD);
	}
	if ((eecd & E1000_EECD_GNT)) {
		DEBUGOUT("Could not release EEPROM grant\n");
	}
	return TRUE;
}
#endif

/******************************************************************************
 * Verifies that the EEPROM has a valid checksum
 *
 * hw - Struct containing variables accessed by shared code
 *
 * Reads the first 64 16 bit words of the EEPROM and sums the values read.
 * If the the sum of the 64 16 bit words is 0xBABA, the EEPROM's checksum is
 * valid.
 *****************************************************************************/
static int
e1000_validate_eeprom_checksum(struct eth_device *nic)
{
	struct e1000_hw *hw = nic->priv;
	uint16_t checksum = 0;
	uint16_t i, eeprom_data;

	DEBUGFUNC();

	for (i = 0; i < (EEPROM_CHECKSUM_REG + 1); i++) {
		if (e1000_read_eeprom(hw, i, &eeprom_data) < 0) {
			DEBUGOUT("EEPROM Read Error\n");
			return -E1000_ERR_EEPROM;
		}
		checksum += eeprom_data;
	}

	if (checksum == (uint16_t) EEPROM_SUM) {
		return 0;
	} else {
		DEBUGOUT("EEPROM Checksum Invalid\n");
		return -E1000_ERR_EEPROM;
	}
}
#endif /* #ifndef CONFIG_AP1000 */

/******************************************************************************
 * Reads the adapter's MAC address from the EEPROM and inverts the LSB for the
 * second function of dual function devices
 *
 * nic - Struct containing variables accessed by shared code
 *****************************************************************************/
static int
e1000_read_mac_addr(struct eth_device *nic)
{
#ifndef CONFIG_AP1000
	struct e1000_hw *hw = nic->priv;
	uint16_t offset;
	uint16_t eeprom_data;
	int i;

	DEBUGFUNC();

	for (i = 0; i < NODE_ADDRESS_SIZE; i += 2) {
		offset = i >> 1;
		if (e1000_read_eeprom(hw, offset, &eeprom_data) < 0) {
			DEBUGOUT("EEPROM Read Error\n");
			return -E1000_ERR_EEPROM;
		}
		nic->enetaddr[i] = eeprom_data & 0xff;
		nic->enetaddr[i + 1] = (eeprom_data >> 8) & 0xff;
	}
	if ((hw->mac_type == e1000_82546) &&
	    (E1000_READ_REG(hw, STATUS) & E1000_STATUS_FUNC_1)) {
		/* Invert the last bit if this is the second device */
		nic->enetaddr[5] += 1;
	}
#else
	/*
	 * The AP1000's e1000 has no eeprom; the MAC address is stored in the
	 * environment variables.  Currently this does not support the addition
	 * of a PMC e1000 card, which is certainly a possibility, so this should
	 * be updated to properly use the env variable only for the onboard e1000
	 */

	int ii;
	char *s, *e;

	DEBUGFUNC();

	s = getenv ("ethaddr");
	if (s == NULL){
		return -E1000_ERR_EEPROM;
	}
	else{
		for(ii = 0; ii < 6; ii++) {
			nic->enetaddr[ii] = s ? simple_strtoul (s, &e, 16) : 0;
			if (s){
				s = (*e) ? e + 1 : e;
			}
		}
	}
#endif
	return 0;
}

/******************************************************************************
 * Initializes receive address filters.
 *
 * hw - Struct containing variables accessed by shared code
 *
 * Places the MAC address in receive address register 0 and clears the rest
 * of the receive addresss registers. Clears the multicast table. Assumes
 * the receiver is in reset when the routine is called.
 *****************************************************************************/
static void
e1000_init_rx_addrs(struct eth_device *nic)
{
	struct e1000_hw *hw = nic->priv;
	uint32_t i;
	uint32_t addr_low;
	uint32_t addr_high;

	DEBUGFUNC();

	/* Setup the receive address. */
	DEBUGOUT("Programming MAC Address into RAR[0]\n");
	addr_low = (nic->enetaddr[0] |
		    (nic->enetaddr[1] << 8) |
		    (nic->enetaddr[2] << 16) | (nic->enetaddr[3] << 24));

	addr_high = (nic->enetaddr[4] | (nic->enetaddr[5] << 8) | E1000_RAH_AV);

	E1000_WRITE_REG_ARRAY(hw, RA, 0, addr_low);
	E1000_WRITE_REG_ARRAY(hw, RA, 1, addr_high);

	/* Zero out the other 15 receive addresses. */
	DEBUGOUT("Clearing RAR[1-15]\n");
	for (i = 1; i < E1000_RAR_ENTRIES; i++) {
		E1000_WRITE_REG_ARRAY(hw, RA, (i << 1), 0);
		E1000_WRITE_REG_ARRAY(hw, RA, ((i << 1) + 1), 0);
	}
}

/******************************************************************************
 * Clears the VLAN filer table
 *
 * hw - Struct containing variables accessed by shared code
 *****************************************************************************/
static void
e1000_clear_vfta(struct e1000_hw *hw)
{
	uint32_t offset;

	for (offset = 0; offset < E1000_VLAN_FILTER_TBL_SIZE; offset++)
		E1000_WRITE_REG_ARRAY(hw, VFTA, offset, 0);
}

/******************************************************************************
 * Set the mac type member in the hw struct.
 *
 * hw - Struct containing variables accessed by shared code
 *****************************************************************************/
static int
e1000_set_mac_type(struct e1000_hw *hw)
{
	DEBUGFUNC();

	switch (hw->device_id) {
	case E1000_DEV_ID_82542:
		switch (hw->revision_id) {
		case E1000_82542_2_0_REV_ID:
			hw->mac_type = e1000_82542_rev2_0;
			break;
		case E1000_82542_2_1_REV_ID:
			hw->mac_type = e1000_82542_rev2_1;
			break;
		default:
			/* Invalid 82542 revision ID */
			return -E1000_ERR_MAC_TYPE;
		}
		break;
	case E1000_DEV_ID_82543GC_FIBER:
	case E1000_DEV_ID_82543GC_COPPER:
		hw->mac_type = e1000_82543;
		break;
	case E1000_DEV_ID_82544EI_COPPER:
	case E1000_DEV_ID_82544EI_FIBER:
	case E1000_DEV_ID_82544GC_COPPER:
	case E1000_DEV_ID_82544GC_LOM:
		hw->mac_type = e1000_82544;
		break;
	case E1000_DEV_ID_82540EM:
	case E1000_DEV_ID_82540EM_LOM:
		hw->mac_type = e1000_82540;
		break;
	case E1000_DEV_ID_82545EM_COPPER:
	case E1000_DEV_ID_82545EM_FIBER:
		hw->mac_type = e1000_82545;
		break;
	case E1000_DEV_ID_82546EB_COPPER:
	case E1000_DEV_ID_82546EB_FIBER:
		hw->mac_type = e1000_82546;
		break;
	default:
		/* Should never have loaded on this device */
		return -E1000_ERR_MAC_TYPE;
	}
	return E1000_SUCCESS;
}

/******************************************************************************
 * Reset the transmit and receive units; mask and clear all interrupts.
 *
 * hw - Struct containing variables accessed by shared code
 *****************************************************************************/
void
e1000_reset_hw(struct e1000_hw *hw)
{
	uint32_t ctrl;
	uint32_t ctrl_ext;
	uint32_t icr;
	uint32_t manc;

	DEBUGFUNC();

	/* For 82542 (rev 2.0), disable MWI before issuing a device reset */
	if (hw->mac_type == e1000_82542_rev2_0) {
		DEBUGOUT("Disabling MWI on 82542 rev 2.0\n");
		pci_write_config_word(hw->pdev, PCI_COMMAND,
				      hw->
				      pci_cmd_word & ~PCI_COMMAND_INVALIDATE);
	}

	/* Clear interrupt mask to stop board from generating interrupts */
	DEBUGOUT("Masking off all interrupts\n");
	E1000_WRITE_REG(hw, IMC, 0xffffffff);

	/* Disable the Transmit and Receive units.  Then delay to allow
	 * any pending transactions to complete before we hit the MAC with
	 * the global reset.
	 */
	E1000_WRITE_REG(hw, RCTL, 0);
	E1000_WRITE_REG(hw, TCTL, E1000_TCTL_PSP);
	E1000_WRITE_FLUSH(hw);

	/* The tbi_compatibility_on Flag must be cleared when Rctl is cleared. */
	hw->tbi_compatibility_on = FALSE;

	/* Delay to allow any outstanding PCI transactions to complete before
	 * resetting the device
	 */
	mdelay(10);

	/* Issue a global reset to the MAC.  This will reset the chip's
	 * transmit, receive, DMA, and link units.  It will not effect
	 * the current PCI configuration.  The global reset bit is self-
	 * clearing, and should clear within a microsecond.
	 */
	DEBUGOUT("Issuing a global reset to MAC\n");
	ctrl = E1000_READ_REG(hw, CTRL);

#if 0
	if (hw->mac_type > e1000_82543)
		E1000_WRITE_REG_IO(hw, CTRL, (ctrl | E1000_CTRL_RST));
	else
#endif
		E1000_WRITE_REG(hw, CTRL, (ctrl | E1000_CTRL_RST));

	/* Force a reload from the EEPROM if necessary */
	if (hw->mac_type < e1000_82540) {
		/* Wait for reset to complete */
		udelay(10);
		ctrl_ext = E1000_READ_REG(hw, CTRL_EXT);
		ctrl_ext |= E1000_CTRL_EXT_EE_RST;
		E1000_WRITE_REG(hw, CTRL_EXT, ctrl_ext);
		E1000_WRITE_FLUSH(hw);
		/* Wait for EEPROM reload */
		mdelay(2);
	} else {
		/* Wait for EEPROM reload (it happens automatically) */
		mdelay(4);
		/* Dissable HW ARPs on ASF enabled adapters */
		manc = E1000_READ_REG(hw, MANC);
		manc &= ~(E1000_MANC_ARP_EN);
		E1000_WRITE_REG(hw, MANC, manc);
	}

	/* Clear interrupt mask to stop board from generating interrupts */
	DEBUGOUT("Masking off all interrupts\n");
	E1000_WRITE_REG(hw, IMC, 0xffffffff);

	/* Clear any pending interrupt events. */
	icr = E1000_READ_REG(hw, ICR);

	/* If MWI was previously enabled, reenable it. */
	if (hw->mac_type == e1000_82542_rev2_0) {
		pci_write_config_word(hw->pdev, PCI_COMMAND, hw->pci_cmd_word);
	}
}

/******************************************************************************
 * Performs basic configuration of the adapter.
 *
 * hw - Struct containing variables accessed by shared code
 *
 * Assumes that the controller has previously been reset and is in a
 * post-reset uninitialized state. Initializes the receive address registers,
 * multicast table, and VLAN filter table. Calls routines to setup link
 * configuration and flow control settings. Clears all on-chip counters. Leaves
 * the transmit and receive units disabled and uninitialized.
 *****************************************************************************/
static int
e1000_init_hw(struct eth_device *nic)
{
	struct e1000_hw *hw = nic->priv;
	uint32_t ctrl, status;
	uint32_t i;
	int32_t ret_val;
	uint16_t pcix_cmd_word;
	uint16_t pcix_stat_hi_word;
	uint16_t cmd_mmrbc;
	uint16_t stat_mmrbc;
	e1000_bus_type bus_type = e1000_bus_type_unknown;

	DEBUGFUNC();
#if 0
	/* Initialize Identification LED */
	ret_val = e1000_id_led_init(hw);
	if (ret_val < 0) {
		DEBUGOUT("Error Initializing Identification LED\n");
		return ret_val;
	}
#endif
	/* Set the Media Type and exit with error if it is not valid. */
	if (hw->mac_type != e1000_82543) {
		/* tbi_compatibility is only valid on 82543 */
		hw->tbi_compatibility_en = FALSE;
	}

	if (hw->mac_type >= e1000_82543) {
		status = E1000_READ_REG(hw, STATUS);
		if (status & E1000_STATUS_TBIMODE) {
			hw->media_type = e1000_media_type_fiber;
			/* tbi_compatibility not valid on fiber */
			hw->tbi_compatibility_en = FALSE;
		} else {
			hw->media_type = e1000_media_type_copper;
		}
	} else {
		/* This is an 82542 (fiber only) */
		hw->media_type = e1000_media_type_fiber;
	}

	/* Disabling VLAN filtering. */
	DEBUGOUT("Initializing the IEEE VLAN\n");
	E1000_WRITE_REG(hw, VET, 0);

	e1000_clear_vfta(hw);

	/* For 82542 (rev 2.0), disable MWI and put the receiver into reset */
	if (hw->mac_type == e1000_82542_rev2_0) {
		DEBUGOUT("Disabling MWI on 82542 rev 2.0\n");
		pci_write_config_word(hw->pdev, PCI_COMMAND,
				      hw->
				      pci_cmd_word & ~PCI_COMMAND_INVALIDATE);
		E1000_WRITE_REG(hw, RCTL, E1000_RCTL_RST);
		E1000_WRITE_FLUSH(hw);
		mdelay(5);
	}

	/* Setup the receive address. This involves initializing all of the Receive
	 * Address Registers (RARs 0 - 15).
	 */
	e1000_init_rx_addrs(nic);

	/* For 82542 (rev 2.0), take the receiver out of reset and enable MWI */
	if (hw->mac_type == e1000_82542_rev2_0) {
		E1000_WRITE_REG(hw, RCTL, 0);
		E1000_WRITE_FLUSH(hw);
		mdelay(1);
		pci_write_config_word(hw->pdev, PCI_COMMAND, hw->pci_cmd_word);
	}

	/* Zero out the Multicast HASH table */
	DEBUGOUT("Zeroing the MTA\n");
	for (i = 0; i < E1000_MC_TBL_SIZE; i++)
		E1000_WRITE_REG_ARRAY(hw, MTA, i, 0);

#if 0
	/* Set the PCI priority bit correctly in the CTRL register.  This
	 * determines if the adapter gives priority to receives, or if it
	 * gives equal priority to transmits and receives.
	 */
	if (hw->dma_fairness) {
		ctrl = E1000_READ_REG(hw, CTRL);
		E1000_WRITE_REG(hw, CTRL, ctrl | E1000_CTRL_PRIOR);
	}
#endif
	if (hw->mac_type >= e1000_82543) {
		status = E1000_READ_REG(hw, STATUS);
		bus_type = (status & E1000_STATUS_PCIX_MODE) ?
		    e1000_bus_type_pcix : e1000_bus_type_pci;
	}
	/* Workaround for PCI-X problem when BIOS sets MMRBC incorrectly. */
	if (bus_type == e1000_bus_type_pcix) {
		pci_read_config_word(hw->pdev, PCIX_COMMAND_REGISTER,
				     &pcix_cmd_word);
		pci_read_config_word(hw->pdev, PCIX_STATUS_REGISTER_HI,
				     &pcix_stat_hi_word);
		cmd_mmrbc =
		    (pcix_cmd_word & PCIX_COMMAND_MMRBC_MASK) >>
		    PCIX_COMMAND_MMRBC_SHIFT;
		stat_mmrbc =
		    (pcix_stat_hi_word & PCIX_STATUS_HI_MMRBC_MASK) >>
		    PCIX_STATUS_HI_MMRBC_SHIFT;
		if (stat_mmrbc == PCIX_STATUS_HI_MMRBC_4K)
			stat_mmrbc = PCIX_STATUS_HI_MMRBC_2K;
		if (cmd_mmrbc > stat_mmrbc) {
			pcix_cmd_word &= ~PCIX_COMMAND_MMRBC_MASK;
			pcix_cmd_word |= stat_mmrbc << PCIX_COMMAND_MMRBC_SHIFT;
			pci_write_config_word(hw->pdev, PCIX_COMMAND_REGISTER,
					      pcix_cmd_word);
		}
	}

	/* Call a subroutine to configure the link and setup flow control. */
	ret_val = e1000_setup_link(nic);

	/* Set the transmit descriptor write-back policy */
	if (hw->mac_type > e1000_82544) {
		ctrl = E1000_READ_REG(hw, TXDCTL);
		ctrl =
		    (ctrl & ~E1000_TXDCTL_WTHRESH) |
		    E1000_TXDCTL_FULL_TX_DESC_WB;
		E1000_WRITE_REG(hw, TXDCTL, ctrl);
	}
#if 0
	/* Clear all of the statistics registers (clear on read).  It is
	 * important that we do this after we have tried to establish link
	 * because the symbol error count will increment wildly if there
	 * is no link.
	 */
	e1000_clear_hw_cntrs(hw);
#endif

	return ret_val;
}

/******************************************************************************
 * Configures flow control and link settings.
 *
 * hw - Struct containing variables accessed by shared code
 *
 * Determines which flow control settings to use. Calls the apropriate media-
 * specific link configuration function. Configures the flow control settings.
 * Assuming the adapter has a valid link partner, a valid link should be
 * established. Assumes the hardware has previously been reset and the
 * transmitter and receiver are not enabled.
 *****************************************************************************/
static int
e1000_setup_link(struct eth_device *nic)
{
	struct e1000_hw *hw = nic->priv;
	uint32_t ctrl_ext;
	int32_t ret_val;
	uint16_t eeprom_data;

	DEBUGFUNC();

#ifndef CONFIG_AP1000
	/* Read and store word 0x0F of the EEPROM. This word contains bits
	 * that determine the hardware's default PAUSE (flow control) mode,
	 * a bit that determines whether the HW defaults to enabling or
	 * disabling auto-negotiation, and the direction of the
	 * SW defined pins. If there is no SW over-ride of the flow
	 * control setting, then the variable hw->fc will
	 * be initialized based on a value in the EEPROM.
	 */
	if (e1000_read_eeprom(hw, EEPROM_INIT_CONTROL2_REG, &eeprom_data) < 0) {
		DEBUGOUT("EEPROM Read Error\n");
		return -E1000_ERR_EEPROM;
	}
#else
	/* we have to hardcode the proper value for our hardware. */
	/* this value is for the 82540EM pci card used for prototyping, and it works. */
	eeprom_data = 0xb220;
#endif

	if (hw->fc == e1000_fc_default) {
		if ((eeprom_data & EEPROM_WORD0F_PAUSE_MASK) == 0)
			hw->fc = e1000_fc_none;
		else if ((eeprom_data & EEPROM_WORD0F_PAUSE_MASK) ==
			 EEPROM_WORD0F_ASM_DIR)
			hw->fc = e1000_fc_tx_pause;
		else
			hw->fc = e1000_fc_full;
	}

	/* We want to save off the original Flow Control configuration just
	 * in case we get disconnected and then reconnected into a different
	 * hub or switch with different Flow Control capabilities.
	 */
	if (hw->mac_type == e1000_82542_rev2_0)
		hw->fc &= (~e1000_fc_tx_pause);

	if ((hw->mac_type < e1000_82543) && (hw->report_tx_early == 1))
		hw->fc &= (~e1000_fc_rx_pause);

	hw->original_fc = hw->fc;

	DEBUGOUT("After fix-ups FlowControl is now = %x\n", hw->fc);

	/* Take the 4 bits from EEPROM word 0x0F that determine the initial
	 * polarity value for the SW controlled pins, and setup the
	 * Extended Device Control reg with that info.
	 * This is needed because one of the SW controlled pins is used for
	 * signal detection.  So this should be done before e1000_setup_pcs_link()
	 * or e1000_phy_setup() is called.
	 */
	if (hw->mac_type == e1000_82543) {
		ctrl_ext = ((eeprom_data & EEPROM_WORD0F_SWPDIO_EXT) <<
			    SWDPIO__EXT_SHIFT);
		E1000_WRITE_REG(hw, CTRL_EXT, ctrl_ext);
	}

	/* Call the necessary subroutine to configure the link. */
	ret_val = (hw->media_type == e1000_media_type_fiber) ?
	    e1000_setup_fiber_link(nic) : e1000_setup_copper_link(nic);
	if (ret_val < 0) {
		return ret_val;
	}

	/* Initialize the flow control address, type, and PAUSE timer
	 * registers to their default values.  This is done even if flow
	 * control is disabled, because it does not hurt anything to
	 * initialize these registers.
	 */
	DEBUGOUT
	    ("Initializing the Flow Control address, type and timer regs\n");

	E1000_WRITE_REG(hw, FCAL, FLOW_CONTROL_ADDRESS_LOW);
	E1000_WRITE_REG(hw, FCAH, FLOW_CONTROL_ADDRESS_HIGH);
	E1000_WRITE_REG(hw, FCT, FLOW_CONTROL_TYPE);
	E1000_WRITE_REG(hw, FCTTV, hw->fc_pause_time);

	/* Set the flow control receive threshold registers.  Normally,
	 * these registers will be set to a default threshold that may be
	 * adjusted later by the driver's runtime code.  However, if the
	 * ability to transmit pause frames in not enabled, then these
	 * registers will be set to 0.
	 */
	if (!(hw->fc & e1000_fc_tx_pause)) {
		E1000_WRITE_REG(hw, FCRTL, 0);
		E1000_WRITE_REG(hw, FCRTH, 0);
	} else {
		/* We need to set up the Receive Threshold high and low water marks
		 * as well as (optionally) enabling the transmission of XON frames.
		 */
		if (hw->fc_send_xon) {
			E1000_WRITE_REG(hw, FCRTL,
					(hw->fc_low_water | E1000_FCRTL_XONE));
			E1000_WRITE_REG(hw, FCRTH, hw->fc_high_water);
		} else {
			E1000_WRITE_REG(hw, FCRTL, hw->fc_low_water);
			E1000_WRITE_REG(hw, FCRTH, hw->fc_high_water);
		}
	}
	return ret_val;
}

/******************************************************************************
 * Sets up link for a fiber based adapter
 *
 * hw - Struct containing variables accessed by shared code
 *
 * Manipulates Physical Coding Sublayer functions in order to configure
 * link. Assumes the hardware has been previously reset and the transmitter
 * and receiver are not enabled.
 *****************************************************************************/
static int
e1000_setup_fiber_link(struct eth_device *nic)
{
	struct e1000_hw *hw = nic->priv;
	uint32_t ctrl;
	uint32_t status;
	uint32_t txcw = 0;
	uint32_t i;
	uint32_t signal;
	int32_t ret_val;

	DEBUGFUNC();
	/* On adapters with a MAC newer that 82544, SW Defineable pin 1 will be
	 * set when the optics detect a signal. On older adapters, it will be
	 * cleared when there is a signal
	 */
	ctrl = E1000_READ_REG(hw, CTRL);
	if ((hw->mac_type > e1000_82544) && !(ctrl & E1000_CTRL_ILOS))
		signal = E1000_CTRL_SWDPIN1;
	else
		signal = 0;

	printf("signal for %s is %x (ctrl %08x)!!!!\n", nic->name, signal,
	       ctrl);
	/* Take the link out of reset */
	ctrl &= ~(E1000_CTRL_LRST);

	e1000_config_collision_dist(hw);

	/* Check for a software override of the flow control settings, and setup
	 * the device accordingly.  If auto-negotiation is enabled, then software
	 * will have to set the "PAUSE" bits to the correct value in the Tranmsit
	 * Config Word Register (TXCW) and re-start auto-negotiation.  However, if
	 * auto-negotiation is disabled, then software will have to manually
	 * configure the two flow control enable bits in the CTRL register.
	 *
	 * The possible values of the "fc" parameter are:
	 *      0:  Flow control is completely disabled
	 *      1:  Rx flow control is enabled (we can receive pause frames, but
	 *          not send pause frames).
	 *      2:  Tx flow control is enabled (we can send pause frames but we do
	 *          not support receiving pause frames).
	 *      3:  Both Rx and TX flow control (symmetric) are enabled.
	 */
	switch (hw->fc) {
	case e1000_fc_none:
		/* Flow control is completely disabled by a software over-ride. */
		txcw = (E1000_TXCW_ANE | E1000_TXCW_FD);
		break;
	case e1000_fc_rx_pause:
		/* RX Flow control is enabled and TX Flow control is disabled by a
		 * software over-ride. Since there really isn't a way to advertise
		 * that we are capable of RX Pause ONLY, we will advertise that we
		 * support both symmetric and asymmetric RX PAUSE. Later, we will
		 *  disable the adapter's ability to send PAUSE frames.
		 */
		txcw = (E1000_TXCW_ANE | E1000_TXCW_FD | E1000_TXCW_PAUSE_MASK);
		break;
	case e1000_fc_tx_pause:
		/* TX Flow control is enabled, and RX Flow control is disabled, by a
		 * software over-ride.
		 */
		txcw = (E1000_TXCW_ANE | E1000_TXCW_FD | E1000_TXCW_ASM_DIR);
		break;
	case e1000_fc_full:
		/* Flow control (both RX and TX) is enabled by a software over-ride. */
		txcw = (E1000_TXCW_ANE | E1000_TXCW_FD | E1000_TXCW_PAUSE_MASK);
		break;
	default:
		DEBUGOUT("Flow control param set incorrectly\n");
		return -E1000_ERR_CONFIG;
		break;
	}

	/* Since auto-negotiation is enabled, take the link out of reset (the link
	 * will be in reset, because we previously reset the chip). This will
	 * restart auto-negotiation.  If auto-neogtiation is successful then the
	 * link-up status bit will be set and the flow control enable bits (RFCE
	 * and TFCE) will be set according to their negotiated value.
	 */
	DEBUGOUT("Auto-negotiation enabled (%#x)\n", txcw);

	E1000_WRITE_REG(hw, TXCW, txcw);
	E1000_WRITE_REG(hw, CTRL, ctrl);
	E1000_WRITE_FLUSH(hw);

	hw->txcw = txcw;
	mdelay(1);

	/* If we have a signal (the cable is plugged in) then poll for a "Link-Up"
	 * indication in the Device Status Register.  Time-out if a link isn't
	 * seen in 500 milliseconds seconds (Auto-negotiation should complete in
	 * less than 500 milliseconds even if the other end is doing it in SW).
	 */
	if ((E1000_READ_REG(hw, CTRL) & E1000_CTRL_SWDPIN1) == signal) {
		DEBUGOUT("Looking for Link\n");
		for (i = 0; i < (LINK_UP_TIMEOUT / 10); i++) {
			mdelay(10);
			status = E1000_READ_REG(hw, STATUS);
			if (status & E1000_STATUS_LU)
				break;
		}
		if (i == (LINK_UP_TIMEOUT / 10)) {
			/* AutoNeg failed to achieve a link, so we'll call
			 * e1000_check_for_link. This routine will force the link up if we
			 * detect a signal. This will allow us to communicate with
			 * non-autonegotiating link partners.
			 */
			DEBUGOUT("Never got a valid link from auto-neg!!!\n");
			hw->autoneg_failed = 1;
			ret_val = e1000_check_for_link(nic);
			if (ret_val < 0) {
				DEBUGOUT("Error while checking for link\n");
				return ret_val;
			}
			hw->autoneg_failed = 0;
		} else {
			hw->autoneg_failed = 0;
			DEBUGOUT("Valid Link Found\n");
		}
	} else {
		DEBUGOUT("No Signal Detected\n");
		return -E1000_ERR_NOLINK;
	}
	return 0;
}

/******************************************************************************
* Detects which PHY is present and the speed and duplex
*
* hw - Struct containing variables accessed by shared code
******************************************************************************/
static int
e1000_setup_copper_link(struct eth_device *nic)
{
	struct e1000_hw *hw = nic->priv;
	uint32_t ctrl;
	int32_t ret_val;
	uint16_t i;
	uint16_t phy_data;

	DEBUGFUNC();

	ctrl = E1000_READ_REG(hw, CTRL);
	/* With 82543, we need to force speed and duplex on the MAC equal to what
	 * the PHY speed and duplex configuration is. In addition, we need to
	 * perform a hardware reset on the PHY to take it out of reset.
	 */
	if (hw->mac_type > e1000_82543) {
		ctrl |= E1000_CTRL_SLU;
		ctrl &= ~(E1000_CTRL_FRCSPD | E1000_CTRL_FRCDPX);
		E1000_WRITE_REG(hw, CTRL, ctrl);
	} else {
		ctrl |=
		    (E1000_CTRL_FRCSPD | E1000_CTRL_FRCDPX | E1000_CTRL_SLU);
		E1000_WRITE_REG(hw, CTRL, ctrl);
		e1000_phy_hw_reset(hw);
	}

	/* Make sure we have a valid PHY */
	ret_val = e1000_detect_gig_phy(hw);
	if (ret_val < 0) {
		DEBUGOUT("Error, did not detect valid phy.\n");
		return ret_val;
	}
	DEBUGOUT("Phy ID = %x \n", hw->phy_id);

	/* Enable CRS on TX. This must be set for half-duplex operation. */
	if (e1000_read_phy_reg(hw, M88E1000_PHY_SPEC_CTRL, &phy_data) < 0) {
		DEBUGOUT("PHY Read Error\n");
		return -E1000_ERR_PHY;
	}
	phy_data |= M88E1000_PSCR_ASSERT_CRS_ON_TX;

#if 0
	/* Options:
	 *   MDI/MDI-X = 0 (default)
	 *   0 - Auto for all speeds
	 *   1 - MDI mode
	 *   2 - MDI-X mode
	 *   3 - Auto for 1000Base-T only (MDI-X for 10/100Base-T modes)
	 */
	phy_data &= ~M88E1000_PSCR_AUTO_X_MODE;
	switch (hw->mdix) {
	case 1:
		phy_data |= M88E1000_PSCR_MDI_MANUAL_MODE;
		break;
	case 2:
		phy_data |= M88E1000_PSCR_MDIX_MANUAL_MODE;
		break;
	case 3:
		phy_data |= M88E1000_PSCR_AUTO_X_1000T;
		break;
	case 0:
	default:
		phy_data |= M88E1000_PSCR_AUTO_X_MODE;
		break;
	}
#else
	phy_data |= M88E1000_PSCR_AUTO_X_MODE;
#endif

#if 0
	/* Options:
	 *   disable_polarity_correction = 0 (default)
	 *       Automatic Correction for Reversed Cable Polarity
	 *   0 - Disabled
	 *   1 - Enabled
	 */
	phy_data &= ~M88E1000_PSCR_POLARITY_REVERSAL;
	if (hw->disable_polarity_correction == 1)
		phy_data |= M88E1000_PSCR_POLARITY_REVERSAL;
#else
	phy_data &= ~M88E1000_PSCR_POLARITY_REVERSAL;
#endif
	if (e1000_write_phy_reg(hw, M88E1000_PHY_SPEC_CTRL, phy_data) < 0) {
		DEBUGOUT("PHY Write Error\n");
		return -E1000_ERR_PHY;
	}

	/* Force TX_CLK in the Extended PHY Specific Control Register
	 * to 25MHz clock.
	 */
	if (e1000_read_phy_reg(hw, M88E1000_EXT_PHY_SPEC_CTRL, &phy_data) < 0) {
		DEBUGOUT("PHY Read Error\n");
		return -E1000_ERR_PHY;
	}
	phy_data |= M88E1000_EPSCR_TX_CLK_25;
	/* Configure Master and Slave downshift values */
	phy_data &= ~(M88E1000_EPSCR_MASTER_DOWNSHIFT_MASK |
		      M88E1000_EPSCR_SLAVE_DOWNSHIFT_MASK);
	phy_data |= (M88E1000_EPSCR_MASTER_DOWNSHIFT_1X |
		     M88E1000_EPSCR_SLAVE_DOWNSHIFT_1X);
	if (e1000_write_phy_reg(hw, M88E1000_EXT_PHY_SPEC_CTRL, phy_data) < 0) {
		DEBUGOUT("PHY Write Error\n");
		return -E1000_ERR_PHY;
	}

	/* SW Reset the PHY so all changes take effect */
	ret_val = e1000_phy_reset(hw);
	if (ret_val < 0) {
		DEBUGOUT("Error Resetting the PHY\n");
		return ret_val;
	}

	/* Options:
	 *   autoneg = 1 (default)
	 *      PHY will advertise value(s) parsed from
	 *      autoneg_advertised and fc
	 *   autoneg = 0
	 *      PHY will be set to 10H, 10F, 100H, or 100F
	 *      depending on value parsed from forced_speed_duplex.
	 */

	/* Is autoneg enabled?  This is enabled by default or by software override.
	 * If so, call e1000_phy_setup_autoneg routine to parse the
	 * autoneg_advertised and fc options. If autoneg is NOT enabled, then the
	 * user should have provided a speed/duplex override.  If so, then call
	 * e1000_phy_force_speed_duplex to parse and set this up.
	 */
	/* Perform some bounds checking on the hw->autoneg_advertised
	 * parameter.  If this variable is zero, then set it to the default.
	 */
	hw->autoneg_advertised &= AUTONEG_ADVERTISE_SPEED_DEFAULT;

	/* If autoneg_advertised is zero, we assume it was not defaulted
	 * by the calling code so we set to advertise full capability.
	 */
	if (hw->autoneg_advertised == 0)
		hw->autoneg_advertised = AUTONEG_ADVERTISE_SPEED_DEFAULT;

	DEBUGOUT("Reconfiguring auto-neg advertisement params\n");
	ret_val = e1000_phy_setup_autoneg(hw);
	if (ret_val < 0) {
		DEBUGOUT("Error Setting up Auto-Negotiation\n");
		return ret_val;
	}
	DEBUGOUT("Restarting Auto-Neg\n");

	/* Restart auto-negotiation by setting the Auto Neg Enable bit and
	 * the Auto Neg Restart bit in the PHY control register.
	 */
	if (e1000_read_phy_reg(hw, PHY_CTRL, &phy_data) < 0) {
		DEBUGOUT("PHY Read Error\n");
		return -E1000_ERR_PHY;
	}
	phy_data |= (MII_CR_AUTO_NEG_EN | MII_CR_RESTART_AUTO_NEG);
	if (e1000_write_phy_reg(hw, PHY_CTRL, phy_data) < 0) {
		DEBUGOUT("PHY Write Error\n");
		return -E1000_ERR_PHY;
	}
#if 0
	/* Does the user want to wait for Auto-Neg to complete here, or
	 * check at a later time (for example, callback routine).
	 */
	if (hw->wait_autoneg_complete) {
		ret_val = e1000_wait_autoneg(hw);
		if (ret_val < 0) {
			DEBUGOUT
			    ("Error while waiting for autoneg to complete\n");
			return ret_val;
		}
	}
#else
	/* If we do not wait for autonegtation to complete I
	 * do not see a valid link status.
	 */
	ret_val = e1000_wait_autoneg(hw);
	if (ret_val < 0) {
		DEBUGOUT("Error while waiting for autoneg to complete\n");
		return ret_val;
	}
#endif

	/* Check link status. Wait up to 100 microseconds for link to become
	 * valid.
	 */
	for (i = 0; i < 10; i++) {
		if (e1000_read_phy_reg(hw, PHY_STATUS, &phy_data) < 0) {
			DEBUGOUT("PHY Read Error\n");
			return -E1000_ERR_PHY;
		}
		if (e1000_read_phy_reg(hw, PHY_STATUS, &phy_data) < 0) {
			DEBUGOUT("PHY Read Error\n");
			return -E1000_ERR_PHY;
		}
		if (phy_data & MII_SR_LINK_STATUS) {
			/* We have link, so we need to finish the config process:
			 *   1) Set up the MAC to the current PHY speed/duplex
			 *      if we are on 82543.  If we
			 *      are on newer silicon, we only need to configure
			 *      collision distance in the Transmit Control Register.
			 *   2) Set up flow control on the MAC to that established with
			 *      the link partner.
			 */
			if (hw->mac_type >= e1000_82544) {
				e1000_config_collision_dist(hw);
			} else {
				ret_val = e1000_config_mac_to_phy(hw);
				if (ret_val < 0) {
					DEBUGOUT
					    ("Error configuring MAC to PHY settings\n");
					return ret_val;
				}
			}
			ret_val = e1000_config_fc_after_link_up(hw);
			if (ret_val < 0) {
				DEBUGOUT("Error Configuring Flow Control\n");
				return ret_val;
			}
			DEBUGOUT("Valid link established!!!\n");
			return 0;
		}
		udelay(10);
	}

	DEBUGOUT("Unable to establish link!!!\n");
	return -E1000_ERR_NOLINK;
}

/******************************************************************************
* Configures PHY autoneg and flow control advertisement settings
*
* hw - Struct containing variables accessed by shared code
******************************************************************************/
static int
e1000_phy_setup_autoneg(struct e1000_hw *hw)
{
	uint16_t mii_autoneg_adv_reg;
	uint16_t mii_1000t_ctrl_reg;

	DEBUGFUNC();

	/* Read the MII Auto-Neg Advertisement Register (Address 4). */
	if (e1000_read_phy_reg(hw, PHY_AUTONEG_ADV, &mii_autoneg_adv_reg) < 0) {
		DEBUGOUT("PHY Read Error\n");
		return -E1000_ERR_PHY;
	}

	/* Read the MII 1000Base-T Control Register (Address 9). */
	if (e1000_read_phy_reg(hw, PHY_1000T_CTRL, &mii_1000t_ctrl_reg) < 0) {
		DEBUGOUT("PHY Read Error\n");
		return -E1000_ERR_PHY;
	}

	/* Need to parse both autoneg_advertised and fc and set up
	 * the appropriate PHY registers.  First we will parse for
	 * autoneg_advertised software override.  Since we can advertise
	 * a plethora of combinations, we need to check each bit
	 * individually.
	 */

	/* First we clear all the 10/100 mb speed bits in the Auto-Neg
	 * Advertisement Register (Address 4) and the 1000 mb speed bits in
	 * the  1000Base-T Control Register (Address 9).
	 */
	mii_autoneg_adv_reg &= ~REG4_SPEED_MASK;
	mii_1000t_ctrl_reg &= ~REG9_SPEED_MASK;

	DEBUGOUT("autoneg_advertised %x\n", hw->autoneg_advertised);

	/* Do we want to advertise 10 Mb Half Duplex? */
	if (hw->autoneg_advertised & ADVERTISE_10_HALF) {
		DEBUGOUT("Advertise 10mb Half duplex\n");
		mii_autoneg_adv_reg |= NWAY_AR_10T_HD_CAPS;
	}

	/* Do we want to advertise 10 Mb Full Duplex? */
	if (hw->autoneg_advertised & ADVERTISE_10_FULL) {
		DEBUGOUT("Advertise 10mb Full duplex\n");
		mii_autoneg_adv_reg |= NWAY_AR_10T_FD_CAPS;
	}

	/* Do we want to advertise 100 Mb Half Duplex? */
	if (hw->autoneg_advertised & ADVERTISE_100_HALF) {
		DEBUGOUT("Advertise 100mb Half duplex\n");
		mii_autoneg_adv_reg |= NWAY_AR_100TX_HD_CAPS;
	}

	/* Do we want to advertise 100 Mb Full Duplex? */
	if (hw->autoneg_advertised & ADVERTISE_100_FULL) {
		DEBUGOUT("Advertise 100mb Full duplex\n");
		mii_autoneg_adv_reg |= NWAY_AR_100TX_FD_CAPS;
	}

	/* We do not allow the Phy to advertise 1000 Mb Half Duplex */
	if (hw->autoneg_advertised & ADVERTISE_1000_HALF) {
		DEBUGOUT
		    ("Advertise 1000mb Half duplex requested, request denied!\n");
	}

	/* Do we want to advertise 1000 Mb Full Duplex? */
	if (hw->autoneg_advertised & ADVERTISE_1000_FULL) {
		DEBUGOUT("Advertise 1000mb Full duplex\n");
		mii_1000t_ctrl_reg |= CR_1000T_FD_CAPS;
	}

	/* Check for a software override of the flow control settings, and
	 * setup the PHY advertisement registers accordingly.  If
	 * auto-negotiation is enabled, then software will have to set the
	 * "PAUSE" bits to the correct value in the Auto-Negotiation
	 * Advertisement Register (PHY_AUTONEG_ADV) and re-start auto-negotiation.
	 *
	 * The possible values of the "fc" parameter are:
	 *      0:  Flow control is completely disabled
	 *      1:  Rx flow control is enabled (we can receive pause frames
	 *          but not send pause frames).
	 *      2:  Tx flow control is enabled (we can send pause frames
	 *          but we do not support receiving pause frames).
	 *      3:  Both Rx and TX flow control (symmetric) are enabled.
	 *  other:  No software override.  The flow control configuration
	 *          in the EEPROM is used.
	 */
	switch (hw->fc) {
	case e1000_fc_none:	/* 0 */
		/* Flow control (RX & TX) is completely disabled by a
		 * software over-ride.
		 */
		mii_autoneg_adv_reg &= ~(NWAY_AR_ASM_DIR | NWAY_AR_PAUSE);
		break;
	case e1000_fc_rx_pause:	/* 1 */
		/* RX Flow control is enabled, and TX Flow control is
		 * disabled, by a software over-ride.
		 */
		/* Since there really isn't a way to advertise that we are
		 * capable of RX Pause ONLY, we will advertise that we
		 * support both symmetric and asymmetric RX PAUSE.  Later
		 * (in e1000_config_fc_after_link_up) we will disable the
		 *hw's ability to send PAUSE frames.
		 */
		mii_autoneg_adv_reg |= (NWAY_AR_ASM_DIR | NWAY_AR_PAUSE);
		break;
	case e1000_fc_tx_pause:	/* 2 */
		/* TX Flow control is enabled, and RX Flow control is
		 * disabled, by a software over-ride.
		 */
		mii_autoneg_adv_reg |= NWAY_AR_ASM_DIR;
		mii_autoneg_adv_reg &= ~NWAY_AR_PAUSE;
		break;
	case e1000_fc_full:	/* 3 */
		/* Flow control (both RX and TX) is enabled by a software
		 * over-ride.
		 */
		mii_autoneg_adv_reg |= (NWAY_AR_ASM_DIR | NWAY_AR_PAUSE);
		break;
	default:
		DEBUGOUT("Flow control param set incorrectly\n");
		return -E1000_ERR_CONFIG;
	}

	if (e1000_write_phy_reg(hw, PHY_AUTONEG_ADV, mii_autoneg_adv_reg) < 0) {
		DEBUGOUT("PHY Write Error\n");
		return -E1000_ERR_PHY;
	}

	DEBUGOUT("Auto-Neg Advertising %x\n", mii_autoneg_adv_reg);

	if (e1000_write_phy_reg(hw, PHY_1000T_CTRL, mii_1000t_ctrl_reg) < 0) {
		DEBUGOUT("PHY Write Error\n");
		return -E1000_ERR_PHY;
	}
	return 0;
}

/******************************************************************************
* Sets the collision distance in the Transmit Control register
*
* hw - Struct containing variables accessed by shared code
*
* Link should have been established previously. Reads the speed and duplex
* information from the Device Status register.
******************************************************************************/
static void
e1000_config_collision_dist(struct e1000_hw *hw)
{
	uint32_t tctl;

	tctl = E1000_READ_REG(hw, TCTL);

	tctl &= ~E1000_TCTL_COLD;
	tctl |= E1000_COLLISION_DISTANCE << E1000_COLD_SHIFT;

	E1000_WRITE_REG(hw, TCTL, tctl);
	E1000_WRITE_FLUSH(hw);
}

/******************************************************************************
* Sets MAC speed and duplex settings to reflect the those in the PHY
*
* hw - Struct containing variables accessed by shared code
* mii_reg - data to write to the MII control register
*
* The contents of the PHY register containing the needed information need to
* be passed in.
******************************************************************************/
static int
e1000_config_mac_to_phy(struct e1000_hw *hw)
{
	uint32_t ctrl;
	uint16_t phy_data;

	DEBUGFUNC();

	/* Read the Device Control Register and set the bits to Force Speed
	 * and Duplex.
	 */
	ctrl = E1000_READ_REG(hw, CTRL);
	ctrl |= (E1000_CTRL_FRCSPD | E1000_CTRL_FRCDPX);
	ctrl &= ~(E1000_CTRL_SPD_SEL | E1000_CTRL_ILOS);

	/* Set up duplex in the Device Control and Transmit Control
	 * registers depending on negotiated values.
	 */
	if (e1000_read_phy_reg(hw, M88E1000_PHY_SPEC_STATUS, &phy_data) < 0) {
		DEBUGOUT("PHY Read Error\n");
		return -E1000_ERR_PHY;
	}
	if (phy_data & M88E1000_PSSR_DPLX)
		ctrl |= E1000_CTRL_FD;
	else
		ctrl &= ~E1000_CTRL_FD;

	e1000_config_collision_dist(hw);

	/* Set up speed in the Device Control register depending on
	 * negotiated values.
	 */
	if ((phy_data & M88E1000_PSSR_SPEED) == M88E1000_PSSR_1000MBS)
		ctrl |= E1000_CTRL_SPD_1000;
	else if ((phy_data & M88E1000_PSSR_SPEED) == M88E1000_PSSR_100MBS)
		ctrl |= E1000_CTRL_SPD_100;
	/* Write the configured values back to the Device Control Reg. */
	E1000_WRITE_REG(hw, CTRL, ctrl);
	return 0;
}

/******************************************************************************
 * Forces the MAC's flow control settings.
 *
 * hw - Struct containing variables accessed by shared code
 *
 * Sets the TFCE and RFCE bits in the device control register to reflect
 * the adapter settings. TFCE and RFCE need to be explicitly set by
 * software when a Copper PHY is used because autonegotiation is managed
 * by the PHY rather than the MAC. Software must also configure these
 * bits when link is forced on a fiber connection.
 *****************************************************************************/
static int
e1000_force_mac_fc(struct e1000_hw *hw)
{
	uint32_t ctrl;

	DEBUGFUNC();

	/* Get the current configuration of the Device Control Register */
	ctrl = E1000_READ_REG(hw, CTRL);

	/* Because we didn't get link via the internal auto-negotiation
	 * mechanism (we either forced link or we got link via PHY
	 * auto-neg), we have to manually enable/disable transmit an
	 * receive flow control.
	 *
	 * The "Case" statement below enables/disable flow control
	 * according to the "hw->fc" parameter.
	 *
	 * The possible values of the "fc" parameter are:
	 *      0:  Flow control is completely disabled
	 *      1:  Rx flow control is enabled (we can receive pause
	 *          frames but not send pause frames).
	 *      2:  Tx flow control is enabled (we can send pause frames
	 *          frames but we do not receive pause frames).
	 *      3:  Both Rx and TX flow control (symmetric) is enabled.
	 *  other:  No other values should be possible at this point.
	 */

	switch (hw->fc) {
	case e1000_fc_none:
		ctrl &= (~(E1000_CTRL_TFCE | E1000_CTRL_RFCE));
		break;
	case e1000_fc_rx_pause:
		ctrl &= (~E1000_CTRL_TFCE);
		ctrl |= E1000_CTRL_RFCE;
		break;
	case e1000_fc_tx_pause:
		ctrl &= (~E1000_CTRL_RFCE);
		ctrl |= E1000_CTRL_TFCE;
		break;
	case e1000_fc_full:
		ctrl |= (E1000_CTRL_TFCE | E1000_CTRL_RFCE);
		break;
	default:
		DEBUGOUT("Flow control param set incorrectly\n");
		return -E1000_ERR_CONFIG;
	}

	/* Disable TX Flow Control for 82542 (rev 2.0) */
	if (hw->mac_type == e1000_82542_rev2_0)
		ctrl &= (~E1000_CTRL_TFCE);

	E1000_WRITE_REG(hw, CTRL, ctrl);
	return 0;
}

/******************************************************************************
 * Configures flow control settings after link is established
 *
 * hw - Struct containing variables accessed by shared code
 *
 * Should be called immediately after a valid link has been established.
 * Forces MAC flow control settings if link was forced. When in MII/GMII mode
 * and autonegotiation is enabled, the MAC flow control settings will be set
 * based on the flow control negotiated by the PHY. In TBI mode, the TFCE
 * and RFCE bits will be automaticaly set to the negotiated flow control mode.
 *****************************************************************************/
static int
e1000_config_fc_after_link_up(struct e1000_hw *hw)
{
	int32_t ret_val;
	uint16_t mii_status_reg;
	uint16_t mii_nway_adv_reg;
	uint16_t mii_nway_lp_ability_reg;
	uint16_t speed;
	uint16_t duplex;

	DEBUGFUNC();

	/* Check for the case where we have fiber media and auto-neg failed
	 * so we had to force link.  In this case, we need to force the
	 * configuration of the MAC to match the "fc" parameter.
	 */
	if ((hw->media_type == e1000_media_type_fiber) && (hw->autoneg_failed)) {
		ret_val = e1000_force_mac_fc(hw);
		if (ret_val < 0) {
			DEBUGOUT("Error forcing flow control settings\n");
			return ret_val;
		}
	}

	/* Check for the case where we have copper media and auto-neg is
	 * enabled.  In this case, we need to check and see if Auto-Neg
	 * has completed, and if so, how the PHY and link partner has
	 * flow control configured.
	 */
	if (hw->media_type == e1000_media_type_copper) {
		/* Read the MII Status Register and check to see if AutoNeg
		 * has completed.  We read this twice because this reg has
		 * some "sticky" (latched) bits.
		 */
		if (e1000_read_phy_reg(hw, PHY_STATUS, &mii_status_reg) < 0) {
			DEBUGOUT("PHY Read Error \n");
			return -E1000_ERR_PHY;
		}
		if (e1000_read_phy_reg(hw, PHY_STATUS, &mii_status_reg) < 0) {
			DEBUGOUT("PHY Read Error \n");
			return -E1000_ERR_PHY;
		}

		if (mii_status_reg & MII_SR_AUTONEG_COMPLETE) {
			/* The AutoNeg process has completed, so we now need to
			 * read both the Auto Negotiation Advertisement Register
			 * (Address 4) and the Auto_Negotiation Base Page Ability
			 * Register (Address 5) to determine how flow control was
			 * negotiated.
			 */
			if (e1000_read_phy_reg
			    (hw, PHY_AUTONEG_ADV, &mii_nway_adv_reg) < 0) {
				DEBUGOUT("PHY Read Error\n");
				return -E1000_ERR_PHY;
			}
			if (e1000_read_phy_reg
			    (hw, PHY_LP_ABILITY,
			     &mii_nway_lp_ability_reg) < 0) {
				DEBUGOUT("PHY Read Error\n");
				return -E1000_ERR_PHY;
			}

			/* Two bits in the Auto Negotiation Advertisement Register
			 * (Address 4) and two bits in the Auto Negotiation Base
			 * Page Ability Register (Address 5) determine flow control
			 * for both the PHY and the link partner.  The following
			 * table, taken out of the IEEE 802.3ab/D6.0 dated March 25,
			 * 1999, describes these PAUSE resolution bits and how flow
			 * control is determined based upon these settings.
			 * NOTE:  DC = Don't Care
			 *
			 *   LOCAL DEVICE  |   LINK PARTNER
			 * PAUSE | ASM_DIR | PAUSE | ASM_DIR | NIC Resolution
			 *-------|---------|-------|---------|--------------------
			 *   0   |    0    |  DC   |   DC    | e1000_fc_none
			 *   0   |    1    |   0   |   DC    | e1000_fc_none
			 *   0   |    1    |   1   |    0    | e1000_fc_none
			 *   0   |    1    |   1   |    1    | e1000_fc_tx_pause
			 *   1   |    0    |   0   |   DC    | e1000_fc_none
			 *   1   |   DC    |   1   |   DC    | e1000_fc_full
			 *   1   |    1    |   0   |    0    | e1000_fc_none
			 *   1   |    1    |   0   |    1    | e1000_fc_rx_pause
			 *
			 */
			/* Are both PAUSE bits set to 1?  If so, this implies
			 * Symmetric Flow Control is enabled at both ends.  The
			 * ASM_DIR bits are irrelevant per the spec.
			 *
			 * For Symmetric Flow Control:
			 *
			 *   LOCAL DEVICE  |   LINK PARTNER
			 * PAUSE | ASM_DIR | PAUSE | ASM_DIR | Result
			 *-------|---------|-------|---------|--------------------
			 *   1   |   DC    |   1   |   DC    | e1000_fc_full
			 *
			 */
			if ((mii_nway_adv_reg & NWAY_AR_PAUSE) &&
			    (mii_nway_lp_ability_reg & NWAY_LPAR_PAUSE)) {
				/* Now we need to check if the user selected RX ONLY
				 * of pause frames.  In this case, we had to advertise
				 * FULL flow control because we could not advertise RX
				 * ONLY. Hence, we must now check to see if we need to
				 * turn OFF  the TRANSMISSION of PAUSE frames.
				 */
				if (hw->original_fc == e1000_fc_full) {
					hw->fc = e1000_fc_full;
					DEBUGOUT("Flow Control = FULL.\r\n");
				} else {
					hw->fc = e1000_fc_rx_pause;
					DEBUGOUT
					    ("Flow Control = RX PAUSE frames only.\r\n");
				}
			}
			/* For receiving PAUSE frames ONLY.
			 *
			 *   LOCAL DEVICE  |   LINK PARTNER
			 * PAUSE | ASM_DIR | PAUSE | ASM_DIR | Result
			 *-------|---------|-------|---------|--------------------
			 *   0   |    1    |   1   |    1    | e1000_fc_tx_pause
			 *
			 */
			else if (!(mii_nway_adv_reg & NWAY_AR_PAUSE) &&
				 (mii_nway_adv_reg & NWAY_AR_ASM_DIR) &&
				 (mii_nway_lp_ability_reg & NWAY_LPAR_PAUSE) &&
				 (mii_nway_lp_ability_reg & NWAY_LPAR_ASM_DIR))
			{
				hw->fc = e1000_fc_tx_pause;
				DEBUGOUT
				    ("Flow Control = TX PAUSE frames only.\r\n");
			}
			/* For transmitting PAUSE frames ONLY.
			 *
			 *   LOCAL DEVICE  |   LINK PARTNER
			 * PAUSE | ASM_DIR | PAUSE | ASM_DIR | Result
			 *-------|---------|-------|---------|--------------------
			 *   1   |    1    |   0   |    1    | e1000_fc_rx_pause
			 *
			 */
			else if ((mii_nway_adv_reg & NWAY_AR_PAUSE) &&
				 (mii_nway_adv_reg & NWAY_AR_ASM_DIR) &&
				 !(mii_nway_lp_ability_reg & NWAY_LPAR_PAUSE) &&
				 (mii_nway_lp_ability_reg & NWAY_LPAR_ASM_DIR))
			{
				hw->fc = e1000_fc_rx_pause;
				DEBUGOUT
				    ("Flow Control = RX PAUSE frames only.\r\n");
			}
			/* Per the IEEE spec, at this point flow control should be
			 * disabled.  However, we want to consider that we could
			 * be connected to a legacy switch that doesn't advertise
			 * desired flow control, but can be forced on the link
			 * partner.  So if we advertised no flow control, that is
			 * what we will resolve to.  If we advertised some kind of
			 * receive capability (Rx Pause Only or Full Flow Control)
			 * and the link partner advertised none, we will configure
			 * ourselves to enable Rx Flow Control only.  We can do
			 * this safely for two reasons:  If the link partner really
			 * didn't want flow control enabled, and we enable Rx, no
			 * harm done since we won't be receiving any PAUSE frames
			 * anyway.  If the intent on the link partner was to have
			 * flow control enabled, then by us enabling RX only, we
			 * can at least receive pause frames and process them.
			 * This is a good idea because in most cases, since we are
			 * predominantly a server NIC, more times than not we will
			 * be asked to delay transmission of packets than asking
			 * our link partner to pause transmission of frames.
			 */
			else if (hw->original_fc == e1000_fc_none ||
				 hw->original_fc == e1000_fc_tx_pause) {
				hw->fc = e1000_fc_none;
				DEBUGOUT("Flow Control = NONE.\r\n");
			} else {
				hw->fc = e1000_fc_rx_pause;
				DEBUGOUT
				    ("Flow Control = RX PAUSE frames only.\r\n");
			}

			/* Now we need to do one last check...  If we auto-
			 * negotiated to HALF DUPLEX, flow control should not be
			 * enabled per IEEE 802.3 spec.
			 */
			e1000_get_speed_and_duplex(hw, &speed, &duplex);

			if (duplex == HALF_DUPLEX)
				hw->fc = e1000_fc_none;

			/* Now we call a subroutine to actually force the MAC
			 * controller to use the correct flow control settings.
			 */
			ret_val = e1000_force_mac_fc(hw);
			if (ret_val < 0) {
				DEBUGOUT
				    ("Error forcing flow control settings\n");
				return ret_val;
			}
		} else {
			DEBUGOUT
			    ("Copper PHY and Auto Neg has not completed.\r\n");
		}
	}
	return 0;
}

/******************************************************************************
 * Checks to see if the link status of the hardware has changed.
 *
 * hw - Struct containing variables accessed by shared code
 *
 * Called by any function that needs to check the link status of the adapter.
 *****************************************************************************/
static int
e1000_check_for_link(struct eth_device *nic)
{
	struct e1000_hw *hw = nic->priv;
	uint32_t rxcw;
	uint32_t ctrl;
	uint32_t status;
	uint32_t rctl;
	uint32_t signal;
	int32_t ret_val;
	uint16_t phy_data;
	uint16_t lp_capability;

	DEBUGFUNC();

	/* On adapters with a MAC newer that 82544, SW Defineable pin 1 will be
	 * set when the optics detect a signal. On older adapters, it will be
	 * cleared when there is a signal
	 */
	ctrl = E1000_READ_REG(hw, CTRL);
	if ((hw->mac_type > e1000_82544) && !(ctrl & E1000_CTRL_ILOS))
		signal = E1000_CTRL_SWDPIN1;
	else
		signal = 0;

	status = E1000_READ_REG(hw, STATUS);
	rxcw = E1000_READ_REG(hw, RXCW);
	DEBUGOUT("ctrl: %#08x status %#08x rxcw %#08x\n", ctrl, status, rxcw);

	/* If we have a copper PHY then we only want to go out to the PHY
	 * registers to see if Auto-Neg has completed and/or if our link
	 * status has changed.  The get_link_status flag will be set if we
	 * receive a Link Status Change interrupt or we have Rx Sequence
	 * Errors.
	 */
	if ((hw->media_type == e1000_media_type_copper) && hw->get_link_status) {
		/* First we want to see if the MII Status Register reports
		 * link.  If so, then we want to get the current speed/duplex
		 * of the PHY.
		 * Read the register twice since the link bit is sticky.
		 */
		if (e1000_read_phy_reg(hw, PHY_STATUS, &phy_data) < 0) {
			DEBUGOUT("PHY Read Error\n");
			return -E1000_ERR_PHY;
		}
		if (e1000_read_phy_reg(hw, PHY_STATUS, &phy_data) < 0) {
			DEBUGOUT("PHY Read Error\n");
			return -E1000_ERR_PHY;
		}

		if (phy_data & MII_SR_LINK_STATUS) {
			hw->get_link_status = FALSE;
		} else {
			/* No link detected */
			return -E1000_ERR_NOLINK;
		}

		/* We have a M88E1000 PHY and Auto-Neg is enabled.  If we
		 * have Si on board that is 82544 or newer, Auto
		 * Speed Detection takes care of MAC speed/duplex
		 * configuration.  So we only need to configure Collision
		 * Distance in the MAC.  Otherwise, we need to force
		 * speed/duplex on the MAC to the current PHY speed/duplex
		 * settings.
		 */
		if (hw->mac_type >= e1000_82544)
			e1000_config_collision_dist(hw);
		else {
			ret_val = e1000_config_mac_to_phy(hw);
			if (ret_val < 0) {
				DEBUGOUT
				    ("Error configuring MAC to PHY settings\n");
				return ret_val;
			}
		}

		/* Configure Flow Control now that Auto-Neg has completed. First, we
		 * need to restore the desired flow control settings because we may
		 * have had to re-autoneg with a different link partner.
		 */
		ret_val = e1000_config_fc_after_link_up(hw);
		if (ret_val < 0) {
			DEBUGOUT("Error configuring flow control\n");
			return ret_val;
		}

		/* At this point we know that we are on copper and we have
		 * auto-negotiated link.  These are conditions for checking the link
		 * parter capability register.  We use the link partner capability to
		 * determine if TBI Compatibility needs to be turned on or off.  If
		 * the link partner advertises any speed in addition to Gigabit, then
		 * we assume that they are GMII-based, and TBI compatibility is not
		 * needed. If no other speeds are advertised, we assume the link
		 * partner is TBI-based, and we turn on TBI Compatibility.
		 */
		if (hw->tbi_compatibility_en) {
			if (e1000_read_phy_reg
			    (hw, PHY_LP_ABILITY, &lp_capability) < 0) {
				DEBUGOUT("PHY Read Error\n");
				return -E1000_ERR_PHY;
			}
			if (lp_capability & (NWAY_LPAR_10T_HD_CAPS |
					     NWAY_LPAR_10T_FD_CAPS |
					     NWAY_LPAR_100TX_HD_CAPS |
					     NWAY_LPAR_100TX_FD_CAPS |
					     NWAY_LPAR_100T4_CAPS)) {
				/* If our link partner advertises anything in addition to
				 * gigabit, we do not need to enable TBI compatibility.
				 */
				if (hw->tbi_compatibility_on) {
					/* If we previously were in the mode, turn it off. */
					rctl = E1000_READ_REG(hw, RCTL);
					rctl &= ~E1000_RCTL_SBP;
					E1000_WRITE_REG(hw, RCTL, rctl);
					hw->tbi_compatibility_on = FALSE;
				}
			} else {
				/* If TBI compatibility is was previously off, turn it on. For
				 * compatibility with a TBI link partner, we will store bad
				 * packets. Some frames have an additional byte on the end and
				 * will look like CRC errors to to the hardware.
				 */
				if (!hw->tbi_compatibility_on) {
					hw->tbi_compatibility_on = TRUE;
					rctl = E1000_READ_REG(hw, RCTL);
					rctl |= E1000_RCTL_SBP;
					E1000_WRITE_REG(hw, RCTL, rctl);
				}
			}
		}
	}
	/* If we don't have link (auto-negotiation failed or link partner cannot
	 * auto-negotiate), the cable is plugged in (we have signal), and our
	 * link partner is not trying to auto-negotiate with us (we are receiving
	 * idles or data), we need to force link up. We also need to give
	 * auto-negotiation time to complete, in case the cable was just plugged
	 * in. The autoneg_failed flag does this.
	 */
	else if ((hw->media_type == e1000_media_type_fiber) &&
		 (!(status & E1000_STATUS_LU)) &&
		 ((ctrl & E1000_CTRL_SWDPIN1) == signal) &&
		 (!(rxcw & E1000_RXCW_C))) {
		if (hw->autoneg_failed == 0) {
			hw->autoneg_failed = 1;
			return 0;
		}
		DEBUGOUT("NOT RXing /C/, disable AutoNeg and force link.\r\n");

		/* Disable auto-negotiation in the TXCW register */
		E1000_WRITE_REG(hw, TXCW, (hw->txcw & ~E1000_TXCW_ANE));

		/* Force link-up and also force full-duplex. */
		ctrl = E1000_READ_REG(hw, CTRL);
		ctrl |= (E1000_CTRL_SLU | E1000_CTRL_FD);
		E1000_WRITE_REG(hw, CTRL, ctrl);

		/* Configure Flow Control after forcing link up. */
		ret_val = e1000_config_fc_after_link_up(hw);
		if (ret_val < 0) {
			DEBUGOUT("Error configuring flow control\n");
			return ret_val;
		}
	}
	/* If we are forcing link and we are receiving /C/ ordered sets, re-enable
	 * auto-negotiation in the TXCW register and disable forced link in the
	 * Device Control register in an attempt to auto-negotiate with our link
	 * partner.
	 */
	else if ((hw->media_type == e1000_media_type_fiber) &&
		 (ctrl & E1000_CTRL_SLU) && (rxcw & E1000_RXCW_C)) {
		DEBUGOUT
		    ("RXing /C/, enable AutoNeg and stop forcing link.\r\n");
		E1000_WRITE_REG(hw, TXCW, hw->txcw);
		E1000_WRITE_REG(hw, CTRL, (ctrl & ~E1000_CTRL_SLU));
	}
	return 0;
}

/******************************************************************************
 * Detects the current speed and duplex settings of the hardware.
 *
 * hw - Struct containing variables accessed by shared code
 * speed - Speed of the connection
 * duplex - Duplex setting of the connection
 *****************************************************************************/
static void
e1000_get_speed_and_duplex(struct e1000_hw *hw,
			   uint16_t * speed, uint16_t * duplex)
{
	uint32_t status;

	DEBUGFUNC();

	if (hw->mac_type >= e1000_82543) {
		status = E1000_READ_REG(hw, STATUS);
		if (status & E1000_STATUS_SPEED_1000) {
			*speed = SPEED_1000;
			DEBUGOUT("1000 Mbs, ");
		} else if (status & E1000_STATUS_SPEED_100) {
			*speed = SPEED_100;
			DEBUGOUT("100 Mbs, ");
		} else {
			*speed = SPEED_10;
			DEBUGOUT("10 Mbs, ");
		}

		if (status & E1000_STATUS_FD) {
			*duplex = FULL_DUPLEX;
			DEBUGOUT("Full Duplex\r\n");
		} else {
			*duplex = HALF_DUPLEX;
			DEBUGOUT(" Half Duplex\r\n");
		}
	} else {
		DEBUGOUT("1000 Mbs, Full Duplex\r\n");
		*speed = SPEED_1000;
		*duplex = FULL_DUPLEX;
	}
}

/******************************************************************************
* Blocks until autoneg completes or times out (~4.5 seconds)
*
* hw - Struct containing variables accessed by shared code
******************************************************************************/
static int
e1000_wait_autoneg(struct e1000_hw *hw)
{
	uint16_t i;
	uint16_t phy_data;

	DEBUGFUNC();
	DEBUGOUT("Waiting for Auto-Neg to complete.\n");

	/* We will wait for autoneg to complete or 4.5 seconds to expire. */
	for (i = PHY_AUTO_NEG_TIME; i > 0; i--) {
		/* Read the MII Status Register and wait for Auto-Neg
		 * Complete bit to be set.
		 */
		if (e1000_read_phy_reg(hw, PHY_STATUS, &phy_data) < 0) {
			DEBUGOUT("PHY Read Error\n");
			return -E1000_ERR_PHY;
		}
		if (e1000_read_phy_reg(hw, PHY_STATUS, &phy_data) < 0) {
			DEBUGOUT("PHY Read Error\n");
			return -E1000_ERR_PHY;
		}
		if (phy_data & MII_SR_AUTONEG_COMPLETE) {
			DEBUGOUT("Auto-Neg complete.\n");
			return 0;
		}
		mdelay(100);
	}
	DEBUGOUT("Auto-Neg timedout.\n");
	return -E1000_ERR_TIMEOUT;
}

/******************************************************************************
* Raises the Management Data Clock
*
* hw - Struct containing variables accessed by shared code
* ctrl - Device control register's current value
******************************************************************************/
static void
e1000_raise_mdi_clk(struct e1000_hw *hw, uint32_t * ctrl)
{
	/* Raise the clock input to the Management Data Clock (by setting the MDC
	 * bit), and then delay 2 microseconds.
	 */
	E1000_WRITE_REG(hw, CTRL, (*ctrl | E1000_CTRL_MDC));
	E1000_WRITE_FLUSH(hw);
	udelay(2);
}

/******************************************************************************
* Lowers the Management Data Clock
*
* hw - Struct containing variables accessed by shared code
* ctrl - Device control register's current value
******************************************************************************/
static void
e1000_lower_mdi_clk(struct e1000_hw *hw, uint32_t * ctrl)
{
	/* Lower the clock input to the Management Data Clock (by clearing the MDC
	 * bit), and then delay 2 microseconds.
	 */
	E1000_WRITE_REG(hw, CTRL, (*ctrl & ~E1000_CTRL_MDC));
	E1000_WRITE_FLUSH(hw);
	udelay(2);
}

/******************************************************************************
* Shifts data bits out to the PHY
*
* hw - Struct containing variables accessed by shared code
* data - Data to send out to the PHY
* count - Number of bits to shift out
*
* Bits are shifted out in MSB to LSB order.
******************************************************************************/
static void
e1000_shift_out_mdi_bits(struct e1000_hw *hw, uint32_t data, uint16_t count)
{
	uint32_t ctrl;
	uint32_t mask;

	/* We need to shift "count" number of bits out to the PHY. So, the value
	 * in the "data" parameter will be shifted out to the PHY one bit at a
	 * time. In order to do this, "data" must be broken down into bits.
	 */
	mask = 0x01;
	mask <<= (count - 1);

	ctrl = E1000_READ_REG(hw, CTRL);

	/* Set MDIO_DIR and MDC_DIR direction bits to be used as output pins. */
	ctrl |= (E1000_CTRL_MDIO_DIR | E1000_CTRL_MDC_DIR);

	while (mask) {
		/* A "1" is shifted out to the PHY by setting the MDIO bit to "1" and
		 * then raising and lowering the Management Data Clock. A "0" is
		 * shifted out to the PHY by setting the MDIO bit to "0" and then
		 * raising and lowering the clock.
		 */
		if (data & mask)
			ctrl |= E1000_CTRL_MDIO;
		else
			ctrl &= ~E1000_CTRL_MDIO;

		E1000_WRITE_REG(hw, CTRL, ctrl);
		E1000_WRITE_FLUSH(hw);

		udelay(2);

		e1000_raise_mdi_clk(hw, &ctrl);
		e1000_lower_mdi_clk(hw, &ctrl);

		mask = mask >> 1;
	}
}

/******************************************************************************
* Shifts data bits in from the PHY
*
* hw - Struct containing variables accessed by shared code
*
* Bits are shifted in in MSB to LSB order.
******************************************************************************/
static uint16_t
e1000_shift_in_mdi_bits(struct e1000_hw *hw)
{
	uint32_t ctrl;
	uint16_t data = 0;
	uint8_t i;

	/* In order to read a register from the PHY, we need to shift in a total
	 * of 18 bits from the PHY. The first two bit (turnaround) times are used
	 * to avoid contention on the MDIO pin when a read operation is performed.
	 * These two bits are ignored by us and thrown away. Bits are "shifted in"
	 * by raising the input to the Management Data Clock (setting the MDC bit),
	 * and then reading the value of the MDIO bit.
	 */
	ctrl = E1000_READ_REG(hw, CTRL);

	/* Clear MDIO_DIR (SWDPIO1) to indicate this bit is to be used as input. */
	ctrl &= ~E1000_CTRL_MDIO_DIR;
	ctrl &= ~E1000_CTRL_MDIO;

	E1000_WRITE_REG(hw, CTRL, ctrl);
	E1000_WRITE_FLUSH(hw);

	/* Raise and Lower the clock before reading in the data. This accounts for
	 * the turnaround bits. The first clock occurred when we clocked out the
	 * last bit of the Register Address.
	 */
	e1000_raise_mdi_clk(hw, &ctrl);
	e1000_lower_mdi_clk(hw, &ctrl);

	for (data = 0, i = 0; i < 16; i++) {
		data = data << 1;
		e1000_raise_mdi_clk(hw, &ctrl);
		ctrl = E1000_READ_REG(hw, CTRL);
		/* Check to see if we shifted in a "1". */
		if (ctrl & E1000_CTRL_MDIO)
			data |= 1;
		e1000_lower_mdi_clk(hw, &ctrl);
	}

	e1000_raise_mdi_clk(hw, &ctrl);
	e1000_lower_mdi_clk(hw, &ctrl);

	return data;
}

/*****************************************************************************
* Reads the value from a PHY register
*
* hw - Struct containing variables accessed by shared code
* reg_addr - address of the PHY register to read
******************************************************************************/
static int
e1000_read_phy_reg(struct e1000_hw *hw, uint32_t reg_addr, uint16_t * phy_data)
{
	uint32_t i;
	uint32_t mdic = 0;
	const uint32_t phy_addr = 1;

	if (reg_addr > MAX_PHY_REG_ADDRESS) {
		DEBUGOUT("PHY Address %d is out of range\n", reg_addr);
		return -E1000_ERR_PARAM;
	}

	if (hw->mac_type > e1000_82543) {
		/* Set up Op-code, Phy Address, and register address in the MDI
		 * Control register.  The MAC will take care of interfacing with the
		 * PHY to retrieve the desired data.
		 */
		mdic = ((reg_addr << E1000_MDIC_REG_SHIFT) |
			(phy_addr << E1000_MDIC_PHY_SHIFT) |
			(E1000_MDIC_OP_READ));

		E1000_WRITE_REG(hw, MDIC, mdic);

		/* Poll the ready bit to see if the MDI read completed */
		for (i = 0; i < 64; i++) {
			udelay(10);
			mdic = E1000_READ_REG(hw, MDIC);
			if (mdic & E1000_MDIC_READY)
				break;
		}
		if (!(mdic & E1000_MDIC_READY)) {
			DEBUGOUT("MDI Read did not complete\n");
			return -E1000_ERR_PHY;
		}
		if (mdic & E1000_MDIC_ERROR) {
			DEBUGOUT("MDI Error\n");
			return -E1000_ERR_PHY;
		}
		*phy_data = (uint16_t) mdic;
	} else {
		/* We must first send a preamble through the MDIO pin to signal the
		 * beginning of an MII instruction.  This is done by sending 32
		 * consecutive "1" bits.
		 */
		e1000_shift_out_mdi_bits(hw, PHY_PREAMBLE, PHY_PREAMBLE_SIZE);

		/* Now combine the next few fields that are required for a read
		 * operation.  We use this method instead of calling the
		 * e1000_shift_out_mdi_bits routine five different times. The format of
		 * a MII read instruction consists of a shift out of 14 bits and is
		 * defined as follows:
		 *    <Preamble><SOF><Op Code><Phy Addr><Reg Addr>
		 * followed by a shift in of 18 bits.  This first two bits shifted in
		 * are TurnAround bits used to avoid contention on the MDIO pin when a
		 * READ operation is performed.  These two bits are thrown away
		 * followed by a shift in of 16 bits which contains the desired data.
		 */
		mdic = ((reg_addr) | (phy_addr << 5) |
			(PHY_OP_READ << 10) | (PHY_SOF << 12));

		e1000_shift_out_mdi_bits(hw, mdic, 14);

		/* Now that we've shifted out the read command to the MII, we need to
		 * "shift in" the 16-bit value (18 total bits) of the requested PHY
		 * register address.
		 */
		*phy_data = e1000_shift_in_mdi_bits(hw);
	}
	return 0;
}

/******************************************************************************
* Writes a value to a PHY register
*
* hw - Struct containing variables accessed by shared code
* reg_addr - address of the PHY register to write
* data - data to write to the PHY
******************************************************************************/
static int
e1000_write_phy_reg(struct e1000_hw *hw, uint32_t reg_addr, uint16_t phy_data)
{
	uint32_t i;
	uint32_t mdic = 0;
	const uint32_t phy_addr = 1;

	if (reg_addr > MAX_PHY_REG_ADDRESS) {
		DEBUGOUT("PHY Address %d is out of range\n", reg_addr);
		return -E1000_ERR_PARAM;
	}

	if (hw->mac_type > e1000_82543) {
		/* Set up Op-code, Phy Address, register address, and data intended
		 * for the PHY register in the MDI Control register.  The MAC will take
		 * care of interfacing with the PHY to send the desired data.
		 */
		mdic = (((uint32_t) phy_data) |
			(reg_addr << E1000_MDIC_REG_SHIFT) |
			(phy_addr << E1000_MDIC_PHY_SHIFT) |
			(E1000_MDIC_OP_WRITE));

		E1000_WRITE_REG(hw, MDIC, mdic);

		/* Poll the ready bit to see if the MDI read completed */
		for (i = 0; i < 64; i++) {
			udelay(10);
			mdic = E1000_READ_REG(hw, MDIC);
			if (mdic & E1000_MDIC_READY)
				break;
		}
		if (!(mdic & E1000_MDIC_READY)) {
			DEBUGOUT("MDI Write did not complete\n");
			return -E1000_ERR_PHY;
		}
	} else {
		/* We'll need to use the SW defined pins to shift the write command
		 * out to the PHY. We first send a preamble to the PHY to signal the
		 * beginning of the MII instruction.  This is done by sending 32
		 * consecutive "1" bits.
		 */
		e1000_shift_out_mdi_bits(hw, PHY_PREAMBLE, PHY_PREAMBLE_SIZE);

		/* Now combine the remaining required fields that will indicate a
		 * write operation. We use this method instead of calling the
		 * e1000_shift_out_mdi_bits routine for each field in the command. The
		 * format of a MII write instruction is as follows:
		 * <Preamble><SOF><Op Code><Phy Addr><Reg Addr><Turnaround><Data>.
		 */
		mdic = ((PHY_TURNAROUND) | (reg_addr << 2) | (phy_addr << 7) |
			(PHY_OP_WRITE << 12) | (PHY_SOF << 14));
		mdic <<= 16;
		mdic |= (uint32_t) phy_data;

		e1000_shift_out_mdi_bits(hw, mdic, 32);
	}
	return 0;
}

/******************************************************************************
* Returns the PHY to the power-on reset state
*
* hw - Struct containing variables accessed by shared code
******************************************************************************/
static void
e1000_phy_hw_reset(struct e1000_hw *hw)
{
	uint32_t ctrl;
	uint32_t ctrl_ext;

	DEBUGFUNC();

	DEBUGOUT("Resetting Phy...\n");

	if (hw->mac_type > e1000_82543) {
		/* Read the device control register and assert the E1000_CTRL_PHY_RST
		 * bit. Then, take it out of reset.
		 */
		ctrl = E1000_READ_REG(hw, CTRL);
		E1000_WRITE_REG(hw, CTRL, ctrl | E1000_CTRL_PHY_RST);
		E1000_WRITE_FLUSH(hw);
		mdelay(10);
		E1000_WRITE_REG(hw, CTRL, ctrl);
		E1000_WRITE_FLUSH(hw);
	} else {
		/* Read the Extended Device Control Register, assert the PHY_RESET_DIR
		 * bit to put the PHY into reset. Then, take it out of reset.
		 */
		ctrl_ext = E1000_READ_REG(hw, CTRL_EXT);
		ctrl_ext |= E1000_CTRL_EXT_SDP4_DIR;
		ctrl_ext &= ~E1000_CTRL_EXT_SDP4_DATA;
		E1000_WRITE_REG(hw, CTRL_EXT, ctrl_ext);
		E1000_WRITE_FLUSH(hw);
		mdelay(10);
		ctrl_ext |= E1000_CTRL_EXT_SDP4_DATA;
		E1000_WRITE_REG(hw, CTRL_EXT, ctrl_ext);
		E1000_WRITE_FLUSH(hw);
	}
	udelay(150);
}

/******************************************************************************
* Resets the PHY
*
* hw - Struct containing variables accessed by shared code
*
* Sets bit 15 of the MII Control regiser
******************************************************************************/
static int
e1000_phy_reset(struct e1000_hw *hw)
{
	uint16_t phy_data;

	DEBUGFUNC();

	if (e1000_read_phy_reg(hw, PHY_CTRL, &phy_data) < 0) {
		DEBUGOUT("PHY Read Error\n");
		return -E1000_ERR_PHY;
	}
	phy_data |= MII_CR_RESET;
	if (e1000_write_phy_reg(hw, PHY_CTRL, phy_data) < 0) {
		DEBUGOUT("PHY Write Error\n");
		return -E1000_ERR_PHY;
	}
	udelay(1);
	return 0;
}

/******************************************************************************
* Probes the expected PHY address for known PHY IDs
*
* hw - Struct containing variables accessed by shared code
******************************************************************************/
static int
e1000_detect_gig_phy(struct e1000_hw *hw)
{
	uint16_t phy_id_high, phy_id_low;
	int match = FALSE;

	DEBUGFUNC();

	/* Read the PHY ID Registers to identify which PHY is onboard. */
	if (e1000_read_phy_reg(hw, PHY_ID1, &phy_id_high) < 0) {
		DEBUGOUT("PHY Read Error\n");
		return -E1000_ERR_PHY;
	}
	hw->phy_id = (uint32_t) (phy_id_high << 16);
	udelay(2);
	if (e1000_read_phy_reg(hw, PHY_ID2, &phy_id_low) < 0) {
		DEBUGOUT("PHY Read Error\n");
		return -E1000_ERR_PHY;
	}
	hw->phy_id |= (uint32_t) (phy_id_low & PHY_REVISION_MASK);

	switch (hw->mac_type) {
	case e1000_82543:
		if (hw->phy_id == M88E1000_E_PHY_ID)
			match = TRUE;
		break;
	case e1000_82544:
		if (hw->phy_id == M88E1000_I_PHY_ID)
			match = TRUE;
		break;
	case e1000_82540:
	case e1000_82545:
	case e1000_82546:
		if (hw->phy_id == M88E1011_I_PHY_ID)
			match = TRUE;
		break;
	default:
		DEBUGOUT("Invalid MAC type %d\n", hw->mac_type);
		return -E1000_ERR_CONFIG;
	}
	if (match) {
		DEBUGOUT("PHY ID 0x%X detected\n", hw->phy_id);
		return 0;
	}
	DEBUGOUT("Invalid PHY ID 0x%X\n", hw->phy_id);
	return -E1000_ERR_PHY;
}

/**
 * e1000_sw_init - Initialize general software structures (struct e1000_adapter)
 *
 * e1000_sw_init initializes the Adapter private data structure.
 * Fields are initialized based on PCI device information and
 * OS network device settings (MTU size).
 **/

static int
e1000_sw_init(struct eth_device *nic, int cardnum)
{
	struct e1000_hw *hw = (typeof(hw)) nic->priv;
	int result;

	/* PCI config space info */
	pci_read_config_word(hw->pdev, PCI_VENDOR_ID, &hw->vendor_id);
	pci_read_config_word(hw->pdev, PCI_DEVICE_ID, &hw->device_id);
	pci_read_config_word(hw->pdev, PCI_SUBSYSTEM_VENDOR_ID,
			     &hw->subsystem_vendor_id);
	pci_read_config_word(hw->pdev, PCI_SUBSYSTEM_ID, &hw->subsystem_id);

	pci_read_config_byte(hw->pdev, PCI_REVISION_ID, &hw->revision_id);
	pci_read_config_word(hw->pdev, PCI_COMMAND, &hw->pci_cmd_word);

	/* identify the MAC */
	result = e1000_set_mac_type(hw);
	if (result) {
		E1000_ERR("Unknown MAC Type\n");
		return result;
	}

	/* lan a vs. lan b settings */
	if (hw->mac_type == e1000_82546)
		/*this also works w/ multiple 82546 cards */
		/*but not if they're intermingled /w other e1000s */
		hw->lan_loc = (cardnum % 2) ? e1000_lan_b : e1000_lan_a;
	else
		hw->lan_loc = e1000_lan_a;

	/* flow control settings */
	hw->fc_high_water = E1000_FC_HIGH_THRESH;
	hw->fc_low_water = E1000_FC_LOW_THRESH;
	hw->fc_pause_time = E1000_FC_PAUSE_TIME;
	hw->fc_send_xon = 1;

	/* Media type - copper or fiber */

	if (hw->mac_type >= e1000_82543) {
		uint32_t status = E1000_READ_REG(hw, STATUS);

		if (status & E1000_STATUS_TBIMODE) {
			DEBUGOUT("fiber interface\n");
			hw->media_type = e1000_media_type_fiber;
		} else {
			DEBUGOUT("copper interface\n");
			hw->media_type = e1000_media_type_copper;
		}
	} else {
		hw->media_type = e1000_media_type_fiber;
	}

	if (hw->mac_type < e1000_82543)
		hw->report_tx_early = 0;
	else
		hw->report_tx_early = 1;

	hw->tbi_compatibility_en = TRUE;
#if 0
	hw->wait_autoneg_complete = FALSE;
	hw->adaptive_ifs = TRUE;

	/* Copper options */
	if (hw->media_type == e1000_media_type_copper) {
		hw->mdix = AUTO_ALL_MODES;
		hw->disable_polarity_correction = FALSE;
	}
#endif
	return E1000_SUCCESS;
}

void
fill_rx(struct e1000_hw *hw)
{
	struct e1000_rx_desc *rd;

	rx_last = rx_tail;
	rd = rx_base + rx_tail;
	rx_tail = (rx_tail + 1) % 8;
	memset(rd, 0, 16);
	rd->buffer_addr = cpu_to_le64((u32) & packet);
	E1000_WRITE_REG(hw, RDT, rx_tail);
}

/**
 * e1000_configure_tx - Configure 8254x Transmit Unit after Reset
 * @adapter: board private structure
 *
 * Configure the Tx unit of the MAC after a reset.
 **/

static void
e1000_configure_tx(struct e1000_hw *hw)
{
	unsigned long ptr;
	unsigned long tctl;
	unsigned long tipg;

	ptr = (u32) tx_pool;
	if (ptr & 0xf)
		ptr = (ptr + 0x10) & (~0xf);

	tx_base = (typeof(tx_base)) ptr;

	E1000_WRITE_REG(hw, TDBAL, (u32) tx_base);
	E1000_WRITE_REG(hw, TDBAH, 0);

	E1000_WRITE_REG(hw, TDLEN, 128);

	/* Setup the HW Tx Head and Tail descriptor pointers */
	E1000_WRITE_REG(hw, TDH, 0);
	E1000_WRITE_REG(hw, TDT, 0);
	tx_tail = 0;

	/* Set the default values for the Tx Inter Packet Gap timer */
	switch (hw->mac_type) {
	case e1000_82542_rev2_0:
	case e1000_82542_rev2_1:
		tipg = DEFAULT_82542_TIPG_IPGT;
		tipg |= DEFAULT_82542_TIPG_IPGR1 << E1000_TIPG_IPGR1_SHIFT;
		tipg |= DEFAULT_82542_TIPG_IPGR2 << E1000_TIPG_IPGR2_SHIFT;
		break;
	default:
		if (hw->media_type == e1000_media_type_fiber)
			tipg = DEFAULT_82543_TIPG_IPGT_FIBER;
		else
			tipg = DEFAULT_82543_TIPG_IPGT_COPPER;
		tipg |= DEFAULT_82543_TIPG_IPGR1 << E1000_TIPG_IPGR1_SHIFT;
		tipg |= DEFAULT_82543_TIPG_IPGR2 << E1000_TIPG_IPGR2_SHIFT;
	}
	E1000_WRITE_REG(hw, TIPG, tipg);
#if 0
	/* Set the Tx Interrupt Delay register */
	E1000_WRITE_REG(hw, TIDV, adapter->tx_int_delay);
	if (hw->mac_type >= e1000_82540)
		E1000_WRITE_REG(hw, TADV, adapter->tx_abs_int_delay);
#endif
	/* Program the Transmit Control Register */
	tctl = E1000_READ_REG(hw, TCTL);
	tctl &= ~E1000_TCTL_CT;
	tctl |= E1000_TCTL_EN | E1000_TCTL_PSP |
	    (E1000_COLLISION_THRESHOLD << E1000_CT_SHIFT);
	E1000_WRITE_REG(hw, TCTL, tctl);

	e1000_config_collision_dist(hw);
#if 0
	/* Setup Transmit Descriptor Settings for this adapter */
	adapter->txd_cmd = E1000_TXD_CMD_IFCS | E1000_TXD_CMD_IDE;

	if (adapter->hw.report_tx_early == 1)
		adapter->txd_cmd |= E1000_TXD_CMD_RS;
	else
		adapter->txd_cmd |= E1000_TXD_CMD_RPS;
#endif
}

/**
 * e1000_setup_rctl - configure the receive control register
 * @adapter: Board private structure
 **/
static void
e1000_setup_rctl(struct e1000_hw *hw)
{
	uint32_t rctl;

	rctl = E1000_READ_REG(hw, RCTL);

	rctl &= ~(3 << E1000_RCTL_MO_SHIFT);

	rctl |= E1000_RCTL_EN | E1000_RCTL_BAM | E1000_RCTL_LBM_NO | E1000_RCTL_RDMTS_HALF;	/* |
												   (hw.mc_filter_type << E1000_RCTL_MO_SHIFT); */

	if (hw->tbi_compatibility_on == 1)
		rctl |= E1000_RCTL_SBP;
	else
		rctl &= ~E1000_RCTL_SBP;

	rctl &= ~(E1000_RCTL_SZ_4096);
#if 0
	switch (adapter->rx_buffer_len) {
	case E1000_RXBUFFER_2048:
	default:
#endif
		rctl |= E1000_RCTL_SZ_2048;
		rctl &= ~(E1000_RCTL_BSEX | E1000_RCTL_LPE);
#if 0
		break;
	case E1000_RXBUFFER_4096:
		rctl |= E1000_RCTL_SZ_4096 | E1000_RCTL_BSEX | E1000_RCTL_LPE;
		break;
	case E1000_RXBUFFER_8192:
		rctl |= E1000_RCTL_SZ_8192 | E1000_RCTL_BSEX | E1000_RCTL_LPE;
		break;
	case E1000_RXBUFFER_16384:
		rctl |= E1000_RCTL_SZ_16384 | E1000_RCTL_BSEX | E1000_RCTL_LPE;
		break;
	}
#endif
	E1000_WRITE_REG(hw, RCTL, rctl);
}

/**
 * e1000_configure_rx - Configure 8254x Receive Unit after Reset
 * @adapter: board private structure
 *
 * Configure the Rx unit of the MAC after a reset.
 **/
static void
e1000_configure_rx(struct e1000_hw *hw)
{
	unsigned long ptr;
	unsigned long rctl;
#if 0
	unsigned long rxcsum;
#endif
	rx_tail = 0;
	/* make sure receives are disabled while setting up the descriptors */
	rctl = E1000_READ_REG(hw, RCTL);
	E1000_WRITE_REG(hw, RCTL, rctl & ~E1000_RCTL_EN);
#if 0
	/* set the Receive Delay Timer Register */

	E1000_WRITE_REG(hw, RDTR, adapter->rx_int_delay);
#endif
	if (hw->mac_type >= e1000_82540) {
#if 0
		E1000_WRITE_REG(hw, RADV, adapter->rx_abs_int_delay);
#endif
		/* Set the interrupt throttling rate.  Value is calculated
		 * as DEFAULT_ITR = 1/(MAX_INTS_PER_SEC * 256ns) */
#define MAX_INTS_PER_SEC        8000
#define DEFAULT_ITR             1000000000/(MAX_INTS_PER_SEC * 256)
		E1000_WRITE_REG(hw, ITR, DEFAULT_ITR);
	}

	/* Setup the Base and Length of the Rx Descriptor Ring */
	ptr = (u32) rx_pool;
	if (ptr & 0xf)
		ptr = (ptr + 0x10) & (~0xf);
	rx_base = (typeof(rx_base)) ptr;
	E1000_WRITE_REG(hw, RDBAL, (u32) rx_base);
	E1000_WRITE_REG(hw, RDBAH, 0);

	E1000_WRITE_REG(hw, RDLEN, 128);

	/* Setup the HW Rx Head and Tail Descriptor Pointers */
	E1000_WRITE_REG(hw, RDH, 0);
	E1000_WRITE_REG(hw, RDT, 0);
#if 0
	/* Enable 82543 Receive Checksum Offload for TCP and UDP */
	if ((adapter->hw.mac_type >= e1000_82543) && (adapter->rx_csum == TRUE)) {
		rxcsum = E1000_READ_REG(hw, RXCSUM);
		rxcsum |= E1000_RXCSUM_TUOFL;
		E1000_WRITE_REG(hw, RXCSUM, rxcsum);
	}
#endif
	/* Enable Receives */

	E1000_WRITE_REG(hw, RCTL, rctl);
	fill_rx(hw);
}

/**************************************************************************
POLL - Wait for a frame
***************************************************************************/
static int
e1000_poll(struct eth_device *nic)
{
	struct e1000_hw *hw = nic->priv;
	struct e1000_rx_desc *rd;
	/* return true if there's an ethernet packet ready to read */
	rd = rx_base + rx_last;
	if (!(le32_to_cpu(rd->status)) & E1000_RXD_STAT_DD)
		return 0;
	/*DEBUGOUT("recv: packet len=%d \n", rd->length); */
	NetReceive((uchar *)packet, le32_to_cpu(rd->length));
	fill_rx(hw);
	return 1;
}

/**************************************************************************
TRANSMIT - Transmit a frame
***************************************************************************/
static int
e1000_transmit(struct eth_device *nic, volatile void *packet, int length)
{
	struct e1000_hw *hw = nic->priv;
	struct e1000_tx_desc *txp;
	int i = 0;

	txp = tx_base + tx_tail;
	tx_tail = (tx_tail + 1) % 8;

	txp->buffer_addr = cpu_to_le64(virt_to_bus(packet));
	txp->lower.data = cpu_to_le32(E1000_TXD_CMD_RPS | E1000_TXD_CMD_EOP |
				      E1000_TXD_CMD_IFCS | length);
	txp->upper.data = 0;
	E1000_WRITE_REG(hw, TDT, tx_tail);

	while (!(le32_to_cpu(txp->upper.data) & E1000_TXD_STAT_DD)) {
		if (i++ > TOUT_LOOP) {
			DEBUGOUT("e1000: tx timeout\n");
			return 0;
		}
		udelay(10);	/* give the nic a chance to write to the register */
	}
	return 1;
}

/*reset function*/
static inline int
e1000_reset(struct eth_device *nic)
{
	struct e1000_hw *hw = nic->priv;

	e1000_reset_hw(hw);
	if (hw->mac_type >= e1000_82544) {
		E1000_WRITE_REG(hw, WUC, 0);
	}
	return e1000_init_hw(nic);
}

/**************************************************************************
DISABLE - Turn off ethernet interface
***************************************************************************/
static void
e1000_disable(struct eth_device *nic)
{
	struct e1000_hw *hw = nic->priv;

	/* Turn off the ethernet interface */
	E1000_WRITE_REG(hw, RCTL, 0);
	E1000_WRITE_REG(hw, TCTL, 0);

	/* Clear the transmit ring */
	E1000_WRITE_REG(hw, TDH, 0);
	E1000_WRITE_REG(hw, TDT, 0);

	/* Clear the receive ring */
	E1000_WRITE_REG(hw, RDH, 0);
	E1000_WRITE_REG(hw, RDT, 0);

	/* put the card in its initial state */
#if 0
	E1000_WRITE_REG(hw, CTRL, E1000_CTRL_RST);
#endif
	mdelay(10);

}

/**************************************************************************
INIT - set up ethernet interface(s)
***************************************************************************/
static int
e1000_init(struct eth_device *nic, bd_t * bis)
{
	struct e1000_hw *hw = nic->priv;
	int ret_val = 0;

	ret_val = e1000_reset(nic);
	if (ret_val < 0) {
		if ((ret_val == -E1000_ERR_NOLINK) ||
		    (ret_val == -E1000_ERR_TIMEOUT)) {
			E1000_ERR("Valid Link not detected\n");
		} else {
			E1000_ERR("Hardware Initialization Failed\n");
		}
		return 0;
	}
	e1000_configure_tx(hw);
	e1000_setup_rctl(hw);
	e1000_configure_rx(hw);
	return 1;
}

/**************************************************************************
PROBE - Look for an adapter, this routine's visible to the outside
You should omit the last argument struct pci_device * for a non-PCI NIC
***************************************************************************/
int
e1000_initialize(bd_t * bis)
{
	pci_dev_t devno;
	int card_number = 0;
	struct eth_device *nic = NULL;
	struct e1000_hw *hw = NULL;
	u32 iobase;
	int idx = 0;
	u32 PciCommandWord;

	while (1) {		/* Find PCI device(s) */
		if ((devno = pci_find_devices(supported, idx++)) < 0) {
			break;
		}

		pci_read_config_dword(devno, PCI_BASE_ADDRESS_0, &iobase);
		iobase &= ~0xf;	/* Mask the bits that say "this is an io addr" */
		DEBUGOUT("e1000#%d: iobase 0x%08x\n", card_number, iobase);

		pci_write_config_dword(devno, PCI_COMMAND,
				       PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER);
		/* Check if I/O accesses and Bus Mastering are enabled. */
		pci_read_config_dword(devno, PCI_COMMAND, &PciCommandWord);
		if (!(PciCommandWord & PCI_COMMAND_MEMORY)) {
			printf("Error: Can not enable MEM access.\n");
			continue;
		} else if (!(PciCommandWord & PCI_COMMAND_MASTER)) {
			printf("Error: Can not enable Bus Mastering.\n");
			continue;
		}

		nic = (struct eth_device *) malloc(sizeof (*nic));
		hw = (struct e1000_hw *) malloc(sizeof (*hw));
		hw->pdev = devno;
		nic->priv = hw;
		nic->iobase = bus_to_phys(devno, iobase);

		sprintf(nic->name, "e1000#%d", card_number);

		/* Are these variables needed? */
#if 0
		hw->fc = e1000_fc_none;
		hw->original_fc = e1000_fc_none;
#else
		hw->fc = e1000_fc_default;
		hw->original_fc = e1000_fc_default;
#endif
		hw->autoneg_failed = 0;
		hw->get_link_status = TRUE;
		hw->hw_addr = (typeof(hw->hw_addr)) iobase;
		hw->mac_type = e1000_undefined;

		/* MAC and Phy settings */
		if (e1000_sw_init(nic, card_number) < 0) {
			free(hw);
			free(nic);
			return 0;
		}
#ifndef CONFIG_AP1000
		if (e1000_validate_eeprom_checksum(nic) < 0) {
			printf("The EEPROM Checksum Is Not Valid\n");
			free(hw);
			free(nic);
			return 0;
		}
#endif
		e1000_read_mac_addr(nic);

		E1000_WRITE_REG(hw, PBA, E1000_DEFAULT_PBA);

		printf("e1000: %02x:%02x:%02x:%02x:%02x:%02x\n",
		       nic->enetaddr[0], nic->enetaddr[1], nic->enetaddr[2],
		       nic->enetaddr[3], nic->enetaddr[4], nic->enetaddr[5]);

		nic->init = e1000_init;
		nic->recv = e1000_poll;
		nic->send = e1000_transmit;
		nic->halt = e1000_disable;

		eth_register(nic);

		card_number++;
	}
	return 1;
}

#endif

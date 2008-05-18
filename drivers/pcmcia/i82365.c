/*
 * (C) Copyright 2003-2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 ********************************************************************
 *
 * Lots of code copied from:
 *
 * i82365.c 1.352 - Linux driver for Intel 82365 and compatible
 * PC Card controllers, and Yenta-compatible PCI-to-CardBus controllers.
 * (C) 1999 David A. Hinds <dahinds@users.sourceforge.net>
 */

#include <common.h>

#include <command.h>
#include <pci.h>
#include <pcmcia.h>
#include <asm/io.h>

#include <pcmcia/ss.h>
#include <pcmcia/i82365.h>
#include <pcmcia/yenta.h>
#ifdef CONFIG_CPC45
#include <pcmcia/cirrus.h>
#else
#include <pcmcia/ti113x.h>
#endif

static struct pci_device_id supported[] = {
#ifdef CONFIG_CPC45
	{PCI_VENDOR_ID_CIRRUS, PCI_DEVICE_ID_CIRRUS_6729},
#else
	{PCI_VENDOR_ID_TI, PCI_DEVICE_ID_TI_1510},
#endif
	{0, 0}
};

#define CYCLE_TIME	120

#ifdef CONFIG_CPC45
extern int SPD67290Init (void);
#endif

#ifdef DEBUG
static void i82365_dump_regions (pci_dev_t dev);
#endif

typedef struct socket_info_t {
	pci_dev_t	dev;
	u_short		bcr;
	u_char		pci_lat, cb_lat, sub_bus, cache;
	u_int		cb_phys;

	socket_cap_t	cap;
	u_short		type;
	u_int		flags;
#ifdef CONFIG_CPC45
	cirrus_state_t	c_state;
#else
	ti113x_state_t	state;
#endif
} socket_info_t;

#ifdef CONFIG_CPC45
/* These definitions must match the pcic table! */
typedef enum pcic_id {
	IS_PD6710, IS_PD672X, IS_VT83C469
} pcic_id;

typedef struct pcic_t {
	char *name;
} pcic_t;

static pcic_t pcic[] = {
	{" Cirrus PD6710: "},
	{" Cirrus PD672x: "},
	{" VIA VT83C469: "},
};
#endif

static socket_info_t socket;
static socket_state_t state;
static struct pccard_mem_map mem;
static struct pccard_io_map io;

/*====================================================================*/

/* Some PCI shortcuts */

static int pci_readb (socket_info_t * s, int r, u_char * v)
{
	return pci_read_config_byte (s->dev, r, v);
}
static int pci_writeb (socket_info_t * s, int r, u_char v)
{
	return pci_write_config_byte (s->dev, r, v);
}
static int pci_readw (socket_info_t * s, int r, u_short * v)
{
	return pci_read_config_word (s->dev, r, v);
}
static int pci_writew (socket_info_t * s, int r, u_short v)
{
	return pci_write_config_word (s->dev, r, v);
}
#ifndef CONFIG_CPC45
static int pci_readl (socket_info_t * s, int r, u_int * v)
{
	return pci_read_config_dword (s->dev, r, v);
}
static int pci_writel (socket_info_t * s, int r, u_int v)
{
	return pci_write_config_dword (s->dev, r, v);
}
#endif	/* !CONFIG_CPC45 */

/*====================================================================*/

#ifdef CONFIG_CPC45

#define cb_readb(s)		readb((s)->cb_phys + 1)
#define cb_writeb(s, v)		writeb(v, (s)->cb_phys)
#define cb_writeb2(s, v)	writeb(v, (s)->cb_phys + 1)
#define cb_readl(s, r)		readl((s)->cb_phys + (r))
#define cb_writel(s, r, v)	writel(v, (s)->cb_phys + (r))


static u_char i365_get (socket_info_t * s, u_short reg)
{
	u_char val;
#ifdef CONFIG_PCMCIA_SLOT_A
	int slot = 0;
#else
	int slot = 1;
#endif

	val = I365_REG (slot, reg);

	cb_writeb (s, val);
	val = cb_readb (s);

	debug ("i365_get slot:%x reg: %x val: %x\n", slot, reg, val);
	return val;
}

static void i365_set (socket_info_t * s, u_short reg, u_char data)
{
#ifdef CONFIG_PCMCIA_SLOT_A
	int slot = 0;
#else
	int slot = 1;
#endif
	u_char val;

	val = I365_REG (slot, reg);

	cb_writeb (s, val);
	cb_writeb2 (s, data);

	debug ("i365_set slot:%x reg: %x data:%x\n", slot, reg, data);
}

#else	/* ! CONFIG_CPC45 */

#define cb_readb(s, r)		readb((s)->cb_phys + (r))
#define cb_readl(s, r)		readl((s)->cb_phys + (r))
#define cb_writeb(s, r, v)	writeb(v, (s)->cb_phys + (r))
#define cb_writel(s, r, v)	writel(v, (s)->cb_phys + (r))

static u_char i365_get (socket_info_t * s, u_short reg)
{
	return cb_readb (s, 0x0800 + reg);
}

static void i365_set (socket_info_t * s, u_short reg, u_char data)
{
	cb_writeb (s, 0x0800 + reg, data);
}
#endif	/* CONFIG_CPC45 */

static void i365_bset (socket_info_t * s, u_short reg, u_char mask)
{
	i365_set (s, reg, i365_get (s, reg) | mask);
}

static void i365_bclr (socket_info_t * s, u_short reg, u_char mask)
{
	i365_set (s, reg, i365_get (s, reg) & ~mask);
}

#if 0	/* not used */
static void i365_bflip (socket_info_t * s, u_short reg, u_char mask, int b)
{
	u_char d = i365_get (s, reg);

	i365_set (s, reg, (b) ? (d | mask) : (d & ~mask));
}

static u_short i365_get_pair (socket_info_t * s, u_short reg)
{
	return (i365_get (s, reg) + (i365_get (s, reg + 1) << 8));
}
#endif	/* not used */

static void i365_set_pair (socket_info_t * s, u_short reg, u_short data)
{
	i365_set (s, reg, data & 0xff);
	i365_set (s, reg + 1, data >> 8);
}

#ifdef CONFIG_CPC45
/*======================================================================

    Code to save and restore global state information for Cirrus
    PD67xx controllers, and to set and report global configuration
    options.

======================================================================*/

#define flip(v,b,f) (v = ((f)<0) ? v : ((f) ? ((v)|(b)) : ((v)&(~b))))

static void cirrus_get_state (socket_info_t * s)
{
	int i;
	cirrus_state_t *p = &s->c_state;

	p->misc1 = i365_get (s, PD67_MISC_CTL_1);
	p->misc1 &= (PD67_MC1_MEDIA_ENA | PD67_MC1_INPACK_ENA);
	p->misc2 = i365_get (s, PD67_MISC_CTL_2);
	for (i = 0; i < 6; i++)
		p->timer[i] = i365_get (s, PD67_TIME_SETUP (0) + i);

}

static void cirrus_set_state (socket_info_t * s)
{
	int i;
	u_char misc;
	cirrus_state_t *p = &s->c_state;

	misc = i365_get (s, PD67_MISC_CTL_2);
	i365_set (s, PD67_MISC_CTL_2, p->misc2);
	if (misc & PD67_MC2_SUSPEND)
		udelay (50000);
	misc = i365_get (s, PD67_MISC_CTL_1);
	misc &= ~(PD67_MC1_MEDIA_ENA | PD67_MC1_INPACK_ENA);
	i365_set (s, PD67_MISC_CTL_1, misc | p->misc1);
	for (i = 0; i < 6; i++)
		i365_set (s, PD67_TIME_SETUP (0) + i, p->timer[i]);
}

static u_int cirrus_set_opts (socket_info_t * s)
{
	cirrus_state_t *p = &s->c_state;
	u_int mask = 0xffff;
#if DEBUG
	char buf[200];

	memset (buf, 0, 200);
#endif

	if (has_ring == -1)
		has_ring = 1;
	flip (p->misc2, PD67_MC2_IRQ15_RI, has_ring);
	flip (p->misc2, PD67_MC2_DYNAMIC_MODE, dynamic_mode);
#if DEBUG
	if (p->misc2 & PD67_MC2_IRQ15_RI)
		strcat (buf, " [ring]");
	if (p->misc2 & PD67_MC2_DYNAMIC_MODE)
		strcat (buf, " [dyn mode]");
	if (p->misc1 & PD67_MC1_INPACK_ENA)
		strcat (buf, " [inpack]");
#endif

	if (p->misc2 & PD67_MC2_IRQ15_RI)
		mask &= ~0x8000;
	if (has_led > 0) {
#if DEBUG
		strcat (buf, " [led]");
#endif
		mask &= ~0x1000;
	}
	if (has_dma > 0) {
#if DEBUG
		strcat (buf, " [dma]");
#endif
		mask &= ~0x0600;
		flip (p->misc2, PD67_MC2_FREQ_BYPASS, freq_bypass);
#if DEBUG
		if (p->misc2 & PD67_MC2_FREQ_BYPASS)
			strcat (buf, " [freq bypass]");
#endif
	}

	if (setup_time >= 0)
		p->timer[0] = p->timer[3] = setup_time;
	if (cmd_time > 0) {
		p->timer[1] = cmd_time;
		p->timer[4] = cmd_time * 2 + 4;
	}
	if (p->timer[1] == 0) {
		p->timer[1] = 6;
		p->timer[4] = 16;
		if (p->timer[0] == 0)
			p->timer[0] = p->timer[3] = 1;
	}
	if (recov_time >= 0)
		p->timer[2] = p->timer[5] = recov_time;

	debug ("i82365 Opt: %s [%d/%d/%d] [%d/%d/%d]\n",
		buf,
		p->timer[0], p->timer[1], p->timer[2],
		p->timer[3], p->timer[4], p->timer[5]);

	return mask;
}

#else	/* !CONFIG_CPC45 */

/*======================================================================

    Code to save and restore global state information for TI 1130 and
    TI 1131 controllers, and to set and report global configuration
    options.

======================================================================*/

static void ti113x_get_state (socket_info_t * s)
{
	ti113x_state_t *p = &s->state;

	pci_readl (s, TI113X_SYSTEM_CONTROL, &p->sysctl);
	pci_readb (s, TI113X_CARD_CONTROL, &p->cardctl);
	pci_readb (s, TI113X_DEVICE_CONTROL, &p->devctl);
	pci_readb (s, TI1250_DIAGNOSTIC, &p->diag);
	pci_readl (s, TI12XX_IRQMUX, &p->irqmux);
}

static void ti113x_set_state (socket_info_t * s)
{
	ti113x_state_t *p = &s->state;

	pci_writel (s, TI113X_SYSTEM_CONTROL, p->sysctl);
	pci_writeb (s, TI113X_CARD_CONTROL, p->cardctl);
	pci_writeb (s, TI113X_DEVICE_CONTROL, p->devctl);
	pci_writeb (s, TI1250_MULTIMEDIA_CTL, 0);
	pci_writeb (s, TI1250_DIAGNOSTIC, p->diag);
	pci_writel (s, TI12XX_IRQMUX, p->irqmux);
	i365_set_pair (s, TI113X_IO_OFFSET (0), 0);
	i365_set_pair (s, TI113X_IO_OFFSET (1), 0);
}

static u_int ti113x_set_opts (socket_info_t * s)
{
	ti113x_state_t *p = &s->state;
	u_int mask = 0xffff;

	p->cardctl &= ~TI113X_CCR_ZVENABLE;
	p->cardctl |= TI113X_CCR_SPKROUTEN;

	return mask;
}
#endif	/* CONFIG_CPC45 */

/*======================================================================

    Routines to handle common CardBus options

======================================================================*/

/* Default settings for PCI command configuration register */
#define CMD_DFLT (PCI_COMMAND_IO|PCI_COMMAND_MEMORY| \
		  PCI_COMMAND_MASTER|PCI_COMMAND_WAIT)

static void cb_get_state (socket_info_t * s)
{
	pci_readb (s, PCI_CACHE_LINE_SIZE, &s->cache);
	pci_readb (s, PCI_LATENCY_TIMER, &s->pci_lat);
	pci_readb (s, CB_LATENCY_TIMER, &s->cb_lat);
	pci_readb (s, CB_CARDBUS_BUS, &s->cap.cardbus);
	pci_readb (s, CB_SUBORD_BUS, &s->sub_bus);
	pci_readw (s, CB_BRIDGE_CONTROL, &s->bcr);
}

static void cb_set_state (socket_info_t * s)
{
#ifndef CONFIG_CPC45
	pci_writel (s, CB_LEGACY_MODE_BASE, 0);
	pci_writel (s, PCI_BASE_ADDRESS_0, s->cb_phys);
#endif
	pci_writew (s, PCI_COMMAND, CMD_DFLT);
	pci_writeb (s, PCI_CACHE_LINE_SIZE, s->cache);
	pci_writeb (s, PCI_LATENCY_TIMER, s->pci_lat);
	pci_writeb (s, CB_LATENCY_TIMER, s->cb_lat);
	pci_writeb (s, CB_CARDBUS_BUS, s->cap.cardbus);
	pci_writeb (s, CB_SUBORD_BUS, s->sub_bus);
	pci_writew (s, CB_BRIDGE_CONTROL, s->bcr);
}

static void cb_set_opts (socket_info_t * s)
{
#ifndef CONFIG_CPC45
	if (s->cache == 0)
		s->cache = 8;
	if (s->pci_lat == 0)
		s->pci_lat = 0xa8;
	if (s->cb_lat == 0)
		s->cb_lat = 0xb0;
#endif
}

/*======================================================================

    Power control for Cardbus controllers: used both for 16-bit and
    Cardbus cards.

======================================================================*/

static int cb_set_power (socket_info_t * s, socket_state_t * state)
{
	u_int reg = 0;

#ifdef CONFIG_CPC45

	reg = I365_PWR_NORESET;
	if (state->flags & SS_PWR_AUTO)
		reg |= I365_PWR_AUTO;
	if (state->flags & SS_OUTPUT_ENA)
		reg |= I365_PWR_OUT;
	if (state->Vpp != 0) {
		if (state->Vpp == 120) {
			reg |= I365_VPP1_12V;
			puts (" 12V card found: ");
		} else if (state->Vpp == state->Vcc) {
			reg |= I365_VPP1_5V;
		} else {
			puts (" power not found: ");
			return -1;
		}
	}
	if (state->Vcc != 0) {
		reg |= I365_VCC_5V;
		if (state->Vcc == 33) {
			puts (" 3.3V card found: ");
			i365_bset (s, PD67_MISC_CTL_1, PD67_MC1_VCC_3V);
		} else if (state->Vcc == 50) {
			puts (" 5V card found: ");
			i365_bclr (s, PD67_MISC_CTL_1, PD67_MC1_VCC_3V);
		} else {
			puts (" power not found: ");
			return -1;
		}
	}

	if (reg != i365_get (s, I365_POWER)) {
		reg = (I365_PWR_OUT | I365_PWR_NORESET | I365_VCC_5V | I365_VPP1_5V);
		i365_set (s, I365_POWER, reg);
	}

#else	/* ! CONFIG_CPC45 */

	/* restart card voltage detection if it seems appropriate */
	if ((state->Vcc == 0) && (state->Vpp == 0) &&
	   !(cb_readl (s, CB_SOCKET_STATE) & CB_SS_VSENSE))
		cb_writel (s, CB_SOCKET_FORCE, CB_SF_CVSTEST);
	switch (state->Vcc) {
	case 0:
		reg = 0;
		break;
	case 33:
		reg = CB_SC_VCC_3V;
		break;
	case 50:
		reg = CB_SC_VCC_5V;
		break;
	default:
		return -1;
	}
	switch (state->Vpp) {
	case 0:
		break;
	case 33:
		reg |= CB_SC_VPP_3V;
		break;
	case 50:
		reg |= CB_SC_VPP_5V;
		break;
	case 120:
		reg |= CB_SC_VPP_12V;
		break;
	default:
		return -1;
	}
	if (reg != cb_readl (s, CB_SOCKET_CONTROL))
		cb_writel (s, CB_SOCKET_CONTROL, reg);
#endif	/* CONFIG_CPC45 */
	return 0;
}

/*======================================================================

    Generic routines to get and set controller options

======================================================================*/

static void get_bridge_state (socket_info_t * s)
{
#ifdef CONFIG_CPC45
	cirrus_get_state (s);
#else
	ti113x_get_state (s);
#endif
	cb_get_state (s);
}

static void set_bridge_state (socket_info_t * s)
{
	cb_set_state (s);
	i365_set (s, I365_GBLCTL, 0x00);
	i365_set (s, I365_GENCTL, 0x00);
#ifdef CONFIG_CPC45
	cirrus_set_state (s);
#else
	ti113x_set_state (s);
#endif
}

static void set_bridge_opts (socket_info_t * s)
{
#ifdef CONFIG_CPC45
	cirrus_set_opts (s);
#else
	ti113x_set_opts (s);
#endif
	cb_set_opts (s);
}

/*====================================================================*/
#define PD67_EXT_INDEX		0x2e	/* Extension index */
#define PD67_EXT_DATA		0x2f	/* Extension data */
#define PD67_EXD_VS1(s)		(0x01 << ((s)<<1))

#define pd67_ext_get(s, r) \
    (i365_set(s, PD67_EXT_INDEX, r), i365_get(s, PD67_EXT_DATA))

static int i365_get_status (socket_info_t * s, u_int * value)
{
	u_int status;
#ifdef CONFIG_CPC45
	u_char val;
	u_char power, vcc, vpp;
	u_int powerstate;
#endif

	status = i365_get (s, I365_IDENT);
	status = i365_get (s, I365_STATUS);
	*value = ((status & I365_CS_DETECT) == I365_CS_DETECT) ? SS_DETECT : 0;
	if (i365_get (s, I365_INTCTL) & I365_PC_IOCARD) {
		*value |= (status & I365_CS_STSCHG) ? 0 : SS_STSCHG;
	} else {
		*value |= (status & I365_CS_BVD1) ? 0 : SS_BATDEAD;
		*value |= (status & I365_CS_BVD2) ? 0 : SS_BATWARN;
	}
	*value |= (status & I365_CS_WRPROT) ? SS_WRPROT : 0;
	*value |= (status & I365_CS_READY) ? SS_READY : 0;
	*value |= (status & I365_CS_POWERON) ? SS_POWERON : 0;

#ifdef CONFIG_CPC45
	/* Check for Cirrus CL-PD67xx chips */
	i365_set (s, PD67_CHIP_INFO, 0);
	val = i365_get (s, PD67_CHIP_INFO);
	s->type = -1;
	if ((val & PD67_INFO_CHIP_ID) == PD67_INFO_CHIP_ID) {
		val = i365_get (s, PD67_CHIP_INFO);
		if ((val & PD67_INFO_CHIP_ID) == 0) {
			s->type = (val & PD67_INFO_SLOTS) ? IS_PD672X : IS_PD6710;
			i365_set (s, PD67_EXT_INDEX, 0xe5);
			if (i365_get (s, PD67_EXT_INDEX) != 0xe5)
				s->type = IS_VT83C469;
		}
	} else {
		printf ("no Cirrus Chip found\n");
		*value = 0;
		return -1;
	}

	power = i365_get (s, I365_POWER);
	state.flags |= (power & I365_PWR_AUTO) ? SS_PWR_AUTO : 0;
	state.flags |= (power & I365_PWR_OUT) ? SS_OUTPUT_ENA : 0;
	vcc = power & I365_VCC_MASK;
	vpp = power & I365_VPP1_MASK;
	state.Vcc = state.Vpp = 0;
	if((vcc== 0) || (vpp == 0)) {
		/*
		 * On the Cirrus we get the info which card voltage
		 * we have in EXTERN DATA and write it to MISC_CTL1
		 */
		powerstate = pd67_ext_get(s, PD67_EXTERN_DATA);
		if (powerstate & PD67_EXD_VS1(0)) {
			/* 5V Card */
			i365_bclr (s, PD67_MISC_CTL_1, PD67_MC1_VCC_3V);
		} else {
			/* 3.3V Card */
			i365_bset (s, PD67_MISC_CTL_1, PD67_MC1_VCC_3V);
		}
		i365_set (s, I365_POWER, (I365_PWR_OUT | I365_PWR_NORESET | I365_VCC_5V | I365_VPP1_5V));
		power = i365_get (s, I365_POWER);
	}
	if (power & I365_VCC_5V) {
		state.Vcc = (i365_get(s, PD67_MISC_CTL_1) & PD67_MC1_VCC_3V) ? 33 : 50;
	}

	if (power == I365_VPP1_12V)
		state.Vpp = 120;

	/* IO card, RESET flags, IO interrupt */
	power = i365_get (s, I365_INTCTL);
	state.flags |= (power & I365_PC_RESET) ? 0 : SS_RESET;
	if (power & I365_PC_IOCARD)
		state.flags |= SS_IOCARD;
	state.io_irq = power & I365_IRQ_MASK;

	/* Card status change mask */
	power = i365_get (s, I365_CSCINT);
	state.csc_mask = (power & I365_CSC_DETECT) ? SS_DETECT : 0;
	if (state.flags & SS_IOCARD)
		state.csc_mask |= (power & I365_CSC_STSCHG) ? SS_STSCHG : 0;
	else {
		state.csc_mask |= (power & I365_CSC_BVD1) ? SS_BATDEAD : 0;
		state.csc_mask |= (power & I365_CSC_BVD2) ? SS_BATWARN : 0;
		state.csc_mask |= (power & I365_CSC_READY) ? SS_READY : 0;
	}
	debug ("i82365: GetStatus(0) = flags %#3.3x, Vcc %d, Vpp %d, "
		"io_irq %d, csc_mask %#2.2x\n", state.flags,
		state.Vcc, state.Vpp, state.io_irq, state.csc_mask);

#else	/* !CONFIG_CPC45 */

	status = cb_readl (s, CB_SOCKET_STATE);
	*value |= (status & CB_SS_32BIT) ? SS_CARDBUS : 0;
	*value |= (status & CB_SS_3VCARD) ? SS_3VCARD : 0;
	*value |= (status & CB_SS_XVCARD) ? SS_XVCARD : 0;
	*value |= (status & CB_SS_VSENSE) ? 0 : SS_PENDING;
	/* For now, ignore cards with unsupported voltage keys */
	if (*value & SS_XVCARD)
		*value &= ~(SS_DETECT | SS_3VCARD | SS_XVCARD);
#endif	/* CONFIG_CPC45 */
	return 0;
}	/* i365_get_status */

static int i365_set_socket (socket_info_t * s, socket_state_t * state)
{
	u_char reg;

	set_bridge_state (s);

	/* IO card, RESET flag */
	reg = 0;
	reg |= (state->flags & SS_RESET) ? 0 : I365_PC_RESET;
	reg |= (state->flags & SS_IOCARD) ? I365_PC_IOCARD : 0;
	i365_set (s, I365_INTCTL, reg);

#ifdef CONFIG_CPC45
	cb_set_power (s, state);

#if 0
	/* Card status change interrupt mask */
	reg = s->cs_irq << 4;
	if (state->csc_mask & SS_DETECT)
		reg |= I365_CSC_DETECT;
	if (state->flags & SS_IOCARD) {
		if (state->csc_mask & SS_STSCHG)
			reg |= I365_CSC_STSCHG;
	} else {
		if (state->csc_mask & SS_BATDEAD)
			reg |= I365_CSC_BVD1;
		if (state->csc_mask & SS_BATWARN)
			reg |= I365_CSC_BVD2;
		if (state->csc_mask & SS_READY)
			reg |= I365_CSC_READY;
	}
	i365_set (s, I365_CSCINT, reg);
	i365_get (s, I365_CSC);
#endif	/* 0 */

#else	/* !CONFIG_CPC45 */

	reg = I365_PWR_NORESET;
	if (state->flags & SS_PWR_AUTO)
		reg |= I365_PWR_AUTO;
	if (state->flags & SS_OUTPUT_ENA)
		reg |= I365_PWR_OUT;

	cb_set_power (s, state);
	reg |= i365_get (s, I365_POWER) & (I365_VCC_MASK | I365_VPP1_MASK);

	if (reg != i365_get (s, I365_POWER))
		i365_set (s, I365_POWER, reg);
#endif	/* CONFIG_CPC45 */

	return 0;
}	/* i365_set_socket */

/*====================================================================*/

static int i365_set_mem_map (socket_info_t * s, struct pccard_mem_map *mem)
{
	u_short base, i;
	u_char map;

	debug ("i82365: SetMemMap(%d, %#2.2x, %d ns, %#5.5lx-%#5.5lx, %#5.5x)\n",
		mem->map, mem->flags, mem->speed,
		mem->sys_start, mem->sys_stop, mem->card_start);

	map = mem->map;
	if ((map > 4) ||
	    (mem->card_start > 0x3ffffff) ||
	    (mem->sys_start > mem->sys_stop) ||
	    (mem->speed > 1000)) {
		return -1;
	}

	/* Turn off the window before changing anything */
	if (i365_get (s, I365_ADDRWIN) & I365_ENA_MEM (map))
		i365_bclr (s, I365_ADDRWIN, I365_ENA_MEM (map));

	/* Take care of high byte, for PCI controllers */
	i365_set (s, CB_MEM_PAGE (map), mem->sys_start >> 24);

	base = I365_MEM (map);
	i = (mem->sys_start >> 12) & 0x0fff;
	if (mem->flags & MAP_16BIT)
		i |= I365_MEM_16BIT;
	if (mem->flags & MAP_0WS)
		i |= I365_MEM_0WS;
	i365_set_pair (s, base + I365_W_START, i);

	i = (mem->sys_stop >> 12) & 0x0fff;
	switch (mem->speed / CYCLE_TIME) {
	case 0:
		break;
	case 1:
		i |= I365_MEM_WS0;
		break;
	case 2:
		i |= I365_MEM_WS1;
		break;
	default:
		i |= I365_MEM_WS1 | I365_MEM_WS0;
		break;
	}
	i365_set_pair (s, base + I365_W_STOP, i);

#ifdef CONFIG_CPC45
	i = 0;
#else
	i = ((mem->card_start - mem->sys_start) >> 12) & 0x3fff;
#endif
	if (mem->flags & MAP_WRPROT)
		i |= I365_MEM_WRPROT;
	if (mem->flags & MAP_ATTRIB)
		i |= I365_MEM_REG;
	i365_set_pair (s, base + I365_W_OFF, i);

#ifdef CONFIG_CPC45
	/* set System Memory map Upper Adress */
	i365_set(s, PD67_EXT_INDEX, PD67_MEM_PAGE(map));
	i365_set(s, PD67_EXT_DATA, ((mem->sys_start >> 24) & 0xff));
#endif

	/* Turn on the window if necessary */
	if (mem->flags & MAP_ACTIVE)
		i365_bset (s, I365_ADDRWIN, I365_ENA_MEM (map));
	return 0;
}	/* i365_set_mem_map */

static int i365_set_io_map (socket_info_t * s, struct pccard_io_map *io)
{
	u_char map, ioctl;

	map = io->map;
	/* comment out: comparison is always false due to limited range of data type */
	if ((map > 1) || /* (io->start > 0xffff) || (io->stop > 0xffff) || */
	    (io->stop < io->start))
		return -1;
	/* Turn off the window before changing anything */
	if (i365_get (s, I365_ADDRWIN) & I365_ENA_IO (map))
		i365_bclr (s, I365_ADDRWIN, I365_ENA_IO (map));
	i365_set_pair (s, I365_IO (map) + I365_W_START, io->start);
	i365_set_pair (s, I365_IO (map) + I365_W_STOP, io->stop);
	ioctl = i365_get (s, I365_IOCTL) & ~I365_IOCTL_MASK (map);
	if (io->speed)
		ioctl |= I365_IOCTL_WAIT (map);
	if (io->flags & MAP_0WS)
		ioctl |= I365_IOCTL_0WS (map);
	if (io->flags & MAP_16BIT)
		ioctl |= I365_IOCTL_16BIT (map);
	if (io->flags & MAP_AUTOSZ)
		ioctl |= I365_IOCTL_IOCS16 (map);
	i365_set (s, I365_IOCTL, ioctl);
	/* Turn on the window if necessary */
	if (io->flags & MAP_ACTIVE)
		i365_bset (s, I365_ADDRWIN, I365_ENA_IO (map));
	return 0;
}	/* i365_set_io_map */

/*====================================================================*/

int i82365_init (void)
{
	u_int val;
	int i;

#ifdef CONFIG_CPC45
	if (SPD67290Init () != 0)
		return 1;
#endif
	if ((socket.dev = pci_find_devices (supported, 0)) < 0) {
		/* Controller not found */
		return 1;
	}
	debug ("i82365 Device Found!\n");

	pci_read_config_dword (socket.dev, PCI_BASE_ADDRESS_0, &socket.cb_phys);
	socket.cb_phys &= ~0xf;

#ifdef CONFIG_CPC45
	/* + 0xfe000000 see MPC 8245 Users Manual Adress Map B */
	socket.cb_phys += 0xfe000000;
#endif

	get_bridge_state (&socket);
	set_bridge_opts (&socket);

	i = i365_get_status (&socket, &val);

#ifdef CONFIG_CPC45
	if (i > -1) {
		puts (pcic[socket.type].name);
	} else {
		printf ("i82365: Controller not found.\n");
		return 1;
	}
	if((val & SS_DETECT) != SS_DETECT){
		puts ("No card\n");
		return 1;
	}
#else	/* !CONFIG_CPC45 */
	if (val & SS_DETECT) {
		if (val & SS_3VCARD) {
			state.Vcc = state.Vpp = 33;
			puts (" 3.3V card found: ");
		} else if (!(val & SS_XVCARD)) {
			state.Vcc = state.Vpp = 50;
			puts (" 5.0V card found: ");
		} else {
			puts ("i82365: unsupported voltage key\n");
			state.Vcc = state.Vpp = 0;
		}
	} else {
		/* No card inserted */
		puts ("No card\n");
		return 1;
	}
#endif	/* CONFIG_CPC45 */

#ifdef CONFIG_CPC45
	state.flags |= SS_OUTPUT_ENA;
#else
	state.flags = SS_IOCARD | SS_OUTPUT_ENA;
	state.csc_mask = 0;
	state.io_irq = 0;
#endif

	i365_set_socket (&socket, &state);

	for (i = 500; i; i--) {
		if ((i365_get (&socket, I365_STATUS) & I365_CS_READY))
			break;
		udelay (1000);
	}

	if (i == 0) {
		/* PC Card not ready for data transfer */
		puts ("i82365 PC Card not ready for data transfer\n");
		return 1;
	}
	debug (" PC Card ready for data transfer: ");

	mem.map = 0;
	mem.flags = MAP_ATTRIB | MAP_ACTIVE;
	mem.speed = 300;
	mem.sys_start = CFG_PCMCIA_MEM_ADDR;
	mem.sys_stop = CFG_PCMCIA_MEM_ADDR + CFG_PCMCIA_MEM_SIZE - 1;
	mem.card_start = 0;
	i365_set_mem_map (&socket, &mem);

#ifdef CONFIG_CPC45
	mem.map = 1;
	mem.flags = MAP_ACTIVE;
	mem.speed = 300;
	mem.sys_start = CFG_PCMCIA_MEM_ADDR + CFG_PCMCIA_MEM_SIZE;
	mem.sys_stop = CFG_PCMCIA_MEM_ADDR + (2 * CFG_PCMCIA_MEM_SIZE) - 1;
	mem.card_start = 0;
	i365_set_mem_map (&socket, &mem);

#else	/* !CONFIG_CPC45 */

	io.map = 0;
	io.flags = MAP_AUTOSZ | MAP_ACTIVE;
	io.speed = 0;
	io.start = 0x0100;
	io.stop = 0x010F;
	i365_set_io_map (&socket, &io);

#endif	/* CONFIG_CPC45 */

#ifdef DEBUG
	i82365_dump_regions (socket.dev);
#endif

	return 0;
}

void i82365_exit (void)
{
	io.map = 0;
	io.flags = 0;
	io.speed = 0;
	io.start = 0;
	io.stop = 0x1;

	i365_set_io_map (&socket, &io);

	mem.map = 0;
	mem.flags = 0;
	mem.speed = 0;
	mem.sys_start = 0;
	mem.sys_stop = 0x1000;
	mem.card_start = 0;

	i365_set_mem_map (&socket, &mem);

#ifdef CONFIG_CPC45
	mem.map = 1;
	mem.flags = 0;
	mem.speed = 0;
	mem.sys_start = 0;
	mem.sys_stop = 0x1000;
	mem.card_start = 0;

	i365_set_mem_map (&socket, &mem);
#else	/* !CONFIG_CPC45 */
	socket.state.sysctl &= 0xFFFF00FF;
#endif
	state.Vcc = state.Vpp = 0;

	i365_set_socket (&socket, &state);
}

/*======================================================================

    Debug stuff

======================================================================*/

#ifdef DEBUG
static void i82365_dump_regions (pci_dev_t dev)
{
	u_int tmp[2];
	u_int *mem = (void *) socket.cb_phys;
	u_char *cis = (void *) CFG_PCMCIA_MEM_ADDR;
	u_char *ide = (void *) (CFG_ATA_BASE_ADDR + CFG_ATA_REG_OFFSET);

	pci_read_config_dword (dev, 0x00, tmp + 0);
	pci_read_config_dword (dev, 0x80, tmp + 1);

	printf ("PCI CONF: %08X ... %08X\n",
		tmp[0], tmp[1]);
	printf ("PCI MEM:  ... %08X ... %08X\n",
		mem[0x8 / 4], mem[0x800 / 4]);
	printf ("CIS:      ...%c%c%c%c%c%c%c%c...\n",
		cis[0x38], cis[0x3a], cis[0x3c], cis[0x3e],
		cis[0x40], cis[0x42], cis[0x44], cis[0x48]);
	printf ("CIS CONF: %02X %02X %02X ...\n",
		cis[0x200], cis[0x202], cis[0x204]);
	printf ("IDE:      %02X %02X %02X %02X %02X %02X %02X %02X\n",
		ide[0], ide[1], ide[2], ide[3],
		ide[4], ide[5], ide[6], ide[7]);
}
#endif	/* DEBUG */

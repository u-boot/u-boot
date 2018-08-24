// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Igor Grinberg <grinberg@compulab.co.il>
 */

#include <common.h>
#include <environment.h>
#include <status_led.h>
#include <net.h>
#include <netdev.h>
#include <usb.h>
#include <mmc.h>
#include <linux/compiler.h>
#include <linux/usb/musb.h>

#include <asm/io.h>
#include <asm/arch/mem.h>
#include <asm/arch/am35x_def.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/musb.h>
#include <asm/omap_musb.h>
#include <asm/ehci-omap.h>

#include "../common/common.h"
#include "../common/eeprom.h"

DECLARE_GLOBAL_DATA_PTR;

const omap3_sysinfo sysinfo = {
	DDR_DISCRETE,
	"CM-T3517 board",
	"NAND 128/512M",
};

#ifdef CONFIG_USB_MUSB_AM35X
static struct musb_hdrc_config cm_t3517_musb_config = {
	.multipoint     = 1,
	.dyn_fifo       = 1,
	.num_eps        = 16,
	.ram_bits       = 12,
};

static struct omap_musb_board_data cm_t3517_musb_board_data = {
	.set_phy_power		= am35x_musb_phy_power,
	.clear_irq		= am35x_musb_clear_irq,
	.reset			= am35x_musb_reset,
};

static struct musb_hdrc_platform_data cm_t3517_musb_pdata = {
#if defined(CONFIG_USB_MUSB_HOST)
	.mode           = MUSB_HOST,
#elif defined(CONFIG_USB_MUSB_GADGET)
	.mode		= MUSB_PERIPHERAL,
#else
#error "Please define either CONFIG_USB_MUSB_HOST or CONFIG_USB_MUSB_GADGET"
#endif
	.config         = &cm_t3517_musb_config,
	.power          = 250,
	.platform_ops	= &am35x_ops,
	.board_data	= &cm_t3517_musb_board_data,
};

static void cm_t3517_musb_init(void)
{
	/*
	 * Set up USB clock/mode in the DEVCONF2 register.
	 * USB2.0 PHY reference clock is 13 MHz
	 */
	clrsetbits_le32(&am35x_scm_general_regs->devconf2,
			CONF2_REFFREQ | CONF2_OTGMODE | CONF2_PHY_GPIOMODE,
			CONF2_REFFREQ_13MHZ | CONF2_SESENDEN |
			CONF2_VBDTCTEN | CONF2_DATPOL);

	if (!musb_register(&cm_t3517_musb_pdata, &cm_t3517_musb_board_data,
			   (void *)AM35XX_IPSS_USBOTGSS_BASE))
		printf("Failed initializing AM35x MUSB!\n");
}
#else
static inline void am3517_evm_musb_init(void) {}
#endif

int board_init(void)
{
	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */

	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

#if defined(CONFIG_LED_STATUS) && defined(CONFIG_LED_STATUS_BOOT_ENABLE)
	status_led_set(CONFIG_LED_STATUS_BOOT, CONFIG_LED_STATUS_ON);
#endif

	cm_t3517_musb_init();

	return 0;
}

/*
 * Routine: get_board_rev
 * Description: read system revision
 */
u32 get_board_rev(void)
{
	return cl_eeprom_get_board_rev(CONFIG_SYS_I2C_EEPROM_BUS);
};

int misc_init_r(void)
{
	cl_print_pcb_info();
	omap_die_id_display();

	return 0;
}

#if defined(CONFIG_MMC)
#define SB_T35_CD_GPIO 144
#define SB_T35_WP_GPIO 59

int board_mmc_init(bd_t *bis)
{
	return omap_mmc_init(0, 0, 0, SB_T35_CD_GPIO, SB_T35_WP_GPIO);
}
#endif

#ifdef CONFIG_DRIVER_TI_EMAC
#define CONTROL_EFUSE_EMAC_LSB  0x48002380
#define CONTROL_EFUSE_EMAC_MSB  0x48002384

static int am3517_get_efuse_enetaddr(u8 *enetaddr)
{
	u32 lsb = __raw_readl(CONTROL_EFUSE_EMAC_LSB);
	u32 msb = __raw_readl(CONTROL_EFUSE_EMAC_MSB);

	enetaddr[0] = (u8)((msb >> 16) & 0xff);
	enetaddr[1] = (u8)((msb >> 8)  & 0xff);
	enetaddr[2] = (u8)(msb & 0xff);
	enetaddr[3] = (u8)((lsb >> 16) & 0xff);
	enetaddr[4] = (u8)((lsb >> 8)  & 0xff);
	enetaddr[5] = (u8)(lsb & 0xff);

	return is_valid_ethaddr(enetaddr);
}

static inline int cm_t3517_init_emac(bd_t *bis)
{
	int ret = cpu_eth_init(bis);

	if (ret > 0)
		return ret;

	printf("Failed initializing EMAC! ");
	return 0;
}
#else /* !CONFIG_DRIVER_TI_EMAC */
static inline int am3517_get_efuse_enetaddr(u8 *enetaddr) { return 1; }
static inline int cm_t3517_init_emac(bd_t *bis) { return 0; }
#endif /* CONFIG_DRIVER_TI_EMAC */

/*
 * Routine: handle_mac_address
 * Description: prepare MAC address for on-board Ethernet.
 */
static int cm_t3517_handle_mac_address(void)
{
	unsigned char enetaddr[6];
	int ret;

	ret = eth_env_get_enetaddr("ethaddr", enetaddr);
	if (ret)
		return 0;

	ret = cl_eeprom_read_mac_addr(enetaddr, CONFIG_SYS_I2C_EEPROM_BUS);
	if (ret) {
		ret = am3517_get_efuse_enetaddr(enetaddr);
		if (ret)
			return ret;
	}

	if (!is_valid_ethaddr(enetaddr))
		return -1;

	return eth_env_set_enetaddr("ethaddr", enetaddr);
}

#define SB_T35_ETH_RST_GPIO 164

/*
 * Routine: board_eth_init
 * Description: initialize module and base-board Ethernet chips
 */
int board_eth_init(bd_t *bis)
{
	int rc = 0, rc1 = 0;

	rc1 = cm_t3517_handle_mac_address();
	if (rc1)
		printf("No MAC address found! ");

	rc1 = cm_t3517_init_emac(bis);
	if (rc1 > 0)
		rc++;

	rc1 = cl_omap3_smc911x_init(0, 4, CONFIG_SMC911X_BASE,
				    NULL, SB_T35_ETH_RST_GPIO);
	if (rc1 > 0)
		rc++;

	return rc;
}

#ifdef CONFIG_USB_EHCI_OMAP
static struct omap_usbhs_board_data cm_t3517_usbhs_bdata = {
	.port_mode[0] = OMAP_EHCI_PORT_MODE_PHY,
	.port_mode[1] = OMAP_EHCI_PORT_MODE_PHY,
	.port_mode[2] = OMAP_USBHS_PORT_MODE_UNUSED,
};

#define CM_T3517_USB_HUB_RESET_GPIO	152
#define SB_T35_USB_HUB_RESET_GPIO	98

int ehci_hcd_init(int index, enum usb_init_type init,
			struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	cl_usb_hub_init(CM_T3517_USB_HUB_RESET_GPIO, "cm-t3517 hub rst");
	cl_usb_hub_init(SB_T35_USB_HUB_RESET_GPIO, "sb-t35 hub rst");

	return omap_ehci_hcd_init(index, &cm_t3517_usbhs_bdata, hccr, hcor);
}

int ehci_hcd_stop(void)
{
	cl_usb_hub_deinit(CM_T3517_USB_HUB_RESET_GPIO);
	cl_usb_hub_deinit(SB_T35_USB_HUB_RESET_GPIO);

	return omap_ehci_hcd_stop();
}
#endif /* CONFIG_USB_EHCI_OMAP */

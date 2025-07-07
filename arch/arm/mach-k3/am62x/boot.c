// SPDX-License-Identifier: GPL-2.0+
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/am62_spl.h>

static u32 __get_backup_bootmedia(u32 devstat)
{
	u32 bkup_bootmode = (devstat & MAIN_DEVSTAT_BACKUP_BOOTMODE_MASK) >>
				MAIN_DEVSTAT_BACKUP_BOOTMODE_SHIFT;
	u32 bkup_bootmode_cfg =
			(devstat & MAIN_DEVSTAT_BACKUP_BOOTMODE_CFG_MASK) >>
				MAIN_DEVSTAT_BACKUP_BOOTMODE_CFG_SHIFT;

	switch (bkup_bootmode) {
	case BACKUP_BOOT_DEVICE_UART:
		return BOOT_DEVICE_UART;

	case BACKUP_BOOT_DEVICE_USB:
		return BOOT_DEVICE_USB;

	case BACKUP_BOOT_DEVICE_ETHERNET:
		return BOOT_DEVICE_ETHERNET;

	case BACKUP_BOOT_DEVICE_MMC:
		if (bkup_bootmode_cfg)
			return BOOT_DEVICE_MMC2;
		return BOOT_DEVICE_MMC1;

	case BACKUP_BOOT_DEVICE_SPI:
		return BOOT_DEVICE_SPI;

	case BACKUP_BOOT_DEVICE_I2C:
		return BOOT_DEVICE_I2C;

	case BACKUP_BOOT_DEVICE_DFU:
		if (bkup_bootmode_cfg & MAIN_DEVSTAT_BACKUP_USB_MODE_MASK)
			return BOOT_DEVICE_USB;
		return BOOT_DEVICE_DFU;
	};

	return BOOT_DEVICE_RAM;
}

static u32 __get_primary_bootmedia(u32 devstat)
{
	u32 bootmode = (devstat & MAIN_DEVSTAT_PRIMARY_BOOTMODE_MASK) >>
				MAIN_DEVSTAT_PRIMARY_BOOTMODE_SHIFT;
	u32 bootmode_cfg = (devstat & MAIN_DEVSTAT_PRIMARY_BOOTMODE_CFG_MASK) >>
				MAIN_DEVSTAT_PRIMARY_BOOTMODE_CFG_SHIFT;

	switch (bootmode) {
	case BOOT_DEVICE_OSPI:
		fallthrough;
	case BOOT_DEVICE_QSPI:
		fallthrough;
	case BOOT_DEVICE_XSPI:
		fallthrough;
	case BOOT_DEVICE_SPI:
		return BOOT_DEVICE_SPI;

	case BOOT_DEVICE_ETHERNET_RGMII:
		fallthrough;
	case BOOT_DEVICE_ETHERNET_RMII:
		return BOOT_DEVICE_ETHERNET;

	case BOOT_DEVICE_EMMC:
		return BOOT_DEVICE_MMC1;

	case BOOT_DEVICE_MMC:
		if ((bootmode_cfg & MAIN_DEVSTAT_PRIMARY_MMC_PORT_MASK) >>
				MAIN_DEVSTAT_PRIMARY_MMC_PORT_SHIFT)
			return BOOT_DEVICE_MMC2;
		return BOOT_DEVICE_MMC1;

	case BOOT_DEVICE_DFU:
		if ((bootmode_cfg & MAIN_DEVSTAT_PRIMARY_USB_MODE_MASK) >>
		    MAIN_DEVSTAT_PRIMARY_USB_MODE_SHIFT)
			return BOOT_DEVICE_USB;
		return BOOT_DEVICE_DFU;

	case BOOT_DEVICE_NOBOOT:
		return BOOT_DEVICE_RAM;
	}

	return bootmode;
}

u32 get_boot_device(void)
{
	u32 devstat = readl(CTRLMMR_MAIN_DEVSTAT);
	u32 bootmode = *(u32 *)(K3_BOOT_PARAM_TABLE_INDEX_OCRAM);
	u32 bootmedia;

	if (bootmode == K3_PRIMARY_BOOTMODE)
		bootmedia = __get_primary_bootmedia(devstat);
	else
		bootmedia = __get_backup_bootmedia(devstat);

	debug("%s: devstat = 0x%x bootmedia = 0x%x bootmode = %d\n",
	      __func__, devstat, bootmedia, bootmode);

	return bootmedia;
}

const char *get_reset_reason(void)
{
	u32 reset_reason = readl(CTRLMMR_MCU_RST_SRC);

	/* After reading reset source register, software must clear it */
	if (reset_reason)
		writel(reset_reason, CTRLMMR_MCU_RST_SRC);

	if (reset_reason == 0 ||
	   (reset_reason & (RST_SRC_SW_MAIN_POR_FROM_MAIN |
			    RST_SRC_SW_MAIN_POR_FROM_MCU |
			    RST_SRC_DS_MAIN_PORZ)))
		return "POR";

	if (reset_reason & (RST_SRC_SAFETY_ERR | RST_SRC_MAIN_ESM_ERR))
		return "ESM";

	if (reset_reason & RST_SRC_DM_WDT_RST)
		return "WDOG";

	if (reset_reason & (RST_SRC_SW_MAIN_WARM_FROM_MAIN |
			    RST_SRC_SW_MAIN_WARM_FROM_MCU  |
			    RST_SRC_SW_MCU_WARM_RST))
		return "RST";

	if (reset_reason & (RST_SRC_SMS_WARM_RST | RST_SRC_SMS_COLD_RST))
		return "DMSC";

	if (reset_reason & RST_SRC_DEBUG_RST)
		return "JTAG";

	if (reset_reason & RST_SRC_THERMAL_RST)
		return "THERMAL";

	if (reset_reason & (RST_SRC_MAIN_RESET_PIN | RST_SRC_MCU_RESET_PIN))
		return "PIN";

	return "UNKNOWN";
}

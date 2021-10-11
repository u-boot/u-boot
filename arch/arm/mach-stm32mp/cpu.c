// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY LOGC_ARCH

#include <common.h>
#include <clk.h>
#include <cpu_func.h>
#include <debug_uart.h>
#include <env.h>
#include <init.h>
#include <log.h>
#include <lmb.h>
#include <misc.h>
#include <net.h>
#include <asm/io.h>
#include <asm/arch/bsec.h>
#include <asm/arch/stm32.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <linux/bitops.h>

/* RCC register */
#define RCC_TZCR		(STM32_RCC_BASE + 0x00)
#define RCC_DBGCFGR		(STM32_RCC_BASE + 0x080C)
#define RCC_BDCR		(STM32_RCC_BASE + 0x0140)
#define RCC_MP_APB5ENSETR	(STM32_RCC_BASE + 0x0208)
#define RCC_MP_AHB5ENSETR	(STM32_RCC_BASE + 0x0210)
#define RCC_BDCR_VSWRST		BIT(31)
#define RCC_BDCR_RTCSRC		GENMASK(17, 16)
#define RCC_DBGCFGR_DBGCKEN	BIT(8)

/* Security register */
#define ETZPC_TZMA1_SIZE	(STM32_ETZPC_BASE + 0x04)
#define ETZPC_DECPROT0		(STM32_ETZPC_BASE + 0x10)

#define TZC_GATE_KEEPER		(STM32_TZC_BASE + 0x008)
#define TZC_REGION_ATTRIBUTE0	(STM32_TZC_BASE + 0x110)
#define TZC_REGION_ID_ACCESS0	(STM32_TZC_BASE + 0x114)

#define TAMP_CR1		(STM32_TAMP_BASE + 0x00)

#define PWR_CR1			(STM32_PWR_BASE + 0x00)
#define PWR_MCUCR		(STM32_PWR_BASE + 0x14)
#define PWR_CR1_DBP		BIT(8)
#define PWR_MCUCR_SBF		BIT(6)

/* DBGMCU register */
#define DBGMCU_IDC		(STM32_DBGMCU_BASE + 0x00)
#define DBGMCU_APB4FZ1		(STM32_DBGMCU_BASE + 0x2C)
#define DBGMCU_APB4FZ1_IWDG2	BIT(2)
#define DBGMCU_IDC_DEV_ID_MASK	GENMASK(11, 0)
#define DBGMCU_IDC_DEV_ID_SHIFT	0
#define DBGMCU_IDC_REV_ID_MASK	GENMASK(31, 16)
#define DBGMCU_IDC_REV_ID_SHIFT	16

/* GPIOZ registers */
#define GPIOZ_SECCFGR		0x54004030

/* boot interface from Bootrom
 * - boot instance = bit 31:16
 * - boot device = bit 15:0
 */
#define BOOTROM_PARAM_ADDR	0x2FFC0078
#define BOOTROM_MODE_MASK	GENMASK(15, 0)
#define BOOTROM_MODE_SHIFT	0
#define BOOTROM_INSTANCE_MASK	 GENMASK(31, 16)
#define BOOTROM_INSTANCE_SHIFT	16

/* Device Part Number (RPN) = OTP_DATA1 lower 8 bits */
#define RPN_SHIFT	0
#define RPN_MASK	GENMASK(7, 0)

/* Package = bit 27:29 of OTP16
 * - 100: LBGA448 (FFI) => AA = LFBGA 18x18mm 448 balls p. 0.8mm
 * - 011: LBGA354 (LCI) => AB = LFBGA 16x16mm 359 balls p. 0.8mm
 * - 010: TFBGA361 (FFC) => AC = TFBGA 12x12mm 361 balls p. 0.5mm
 * - 001: TFBGA257 (LCC) => AD = TFBGA 10x10mm 257 balls p. 0.5mm
 * - others: Reserved
 */
#define PKG_SHIFT	27
#define PKG_MASK	GENMASK(2, 0)

/*
 * early TLB into the .data section so that it not get cleared
 * with 16kB allignment (see TTBR0_BASE_ADDR_MASK)
 */
u8 early_tlb[PGTABLE_SIZE] __section(".data") __aligned(0x4000);

struct lmb lmb;

static void security_init(void)
{
	/* Disable the backup domain write protection */
	/* the protection is enable at each reset by hardware */
	/* And must be disable by software */
	setbits_le32(PWR_CR1, PWR_CR1_DBP);

	while (!(readl(PWR_CR1) & PWR_CR1_DBP))
		;

	/* If RTC clock isn't enable so this is a cold boot then we need
	 * to reset the backup domain
	 */
	if (!(readl(RCC_BDCR) & RCC_BDCR_RTCSRC)) {
		setbits_le32(RCC_BDCR, RCC_BDCR_VSWRST);
		while (!(readl(RCC_BDCR) & RCC_BDCR_VSWRST))
			;
		clrbits_le32(RCC_BDCR, RCC_BDCR_VSWRST);
	}

	/* allow non secure access in Write/Read for all peripheral */
	writel(GENMASK(25, 0), ETZPC_DECPROT0);

	/* Open SYSRAM for no secure access */
	writel(0x0, ETZPC_TZMA1_SIZE);

	/* enable TZC1 TZC2 clock */
	writel(BIT(11) | BIT(12), RCC_MP_APB5ENSETR);

	/* Region 0 set to no access by default */
	/* bit 0 / 16 => nsaid0 read/write Enable
	 * bit 1 / 17 => nsaid1 read/write Enable
	 * ...
	 * bit 15 / 31 => nsaid15 read/write Enable
	 */
	writel(0xFFFFFFFF, TZC_REGION_ID_ACCESS0);
	/* bit 30 / 31 => Secure Global Enable : write/read */
	/* bit 0 / 1 => Region Enable for filter 0/1 */
	writel(BIT(0) | BIT(1) | BIT(30) | BIT(31), TZC_REGION_ATTRIBUTE0);

	/* Enable Filter 0 and 1 */
	setbits_le32(TZC_GATE_KEEPER, BIT(0) | BIT(1));

	/* RCC trust zone deactivated */
	writel(0x0, RCC_TZCR);

	/* TAMP: deactivate the internal tamper
	 * Bit 23 ITAMP8E: monotonic counter overflow
	 * Bit 20 ITAMP5E: RTC calendar overflow
	 * Bit 19 ITAMP4E: HSE monitoring
	 * Bit 18 ITAMP3E: LSE monitoring
	 * Bit 16 ITAMP1E: RTC power domain supply monitoring
	 */
	writel(0x0, TAMP_CR1);

	/* GPIOZ: deactivate the security */
	writel(BIT(0), RCC_MP_AHB5ENSETR);
	writel(0x0, GPIOZ_SECCFGR);
}

/*
 * Debug init
 */
static void dbgmcu_init(void)
{
	/*
	 * Freeze IWDG2 if Cortex-A7 is in debug mode
	 * done in TF-A for TRUSTED boot and
	 * DBGMCU access is controlled by BSEC_DENABLE.DBGSWENABLE
	*/
	if (bsec_dbgswenable()) {
		setbits_le32(RCC_DBGCFGR, RCC_DBGCFGR_DBGCKEN);
		setbits_le32(DBGMCU_APB4FZ1, DBGMCU_APB4FZ1_IWDG2);
	}
}

void spl_board_init(void)
{
	struct udevice *dev;
	int ret;

	dbgmcu_init();

	/* force probe of BSEC driver to shadow the upper OTP */
	ret = uclass_get_device_by_driver(UCLASS_MISC, DM_DRIVER_GET(stm32mp_bsec), &dev);
	if (ret)
		log_warning("BSEC probe failed: %d\n", ret);
}

/* get bootmode from ROM code boot context: saved in TAMP register */
static void update_bootmode(void)
{
	u32 boot_mode;
	u32 bootrom_itf = readl(BOOTROM_PARAM_ADDR);
	u32 bootrom_device, bootrom_instance;

	/* enable TAMP clock = RTCAPBEN */
	writel(BIT(8), RCC_MP_APB5ENSETR);

	/* read bootrom context */
	bootrom_device =
		(bootrom_itf & BOOTROM_MODE_MASK) >> BOOTROM_MODE_SHIFT;
	bootrom_instance =
		(bootrom_itf & BOOTROM_INSTANCE_MASK) >> BOOTROM_INSTANCE_SHIFT;
	boot_mode =
		((bootrom_device << BOOT_TYPE_SHIFT) & BOOT_TYPE_MASK) |
		((bootrom_instance << BOOT_INSTANCE_SHIFT) &
		 BOOT_INSTANCE_MASK);

	/* save the boot mode in TAMP backup register */
	clrsetbits_le32(TAMP_BOOT_CONTEXT,
			TAMP_BOOT_MODE_MASK,
			boot_mode << TAMP_BOOT_MODE_SHIFT);
}

u32 get_bootmode(void)
{
	/* read bootmode from TAMP backup register */
	return (readl(TAMP_BOOT_CONTEXT) & TAMP_BOOT_MODE_MASK) >>
		    TAMP_BOOT_MODE_SHIFT;
}

/*
 * weak function overidde: set the DDR/SYSRAM executable before to enable the
 * MMU and configure DACR, for early early_enable_caches (SPL or pre-reloc)
 */
void dram_bank_mmu_setup(int bank)
{
	struct bd_info *bd = gd->bd;
	int	i;
	phys_addr_t start;
	phys_size_t size;
	bool use_lmb = false;
	enum dcache_option option;

	if (IS_ENABLED(CONFIG_SPL_BUILD)) {
		start = ALIGN_DOWN(STM32_SYSRAM_BASE, MMU_SECTION_SIZE);
		size = ALIGN(STM32_SYSRAM_SIZE, MMU_SECTION_SIZE);
	} else if (gd->flags & GD_FLG_RELOC) {
		/* bd->bi_dram is available only after relocation */
		start = bd->bi_dram[bank].start;
		size =  bd->bi_dram[bank].size;
		use_lmb = true;
	} else {
		/* mark cacheable and executable the beggining of the DDR */
		start = STM32_DDR_BASE;
		size = CONFIG_DDR_CACHEABLE_SIZE;
	}

	for (i = start >> MMU_SECTION_SHIFT;
	     i < (start >> MMU_SECTION_SHIFT) + (size >> MMU_SECTION_SHIFT);
	     i++) {
		option = DCACHE_DEFAULT_OPTION;
		if (use_lmb && lmb_is_reserved_flags(&lmb, i << MMU_SECTION_SHIFT, LMB_NOMAP))
			option = 0; /* INVALID ENTRY in TLB */
		set_section_dcache(i, option);
	}
}
/*
 * initialize the MMU and activate cache in SPL or in U-Boot pre-reloc stage
 * MMU/TLB is updated in enable_caches() for U-Boot after relocation
 * or is deactivated in U-Boot entry function start.S::cpu_init_cp15
 */
static void early_enable_caches(void)
{
	/* I-cache is already enabled in start.S: cpu_init_cp15 */

	if (CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		return;

	if (!(CONFIG_IS_ENABLED(SYS_ICACHE_OFF) && CONFIG_IS_ENABLED(SYS_DCACHE_OFF))) {
		gd->arch.tlb_size = PGTABLE_SIZE;
		gd->arch.tlb_addr = (unsigned long)&early_tlb;
	}

	/* enable MMU (default configuration) */
	dcache_enable();
}

/*
 * Early system init
 */
int arch_cpu_init(void)
{
	u32 boot_mode;

	early_enable_caches();

	/* early armv7 timer init: needed for polling */
	timer_init();

	if (IS_ENABLED(CONFIG_SPL_BUILD)) {
		security_init();
		update_bootmode();
	}
/* reset copro state in SPL, when used, or in U-Boot */
	if (!IS_ENABLED(CONFIG_SPL) || IS_ENABLED(CONFIG_SPL_BUILD)) {
		/* Reset Coprocessor state unless it wakes up from Standby power mode */
		if (!(readl(PWR_MCUCR) & PWR_MCUCR_SBF)) {
			writel(TAMP_COPRO_STATE_OFF, TAMP_COPRO_STATE);
			writel(0, TAMP_COPRO_RSC_TBL_ADDRESS);
		}
	}

	boot_mode = get_bootmode();

	if (IS_ENABLED(CONFIG_CMD_STM32PROG_SERIAL) &&
	    (boot_mode & TAMP_BOOT_DEVICE_MASK) == BOOT_SERIAL_UART)
		gd->flags |= GD_FLG_SILENT | GD_FLG_DISABLE_CONSOLE;
	else if (IS_ENABLED(CONFIG_DEBUG_UART) && IS_ENABLED(CONFIG_SPL_BUILD))
		debug_uart_init();

	return 0;
}

void enable_caches(void)
{
	/* parse device tree when data cache is still activated */
	lmb_init_and_reserve(&lmb, gd->bd, (void *)gd->fdt_blob);

	/* I-cache is already enabled in start.S: icache_enable() not needed */

	/* deactivate the data cache, early enabled in arch_cpu_init() */
	dcache_disable();
	/*
	 * update MMU after relocation and enable the data cache
	 * warning: the TLB location udpated in board_f.c::reserve_mmu
	 */
	dcache_enable();
}

static u32 read_idc(void)
{
	/* DBGMCU access is controlled by BSEC_DENABLE.DBGSWENABLE */
	if (bsec_dbgswenable()) {
		setbits_le32(RCC_DBGCFGR, RCC_DBGCFGR_DBGCKEN);

		return readl(DBGMCU_IDC);
	}

	if (CONFIG_IS_ENABLED(STM32MP15x))
		return CPU_DEV_STM32MP15; /* STM32MP15x and unknown revision */
	else
		return 0x0;
}

u32 get_cpu_dev(void)
{
	return (read_idc() & DBGMCU_IDC_DEV_ID_MASK) >> DBGMCU_IDC_DEV_ID_SHIFT;
}

u32 get_cpu_rev(void)
{
	return (read_idc() & DBGMCU_IDC_REV_ID_MASK) >> DBGMCU_IDC_REV_ID_SHIFT;
}

static u32 get_otp(int index, int shift, int mask)
{
	int ret;
	struct udevice *dev;
	u32 otp = 0;

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(stm32mp_bsec),
					  &dev);

	if (!ret)
		ret = misc_read(dev, STM32_BSEC_SHADOW(index),
				&otp, sizeof(otp));

	return (otp >> shift) & mask;
}

/* Get Device Part Number (RPN) from OTP */
static u32 get_cpu_rpn(void)
{
	return get_otp(BSEC_OTP_RPN, RPN_SHIFT, RPN_MASK);
}

u32 get_cpu_type(void)
{
	return (get_cpu_dev() << 16) | get_cpu_rpn();
}

/* Get Package options from OTP */
u32 get_cpu_package(void)
{
	return get_otp(BSEC_OTP_PKG, PKG_SHIFT, PKG_MASK);
}

static const char * const soc_type[] = {
	"????",
	"151C", "151A", "151F", "151D",
	"153C", "153A", "153F", "153D",
	"157C", "157A", "157F", "157D"
};

static const char * const soc_pkg[] = { "??", "AD", "AC", "AB", "AA" };
static const char * const soc_rev[] = { "?", "A", "B", "Z" };

static void get_cpu_string_offsets(unsigned int *type, unsigned int *pkg,
				   unsigned int *rev)
{
	u32 cpu_type = get_cpu_type();
	u32 ct = cpu_type & ~(BIT(7) | BIT(0));
	u32 cm = ((cpu_type & BIT(7)) >> 6) | (cpu_type & BIT(0));
	u32 cp = get_cpu_package();

	/* Bits 0 and 7 are the ACDF, 00:C 01:A 10:F 11:D */
	switch (ct) {
	case CPU_STM32MP151Cxx:
		*type = cm + 1;
		break;
	case CPU_STM32MP153Cxx:
		*type = cm + 5;
		break;
	case CPU_STM32MP157Cxx:
		*type = cm + 9;
		break;
	default:
		*type = 0;
		break;
	}

	/* Package */
	switch (cp) {
	case PKG_AA_LBGA448:
	case PKG_AB_LBGA354:
	case PKG_AC_TFBGA361:
	case PKG_AD_TFBGA257:
		*pkg = cp;
		break;
	default:
		*pkg = 0;
		break;
	}

	/* Revision */
	switch (get_cpu_rev()) {
	case CPU_REVA:
		*rev = 1;
		break;
	case CPU_REVB:
		*rev = 2;
		break;
	case CPU_REVZ:
		*rev = 3;
		break;
	default:
		*rev = 0;
		break;
	}
}

void get_soc_name(char name[SOC_NAME_SIZE])
{
	unsigned int type, pkg, rev;

	get_cpu_string_offsets(&type, &pkg, &rev);

	snprintf(name, SOC_NAME_SIZE, "STM32MP%s%s Rev.%s",
		 soc_type[type], soc_pkg[pkg], soc_rev[rev]);
}

/* used when CONFIG_DISPLAY_CPUINFO is activated */
int print_cpuinfo(void)
{
	char name[SOC_NAME_SIZE];

	get_soc_name(name);
	printf("CPU: %s\n", name);

	return 0;
}

static void setup_boot_mode(void)
{
	const u32 serial_addr[] = {
		STM32_USART1_BASE,
		STM32_USART2_BASE,
		STM32_USART3_BASE,
		STM32_UART4_BASE,
		STM32_UART5_BASE,
		STM32_USART6_BASE,
		STM32_UART7_BASE,
		STM32_UART8_BASE
	};
	const u32 sdmmc_addr[] = {
		STM32_SDMMC1_BASE,
		STM32_SDMMC2_BASE,
		STM32_SDMMC3_BASE
	};
	char cmd[60];
	u32 boot_ctx = readl(TAMP_BOOT_CONTEXT);
	u32 boot_mode =
		(boot_ctx & TAMP_BOOT_MODE_MASK) >> TAMP_BOOT_MODE_SHIFT;
	unsigned int instance = (boot_mode & TAMP_BOOT_INSTANCE_MASK) - 1;
	u32 forced_mode = (boot_ctx & TAMP_BOOT_FORCED_MASK);
	struct udevice *dev;

	log_debug("%s: boot_ctx=0x%x => boot_mode=%x, instance=%d forced=%x\n",
		  __func__, boot_ctx, boot_mode, instance, forced_mode);
	switch (boot_mode & TAMP_BOOT_DEVICE_MASK) {
	case BOOT_SERIAL_UART:
		if (instance > ARRAY_SIZE(serial_addr))
			break;
		/* serial : search associated node in devicetree */
		sprintf(cmd, "serial@%x", serial_addr[instance]);
		if (uclass_get_device_by_name(UCLASS_SERIAL, cmd, &dev)) {
			/* restore console on error */
			if (IS_ENABLED(CONFIG_CMD_STM32PROG_SERIAL))
				gd->flags &= ~(GD_FLG_SILENT |
					       GD_FLG_DISABLE_CONSOLE);
			log_err("uart%d = %s not found in device tree!\n",
				instance + 1, cmd);
			break;
		}
		sprintf(cmd, "%d", dev_seq(dev));
		env_set("boot_device", "serial");
		env_set("boot_instance", cmd);

		/* restore console on uart when not used */
		if (IS_ENABLED(CONFIG_CMD_STM32PROG_SERIAL) && gd->cur_serial_dev != dev) {
			gd->flags &= ~(GD_FLG_SILENT |
				       GD_FLG_DISABLE_CONSOLE);
			log_info("serial boot with console enabled!\n");
		}
		break;
	case BOOT_SERIAL_USB:
		env_set("boot_device", "usb");
		env_set("boot_instance", "0");
		break;
	case BOOT_FLASH_SD:
	case BOOT_FLASH_EMMC:
		if (instance > ARRAY_SIZE(sdmmc_addr))
			break;
		/* search associated sdmmc node in devicetree */
		sprintf(cmd, "mmc@%x", sdmmc_addr[instance]);
		if (uclass_get_device_by_name(UCLASS_MMC, cmd, &dev)) {
			printf("mmc%d = %s not found in device tree!\n",
			       instance, cmd);
			break;
		}
		sprintf(cmd, "%d", dev_seq(dev));
		env_set("boot_device", "mmc");
		env_set("boot_instance", cmd);
		break;
	case BOOT_FLASH_NAND:
		env_set("boot_device", "nand");
		env_set("boot_instance", "0");
		break;
	case BOOT_FLASH_SPINAND:
		env_set("boot_device", "spi-nand");
		env_set("boot_instance", "0");
		break;
	case BOOT_FLASH_NOR:
		env_set("boot_device", "nor");
		env_set("boot_instance", "0");
		break;
	default:
		env_set("boot_device", "invalid");
		env_set("boot_instance", "");
		log_err("unexpected boot mode = %x\n", boot_mode);
		break;
	}

	switch (forced_mode) {
	case BOOT_FASTBOOT:
		log_info("Enter fastboot!\n");
		env_set("preboot", "env set preboot; fastboot 0");
		break;
	case BOOT_STM32PROG:
		env_set("boot_device", "usb");
		env_set("boot_instance", "0");
		break;
	case BOOT_UMS_MMC0:
	case BOOT_UMS_MMC1:
	case BOOT_UMS_MMC2:
		log_info("Enter UMS!\n");
		instance = forced_mode - BOOT_UMS_MMC0;
		sprintf(cmd, "env set preboot; ums 0 mmc %d", instance);
		env_set("preboot", cmd);
		break;
	case BOOT_RECOVERY:
		env_set("preboot", "env set preboot; run altbootcmd");
		break;
	case BOOT_NORMAL:
		break;
	default:
		log_debug("unexpected forced boot mode = %x\n", forced_mode);
		break;
	}

	/* clear TAMP for next reboot */
	clrsetbits_le32(TAMP_BOOT_CONTEXT, TAMP_BOOT_FORCED_MASK, BOOT_NORMAL);
}

/*
 * If there is no MAC address in the environment, then it will be initialized
 * (silently) from the value in the OTP.
 */
__weak int setup_mac_address(void)
{
	int ret;
	int i;
	u32 otp[2];
	uchar enetaddr[6];
	struct udevice *dev;

	if (!IS_ENABLED(CONFIG_NET))
		return 0;

	/* MAC already in environment */
	if (eth_env_get_enetaddr("ethaddr", enetaddr))
		return 0;

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(stm32mp_bsec),
					  &dev);
	if (ret)
		return ret;

	ret = misc_read(dev, STM32_BSEC_SHADOW(BSEC_OTP_MAC),
			otp, sizeof(otp));
	if (ret < 0)
		return ret;

	for (i = 0; i < 6; i++)
		enetaddr[i] = ((uint8_t *)&otp)[i];

	if (!is_valid_ethaddr(enetaddr)) {
		log_err("invalid MAC address in OTP %pM\n", enetaddr);
		return -EINVAL;
	}
	log_debug("OTP MAC address = %pM\n", enetaddr);
	ret = eth_env_set_enetaddr("ethaddr", enetaddr);
	if (ret)
		log_err("Failed to set mac address %pM from OTP: %d\n", enetaddr, ret);

	return 0;
}

static int setup_serial_number(void)
{
	char serial_string[25];
	u32 otp[3] = {0, 0, 0 };
	struct udevice *dev;
	int ret;

	if (env_get("serial#"))
		return 0;

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(stm32mp_bsec),
					  &dev);
	if (ret)
		return ret;

	ret = misc_read(dev, STM32_BSEC_SHADOW(BSEC_OTP_SERIAL),
			otp, sizeof(otp));
	if (ret < 0)
		return ret;

	sprintf(serial_string, "%08X%08X%08X", otp[0], otp[1], otp[2]);
	env_set("serial#", serial_string);

	return 0;
}

static void setup_soc_type_pkg_rev(void)
{
	unsigned int type, pkg, rev;

	get_cpu_string_offsets(&type, &pkg, &rev);

	env_set("soc_type", soc_type[type]);
	env_set("soc_pkg", soc_pkg[pkg]);
	env_set("soc_rev", soc_rev[rev]);
}

int arch_misc_init(void)
{
	setup_boot_mode();
	setup_mac_address();
	setup_serial_number();
	setup_soc_type_pkg_rev();

	return 0;
}

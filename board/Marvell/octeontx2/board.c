// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#include <common.h>
#include <console.h>
#include <dm.h>
#include <dm/uclass-internal.h>
#include <malloc.h>
#include <errno.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <asm/arch/smc.h>
#include <asm/arch/soc.h>
#include <asm/arch/board.h>
#include <dm/util.h>

DECLARE_GLOBAL_DATA_PTR;

#define BOOTCMD_NAME	"pci-bootcmd"
#define CONSOLE_NAME	"pci-console@0"

extern unsigned long fdt_base_addr;
extern void cgx_intf_shutdown(void);

void cleanup_env_ethaddr(void)
{
	char ename[32];

	for (int i = 0; i < 20; i++) {
		sprintf(ename, i ? "eth%daddr" : "ethaddr", i);
		if (env_get(ename))
			env_set(ename, NULL);
	}
}

void octeontx2_board_get_mac_addr(u8 index, u8 *mac_addr)
{
	u64 tmp_mac, board_mac_addr = fdt_get_board_mac_addr();
	static int board_mac_num;

	board_mac_num = fdt_get_board_mac_cnt();
	if ((!is_zero_ethaddr((u8 *)&board_mac_addr)) && board_mac_num) {
		tmp_mac = board_mac_addr;
		tmp_mac += index;
		tmp_mac = swab64(tmp_mac) >> 16;
		memcpy(mac_addr, (u8 *)&tmp_mac, ARP_HLEN);
		board_mac_num--;
	} else {
		memset(mac_addr, 0, ARP_HLEN);
	}
	debug("%s mac %pM\n", __func__, mac_addr);
}

void board_get_env_spi_bus_cs(int *bus, int *cs)
{
	const void *blob = gd->fdt_blob;
	int env_bus, env_cs;
	int node, preg;

	env_bus = -1;
	env_cs = -1;
	node = fdt_node_offset_by_compatible(blob, -1, "spi-flash");
	while (node > 0) {
		if (fdtdec_get_bool(blob, node, "u-boot,env")) {
			env_cs = fdtdec_get_int(blob, node, "reg", -1);
			preg = fdtdec_get_int(blob,
					      fdt_parent_offset(blob, node),
					      "reg", -1);
			/* SPI node will have PCI addr, so map it */
			if (preg == 0x3000)
				env_bus = 0;
			if (preg == 0x3800)
				env_bus = 1;
			debug("\n Env SPI [bus:cs] [%d:%d]\n",
			      env_bus, env_cs);
			break;
		}
		node = fdt_node_offset_by_compatible(blob, node, "spi-flash");
	}
	if (env_bus == -1)
		debug("\'u-boot,env\' property not found in fdt\n");

	*bus = env_bus;
	*cs = env_cs;
}

void board_quiesce_devices(void)
{
	struct uclass *uc_dev;
	int ret;

	/* Removes all RVU PF devices */
	ret = uclass_get(UCLASS_ETH, &uc_dev);
	if (uc_dev)
		ret = uclass_destroy(uc_dev);
	if (ret)
		printf("couldn't remove rvu pf devices\n");

#ifdef CONFIG_OCTEONTX2_CGX_INTF
	/* Bring down all cgx lmac links */
	cgx_intf_shutdown();
#endif

	/* Removes all CGX and RVU AF devices */
	ret = uclass_get(UCLASS_MISC, &uc_dev);
	if (uc_dev)
		ret = uclass_destroy(uc_dev);
	if (ret)
		printf("couldn't remove misc (cgx/rvu_af) devices\n");

	/* SMC call - removes all LF<->PF mappings */
	smc_disable_rvu_lfs(0);
}

int board_early_init_r(void)
{
	pci_init();
	return 0;
}

int board_init(void)
{
	return 0;
}

int timer_init(void)
{
	return 0;
}

int dram_init(void)
{
	gd->ram_size = smc_dram_size(0);
	gd->ram_size -= CONFIG_SYS_SDRAM_BASE;

	mem_map_fill();

	return 0;
}

#ifdef CONFIG_NET_OCTEONTX2
void board_late_probe_devices(void)
{
	struct udevice *dev;
	int err, cgx_cnt, i;

	switch (read_partnum()) {
	case CN98XX:
		cgx_cnt = 5;
		break;
	case F95MM:
		cgx_cnt = 2;
		break;
	case LOKI:
		cgx_cnt = 4;
		break;
	default:
		cgx_cnt = 3;
		break;
	}
	/* Probe MAC(CGX) and NIC AF devices before Network stack init */
	for (i = 0; i < cgx_cnt; i++) {
		err = dm_pci_find_device(PCI_VENDOR_ID_CAVIUM, 0xA059, i,
					 &dev);
		if (err)
			debug("%s CGX%d device not found\n", __func__, i);
	}
	err = dm_pci_find_device(PCI_VENDOR_ID_CAVIUM, 0xA065, 0, &dev);
	if (err)
		debug("NIC AF device not found\n");
}
#endif

#if (CONFIG_IS_ENABLED(OCTEONTX_SERIAL_BOOTCMD) ||	\
	CONFIG_IS_ENABLED(OCTEONTX_SERIAL_PCIE_CONSOLE)) &&	\
	!CONFIG_IS_ENABLED(CONSOLE_MUX)
# error CONFIG_CONSOLE_MUX must be enabled!
#endif

#if CONFIG_IS_ENABLED(OCTEONTX_SERIAL_BOOTCMD)
static int init_bootcmd_console(void)
{
	int ret = 0;
	char *stdinname = env_get("stdin");
	struct udevice *bootcmd_dev = NULL;
	bool stdin_set;
	char iomux_name[128];

	debug("%s: stdin before: %s\n", __func__,
	      stdinname ? stdinname : "NONE");
	if (!stdinname) {
		env_set("stdin", "serial");
		stdinname = env_get("stdin");
	}
	stdin_set = !!strstr(stdinname, BOOTCMD_NAME);
	ret = uclass_get_device_by_driver(UCLASS_SERIAL,
					  DM_GET_DRIVER(octeontx_bootcmd),
					  &bootcmd_dev);
	if (ret) {
		pr_err("%s: Error getting %s serial class\n", __func__,
		       BOOTCMD_NAME);
	} else if (bootcmd_dev) {
		if (stdin_set)
			strncpy(iomux_name, stdinname, sizeof(iomux_name));
		else
			snprintf(iomux_name, sizeof(iomux_name), "%s,%s",
				 stdinname, bootcmd_dev->name);
		ret = iomux_doenv(stdin, iomux_name);
		if (ret)
			pr_err("%s: Error %d enabling the PCI bootcmd input console \"%s\"\n",
			       __func__, ret, iomux_name);
		if (!stdin_set)
			env_set("stdin", iomux_name);
	}
	debug("%s: Set iomux and stdin to %s (ret: %d)\n",
	      __func__, iomux_name, ret);
	return ret;
}
#endif

#if CONFIG_IS_ENABLED(OCTEONTX_SERIAL_PCIE_CONSOLE)
static int init_pcie_console(void)
{
	int ret = 0;
	char *stdinname = env_get("stdin");
	char *stdoutname = env_get("stdout");
	char *stderrname = env_get("stderr");
	struct udevice *pcie_console_dev = NULL;
	bool stdin_set, stdout_set, stderr_set;
	char iomux_name[128];

	debug("%s: stdin: %s, stdout: %s, stderr: %s\n", __func__, stdinname,
	      stdoutname, stderrname);
	if (!stdinname) {
		env_set("stdin", "serial");
		stdinname = env_get("stdin");
	}
	if (!stdoutname) {
		env_set("stdout", "serial");
		stdoutname = env_get("stdout");
	}
	if (!stderrname) {
		env_set("stderr", "serial");
		stderrname = env_get("stderr");
	}

	if (!stdinname || !stdoutname || !stderrname) {
		printf("%s: Error setting environment variables for serial\n",
		       __func__);
		return -1;
	}

	stdin_set = !!strstr(stdinname, CONSOLE_NAME);
	stdout_set = !!strstr(stdoutname, CONSOLE_NAME);
	stderr_set = !!strstr(stderrname, CONSOLE_NAME);

	pr_debug("stdin: %d, \"%s\", stdout: %d, \"%s\", stderr: %d, \"%s\"\n",
		 stdin_set, stdinname, stdout_set, stdoutname,
		 stderr_set, stderrname);
	ret = uclass_get_device_by_name(UCLASS_SERIAL, CONSOLE_NAME,
					&pcie_console_dev);
	if (ret || !pcie_console_dev) {
		debug("%s: No PCI console device %s found\n", __func__,
		      CONSOLE_NAME);
		return 0;
	}

	if (stdin_set)
		strncpy(iomux_name, stdinname, sizeof(iomux_name));
	else
		snprintf(iomux_name, sizeof(iomux_name), "%s,%s",
			 stdinname, pcie_console_dev->name);

	ret = iomux_doenv(stdin, iomux_name);
	if (ret) {
		pr_err("%s: Error setting I/O stdin MUX to %s\n",
		       __func__, iomux_name);
		return ret;
	}

	if (!stdin_set)
		env_set("stdin", iomux_name);

	if (stdout_set)
		strncpy(iomux_name, stdoutname, sizeof(iomux_name));
	else
		snprintf(iomux_name, sizeof(iomux_name), "%s,%s", stdoutname,
			 pcie_console_dev->name);

	ret = iomux_doenv(stdout, iomux_name);
	if (ret) {
		pr_err("%s: Error setting I/O stdout MUX to %s\n",
		       __func__, iomux_name);
		return ret;
	}
	if (!stdout_set)
		env_set("stdout", iomux_name);

	if (stderr_set)
		strncpy(iomux_name, stderrname, sizeof(iomux_name));
	else
		snprintf(iomux_name, sizeof(iomux_name), "%s,%s", stderrname,
			 pcie_console_dev->name);

	ret = iomux_doenv(stderr, iomux_name);
	if (ret) {
		pr_err("%s: Error setting I/O stderr MUX to %s\n",
		       __func__, iomux_name);
		return ret;
	}

	if (!stderr_set)
		env_set("stderr", iomux_name);

	debug("%s: stdin: %s, stdout: %s, stderr: %s, ret: %d\n",
	      __func__, env_get("stdin"), env_get("stdout"),
	      env_get("stderr"), ret);

	return ret;
}
#endif

/**
 * Board late initialization routine.
 */
int board_late_init(void)
{
	char boardname[32];
	char boardserial[150], boardrev[150];
	long val;
	bool save_env = false;
	const char *str;

	debug("%s()\n", __func__);

	/*
	 * Now that pci_init initializes env device.
	 * Try to cleanup ethaddr env variables, this is needed
	 * as with each boot, configuration of QLM can change.
	 */
	cleanup_env_ethaddr();

	snprintf(boardname, sizeof(boardname), "%s> ", fdt_get_board_model());
	env_set("prompt", boardname);
	set_working_fdt_addr(env_get_hex("fdtcontroladdr", fdt_base_addr));

	str = fdt_get_board_revision();
	if (str) {
		snprintf(boardrev, sizeof(boardrev), "%s", str);
		if (env_get("boardrev") &&
		    strcmp(boardrev, env_get("boardrev")))
			save_env = true;
		env_set("boardrev", boardrev);
	}

	str = fdt_get_board_serial();
	if (str) {
		snprintf(boardserial, sizeof(boardserial), "%s", str);
		if (env_get("serial#") &&
		    strcmp(boardserial, env_get("serial#")))
			save_env = true;
		env_set("serial#", boardserial);
	}

	val = env_get_hex("disable_ooo", 0);
	smc_configure_ooo(val);

#ifdef CONFIG_NET_OCTEONTX2
	board_late_probe_devices();
#endif

#if CONFIG_IS_ENABLED(OCTEONTX_SERIAL_BOOTCMD)
	if (init_bootcmd_console())
		printf("Failed to init bootcmd input\n");
#endif
#if CONFIG_IS_ENABLED(OCTEONTX_SERIAL_PCIE_CONSOLE)
	if (init_pcie_console())
		printf("Failed to init pci console\n");
#endif
	if (save_env)
		env_save();

	return 0;
}

/*
 * Invoked before relocation, so limit to stack variables.
 */
int show_board_info(void)
{
	char *str = NULL;

	if (otx_is_soc(CN96XX))
		str = "CN96XX";
	if (otx_is_soc(CN95XX))
		str = "CN95XX";
	if (otx_is_soc(LOKI))
		str = "LOKI";
	if (otx_is_soc(CN98XX))
		str = "CN98XX";
	if (otx_is_soc(F95MM))
		str = "F95MM";
	printf("OcteonTX2 %s ARM V8 Core\n", str);

	printf("Board: %s\n", fdt_get_board_model());

	return 0;
}

void acquire_flash_arb(bool acquire)
{
	union cpc_boot_ownerx ownerx;

	if (!acquire) {
		ownerx.u = readl(CPC_BOOT_OWNERX(3));
		ownerx.s.boot_req = 0;
		writel(ownerx.u, CPC_BOOT_OWNERX(3));
	} else {
		ownerx.u = 0;
		ownerx.s.boot_req = 1;
		writel(ownerx.u, CPC_BOOT_OWNERX(3));
		udelay(1);
		do {
			ownerx.u = readl(CPC_BOOT_OWNERX(3));
		} while (ownerx.s.boot_wait);
	}
}

#ifdef CONFIG_LAST_STAGE_INIT
int last_stage_init(void)
{
	(void)smc_flsf_fw_booted();
	return 0;
}
#endif

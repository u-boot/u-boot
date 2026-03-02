// SPDX-License-Identifier: GPL-2.0+
#include <command.h>
#include <env.h>
#include <log.h>
#include <firmware/imx/sci/sci.h>
#include <asm/mach-imx/sys_proto.h>
#include <imx_sip.h>
#include <linux/arm-smccc.h>

int sc_pm_setup_uart(sc_rsrc_t uart_rsrc, sc_pm_clock_rate_t clk_rate)
{
	sc_pm_clock_rate_t rate = clk_rate;
	int ret;

	/* Power up UARTn */
	ret = sc_pm_set_resource_power_mode(-1, uart_rsrc, SC_PM_PW_MODE_ON);
	if (ret)
		return ret;

	/* Set UARTn clock root to 'rate' MHz */
	ret = sc_pm_set_clock_rate(-1, uart_rsrc, SC_PM_CLK_PER, &rate);
	if (ret)
		return ret;

	/* Enable UARTn clock root */
	ret = sc_pm_clock_enable(-1, uart_rsrc, SC_PM_CLK_PER, true, false);
	if (ret)
		return ret;

	return 0;
}

void build_info(void)
{
	struct arm_smccc_res res;
	u32 seco_build = 0, seco_commit = 0;
	u32 sc_build = 0, sc_commit = 0;
	ulong atf_commit = 0;

	/* Get SCFW build and commit id */
	sc_misc_build_info(-1, &sc_build, &sc_commit);
	if (!sc_build) {
		printf("SCFW does not support build info\n");
		sc_commit = 0; /* Display 0 if build info not supported */
	}

	/* Get SECO FW build and commit id */
	sc_seco_build_info(-1, &seco_build, &seco_commit);
	if (!seco_build) {
		debug("SECO FW does not support build info\n");
		/* Display 0 when the build info is not supported */
		seco_commit = 0;
	}

	/* Get ARM Trusted Firmware commit id */
	arm_smccc_smc(IMX_SIP_BUILDINFO, IMX_SIP_BUILDINFO_GET_COMMITHASH,
		      0, 0, 0, 0, 0, 0, &res);
	atf_commit = res.a0;
	if (atf_commit == 0xffffffff) {
		debug("ATF does not support build info\n");
		atf_commit = 0x30; /* Display 0 */
	}

	printf("Build: SCFW %08x, SECO-FW %08x, ATF %s\n",
	       sc_commit, seco_commit, (char *)&atf_commit);
}

int do_boottype(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	sc_misc_bt_t boot_type;

	if (argc > 2)
		return CMD_RET_USAGE;

	if (sc_misc_get_boot_type(-1, &boot_type) != 0) {
		puts("boottype cannot be retrieved\n");
		return CMD_RET_FAILURE;
	}

	if (argc > 1)
		printf("Boottype: %d\n", boot_type);

	env_set_ulong("boottype", boot_type);

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(boottype, CONFIG_SYS_MAXARGS, 2, do_boottype,
	   "save current boot-container in env variable 'boottype'",
	   "possible values for boottype:\n"
	   "0: SC_MISC_BT_PRIMARY\n"
	   "1: SC_MISC_BT_SECONDARY\n"
	   "2: SC_MISC_BT_RECOVERY\n"
	   "3: SC_MISC_BT_MANUFACTURE\n"
	   "4: SC_MISC_BT_SERIAL\n"
	   "[print] - print current boottype"
);

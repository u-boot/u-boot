// SPDX-License-Identifier: GPL-2.0+
/*
 * https://spdx.org/licenses
 *
 * Copyright (C) 2019 Marvell International Ltd.
 */

#include <common.h>
#include <command.h>
#include <asm/arch/soc.h>

/*
 *	Usage example:
 *	The PHY register data width is 16 bits.
 *
 *	In order to write some value to the phy register need to do first:
 *	ddr_phy write D0000 0 //take ownership on the PHY CSR
 *	ddr_phy write C0080 3 //enable phy init engine clk
 *
 *	Then:
 *	ddr_phy write 100c0 5 //for example: writing 5 taps of LCDL
 *	to the centralization TX byte0 bit 0
 */

/* this routine reads/writes 'data' from/to specified 'address' offset */
int ddr_phy_access_smc(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	if (argc != 3 && argc != 4)
		goto EXIT;

	if (strcmp(argv[1], "read") == 0) {
		struct pt_regs pregs = {0};

		u32 offset = simple_strtoul(argv[2], NULL, 16);

		pregs.regs[0] = MV_SIP_DDR_PHY_READ;
		pregs.regs[1] = offset;

		smc_call(&pregs);

		printf("\nread val = 0x%x\n", (unsigned int)pregs.regs[1]);
		return CMD_RET_SUCCESS;
	} else if (strcmp(argv[1], "write") == 0) {
		struct pt_regs pregs = {0};

		u32 offset = simple_strtoul(argv[2], NULL, 16);
		u32 data = simple_strtoul(argv[3], NULL, 16);

		pregs.regs[0] = MV_SIP_DDR_PHY_WRITE;
		pregs.regs[1] = offset;
		pregs.regs[2] = data;

		smc_call(&pregs);
		printf("\nsnps_fw_write: data written\n");

		return CMD_RET_SUCCESS;
	}
EXIT:
	printf("usage: ddr_phy <read | write> <offset> [data]\n");
	return CMD_RET_FAILURE;
}

U_BOOT_CMD(ddr_phy, 4, 0, ddr_phy_access_smc,
	   "Access ddr_phy registers",
	   "command <read | write> <offset> [data]\n"
	   "\t-offset- hex value of the address offset"
	   "\t-datd-hex value of the data to write"
	   "Usage Example:\n"
	   "\t-ddr_phy write D0000 3"
	   "\t-ddr_phy read 100c0");

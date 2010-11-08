/*
 * Copyright 2008 Silicon Turnkey Express, Inc.
 * Martha Marx <mmarx@silicontkx.com>
 *
 * ADS5121 IIM (Fusebox) Interface
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

#include <common.h>
#include <command.h>
#include <asm/io.h>

#ifdef CONFIG_CMD_FUSE

DECLARE_GLOBAL_DATA_PTR;

static char cur_bank = '1';

char *iim_err_msg(u32 err)
{
	static char *IIM_errs[] = {
		"Parity Error in cache",
		"Explicit Sense Cycle Error",
		"Write to Locked Register Error",
		"Read Protect Error",
		"Override Protect Error",
		"Write Protect Error"};

	int i;

	if (!err)
		return "";
	for (i = 1; i < 8; i++)
		if (err & (1 << i))
			printf("IIM - %s\n", IIM_errs[i-1]);
	return "";
}

int in_range(int n, int min, int max, char *err, char *usg)
{
	if (n > max || n < min) {
		printf(err);
		printf("Usage:\n%s\n", usg);
		return 0;
	}
	return 1;
}

int ads5121_fuse_read(int bank, int fstart, int num)
{
	iim512x_t *iim = &((immap_t *) CONFIG_SYS_IMMR)->iim;
	u32 *iim_fb, dummy;
	int f, ctr;

	out_be32(&iim->err, in_be32(&iim->err));
	if (bank == 0)
		iim_fb = (u32 *)&(iim->fbac0);
	else
		iim_fb = (u32 *)&(iim->fbac1);
/* try a read to see if Read Protect is set */
	dummy = in_be32(&iim_fb[0]);
	if (in_be32(&iim->err) & IIM_ERR_RPE) {
		printf("\tRead protect fuse is set\n");
		out_be32(&iim->err, IIM_ERR_RPE);
		return 0;
	}
	printf("Reading Bank %d cache\n", bank);
	for (f = fstart, ctr = 0; num > 0; ctr++, num--, f++) {
		if (ctr % 4 == 0)
			printf("F%2d:", f);
		printf("\t%#04x", (u8)(iim_fb[f]));
		if (ctr % 4 == 3)
			printf("\n");
	}
	if (ctr % 4 != 0)
		printf("\n");
}

int ads5121_fuse_override(int bank, int f, u8 val)
{
	iim512x_t *iim = &((immap_t *) CONFIG_SYS_IMMR)->iim;
	u32 *iim_fb;
	u32 iim_stat;
	int i;

	out_be32(&iim->err, in_be32(&iim->err));
	if (bank == 0)
		iim_fb = (u32 *)&(iim->fbac0);
	else
		iim_fb = (u32 *)&(iim->fbac1);
/* try a read to see if Read Protect is set */
	iim_stat = in_be32(&iim_fb[0]);
	if (in_be32(&iim->err) & IIM_ERR_RPE) {
		printf("Read protect fuse is set on bank %d;"
			"Override protect may also be set\n", bank);
		printf("An attempt will be made to override\n");
		out_be32(&iim->err, IIM_ERR_RPE);
	}
	if (iim_stat & IIM_FBAC_FBOP) {
		printf("Override protect fuse is set on bank %d\n", bank);
		return 1;
	}
	if (f > IIM_FMAX) /* reset the entire bank */
		for (i = 0; i < IIM_FMAX + 1; i++)
			out_be32(&iim_fb[i],  0);
	else
		out_be32(&iim_fb[f], val);
	return 0;
}

int ads5121_fuse_prog(cmd_tbl_t *cmdtp, int bank, char *fuseno_bitno)
{
	iim512x_t *iim = &((immap_t *) CONFIG_SYS_IMMR)->iim;
	int f, i, bitno;
	u32 stat, err;

	f = simple_strtol(fuseno_bitno, NULL, 10);
	if (f == 0 && fuseno_bitno[0] != '0')
		f = -1;
	if (!in_range(f, 0, IIM_FMAX,
		"<frow> must be between 0-31\n\n", cmdtp->usage))
		return 1;
	bitno = -1;
	for (i = 0; i < 6; i++) {
		if (fuseno_bitno[i] == '_') {
			bitno = simple_strtol(&(fuseno_bitno[i+1]), NULL, 10);
			if (bitno == 0 && fuseno_bitno[i+1] != '0')
				bitno = -1;
			break;
		}
	}
	if (!in_range(bitno, 0, 7, "Bit number ranges from 0-7\n"
		"Example of <frow_bitno>: \"18_4\" sets bit 4 of row 18\n",
		cmdtp->usage))
		return 1;
	out_be32(&iim->err, in_be32(&iim->err));
	out_be32(&iim->prg_p, IIM_PRG_P_SET);
	out_be32(&iim->ua, IIM_SET_UA(bank, f));
	out_be32(&iim->la, IIM_SET_LA(f, bitno));
#ifdef DEBUG
	printf("Programming disabled with DEBUG defined \n");
	printf(""Set up to pro
	printf("iim.ua = %x; iim.la = %x\n", iim->ua, iim->la);
#else
	out_be32(&iim->fctl, IIM_FCTL_PROG_PULSE | IIM_FCTL_PROG);
	do
		udelay(20);
	while ((stat = in_be32(&iim->stat)) & IIM_STAT_BUSY);
	out_be32(&iim->prg_p, 0);
	err = in_be32(&iim->err);
	if (stat & IIM_STAT_PRGD) {
		if (!(err & (IIM_ERR_WPE | IIM_ERR_WPE))) {
			printf("Fuse is successfully set");
			if (err)
				printf(" - however there are other errors");
			printf("\n");
		}
		iim->stat = 0;
	}
	if (err) {
		iim_err_msg(err);
		out_be32(&iim->err, in_be32(&iim->err));
	}
#endif
}

int ads5121_fuse_sense(int bank, int fstart, int num)
{
	iim512x_t *iim = &((immap_t *) CONFIG_SYS_IMMR)->iim;
	u32 iim_fbac;
	u32 stat, err, err_hold = 0;
	int f, ctr;

	out_be32(&iim->err, in_be32(&iim->err));
	if (bank == 0)
		iim_fbac = in_be32(&iim->fbac0);
	else
		iim_fbac = in_be32(&iim->fbac1);
	if (iim_fbac & IIM_FBAC_FBESP) {
		printf("\tSense Protect disallows this operation\n");
		out_be32(&iim->err, IIM_FBAC_FBESP);
		return 1;
	}
	err = in_be32(&iim->err);
	if (err) {
		iim_err_msg(err);
		err_hold |= err;
	}
	if (err & IIM_ERR_RPE)
		printf("\tRead protect fuse is set; "
			"Sense Protect may be set but will be attempted\n");
	if (err)
		out_be32(&iim->err, err);
	printf("Sensing fuse(s) on Bank %d\n", bank);
	for (f = fstart, ctr = 0; num > 0; ctr++, f++, num--) {
		out_be32(&iim->ua, IIM_SET_UA(bank, f));
		out_be32(&iim->la, IIM_SET_LA(f, 0));
		out_be32(&iim->fctl,  IIM_FCTL_ESNS_N);
		do
			udelay(20);
		while ((stat = in_be32(&iim->stat)) & IIM_STAT_BUSY);
		err = in_be32(&iim->err);
		if (err & IIM_ERR_SNSE) {
			iim_err_msg(err);
			out_be32(&iim->err, IIM_ERR_SNSE);
			return 1;
		}
		if (stat & IIM_STAT_SNSD) {
			out_be32(&iim->stat, 0);
			if (ctr % 4 == 0)
				printf("F%2d:", f);
			printf("\t%#04x", (u8)iim->sdat);
			if (ctr % 4 == 3)
				printf("\n");
		}
		if (err) {
			err_hold |= err;
			out_be32(&iim->err, err);
		}
	}
	if (ctr % 4 != 0)
		printf("\n");
	if (err_hold)
		iim_err_msg(err_hold);

	return 0;
}

int ads5121_fuse_stat(int bank)
{
	iim512x_t *iim = &((immap_t *) CONFIG_SYS_IMMR)->iim;
	u32 iim_fbac;
	u32 err;

	out_be32(&iim->err, in_be32(&iim->err));
	if (bank == 0)
		iim_fbac = in_be32(&iim->fbac0);
	else
		iim_fbac = in_be32(&iim->fbac1);
	err = in_be32(&iim->err);
	if (err)
		iim_err_msg(err);
	if (err & IIM_ERR_RPE  || iim_fbac & IIM_FBAC_FBRP) {
		if (iim_fbac == 0)
			printf("Since protection settings can't be read - "
				"try sensing fuse row 0;\n");
		return 0;
	}
	if (iim_fbac & IIM_PROTECTION)
		printf("Protection Fuses Bank %d = %#04x:\n", bank, iim_fbac);
	else if (!(err & IIM_ERR_RPE))
		printf("No Protection fuses are set\n");
	if (iim_fbac & IIM_FBAC_FBWP)
		printf("\tWrite Protect fuse is set\n");
	if (iim_fbac & IIM_FBAC_FBOP)
		printf("\tOverride Protect fuse is set\n");
	if (iim_fbac & IIM_FBAC_FBESP)
		printf("\tSense Protect Fuse is set\n");
	out_be32(&iim->err, in_be32(&iim->err));

	return 0;
}

int do_ads5121_fuse(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int frow, n, v, bank;

	if (cur_bank == '0')
		bank = 0;
	else
		bank = 1;

	switch (argc) {
	case 0:
	case 1:
		printf("Usage:\n%s\n", cmdtp->usage);
		return 1;
	case 2:
		if (strncmp(argv[1], "stat", 4) == 0)
			return ads5121_fuse_stat(bank);
		if (strncmp(argv[1], "read", 4) == 0)
			return ads5121_fuse_read(bank, 0, IIM_FMAX + 1);
		if (strncmp(argv[1], "sense", 5) == 0)
			return ads5121_fuse_sense(bank, 0, IIM_FMAX + 1);
		if (strncmp(argv[1], "ovride", 6) == 0)
			return ads5121_fuse_override(bank, IIM_FMAX + 1, 0);
		if (strncmp(argv[1], "bank", 4) == 0) {
			printf("Active Fuse Bank is %c\n", cur_bank);
			return 0;
		}
		printf("Usage:\n%s\n", cmdtp->usage);
		return 1;
	case 3:
		if (strncmp(argv[1], "bank", 4) == 0) {
			if (argv[2][0] == '0')
				cur_bank = '0';
			else if (argv[2][0] == '1')
				cur_bank = '1';
			else {
				printf("Usage:\n%s\n", cmdtp->usage);
				return 1;
			}

			printf("Setting Active Fuse Bank to %c\n", cur_bank);
			return 0;
		}
		if (strncmp(argv[1], "prog", 4) == 0)
			return ads5121_fuse_prog(cmdtp, bank, argv[2]);

		frow = (int)simple_strtol(argv[2], NULL, 10);
		if (frow == 0 && argv[2][0] != '0')
			frow = -1;
		if (!in_range(frow, 0, IIM_FMAX,
			"<frow> must be between 0-31\n\n", cmdtp->usage))
			return 1;
		if (strncmp(argv[1], "read", 4) == 0)
			return ads5121_fuse_read(bank, frow, 1);
		if (strncmp(argv[1], "ovride", 6) == 0)
			return ads5121_fuse_override(bank, frow, 0);
		if (strncmp(argv[1], "sense", 5) == 0)
			return ads5121_fuse_sense(bank, frow, 1);
		printf("Usage:\n%s\n", cmdtp->usage);
		return 1;
	case 4:
		frow = (int)simple_strtol(argv[2], NULL, 10);
		if (frow == 0 && argv[2][0] != '0')
			frow = -1;
		if (!in_range(frow, 0, IIM_FMAX,
			"<frow> must be between 0-31\n\n", cmdtp->usage))
			return 1;
		if (strncmp(argv[1], "read", 4) == 0) {
			n = (int)simple_strtol(argv[3], NULL, 10);
			if (!in_range(frow + n, frow + 1, IIM_FMAX + 1,
				"<frow>+<n> must be between 1-32\n\n",
				cmdtp->usage))
				return 1;
			return ads5121_fuse_read(bank, frow, n);
		}
		if (strncmp(argv[1], "ovride", 6) == 0) {
			v = (int)simple_strtol(argv[3], NULL, 10);
			return ads5121_fuse_override(bank, frow, v);
		}
		if (strncmp(argv[1], "sense", 5) == 0) {
			n = (int)simple_strtol(argv[3], NULL, 10);
			if (!in_range(frow + n, frow + 1, IIM_FMAX + 1,
				"<frow>+<n> must be between 1-32\n\n",
				cmdtp->usage))
				return 1;
			return ads5121_fuse_sense(bank, frow, n);
		}
		printf("Usage:\n%s\n", cmdtp->usage);
		return 1;
	default: /* at least 5 args */
		printf("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}
}

U_BOOT_CMD(
	fuse, CONFIG_SYS_MAXARGS, 0, do_ads5121_fuse,
	"   - Read, Sense, Override or Program Fuses\n",
	"bank <n>		- sets active Fuse Bank to 0 or 1\n"
	"			    no args shows current active bank\n"
	"fuse stat		- print active fuse bank's protection status\n"
	"fuse read [<frow> [<n>]] - print <n> fuse rows starting at <frow>\n"
	"			    no args to print entire bank's fuses\n"
	"fuse ovride [<frow> [<v>]]- override fuses at <frow> with <v>\n"
	"			    no <v> defaults to 0 for the row\n"
	"			    no args resets entire bank to 0\n"
	"			  NOTE - settings persist until hard reset\n"
	"fuse sense [<frow>]	- senses current fuse at <frow>\n"
	"			    no args for entire bank\n"
	"fuse prog <frow_bit> 	- program fuse at row <frow>, bit <_bit>\n"
	"			    <frow> is 0-31, <bit> is 0-7; eg. 13_2 \n"
	"			  WARNING - this is permanent"
);
#endif /* CONFIG_CMD_FUSE */

/*
 * (C) Copyright 2009
 * Vipin Kumar, ST Micoelectronics, vipin.kumar@st.com.
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
#include <nand.h>
#include <linux/mtd/nand_ecc.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/spr_nand.h>

static struct fsmc_regs *const fsmc_regs_p =
    (struct fsmc_regs *)CONFIG_SPEAR_FSMCBASE;

static struct nand_ecclayout spear_nand_ecclayout = {
	.eccbytes = 24,
	.eccpos = {2, 3, 4, 18, 19, 20, 34, 35, 36, 50, 51, 52,
		   66, 67, 68, 82, 83, 84, 98, 99, 100, 114, 115, 116},
	.oobfree = {
		    {.offset = 8, .length = 8},
		    {.offset = 24, .length = 8},
		    {.offset = 40, .length = 8},
		    {.offset = 56, .length = 8},
		    {.offset = 72, .length = 8},
		    {.offset = 88, .length = 8},
		    {.offset = 104, .length = 8},
		    {.offset = 120, .length = 8}
		    }
};

static void spear_nand_hwcontrol(struct mtd_info *mtd, int cmd, uint ctrl)
{
	struct nand_chip *this = mtd->priv;
	ulong IO_ADDR_W;

	if (ctrl & NAND_CTRL_CHANGE) {
		IO_ADDR_W = (ulong)this->IO_ADDR_W;

		IO_ADDR_W &= ~(CONFIG_SYS_NAND_CLE | CONFIG_SYS_NAND_ALE);
		if (ctrl & NAND_CLE)
			IO_ADDR_W |= CONFIG_SYS_NAND_CLE;
		if (ctrl & NAND_ALE)
			IO_ADDR_W |= CONFIG_SYS_NAND_ALE;

		if (ctrl & NAND_NCE) {
			writel(readl(&fsmc_regs_p->genmemctrl_pc) |
			       FSMC_ENABLE, &fsmc_regs_p->genmemctrl_pc);
		} else {
			writel(readl(&fsmc_regs_p->genmemctrl_pc) &
			       ~FSMC_ENABLE, &fsmc_regs_p->genmemctrl_pc);
		}
		this->IO_ADDR_W = (void *)IO_ADDR_W;
	}

	if (cmd != NAND_CMD_NONE)
		writeb(cmd, this->IO_ADDR_W);
}

static int spear_read_hwecc(struct mtd_info *mtd,
			    const u_char *data, u_char ecc[3])
{
	u_int ecc_tmp;

	/* read the h/w ECC */
	ecc_tmp = readl(&fsmc_regs_p->genmemctrl_ecc);

	ecc[0] = (u_char) (ecc_tmp & 0xFF);
	ecc[1] = (u_char) ((ecc_tmp & 0xFF00) >> 8);
	ecc[2] = (u_char) ((ecc_tmp & 0xFF0000) >> 16);

	return 0;
}

void spear_enable_hwecc(struct mtd_info *mtd, int mode)
{
	writel(readl(&fsmc_regs_p->genmemctrl_pc) & ~0x80,
	       &fsmc_regs_p->genmemctrl_pc);
	writel(readl(&fsmc_regs_p->genmemctrl_pc) & ~FSMC_ECCEN,
	       &fsmc_regs_p->genmemctrl_pc);
	writel(readl(&fsmc_regs_p->genmemctrl_pc) | FSMC_ECCEN,
	       &fsmc_regs_p->genmemctrl_pc);
}

int spear_nand_init(struct nand_chip *nand)
{
	writel(FSMC_DEVWID_8 | FSMC_DEVTYPE_NAND | FSMC_ENABLE | FSMC_WAITON,
	       &fsmc_regs_p->genmemctrl_pc);
	writel(readl(&fsmc_regs_p->genmemctrl_pc) | FSMC_TCLR_1 | FSMC_TAR_1,
	       &fsmc_regs_p->genmemctrl_pc);
	writel(FSMC_THIZ_1 | FSMC_THOLD_4 | FSMC_TWAIT_6 | FSMC_TSET_0,
	       &fsmc_regs_p->genmemctrl_comm);
	writel(FSMC_THIZ_1 | FSMC_THOLD_4 | FSMC_TWAIT_6 | FSMC_TSET_0,
	       &fsmc_regs_p->genmemctrl_attrib);

	nand->options = 0;
	nand->ecc.mode = NAND_ECC_HW;
	nand->ecc.layout = &spear_nand_ecclayout;
	nand->ecc.size = 512;
	nand->ecc.bytes = 3;
	nand->ecc.calculate = spear_read_hwecc;
	nand->ecc.hwctl = spear_enable_hwecc;
	nand->ecc.correct = nand_correct_data;
	nand->cmd_ctrl = spear_nand_hwcontrol;
	return 0;
}

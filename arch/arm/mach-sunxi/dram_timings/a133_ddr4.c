#include <asm/arch/cpu.h>
#include <asm/arch/dram.h>

void mctl_set_timing_params(const struct dram_para *para)
{
	struct sunxi_mctl_ctl_reg *const mctl_ctl =
		(struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;

	u8 txsr = 4;
	u8 tccd = 3;
	u8 rd2wr = 5;
	u8 tmrd = 4;
	u8 tmrw = 0;
	u8 wrlat = 5;
	u8 rdlat = 7;
	u8 wr2pre = 14;
	u8 dfi_tphy_wrlat = 6;
	u8 dfi_trddata_en = 10;

	u8 tfaw = ns_to_t(35);
	u8 trrd = max(ns_to_t(8), 2);
	u8 txp = max(ns_to_t(6), 2);
	u8 tmrd_pda = max(ns_to_t(10), 8);
	u8 trp = ns_to_t(15);
	u8 trc = ns_to_t(49);
	u8 wr2rd_s = max(ns_to_t(3), 1) + 7;
	u8 tras_min = ns_to_t(34);
	u16 trefi_x32 = ns_to_t(7800) / 32;
	u16 trfc_min = ns_to_t(350);
	u16 txs_x32 = ns_to_t(360) / 32;
	u16 tmod = max(ns_to_t(15), 12);
	u8 tcke = max(ns_to_t(5), 2);
	u8 tcksrx = max(ns_to_t(10), 3);
	u8 txs_abort_x32 = ns_to_t(170) / 32;
	u8 tras_max = ns_to_t(70200) / 1024;

	u8 rd2pre = (trp < 5 ? 9 - trp : 4);
	u8 wr2rd = trrd + 7;
	u8 tckesr = tcke + 1;
	u8 trcd = trp;
	u8 trrd_s = txp;
	u8 tcksre = tcksrx;

	writel(tras_min | tras_max << 8 | tfaw << 16 | wr2pre << 24,
	       &mctl_ctl->dramtmg[0]);
	writel(trc | rd2pre << 8 | txp << 16, &mctl_ctl->dramtmg[1]);
	writel(wr2rd | rd2wr << 8 | rdlat << 16 | wrlat << 24,
	       &mctl_ctl->dramtmg[2]);
	writel(tmod | tmrd << 12 | tmrw << 20, &mctl_ctl->dramtmg[3]);
	writel(trp | trrd << 8 | tccd << 16 | trcd << 24,
	       &mctl_ctl->dramtmg[4]);
	writel(tcke | tckesr << 8 | tcksre << 16 | tcksrx << 24,
	       &mctl_ctl->dramtmg[5]);
	writel((txp + 2) | 0x20 << 16 | 0x20 << 24,
	       &mctl_ctl->dramtmg[6]);
	writel(txs_x32 | 0x10 << 8 | txs_abort_x32 << 16 | txs_abort_x32 << 24,
	       &mctl_ctl->dramtmg[8]);
	writel(wr2rd_s | trrd_s << 8 | 0x2 << 16, &mctl_ctl->dramtmg[9]);
	writel(0xe0c05, &mctl_ctl->dramtmg[10]);
	writel(0x440c021c, &mctl_ctl->dramtmg[11]);
	writel(tmrd_pda, &mctl_ctl->dramtmg[12]);
	writel(0xa100002, &mctl_ctl->dramtmg[13]);
	writel(txsr, &mctl_ctl->dramtmg[14]);

	clrsetbits_le32(&mctl_ctl->init[0], 0xc0000fff, 1008);
	writel(0x1f20000, &mctl_ctl->init[1]);
	clrsetbits_le32(&mctl_ctl->init[2], 0xff0f, 0xd05);
	writel(0, &mctl_ctl->dfimisc);

	writel(0x840 << 16 | 0x601, &mctl_ctl->init[3]);	/* MR0 / MR1 */
	writel(0x8 << 16 | 0x0, &mctl_ctl->init[4]);		/* MR2 / MR3 */
	writel(0x0 << 16 | 0x400, &mctl_ctl->init[6]);		/* MR4 / MR5 */
	writel(0x826, &mctl_ctl->init[7]);			/* MR6 */

	clrsetbits_le32(&mctl_ctl->rankctl, 0xff0, 0x660);
	writel((dfi_tphy_wrlat - 1) | 0x2000000 | (dfi_trddata_en - 1) << 16 |
	       0x808000, &mctl_ctl->dfitmg0);
	writel(0x100202, &mctl_ctl->dfitmg1);
	writel(trfc_min | trefi_x32 << 16, &mctl_ctl->rfshtmg);
}

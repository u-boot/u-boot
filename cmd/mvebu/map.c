// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#include <config.h>
#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <linux/sizes.h>
#include <mach/soc.h>

#define ADDRESS_SHIFT			(20 - 4)
#define WIN_ENABLE_BIT			(0x1)
#define ADDRESS_RSHIFT			(26)
#define ADDRESS_LSHIFT			(10)

#define MVEBU_CCU_BASE(ap)		(MVEBU_REGS_BASE_AP(ap) + 0x4000)
#define MVEBU_AR_RFU_BASE(ap)		(MVEBU_REGS_BASE_AP(ap) + 0x6F0000)
#define MVEBU_IO_WIN_BASE(ap)		(MVEBU_AR_RFU_BASE(ap))
#define MVEBU_GWIN_BASE(ap)		(MVEBU_REGS_BASE_AP(ap) + 0x5400)

/* CCU registers definitions */
#define CCU_WIN_CR_OFFSET(ap, win)	(MVEBU_CCU_BASE(ap) + \
						0x0 + (0x10 * win))
#define CCU_TARGET_ID_OFFSET		(8)
#define CCU_TARGET_ID_MASK		(0x7F)

#define CCU_WIN_ALR_OFFSET(ap, win)	(MVEBU_CCU_BASE(ap) + \
						0x8 + (0x10 * win))
#define CCU_WIN_AHR_OFFSET(ap, win)	(MVEBU_CCU_BASE(ap) + \
						0xC + (0x10 * win))
#define CCU_WIN_GCR_OFFSET(ap)		(MVEBU_CCU_BASE(ap) + 0xD0)
#define CCU_GCR_TARGET_OFFSET		(8)
#define CCU_GCR_TARGET_MASK		(0xFF)

/* IO Win registers definitions */
#define IO_WIN_ALR_OFFSET(ap, win)	(MVEBU_IO_WIN_BASE(ap) + \
						0x0 + (0x10 * win))
#define IO_WIN_AHR_OFFSET(ap, win)	(MVEBU_IO_WIN_BASE(ap) + \
						0x8 + (0x10 * win))
#define IO_WIN_CR_OFFSET(ap, win)	(MVEBU_IO_WIN_BASE(ap) + \
						0xC + (0x10 * win))

/* GWin registers definitions */
#define GWIN_CR_OFFSET(ap, win)		(MVEBU_GWIN_BASE(ap) + \
						0x0 + (0x10 * (win)))
#define GWIN_ALR_OFFSET(ap, win)	(MVEBU_GWIN_BASE(ap) + \
						0x8 + (0x10 * (win)))
#define GWIN_AHR_OFFSET(ap, win)	(MVEBU_GWIN_BASE(ap) + \
						0xc + (0x10 * (win)))
/* IOB registers definitions */
#define MVEBU_IOB_OFFSET		(0x190000)
#define MVEBU_IOB_BASE(ap, cp)		(MVEBU_REGS_BASE_CP(ap, cp) + \
						MVEBU_IOB_OFFSET)
#define MVEBU_IOB_MAX_WINS		(16)

#define IOB_WIN_CR_OFFSET(ap, cp, win)	(MVEBU_IOB_BASE(ap, cp) + \
						0x0 + (0x20 * win))
#define IOB_TARGET_ID_OFFSET		(8)
#define IOB_TARGET_ID_MASK		(0xF)
#define IOB_WIN_ALR_OFFSET(ap, cp, win)	(MVEBU_IOB_BASE(ap, cp) + \
						0x8 + (0x20 * win))
#define IOB_WIN_AHR_OFFSET(ap, cp, win)	(MVEBU_IOB_BASE(ap, cp) + \
						0xC + (0x20 * win))

/* AMB address decoding registers definitions */
#define MVEBU_AMB_ADEC_OFFSET		(0x70ff00)
#define MVEBU_AMB_ADEC_BASE(ap, cp)	(MVEBU_REGS_BASE_CP(ap, cp) + \
						MVEBU_AMB_ADEC_OFFSET)
#define AMB_MAX_WIN_ID			(7)
#define AMB_WIN_CR_OFFSET(ap, cp, win)	(MVEBU_AMB_ADEC_BASE(ap, cp) \
						+ 0x0 + (0x8 * win))
#define AMB_ATTR_OFFSET			8
#define AMB_ATTR_MASK			0xFF
#define AMB_SIZE_OFFSET			16
#define AMB_SIZE_MASK			0xFF

#define AMB_WIN_BASE_OFFSET(ap, cp, win) (MVEBU_AMB_ADEC_BASE(ap, cp) + \
						0x4 + 0x8 * (win))
#define AMB_BASE_OFFSET			(16)
#define AMB_BASE_ADDR_MASK		((1 << (32 - AMB_BASE_OFFSET)) - 1)

static void dump_ccu(int ap)
{
	u32 win_id, win_cr, alr, ahr;
	u8 target_id;
	u64 start, end;

	/* Dump all CCU windows */
	printf("-----------\n");
	printf("AP-%d CCU:\n", ap);
	printf("bank  id target   start		     end\n");
	printf("----------------------------------------------------\n");
	for (win_id = 0; win_id < MVEBU_CCU_MAX_WINS; win_id++) {
		win_cr = readl(CCU_WIN_CR_OFFSET(ap, win_id));
		if (win_cr & WIN_ENABLE_BIT) {
			printf("ccu   %02x", win_id);
			target_id = (win_cr >> CCU_TARGET_ID_OFFSET) &
						CCU_TARGET_ID_MASK;
			alr = readl(CCU_WIN_ALR_OFFSET(ap, win_id));
			ahr = readl(CCU_WIN_AHR_OFFSET(ap, win_id));
			start = ((u64)alr << ADDRESS_SHIFT);
			end = (((u64)ahr + 0x10) << ADDRESS_SHIFT);
			printf(" %02x  0x%016llx 0x%016llx\n",
			       target_id, start, end);
		}
	}
	win_cr = readl(CCU_WIN_GCR_OFFSET(ap));
	target_id = (win_cr >> CCU_GCR_TARGET_OFFSET) & CCU_GCR_TARGET_MASK;
	printf("ccu   GCR 0x%x\n", target_id);
}

static void dump_io_win(int ap)
{
	u32 trgt_id, win_id;
	u32 alr, ahr;
	u64 start, end;

	/* Dump all IO windows */
	printf("-----------\n");
	printf("AP-%d IOW:\n", ap);
	printf("bank  target     start              end\n");
	printf("----------------------------------------------------\n");
	for (win_id = 0; win_id < MVEBU_IO_WIN_MAX_WINS; win_id++) {
		alr = readl(IO_WIN_ALR_OFFSET(ap, win_id));
		if (alr & WIN_ENABLE_BIT) {
			alr &= ~WIN_ENABLE_BIT;
			ahr = readl(IO_WIN_AHR_OFFSET(ap, win_id));
			trgt_id = readl(IO_WIN_CR_OFFSET(ap, win_id));
			start = ((u64)alr << ADDRESS_SHIFT);
			end = (((u64)ahr + 0x10) << ADDRESS_SHIFT);
			printf("io-win %d  0x%016llx 0x%016llx\n",
			       trgt_id, start, end);
		}
	}
	printf("io-win GCR - 0x%x\n",
	       readl(MVEBU_IO_WIN_BASE(ap) + MVEBU_IO_WIN_GCR_OFFSET));
}

static void dump_gwin(int ap)
{
#if defined(CONFIG_ARMADA_8K_PLUS)
	u32 win_num;

	/* Dump all GWIN windows */
	printf("-----------\n");
	printf("AP-%d GWIN:\n", ap);
	printf("win\ttarget\tstart\t\t\tend\n");
	printf("----------------------------------------------------\n");
	for (win_num = 0; win_num < MVEBU_GWIN_MAX_WINS; win_num++) {
		u32 cr;
		u64 alr, ahr;

		cr  = readl(GWIN_CR_OFFSET(ap, win_num));
		/* Window enabled */
		if (cr & WIN_ENABLE_BIT) {
			printf("%02d\t %02d\t ", win_num, (cr >> 8) & 0xF);
			alr = readl(GWIN_ALR_OFFSET(ap, win_num));
			alr = (alr >> ADDRESS_LSHIFT) << ADDRESS_RSHIFT;
			ahr = readl(GWIN_AHR_OFFSET(ap, win_num));
			ahr = (ahr >> ADDRESS_LSHIFT) << ADDRESS_RSHIFT;
			printf("0x%016llx 0x%016llx\n", alr, ahr);
		}
	}
#endif
}

static void dump_iob(int ap, int cp)
{
	u32 win_id, win_cr, alr, ahr;
	u8 target_id;
	u64 start, end;

	static const char * const iob_target_name[] = {"CFG  ", "MCI0 ",
			"PEX1 ", "PEX2 ", "PEX0 ", "NAND ", "RUNIT", "MCI1 "};

	/* Dump all IOB windows */
	printf("AP-%d CP-%d IOB:\n", ap, cp);
	printf("bank  id target  start              end\n");
	printf("----------------------------------------------------\n");
	for (win_id = 0; win_id < MVEBU_IOB_MAX_WINS; win_id++) {
		win_cr = readl(IOB_WIN_CR_OFFSET(ap, cp, win_id));
		if (win_cr & WIN_ENABLE_BIT) {
			target_id = (win_cr >> IOB_TARGET_ID_OFFSET) &
						IOB_TARGET_ID_MASK;
			alr = readl(IOB_WIN_ALR_OFFSET(ap, cp, win_id));
			start = ((u64)alr << ADDRESS_SHIFT);
			if (win_id != 0) {
				ahr = readl(IOB_WIN_AHR_OFFSET(ap, cp, win_id));
				end = (((u64)ahr + 0x10) << ADDRESS_SHIFT);
			} else {
				/* Window #0 size is hardcoded to 16MB, as it's
				 * reserved for CP configuration space.
				 */
				end = start + (16 << 20);
			}
			printf("iob   %02d %s   0x%016llx 0x%016llx\n",
			       win_id, iob_target_name[target_id], start, end);
		}
	}
}

static void dump_amb_adec(int ap, int cp)
{
	u32 ctrl, base, win_id, attr;
	u32 size, size_count;

	/* Dump all AMB windows */
	printf("AP-%d CP-%d AMB addr.decode:\n", ap, cp);
	printf("bank  attribute     base          size\n");
	printf("--------------------------------------------\n");
	for (win_id = 0; win_id < AMB_MAX_WIN_ID; win_id++) {
		ctrl = readl(AMB_WIN_CR_OFFSET(ap, cp, win_id));
		if (ctrl & WIN_ENABLE_BIT) {
			base = readl(AMB_WIN_BASE_OFFSET(ap, cp, win_id));
			attr = (ctrl >> AMB_ATTR_OFFSET) & AMB_ATTR_MASK;
			size_count = (ctrl >> AMB_SIZE_OFFSET) & AMB_SIZE_MASK;
			size = (size_count + 1) * SZ_64K;
			printf("amb   0x%04x        0x%08x    0x%08x\n",
			       attr, base, size);
		}
	}
}

int do_map_cmd(cmd_tbl_t *cmdtp, int flag, int argc,
	       char * const argv[])
{
	int ap, cp, ap_num, cp_num;

	soc_get_ap_cp_num(&ap_num, &cp_num);

	for (ap = 0; ap < ap_num; ap++) {
		printf("AP-%d address decoding:\n", ap);
		dump_ccu(ap);
		dump_io_win(ap);
		dump_gwin(ap);
		for (cp = 0; cp < (cp_num / ap_num); cp++) {
			printf("\nAP-%d CP-%d address decoding:\n", ap, cp);
			dump_iob(ap, cp);
			dump_amb_adec(ap, cp);
		}
	}

	return 0;
}

U_BOOT_CMD(
	map,      1,     1,      do_map_cmd,
	"Display address decode windows\n",
	"\tDisplay address decode windows\n"
);

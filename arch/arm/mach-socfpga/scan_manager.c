/*
 *  Copyright (C) 2013 Altera Corporation <www.altera.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/arch/freeze_controller.h>
#include <asm/arch/scan_manager.h>

/*
 * Maximum polling loop to wait for IO scan chain engine becomes idle
 * to prevent infinite loop. It is important that this is NOT changed
 * to delay using timer functions, since at the time this function is
 * called, timer might not yet be inited.
 */
#define SCANMGR_MAX_DELAY		100

/*
 * Maximum length of TDI_TDO packet payload is 128 bits,
 * represented by (length - 1) in TDI_TDO header.
 */
#define TDI_TDO_MAX_PAYLOAD		127

#define SCANMGR_STAT_ACTIVE		(1 << 31)
#define SCANMGR_STAT_WFIFOCNT_MASK	0x70000000

DECLARE_GLOBAL_DATA_PTR;

static const struct socfpga_scan_manager *scan_manager_base =
		(void *)(SOCFPGA_SCANMGR_ADDRESS);
static const struct socfpga_freeze_controller *freeze_controller_base =
		(void *)(SOCFPGA_SYSMGR_ADDRESS + SYSMGR_FRZCTRL_ADDRESS);

/**
 * scan_chain_engine_is_idle() - Check if the JTAG scan chain is idle
 * @max_iter:	Maximum number of iterations to wait for idle
 *
 * Function to check IO scan chain engine status and wait if the engine is
 * is active. Poll the IO scan chain engine till maximum iteration reached.
 */
static u32 scan_chain_engine_is_idle(u32 max_iter)
{
	const u32 mask = SCANMGR_STAT_ACTIVE | SCANMGR_STAT_WFIFOCNT_MASK;
	u32 status;

	/* Poll the engine until the scan engine is inactive. */
	do {
		status = readl(&scan_manager_base->stat);
		if (!(status & mask))
			return 0;
	} while (max_iter--);

	return -ETIMEDOUT;
}

#define JTAG_BP_INSN		(1 << 0)
#define JTAG_BP_TMS		(1 << 1)
#define JTAG_BP_PAYLOAD		(1 << 2)
#define JTAG_BP_2BYTE		(1 << 3)
#define JTAG_BP_4BYTE		(1 << 4)

/**
 * scan_mgr_jtag_io() - Access the JTAG chain
 * @flags:	Control flags, used to configure the action on the JTAG
 * @iarg:	Instruction argument
 * @parg:	Payload argument or data
 *
 * Perform I/O on the JTAG chain
 */
static void scan_mgr_jtag_io(const u32 flags, const u8 iarg, const u32 parg)
{
	u32 data = parg;

	if (flags & JTAG_BP_INSN) {	/* JTAG instruction */
		/*
		 * The SCC JTAG register is LSB first, so make
		 * space for the instruction at the LSB.
		 */
		data <<= 8;
		if (flags & JTAG_BP_TMS) {
			data |= (0 << 7);	/* TMS instruction. */
			data |= iarg & 0x3f;	/* TMS arg is 6 bits. */
			if (flags & JTAG_BP_PAYLOAD)
				data |= (1 << 6);
		} else {
			data |= (1 << 7);	/* TDI/TDO instruction. */
			data |= iarg & 0xf;	/* TDI/TDO arg is 4 bits. */
			if (flags & JTAG_BP_PAYLOAD)
				data |= (1 << 4);
		}
	}

	if (flags & JTAG_BP_4BYTE)
		writel(data, &scan_manager_base->fifo_quad_byte);
	else if (flags & JTAG_BP_2BYTE)
		writel(data & 0xffff, &scan_manager_base->fifo_double_byte);
	else
		writel(data & 0xff, &scan_manager_base->fifo_single_byte);
}

/**
 * scan_mgr_io_scan_chain_prg() - Program HPS IO Scan Chain
 * @io_scan_chain_id:		IO scan chain ID
 */
static int scan_mgr_io_scan_chain_prg(const unsigned int io_scan_chain_id)
{
	u32 residual;
	u32 io_scan_chain_len_in_bits;
	const unsigned long *iocsr_scan_chain;
	int i, ret, index = 0;

	ret = iocsr_get_config_table(io_scan_chain_id, &iocsr_scan_chain,
				     &io_scan_chain_len_in_bits);
	if (ret)
		return 1;

	/*
	 * De-assert reinit if the IO scan chain is intended for HIO. In
	 * this, its the chain 3.
	 */
	if (io_scan_chain_id == 3)
		clrbits_le32(&freeze_controller_base->hioctrl,
			     SYSMGR_FRZCTRL_HIOCTRL_DLLRST_MASK);

	/*
	 * Check if the scan chain engine is inactive and the
	 * WFIFO is empty before enabling the IO scan chain
	 */
	ret = scan_chain_engine_is_idle(SCANMGR_MAX_DELAY);
	if (ret)
		return ret;

	/*
	 * Enable IO Scan chain based on scan chain id
	 * Note: only one chain can be enabled at a time
	 */
	setbits_le32(&scan_manager_base->en, 1 << io_scan_chain_id);

	/* Program IO scan chain in 128-bit iteration */
	for (i = 0; i < io_scan_chain_len_in_bits / 128; i++) {
		/* Write TDI_TDO packet header for 128-bit IO scan chain */
		scan_mgr_jtag_io(JTAG_BP_INSN | JTAG_BP_2BYTE, 0x0,
				 TDI_TDO_MAX_PAYLOAD);

		/* write 4 successive 32-bit IO scan chain data into WFIFO */
		scan_mgr_jtag_io(JTAG_BP_4BYTE, 0x0, iocsr_scan_chain[index++]);
		scan_mgr_jtag_io(JTAG_BP_4BYTE, 0x0, iocsr_scan_chain[index++]);
		scan_mgr_jtag_io(JTAG_BP_4BYTE, 0x0, iocsr_scan_chain[index++]);
		scan_mgr_jtag_io(JTAG_BP_4BYTE, 0x0, iocsr_scan_chain[index++]);

		/*
		 * Check if the scan chain engine has completed the
		 * IO scan chain data shifting
		 */
		ret = scan_chain_engine_is_idle(SCANMGR_MAX_DELAY);
		if (ret)
			goto error;
	}

	residual = io_scan_chain_len_in_bits % 128;

	/* Final TDI_TDO packet (if chain length is not aligned to 128 bits) */
	if (residual) {
		/*
		 * Program the last part of IO scan chain write TDI_TDO
		 * packet header (2 bytes) to scan manager.
		 */
		scan_mgr_jtag_io(JTAG_BP_INSN | JTAG_BP_2BYTE, 0x0,
				 residual - 1);

		for (i = 0; i < residual / 32; i++) {
			/*
			 * write remaining scan chain data into scan
			 * manager WFIFO with 4 bytes write
			 */
			scan_mgr_jtag_io(JTAG_BP_4BYTE, 0x0,
					 iocsr_scan_chain[index++]);
		}

		residual = io_scan_chain_len_in_bits % 32;
		if (residual > 24) {
			/*
			 * write the last 4B scan chain data
			 * into scan manager WFIFO
			 */
			scan_mgr_jtag_io(JTAG_BP_4BYTE, 0x0,
					 iocsr_scan_chain[index]);
		} else {
			/*
			 * write the remaining 1 - 3 bytes scan chain
			 * data into scan manager WFIFO byte by byte
			 * to prevent JTAG engine shifting unused data
			 * from the FIFO and mistaken the data as a
			 * valid command (even though unused bits are
			 * set to 0, but just to prevent hardware
			 * glitch)
			 */
			for (i = 0; i < residual; i += 8) {
				scan_mgr_jtag_io(0, 0x0,
					 iocsr_scan_chain[index] >> i);
			}
		}

		/*
		 * Check if the scan chain engine has completed the
		 * IO scan chain data shifting
		 */
		ret = scan_chain_engine_is_idle(SCANMGR_MAX_DELAY);
		if (ret)
			goto error;
	}

	/* Disable IO Scan chain when configuration done*/
	clrbits_le32(&scan_manager_base->en, 1 << io_scan_chain_id);
	return 0;

error:
	/* Disable IO Scan chain when error detected */
	clrbits_le32(&scan_manager_base->en, 1 << io_scan_chain_id);
	return ret;
}

int scan_mgr_configure_iocsr(void)
{
	int status = 0;

	/* configure the IOCSR through scan chain */
	status |= scan_mgr_io_scan_chain_prg(0);
	status |= scan_mgr_io_scan_chain_prg(1);
	status |= scan_mgr_io_scan_chain_prg(2);
	status |= scan_mgr_io_scan_chain_prg(3);
	return status;
}

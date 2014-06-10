/*
 *  Copyright (C) 2013 Altera Corporation <www.altera.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/freeze_controller.h>
#include <asm/arch/scan_manager.h>

DECLARE_GLOBAL_DATA_PTR;

static const struct socfpga_scan_manager *scan_manager_base =
		(void *)(SOCFPGA_SCANMGR_ADDRESS);
static const struct socfpga_freeze_controller *freeze_controller_base =
		(void *)(SOCFPGA_SYSMGR_ADDRESS + SYSMGR_FRZCTRL_ADDRESS);

/*
 * Function to check IO scan chain engine status and wait if the engine is
 * is active. Poll the IO scan chain engine till maximum iteration reached.
 */
static inline uint32_t scan_chain_engine_is_idle(uint32_t max_iter)
{
	uint32_t scanmgr_status;

	scanmgr_status = readl(&scan_manager_base->stat);

	/* Poll the engine until the scan engine is inactive */
	while (SCANMGR_STAT_ACTIVE_GET(scanmgr_status) ||
	      (SCANMGR_STAT_WFIFOCNT_GET(scanmgr_status) > 0)) {
		max_iter--;
		if (max_iter > 0)
			scanmgr_status = readl(&scan_manager_base->stat);
		else
			return 0;
	}
	return 1;
}

/* Program HPS IO Scan Chain */
uint32_t scan_mgr_io_scan_chain_prg(
	uint32_t io_scan_chain_id,
	uint32_t io_scan_chain_len_in_bits,
	const uint32_t *iocsr_scan_chain)
{
	uint16_t tdi_tdo_header;
	uint32_t io_program_iter;
	uint32_t io_scan_chain_data_residual;
	uint32_t residual;
	uint32_t i;
	uint32_t index = 0;

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
	if (!scan_chain_engine_is_idle(SCAN_MAX_DELAY))
		return 1;

	/*
	 * Enable IO Scan chain based on scan chain id
	 * Note: only one chain can be enabled at a time
	 */
	setbits_le32(&scan_manager_base->en, 1 << io_scan_chain_id);

	/*
	 * Calculate number of iteration needed for full 128-bit (4 x32-bits)
	 * bits shifting. Each TDI_TDO packet can shift in maximum 128-bits
	 */
	io_program_iter	= io_scan_chain_len_in_bits >>
		IO_SCAN_CHAIN_128BIT_SHIFT;
	io_scan_chain_data_residual = io_scan_chain_len_in_bits &
		IO_SCAN_CHAIN_128BIT_MASK;

	/* Construct TDI_TDO packet for 128-bit IO scan chain (2 bytes) */
	tdi_tdo_header = TDI_TDO_HEADER_FIRST_BYTE |
		(TDI_TDO_MAX_PAYLOAD <<	TDI_TDO_HEADER_SECOND_BYTE_SHIFT);

	/* Program IO scan chain in 128-bit iteration */
	for (i = 0; i < io_program_iter; i++) {
		/* write TDI_TDO packet header to scan manager */
		writel(tdi_tdo_header,	&scan_manager_base->fifo_double_byte);

		/* calculate array index. Multiply by 4 as write 4 x 32bits */
		index = i * 4;

		/* write 4 successive 32-bit IO scan chain data into WFIFO */
		writel(iocsr_scan_chain[index],
		       &scan_manager_base->fifo_quad_byte);
		writel(iocsr_scan_chain[index + 1],
		       &scan_manager_base->fifo_quad_byte);
		writel(iocsr_scan_chain[index + 2],
		       &scan_manager_base->fifo_quad_byte);
		writel(iocsr_scan_chain[index + 3],
		       &scan_manager_base->fifo_quad_byte);

		/*
		 * Check if the scan chain engine has completed the
		 * IO scan chain data shifting
		 */
		if (!scan_chain_engine_is_idle(SCAN_MAX_DELAY))
			goto error;
	}

	/* Calculate array index for final TDI_TDO packet */
	index = io_program_iter * 4;

	/* Final TDI_TDO packet if any */
	if (io_scan_chain_data_residual) {
		/*
		 * Calculate number of quad bytes FIFO write
		 * needed for the final TDI_TDO packet
		 */
		io_program_iter	= io_scan_chain_data_residual >>
			IO_SCAN_CHAIN_32BIT_SHIFT;

		/*
		 * Construct TDI_TDO packet for remaining IO
		 * scan chain (2 bytes)
		 */
		tdi_tdo_header	= TDI_TDO_HEADER_FIRST_BYTE |
			((io_scan_chain_data_residual - 1) <<
			TDI_TDO_HEADER_SECOND_BYTE_SHIFT);

		/*
		 * Program the last part of IO scan chain write TDI_TDO packet
		 * header (2 bytes) to scan manager
		 */
		writel(tdi_tdo_header, &scan_manager_base->fifo_double_byte);

		for (i = 0; i < io_program_iter; i++) {
			/*
			 * write remaining scan chain data into scan
			 * manager WFIFO with 4 bytes write
			*/
			writel(iocsr_scan_chain[index + i],
			       &scan_manager_base->fifo_quad_byte);
		}

		index += io_program_iter;
		residual = io_scan_chain_data_residual &
			IO_SCAN_CHAIN_32BIT_MASK;

		if (IO_SCAN_CHAIN_PAYLOAD_24BIT < residual) {
			/*
			 * write the last 4B scan chain data
			 * into scan manager WFIFO
			 */
			writel(iocsr_scan_chain[index],
			       &scan_manager_base->fifo_quad_byte);
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
				writel(((iocsr_scan_chain[index] >> i)
					& IO_SCAN_CHAIN_BYTE_MASK),
					&scan_manager_base->fifo_single_byte);
			}
		}

		/*
		 * Check if the scan chain engine has completed the
		 * IO scan chain data shifting
		 */
		if (!scan_chain_engine_is_idle(SCAN_MAX_DELAY))
			goto error;
	}

	/* Disable IO Scan chain when configuration done*/
	clrbits_le32(&scan_manager_base->en, 1 << io_scan_chain_id);
	return 0;

error:
	/* Disable IO Scan chain when error detected */
	clrbits_le32(&scan_manager_base->en, 1 << io_scan_chain_id);
	return 1;
}

int scan_mgr_configure_iocsr(void)
{
	int status = 0;

	/* configure the IOCSR through scan chain */
	status |= scan_mgr_io_scan_chain_prg(0,
		CONFIG_HPS_IOCSR_SCANCHAIN0_LENGTH, iocsr_scan_chain0_table);
	status |= scan_mgr_io_scan_chain_prg(1,
		CONFIG_HPS_IOCSR_SCANCHAIN1_LENGTH, iocsr_scan_chain1_table);
	status |= scan_mgr_io_scan_chain_prg(2,
		CONFIG_HPS_IOCSR_SCANCHAIN2_LENGTH, iocsr_scan_chain2_table);
	status |= scan_mgr_io_scan_chain_prg(3,
		CONFIG_HPS_IOCSR_SCANCHAIN3_LENGTH, iocsr_scan_chain3_table);
	return status;
}

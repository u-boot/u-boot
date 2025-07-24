// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007
 * Eran Liberty, Extricom , eran.liberty@gmail.com
 */

#include <altera.h>
#include <stratixII.h>
#include <linux/delay.h>

/****************************************************************/
/* Stratix II Generic Implementation                            */
int StratixII_ps_fpp_dump(Altera_desc *desc, const void *buf, size_t bsize)
{
	printf("Stratix II Fast Passive Parallel dump is not implemented\n");
	return FPGA_FAIL;
}

int StratixII_ps_fpp_load(Altera_desc *desc, const void *buf, size_t bsize,
			  int isSerial, int isSecure)
{
	altera_board_specific_func *fns;
	int cookie;
	int ret_val = FPGA_FAIL;
	int bytecount;
	const char *buff = buf;
	int i;

	if (!desc) {
		log_err("Altera_desc missing\n");
		return FPGA_FAIL;
	}
	if (!buff) {
		log_err("buffer is missing\n");
		return FPGA_FAIL;
	}
	if (!bsize) {
		log_err("size is zero\n");
		return FPGA_FAIL;
	}
	if (!desc->iface_fns) {
		log_err("Altera_desc function interface table is missing\n");
		return FPGA_FAIL;
	}
	fns = (altera_board_specific_func *) (desc->iface_fns);
	cookie = desc->cookie;

	if (!
	    (fns->config && fns->status && fns->done && fns->data
	     && fns->abort)) {
		log_err("Missing some function in the function interface table\n");
		return FPGA_FAIL;
	}

	/* 1. give board specific a chance to do anything before we start */
	if (fns->pre) {
		if ((ret_val = fns->pre (cookie)) < 0) {
			return ret_val;
		}
	}

	/* from this point on we must fail gracfully by calling lower layer abort */

	/* 2. Strat burn cycle by deasserting config for t_CFG and waiting t_CF2CK after reaserted */
	fns->config (0, 1, cookie);
	udelay(5);		/* nCONFIG low pulse width 2usec */
	fns->config (1, 1, cookie);
	udelay(100);		/* nCONFIG high to first rising edge on DCLK */

	/* 3. Start the Data cycle with clk deasserted */
	bytecount = 0;
	fns->clk (0, 1, cookie);

	printf("loading to fpga    ");
	while (bytecount < bsize) {
		/* 3.1 check stratix has not signaled us an error */
		if (fns->status (cookie) != 1) {
			log_err("\nStratix failed (byte transferred till failure 0x%x)\n",
				bytecount);
			fns->abort (cookie);
			return FPGA_FAIL;
		}
		if (isSerial) {
			int i;
			uint8_t data = buff[bytecount++];
			for (i = 0; i < 8; i++) {
				/* 3.2(ps) put data on the bus */
				fns->data ((data >> i) & 1, 1, cookie);

				/* 3.3(ps) clock once */
				fns->clk (1, 1, cookie);
				fns->clk (0, 1, cookie);
			}
		} else {
			/* 3.2(fpp) put data on the bus */
			fns->data (buff[bytecount++], 1, cookie);

			/* 3.3(fpp) clock once */
			fns->clk (1, 1, cookie);
			fns->clk (0, 1, cookie);

			/* 3.4(fpp) for secure cycle push 3 more  clocks */
			for (i = 0; isSecure && i < 3; i++) {
				fns->clk (1, 1, cookie);
				fns->clk (0, 1, cookie);
			}
		}

		/* 3.5 while clk is deasserted it is safe to print some progress indication */
		if ((bytecount % (bsize / 100)) == 0) {
			printf("\b\b\b%02zu\%%", bytecount * 100 / bsize);
		}
	}

	/* 4. Set one last clock and check conf done signal */
	fns->clk (1, 1, cookie);
	udelay(100);
	if (!fns->done (cookie)) {
		printf(" error!.\n");
		fns->abort (cookie);
		return FPGA_FAIL;
	} else {
		printf("\b\b\b done.\n");
	}

	/* 5. call lower layer post configuration */
	if (fns->post) {
		if ((ret_val = fns->post (cookie)) < 0) {
			fns->abort (cookie);
			return ret_val;
		}
	}

	return FPGA_SUCCESS;
}

int StratixII_load(Altera_desc *desc, const void *buf, size_t size)
{
	int ret_val = FPGA_FAIL;

	switch (desc->iface) {
	case passive_serial:
		ret_val = StratixII_ps_fpp_load(desc, buf, size, 1, 0);
		break;
	case fast_passive_parallel:
		ret_val = StratixII_ps_fpp_load(desc, buf, size, 0, 0);
		break;
	case fast_passive_parallel_security:
		ret_val = StratixII_ps_fpp_load(desc, buf, size, 0, 1);
		break;

		/* Add new interface types here */
	default:
		log_err("Unsupported interface type, %d\n", desc->iface);
	}
	return ret_val;
}

int StratixII_dump(Altera_desc *desc, const void *buf, size_t bsize)
{
	int ret_val = FPGA_FAIL;

	switch (desc->iface) {
	case passive_serial:
	case fast_passive_parallel:
	case fast_passive_parallel_security:
		ret_val = StratixII_ps_fpp_dump(desc, buf, bsize);
		break;
		/* Add new interface types here */
	default:
		log_err("Unsupported interface type, %d\n", desc->iface);
	}
	return ret_val;
}

int StratixII_info(Altera_desc *desc)
{
	return FPGA_SUCCESS;
}

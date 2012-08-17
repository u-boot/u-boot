/*
 * (C) Copyright 2006
 * Heiko Schocher, DENX Software Engineering, hs@denx.de
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
 *
 */

/*
 * Altera FPGA configuration support for the ALPR computer from prodrive
 */

#include <common.h>
#include <altera.h>
#include <ACEX1K.h>
#include <command.h>
#include <asm/processor.h>
#include <asm/ppc440.h>
#include "fpga.h"

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_FPGA)

#ifdef FPGA_DEBUG
#define	PRINTF(fmt, args...)	printf(fmt , ##args)
#else
#define	PRINTF(fmt, args...)
#endif

static unsigned long regval;

#define SET_GPIO_REG_0(reg, bit) do {				\
		regval = in32(reg);				\
		regval &= ~(0x80000000 >> bit);			\
		out32(reg, regval);				\
	} while (0)

#define SET_GPIO_REG_1(reg, bit) do {				\
		regval = in32(reg);				\
		regval |= (0x80000000 >> bit);			\
		out32(reg, regval);				\
	} while (0)

#define	SET_GPIO_0(bit)		SET_GPIO_REG_0(GPIO0_OR, bit)
#define	SET_GPIO_1(bit)		SET_GPIO_REG_1(GPIO0_OR, bit)

#define FPGA_PRG		(0x80000000 >> CONFIG_SYS_GPIO_PROG_EN)
#define FPGA_CONFIG		(0x80000000 >> CONFIG_SYS_GPIO_CONFIG)
#define FPGA_DATA		(0x80000000 >> CONFIG_SYS_GPIO_DATA)
#define FPGA_CLK		(0x80000000 >> CONFIG_SYS_GPIO_CLK)
#define OLD_VAL			(FPGA_PRG | FPGA_CONFIG)

#define SET_FPGA(data)		out32(GPIO0_OR, data)

#define FPGA_WRITE_1 do {							    \
	SET_FPGA(OLD_VAL | 0	    | FPGA_DATA);	/* set data to 1 */ \
	SET_FPGA(OLD_VAL | FPGA_CLK | FPGA_DATA);	/* set data to 1 */ \
} while (0)

#define FPGA_WRITE_0 do {							    \
	SET_FPGA(OLD_VAL | 0	    | 0);		/* set data to 0 */ \
	SET_FPGA(OLD_VAL | FPGA_CLK | 0);		/* set data to 1 */ \
} while (0)

/* Plattforminitializations */
/* Here we have to set the FPGA Chain */
/* PROGRAM_PROG_EN	= HIGH */
/* PROGRAM_SEL_DPR	= LOW */
int fpga_pre_fn(int cookie)
{
	/* Enable the FPGA Chain */
	SET_GPIO_REG_1(GPIO0_TCR, CONFIG_SYS_GPIO_PROG_EN);
	SET_GPIO_REG_0(GPIO0_ODR, CONFIG_SYS_GPIO_PROG_EN);
	SET_GPIO_1(CONFIG_SYS_GPIO_PROG_EN);
	SET_GPIO_REG_1(GPIO0_TCR, CONFIG_SYS_GPIO_SEL_DPR);
	SET_GPIO_REG_0(GPIO0_ODR, CONFIG_SYS_GPIO_SEL_DPR);
	SET_GPIO_0((CONFIG_SYS_GPIO_SEL_DPR));

	/* initialize the GPIO Pins */
	/* output */
	SET_GPIO_0(CONFIG_SYS_GPIO_CLK);
	SET_GPIO_REG_1(GPIO0_TCR, CONFIG_SYS_GPIO_CLK);
	SET_GPIO_REG_0(GPIO0_ODR, CONFIG_SYS_GPIO_CLK);

	/* output */
	SET_GPIO_0(CONFIG_SYS_GPIO_DATA);
	SET_GPIO_REG_1(GPIO0_TCR, CONFIG_SYS_GPIO_DATA);
	SET_GPIO_REG_0(GPIO0_ODR, CONFIG_SYS_GPIO_DATA);

	/* First we set STATUS to 0 then as an input */
	SET_GPIO_REG_1(GPIO0_TCR, CONFIG_SYS_GPIO_STATUS);
	SET_GPIO_REG_0(GPIO0_ODR, CONFIG_SYS_GPIO_STATUS);
	SET_GPIO_0(CONFIG_SYS_GPIO_STATUS);
	SET_GPIO_REG_0(GPIO0_TCR, CONFIG_SYS_GPIO_STATUS);
	SET_GPIO_REG_0(GPIO0_ODR, CONFIG_SYS_GPIO_STATUS);

	/* output */
	SET_GPIO_REG_1(GPIO0_TCR, CONFIG_SYS_GPIO_CONFIG);
	SET_GPIO_REG_0(GPIO0_ODR, CONFIG_SYS_GPIO_CONFIG);
	SET_GPIO_0(CONFIG_SYS_GPIO_CONFIG);

	/* input */
	SET_GPIO_0(CONFIG_SYS_GPIO_CON_DON);
	SET_GPIO_REG_0(GPIO0_TCR, CONFIG_SYS_GPIO_CON_DON);
	SET_GPIO_REG_0(GPIO0_ODR, CONFIG_SYS_GPIO_CON_DON);

	/* CONFIG = 0 STATUS = 0 -> FPGA in reset state */
	SET_GPIO_0(CONFIG_SYS_GPIO_CONFIG);
	return FPGA_SUCCESS;
}

/* Set the state of CONFIG Pin */
int fpga_config_fn(int assert_config, int flush, int cookie)
{
	if (assert_config)
		SET_GPIO_1(CONFIG_SYS_GPIO_CONFIG);
	else
		SET_GPIO_0(CONFIG_SYS_GPIO_CONFIG);

	return FPGA_SUCCESS;
}

/* Returns the state of STATUS Pin */
int fpga_status_fn(int cookie)
{
	unsigned long	reg;

	reg = in32(GPIO0_IR);
	if (reg & (0x80000000 >> CONFIG_SYS_GPIO_STATUS)) {
		PRINTF("STATUS = HIGH\n");
		return FPGA_FAIL;
	}
	PRINTF("STATUS = LOW\n");
	return FPGA_SUCCESS;
}

/* Returns the state of CONF_DONE Pin */
int fpga_done_fn(int cookie)
{
	unsigned long	reg;
	reg = in32(GPIO0_IR);
	if (reg & (0x80000000 >> CONFIG_SYS_GPIO_CON_DON)) {
		PRINTF("CONF_DON = HIGH\n");
		return FPGA_FAIL;
	}
	PRINTF("CONF_DON = LOW\n");
	return FPGA_SUCCESS;
}

/* writes the complete buffer to the FPGA
   writing the complete buffer in one function is much faster,
   then calling it for every bit */
int fpga_write_fn(const void *buf, size_t len, int flush, int cookie)
{
	size_t bytecount = 0;
	unsigned char *data = (unsigned char *) buf;
	unsigned char val = 0;
	int		i;
	int len_40 = len / 40;

	while (bytecount < len) {
		val = data[bytecount++];
		i = 8;
		do {
			if (val & 0x01)
				FPGA_WRITE_1;
			else
				FPGA_WRITE_0;

			val >>= 1;
			i--;
		} while (i > 0);

#ifdef CONFIG_SYS_FPGA_PROG_FEEDBACK
		if (bytecount % len_40 == 0) {
			putc('.');		/* let them know we are alive */
#ifdef CONFIG_SYS_FPGA_CHECK_CTRLC
			if (ctrlc())
				return FPGA_FAIL;
#endif
		}
#endif
	}
	return FPGA_SUCCESS;
}

/* called, when programming is aborted */
int fpga_abort_fn(int cookie)
{
	SET_GPIO_1((CONFIG_SYS_GPIO_SEL_DPR));
	return FPGA_SUCCESS;
}

/* called, when programming was succesful */
int fpga_post_fn(int cookie)
{
	return fpga_abort_fn(cookie);
}

/* Note that these are pointers to code that is in Flash.  They will be
 * relocated at runtime.
 */
Altera_CYC2_Passive_Serial_fns fpga_fns = {
	fpga_pre_fn,
	fpga_config_fn,
	fpga_status_fn,
	fpga_done_fn,
	fpga_write_fn,
	fpga_abort_fn,
	fpga_post_fn
};

Altera_desc fpga[CONFIG_FPGA_COUNT] = {
	{Altera_CYC2,
	 passive_serial,
	 Altera_EP2C35_SIZE,
	 (void *) &fpga_fns,
	 NULL,
	 0}
};

/*
 * Initialize the fpga.  Return 1 on success, 0 on failure.
 */
int alpr_fpga_init(void)
{
	int i;

	PRINTF("%s:%d: Initialize FPGA interface\n", __func__, __LINE__);
	fpga_init();

	for (i = 0; i < CONFIG_FPGA_COUNT; i++) {
		PRINTF("%s:%d: Adding fpga %d\n", __func__, __LINE__, i);
		fpga_add(fpga_altera, &fpga[i]);
	}
	return 1;
}

#endif

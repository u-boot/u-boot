/*
 * (C) Copyright 2006
 * Markus Klotzbuecher, DENX Software Engineering, mk@denx.de.
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

/*
 * Host Port Interface (HPI)
 */

/* debug levels:
 *  0 : errors
 *  1 : usefull info
 *  2 : lots of info
 *  3 : noisy
 */

#define DEBUG 0

#include <config.h>
#include <common.h>
#include <mpc8xx.h>

#include "pld.h"
#include "hpi.h"

#define	_NOT_USED_	0xFFFFFFFF

/* original table:
 * - inserted loops to achieve long CS low and high Periods (~217ns)
 * - move cs high 2/4 to the right
 */
const uint dsp_table_slow[] =
{
	/* single read   (offset  0x00 in upm ram) */
	0x8fffdc04, 0x0fffdc84, 0x0fffdc84, 0x0fffdc00,
	0x3fffdc04, 0xffffdc84, 0xffffdc84, 0xffffdc05,

	/* burst read    (offset 0x08 in upm ram) */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/* single write  (offset 0x18 in upm ram) */
	0x8fffd004, 0x0fffd084, 0x0fffd084, 0x3fffd000,
	0xffffd084, 0xffffd084, 0xffffd005, _NOT_USED_,

	/* burst write   (offset 0x20 in upm ram) */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/* refresh       (offset 0x30 in upm ram) */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/* exception     (offset 0x3C in upm ram) */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
};

/* dsp hpi upm ram table
 * works fine for noninc access, failes on incremental.
 * - removed first word
 */
const uint dsp_table_fast[] =
{
	/* single read   (offset  0x00 in upm ram) */
	0x8fffdc04, 0x0fffdc04, 0x0fffdc00, 0x3fffdc04,
	0xffffdc05, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/* burst read    (offset 0x08 in upm ram) */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/* single write  (offset 0x18 in upm ram) */
	0x8fffd004, 0x0fffd004, 0x3fffd000, 0xffffd005,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/* burst write   (offset 0x20 in upm ram) */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/* refresh       (offset 0x30 in upm ram) */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/* exception     (offset 0x3C in upm ram) */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
};


#ifdef CONFIG_SPC1920_HPI_TEST
#undef HPI_TEST_OSZI

#define HPI_TEST_CHUNKSIZE	0x1000
#define HPI_TEST_PATTERN	0x00000000
#define HPI_TEST_START		0x0
#define HPI_TEST_END		0x30000

#define TINY_AUTOINC_DATA_SIZE 16 /* 32bit words */
#define TINY_AUTOINC_BASE_ADDR 0x0

static int hpi_activate(void);
#if 0
static void hpi_inactivate(void);
#endif
static void dsp_reset(void);

static int hpi_write_inc(u32 addr, u32 *data, u32 count);
static int hpi_read_inc(u32 addr, u32 *buf, u32 count);
static int hpi_write_noinc(u32 addr, u32 data);
static u32 hpi_read_noinc(u32 addr);

int hpi_test(void);
static int hpi_write_addr_test(u32 addr);
static int hpi_read_write_test(u32 addr, u32 data);
#ifdef DO_TINY_TEST
static int hpi_tiny_autoinc_test(void);
#endif /* DO_TINY_TEST */
#endif /* CONFIG_SPC1920_HPI_TEST */


/* init the host port interface on UPMA */
int hpi_init(void)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immr->im_memctl;
	volatile spc1920_pld_t *pld = (spc1920_pld_t *) CFG_SPC1920_PLD_BASE;

	upmconfig(UPMA, (uint *)dsp_table_slow, sizeof(dsp_table_slow)/sizeof(uint));
	udelay(100);

	memctl->memc_mamr = CFG_MAMR;
	memctl->memc_or3 = CFG_OR3;
	memctl->memc_br3 = CFG_BR3;

	/* reset dsp */
	dsp_reset();

	/* activate hpi switch*/
	pld->dsp_hpi_on = 0x1;

	udelay(100);

	return 0;
}

#ifdef CONFIG_SPC1920_HPI_TEST
/* activate the Host Port interface */
static int hpi_activate(void)
{
	volatile spc1920_pld_t *pld = (spc1920_pld_t *) CFG_SPC1920_PLD_BASE;

	/* turn on hpi */
	pld->dsp_hpi_on = 0x1;

	udelay(5);

	/* turn on the power EN_DSP_POWER high*/
	/* currently always on TBD */

	/* setup hpi control register */
	HPI_HPIC_1 = (u16) 0x0008;
	HPI_HPIC_2 = (u16) 0x0008;

	udelay(100);

	return 0;
}

#if 0
/* turn off the host port interface */
static void hpi_inactivate(void)
{
	volatile spc1920_pld_t *pld = (spc1920_pld_t *) CFG_SPC1920_PLD_BASE;

	/* deactivate hpi */
	pld->dsp_hpi_on = 0x0;

	/* reset the dsp */
	/* pld->dsp_reset = 0x0; */

	/* turn off the power EN_DSP_POWER# high*/
	/* currently always on TBD */

}
#endif

/* reset the DSP */
static void dsp_reset(void)
{
	volatile spc1920_pld_t *pld = (spc1920_pld_t *) CFG_SPC1920_PLD_BASE;
	pld->dsp_reset = 0x1;
	pld->dsp_hpi_on = 0x0;

	udelay(300000);

	pld->dsp_reset = 0x0;
	pld->dsp_hpi_on = 0x1;
}


/* write using autoinc (count is number of 32bit words) */
static int hpi_write_inc(u32 addr, u32 *data, u32 count)
{
	int i;
	u16 addr1, addr2;

	addr1 = (u16) ((addr >> 16) & 0xffff); /* First HW is most significant */
	addr2 = (u16) (addr & 0xffff);

	/* write address */
	HPI_HPIA_1 = addr1;
	HPI_HPIA_2 = addr2;

	debugX(4, "writing from data=0x%lx to 0x%lx\n",
		(ulong)data, (ulong)(data+count));

	for(i=0; i<count; i++) {
		HPI_HPID_INC_1 = (u16) ((data[i] >> 16) & 0xffff);
		HPI_HPID_INC_2 = (u16) (data[i] & 0xffff);
		debugX(4, "hpi_write_inc: data1=0x%x, data2=0x%x\n",
		       (u16) ((data[i] >> 16) & 0xffff),
		       (u16) (data[i] & 0xffff));
	}
#if 0
	while(data_ptr < (u16*) (data + count)) {
		HPI_HPID_INC_1 = *(data_ptr++);
		HPI_HPID_INC_2 = *(data_ptr++);
	}
#endif

	/* return number of bytes written */
	return count;
}

/*
 * read using autoinc (count is number of 32bit words)
 */
static int hpi_read_inc(u32 addr, u32 *buf, u32 count)
{
	int i;
	u16 addr1, addr2, data1, data2;

	addr1 = (u16) ((addr >> 16) & 0xffff); /* First HW is most significant */
	addr2 = (u16) (addr & 0xffff);

	/* write address */
	HPI_HPIA_1 = addr1;
	HPI_HPIA_2 = addr2;

	for(i=0; i<count; i++) {
		data1 = HPI_HPID_INC_1;
		data2 = HPI_HPID_INC_2;
		debugX(4, "hpi_read_inc: data1=0x%x, data2=0x%x\n", data1, data2);
		buf[i] = (((u32) data1) << 16) | (data2 & 0xffff);
	}

#if 0
	while(buf_ptr < (u16*) (buf + count)) {
		*(buf_ptr++) = HPI_HPID_INC_1;
		*(buf_ptr++) = HPI_HPID_INC_2;
	}
#endif

	/* return number of bytes read */
	return count;
}


/* write to non- auto inc regs */
static int hpi_write_noinc(u32 addr, u32 data)
{

	u16 addr1, addr2, data1, data2;

	addr1 = (u16) ((addr >> 16) & 0xffff); /* First HW is most significant */
	addr2 = (u16) (addr & 0xffff);

	/* printf("hpi_write_noinc: addr1=0x%x, addr2=0x%x\n", addr1, addr2); */

	HPI_HPIA_1 = addr1;
	HPI_HPIA_2 = addr2;

	data1 = (u16) ((data >> 16) & 0xffff);
	data2 = (u16) (data & 0xffff);

	/* printf("hpi_write_noinc: data1=0x%x, data2=0x%x\n", data1, data2); */

	HPI_HPID_NOINC_1 = data1;
	HPI_HPID_NOINC_2 = data2;

	return 0;
}

/* read from non- auto inc regs */
static u32 hpi_read_noinc(u32 addr)
{
	u16 addr1, addr2, data1, data2;
	u32 ret;

	addr1 = (u16) ((addr >> 16) & 0xffff); /* First HW is most significant */
	addr2 = (u16) (addr & 0xffff);

	HPI_HPIA_1 = addr1;
	HPI_HPIA_2 = addr2;

	/* printf("hpi_read_noinc: addr1=0x%x, addr2=0x%x\n", addr1, addr2); */

	data1 = HPI_HPID_NOINC_1;
	data2 = HPI_HPID_NOINC_2;

	/* printf("hpi_read_noinc: data1=0x%x, data2=0x%x\n", data1, data2); */

	ret = (((u32) data1) << 16) | (data2 & 0xffff);
	return ret;

}

/*
 * Host Port Interface Tests
 */

#ifndef HPI_TEST_OSZI
/* main test function */
int hpi_test(void)
{
	int err = 0;
	u32 i, ii, pattern, tmp;

	pattern = HPI_TEST_PATTERN;

	u32 test_data[HPI_TEST_CHUNKSIZE];
	u32 read_data[HPI_TEST_CHUNKSIZE];

	debugX(2, "hpi_test: activating hpi...");
	hpi_activate();
	debugX(2, "OK.\n");

#if 0
	/* Dump the first 1024 bytes
	 *
	 */
	for(i=0; i<1024; i+=4) {
		if(i%16==0)
			printf("\n0x%08x: ", i);
		printf("0x%08x ", hpi_read_noinc(i));
	}
#endif

	/* HPIA read-write test
	 *
	 */
	debugX(1, "hpi_test: starting HPIA read-write tests...\n");
	err |= hpi_write_addr_test(0xdeadc0de);
	err |= hpi_write_addr_test(0xbeefd00d);
	err |= hpi_write_addr_test(0xabcd1234);
	err |= hpi_write_addr_test(0xaaaaaaaa);
	if(err) {
		debugX(1, "hpi_test: HPIA read-write tests: *** FAILED ***\n");
		return -1;
	}
	debugX(1, "hpi_test: HPIA read-write tests: OK\n");


	/* read write test using nonincremental data regs
	 *
	 */
	debugX(1, "hpi_test: starting nonincremental tests...\n");
	for(i=HPI_TEST_START; i<HPI_TEST_END; i+=4) {
		err |= hpi_read_write_test(i, pattern);

		/* stolen from cmd_mem.c */
		if(pattern & 0x80000000) {
			pattern = -pattern;	/* complement & increment */
		} else {
			pattern = ~pattern;
		}
		err |= hpi_read_write_test(i, pattern);

		if(err) {
			debugX(1, "hpi_test: nonincremental tests *** FAILED ***\n");
			return -1;
		}
	}
	debugX(1, "hpi_test: nonincremental test OK\n");

	/* read write a chunk of data using nonincremental data regs
	 *
	 */
	debugX(1, "hpi_test: starting nonincremental chunk tests...\n");
	pattern = HPI_TEST_PATTERN;
	for(i=HPI_TEST_START; i<HPI_TEST_END; i+=4) {
		hpi_write_noinc(i, pattern);

		/* stolen from cmd_mem.c */
		if(pattern & 0x80000000) {
			pattern = -pattern;	/* complement & increment */
		} else {
			pattern = ~pattern;
		}
	}
	pattern = HPI_TEST_PATTERN;
	for(i=HPI_TEST_START; i<HPI_TEST_END; i+=4) {
		tmp = hpi_read_noinc(i);

		if(tmp != pattern) {
			debugX(1, "hpi_test: noninc chunk test *** FAILED *** @ 0x%x, written=0x%x, read=0x%x\n", i, pattern, tmp);
			err = -1;
		}
		/* stolen from cmd_mem.c */
		if(pattern & 0x80000000) {
			pattern = -pattern;	/* complement & increment */
		} else {
			pattern = ~pattern;
		}
	}
	if(err)
		return -1;
	debugX(1, "hpi_test: nonincremental chunk test OK\n");


#ifdef DO_TINY_TEST
	/* small verbose test using autoinc and nonautoinc to compare
	 *
	 */
	debugX(1, "hpi_test: tiny_autoinc_test...\n");
	hpi_tiny_autoinc_test();
	debugX(1, "hpi_test: tiny_autoinc_test done\n");
#endif /* DO_TINY_TEST */


	/* $%& write a chunk of data using the autoincremental regs
	 *
	 */
	debugX(1, "hpi_test: starting autoinc test %d chunks with 0x%x bytes...\n",
	       ((HPI_TEST_END - HPI_TEST_START) / HPI_TEST_CHUNKSIZE),
	       HPI_TEST_CHUNKSIZE);

	for(i=HPI_TEST_START;
	    i < ((HPI_TEST_END - HPI_TEST_START) / HPI_TEST_CHUNKSIZE);
	    i++) {
		/* generate the pattern data */
		debugX(3, "generating pattern data: ");
		for(ii = 0; ii < HPI_TEST_CHUNKSIZE; ii++) {
			debugX(3, "0x%x ", pattern);

			test_data[ii] = pattern;
			read_data[ii] = 0x0; /* zero to be sure */

			/* stolen from cmd_mem.c */
			if(pattern & 0x80000000) {
				pattern = -pattern;	/* complement & increment */
			} else {
				pattern = ~pattern;
			}
		}
		debugX(3, "done\n");

		debugX(2, "Writing autoinc data @ 0x%x\n", i);
		hpi_write_inc(i, test_data, HPI_TEST_CHUNKSIZE);

		debugX(2, "Reading autoinc data @ 0x%x\n", i);
		hpi_read_inc(i, read_data, HPI_TEST_CHUNKSIZE);

		/* compare */
		for(ii = 0; ii < HPI_TEST_CHUNKSIZE; ii++) {
			debugX(3, "hpi_test_autoinc: @ 0x%x, written=0x%x, read=0x%x", i+ii, test_data[ii], read_data[ii]);
			if(read_data[ii] != test_data[ii]) {
				debugX(0, "hpi_test: autoinc test @ 0x%x, written=0x%x, read=0x%x *** FAILED ***\n", i+ii, test_data[ii], read_data[ii]);
				return -1;
			}
		}
	}
	debugX(1, "hpi_test: autoinc test OK\n");

	return 0;
}
#else /* HPI_TEST_OSZI */
int hpi_test(void)
{
	int i;
	u32 read_data[TINY_AUTOINC_DATA_SIZE];

	unsigned int dummy_data[TINY_AUTOINC_DATA_SIZE] = {
		0x11112222, 0x33334444, 0x55556666, 0x77778888,
		0x9999aaaa, 0xbbbbcccc, 0xddddeeee, 0xffff1111,
		0x00010002, 0x00030004, 0x00050006, 0x00070008,
		0x0009000a, 0x000b000c, 0x000d000e, 0x000f0001
	};

	debugX(0, "hpi_test: activating hpi...");
	hpi_activate();
	debugX(0, "OK.\n");

	while(1) {
		led9(1);
		debugX(0, " writing to autoinc...\n");
		hpi_write_inc(TINY_AUTOINC_BASE_ADDR,
			      dummy_data, TINY_AUTOINC_DATA_SIZE);

		debugX(0, " reading from autoinc...\n");
		hpi_read_inc(TINY_AUTOINC_BASE_ADDR,
			     read_data, TINY_AUTOINC_DATA_SIZE);

		for(i=0; i < (TINY_AUTOINC_DATA_SIZE); i++) {
			debugX(0, " written=0x%x, read(inc)=0x%x\n",
			       dummy_data[i], read_data[i]);
		}
		led9(0);
		udelay(2000000);
	}
	return 0;
}
#endif

/* test if Host Port Address Register can be written correctly */
static int hpi_write_addr_test(u32 addr)
{
	u32 read_back;
	/* write address */
	HPI_HPIA_1 = ((u16) (addr >> 16)); /* First HW is most significant */
	HPI_HPIA_2 = ((u16) addr);

	read_back = (((u32) HPI_HPIA_1)<<16) | ((u32) HPI_HPIA_2);

	if(read_back == addr) {
		debugX(2, " hpi_write_addr_test OK: written=0x%x, read=0x%x\n",
		       addr, read_back);
		return 0;
	} else {
		debugX(0, " hpi_write_addr_test *** FAILED ***: written=0x%x, read=0x%x\n",
		      addr, read_back);
		return -1;
	}

	return 0;
}

/* test if a simple read/write sequence succeeds */
static int hpi_read_write_test(u32 addr, u32 data)
{
	u32 read_back;

	hpi_write_noinc(addr, data);
	read_back = hpi_read_noinc(addr);

	if(read_back == data) {
		debugX(2, " hpi_read_write_test: OK, addr=0x%x written=0x%x, read=0x%x\n", addr, data, read_back);
		return 0;
	} else {
		debugX(0, " hpi_read_write_test: *** FAILED ***, addr=0x%x written=0x%x, read=0x%x\n", addr, data, read_back);
		return -1;
	}

	return 0;
}

#ifdef DO_TINY_TEST
static int hpi_tiny_autoinc_test(void)
{
	int i;
	u32 read_data[TINY_AUTOINC_DATA_SIZE];
	u32 read_data_noinc[TINY_AUTOINC_DATA_SIZE];

	unsigned int dummy_data[TINY_AUTOINC_DATA_SIZE] = {
		0x11112222, 0x33334444, 0x55556666, 0x77778888,
		0x9999aaaa, 0xbbbbcccc, 0xddddeeee, 0xffff1111,
		0x00010002, 0x00030004, 0x00050006, 0x00070008,
		0x0009000a, 0x000b000c, 0x000d000e, 0x000f0001
	};

	printf(" writing to autoinc...\n");
	hpi_write_inc(TINY_AUTOINC_BASE_ADDR, dummy_data, TINY_AUTOINC_DATA_SIZE);

	printf(" reading from autoinc...\n");
	hpi_read_inc(TINY_AUTOINC_BASE_ADDR, read_data, TINY_AUTOINC_DATA_SIZE);

	printf(" reading from noinc for comparison...\n");
	for(i=0; i < (TINY_AUTOINC_DATA_SIZE); i++)
		read_data_noinc[i] = hpi_read_noinc(TINY_AUTOINC_BASE_ADDR+i*4);

	for(i=0; i < (TINY_AUTOINC_DATA_SIZE); i++) {
		printf(" written=0x%x, read(inc)=0x%x, read(noinc)=0x%x\n",
		       dummy_data[i], read_data[i], read_data_noinc[i]);
	}
	return 0;
}
#endif /* DO_TINY_TEST */

#endif /* CONFIG_SPC1920_HPI_TEST */

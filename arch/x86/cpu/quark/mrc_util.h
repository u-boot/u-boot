/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * Ported from Intel released Quark UEFI BIOS
 * QuarkSocPkg/QuarkNorthCluster/MemoryInit/Pei
 *
 * SPDX-License-Identifier:	Intel
 */

#ifndef _MRC_UTIL_H_
#define _MRC_UTIL_H_

/* Turn on this macro to enable MRC debugging output */
#undef  MRC_DEBUG

/* MRC Debug Support */
#define DPF		debug_cond

/* debug print type */

#ifdef MRC_DEBUG
#define D_ERROR		0x0001
#define D_INFO		0x0002
#define D_REGRD		0x0004
#define D_REGWR		0x0008
#define D_FCALL		0x0010
#define D_TRN		0x0020
#define D_TIME		0x0040
#else
#define D_ERROR		0
#define D_INFO		0
#define D_REGRD		0
#define D_REGWR		0
#define D_FCALL		0
#define D_TRN		0
#define D_TIME		0
#endif

#define ENTERFN(...)	debug_cond(D_FCALL, "<%s>\n", __func__)
#define LEAVEFN(...)	debug_cond(D_FCALL, "</%s>\n", __func__)
#define REPORTFN(...)	debug_cond(D_FCALL, "<%s/>\n", __func__)

/* Generic Register Bits */
#define BIT0		0x00000001
#define BIT1		0x00000002
#define BIT2		0x00000004
#define BIT3		0x00000008
#define BIT4		0x00000010
#define BIT5		0x00000020
#define BIT6		0x00000040
#define BIT7		0x00000080
#define BIT8		0x00000100
#define BIT9		0x00000200
#define BIT10		0x00000400
#define BIT11		0x00000800
#define BIT12		0x00001000
#define BIT13		0x00002000
#define BIT14		0x00004000
#define BIT15		0x00008000
#define BIT16		0x00010000
#define BIT17		0x00020000
#define BIT18		0x00040000
#define BIT19		0x00080000
#define BIT20		0x00100000
#define BIT21		0x00200000
#define BIT22		0x00400000
#define BIT23		0x00800000
#define BIT24		0x01000000
#define BIT25		0x02000000
#define BIT26		0x04000000
#define BIT27		0x08000000
#define BIT28		0x10000000
#define BIT29		0x20000000
#define BIT30		0x40000000
#define BIT31		0x80000000

/* Message Bus Port */
#define MEM_CTLR	0x01
#define HOST_BRIDGE	0x03
#define MEM_MGR		0x05
#define HTE		0x11
#define DDRPHY		0x12

/* number of sample points */
#define SAMPLE_CNT	3
/* number of PIs to increment per sample */
#define SAMPLE_DLY	26

enum {
	/* indicates to decrease delays when looking for edge */
	BACKWARD,
	/* indicates to increase delays when looking for edge */
	FORWARD
};

enum {
	RCVN,
	WDQS,
	WDQX,
	RDQS,
	VREF,
	WCMD,
	WCTL,
	WCLK,
	MAX_ALGOS,
};

void mrc_write_mask(u32 unit, u32 addr, u32 data, u32 mask);
void mrc_alt_write_mask(u32 unit, u32 addr, u32 data, u32 mask);
void mrc_post_code(uint8_t major, uint8_t minor);
void delay_n(uint32_t ns);
void delay_u(uint32_t ms);
void select_mem_mgr(void);
void select_hte(void);
void dram_init_command(uint32_t data);
void dram_wake_command(void);
void training_message(uint8_t channel, uint8_t rank, uint8_t byte_lane);

void set_rcvn(uint8_t channel, uint8_t rank,
	      uint8_t byte_lane, uint32_t pi_count);
uint32_t get_rcvn(uint8_t channel, uint8_t rank, uint8_t byte_lane);
void set_rdqs(uint8_t channel, uint8_t rank,
	      uint8_t byte_lane, uint32_t pi_count);
uint32_t get_rdqs(uint8_t channel, uint8_t rank, uint8_t byte_lane);
void set_wdqs(uint8_t channel, uint8_t rank,
	      uint8_t byte_lane, uint32_t pi_count);
uint32_t get_wdqs(uint8_t channel, uint8_t rank, uint8_t byte_lane);
void set_wdq(uint8_t channel, uint8_t rank,
	     uint8_t byte_lane, uint32_t pi_count);
uint32_t get_wdq(uint8_t channel, uint8_t rank, uint8_t byte_lane);
void set_wcmd(uint8_t channel, uint32_t pi_count);
uint32_t get_wcmd(uint8_t channel);
void set_wclk(uint8_t channel, uint8_t rank, uint32_t pi_count);
uint32_t get_wclk(uint8_t channel, uint8_t rank);
void set_wctl(uint8_t channel, uint8_t rank, uint32_t pi_count);
uint32_t get_wctl(uint8_t channel, uint8_t rank);
void set_vref(uint8_t channel, uint8_t byte_lane, uint32_t setting);
uint32_t get_vref(uint8_t channel, uint8_t byte_lane);

uint32_t get_addr(uint8_t channel, uint8_t rank);
uint32_t sample_dqs(struct mrc_params *mrc_params, uint8_t channel,
		    uint8_t rank, bool rcvn);
void find_rising_edge(struct mrc_params *mrc_params, uint32_t delay[],
		      uint8_t channel, uint8_t rank, bool rcvn);
uint32_t byte_lane_mask(struct mrc_params *mrc_params);
uint32_t check_rw_coarse(struct mrc_params *mrc_params, uint32_t address);
uint32_t check_bls_ex(struct mrc_params *mrc_params, uint32_t address);
void lfsr32(uint32_t *lfsr_ptr);
void clear_pointers(void);
void print_timings(struct mrc_params *mrc_params);

#endif /* _MRC_UTIL_H_ */

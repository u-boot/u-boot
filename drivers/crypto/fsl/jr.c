// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2008-2014 Freescale Semiconductor, Inc.
 * Copyright 2018, 2021 NXP
 *
 * Based on CAAM driver in drivers/crypto/caam in Linux
 */

#include <config.h>
#include <cpu_func.h>
#include <linux/kernel.h>
#include <log.h>
#include <malloc.h>
#include <power-domain.h>
#include "jr.h"
#include "jobdesc.h"
#include "desc_constr.h"
#include <time.h>
#include <asm/cache.h>
#ifdef CONFIG_FSL_CORENET
#include <asm/cache.h>
#include <asm/fsl_pamu.h>
#endif
#include <dm.h>
#include <dm/lists.h>
#include <dm/root.h>
#include <dm/device-internal.h>
#include <linux/delay.h>

#define CIRC_CNT(head, tail, size)	(((head) - (tail)) & (size - 1))
#define CIRC_SPACE(head, tail, size)	CIRC_CNT((tail), (head) + 1, (size))

uint32_t sec_offset[CONFIG_SYS_FSL_MAX_NUM_OF_SEC] = {
	0,
#if defined(CONFIG_ARCH_C29X)
	CFG_SYS_FSL_SEC_IDX_OFFSET,
	2 * CFG_SYS_FSL_SEC_IDX_OFFSET
#endif
};

#if CONFIG_IS_ENABLED(DM)
struct udevice *caam_dev;
#else
#define SEC_ADDR(idx)	\
	(ulong)((CFG_SYS_FSL_SEC_ADDR + sec_offset[idx]))

#define SEC_JR0_ADDR(idx)	\
	(ulong)(SEC_ADDR(idx) +	\
	 (CFG_SYS_FSL_JR0_OFFSET - CFG_SYS_FSL_SEC_OFFSET))
struct caam_regs caam_st;
#endif

static inline u32 jr_start_reg(u8 jrid)
{
	return (1 << jrid);
}

static inline void start_jr(struct caam_regs *caam)
{
	ccsr_sec_t *sec = caam->sec;
	u32 ctpr_ms = sec_in32(&sec->ctpr_ms);
	u32 scfgr = sec_in32(&sec->scfgr);
	u32 jrstart = jr_start_reg(caam->jrid);

	if (ctpr_ms & SEC_CTPR_MS_VIRT_EN_INCL) {
		/* VIRT_EN_INCL = 1 & VIRT_EN_POR = 1 or
		 * VIRT_EN_INCL = 1 & VIRT_EN_POR = 0 & SEC_SCFGR_VIRT_EN = 1
		 */
		if ((ctpr_ms & SEC_CTPR_MS_VIRT_EN_POR) ||
		    (scfgr & SEC_SCFGR_VIRT_EN))
			sec_out32(&sec->jrstartr, jrstart);
	} else {
		/* VIRT_EN_INCL = 0 && VIRT_EN_POR_VALUE = 1 */
		if (ctpr_ms & SEC_CTPR_MS_VIRT_EN_POR)
			sec_out32(&sec->jrstartr, jrstart);
	}
}

static inline void jr_disable_irq(struct jr_regs *regs)
{
	uint32_t jrcfg = sec_in32(&regs->jrcfg1);

	jrcfg = jrcfg | JR_INTMASK;

	sec_out32(&regs->jrcfg1, jrcfg);
}

static void jr_initregs(uint8_t sec_idx, struct caam_regs *caam)
{
	struct jr_regs *regs = caam->regs;
	struct jobring *jr = &caam->jr[sec_idx];
	caam_dma_addr_t ip_base = virt_to_phys((void *)jr->input_ring);
	caam_dma_addr_t op_base = virt_to_phys((void *)jr->output_ring);

#ifdef CONFIG_CAAM_64BIT
	sec_out32(&regs->irba_h, ip_base >> 32);
#else
	sec_out32(&regs->irba_h, 0x0);
#endif
	sec_out32(&regs->irba_l, (uint32_t)ip_base);
#ifdef CONFIG_CAAM_64BIT
	sec_out32(&regs->orba_h, op_base >> 32);
#else
	sec_out32(&regs->orba_h, 0x0);
#endif
	sec_out32(&regs->orba_l, (uint32_t)op_base);
	sec_out32(&regs->ors, JR_SIZE);
	sec_out32(&regs->irs, JR_SIZE);

	if (!jr->irq)
		jr_disable_irq(regs);
}

static int jr_init(uint8_t sec_idx, struct caam_regs *caam)
{
	struct jobring *jr = &caam->jr[sec_idx];
#if CONFIG_IS_ENABLED(OF_CONTROL)
	ofnode scu_node = ofnode_by_compatible(ofnode_null(), "fsl,imx8-mu");
#endif
	memset(jr, 0, sizeof(struct jobring));

	jr->jq_id = caam->jrid;
	jr->irq = DEFAULT_IRQ;

#ifdef CONFIG_FSL_CORENET
	jr->liodn = DEFAULT_JR_LIODN;
#endif
	jr->size = JR_SIZE;
	jr->input_ring = (caam_dma_addr_t *)memalign(ARCH_DMA_MINALIGN,
				JR_SIZE * sizeof(caam_dma_addr_t));
	if (!jr->input_ring)
		return -1;

	jr->op_size = roundup(JR_SIZE * sizeof(struct op_ring),
			      ARCH_DMA_MINALIGN);
	jr->output_ring =
	    (struct op_ring *)memalign(ARCH_DMA_MINALIGN, jr->op_size);
	if (!jr->output_ring)
		return -1;

	memset(jr->input_ring, 0, JR_SIZE * sizeof(caam_dma_addr_t));
	memset(jr->output_ring, 0, jr->op_size);

#if CONFIG_IS_ENABLED(OF_CONTROL)
	if (!ofnode_valid(scu_node))
#endif
	start_jr(caam);

	jr_initregs(sec_idx, caam);

	return 0;
}

/* -1 --- error, can't enqueue -- no space available */
static int jr_enqueue(uint32_t *desc_addr,
	       void (*callback)(uint32_t status, void *arg),
	       void *arg, uint8_t sec_idx, struct caam_regs *caam)
{
	struct jr_regs *regs = caam->regs;
	struct jobring *jr = &caam->jr[sec_idx];
	int head = jr->head;
	uint32_t desc_word;
	int length = desc_len(desc_addr);
	int i;
#ifdef CONFIG_CAAM_64BIT
	uint32_t *addr_hi, *addr_lo;
#endif

	/* The descriptor must be submitted to SEC block as per endianness
	 * of the SEC Block.
	 * So, if the endianness of Core and SEC block is different, each word
	 * of the descriptor will be byte-swapped.
	 */
	for (i = 0; i < length; i++) {
		desc_word = desc_addr[i];
		sec_out32((uint32_t *)&desc_addr[i], desc_word);
	}

	caam_dma_addr_t desc_phys_addr = virt_to_phys(desc_addr);

	jr->info[head].desc_phys_addr = desc_phys_addr;
	jr->info[head].callback = (void *)callback;
	jr->info[head].arg = arg;
	jr->info[head].op_done = 0;

	unsigned long start = (unsigned long)&jr->info[head] &
					~(ARCH_DMA_MINALIGN - 1);
	unsigned long end = ALIGN((unsigned long)&jr->info[head] +
				  sizeof(struct jr_info), ARCH_DMA_MINALIGN);
	flush_dcache_range(start, end);

#ifdef CONFIG_CAAM_64BIT
	/* Write the 64 bit Descriptor address on Input Ring.
	 * The 32 bit hign and low part of the address will
	 * depend on endianness of SEC block.
	 */
#ifdef CONFIG_SYS_FSL_SEC_LE
	addr_lo = (uint32_t *)(&jr->input_ring[head]);
	addr_hi = (uint32_t *)(&jr->input_ring[head]) + 1;
#elif defined(CONFIG_SYS_FSL_SEC_BE)
	addr_hi = (uint32_t *)(&jr->input_ring[head]);
	addr_lo = (uint32_t *)(&jr->input_ring[head]) + 1;
#endif /* ifdef CONFIG_SYS_FSL_SEC_LE */

	sec_out32(addr_hi, (uint32_t)(desc_phys_addr >> 32));
	sec_out32(addr_lo, (uint32_t)(desc_phys_addr));

#else
	/* Write the 32 bit Descriptor address on Input Ring. */
	sec_out32(&jr->input_ring[head], desc_phys_addr);
#endif /* ifdef CONFIG_CAAM_64BIT */

	start = (unsigned long)&jr->input_ring[head] & ~(ARCH_DMA_MINALIGN - 1);
	end = ALIGN((unsigned long)&jr->input_ring[head] +
		     sizeof(caam_dma_addr_t), ARCH_DMA_MINALIGN);
	flush_dcache_range(start, end);

	jr->head = (head + 1) & (jr->size - 1);

	/* Invalidate output ring */
	start = (unsigned long)jr->output_ring &
					~(ARCH_DMA_MINALIGN - 1);
	end = ALIGN((unsigned long)jr->output_ring + jr->op_size,
		    ARCH_DMA_MINALIGN);
	invalidate_dcache_range(start, end);

	sec_out32(&regs->irja, 1);

	return 0;
}

static int jr_dequeue(int sec_idx, struct caam_regs *caam)
{
	struct jr_regs *regs = caam->regs;
	struct jobring *jr = &caam->jr[sec_idx];
	int head = jr->head;
	int tail = jr->tail;
	int idx, i, found;
	void (*callback)(uint32_t status, void *arg);
	void *arg = NULL;
#ifdef CONFIG_CAAM_64BIT
	uint32_t *addr_hi, *addr_lo;
#else
	uint32_t *addr;
#endif

	while (sec_in32(&regs->orsf) && CIRC_CNT(jr->head, jr->tail,
						 jr->size)) {

		found = 0;

		caam_dma_addr_t op_desc;
	#ifdef CONFIG_CAAM_64BIT
		/* Read the 64 bit Descriptor address from Output Ring.
		 * The 32 bit hign and low part of the address will
		 * depend on endianness of SEC block.
		 */
	#ifdef CONFIG_SYS_FSL_SEC_LE
		addr_lo = (uint32_t *)(&jr->output_ring[jr->tail].desc);
		addr_hi = (uint32_t *)(&jr->output_ring[jr->tail].desc) + 1;
	#elif defined(CONFIG_SYS_FSL_SEC_BE)
		addr_hi = (uint32_t *)(&jr->output_ring[jr->tail].desc);
		addr_lo = (uint32_t *)(&jr->output_ring[jr->tail].desc) + 1;
	#endif /* ifdef CONFIG_SYS_FSL_SEC_LE */

		op_desc = ((u64)sec_in32(addr_hi) << 32) |
			  ((u64)sec_in32(addr_lo));

	#else
		/* Read the 32 bit Descriptor address from Output Ring. */
		addr = (uint32_t *)&jr->output_ring[jr->tail].desc;
		op_desc = sec_in32(addr);
	#endif /* ifdef CONFIG_CAAM_64BIT */

		uint32_t status = sec_in32(&jr->output_ring[jr->tail].status);

		for (i = 0; CIRC_CNT(head, tail + i, jr->size) >= 1; i++) {
			idx = (tail + i) & (jr->size - 1);
			if (op_desc == jr->info[idx].desc_phys_addr) {
				found = 1;
				break;
			}
		}

		/* Error condition if match not found */
		if (!found)
			return -1;

		jr->info[idx].op_done = 1;
		callback = (void *)jr->info[idx].callback;
		arg = jr->info[idx].arg;

		/* When the job on tail idx gets done, increment
		 * tail till the point where job completed out of oredr has
		 * been taken into account
		 */
		if (idx == tail)
			do {
				tail = (tail + 1) & (jr->size - 1);
			} while (jr->info[tail].op_done);

		jr->tail = tail;
		jr->read_idx = (jr->read_idx + 1) & (jr->size - 1);

		sec_out32(&regs->orjr, 1);
		jr->info[idx].op_done = 0;

		callback(status, arg);
	}

	return 0;
}

static void desc_done(uint32_t status, void *arg)
{
	struct result *x = arg;
	x->status = status;
	caam_jr_strstatus(status);
	x->done = 1;
}

static inline int run_descriptor_jr_idx(uint32_t *desc, uint8_t sec_idx)
{
	struct caam_regs *caam;
#if CONFIG_IS_ENABLED(DM)
	caam = dev_get_priv(caam_dev);
#else
	caam = &caam_st;
#endif
	unsigned long long timeval = 0;
	unsigned long long timeout = CFG_USEC_DEQ_TIMEOUT;
	struct result op;
	int ret = 0;

	memset(&op, 0, sizeof(op));

	ret = jr_enqueue(desc, desc_done, &op, sec_idx, caam);
	if (ret) {
		debug("Error in SEC enq\n");
		ret = JQ_ENQ_ERR;
		goto out;
	}

	while (op.done != 1) {
		udelay(1);
		timeval += 1;

		ret = jr_dequeue(sec_idx, caam);
		if (ret) {
			debug("Error in SEC deq\n");
			ret = JQ_DEQ_ERR;
			goto out;
		}

		if (timeval > timeout) {
			debug("SEC Dequeue timed out\n");
			ret = JQ_DEQ_TO_ERR;
			goto out;
		}
	}

	if (op.status) {
		debug("Error %x\n", op.status);
		ret = op.status;
	}
out:
	return ret;
}

int run_descriptor_jr(uint32_t *desc)
{
	return run_descriptor_jr_idx(desc, 0);
}

static int jr_sw_cleanup(uint8_t sec_idx, struct caam_regs *caam)
{
	struct jobring *jr = &caam->jr[sec_idx];

	jr->head = 0;
	jr->tail = 0;
	jr->read_idx = 0;
	jr->write_idx = 0;
	memset(jr->info, 0, sizeof(jr->info));
	memset(jr->input_ring, 0, jr->size * sizeof(caam_dma_addr_t));
	memset(jr->output_ring, 0, jr->size * sizeof(struct op_ring));

	return 0;
}

static int jr_hw_reset(struct jr_regs *regs)
{
	uint32_t timeout = 100000;
	uint32_t jrint, jrcr;

	sec_out32(&regs->jrcr, JRCR_RESET);
	do {
		jrint = sec_in32(&regs->jrint);
	} while (((jrint & JRINT_ERR_HALT_MASK) ==
		  JRINT_ERR_HALT_INPROGRESS) && --timeout);

	jrint = sec_in32(&regs->jrint);
	if (((jrint & JRINT_ERR_HALT_MASK) !=
	     JRINT_ERR_HALT_INPROGRESS) && timeout == 0)
		return -1;

	timeout = 100000;
	sec_out32(&regs->jrcr, JRCR_RESET);
	do {
		jrcr = sec_in32(&regs->jrcr);
	} while ((jrcr & JRCR_RESET) && --timeout);

	if (timeout == 0)
		return -1;

	return 0;
}

static inline int jr_reset_sec(uint8_t sec_idx)
{
	struct caam_regs *caam;
#if CONFIG_IS_ENABLED(DM)
	caam = dev_get_priv(caam_dev);
#else
	caam = &caam_st;
#endif
	if (jr_hw_reset(caam->regs) < 0)
		return -1;

	/* Clean up the jobring structure maintained by software */
	jr_sw_cleanup(sec_idx, caam);

	return 0;
}

int jr_reset(void)
{
	return jr_reset_sec(0);
}

int sec_reset(void)
{
	struct caam_regs *caam;
#if CONFIG_IS_ENABLED(DM)
	caam = dev_get_priv(caam_dev);
#else
	caam = &caam_st;
#endif
	ccsr_sec_t *sec = caam->sec;
	uint32_t mcfgr = sec_in32(&sec->mcfgr);
	uint32_t timeout = 100000;

	mcfgr |= MCFGR_SWRST;
	sec_out32(&sec->mcfgr, mcfgr);

	mcfgr |= MCFGR_DMA_RST;
	sec_out32(&sec->mcfgr, mcfgr);
	do {
		mcfgr = sec_in32(&sec->mcfgr);
	} while ((mcfgr & MCFGR_DMA_RST) == MCFGR_DMA_RST && --timeout);

	if (timeout == 0)
		return -1;

	timeout = 100000;
	do {
		mcfgr = sec_in32(&sec->mcfgr);
	} while ((mcfgr & MCFGR_SWRST) == MCFGR_SWRST && --timeout);

	if (timeout == 0)
		return -1;

	return 0;
}

static int deinstantiate_rng(u8 sec_idx, int state_handle_mask)
{
	u32 *desc;
	int sh_idx, ret = 0;
	int desc_size = ALIGN(sizeof(u32) * 2, ARCH_DMA_MINALIGN);

	desc = memalign(ARCH_DMA_MINALIGN, desc_size);
	if (!desc) {
		debug("cannot allocate RNG init descriptor memory\n");
		return -ENOMEM;
	}

	for (sh_idx = 0; sh_idx < RNG4_MAX_HANDLES; sh_idx++) {
		/*
		 * If the corresponding bit is set, then it means the state
		 * handle was initialized by us, and thus it needs to be
		 * deinitialized as well
		 */

		if (state_handle_mask & RDSTA_IF(sh_idx)) {
			/*
			 * Create the descriptor for deinstantating this state
			 * handle.
			 */
			inline_cnstr_jobdesc_rng_deinstantiation(desc, sh_idx);
			flush_dcache_range((unsigned long)desc,
					   (unsigned long)desc + desc_size);

			ret = run_descriptor_jr_idx(desc, sec_idx);
			if (ret) {
				printf("SEC%u:  RNG4 SH%d deinstantiation failed with error 0x%x\n",
				       sec_idx, sh_idx, ret);
				ret = -EIO;
				break;
			}

			printf("SEC%u:  Deinstantiated RNG4 SH%d\n",
			       sec_idx, sh_idx);
		}
	}

	free(desc);
	return ret;
}

static int instantiate_rng(uint8_t sec_idx, ccsr_sec_t *sec, int gen_sk)
{
	u32 *desc;
	u32 rdsta_val;
	int ret = 0, sh_idx, size;
	struct rng4tst __iomem *rng =
			(struct rng4tst __iomem *)&sec->rng;

	desc = memalign(ARCH_DMA_MINALIGN, sizeof(uint32_t) * 6);
	if (!desc) {
		printf("cannot allocate RNG init descriptor memory\n");
		return -1;
	}

	for (sh_idx = 0; sh_idx < RNG4_MAX_HANDLES; sh_idx++) {
		/*
		 * If the corresponding bit is set, this state handle
		 * was initialized by somebody else, so it's left alone.
		 */
		rdsta_val = sec_in32(&rng->rdsta);
		if (rdsta_val & (RDSTA_IF(sh_idx))) {
			if (rdsta_val & RDSTA_PR(sh_idx))
				continue;

			printf("SEC%u:  RNG4 SH%d was instantiated w/o prediction resistance. Tearing it down\n",
			       sec_idx, sh_idx);

			ret = deinstantiate_rng(sec_idx, RDSTA_IF(sh_idx));
			if (ret)
				break;
		}

		inline_cnstr_jobdesc_rng_instantiation(desc, sh_idx, gen_sk);
		size = roundup(sizeof(uint32_t) * 6, ARCH_DMA_MINALIGN);
		flush_dcache_range((unsigned long)desc,
				   (unsigned long)desc + size);

		ret = run_descriptor_jr_idx(desc, sec_idx);

		if (ret)
			printf("SEC%u:  RNG4 SH%d instantiation failed with error 0x%x\n",
			       sec_idx, sh_idx, ret);

		rdsta_val = sec_in32(&rng->rdsta);
		if (!(rdsta_val & RDSTA_IF(sh_idx))) {
			free(desc);
			return -1;
		}

		memset(desc, 0, sizeof(uint32_t) * 6);
	}

	free(desc);

	return ret;
}

static u8 get_rng_vid(ccsr_sec_t *sec)
{
	u8 vid;

	if (caam_get_era() < 10) {
		vid = (sec_in32(&sec->chavid_ls) & SEC_CHAVID_RNG_LS_MASK)
		       >> SEC_CHAVID_LS_RNG_SHIFT;
	} else {
		vid = (sec_in32(&sec->vreg.rng) & CHA_VER_VID_MASK)
		       >> CHA_VER_VID_SHIFT;
	}

	return vid;
}

/*
 * By default, the TRNG runs for 200 clocks per sample;
 * 1200 clocks per sample generates better entropy.
 */
static void kick_trng(int ent_delay, ccsr_sec_t *sec)
{
	struct rng4tst __iomem *rng =
			(struct rng4tst __iomem *)&sec->rng;
	u32 val;

	/* put RNG4 into program mode */
	sec_setbits32(&rng->rtmctl, RTMCTL_PRGM);
	/* rtsdctl bits 0-15 contain "Entropy Delay, which defines the
	 * length (in system clocks) of each Entropy sample taken
	 * */
	val = sec_in32(&rng->rtsdctl);
	val = (val & ~RTSDCTL_ENT_DLY_MASK) |
	      (ent_delay << RTSDCTL_ENT_DLY_SHIFT);
	sec_out32(&rng->rtsdctl, val);
	/* min. freq. count, equal to 1/4 of the entropy sample length */
	sec_out32(&rng->rtfreqmin, ent_delay >> 2);
	/* disable maximum frequency count */
	sec_out32(&rng->rtfreqmax, RTFRQMAX_DISABLE);
	/*
	 * select raw sampling in both entropy shifter
	 * and statistical checker
	 */
	sec_setbits32(&rng->rtmctl, RTMCTL_SAMP_MODE_RAW_ES_SC);
	/* put RNG4 into run mode */
	sec_clrbits32(&rng->rtmctl, RTMCTL_PRGM);
}

static int rng_init(uint8_t sec_idx, ccsr_sec_t *sec)
{
	int ret, gen_sk, ent_delay = RTSDCTL_ENT_DLY;
	struct rng4tst __iomem *rng =
			(struct rng4tst __iomem *)&sec->rng;
	u32 inst_handles;

	gen_sk = !(sec_in32(&rng->rdsta) & RDSTA_SKVN);
	do {
		inst_handles = sec_in32(&rng->rdsta) & RDSTA_MASK;

		/*
		 * If either of the SH's were instantiated by somebody else
		 * then it is assumed that the entropy
		 * parameters are properly set and thus the function
		 * setting these (kick_trng(...)) is skipped.
		 * Also, if a handle was instantiated, do not change
		 * the TRNG parameters.
		 */
		if (!inst_handles) {
			kick_trng(ent_delay, sec);
			ent_delay += 400;
		}
		/*
		 * if instantiate_rng(...) fails, the loop will rerun
		 * and the kick_trng(...) function will modfiy the
		 * upper and lower limits of the entropy sampling
		 * interval, leading to a sucessful initialization of
		 * the RNG.
		 */
		ret = instantiate_rng(sec_idx, sec, gen_sk);
		/*
		 * entropy delay is calculated via self-test method.
		 * self-test are run across different voltage, temp.
		 * if worst case value for ent_dly is identified,
		 * loop can be skipped for that platform.
		 */
		if (IS_ENABLED(CONFIG_MX6SX))
			break;

	} while ((ret == -1) && (ent_delay < RTSDCTL_ENT_DLY_MAX));
	if (ret) {
		printf("SEC%u:  Failed to instantiate RNG\n", sec_idx);
		return ret;
	}

	 /* Enable RDB bit so that RNG works faster */
	sec_setbits32(&sec->scfgr, SEC_SCFGR_RDBENABLE);

	return ret;
}

#if CONFIG_IS_ENABLED(FSL_CAAM_JR_NTZ_ACCESS)
static void jr_setown_non_trusted(ccsr_sec_t *sec)
{
	u32 jrown_ns;
	int i;

	/* Set ownership of job rings to non-TrustZone mode */
	for (i = 0; i < ARRAY_SIZE(sec->jrliodnr); i++) {
		jrown_ns = sec_in32(&sec->jrliodnr[i].ms);
		jrown_ns |= JROWN_NS | JRMID_NS;
		sec_out32(&sec->jrliodnr[i].ms, jrown_ns);
	}
}
#endif

int sec_init_idx(uint8_t sec_idx)
{
	int ret = 0;
	struct caam_regs *caam;
#if CONFIG_IS_ENABLED(DM)
	if (!caam_dev) {
		printf("caam_jr: caam not found\n");
		return -1;
	}
	caam = dev_get_priv(caam_dev);
#else
	caam_st.sec = (void *)SEC_ADDR(sec_idx);
	caam_st.regs = (struct jr_regs *)SEC_JR0_ADDR(sec_idx);
	caam_st.jrid = 0;
	caam = &caam_st;
#endif
#if CONFIG_IS_ENABLED(OF_CONTROL)
	ofnode scu_node = ofnode_by_compatible(ofnode_null(), "fsl,imx8-mu");

	if (ofnode_valid(scu_node))
		goto init;
#endif

	ccsr_sec_t *sec = caam->sec;
	uint32_t mcr = sec_in32(&sec->mcfgr);
#if defined(CONFIG_XPL_BUILD) && defined(CONFIG_IMX8M)
	uint32_t jrdid_ms = 0;
#endif
#ifdef CONFIG_FSL_CORENET
	uint32_t liodnr;
	uint32_t liodn_ns;
	uint32_t liodn_s;
#endif

	if (!(sec_idx < CONFIG_SYS_FSL_MAX_NUM_OF_SEC)) {
		printf("SEC%u:  initialization failed\n", sec_idx);
		return -1;
	}

	/*
	 * Modifying CAAM Read/Write Attributes
	 * For LS2080A
	 * For AXI Write - Cacheable, Write Back, Write allocate
	 * For AXI Read - Cacheable, Read allocate
	 * Only For LS2080a, to solve CAAM coherency issues
	 */
#ifdef CONFIG_ARCH_LS2080A
	mcr = (mcr & ~MCFGR_AWCACHE_MASK) | (0xb << MCFGR_AWCACHE_SHIFT);
	mcr = (mcr & ~MCFGR_ARCACHE_MASK) | (0x6 << MCFGR_ARCACHE_SHIFT);
#else
	mcr = (mcr & ~MCFGR_AWCACHE_MASK) | (0x2 << MCFGR_AWCACHE_SHIFT);
#endif

#ifdef CONFIG_CAAM_64BIT
	mcr |= (1 << MCFGR_PS_SHIFT);
#endif
	sec_out32(&sec->mcfgr, mcr);
#if defined(CONFIG_XPL_BUILD) && defined(CONFIG_IMX8M)
	jrdid_ms = JRDID_MS_TZ_OWN | JRDID_MS_PRIM_TZ | JRDID_MS_PRIM_DID;
	sec_out32(&sec->jrliodnr[caam->jrid].ms, jrdid_ms);
#endif
	jr_reset();

#ifdef CONFIG_FSL_CORENET
#ifdef CONFIG_XPL_BUILD
	/*
	 * For SPL Build, Set the Liodns in SEC JR0 for
	 * creating PAMU entries corresponding to these.
	 * For normal build, these are set in set_liodns().
	 */
	liodn_ns = CFG_SPL_JR0_LIODN_NS & JRNSLIODN_MASK;
	liodn_s = CFG_SPL_JR0_LIODN_S & JRSLIODN_MASK;

	liodnr = sec_in32(&sec->jrliodnr[caam->jrid].ls) &
		 ~(JRNSLIODN_MASK | JRSLIODN_MASK);
	liodnr = liodnr |
		 (liodn_ns << JRNSLIODN_SHIFT) |
		 (liodn_s << JRSLIODN_SHIFT);
	sec_out32(&sec->jrliodnr[caam->jrid].ls, liodnr);
#else
	liodnr = sec_in32(&sec->jrliodnr[caam->jrid].ls);
	liodn_ns = (liodnr & JRNSLIODN_MASK) >> JRNSLIODN_SHIFT;
	liodn_s = (liodnr & JRSLIODN_MASK) >> JRSLIODN_SHIFT;
#endif
#endif
#if CONFIG_IS_ENABLED(OF_CONTROL)
init:
#endif
#if CONFIG_IS_ENABLED(FSL_CAAM_JR_NTZ_ACCESS)
	jr_setown_non_trusted(sec);
#endif

	ret = jr_init(sec_idx, caam);
	if (ret < 0) {
		printf("SEC%u:  initialization failed\n", sec_idx);
		return -1;
	}
#if CONFIG_IS_ENABLED(OF_CONTROL)
	if (ofnode_valid(scu_node)) {
		if (CONFIG_IS_ENABLED(DM_RNG)) {
			ret = device_bind_driver(NULL, "caam-rng", "caam-rng", NULL);
			if (ret)
				printf("Couldn't bind rng driver (%d)\n", ret);
		}
		return ret;
	}
#endif

#ifdef CONFIG_FSL_CORENET
	ret = sec_config_pamu_table(liodn_ns, liodn_s);
	if (ret < 0)
		return -1;

	pamu_enable();
#endif

	if (get_rng_vid(caam->sec) >= 4) {
		if (rng_init(sec_idx, caam->sec) < 0) {
			printf("SEC%u:  RNG instantiation failed\n", sec_idx);
			return -1;
		}

		if (CONFIG_IS_ENABLED(DM_RNG)) {
			ret = device_bind_driver(NULL, "caam-rng", "caam-rng",
						 NULL);
			if (ret)
				printf("Couldn't bind rng driver (%d)\n", ret);
		}

		printf("SEC%u:  RNG instantiated\n", sec_idx);
	}
	return ret;
}

int sec_init(void)
{
	return sec_init_idx(0);
}

#if CONFIG_IS_ENABLED(DM)
static int jr_power_on(ofnode node)
{
#if CONFIG_IS_ENABLED(POWER_DOMAIN)
	struct udevice __maybe_unused jr_dev;
	struct power_domain pd;

	dev_set_ofnode(&jr_dev, node);

	/* Power on Job Ring before access it */
	if (!power_domain_get(&jr_dev, &pd)) {
		if (power_domain_on(&pd))
			return -EINVAL;
	}
#endif
	return 0;
}

static int caam_jr_ioctl(struct udevice *dev, unsigned long request, void *buf)
{
	if (request != CAAM_JR_RUN_DESC)
		return -ENOSYS;

	return run_descriptor_jr(buf);
}

static int caam_jr_probe(struct udevice *dev)
{
	struct caam_regs *caam = dev_get_priv(dev);
	fdt_addr_t addr;
	ofnode node, scu_node;
	unsigned int jr_node = 0;

	caam_dev = dev;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE) {
		printf("caam_jr: crypto not found\n");
		return -EINVAL;
	}
	caam->sec = (ccsr_sec_t *)(uintptr_t)addr;
	caam->regs = (struct jr_regs *)caam->sec;

	/* Check for enabled job ring node */
	ofnode_for_each_subnode(node, dev_ofnode(dev)) {
		if (!ofnode_is_enabled(node))
			continue;

		jr_node = ofnode_read_u32_default(node, "reg", -1);
		if (jr_node > 0) {
			caam->regs = (struct jr_regs *)((ulong)caam->sec + jr_node);
			while (!(jr_node & 0x0F))
				jr_node = jr_node >> 4;

			caam->jrid = jr_node - 1;
			scu_node = ofnode_by_compatible(ofnode_null(), "fsl,imx8-mu");
			if (ofnode_valid(scu_node)) {
				if (jr_power_on(node))
					return -EINVAL;
			}
			break;
		}
	}

	if (sec_init())
		printf("\nsec_init failed!\n");

	return 0;
}

static int caam_jr_bind(struct udevice *dev)
{
	return 0;
}

static const struct misc_ops caam_jr_ops = {
	.ioctl = caam_jr_ioctl,
};

static const struct udevice_id caam_jr_match[] = {
	{ .compatible = "fsl,sec-v4.0" },
	{ }
};

U_BOOT_DRIVER(caam_jr) = {
	.name		= "caam_jr",
	.id		= UCLASS_MISC,
	.of_match	= caam_jr_match,
	.ops		= &caam_jr_ops,
	.bind		= caam_jr_bind,
	.probe		= caam_jr_probe,
	.priv_auto	= sizeof(struct caam_regs),
};
#endif

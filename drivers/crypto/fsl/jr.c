/*
 * Copyright 2008-2014 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Based on CAAM driver in drivers/crypto/caam in Linux
 */

#include <common.h>
#include <malloc.h>
#include "fsl_sec.h"
#include "jr.h"
#include "jobdesc.h"

#define CIRC_CNT(head, tail, size)	(((head) - (tail)) & (size - 1))
#define CIRC_SPACE(head, tail, size)	CIRC_CNT((tail), (head) + 1, (size))

struct jobring jr;

static inline void start_jr0(void)
{
	ccsr_sec_t *sec = (void *)CONFIG_SYS_FSL_SEC_ADDR;
	u32 ctpr_ms = sec_in32(&sec->ctpr_ms);
	u32 scfgr = sec_in32(&sec->scfgr);

	if (ctpr_ms & SEC_CTPR_MS_VIRT_EN_INCL) {
		/* VIRT_EN_INCL = 1 & VIRT_EN_POR = 1 or
		 * VIRT_EN_INCL = 1 & VIRT_EN_POR = 0 & SEC_SCFGR_VIRT_EN = 1
		 */
		if ((ctpr_ms & SEC_CTPR_MS_VIRT_EN_POR) ||
		    (!(ctpr_ms & SEC_CTPR_MS_VIRT_EN_POR) &&
					(scfgr & SEC_SCFGR_VIRT_EN)))
			sec_out32(&sec->jrstartr, CONFIG_JRSTARTR_JR0);
	} else {
		/* VIRT_EN_INCL = 0 && VIRT_EN_POR_VALUE = 1 */
		if (ctpr_ms & SEC_CTPR_MS_VIRT_EN_POR)
			sec_out32(&sec->jrstartr, CONFIG_JRSTARTR_JR0);
	}
}

static inline void jr_reset_liodn(void)
{
	ccsr_sec_t *sec = (void *)CONFIG_SYS_FSL_SEC_ADDR;
	sec_out32(&sec->jrliodnr[0].ls, 0);
}

static inline void jr_disable_irq(void)
{
	struct jr_regs *regs = (struct jr_regs *)CONFIG_SYS_FSL_JR0_ADDR;
	uint32_t jrcfg = sec_in32(&regs->jrcfg1);

	jrcfg = jrcfg | JR_INTMASK;

	sec_out32(&regs->jrcfg1, jrcfg);
}

static void jr_initregs(void)
{
	struct jr_regs *regs = (struct jr_regs *)CONFIG_SYS_FSL_JR0_ADDR;
	phys_addr_t ip_base = virt_to_phys((void *)jr.input_ring);
	phys_addr_t op_base = virt_to_phys((void *)jr.output_ring);

#ifdef CONFIG_PHYS_64BIT
	sec_out32(&regs->irba_h, ip_base >> 32);
#else
	sec_out32(&regs->irba_h, 0x0);
#endif
	sec_out32(&regs->irba_l, (uint32_t)ip_base);
#ifdef CONFIG_PHYS_64BIT
	sec_out32(&regs->orba_h, op_base >> 32);
#else
	sec_out32(&regs->orba_h, 0x0);
#endif
	sec_out32(&regs->orba_l, (uint32_t)op_base);
	sec_out32(&regs->ors, JR_SIZE);
	sec_out32(&regs->irs, JR_SIZE);

	if (!jr.irq)
		jr_disable_irq();
}

static int jr_init(void)
{
	memset(&jr, 0, sizeof(struct jobring));

	jr.jq_id = DEFAULT_JR_ID;
	jr.irq = DEFAULT_IRQ;

#ifdef CONFIG_FSL_CORENET
	jr.liodn = DEFAULT_JR_LIODN;
#endif
	jr.size = JR_SIZE;
	jr.input_ring = (dma_addr_t *)memalign(ARCH_DMA_MINALIGN,
				JR_SIZE * sizeof(dma_addr_t));
	if (!jr.input_ring)
		return -1;
	jr.output_ring =
	    (struct op_ring *)memalign(ARCH_DMA_MINALIGN,
				JR_SIZE * sizeof(struct op_ring));
	if (!jr.output_ring)
		return -1;

	memset(jr.input_ring, 0, JR_SIZE * sizeof(dma_addr_t));
	memset(jr.output_ring, 0, JR_SIZE * sizeof(struct op_ring));

	start_jr0();

	jr_initregs();

	return 0;
}

static int jr_sw_cleanup(void)
{
	jr.head = 0;
	jr.tail = 0;
	jr.read_idx = 0;
	jr.write_idx = 0;
	memset(jr.info, 0, sizeof(jr.info));
	memset(jr.input_ring, 0, jr.size * sizeof(dma_addr_t));
	memset(jr.output_ring, 0, jr.size * sizeof(struct op_ring));

	return 0;
}

static int jr_hw_reset(void)
{
	struct jr_regs *regs = (struct jr_regs *)CONFIG_SYS_FSL_JR0_ADDR;
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

/* -1 --- error, can't enqueue -- no space available */
static int jr_enqueue(uint32_t *desc_addr,
	       void (*callback)(uint32_t desc, uint32_t status, void *arg),
	       void *arg)
{
	struct jr_regs *regs = (struct jr_regs *)CONFIG_SYS_FSL_JR0_ADDR;
	int head = jr.head;
	dma_addr_t desc_phys_addr = virt_to_phys(desc_addr);

	if (sec_in32(&regs->irsa) == 0 ||
	    CIRC_SPACE(jr.head, jr.tail, jr.size) <= 0)
		return -1;

	jr.info[head].desc_phys_addr = desc_phys_addr;
	jr.info[head].desc_addr = (uint32_t)desc_addr;
	jr.info[head].callback = (void *)callback;
	jr.info[head].arg = arg;
	jr.info[head].op_done = 0;

	unsigned long start = (unsigned long)&jr.info[head] &
					~(ARCH_DMA_MINALIGN - 1);
	unsigned long end = ALIGN(start + sizeof(struct jr_info),
					ARCH_DMA_MINALIGN);
	flush_dcache_range(start, end);

	jr.input_ring[head] = desc_phys_addr;
	start = (unsigned long)&jr.input_ring[head] & ~(ARCH_DMA_MINALIGN - 1);
	end = ALIGN(start + sizeof(dma_addr_t), ARCH_DMA_MINALIGN);
	flush_dcache_range(start, end);

	jr.head = (head + 1) & (jr.size - 1);

	sec_out32(&regs->irja, 1);

	return 0;
}

static int jr_dequeue(void)
{
	struct jr_regs *regs = (struct jr_regs *)CONFIG_SYS_FSL_JR0_ADDR;
	int head = jr.head;
	int tail = jr.tail;
	int idx, i, found;
	void (*callback)(uint32_t desc, uint32_t status, void *arg);
	void *arg = NULL;

	while (sec_in32(&regs->orsf) && CIRC_CNT(jr.head, jr.tail, jr.size)) {
		unsigned long start = (unsigned long)jr.output_ring &
					~(ARCH_DMA_MINALIGN - 1);
		unsigned long end = ALIGN(start +
					  sizeof(struct op_ring)*JR_SIZE,
					  ARCH_DMA_MINALIGN);
		invalidate_dcache_range(start, end);

		found = 0;

		dma_addr_t op_desc = jr.output_ring[jr.tail].desc;
		uint32_t status = jr.output_ring[jr.tail].status;
		uint32_t desc_virt;

		for (i = 0; CIRC_CNT(head, tail + i, jr.size) >= 1; i++) {
			idx = (tail + i) & (jr.size - 1);
			if (op_desc == jr.info[idx].desc_phys_addr) {
				desc_virt = jr.info[idx].desc_addr;
				found = 1;
				break;
			}
		}

		/* Error condition if match not found */
		if (!found)
			return -1;

		jr.info[idx].op_done = 1;
		callback = (void *)jr.info[idx].callback;
		arg = jr.info[idx].arg;

		/* When the job on tail idx gets done, increment
		 * tail till the point where job completed out of oredr has
		 * been taken into account
		 */
		if (idx == tail)
			do {
				tail = (tail + 1) & (jr.size - 1);
			} while (jr.info[tail].op_done);

		jr.tail = tail;
		jr.read_idx = (jr.read_idx + 1) & (jr.size - 1);

		sec_out32(&regs->orjr, 1);
		jr.info[idx].op_done = 0;

		callback(desc_virt, status, arg);
	}

	return 0;
}

static void desc_done(uint32_t desc, uint32_t status, void *arg)
{
	struct result *x = arg;
	x->status = status;
	caam_jr_strstatus(status);
	x->done = 1;
}

int run_descriptor_jr(uint32_t *desc)
{
	unsigned long long timeval = get_ticks();
	unsigned long long timeout = usec2ticks(CONFIG_SEC_DEQ_TIMEOUT);
	struct result op;
	int ret = 0;

	memset(&op, 0, sizeof(op));

	ret = jr_enqueue(desc, desc_done, &op);
	if (ret) {
		debug("Error in SEC enq\n");
		ret = JQ_ENQ_ERR;
		goto out;
	}

	timeval = get_ticks();
	timeout = usec2ticks(CONFIG_SEC_DEQ_TIMEOUT);
	while (op.done != 1) {
		ret = jr_dequeue();
		if (ret) {
			debug("Error in SEC deq\n");
			ret = JQ_DEQ_ERR;
			goto out;
		}

		if ((get_ticks() - timeval) > timeout) {
			debug("SEC Dequeue timed out\n");
			ret = JQ_DEQ_TO_ERR;
			goto out;
		}
	}

	if (!op.status) {
		debug("Error %x\n", op.status);
		ret = op.status;
	}
out:
	return ret;
}

int jr_reset(void)
{
	if (jr_hw_reset() < 0)
		return -1;

	/* Clean up the jobring structure maintained by software */
	jr_sw_cleanup();

	return 0;
}

int sec_reset(void)
{
	ccsr_sec_t *sec = (void *)CONFIG_SYS_FSL_SEC_ADDR;
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

static int instantiate_rng(void)
{
	struct result op;
	u32 *desc;
	u32 rdsta_val;
	int ret = 0;
	ccsr_sec_t __iomem *sec =
			(ccsr_sec_t __iomem *)CONFIG_SYS_FSL_SEC_ADDR;
	struct rng4tst __iomem *rng =
			(struct rng4tst __iomem *)&sec->rng;

	memset(&op, 0, sizeof(struct result));

	desc = memalign(ARCH_DMA_MINALIGN, sizeof(uint32_t) * 6);
	if (!desc) {
		printf("cannot allocate RNG init descriptor memory\n");
		return -1;
	}

	inline_cnstr_jobdesc_rng_instantiation(desc);
	int size = roundup(sizeof(uint32_t) * 6, ARCH_DMA_MINALIGN);
	flush_dcache_range((unsigned long)desc,
			   (unsigned long)desc + size);

	ret = run_descriptor_jr(desc);

	if (ret)
		printf("RNG: Instantiation failed with error %x\n", ret);

	rdsta_val = sec_in32(&rng->rdsta);
	if (op.status || !(rdsta_val & RNG_STATE0_HANDLE_INSTANTIATED))
		return -1;

	return ret;
}

static u8 get_rng_vid(void)
{
	ccsr_sec_t *sec = (void *)CONFIG_SYS_FSL_SEC_ADDR;
	u32 cha_vid = sec_in32(&sec->chavid_ls);

	return (cha_vid & SEC_CHAVID_RNG_LS_MASK) >> SEC_CHAVID_LS_RNG_SHIFT;
}

/*
 * By default, the TRNG runs for 200 clocks per sample;
 * 1200 clocks per sample generates better entropy.
 */
static void kick_trng(int ent_delay)
{
	ccsr_sec_t __iomem *sec =
			(ccsr_sec_t __iomem *)CONFIG_SYS_FSL_SEC_ADDR;
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
	/* max. freq. count, equal to 8 times the entropy sample length */
	sec_out32(&rng->rtfreqmax, ent_delay << 3);
	/* put RNG4 into run mode */
	sec_clrbits32(&rng->rtmctl, RTMCTL_PRGM);
}

static int rng_init(void)
{
	int ret, ent_delay = RTSDCTL_ENT_DLY_MIN;
	ccsr_sec_t __iomem *sec =
			(ccsr_sec_t __iomem *)CONFIG_SYS_FSL_SEC_ADDR;
	struct rng4tst __iomem *rng =
			(struct rng4tst __iomem *)&sec->rng;

	u32 rdsta = sec_in32(&rng->rdsta);

	/* Check if RNG state 0 handler is already instantiated */
	if (rdsta & RNG_STATE0_HANDLE_INSTANTIATED)
		return 0;

	do {
		/*
		 * If either of the SH's were instantiated by somebody else
		 * then it is assumed that the entropy
		 * parameters are properly set and thus the function
		 * setting these (kick_trng(...)) is skipped.
		 * Also, if a handle was instantiated, do not change
		 * the TRNG parameters.
		 */
		kick_trng(ent_delay);
		ent_delay += 400;
		/*
		 * if instantiate_rng(...) fails, the loop will rerun
		 * and the kick_trng(...) function will modfiy the
		 * upper and lower limits of the entropy sampling
		 * interval, leading to a sucessful initialization of
		 * the RNG.
		 */
		ret = instantiate_rng();
	} while ((ret == -1) && (ent_delay < RTSDCTL_ENT_DLY_MAX));
	if (ret) {
		printf("RNG: Failed to instantiate RNG\n");
		return ret;
	}

	 /* Enable RDB bit so that RNG works faster */
	sec_setbits32(&sec->scfgr, SEC_SCFGR_RDBENABLE);

	return ret;
}

int sec_init(void)
{
	int ret = 0;

#ifdef CONFIG_PHYS_64BIT
	ccsr_sec_t *sec = (void *)CONFIG_SYS_FSL_SEC_ADDR;
	uint32_t mcr = sec_in32(&sec->mcfgr);

	sec_out32(&sec->mcfgr, mcr | 1 << MCFGR_PS_SHIFT);
#endif
	ret = jr_init();
	if (ret < 0) {
		printf("SEC initialization failed\n");
		return -1;
	}

	if (get_rng_vid() >= 4) {
		if (rng_init() < 0) {
			printf("RNG instantiation failed\n");
			return -1;
		}
		printf("SEC: RNG instantiated\n");
	}

	return ret;
}

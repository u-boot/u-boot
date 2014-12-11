/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>

#include "fsl_epu.h"

/**
 * fsl_epu_clean - Clear EPU registers
 */
void fsl_epu_clean(void *epu_base)
{
	u32 offset;

	/* follow the exact sequence to clear the registers */
	/* Clear EPACRn */
	for (offset = EPACR0; offset <= EPACR15; offset += EPACR_STRIDE)
		out_be32(epu_base + offset, 0);

	/* Clear EPEVTCRn */
	for (offset = EPEVTCR0; offset <= EPEVTCR9; offset += EPEVTCR_STRIDE)
		out_be32(epu_base + offset, 0);

	/* Clear EPGCR */
	out_be32(epu_base + EPGCR, 0);

	/* Clear EPSMCRn */
	for (offset = EPSMCR0; offset <= EPSMCR15; offset += EPSMCR_STRIDE)
		out_be32(epu_base + offset, 0);

	/* Clear EPCCRn */
	for (offset = EPCCR0; offset <= EPCCR31; offset += EPCCR_STRIDE)
		out_be32(epu_base + offset, 0);

	/* Clear EPCMPRn */
	for (offset = EPCMPR0; offset <= EPCMPR31; offset += EPCMPR_STRIDE)
		out_be32(epu_base + offset, 0);

	/* Clear EPCTRn */
	for (offset = EPCTR0; offset <= EPCTR31; offset += EPCTR_STRIDE)
		out_be32(epu_base + offset, 0);

	/* Clear EPIMCRn */
	for (offset = EPIMCR0; offset <= EPIMCR31; offset += EPIMCR_STRIDE)
		out_be32(epu_base + offset, 0);

	/* Clear EPXTRIGCRn */
	out_be32(epu_base + EPXTRIGCR, 0);

	/* Clear EPECRn */
	for (offset = EPECR0; offset <= EPECR15; offset += EPECR_STRIDE)
		out_be32(epu_base + offset, 0);
}

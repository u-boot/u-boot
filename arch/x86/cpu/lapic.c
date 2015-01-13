/*
 * From coreboot file of same name
 *
 * Copyright (C) 2008-2009 coresystems GmbH
 * Copyright (C) 2014 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <asm/msr.h>
#include <asm/io.h>
#include <asm/lapic.h>
#include <asm/post.h>

void lapic_setup(void)
{
#if NEED_LAPIC == 1
	/* Only Pentium Pro and later have those MSR stuff */
	debug("Setting up local apic: ");

	/* Enable the local apic */
	enable_lapic();

	/*
	 * Set Task Priority to 'accept all'.
	 */
	lapic_write_around(LAPIC_TASKPRI,
			   lapic_read_around(LAPIC_TASKPRI) & ~LAPIC_TPRI_MASK);

	/* Put the local apic in virtual wire mode */
	lapic_write_around(LAPIC_SPIV, (lapic_read_around(LAPIC_SPIV) &
				~(LAPIC_VECTOR_MASK)) | LAPIC_SPIV_ENABLE);
	lapic_write_around(LAPIC_LVT0, (lapic_read_around(LAPIC_LVT0) &
			~(LAPIC_LVT_MASKED | LAPIC_LVT_LEVEL_TRIGGER |
			  LAPIC_LVT_REMOTE_IRR | LAPIC_INPUT_POLARITY |
			  LAPIC_SEND_PENDING | LAPIC_LVT_RESERVED_1 |
			  LAPIC_DELIVERY_MODE_MASK)) |
			(LAPIC_LVT_REMOTE_IRR | LAPIC_SEND_PENDING |
			 LAPIC_DELIVERY_MODE_EXTINT));
	lapic_write_around(LAPIC_LVT1, (lapic_read_around(LAPIC_LVT1) &
			~(LAPIC_LVT_MASKED | LAPIC_LVT_LEVEL_TRIGGER |
			  LAPIC_LVT_REMOTE_IRR | LAPIC_INPUT_POLARITY |
			  LAPIC_SEND_PENDING | LAPIC_LVT_RESERVED_1 |
			  LAPIC_DELIVERY_MODE_MASK)) |
		(LAPIC_LVT_REMOTE_IRR | LAPIC_SEND_PENDING |
			LAPIC_DELIVERY_MODE_NMI));

	debug("apic_id: 0x%02lx, ", lapicid());
#else /* !NEED_LLAPIC */
	/* Only Pentium Pro and later have those MSR stuff */
	debug("Disabling local apic: ");
	disable_lapic();
#endif /* !NEED_LAPIC */
	debug("done.\n");
	post_code(POST_LAPIC);
}

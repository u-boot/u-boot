/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fsl_sec_mon.h>

int change_sec_mon_state(u32 initial_state, u32 final_state)
{
	struct ccsr_sec_mon_regs *sec_mon_regs = (void *)
						(CONFIG_SYS_SEC_MON_ADDR);
	u32 sts = sec_mon_in32(&sec_mon_regs->hp_stat);
	int timeout = 10;

	if ((sts & HPSR_SSM_ST_MASK) != initial_state)
		return -1;

	if (initial_state == HPSR_SSM_ST_TRUST) {
		switch (final_state) {
		case HPSR_SSM_ST_NON_SECURE:
			printf("SEC_MON state transitioning to Soft Fail.\n");
			sec_mon_setbits32(&sec_mon_regs->hp_com, HPCOMR_SW_SV);

			/*
			 * poll till SEC_MON is in
			 * Soft Fail state
			 */
			while (((sts & HPSR_SSM_ST_MASK) !=
				HPSR_SSM_ST_SOFT_FAIL)) {
				while (timeout) {
					sts = sec_mon_in32
						(&sec_mon_regs->hp_stat);

					if ((sts & HPSR_SSM_ST_MASK) ==
						HPSR_SSM_ST_SOFT_FAIL)
						break;

					udelay(10);
					timeout--;
				}
			}

			if (timeout == 0) {
				printf("SEC_MON state transition timeout.\n");
				return -1;
			}

			timeout = 10;

			printf("SEC_MON state transitioning to Non Secure.\n");
			sec_mon_setbits32(&sec_mon_regs->hp_com, HPCOMR_SSM_ST);

			/*
			 * poll till SEC_MON is in
			 * Non Secure state
			 */
			while (((sts & HPSR_SSM_ST_MASK) !=
				HPSR_SSM_ST_NON_SECURE)) {
				while (timeout) {
					sts = sec_mon_in32
						(&sec_mon_regs->hp_stat);

					if ((sts & HPSR_SSM_ST_MASK) ==
						HPSR_SSM_ST_NON_SECURE)
						break;

					udelay(10);
					timeout--;
				}
			}

			if (timeout == 0) {
				printf("SEC_MON state transition timeout.\n");
				return -1;
			}
			break;
		case HPSR_SSM_ST_SOFT_FAIL:
			printf("SEC_MON state transitioning to Soft Fail.\n");
			sec_mon_setbits32(&sec_mon_regs->hp_com, HPCOMR_SW_FSV);

			/*
			 * polling loop till SEC_MON is in
			 * Soft Fail state
			 */
			while (((sts & HPSR_SSM_ST_MASK) !=
				HPSR_SSM_ST_SOFT_FAIL)) {
				while (timeout) {
					sts = sec_mon_in32
						(&sec_mon_regs->hp_stat);

					if ((sts & HPSR_SSM_ST_MASK) ==
						HPSR_SSM_ST_SOFT_FAIL)
						break;

					udelay(10);
					timeout--;
				}
			}

			if (timeout == 0) {
				printf("SEC_MON state transition timeout.\n");
				return -1;
			}
			break;
		default:
			return -1;
		}
	} else if (initial_state == HPSR_SSM_ST_NON_SECURE) {
		switch (final_state) {
		case HPSR_SSM_ST_SOFT_FAIL:
			printf("SEC_MON state transitioning to Soft Fail.\n");
			sec_mon_setbits32(&sec_mon_regs->hp_com, HPCOMR_SW_FSV);

			/*
			 * polling loop till SEC_MON is in
			 * Soft Fail state
			 */
			while (((sts & HPSR_SSM_ST_MASK) !=
				HPSR_SSM_ST_SOFT_FAIL)) {
				while (timeout) {
					sts = sec_mon_in32
						(&sec_mon_regs->hp_stat);

					if ((sts & HPSR_SSM_ST_MASK) ==
						HPSR_SSM_ST_SOFT_FAIL)
						break;

					udelay(10);
					timeout--;
				}
			}

			if (timeout == 0) {
				printf("SEC_MON state transition timeout.\n");
				return -1;
			}
			break;
		default:
			return -1;
		}
	}

	return 0;
}

/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * This file is based on code
 * (C) Copyright Motorola, Inc., 2000
 */

#include <common.h>
#include <mpc5xxx.h>

/* BestComm/SmartComm microcode */
extern int taskTable;

void loadtask(int basetask, int tasks)
{
	int *sram = (int *)MPC5XXX_SRAM;
	int *task_org = &taskTable;
	unsigned int start, offset, end;
	int i;

#ifdef DEBUG
	printf("basetask = %d, tasks = %d\n", basetask, tasks);
	printf("task_org = 0x%08x\n", (unsigned int)task_org);
#endif

	/* setup TaskBAR register */
	*(vu_long *)MPC5XXX_SDMA = MPC5XXX_SRAM;

	/* relocate task table entries */
	offset = (unsigned int)sram;
	for (i = basetask; i < basetask + tasks; i++) {
		sram[i * 8 + 0] = task_org[i * 8 + 0] + offset;
		sram[i * 8 + 1] = task_org[i * 8 + 1] + offset;
		sram[i * 8 + 2] = task_org[i * 8 + 2] + offset;
		sram[i * 8 + 3] = task_org[i * 8 + 3] + offset;
		sram[i * 8 + 4] = task_org[i * 8 + 4];
		sram[i * 8 + 5] = task_org[i * 8 + 5];
		sram[i * 8 + 6] = task_org[i * 8 + 6] + offset;
		sram[i * 8 + 7] = task_org[i * 8 + 7];
	}

	/* relocate task descriptors */
	start = (sram[basetask * 8] - (unsigned int)sram);
	end = (sram[(basetask + tasks - 1) * 8 + 1] - (unsigned int)sram);

#ifdef DEBUG
	printf ("TDT start = 0x%08x, end = 0x%08x\n", start, end);
#endif

	start /= 4;
	end /= 4;
	for (i = start; i <= end; i++) {
		sram[i] = task_org[i];
	}

	/* relocate variables */
	start = (sram[basetask * 8 + 2] - (unsigned int)sram);
	end = (sram[(basetask + tasks - 1) * 8 + 2] + 256 - (unsigned int)sram);
	start /= 4;
	end /= 4;
	for (i = start; i < end; i++) {
		sram[i] = task_org[i];
	}

	/* relocate function decriptors */
	start = ((sram[basetask * 8 + 3] & 0xfffffffc) - (unsigned int)sram);
	end = ((sram[(basetask + tasks - 1) * 8 + 3] & 0xfffffffc) + 256 - (unsigned int)sram);
	start /= 4;
	end /= 4;
	for (i = start; i < end; i++) {
		sram[i] = task_org[i];
	}

	asm volatile ("sync");
}

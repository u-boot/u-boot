/*
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <asm/post.h>

.globl car_init
car_init:
	/*
	 * Note: ebp holds the BIST value (built-in self test) so far, but ebp
	 * will be destroyed through the FSP call, thus we have to test the
	 * BIST value here before we call into FSP.
	 */
	test	%ebp, %ebp
	jz	car_init_start
	post_code(POST_BIST_FAILURE)
	jmp	die

car_init_start:
	post_code(POST_CAR_START)
	lea	find_fsp_header_romstack, %esp
	jmp	find_fsp_header

find_fsp_header_ret:
	/* EAX points to FSP_INFO_HEADER */
	mov	%eax, %ebp

	/* sanity test */
	cmp	$CONFIG_FSP_ADDR, %eax
	jb	die

	/* calculate TempRamInitEntry address */
	mov	0x30(%ebp), %eax
	add	0x1c(%ebp), %eax

	/* call FSP TempRamInitEntry to setup temporary stack */
	lea	temp_ram_init_romstack, %esp
	jmp	*%eax

temp_ram_init_ret:
	addl	$4, %esp
	cmp	$0, %eax
	jnz	car_init_fail

	post_code(POST_CAR_CPU_CACHE)

	/*
	 * The FSP TempRamInit initializes the ecx and edx registers to
	 * point to a temporary but writable memory range (Cache-As-RAM).
	 * ecx: the start of this temporary memory range,
	 * edx: the end of this range.
	 */

	/* stack grows down from top of CAR */
	movl	%edx, %esp

	/*
	 * TODO:
	 *
	 * According to FSP architecture spec, the fsp_init() will not return
	 * to its caller, instead it requires the bootloader to provide a
	 * so-called continuation function to pass into the FSP as a parameter
	 * of fsp_init, and fsp_init() will call that continuation function
	 * directly.
	 *
	 * The call to fsp_init() may need to be moved out of the car_init()
	 * to cpu_init_f() with the help of some inline assembly codes.
	 * Note there is another issue that fsp_init() will setup another stack
	 * using the fsp_init parameter stack_top after DRAM is initialized,
	 * which means any data on the previous stack (on the CAR) gets lost
	 * (ie: U-Boot global_data). FSP is supposed to support such scenario,
	 * however it does not work. This should be revisited in the future.
	 */
	movl	$CONFIG_FSP_TEMP_RAM_ADDR, %eax
	xorl	%edx, %edx
	xorl	%ecx, %ecx
	call	fsp_init

.global fsp_init_done
fsp_init_done:
	/*
	 * We come here from FspInit with eax pointing to the HOB list.
	 * Save eax to esi temporarily.
	 */
	movl	%eax, %esi
	/*
	 * Re-initialize the ebp (BIST) to zero, as we already reach here
	 * which means we passed BIST testing before.
	 */
	xorl	%ebp, %ebp
	jmp	car_init_ret

car_init_fail:
	post_code(POST_CAR_FAILURE)

die:
	hlt
	jmp	die
	hlt

	/*
	 * The function call before CAR initialization is tricky. It cannot
	 * be called using the 'call' instruction but only the 'jmp' with
	 * the help of a handcrafted stack in the ROM. The stack needs to
	 * contain the function return address as well as the parameters.
	 */
	.balign	4
find_fsp_header_romstack:
	.long	find_fsp_header_ret

	.balign	4
temp_ram_init_romstack:
	.long	temp_ram_init_ret
	.long	temp_ram_init_params
temp_ram_init_params:
_dt_ucode_base_size:
	/* These next two fields are filled in by ifdtool */
	.long	0			/* microcode base */
	.long	0			/* microcode size */
	.long	CONFIG_SYS_MONITOR_BASE	/* code region base */
	.long	CONFIG_SYS_MONITOR_LEN	/* code region size */

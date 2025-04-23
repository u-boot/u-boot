// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * An implementation of initjmp() in C, that plays well with the system's
 * setjmp() and longjmp() functions.
 * Taken verbatim from arch/sandbox/os/setjmp.c in the barebox project.
 * Modified so that initjmp() accepts a stack_size argument.
 *
 * Copyright (C) 2006  Anthony Liguori <anthony@codemonkey.ws>
 * Copyright (C) 2011  Kevin Wolf <kwolf@redhat.com>
 * Copyright (C) 2012  Alex Barcelo <abarcelo@ac.upc.edu>
 * Copyright (C) 2021  Ahmad Fatoum, Pengutronix
 * Copyright (C) 2025  Linaro Ltd.
 * This file is partly based on pth_mctx.c, from the GNU Portable Threads
 *  Copyright (c) 1999-2006 Ralf S. Engelschall <rse@engelschall.com>
 */

/* XXX Is there a nicer way to disable glibc's stack check for longjmp? */
#ifdef _FORTIFY_SOURCE
#undef _FORTIFY_SOURCE
#endif

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <signal.h>

typedef sigjmp_buf _jmp_buf __attribute__((aligned((16))));
_Static_assert(sizeof(_jmp_buf) <= 512, "sigjmp_buf size exceeds expectation");

/*
 * Information for the signal handler (trampoline)
 */
static struct {
	_jmp_buf *reenter;
	void (*entry)(void);
	volatile sig_atomic_t called;
} tr_state;

/*
 * "boot" function
 * This is what starts the coroutine, is called from the trampoline
 * (from the signal handler when it is not signal handling, read ahead
 * for more information).
 */
static void __attribute__((noinline, noreturn))
coroutine_bootstrap(void (*entry)(void))
{
	for (;;)
		entry();
}

/*
 * This is used as the signal handler. This is called with the brand new stack
 * (thanks to sigaltstack). We have to return, given that this is a signal
 * handler and the sigmask and some other things are changed.
 */
static void coroutine_trampoline(int signal)
{
	/* Get the thread specific information */
	tr_state.called = 1;

	/*
	 * Here we have to do a bit of a ping pong between the caller, given that
	 * this is a signal handler and we have to do a return "soon". Then the
	 * caller can reestablish everything and do a siglongjmp here again.
	 */
	if (!sigsetjmp(*tr_state.reenter, 0)) {
		return;
	}

	/*
	 * Ok, the caller has siglongjmp'ed back to us, so now prepare
	 * us for the real machine state switching. We have to jump
	 * into another function here to get a new stack context for
	 * the auto variables (which have to be auto-variables
	 * because the start of the thread happens later). Else with
	 * PIC (i.e. Position Independent Code which is used when PTH
	 * is built as a shared library) most platforms would
	 * horrible core dump as experience showed.
	 */
	coroutine_bootstrap(tr_state.entry);
}

int __attribute__((weak)) initjmp(_jmp_buf jmp, void (*func)(void),
				  void *stack_base, size_t stack_size)
{
	struct sigaction sa;
	struct sigaction osa;
	stack_t ss;
	stack_t oss;
	sigset_t sigs;
	sigset_t osigs;

	/* The way to manipulate stack is with the sigaltstack function. We
	 * prepare a stack, with it delivering a signal to ourselves and then
	 * put sigsetjmp/siglongjmp where needed.
	 * This has been done keeping coroutine-ucontext (from the QEMU project)
	 * as a model and with the pth ideas (GNU Portable Threads).
	 * See coroutine-ucontext for the basics of the coroutines and see
	 * pth_mctx.c (from the pth project) for the
	 * sigaltstack way of manipulating stacks.
	 */

	tr_state.entry = func;
	tr_state.reenter = (void *)jmp;

	/*
	 * Preserve the SIGUSR2 signal state, block SIGUSR2,
	 * and establish our signal handler. The signal will
	 * later transfer control onto the signal stack.
	 */
	sigemptyset(&sigs);
	sigaddset(&sigs, SIGUSR2);
	pthread_sigmask(SIG_BLOCK, &sigs, &osigs);
	sa.sa_handler = coroutine_trampoline;
	sigfillset(&sa.sa_mask);
	sa.sa_flags = SA_ONSTACK;
	if (sigaction(SIGUSR2, &sa, &osa) != 0) {
		return -1;
	}

	/*
	 * Set the new stack.
	 */
	ss.ss_sp = stack_base;
	ss.ss_size = stack_size;
	ss.ss_flags = 0;
	if (sigaltstack(&ss, &oss) < 0) {
		return -1;
	}

	/*
	 * Now transfer control onto the signal stack and set it up.
	 * It will return immediately via "return" after the sigsetjmp()
	 * was performed. Be careful here with race conditions.  The
	 * signal can be delivered the first time sigsuspend() is
	 * called.
	 */
	tr_state.called = 0;
	pthread_kill(pthread_self(), SIGUSR2);
	sigfillset(&sigs);
	sigdelset(&sigs, SIGUSR2);
	while (!tr_state.called) {
		sigsuspend(&sigs);
	}

	/*
	 * Inform the system that we are back off the signal stack by
	 * removing the alternative signal stack. Be careful here: It
	 * first has to be disabled, before it can be removed.
	 */
	sigaltstack(NULL, &ss);
	ss.ss_flags = SS_DISABLE;
	if (sigaltstack(&ss, NULL) < 0) {
		return -1;
	}
	sigaltstack(NULL, &ss);
	if (!(oss.ss_flags & SS_DISABLE)) {
		sigaltstack(&oss, NULL);
	}

	/*
	 * Restore the old SIGUSR2 signal handler and mask
	 */
	sigaction(SIGUSR2, &osa, NULL);
	pthread_sigmask(SIG_SETMASK, &osigs, NULL);

	/*
	 * jmp can now be used to enter the trampoline again, but not as a
	 * signal handler. Instead it's longjmp'd to directly.
	 */
	return 0;
}


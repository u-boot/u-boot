/* Copyright MontaVista Software Incorporated, 2000 */


	.section .resetvec,"ax"
#if defined(CONFIG_440)
	b _start_440
#else
	b _start
#endif


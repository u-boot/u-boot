/*
 * (C) Copyright 2003
 * Steven Scholz, imc Measurement & Control, steven.scholz@imc-berlin.de
 *
 * (C) Copyright 2002
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 *  Altera FPGA support
 */
#include <common.h>
#include <ACEX1K.h>
#include <stratixII.h>

/* Define FPGA_DEBUG to 1 to get debug printf's */
#define FPGA_DEBUG	0

/* Local Static Functions */
static int altera_validate (Altera_desc * desc, const char *fn);

/* ------------------------------------------------------------------------- */
int altera_load(Altera_desc *desc, const void *buf, size_t bsize)
{
	int ret_val = FPGA_FAIL;	/* assume a failure */

	if (!altera_validate(desc, (char *)__func__)) {
		printf("%s: Invalid device descriptor\n", __func__);
		return FPGA_FAIL;
	}

	switch (desc->family) {
	case Altera_ACEX1K:
	case Altera_CYC2:
#if defined(CONFIG_FPGA_ACEX1K)
		debug_cond(FPGA_DEBUG,
			   "%s: Launching the ACEX1K Loader...\n",
			   __func__);
		ret_val = ACEX1K_load (desc, buf, bsize);
#elif defined(CONFIG_FPGA_CYCLON2)
		debug_cond(FPGA_DEBUG,
			   "%s: Launching the CYCLONE II Loader...\n",
			   __func__);
		ret_val = CYC2_load (desc, buf, bsize);
#else
		printf("%s: No support for ACEX1K devices.\n",
		       __func__);
#endif
		break;

#if defined(CONFIG_FPGA_STRATIX_II)
	case Altera_StratixII:
		debug_cond(FPGA_DEBUG,
			   "%s: Launching the Stratix II Loader...\n",
			   __func__);
		ret_val = StratixII_load (desc, buf, bsize);
		break;
#endif
	default:
		printf("%s: Unsupported family type, %d\n",
		       __func__, desc->family);
	}

	return ret_val;
}

int altera_dump(Altera_desc *desc, const void *buf, size_t bsize)
{
	int ret_val = FPGA_FAIL;	/* assume a failure */

	if (!altera_validate (desc, (char *)__func__)) {
		printf("%s: Invalid device descriptor\n", __func__);
		return FPGA_FAIL;
	}

	switch (desc->family) {
	case Altera_ACEX1K:
#if defined(CONFIG_FPGA_ACEX)
		debug_cond(FPGA_DEBUG,
			   "%s: Launching the ACEX1K Reader...\n",
			   __func__);
		ret_val = ACEX1K_dump (desc, buf, bsize);
#else
		printf("%s: No support for ACEX1K devices.\n",
		       __func__);
#endif
		break;

#if defined(CONFIG_FPGA_STRATIX_II)
	case Altera_StratixII:
		debug_cond(FPGA_DEBUG,
			   "%s: Launching the Stratix II Reader...\n",
			   __func__);
		ret_val = StratixII_dump (desc, buf, bsize);
		break;
#endif
	default:
		printf("%s: Unsupported family type, %d\n",
		       __func__, desc->family);
	}

	return ret_val;
}

int altera_info(Altera_desc *desc)
{
	int ret_val = FPGA_FAIL;

	if (!altera_validate (desc, (char *)__func__)) {
		printf("%s: Invalid device descriptor\n", __func__);
		return FPGA_FAIL;
	}

	printf("Family:        \t");
	switch (desc->family) {
	case Altera_ACEX1K:
		printf("ACEX1K\n");
		break;
	case Altera_CYC2:
		printf("CYCLON II\n");
		break;
	case Altera_StratixII:
		printf("Stratix II\n");
		break;
		/* Add new family types here */
	default:
		printf("Unknown family type, %d\n", desc->family);
	}

	printf("Interface type:\t");
	switch (desc->iface) {
	case passive_serial:
		printf("Passive Serial (PS)\n");
		break;
	case passive_parallel_synchronous:
		printf("Passive Parallel Synchronous (PPS)\n");
		break;
	case passive_parallel_asynchronous:
		printf("Passive Parallel Asynchronous (PPA)\n");
		break;
	case passive_serial_asynchronous:
		printf("Passive Serial Asynchronous (PSA)\n");
		break;
	case altera_jtag_mode:		/* Not used */
		printf("JTAG Mode\n");
		break;
	case fast_passive_parallel:
		printf("Fast Passive Parallel (FPP)\n");
		break;
	case fast_passive_parallel_security:
		printf("Fast Passive Parallel with Security (FPPS)\n");
		break;
		/* Add new interface types here */
	default:
		printf("Unsupported interface type, %d\n", desc->iface);
	}

	printf("Device Size:   \t%zd bytes\n"
	       "Cookie:        \t0x%x (%d)\n",
	       desc->size, desc->cookie, desc->cookie);

	if (desc->iface_fns) {
		printf("Device Function Table @ 0x%p\n", desc->iface_fns);
		switch (desc->family) {
		case Altera_ACEX1K:
		case Altera_CYC2:
#if defined(CONFIG_FPGA_ACEX1K)
			ACEX1K_info(desc);
#elif defined(CONFIG_FPGA_CYCLON2)
			CYC2_info(desc);
#else
			/* just in case */
			printf("%s: No support for ACEX1K devices.\n",
					__func__);
#endif
			break;
#if defined(CONFIG_FPGA_STRATIX_II)
		case Altera_StratixII:
			StratixII_info(desc);
			break;
#endif
			/* Add new family types here */
		default:
			/* we don't need a message here - we give one up above */
			break;
		}
	} else {
		printf("No Device Function Table.\n");
	}

	ret_val = FPGA_SUCCESS;

	return ret_val;
}

/* ------------------------------------------------------------------------- */

static int altera_validate(Altera_desc *desc, const char *fn)
{
	if (!desc) {
		printf("%s: NULL descriptor!\n", fn);
		return false;
	}

	if ((desc->family < min_altera_type) ||
	    (desc->family > max_altera_type)) {
		printf("%s: Invalid family type, %d\n", fn, desc->family);
		return false;
	}

	if ((desc->iface < min_altera_iface_type) ||
	    (desc->iface > max_altera_iface_type)) {
		printf("%s: Invalid Interface type, %d\n", fn, desc->iface);
		return false;
	}

	if (!desc->size) {
		printf("%s: NULL part size\n", fn);
		return false;
	}

	return true;
}

/* ------------------------------------------------------------------------- */

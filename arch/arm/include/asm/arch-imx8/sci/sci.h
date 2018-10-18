/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018 NXP
 */

#ifndef _SC_SCI_H
#define _SC_SCI_H

#include <asm/arch/sci/types.h>
#include <asm/arch/sci/svc/misc/api.h>
#include <asm/arch/sci/svc/pad/api.h>
#include <asm/arch/sci/svc/pm/api.h>
#include <asm/arch/sci/svc/rm/api.h>
#include <asm/arch/sci/rpc.h>
#include <dt-bindings/soc/imx_rsrc.h>
#include <linux/errno.h>

static inline int sc_err_to_linux(sc_err_t err)
{
	int ret;

	switch (err) {
	case SC_ERR_NONE:
		return 0;
	case SC_ERR_VERSION:
	case SC_ERR_CONFIG:
	case SC_ERR_PARM:
		ret = -EINVAL;
		break;
	case SC_ERR_NOACCESS:
	case SC_ERR_LOCKED:
	case SC_ERR_UNAVAILABLE:
		ret = -EACCES;
		break;
	case SC_ERR_NOTFOUND:
	case SC_ERR_NOPOWER:
		ret = -ENODEV;
		break;
	case SC_ERR_IPC:
		ret = -EIO;
		break;
	case SC_ERR_BUSY:
		ret = -EBUSY;
		break;
	case SC_ERR_FAIL:
		ret = -EIO;
		break;
	default:
		ret = 0;
		break;
	}

	debug("%s %d %d\n", __func__, err, ret);

	return ret;
}

#endif

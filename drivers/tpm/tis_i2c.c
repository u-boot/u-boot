/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <config.h>
#include <common.h>
#include <fdtdec.h>
#include <i2c.h>
#include "slb9635_i2c/tpm.h"

DECLARE_GLOBAL_DATA_PTR;

/* TPM configuration */
struct tpm {
	int i2c_bus;
	int slave_addr;
	char inited;
	int old_bus;
} tpm;


static int tpm_select(void)
{
	int ret;

	tpm.old_bus = i2c_get_bus_num();
	if (tpm.old_bus != tpm.i2c_bus) {
		ret = i2c_set_bus_num(tpm.i2c_bus);
		if (ret) {
			debug("%s: Fail to set i2c bus %d\n", __func__,
			      tpm.i2c_bus);
			return -1;
		}
	}
	return 0;
}

static int tpm_deselect(void)
{
	int ret;

	if (tpm.old_bus != i2c_get_bus_num()) {
		ret = i2c_set_bus_num(tpm.old_bus);
		if (ret) {
			debug("%s: Fail to restore i2c bus %d\n",
			      __func__, tpm.old_bus);
			return -1;
		}
	}
	tpm.old_bus = -1;
	return 0;
}

/**
 * Decode TPM configuration.
 *
 * @param dev	Returns a configuration of TPM device
 * @return 0 if ok, -1 on error
 */
static int tpm_decode_config(struct tpm *dev)
{
#ifdef CONFIG_OF_CONTROL
	const void *blob = gd->fdt_blob;
	int node, parent;
	int i2c_bus;

	node = fdtdec_next_compatible(blob, 0, COMPAT_INFINEON_SLB9635_TPM);
	if (node < 0) {
		node = fdtdec_next_compatible(blob, 0,
					      COMPAT_INFINEON_SLB9645_TPM);
	}
	if (node < 0) {
		debug("%s: Node not found\n", __func__);
		return -1;
	}
	parent = fdt_parent_offset(blob, node);
	if (parent < 0) {
		debug("%s: Cannot find node parent\n", __func__);
		return -1;
	}
	i2c_bus = i2c_get_bus_num_fdt(parent);
	if (i2c_bus < 0)
		return -1;
	dev->i2c_bus = i2c_bus;
	dev->slave_addr = fdtdec_get_addr(blob, node, "reg");
#else
	dev->i2c_bus = CONFIG_INFINEON_TPM_I2C_BUS;
	dev->slave_addr = CONFIG_INFINEON_TPM_I2C_ADDR;
#endif
	return 0;
}

int tis_init(void)
{
	if (tpm.inited)
		return 0;

	if (tpm_decode_config(&tpm))
		return -1;

	if (tpm_select())
		return -1;

	/*
	 * Probe TPM twice; the first probing might fail because TPM is asleep,
	 * and the probing can wake up TPM.
	 */
	if (i2c_probe(tpm.slave_addr) && i2c_probe(tpm.slave_addr)) {
		debug("%s: fail to probe i2c addr 0x%x\n", __func__,
		      tpm.slave_addr);
		return -1;
	}

	tpm_deselect();

	tpm.inited = 1;

	return 0;
}

int tis_open(void)
{
	int rc;

	if (!tpm.inited)
		return -1;

	if (tpm_select())
		return -1;

	rc = tpm_open(tpm.slave_addr);

	tpm_deselect();

	return rc;
}

int tis_close(void)
{
	if (!tpm.inited)
		return -1;

	if (tpm_select())
		return -1;

	tpm_close();

	tpm_deselect();

	return 0;
}

int tis_sendrecv(const uint8_t *sendbuf, size_t sbuf_size,
		uint8_t *recvbuf, size_t *rbuf_len)
{
	int len;
	uint8_t buf[4096];

	if (!tpm.inited)
		return -1;

	if (sizeof(buf) < sbuf_size)
		return -1;

	memcpy(buf, sendbuf, sbuf_size);

	if (tpm_select())
		return -1;

	len = tpm_transmit(buf, sbuf_size);

	tpm_deselect();

	if (len < 10) {
		*rbuf_len = 0;
		return -1;
	}

	memcpy(recvbuf, buf, len);
	*rbuf_len = len;

	return 0;
}

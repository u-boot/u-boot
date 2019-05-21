// SPDX-License-Identifier: GPL-2.0+
/*
 * Inter-Processor Communication with the Platform Management Unit (PMU)
 * firmware.
 *
 * (C) Copyright 2019 Luca Ceresoli
 * Luca Ceresoli <luca@lucaceresoli.net>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>

/* IPI bitmasks, register base and register offsets */
#define IPI_BIT_MASK_APU      0x00001
#define IPI_BIT_MASK_PMU0     0x10000
#define IPI_REG_BASE_APU      0xFF300000
#define IPI_REG_BASE_PMU0     0xFF330000
#define IPI_REG_OFFSET_TRIG   0x00
#define IPI_REG_OFFSET_OBR    0x04

/* IPI mailbox buffer offsets */
#define IPI_BUF_BASE_APU               0xFF990400
#define IPI_BUF_OFFSET_TARGET_PMU      0x1C0
#define IPI_BUF_OFFSET_REQ             0x00
#define IPI_BUF_OFFSET_RESP            0x20

#define PMUFW_PAYLOAD_ARG_CNT          8

/* PMUFW commands */
#define PMUFW_CMD_SET_CONFIGURATION    2

static void pmu_ipc_send_request(const u32 *req, size_t req_len)
{
	u32 *mbx = (u32 *)(IPI_BUF_BASE_APU +
			   IPI_BUF_OFFSET_TARGET_PMU +
			   IPI_BUF_OFFSET_REQ);
	size_t i;

	for (i = 0; i < req_len; i++)
		writel(req[i], &mbx[i]);
}

static void pmu_ipc_read_response(unsigned int *value, size_t count)
{
	u32 *mbx = (u32 *)(IPI_BUF_BASE_APU +
			   IPI_BUF_OFFSET_TARGET_PMU +
			   IPI_BUF_OFFSET_RESP);
	size_t i;

	for (i = 0; i < count; i++)
		value[i] = readl(&mbx[i]);
}

/**
 * Send request to PMU and get the response.
 *
 * @req:        Request buffer. Byte 0 is the API ID, other bytes are optional
 *              parameters.
 * @req_len:    Request length in number of 32-bit words.
 * @res:        Response buffer. Byte 0 is the error code, other bytes are
 *              optional parameters. Optional, if @res_maxlen==0 the parameters
 *              will not be read.
 * @res_maxlen: Space allocated for the response in number of 32-bit words.
 *
 * @return Error code returned by the PMU (i.e. the first word of the response)
 */
static int pmu_ipc_request(const u32 *req, size_t req_len,
			   u32 *res, size_t res_maxlen)
{
	u32 status;

	if (req_len > PMUFW_PAYLOAD_ARG_CNT ||
	    res_maxlen > PMUFW_PAYLOAD_ARG_CNT)
		return -EINVAL;

	pmu_ipc_send_request(req, req_len);

	/* Raise Inter-Processor Interrupt to PMU and wait for response */
	writel(IPI_BIT_MASK_PMU0, IPI_REG_BASE_APU + IPI_REG_OFFSET_TRIG);
	do {
		status = readl(IPI_REG_BASE_APU + IPI_REG_OFFSET_OBR);
	} while (status & IPI_BIT_MASK_PMU0);

	pmu_ipc_read_response(res, res_maxlen);

	return 0;
}

/**
 * Send a configuration object to the PMU firmware.
 *
 * @cfg_obj: Pointer to the configuration object
 * @size:    Size of @cfg_obj in bytes
 */
void zynqmp_pmufw_load_config_object(const void *cfg_obj, size_t size)
{
	const u32 request[] = {
		PMUFW_CMD_SET_CONFIGURATION,
		(u32)((u64)cfg_obj)
	};
	u32 response;
	int err;

	printf("Loading PMUFW cfg obj (%ld bytes)\n", size);

	err = pmu_ipc_request(request,  ARRAY_SIZE(request), &response, 1);
	if (err)
		panic("Cannot load PMUFW configuration object (%d)\n", err);
	if (response != 0)
		panic("PMUFW returned 0x%08x status!\n", response);
}

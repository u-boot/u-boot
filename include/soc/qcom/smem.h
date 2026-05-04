/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __QCOM_SMEM_H__
#define __QCOM_SMEM_H__

#include <linux/err.h>
#include <stdbool.h>

#define QCOM_SMEM_HOST_ANY -1

#if defined(CONFIG_QCOM_SMEM)
int qcom_smem_init(void);
int qcom_socinfo_init(void);

bool qcom_smem_is_available(void);
int qcom_smem_alloc(unsigned host, unsigned item, size_t size);
void *qcom_smem_get(unsigned host, unsigned item, size_t *size);

int qcom_smem_get_free_space(unsigned host);
#else
static int qcom_smem_init(void) { return -ENOSYS; }

static bool qcom_smem_is_available(void) { return false; }
int qcom_smem_alloc(unsigned host, unsigned item, size_t size)
{
	return -ENOSYS;
}

void *qcom_smem_get(unsigned host, unsigned item, size_t *size)
{
	return ERR_PTR(-ENOSYS);
}

int qcom_smem_get_free_space(unsigned host);
#endif

#endif

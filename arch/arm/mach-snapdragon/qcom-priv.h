// SPDX-License-Identifier: GPL-2.0

#ifndef __QCOM_PRIV_H__
#define __QCOM_PRIV_H__

#if CONFIG_IS_ENABLED(OF_LIVE)
/**
 * qcom_of_fixup_nodes() - Fixup Qualcomm DT nodes
 *
 * Adjusts nodes in the live tree to improve compatibility with U-Boot.
 */
void qcom_of_fixup_nodes(void);
#else
static inline void qcom_of_fixup_nodes(void)
{
	log_debug("Unable to dynamically fixup USB nodes, please enable CONFIG_OF_LIVE\n");
}
#endif /* OF_LIVE */

#endif /* __QCOM_PRIV_H__ */

// SPDX-License-Identifier: GPL-2.0+
/*
 * Generic Error-Correcting Code (ECC) engine
 *
 * Copyright (C) 2019 Macronix
 * Author:
 *     Miquèl RAYNAL <miquel.raynal@bootlin.com>
 *
 *
 * This file describes the abstraction of any NAND ECC engine. It has been
 * designed to fit most cases, including parallel NANDs and SPI-NANDs.
 *
 * There are three main situations where instantiating this ECC engine makes
 * sense:
 *   - external: The ECC engine is outside the NAND pipeline, typically this
 *               is a software ECC engine, or an hardware engine that is
 *               outside the NAND controller pipeline.
 *   - pipelined: The ECC engine is inside the NAND pipeline, ie. on the
 *                controller's side. This is the case of most of the raw NAND
 *                controllers. In the pipeline case, the ECC bytes are
 *                generated/data corrected on the fly when a page is
 *                written/read.
 *   - ondie: The ECC engine is inside the NAND pipeline, on the chip's side.
 *            Some NAND chips can correct themselves the data.
 *
 * Besides the initial setup and final cleanups, the interfaces are rather
 * simple:
 *   - prepare: Prepare an I/O request. Enable/disable the ECC engine based on
 *              the I/O request type. In case of software correction or external
 *              engine, this step may involve to derive the ECC bytes and place
 *              them in the OOB area before a write.
 *   - finish: Finish an I/O request. Correct the data in case of a read
 *             request and report the number of corrected bits/uncorrectable
 *             errors. Most likely empty for write operations, unless you have
 *             hardware specific stuff to do, like shutting down the engine to
 *             save power.
 *
 * The I/O request should be enclosed in a prepare()/finish() pair of calls
 * and will behave differently depending on the requested I/O type:
 *   - raw: Correction disabled
 *   - ecc: Correction enabled
 *
 * The request direction is impacting the logic as well:
 *   - read: Load data from the NAND chip
 *   - write: Store data in the NAND chip
 *
 * Mixing all this combinations together gives the following behavior.
 * Those are just examples, drivers are free to add custom steps in their
 * prepare/finish hook.
 *
 * [external ECC engine]
 *   - external + prepare + raw + read: do nothing
 *   - external + finish  + raw + read: do nothing
 *   - external + prepare + raw + write: do nothing
 *   - external + finish  + raw + write: do nothing
 *   - external + prepare + ecc + read: do nothing
 *   - external + finish  + ecc + read: calculate expected ECC bytes, extract
 *                                      ECC bytes from OOB buffer, correct
 *                                      and report any bitflip/error
 *   - external + prepare + ecc + write: calculate ECC bytes and store them at
 *                                       the right place in the OOB buffer based
 *                                       on the OOB layout
 *   - external + finish  + ecc + write: do nothing
 *
 * [pipelined ECC engine]
 *   - pipelined + prepare + raw + read: disable the controller's ECC engine if
 *                                       activated
 *   - pipelined + finish  + raw + read: do nothing
 *   - pipelined + prepare + raw + write: disable the controller's ECC engine if
 *                                        activated
 *   - pipelined + finish  + raw + write: do nothing
 *   - pipelined + prepare + ecc + read: enable the controller's ECC engine if
 *                                       deactivated
 *   - pipelined + finish  + ecc + read: check the status, report any
 *                                       error/bitflip
 *   - pipelined + prepare + ecc + write: enable the controller's ECC engine if
 *                                        deactivated
 *   - pipelined + finish  + ecc + write: do nothing
 *
 * [ondie ECC engine]
 *   - ondie + prepare + raw + read: send commands to disable the on-chip ECC
 *                                   engine if activated
 *   - ondie + finish  + raw + read: do nothing
 *   - ondie + prepare + raw + write: send commands to disable the on-chip ECC
 *                                    engine if activated
 *   - ondie + finish  + raw + write: do nothing
 *   - ondie + prepare + ecc + read: send commands to enable the on-chip ECC
 *                                   engine if deactivated
 *   - ondie + finish  + ecc + read: send commands to check the status, report
 *                                   any error/bitflip
 *   - ondie + prepare + ecc + write: send commands to enable the on-chip ECC
 *                                    engine if deactivated
 *   - ondie + finish  + ecc + write: do nothing
 */

#ifndef __UBOOT__
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#endif
#include <linux/mtd/nand.h>

/**
 * nand_ecc_init_ctx - Init the ECC engine context
 * @nand: the NAND device
 *
 * On success, the caller is responsible of calling @nand_ecc_cleanup_ctx().
 */
int nand_ecc_init_ctx(struct nand_device *nand)
{
	if (!nand->ecc.engine || !nand->ecc.engine->ops->init_ctx)
		return 0;

	return nand->ecc.engine->ops->init_ctx(nand);
}
EXPORT_SYMBOL(nand_ecc_init_ctx);

/**
 * nand_ecc_cleanup_ctx - Cleanup the ECC engine context
 * @nand: the NAND device
 */
void nand_ecc_cleanup_ctx(struct nand_device *nand)
{
	if (nand->ecc.engine && nand->ecc.engine->ops->cleanup_ctx)
		nand->ecc.engine->ops->cleanup_ctx(nand);
}
EXPORT_SYMBOL(nand_ecc_cleanup_ctx);

/**
 * nand_ecc_prepare_io_req - Prepare an I/O request
 * @nand: the NAND device
 * @req: the I/O request
 */
int nand_ecc_prepare_io_req(struct nand_device *nand,
			    struct nand_page_io_req *req)
{
	if (!nand->ecc.engine || !nand->ecc.engine->ops->prepare_io_req)
		return 0;

	return nand->ecc.engine->ops->prepare_io_req(nand, req);
}
EXPORT_SYMBOL(nand_ecc_prepare_io_req);

/**
 * nand_ecc_finish_io_req - Finish an I/O request
 * @nand: the NAND device
 * @req: the I/O request
 */
int nand_ecc_finish_io_req(struct nand_device *nand,
			   struct nand_page_io_req *req)
{
	if (!nand->ecc.engine || !nand->ecc.engine->ops->finish_io_req)
		return 0;

	return nand->ecc.engine->ops->finish_io_req(nand, req);
}
EXPORT_SYMBOL(nand_ecc_finish_io_req);

void of_get_nand_ecc_user_config(struct nand_device *nand)
{
	nand->ecc.user_conf.engine_type = NAND_ECC_ENGINE_TYPE_ON_DIE;
	nand->ecc.user_conf.algo = NAND_ECC_ALGO_UNKNOWN;
	nand->ecc.user_conf.placement = NAND_ECC_PLACEMENT_UNKNOWN;
}
EXPORT_SYMBOL(of_get_nand_ecc_user_config);

/**
 * nand_ecc_is_strong_enough - Check if the chip configuration meets the
 *                             datasheet requirements.
 *
 * @nand: Device to check
 *
 * If our configuration corrects A bits per B bytes and the minimum
 * required correction level is X bits per Y bytes, then we must ensure
 * both of the following are true:
 *
 * (1) A / B >= X / Y
 * (2) A >= X
 *
 * Requirement (1) ensures we can correct for the required bitflip density.
 * Requirement (2) ensures we can correct even when all bitflips are clumped
 * in the same sector.
 */
bool nand_ecc_is_strong_enough(struct nand_device *nand)
{
	const struct nand_ecc_props *reqs = nanddev_get_ecc_requirements(nand);
	const struct nand_ecc_props *conf = nanddev_get_ecc_conf(nand);
	struct mtd_info *mtd = nanddev_to_mtd(nand);
	int corr, ds_corr;

	if (conf->step_size == 0 || reqs->step_size == 0)
		/* Not enough information */
		return true;

	/*
	 * We get the number of corrected bits per page to compare
	 * the correction density.
	 */
	corr = (mtd->writesize * conf->strength) / conf->step_size;
	ds_corr = (mtd->writesize * reqs->strength) / reqs->step_size;

	return corr >= ds_corr && conf->strength >= reqs->strength;
}
EXPORT_SYMBOL(nand_ecc_is_strong_enough);

struct nand_ecc_engine *nand_ecc_get_sw_engine(struct nand_device *nand)
{
	unsigned int algo = nand->ecc.user_conf.algo;

	if (algo == NAND_ECC_ALGO_UNKNOWN)
		algo = nand->ecc.defaults.algo;

	switch (algo) {
	case NAND_ECC_ALGO_HAMMING:
		return nand_ecc_sw_hamming_get_engine();
	case NAND_ECC_ALGO_BCH:
		return nand_ecc_sw_bch_get_engine();
	default:
		break;
	}

	return NULL;
}
EXPORT_SYMBOL(nand_ecc_get_sw_engine);

struct nand_ecc_engine *nand_ecc_get_on_die_hw_engine(struct nand_device *nand)
{
	return nand->ecc.ondie_engine;
}
EXPORT_SYMBOL(nand_ecc_get_on_die_hw_engine);

struct nand_ecc_engine *nand_ecc_get_on_host_hw_engine(struct nand_device *nand)
{
	return NULL;
}
EXPORT_SYMBOL(nand_ecc_get_on_host_hw_engine);

void nand_ecc_put_on_host_hw_engine(struct nand_device *nand)
{
}
EXPORT_SYMBOL(nand_ecc_put_on_host_hw_engine);

#ifndef __UBOOT__
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Miquel Raynal <miquel.raynal@bootlin.com>");
MODULE_DESCRIPTION("Generic ECC engine");
#endif /* __UBOOT__ */

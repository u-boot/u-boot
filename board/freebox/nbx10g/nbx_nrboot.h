/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * MVEBU NRBoot (Number of Reboots) tracking header
 *
 * Copyright (C) 2026 Free Mobile, Freebox
 */

#ifndef __MVEBU_NRBOOT_H
#define __MVEBU_NRBOOT_H

#include <mmc.h>

/**
 * mvebu_check_nrboot() - Check and update reboot tracking counter
 * @mmc: MMC device
 * @offset: Byte offset in MMC where nrboot data is stored
 *
 * This function reads the reboot tracking counter, checks if we've
 * exceeded the maximum number of failed boots (4), and updates the
 * counter for the current boot attempt.
 *
 * The counter uses a bit-field encoding:
 * - nrboot: Running count of boot attempts
 * - nrsuccess: Count of successful boots
 *
 * If boot - success >= MAX_FAILURE (4), the system is considered
 * degraded and should use the fallback boot bank.
 *
 * Return: 1 if system is healthy (try newer bank first),
 *         0 if system is degraded (use stable bank first)
 */
int mvebu_check_nrboot(struct mmc *mmc, unsigned long offset);

#endif /* __MVEBU_NRBOOT_H */

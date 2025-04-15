/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2024 9elements GmbH
 */

/**
 * smc_get_mpidr() - Call into SMC and get the MPIDR for given CPU
 *
 * @id:		CPU index
 * @mpidr:	Pointer where to place the MPIDR
 * @return 0 if OK, other -ve on error
 */
int smc_get_mpidr(unsigned long id, u64 *mpidr);

/**
 * smc_get_gic_dist_base() - Call into SMC and get GIC dist base address
 *
 * @mpidr:	Pointer where to place the base address
 * @return 0 if OK, other -ve on error
 */
int smc_get_gic_dist_base(u64 *base);

/**
 * smc_get_gic_redist_base() - Call into SMC and get the GIC redistributor
 *                             base address
 *
 * @mpidr:	Pointer where to place the base address
 * @return 0 if OK, other -ve on error
 */
int smc_get_gic_redist_base(u64 *base);

/**
 * smc_get_gic_its_base() - Call into SMC and get the ITS base address
 *
 * @mpidr:	Pointer where to place the base address
 * @return 0 if OK, other -ve on error
 */
int smc_get_gic_its_base(u64 *base);

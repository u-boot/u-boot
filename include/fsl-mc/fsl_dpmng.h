/* Copyright 2014 Freescale Semiconductor Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
/*!
 *  @file    fsl_dpmng.h
 *  @brief   Management Complex General API
 */

#ifndef __FSL_DPMNG_H
#define __FSL_DPMNG_H

/*!
 * @Group grp_dpmng	Management Complex General API
 *
 * @brief	Contains general API for the Management Complex firmware
 * @{
 */

struct fsl_mc_io;

/**
 * @brief	Management Complex firmware version information
 */
#define MC_VER_MAJOR 4
#define MC_VER_MINOR 0

struct mc_version {
	uint32_t major;
	/*!< Major version number: incremented on API compatibility changes */
	uint32_t minor;
	/*!< Minor version number: incremented on API additions (that are
	 * backward compatible); reset when major version is incremented
	 */
	uint32_t revision;
	/*!< Internal revision number: incremented on implementation changes
	 * and/or bug fixes that have no impact on API
	 */
};

/**
 * @brief	Retrieves the Management Complex firmware version information
 *
 * @param[in]	mc_io		Pointer to opaque I/O object
 * @param[out]	mc_ver_info	Pointer to version information structure
 *
 * @returns	'0' on Success; Error code otherwise.
 */
int mc_get_version(struct fsl_mc_io *mc_io, struct mc_version *mc_ver_info);

/**
 * @brief	Resets an AIOP tile
 *
 * @param[in]	mc_io		Pointer to opaque I/O object
 * @param[in]	container_id	AIOP container ID
 * @param[in]	aiop_tile_id	AIOP tile ID to reset
 *
 * @returns	'0' on Success; Error code otherwise.
 */
int dpmng_reset_aiop(struct fsl_mc_io	*mc_io,
		     int		container_id,
		     int		aiop_tile_id);

/**
 * @brief	Loads an image to AIOP tile
 *
 * @param[in]	mc_io		Pointer to opaque I/O object
 * @param[in]	container_id	AIOP container ID
 * @param[in]	aiop_tile_id	AIOP tile ID to reset
 * @param[in]	img_iova	I/O virtual address of AIOP ELF image
 * @param[in]	img_size	Size of AIOP ELF image in memory (in bytes)
 *
 * @returns	'0' on Success; Error code otherwise.
 */
int dpmng_load_aiop(struct fsl_mc_io	*mc_io,
		    int			container_id,
		    int			aiop_tile_id,
		    uint64_t		img_iova,
		    uint32_t		img_size);

/**
 * @brief	AIOP run configuration
 */
struct dpmng_aiop_run_cfg {
	uint32_t cores_mask;
	/*!< Mask of AIOP cores to run (core 0 in most significant bit) */
	uint64_t options;
	/*!< Execution options (currently none defined) */
};

/**
 * @brief	Starts AIOP tile execution
 *
 * @param[in]	mc_io		Pointer to MC portal's I/O object
 * @param[in]	container_id	AIOP container ID
 * @param[in]	aiop_tile_id	AIOP tile ID to reset
 * @param[in]	cfg		AIOP run configuration
 *
 * @returns	'0' on Success; Error code otherwise.
 */
int dpmng_run_aiop(struct fsl_mc_io			*mc_io,
		   int					container_id,
		   int					aiop_tile_id,
		   const struct dpmng_aiop_run_cfg	*cfg);

/**
 * @brief	Resets MC portal
 *
 * This function closes all object handles (tokens) that are currently
 * open in the MC portal on which the command is submitted. This allows
 * cleanup of stale handles that belong to non-functional user processes.
 *
 * @param[in]	mc_io	Pointer to MC portal's I/O object
 *
 * @returns	'0' on Success; Error code otherwise.
 */
int dpmng_reset_mc_portal(struct fsl_mc_io *mc_io);

/** @} */

#endif /* __FSL_DPMNG_H */

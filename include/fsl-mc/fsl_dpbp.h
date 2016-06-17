/*
 * Freescale Layerscape MC I/O wrapper
 *
 * Copyright (C) 2013-2015 Freescale Semiconductor, Inc.
 * Author: German Rivera <German.Rivera@freescale.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
/*!
 *  @file    fsl_dpbp.h
 *  @brief   Data Path Buffer Pool API
 */
#ifndef __FSL_DPBP_H
#define __FSL_DPBP_H

/* DPBP Version */
#define DPBP_VER_MAJOR				2
#define DPBP_VER_MINOR				2

/* Command IDs */
#define DPBP_CMDID_CLOSE				0x800
#define DPBP_CMDID_OPEN					0x804
#define DPBP_CMDID_CREATE				0x904
#define DPBP_CMDID_DESTROY				0x900

#define DPBP_CMDID_ENABLE				0x002
#define DPBP_CMDID_DISABLE				0x003
#define DPBP_CMDID_GET_ATTR				0x004
#define DPBP_CMDID_RESET				0x005

/*                cmd, param, offset, width, type, arg_name */
#define DPBP_CMD_OPEN(cmd, dpbp_id) \
	MC_CMD_OP(cmd, 0, 0,  32, int,	    dpbp_id)

/*                cmd, param, offset, width, type, arg_name */
#define DPBP_RSP_GET_ATTRIBUTES(cmd, attr) \
do { \
	MC_RSP_OP(cmd, 0, 16, 16, uint16_t, attr->bpid); \
	MC_RSP_OP(cmd, 0, 32, 32, int,	    attr->id);\
	MC_RSP_OP(cmd, 1, 0,  16, uint16_t, attr->version.major);\
	MC_RSP_OP(cmd, 1, 16, 16, uint16_t, attr->version.minor);\
} while (0)

/* Data Path Buffer Pool API
 * Contains initialization APIs and runtime control APIs for DPBP
 */

struct fsl_mc_io;

/**
 * dpbp_open() - Open a control session for the specified object.
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @dpbp_id:	DPBP unique ID
 * @token:	Returned token; use in subsequent API calls
 *
 * This function can be used to open a control session for an
 * already created object; an object may have been declared in
 * the DPL or by calling the dpbp_create function.
 * This function returns a unique authentication token,
 * associated with the specific object ID and the specific MC
 * portal; this token must be used in all subsequent commands for
 * this specific object
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpbp_open(struct fsl_mc_io	*mc_io,
	      uint32_t		cmd_flags,
	      int		dpbp_id,
	      uint16_t		*token);

/**
 * dpbp_close() - Close the control session of the object
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPBP object
 *
 * After this function is called, no further operations are
 * allowed on the object without opening a new control session.
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpbp_close(struct fsl_mc_io	*mc_io,
	       uint32_t		cmd_flags,
	       uint16_t	token);

/**
 * struct dpbp_cfg - Structure representing DPBP configuration
 * @options:	place holder
 */
struct dpbp_cfg {
	uint32_t options;
};

/**
 * dpbp_create() - Create the DPBP object.
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @cfg:	Configuration structure
 * @token:	Returned token; use in subsequent API calls
 *
 * Create the DPBP object, allocate required resources and
 * perform required initialization.
 *
 * The object can be created either by declaring it in the
 * DPL file, or by calling this function.
 * This function returns a unique authentication token,
 * associated with the specific object ID and the specific MC
 * portal; this token must be used in all subsequent calls to
 * this specific object. For objects that are created using the
 * DPL file, call dpbp_open function to get an authentication
 * token first.
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpbp_create(struct fsl_mc_io	*mc_io,
		uint32_t		cmd_flags,
		const struct dpbp_cfg	*cfg,
		uint16_t		*token);

/**
 * dpbp_destroy() - Destroy the DPBP object and release all its resources.
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPBP object
 *
 * Return:	'0' on Success; error code otherwise.
 */
int dpbp_destroy(struct fsl_mc_io	*mc_io,
		 uint32_t		cmd_flags,
		 uint16_t		token);

/**
 * dpbp_enable() - Enable the DPBP.
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPBP object
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpbp_enable(struct fsl_mc_io	*mc_io,
		uint32_t		cmd_flags,
		uint16_t		token);

/**
 * dpbp_disable() - Disable the DPBP.
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPBP object
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpbp_disable(struct fsl_mc_io	*mc_io,
		 uint32_t		cmd_flags,
		 uint16_t		token);

/**
 * dpbp_is_enabled() - Check if the DPBP is enabled.
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPBP object
 * @en:		Returns '1' if object is enabled; '0' otherwise
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpbp_is_enabled(struct fsl_mc_io	*mc_io,
		    uint32_t		cmd_flags,
		    uint16_t		token,
		    int		*en);

/**
 * dpbp_reset() - Reset the DPBP, returns the object to initial state.
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPBP object
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpbp_reset(struct fsl_mc_io	*mc_io,
	       uint32_t		cmd_flags,
	       uint16_t	token);


/**
 * struct dpbp_attr - Structure representing DPBP attributes
 * @id:		DPBP object ID
 * @version:	DPBP version
 * @bpid:	Hardware buffer pool ID; should be used as an argument in
 *		acquire/release operations on buffers
 */
struct dpbp_attr {
	int id;
	/**
	 * struct version - Structure representing DPBP version
	 * @major:	DPBP major version
	 * @minor:	DPBP minor version
	 */
	struct {
		uint16_t major;
		uint16_t minor;
	} version;
	uint16_t bpid;
};

/**
 * dpbp_get_attributes - Retrieve DPBP attributes.
 *
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPBP object
 * @attr:	Returned object's attributes
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dpbp_get_attributes(struct fsl_mc_io	*mc_io,
			uint32_t	cmd_flags,
			uint16_t		token,
			struct dpbp_attr	*attr);

/** @} */

#endif /* __FSL_DPBP_H */

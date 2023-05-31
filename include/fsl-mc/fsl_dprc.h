/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Freescale Layerscape MC I/O wrapper
 *
 * Copyright 2013-2016 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 */
#ifndef _FSL_DPRC_H
#define _FSL_DPRC_H

/* DPRC Version */
#define DPRC_VER_MAJOR				6
#define DPRC_VER_MINOR				1

/* Command IDs */
#define DPRC_CMDID_CLOSE			0x8001
#define DPRC_CMDID_OPEN				0x8051

#define DPRC_CMDID_GET_API_VERSION              0xa051

#define DPRC_CMDID_CREATE_CONT			0x1511
#define DPRC_CMDID_DESTROY_CONT			0x1521
#define DPRC_CMDID_GET_CONT_ID			0x8301

#define DPRC_CMDID_CONNECT			0x1671
#define DPRC_CMDID_DISCONNECT			0x1681
#define DPRC_CMDID_GET_CONNECTION		0x16C1

/*                cmd, param, offset, width, type, arg_name */
#define DPRC_RSP_GET_CONTAINER_ID(cmd, container_id) \
	MC_RSP_OP(cmd, 0, 0,  32,  int,	    container_id)

/*                cmd, param, offset, width, type, arg_name */
#define DPRC_CMD_OPEN(cmd, container_id) \
	MC_CMD_OP(cmd, 0, 0,  32, int,	    container_id)

/*                cmd, param, offset, width, type, arg_name */
#define DPRC_CMD_CREATE_CONTAINER(cmd, cfg) \
do { \
	MC_CMD_OP(cmd, 0, 32, 16, uint16_t, cfg->icid); \
	MC_CMD_OP(cmd, 0, 0,  32, uint32_t, cfg->options); \
	MC_CMD_OP(cmd, 1, 32, 32, int,	    cfg->portal_id); \
	MC_CMD_OP(cmd, 2, 0,  8,  char,	    cfg->label[0]);\
	MC_CMD_OP(cmd, 2, 8,  8,  char,	    cfg->label[1]);\
	MC_CMD_OP(cmd, 2, 16, 8,  char,	    cfg->label[2]);\
	MC_CMD_OP(cmd, 2, 24, 8,  char,	    cfg->label[3]);\
	MC_CMD_OP(cmd, 2, 32, 8,  char,	    cfg->label[4]);\
	MC_CMD_OP(cmd, 2, 40, 8,  char,	    cfg->label[5]);\
	MC_CMD_OP(cmd, 2, 48, 8,  char,	    cfg->label[6]);\
	MC_CMD_OP(cmd, 2, 56, 8,  char,	    cfg->label[7]);\
	MC_CMD_OP(cmd, 3, 0,  8,  char,	    cfg->label[8]);\
	MC_CMD_OP(cmd, 3, 8,  8,  char,	    cfg->label[9]);\
	MC_CMD_OP(cmd, 3, 16, 8,  char,	    cfg->label[10]);\
	MC_CMD_OP(cmd, 3, 24, 8,  char,	    cfg->label[11]);\
	MC_CMD_OP(cmd, 3, 32, 8,  char,	    cfg->label[12]);\
	MC_CMD_OP(cmd, 3, 40, 8,  char,	    cfg->label[13]);\
	MC_CMD_OP(cmd, 3, 48, 8,  char,	    cfg->label[14]);\
	MC_CMD_OP(cmd, 3, 56, 8,  char,	    cfg->label[15]);\
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPRC_RSP_CREATE_CONTAINER(cmd, child_container_id, child_portal_offset)\
do { \
	MC_RSP_OP(cmd, 1, 0,  32, int,	   child_container_id); \
	MC_RSP_OP(cmd, 2, 0,  64, uint64_t, child_portal_offset);\
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPRC_CMD_DESTROY_CONTAINER(cmd, child_container_id) \
	MC_CMD_OP(cmd, 0, 0,  32, int,	    child_container_id)

/*                cmd, param, offset, width, type, arg_name */
#define DPRC_CMD_CONNECT(cmd, endpoint1, endpoint2, cfg) \
do { \
	MC_CMD_OP(cmd, 0, 0,  32, int,      endpoint1->id); \
	MC_CMD_OP(cmd, 0, 32, 32, int, endpoint1->if_id); \
	MC_CMD_OP(cmd, 1, 0,  32, int,	    endpoint2->id); \
	MC_CMD_OP(cmd, 1, 32, 32, int, endpoint2->if_id); \
	MC_CMD_OP(cmd, 2, 0,  8,  char,     endpoint1->type[0]); \
	MC_CMD_OP(cmd, 2, 8,  8,  char,	    endpoint1->type[1]); \
	MC_CMD_OP(cmd, 2, 16, 8,  char,	    endpoint1->type[2]); \
	MC_CMD_OP(cmd, 2, 24, 8,  char,	    endpoint1->type[3]); \
	MC_CMD_OP(cmd, 2, 32, 8,  char,	    endpoint1->type[4]); \
	MC_CMD_OP(cmd, 2, 40, 8,  char,	    endpoint1->type[5]); \
	MC_CMD_OP(cmd, 2, 48, 8,  char,	    endpoint1->type[6]); \
	MC_CMD_OP(cmd, 2, 56, 8,  char,	    endpoint1->type[7]); \
	MC_CMD_OP(cmd, 3, 0,  8,  char,	    endpoint1->type[8]); \
	MC_CMD_OP(cmd, 3, 8,  8,  char,	    endpoint1->type[9]); \
	MC_CMD_OP(cmd, 3, 16, 8,  char,	    endpoint1->type[10]); \
	MC_CMD_OP(cmd, 3, 24, 8,  char,	    endpoint1->type[11]); \
	MC_CMD_OP(cmd, 3, 32, 8,  char,     endpoint1->type[12]); \
	MC_CMD_OP(cmd, 3, 40, 8,  char,	    endpoint1->type[13]); \
	MC_CMD_OP(cmd, 3, 48, 8,  char,	    endpoint1->type[14]); \
	MC_CMD_OP(cmd, 3, 56, 8,  char,	    endpoint1->type[15]); \
	MC_CMD_OP(cmd, 4, 0,  32, uint32_t, cfg->max_rate); \
	MC_CMD_OP(cmd, 4, 32, 32, uint32_t, cfg->committed_rate); \
	MC_CMD_OP(cmd, 5, 0,  8,  char,	    endpoint2->type[0]); \
	MC_CMD_OP(cmd, 5, 8,  8,  char,	    endpoint2->type[1]); \
	MC_CMD_OP(cmd, 5, 16, 8,  char,	    endpoint2->type[2]); \
	MC_CMD_OP(cmd, 5, 24, 8,  char,	    endpoint2->type[3]); \
	MC_CMD_OP(cmd, 5, 32, 8,  char,	    endpoint2->type[4]); \
	MC_CMD_OP(cmd, 5, 40, 8,  char,	    endpoint2->type[5]); \
	MC_CMD_OP(cmd, 5, 48, 8,  char,	    endpoint2->type[6]); \
	MC_CMD_OP(cmd, 5, 56, 8,  char,	    endpoint2->type[7]); \
	MC_CMD_OP(cmd, 6, 0,  8,  char,	    endpoint2->type[8]); \
	MC_CMD_OP(cmd, 6, 8,  8,  char,	    endpoint2->type[9]); \
	MC_CMD_OP(cmd, 6, 16, 8,  char,	    endpoint2->type[10]); \
	MC_CMD_OP(cmd, 6, 24, 8,  char,	    endpoint2->type[11]); \
	MC_CMD_OP(cmd, 6, 32, 8,  char,	    endpoint2->type[12]); \
	MC_CMD_OP(cmd, 6, 40, 8,  char,	    endpoint2->type[13]); \
	MC_CMD_OP(cmd, 6, 48, 8,  char,	    endpoint2->type[14]); \
	MC_CMD_OP(cmd, 6, 56, 8,  char,	    endpoint2->type[15]); \
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPRC_CMD_DISCONNECT(cmd, endpoint) \
do { \
	MC_CMD_OP(cmd, 0, 0,  32, int,	    endpoint->id); \
	MC_CMD_OP(cmd, 0, 32, 16, uint16_t, endpoint->if_id); \
	MC_CMD_OP(cmd, 1, 0,  8,  char,	    endpoint->type[0]); \
	MC_CMD_OP(cmd, 1, 8,  8,  char,	    endpoint->type[1]); \
	MC_CMD_OP(cmd, 1, 16, 8,  char,	    endpoint->type[2]); \
	MC_CMD_OP(cmd, 1, 24, 8,  char,	    endpoint->type[3]); \
	MC_CMD_OP(cmd, 1, 32, 8,  char,	    endpoint->type[4]); \
	MC_CMD_OP(cmd, 1, 40, 8,  char,	    endpoint->type[5]); \
	MC_CMD_OP(cmd, 1, 48, 8,  char,	    endpoint->type[6]); \
	MC_CMD_OP(cmd, 1, 56, 8,  char,	    endpoint->type[7]); \
	MC_CMD_OP(cmd, 2, 0,  8,  char,	    endpoint->type[8]); \
	MC_CMD_OP(cmd, 2, 8,  8,  char,	    endpoint->type[9]); \
	MC_CMD_OP(cmd, 2, 16, 8,  char,	    endpoint->type[10]); \
	MC_CMD_OP(cmd, 2, 24, 8,  char,	    endpoint->type[11]); \
	MC_CMD_OP(cmd, 2, 32, 8,  char,	    endpoint->type[12]); \
	MC_CMD_OP(cmd, 2, 40, 8,  char,	    endpoint->type[13]); \
	MC_CMD_OP(cmd, 2, 48, 8,  char,	    endpoint->type[14]); \
	MC_CMD_OP(cmd, 2, 56, 8,  char,	    endpoint->type[15]); \
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPRC_CMD_GET_CONNECTION(cmd, endpoint1) \
do { \
	MC_CMD_OP(cmd, 0, 0,  32, int,	    endpoint1->id); \
	MC_CMD_OP(cmd, 0, 32, 32, int,	    endpoint1->if_id); \
	MC_CMD_OP(cmd, 1, 0,  8,  char,     endpoint1->type[0]); \
	MC_CMD_OP(cmd, 1, 8,  8,  char,	    endpoint1->type[1]); \
	MC_CMD_OP(cmd, 1, 16, 8,  char,	    endpoint1->type[2]); \
	MC_CMD_OP(cmd, 1, 24, 8,  char,	    endpoint1->type[3]); \
	MC_CMD_OP(cmd, 1, 32, 8,  char,	    endpoint1->type[4]); \
	MC_CMD_OP(cmd, 1, 40, 8,  char,	    endpoint1->type[5]); \
	MC_CMD_OP(cmd, 1, 48, 8,  char,	    endpoint1->type[6]); \
	MC_CMD_OP(cmd, 1, 56, 8,  char,	    endpoint1->type[7]); \
	MC_CMD_OP(cmd, 2, 0,  8,  char,	    endpoint1->type[8]); \
	MC_CMD_OP(cmd, 2, 8,  8,  char,	    endpoint1->type[9]); \
	MC_CMD_OP(cmd, 2, 16, 8,  char,	    endpoint1->type[10]); \
	MC_CMD_OP(cmd, 2, 24, 8,  char,	    endpoint1->type[11]); \
	MC_CMD_OP(cmd, 2, 32, 8,  char,     endpoint1->type[12]); \
	MC_CMD_OP(cmd, 2, 40, 8,  char,	    endpoint1->type[13]); \
	MC_CMD_OP(cmd, 2, 48, 8,  char,	    endpoint1->type[14]); \
	MC_CMD_OP(cmd, 2, 56, 8,  char,	    endpoint1->type[15]); \
} while (0)

/*                cmd, param, offset, width, type, arg_name */
#define DPRC_RSP_GET_CONNECTION(cmd, endpoint2, state) \
do { \
	MC_RSP_OP(cmd, 3, 0,  32, int,	    endpoint2->id); \
	MC_RSP_OP(cmd, 3, 32, 16, uint16_t, endpoint2->if_id); \
	MC_RSP_OP(cmd, 4, 0,  8,  char,	    endpoint2->type[0]); \
	MC_RSP_OP(cmd, 4, 8,  8,  char,	    endpoint2->type[1]); \
	MC_RSP_OP(cmd, 4, 16, 8,  char,	    endpoint2->type[2]); \
	MC_RSP_OP(cmd, 4, 24, 8,  char,	    endpoint2->type[3]); \
	MC_RSP_OP(cmd, 4, 32, 8,  char,	    endpoint2->type[4]); \
	MC_RSP_OP(cmd, 4, 40, 8,  char,	    endpoint2->type[5]); \
	MC_RSP_OP(cmd, 4, 48, 8,  char,	    endpoint2->type[6]); \
	MC_RSP_OP(cmd, 4, 56, 8,  char,	    endpoint2->type[7]); \
	MC_RSP_OP(cmd, 5, 0,  8,  char,	    endpoint2->type[8]); \
	MC_RSP_OP(cmd, 5, 8,  8,  char,	    endpoint2->type[9]); \
	MC_RSP_OP(cmd, 5, 16, 8,  char,	    endpoint2->type[10]); \
	MC_RSP_OP(cmd, 5, 24, 8,  char,	    endpoint2->type[11]); \
	MC_RSP_OP(cmd, 5, 32, 8,  char,	    endpoint2->type[12]); \
	MC_RSP_OP(cmd, 5, 40, 8,  char,	    endpoint2->type[13]); \
	MC_RSP_OP(cmd, 5, 48, 8,  char,	    endpoint2->type[14]); \
	MC_RSP_OP(cmd, 5, 56, 8,  char,	    endpoint2->type[15]); \
	MC_RSP_OP(cmd, 6, 0,  32, int,	    state); \
} while (0)

/* Data Path Resource Container API
 * Contains DPRC API for managing and querying DPAA resources
 */

struct fsl_mc_io;

/**
 * Set this value as the icid value in dprc_cfg structure when creating a
 * container, in case the ICID is not selected by the user and should be
 * allocated by the DPRC from the pool of ICIDs.
 */
#define DPRC_GET_ICID_FROM_POOL			(uint16_t)(~(0))

/**
 * Set this value as the portal_id value in dprc_cfg structure when creating a
 * container, in case the portal ID is not specifically selected by the
 * user and should be allocated by the DPRC from the pool of portal ids.
 */
#define DPRC_GET_PORTAL_ID_FROM_POOL	(int)(~(0))

/**
 * dprc_get_container_id() - Get container ID associated with a given portal.
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @container_id:	Requested container ID
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dprc_get_container_id(struct fsl_mc_io	*mc_io,
			  uint32_t		cmd_flags,
			  int			*container_id);

/**
 * dprc_open() - Open DPRC object for use
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @container_id: Container ID to open
 * @token:	Returned token of DPRC object
 *
 * Return:	'0' on Success; Error code otherwise.
 *
 * @warning	Required before any operation on the object.
 */
int dprc_open(struct fsl_mc_io	*mc_io,
	      uint32_t		cmd_flags,
	      int		container_id,
	      uint16_t		*token);

/**
 * dprc_close() - Close the control session of the object
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPRC object
 *
 * After this function is called, no further operations are
 * allowed on the object without opening a new control session.
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dprc_close(struct fsl_mc_io	*mc_io,
	       uint32_t		cmd_flags,
	       uint16_t		token);

/**
 * Container general options
 *
 * These options may be selected at container creation by the container creator
 * and can be retrieved using dprc_get_attributes()
 */

/* Spawn Policy Option allowed - Indicates that the new container is allowed
 * to spawn and have its own child containers.
 */
#define DPRC_CFG_OPT_SPAWN_ALLOWED		0x00000001

/* General Container allocation policy - Indicates that the new container is
 * allowed to allocate requested resources from its parent container; if not
 * set, the container is only allowed to use resources in its own pools; Note
 * that this is a container's global policy, but the parent container may
 * override it and set specific quota per resource type.
 */
#define DPRC_CFG_OPT_ALLOC_ALLOWED		0x00000002

/* Object initialization allowed - software context associated with this
 * container is allowed to invoke object initialization operations.
 */
#define DPRC_CFG_OPT_OBJ_CREATE_ALLOWED		0x00000004

/* Topology change allowed - software context associated with this
 * container is allowed to invoke topology operations, such as attach/detach
 * of network objects.
 */
#define DPRC_CFG_OPT_TOPOLOGY_CHANGES_ALLOWED	0x00000008


/* AIOP - Indicates that container belongs to AIOP. */
#define DPRC_CFG_OPT_AIOP			0x00000020

/* IRQ Config - Indicates that the container allowed to configure its IRQs.*/
#define DPRC_CFG_OPT_IRQ_CFG_ALLOWED		0x00000040

/**
 * struct dprc_cfg - Container configuration options
 * @icid: Container's ICID; if set to 'DPRC_GET_ICID_FROM_POOL', a free
 *		ICID value is allocated by the DPRC
 * @portal_id: Portal ID; if set to 'DPRC_GET_PORTAL_ID_FROM_POOL', a free
 *		portal ID is allocated by the DPRC
 * @options: Combination of 'DPRC_CFG_OPT_<X>' options
 * @label: Object's label
 */
struct dprc_cfg {
	uint16_t icid;
	int portal_id;
	uint64_t options;
	char label[16];
};

/**
 * dprc_create_container() - Create child container
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPRC object
 * @cfg:	Child container configuration
 * @child_container_id:	Returned child container ID
 * @child_portal_offset: Returned child portal offset from MC portal base
 *
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dprc_create_container(struct fsl_mc_io	*mc_io,
			  uint32_t		cmd_flags,
			  uint16_t		token,
			  struct dprc_cfg	*cfg,
			  int			*child_container_id,
			  uint64_t		*child_portal_offset);

/**
 * dprc_destroy_container() - Destroy child container.
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPRC object
 * @child_container_id:	ID of the container to destroy
 *
 * This function terminates the child container, so following this call the
 * child container ID becomes invalid.
 *
 * Notes:
 * - All resources and objects of the destroyed container are returned to the
 * parent container or destroyed if were created be the destroyed container.
 * - This function destroy all the child containers of the specified
 *   container prior to destroying the container itself.
 *
 * warning: Only the parent container is allowed to destroy a child policy
 *		Container 0 can't be destroyed
 *
 * Return:	'0' on Success; Error code otherwise.
 *
 */
int dprc_destroy_container(struct fsl_mc_io	*mc_io,
			   uint32_t		cmd_flags,
			   uint16_t		token,
			   int			child_container_id);

/**
 * struct dprc_endpoint - Endpoint description for link connect/disconnect
 *			operations
 * @type: Endpoint object type: NULL terminated string
 * @id: Endpoint object ID
 * @if_id: Interface ID; should be set for endpoints with multiple
 *		interfaces ("dpsw", "dpdmux"); for others, always set to 0
 */
struct dprc_endpoint {
	char		type[16];
	int		id;
	uint16_t	if_id;
};

/**
 * struct dprc_connection_cfg - Connection configuration.
 *				Used for virtual connections only
 * @committed_rate: Committed rate (Mbits/s)
 * @max_rate: Maximum rate (Mbits/s)
 */
struct dprc_connection_cfg {
	uint32_t committed_rate;
	uint32_t max_rate;
};

/**
 * dprc_connect() - Connect two endpoints to create a network link between them
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPRC object
 * @endpoint1:	Endpoint 1 configuration parameters
 * @endpoint2:	Endpoint 2 configuration parameters
 * @cfg: Connection configuration. The connection configuration is ignored for
 *	connections made to DPMAC objects, where rate is retrieved from the
 *	MAC configuration.
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dprc_connect(struct fsl_mc_io			*mc_io,
		 uint32_t				cmd_flags,
		 uint16_t				token,
		 const struct dprc_endpoint		*endpoint1,
		 const struct dprc_endpoint		*endpoint2,
		 const struct dprc_connection_cfg	*cfg);

/**
 * dprc_disconnect() - Disconnect one endpoint to remove its network connection
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @token:	Token of DPRC object
 * @endpoint:	Endpoint configuration parameters
 *
 * Return:	'0' on Success; Error code otherwise.
 */
int dprc_disconnect(struct fsl_mc_io		*mc_io,
		    uint32_t			cmd_flags,
		    uint16_t			token,
		    const struct dprc_endpoint	*endpoint);

/**
* dprc_get_connection() - Get connected endpoint and link status if connection
*			exists.
* @mc_io:	Pointer to MC portal's I/O object
* @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
* @token:	Token of DPRC object
* @endpoint1:	Endpoint 1 configuration parameters
* @endpoint2:	Returned endpoint 2 configuration parameters
* @state:	Returned link state:
*           1 - link is up;
*           0 - link is down;
*           -1 - no connection (endpoint2 information is irrelevant)
*
* Return:     '0' on Success; -ENAVAIL if connection does not exist.
*/
int dprc_get_connection(struct fsl_mc_io		*mc_io,
			uint32_t			cmd_flags,
			uint16_t			token,
			const struct dprc_endpoint	*endpoint1,
			struct dprc_endpoint		*endpoint2,
			int				*state);

/**
 * dprc_get_api_version - Retrieve DPRC Major and Minor version info.
 *
 * @mc_io:	Pointer to MC portal's I/O object
 * @cmd_flags:	Command flags; one or more of 'MC_CMD_FLAG_'
 * @major_ver:	DPRC major version
 * @minor_ver:	DPRC minor version
 *
 * Return:     '0' on Success; Error code otherwise.
 */
int dprc_get_api_version(struct fsl_mc_io *mc_io,
			 u32 cmd_flags,
			 u16 *major_ver,
			 u16 *minor_ver);

#endif /* _FSL_DPRC_H */

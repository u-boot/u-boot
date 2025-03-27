/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2025 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#ifndef _DM_OFNODE_GRAPH_H
#define _DM_OFNODE_GRAPH_H

#include <dm/of.h>

/**
 * ofnode_graph_get_endpoint_count() - get the number of endpoints in a device ofnode
 * @parent: ofnode to the device containing ports and endpoints
 *
 * Return: count of endpoint of this device ofnode
 */
unsigned int ofnode_graph_get_endpoint_count(ofnode parent);

/**
 * ofnode_graph_get_port_count() - get the number of port in a device or ports ofnode
 * @parent: ofnode to the device or ports node
 *
 * Return: count of port of this device or ports node
 */
unsigned int ofnode_graph_get_port_count(ofnode parent);

/**
 * ofnode_graph_get_port_by_id() - get the port matching a given id
 * @parent: parent ofnode
 * @id: id of the port
 *
 * Return: ofnode in given port.
 */
ofnode ofnode_graph_get_port_by_id(ofnode parent, u32 id);

/**
 * ofnode_graph_get_endpoint_by_regs() - get the endpoint matching a given id
 * @parent: parent ofnode
 * @reg_id: id of the port
 * @id: id for the endpoint
 *
 * Return: ofnode in given endpoint or NULL if not found.
 * reg and port_reg are ignored when they are -1.
 */
ofnode ofnode_graph_get_endpoint_by_regs(ofnode parent, u32 reg_id, u32 id);

/**
 * ofnode_graph_get_remote_endpoint() - get remote endpoint node
 * @endoint: ofnode of a local endpoint
 *
 * Return: Remote endpoint ofnode linked with local endpoint.
 */
ofnode ofnode_graph_get_remote_endpoint(ofnode endpoint);

/**
 * ofnode_graph_get_port_parent() - get port's parent node
 * @endpoint: ofnode of a local endpoint
 *
 * Return: device ofnode associated with endpoint
 */
ofnode ofnode_graph_get_port_parent(ofnode endpoint);

/**
 * ofnode_graph_get_remote_port_parent() - get remote port's parent ofnode
 * @endoint: ofnode of a local endpoint
 *
 * Return: device ofnode associated with endpoint linked to local endpoint.
 */
ofnode ofnode_graph_get_remote_port_parent(ofnode endpoint);

/**
 * ofnode_graph_get_remote_port() - get remote port ofnode
 * @endoint: ofnode of a local endpoint
 *
 * Return: port ofnode associated with remote endpoint node linked
 * to local endpoint.
 */
ofnode ofnode_graph_get_remote_port(ofnode endpoint);

/**
 * ofnode_graph_get_remote_node() - get remote parent ofnode for given port/endpoint
 * @parent: parent ofnode containing graph port/endpoint
 * @port: identifier (value of reg property) of the parent port ofnode
 * @endpoint: identifier (value of reg property) of the endpoint ofnode
 *
 * Return: device ofnode associated with endpoint linked to local endpoint.
 */
ofnode ofnode_graph_get_remote_node(ofnode parent, u32 port, u32 endpoint);

#endif

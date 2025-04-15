// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2025 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#define LOG_CATEGORY	LOGC_DT

#include <dm.h>
#include <log.h>
#include <dm/ofnode.h>
#include <linux/err.h>

/**
 * ofnode_graph_get_endpoint_count() - get the number of endpoints in a device ofnode
 * @parent: ofnode to the device containing ports and endpoints
 *
 * Return: count of endpoint of this device ofnode
 */
unsigned int ofnode_graph_get_endpoint_count(ofnode parent)
{
	ofnode ports, port, endpoint;
	unsigned int num = 0;

	/* Check if ports node exists */
	ports = ofnode_find_subnode(parent, "ports");
	if (ofnode_valid(ports))
		parent = ports;

	ofnode_for_each_subnode(port, parent) {
		if (!strncmp(ofnode_get_name(port), "port", 4)) {
			/* Port node can only contain endpoints */
			ofnode_for_each_subnode(endpoint, port)
				num++;
		}
	};

	log_debug("%s: detected %d endpoints\n", __func__, num);

	return num++;
}

/**
 * ofnode_graph_get_port_count() - get the number of port in a device or ports ofnode
 * @parent: ofnode to the device or ports node
 *
 * Return: count of port of this device or ports node
 */
unsigned int ofnode_graph_get_port_count(ofnode parent)
{
	ofnode ports, port;
	unsigned int num = 0;

	/* Check if ports node exists */
	ports = ofnode_find_subnode(parent, "ports");
	if (ofnode_valid(ports))
		parent = ports;

	ofnode_for_each_subnode(port, parent)
		if (!strncmp(ofnode_get_name(port), "port", 4))
			num++;

	log_debug("%s: detected %d ports\n", __func__, num);

	return num++;
}

/**
 * ofnode_graph_get_port_by_id() - get the port matching a given id
 * @parent: parent ofnode
 * @id: id of the port
 *
 * Return: ofnode in given port.
 */
ofnode ofnode_graph_get_port_by_id(ofnode parent, u32 id)
{
	ofnode ports, port;
	u32 port_id;

	ports = ofnode_find_subnode(parent, "ports");
	if (!ofnode_valid(ports))
		return ofnode_null();

	/* Check ports for node with desired id */
	ofnode_for_each_subnode(port, ports) {
		ofnode_read_u32(port, "reg", &port_id);
		log_debug("%s: detected port %d\n", __func__, port_id);
		if (port_id == id)
			return port;
	}

	return ofnode_null();
}

/**
 * ofnode_graph_get_endpoint_by_regs() - get the endpoint matching a given id
 * @parent: parent ofnode
 * @reg_id: id of the port
 * @id: id for the endpoint
 *
 * Return: ofnode in given endpoint or ofnode_null() if not found.
 * reg_id and id are ignored when they are -1.
 */
ofnode ofnode_graph_get_endpoint_by_regs(ofnode parent, int reg_id, int id)
{
	ofnode port, endpoint;
	u32 ep_id;

	/* get the port to work with */
	if (reg_id < 0)
		port = ofnode_find_subnode(parent, "port");
	else
		port = ofnode_graph_get_port_by_id(parent, reg_id);

	if (!ofnode_valid(port)) {
		log_debug("%s: port node is not found\n", __func__);
		return ofnode_null();
	}

	if (id < 0)
		return ofnode_find_subnode(port, "endpoint");

	/* Check endpoints for node with desired id */
	ofnode_for_each_subnode(endpoint, port) {
		ofnode_read_u32(endpoint, "reg", &ep_id);
		log_debug("%s: detected endpoint %d\n", __func__, ep_id);
		if (ep_id == id)
			return endpoint;
	}

	return ofnode_null();
}

/**
 * ofnode_graph_get_remote_endpoint() - get remote endpoint node
 * @endpoint: ofnode of a local endpoint
 *
 * Return: Remote endpoint ofnode linked with local endpoint.
 */
ofnode ofnode_graph_get_remote_endpoint(ofnode endpoint)
{
	/* Get remote endpoint node. */
	return ofnode_parse_phandle(endpoint, "remote-endpoint", 0);
}

/**
 * ofnode_graph_get_port_parent() - get port's parent node
 * @endpoint: ofnode of a local endpoint
 *
 * Return: device ofnode associated with endpoint
 */
ofnode ofnode_graph_get_port_parent(ofnode endpoint)
{
	ofnode port = ofnode_get_parent(endpoint);
	ofnode parent = ofnode_get_parent(port);

	/* check if we are on top level or in ports node */
	if (!strcmp(ofnode_get_name(parent), "ports"))
		parent = ofnode_get_parent(parent);

	return parent;
}

/**
 * ofnode_graph_get_remote_port_parent() - get remote port's parent ofnode
 * @endpoint: ofnode of a local endpoint
 *
 * Return: device ofnode associated with endpoint linked to local endpoint.
 */
ofnode ofnode_graph_get_remote_port_parent(ofnode endpoint)
{
	ofnode remote_endpoint = ofnode_graph_get_remote_endpoint(endpoint);
	if (!ofnode_valid(remote_endpoint)) {
		log_debug("%s: remote endpoint is not found\n", __func__);
		return ofnode_null();
	}

	return ofnode_graph_get_port_parent(remote_endpoint);
}

/**
 * ofnode_graph_get_remote_port() - get remote port ofnode
 * @endpoint: ofnode of a local endpoint
 *
 * Return: port ofnode associated with remote endpoint node linked
 * to local endpoint.
 */
ofnode ofnode_graph_get_remote_port(ofnode endpoint)
{
	ofnode remote_endpoint = ofnode_graph_get_remote_endpoint(endpoint);
	if (!ofnode_valid(remote_endpoint)) {
		log_debug("%s: remote endpoint is not found\n", __func__);
		return ofnode_null();
	}

	return ofnode_get_parent(remote_endpoint);
}

/**
 * ofnode_graph_get_remote_node() - get remote parent ofnode for given port/endpoint
 * @parent: parent ofnode containing graph port/endpoint
 * @port: identifier (value of reg property) of the parent port ofnode
 * @endpoint: identifier (value of reg property) of the endpoint ofnode
 *
 * Return: device ofnode associated with endpoint linked to local endpoint.
 */
ofnode ofnode_graph_get_remote_node(ofnode parent, int port, int endpoint)
{
	ofnode endpoint_ofnode;

	endpoint_ofnode = ofnode_graph_get_endpoint_by_regs(parent, port, endpoint);
	if (!ofnode_valid(endpoint_ofnode)) {
		log_debug("%s: endpoint is not found\n", __func__);
		return ofnode_null();
	}

	return ofnode_graph_get_remote_port_parent(endpoint_ofnode);
}

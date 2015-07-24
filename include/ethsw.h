/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:      GPL-2.0+
 *
 * Ethernet Switch commands
 */

#ifndef _CMD_ETHSW_H_
#define _CMD_ETHSW_H_

#define ETHSW_MAX_CMD_PARAMS 20
#define ETHSW_CMD_PORT_ALL -1

/* IDs used to track keywords in a command */
enum ethsw_keyword_id {
	ethsw_id_key_end = -1,
	ethsw_id_help,
	ethsw_id_show,
	ethsw_id_port,
	ethsw_id_enable,
	ethsw_id_disable,
	ethsw_id_count,	/* keep last */
};

enum ethsw_keyword_opt_id {
	ethsw_id_port_no = ethsw_id_count + 1,
	ethsw_id_count_all,	/* keep last */
};

struct ethsw_command_def {
	int cmd_to_keywords[ETHSW_MAX_CMD_PARAMS];
	int cmd_keywords_nr;
	int port;
	int (*cmd_function)(struct ethsw_command_def *parsed_cmd);
};

/* Structure to be created and initialized by an Ethernet Switch driver */
struct ethsw_command_func {
	const char *ethsw_name;
	int (*port_enable)(struct ethsw_command_def *parsed_cmd);
	int (*port_disable)(struct ethsw_command_def *parsed_cmd);
	int (*port_show)(struct ethsw_command_def *parsed_cmd);
};

int ethsw_define_functions(const struct ethsw_command_func *cmd_func);

#endif /* _CMD_ETHSW_H_ */

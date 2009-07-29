/*
 * caddy.c -- esd VME8349 support for "missing" access modes in TSI148.
 * Copyright (c) 2009 esd gmbh.
 *
 * Reinhard Arlt <reinhard.arlt@esd-electronics.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

#ifndef __CADDY_H__
#define __CADDY_H__

#define CMD_SIZE	1024
#define ANSWER_SIZE	1024
#define CADDY_MAGIC	"esd vme8349 V1.0"

enum caddy_cmds {
	CADDY_CMD_IO_READ_8,
	CADDY_CMD_IO_READ_16,
	CADDY_CMD_IO_READ_32,
	CADDY_CMD_IO_WRITE_8,
	CADDY_CMD_IO_WRITE_16,
	CADDY_CMD_IO_WRITE_32,
	CADDY_CMD_CONFIG_READ_8,
	CADDY_CMD_CONFIG_READ_16,
	CADDY_CMD_CONFIG_READ_32,
	CADDY_CMD_CONFIG_WRITE_8,
	CADDY_CMD_CONFIG_WRITE_16,
	CADDY_CMD_CONFIG_WRITE_32,
};

struct caddy_cmd {
	uint32_t cmd;
	uint32_t issue;
	uint32_t addr;
	uint32_t par[5];
};

struct caddy_answer {
	uint32_t answer;
	uint32_t issue;
	uint32_t status;
	uint32_t par[5];
};

struct caddy_interface {
	uint8_t  magic[16];
	uint32_t cmd_in;
	uint32_t cmd_out;
	uint32_t heartbeat;
	uint32_t reserved1;
	struct caddy_cmd cmd[CMD_SIZE];
	uint32_t answer_in;
	uint32_t answer_out;
	uint32_t reserved2;
	uint32_t reserved3;
	struct caddy_answer answer[CMD_SIZE];
};

#endif /* of __CADDY_H__ */

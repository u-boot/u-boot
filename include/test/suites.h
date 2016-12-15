/*
 * (C) Copyright 2015
 * Joe Hershberger, National Instruments, joe.hershberger@ni.com
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef __TEST_SUITES_H__
#define __TEST_SUITES_H__

int do_ut_dm(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_ut_env(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_ut_overlay(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_ut_time(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

#endif /* __TEST_SUITES_H__ */

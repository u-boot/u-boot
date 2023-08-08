// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Marvell International Ltd.
 */

#include <config.h>
#include <command.h>
#include <common.h>
#include <errno.h>
#include <console.h>
#include <env.h>
#include <mvebu/cfg_eeprom.h>

/* load the HW configuration from cfg_eeprom module and dump them */
static int cmd_hw_info_dump(char *name)
{
	int idx;
	int hw_param_num;
	struct hw_info_data_struct hw_info_data_array[HW_INFO_MAX_PARAM_NUM];

	hw_param_num = cfg_eeprom_parse_hw_info(hw_info_data_array);

	/* it is possible the HW configuration is empty */
	if (hw_param_num == 0)
		return 0;

	if (hw_param_num > MVEBU_HW_INFO_LEN) {
		pr_err("hw_info internal error, counter should not exceed %d\n",
		       MVEBU_HW_INFO_LEN);
		return -EINVAL;
	}

	printf("\nname               value\n");
	printf("------------------------------------\n");
	for (idx = 0; idx < hw_param_num; idx++) {
		if (name) {
			if (strcmp(name, hw_info_data_array[idx].name) == 0) {
				printf("%-16s   %-s\n",
				       hw_info_data_array[idx].name,
				       hw_info_data_array[idx].value);
				break;
			}
		} else {
			printf("%-16s   %-s\n",
			       hw_info_data_array[idx].name,
			       hw_info_data_array[idx].value);
		}
	}

	return 0;
}

int cmd_hw_info_load(char *name, int silence)
{
	int idx;
	int hw_param_num;
	struct hw_info_data_struct hw_info_data_array[HW_INFO_MAX_PARAM_NUM];

	/* get hw_info from system
	 * need to memset the hw_info to 0 for later string operation
	 */
	hw_param_num = cfg_eeprom_parse_hw_info(hw_info_data_array);

	/* it is possible the HW configuration is empty */
	if (hw_param_num == 0)
		return 0;

	if (hw_param_num > MVEBU_HW_INFO_LEN) {
		pr_err("HW info: variables from EEPROM can not exceed %d\n",
		       MVEBU_HW_INFO_LEN);
		return -EINVAL;
	}

	/* save the HW parameter to env varibles one by one */
	for (idx = 0; idx < hw_param_num; idx++) {
		/* if customer input a specific and valid HW parameter name,
		 * only save this HW parameter from EEPROM to env variables.
		 * otherwise save all the HW parameters from EEPROM to env.
		 */
		if (name) {
			if (strcmp(name, hw_info_data_array[idx].name) == 0) {
				env_set(hw_info_data_array[idx].name,
					hw_info_data_array[idx].value);
				break;
			}
		} else {
			env_set(hw_info_data_array[idx].name,
				hw_info_data_array[idx].value);
		}
	}

	printf("HW information is loaded to environment variables\n");
	cmd_hw_info_dump(name);

	/* just print indication to ask user to perform saveenv manually in
	 * silence mode, which is used when restore the HW configuration to env
	 * variables with env reset. to ask confirmation that if need to save
	 * env in non-silence mode, which is used by hw_info cmd.
	 */
	if (silence) {
		printf("To save the changes, please run the command saveenv\n");
	} else {
		printf("Do you want to save environment variables? <y/N> ");
		if (confirm_yesno())
			env_save();
		else
			printf("To save changes, please run command saveenv\n");
	}

	return 0;
}

static int cmd_hw_info_store(char *name)
{
	int idx;
	int str_len = 0;
	int total_str_len = 0;
	int ret;
	int hw_param_num;
	int name_in_eeprom_flag = 0;
	char *target_value;
	uchar hw_info_str[MVEBU_HW_INFO_LEN];
	struct hw_info_data_struct hw_info_data_arry[HW_INFO_MAX_PARAM_NUM];

	printf("Are you sure to override factory settings in EEPROM? <y/N>");
	if (!confirm_yesno())
		return 0;

	/* need to memset the arry to 0 for later string operation */
	memset(hw_info_data_arry, 0, sizeof(hw_info_data_arry));

	if (!name) {
		/* get hw_info from env */
		hw_param_num = cfg_eeprom_parse_env(&hw_info_data_arry[0],
						    sizeof(hw_info_data_arry));
		/* return in case no valid env variables */
		if (hw_param_num == 0) {
			printf("There is no supported HW configuration ");
			printf("in env\n");

			return 0;
		}
	} else {
		/* get hw_info from eeprom */
		hw_param_num = cfg_eeprom_parse_hw_info(hw_info_data_arry);

		/* name valid check */
		ret = cfg_eeprom_validate_name(name);
		if (ret) {
			printf("The HW parameter is invalid\n");

			return 0;
		}
		/* check the name in hw_info_data_arry */
		for (idx = 0; idx < hw_param_num; idx++) {
			if (strcmp(name, hw_info_data_arry[idx].name) == 0) {
				target_value = env_get(name);
				str_len = strlen(target_value);
				/* clear value */
				memset(hw_info_data_arry[idx].value, 0,
				       sizeof(hw_info_data_arry[idx].value));
				/* overwrite value */
				memcpy(hw_info_data_arry[idx].value,
				       target_value, str_len);
				name_in_eeprom_flag = 1;
			}
		}
		/* name is not in hw_info_data_arry */
		if (!name_in_eeprom_flag) {
			target_value = env_get(name);
			str_len = strlen(name);
			/* clear name */
			memset(hw_info_data_arry[idx].name, 0,
			       sizeof(hw_info_data_arry[idx].name));
			/* add name */
			memcpy(hw_info_data_arry[idx].name, name, str_len);

			str_len = strlen(target_value);
			/* clear value */
			memset(hw_info_data_arry[idx].value, 0,
			       sizeof(hw_info_data_arry[idx].value));
			/* add value */
			memcpy(hw_info_data_arry[idx].value, target_value,
			       str_len);

			hw_param_num++;
		}
	}

	 /* need to memset the hw_info to 0 for later string operation */
	memset(hw_info_str, 0, sizeof(hw_info_str));
	for (idx = 0;
	     (idx < hw_param_num) && (total_str_len < MVEBU_HW_INFO_LEN);
	     idx++) {
		str_len = strlen(hw_info_data_arry[idx].name);
		if (str_len > HW_INFO_MAX_NAME_LEN)
			str_len = HW_INFO_MAX_NAME_LEN;

		if ((total_str_len + str_len) > MVEBU_HW_INFO_LEN) {
			pr_err("HW info string from env exceeds %d\n",
			       MVEBU_HW_INFO_LEN);
			break;
		}

		memcpy(hw_info_str + total_str_len,
		       hw_info_data_arry[idx].name, str_len);
		total_str_len += str_len;

		if ((total_str_len + 1) > MVEBU_HW_INFO_LEN) {
			pr_err("HW information string from env exceeds %d\n",
			       MVEBU_HW_INFO_LEN);
			break;
		}

		hw_info_str[total_str_len++] = '=';

		str_len = strlen(hw_info_data_arry[idx].value);
		if (str_len > HW_INFO_MAX_VALUE_LEN)
			str_len = HW_INFO_MAX_VALUE_LEN;

		if ((total_str_len + str_len) > MVEBU_HW_INFO_LEN) {
			pr_err("HW information string from env exceeds %d\n",
			       MVEBU_HW_INFO_LEN);
			break;
		}

		memcpy(hw_info_str + total_str_len,
		       hw_info_data_arry[idx].value, str_len);
		total_str_len += str_len;

		if ((total_str_len + 1) > MVEBU_HW_INFO_LEN) {
			pr_err("HW information string from env exceeds %d\n",
			       MVEBU_HW_INFO_LEN);
			break;
		}
		hw_info_str[total_str_len++] = ' ';
	}

	cfg_eeprom_set_hw_info_str(hw_info_str);
	/* save hw_info to EEPROM, and also the rest without changing */
	cfg_eeprom_save(EEPROM_STRUCT_SIZE);

	printf("hw_info is saved to EEPROM\n");
	cmd_hw_info_dump(name);

	return 0;
}

int do_hw_info(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const char *cmd = argv[1];

	if (argc < 2)
		return CMD_RET_USAGE;

	if (!strcmp(cmd, "dump")) {
		if (cmd_hw_info_dump(argv[2]))
			return -EINVAL;
	} else if (!strcmp(cmd, "load")) {
		if (cmd_hw_info_load(argv[2], 0))
			return -EINVAL;
	} else if (!strcmp(cmd, "store")) {
		if (cmd_hw_info_store(argv[2]))
			return -EINVAL;
	} else {
		return CMD_RET_USAGE;
	}

	return 0;
}

U_BOOT_CMD(
	hw_info,      3,     0,      do_hw_info,
	"hw_info\n",
	"\n"
	"Load/Store HW information environment variables from/to EEPROM\n"
	"HW information includes predefined list of Env. variables (such as PCB SLM number, MAC addresses, etc).\n"
	"\tdump  <evn_name>            - Dump all (or specific <env_name>) HW parameter from EEPROM\n"
	"\tload  <env_name>            - Load all (or specific <env_name>) HW parameter from EEPROM to env variables\n"
	"\tstore <env_name>            - Store all or specific HW parameter from env variables to EEPROM\n"
	"\t				 usage of 'hw_info store' requires proper values to be set for the variables listed below\n"
	"Supported HW information parameters\n"
	"\tpcb_slm       PCB SLM number\n"
	"\tpcb_rev       PCB revision number\n"
	"\teco_rev       ECO revision number\n"
	"\tpcb_sn        PCB SN\n"
	"\tethaddr       first MAC address\n"
	"\teth1addr      second MAC address\n"
	"\teth2addr      third MAC address\n"
	"\teth3addr      fourth MAC address\n"
	"\teth4addr      fifth MAC address\n"
	"\teth5addr      sixth MAC address\n"
	"\teth6addr      seventh MAC address\n"
	"\teth7addr      eighth MAC address\n"
	"\teth8addr      ninth MAC address\n"
	"\teth9addr      tenth MAC address\n"
);

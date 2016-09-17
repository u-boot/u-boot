#! /bin/bash
#
# Copyright (C) 2016, Lukasz Majewski <l.majewski@majess.pl>
#
# SPDX-License-Identifier:      GPL-2.0+
#

# This file extracts default envs from built u-boot
# usage: get_default_envs.sh > u-boot-env-default.txt
set -ue

ENV_OBJ_FILE="env_common.o"
ENV_OBJ_FILE_COPY="copy_${ENV_OBJ_FILE}"

echoerr() { echo "$@" 1>&2; }

path=$(readlink -f $0)
env_obj_file_path=$(find ${path%/scripts*} -not -path "*/spl/*" \
			 -name "${ENV_OBJ_FILE}")
[ -z "${env_obj_file_path}" ] && \
    { echoerr "File '${ENV_OBJ_FILE}' not found!"; exit 1; }

cp ${env_obj_file_path} ${ENV_OBJ_FILE_COPY}

# NOTE: objcopy saves its output to file passed in
# (copy_env_common.o in this case)
objcopy -O binary -j ".rodata.default_environment" ${ENV_OBJ_FILE_COPY}

# Replace default '\0' with '\n' and sort entries
tr '\0' '\n' < ${ENV_OBJ_FILE_COPY} | sort -u

rm ${ENV_OBJ_FILE_COPY}

exit 0

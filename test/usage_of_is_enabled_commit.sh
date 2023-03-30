#!/bin/bash
# SPDX-License-Identifier: GPL-2.0+
#
# Written by Troy Kisky <troykiskyboundary@gmail.com>

scriptdir=`dirname "$0"`;
${scriptdir}/usage_of_is_enabled_list.sh | \
xargs -I {} sh -c "${scriptdir}/usage_of_is_enabled_correct.sh {}; \
git commit -a -m\"CONFIG_{}: correct usage of CONFIG_IS_ENABLED/IS_ENABLED\";"


rm ${scriptdir}/splcfg.tmp ${scriptdir}/exclude.tmp

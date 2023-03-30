#!/bin/bash
# SPDX-License-Identifier: GPL-2.0+
#
# Written by Troy Kisky <troykiskyboundary@gmail.com>

scriptdir=`dirname "$0"`;
${scriptdir}/usage_of_is_enabled_list.sh | grep -vw FOO;
if [ $? -eq 0 ] ; then
	echo "The above may have incorrect usage of IS_ENABLED/"\
"CONFIG_IS_ENABLED"
	echo "Run test/usage_of_is_enabled_commit.sh and "\
"squash with appropriate commit"
	ret=1;
else
	ret=0;
fi

rm ${scriptdir}/splcfg.tmp ${scriptdir}/exclude.tmp
exit ${ret}

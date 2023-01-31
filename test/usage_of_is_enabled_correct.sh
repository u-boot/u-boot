#!/bin/bash
# SPDX-License-Identifier: GPL-2.0+
#
# Written by Troy Kisky <troykiskyboundary@gmail.com>

scriptdir=`dirname "$0"`;

if [ -z "$1" ] ; then
	echo missing config
	exit 1;
fi
if [ ! -f "${scriptdir}/splcfg.tmp" ] ; then
	echo missing splcfg.tmp
	exit 1;
fi


grep -qw $1 ${scriptdir}/splcfg.tmp
if [ $? -ne 0 ] ; then
    # not splcfg
    # change CONFIG_IS_ENABLED to IS_ENABLED
    git grep -l \
    -e "CONFIG_IS_ENABLED($1)" \
     | \
    xargs -IFile sh -c \
    " \
    sed -i -E \"\
s/CONFIG_IS_ENABLED\($1\)/IS_ENABLED\(CONFIG_$1\)/g; \
\" File";
else
    # splcfg
    # change IS_ENABLED to CONFIG_IS_ENABLED
    # change ifdef to CONFIG_IS_ENABLED
    # change ifndef to !CONFIG_IS_ENABLED
    # change defined to CONFIG_IS_ENABLED
    git grep -l \
    -e "IS_ENABLED(CONFIG_$1)" \
    -e "^#ifdef[ \t]\+CONFIG_$1\>" \
    -e "^#ifndef[ \t]\+CONFIG_$1\>" \
    -e "defined(CONFIG_$1)" \
     | \
    xargs -IFile sh -c \
    " \
    sed -i -E \"\
s/([^_])IS_ENABLED\(CONFIG_$1\)/\1CONFIG_IS_ENABLED($1)/g; \
s/^#ifdef[ \t]+CONFIG_$1\>/#if CONFIG_IS_ENABLED\($1\)/; \
s/^#ifndef[ \t]+CONFIG_$1\>/#if !CONFIG_IS_ENABLED\($1\)/; \
s/defined\(CONFIG_$1\)/CONFIG_IS_ENABLED\($1\)/; \
\" File";
fi

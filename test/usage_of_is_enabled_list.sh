#!/bin/bash
# SPDX-License-Identifier: GPL-2.0+
#
# Written by Troy Kisky <troykiskyboundary@gmail.com>

scriptdir=`dirname "$0"`;
# generate list of excluded configs
{
# 1. ignore configs that have a number or string for a value
git grep -h -A2 -E "^config " '*Kconfig*' | \
sed -En '/depends on/!p' | \
sed -En '/^config/{h;$!d} ;H;x; s/config[ \t]+(.*)\n[ \t]*/config \1 #/p' | \
sed -E "/#bool/d; /#def_bool/d; /#tristate/d; \
/#default y/d; /#select/d; /#prompt/d; /#imply/d" |
sed -n -r "s/^config[[:space:]]+([0-9a-zA-Z_]+)/\n\{\1\}\n/p" | \
sed -n -r 's/^\{([0-9a-zA-Z_]+)\}/\1/p' | sort -u;
# 2. configs that are exempt for other reasons
cat ${scriptdir}/usage_of_is_enabled_exempt.txt;
# 3. configs that need converted later
[ -f ${scriptdir}/usage_of_is_enabled_todo.txt ] && \
cat ${scriptdir}/usage_of_is_enabled_todo.txt
} | sort -u > ${scriptdir}/exclude.tmp

# generate list of CONFIGs that should use CONFIG_IS_ENABLED
{
# 1. all obj-$(CONFIG_$(SPL_)xxx in Makefiles
git grep -h 'obj-$(CONFIG_$(SPL_' '*Makefile' | sed -e "s/SPL_TPL_/SPL_/"| \
sed -n -r 's/obj\-\$\(CONFIG_\$\(SPL_\)([0-9a-zA-Z_]+)\)/\n\{\1\}\n/gp'| \
sed -n -r 's/\{([0-9a-zA-Z_]+)\}/\1/p';

# 2. all SPL_xxx in Kconfig files
git grep -h -E 'config [ST]PL_' '*Kconfig*' | \
sed -n -r "s/config [ST]PL_([0-9a-zA-Z_]+)/\n\{\1\}\n/p" | \
sed -n -r 's/\{([0-9a-zA-Z_]+)\}/\1/p';

# 3. all CONFIG_CMD_xxx which already use CONFIG_IS_ENABLED
#    The Makefile for most if these use ifndef CONFIG_SPL_BUILD
#    instead of obj-$(CONFIG_$(SPL_)xxx
git grep -h -E 'CONFIG_IS_ENABLED\(CMD_' | \
sed -n -e "s/\(CONFIG_IS_ENABLED(CMD_[0-9a-zA-Z_]*)\)/\n\1\n/gp"| \
sed -n -r "s/CONFIG_IS_ENABLED\((CMD_[0-9a-zA-Z_]+)\)/\1/p";

# 4. A list of other configs that should use CONFIG_IS_ENABLED
#    This list could be reduced if obj-$(CONFIG_$(SPL_)xxx was used instead of
#    ifndef CONFIG_SPL_BUILD in Makefiles
# usage_of_is_enabled_splcfg.txt mostly contains configs that should always
# be undefined in SPL/TPL
# Note: CONFIG_CLK was included to prevent a change in test_checkpatch.py
# which is checking for an error.
cat ${scriptdir}/usage_of_is_enabled_splcfg.txt;
} | sort -u | \
comm -23 - ${scriptdir}/exclude.tmp >${scriptdir}/splcfg.tmp

{
# generate list of CONFIGs that incorrectly use CONFIG_IS_ENABLED
git grep -h CONFIG_IS_ENABLED | \
sed -n -e "s/\(CONFIG_IS_ENABLED([0-9a-zA-Z_]*)\)/\n\1\n/gp"| \
sed -n -r "s/CONFIG_IS_ENABLED\(([0-9a-zA-Z_]+)\)/\1/p" |sort -u| \
comm -23 - ${scriptdir}/exclude.tmp | \
comm -23 - ${scriptdir}/splcfg.tmp ;

# generate list of CONFIGs that incorrectly use IS_ENABLED
git grep -h -w IS_ENABLED | \
sed -n -e "s/\(IS_ENABLED(CONFIG_[0-9a-zA-Z_]*)\)/\n\1\n/gp"| \
sed -n -r "s/IS_ENABLED\(CONFIG_([0-9a-zA-Z_]+)\)/\1/p" |sort -u| \
join - ${scriptdir}/splcfg.tmp;

# generate list of CONFIGs that incorrectly use ifdef
git grep -h -E "^#ifdef[ \t]+CONFIG_" | \
sed -n -E "s/(ifdef[ \t]+CONFIG_[0-9a-zA-Z_]+)/\n\1\n/p"| \
sed -n -E "s/ifdef[ \t]+CONFIG_([0-9a-zA-Z_]+)/\1/p" |sort -u| \
join - ${scriptdir}/splcfg.tmp ;

# generate list of CONFIGs that incorrectly use ifndef
git grep -h -E "^#ifndef[ \t]+CONFIG_" | \
sed -n -E "s/(ifndef[ \t]+CONFIG_[0-9a-zA-Z_]+)/\n\1\n/p"| \
sed -n -E "s/ifndef[ \t]+CONFIG_([0-9a-zA-Z_]+)/\1/p" |sort -u| \
join - ${scriptdir}/splcfg.tmp ;

# generate list of CONFIGs that incorrectly use defined
git grep -h -E "defined\(CONFIG_" | \
sed -n -E "s/(defined\(CONFIG_[0-9a-zA-Z_]+\))/\n\1\n/gp"| \
sed -n -E "s/defined\(CONFIG_([0-9a-zA-Z_]+)\)/\1/p" |sort -u| \
join - ${scriptdir}/splcfg.tmp ;

} | sort -u;

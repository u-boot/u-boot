#!/bin/sh
#
# dtc-version dtc-command
#
# Prints the dtc version of `dtc-command' in a canonical 4-digit form
# such as `0222' for binutils 2.22
#

dtc="$*"

if [ ${#dtc} -eq 0 ]; then
	echo "Error: No dtc command specified."
	printf "Usage:\n\t$0 <dtc-command>\n"
	exit 1
fi

MAJOR=$($dtc -v | head -1 | awk '{print $NF}' | cut -d . -f 1)
MINOR=$($dtc -v | head -1 | awk '{print $NF}' | cut -d . -f 2)

printf "%02d%02d\\n" $MAJOR $MINOR

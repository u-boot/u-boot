#!/bin/sh
# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2024 Linaro Ltd.
#
# Usage: from the top level U-Boot source tree, run:
# $ ./lib/mbedtls/update-mbedtls-subtree.sh pull <release-tag>
# $ ./lib/mbedtls/update-mbedtls-subtree.sh pick <commit-id>
#
# The script will pull changes from MbedTLS repo into U-Boot
# as a subtree located as <U-Boot>/lib/mbedtls/external/mbedtls sub-directory.
# It will automatically create a squash/merge commit listing the commits
# imported.

set -e

merge_commit_msg=$(cat << EOF
Subtree merge tag '$2' of MbedTLS repo [1] into lib/mbedtls/external/mbedtls

[1] https://github.com/Mbed-TLS/mbedtls.git
EOF
)

remote_add_and_fetch() {
    if ! git remote get-url mbedtls_upstream 2>/dev/null
    then
        echo "Warning: Script automatically adds new git remote via:"
        echo "    git remote add mbedtls_upstream \\"
        echo "        https://github.com/Mbed-TLS/mbedtls.git"
        git remote add mbedtls_upstream \
            https://github.com/Mbed-TLS/mbedtls.git
    fi
    git fetch mbedtls_upstream master
}

if [ "$1" = "pull" ]
then
    remote_add_and_fetch
    git subtree pull --prefix lib/mbedtls/external/mbedtls mbedtls_upstream \
        "$2" --squash -m "${merge_commit_msg}"
elif [ "$1" = "pick" ]
then
    remote_add_and_fetch
    git cherry-pick -x --strategy=subtree \
        -Xsubtree=lib/mbedtls/external/mbedtls/ "$2"
else
    echo "usage: $0 <op> <ref>"
    echo "  <op>     pull or pick"
    echo "  <ref>    release tag [pull] or commit id [pick]"
fi

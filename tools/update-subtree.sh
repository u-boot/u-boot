#!/bin/sh
# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2024 Linaro Limited
# Author: Raymond Mao <raymond.mao@linaro.org>
#
# Usage: from the top level U-Boot source tree, run:
# $ ./tools/update-subtree.sh pull <subtree-name> <release-tag>
# Or:
# $ ./tools/update-subtree.sh pick <subtree-name> <commit-id>
#
# The script will pull changes from subtree repo into U-Boot.
# It will automatically create a squash/merge commit listing the commits
# imported.

set -e

repo_url_mbedtls="https://github.com/Mbed-TLS/mbedtls.git"
repo_url_dts="https://git.kernel.org/pub/scm/linux/kernel/git/devicetree/devicetree-rebasing.git"
repo_url_lwip="https://git.savannah.gnu.org/git/lwip.git"

path_mbedtls="lib/mbedtls/external/mbedtls"
path_dts="dts/upstream"
path_lwlip="lib/lwip/lwip"

print_usage() {
    echo "usage: $0 <op> <subtree-name> <ref>"
    echo "  <op>           pull or pick"
    echo "  <subtree-name> mbedtls or dts or lwip"
    echo "  <ref>          release tag [pull] or commit id [pick]"
}

if [ $# -ne 3 ]; then
    print_usage
    exit 1
fi

op=$1
subtree_name=$2
ref=$3

if [ "$subtree_name" = "mbedtls" ]; then
    repo_url=$repo_url_mbedtls
    remote_name="mbedtls_upstream"
    path=$path_mbedtls
elif [ "$subtree_name" = "dts" ]; then
    repo_url=$repo_url_dts
    remote_name="devicetree-rebasing"
    path=$path_dts
elif [ "$subtree_name" = "lwip" ]; then
    repo_url=$repo_url_lwip
    remote_name="lwip_upstream"
    path=$path_lwip
else
    echo "Invalid subtree name: $subtree_name"
    print_usage
    exit 1
fi

merge_commit_msg=$(cat << EOF
Subtree merge tag '$ref' of $subtree_name repo [1] into $path

[1] $repo_url
EOF
)

remote_add_and_fetch() {
    if [ -z "$(git remote get-url $remote_name 2>/dev/null)" ]; then
        echo "Warning: Script automatically adds new git remote via:"
        echo "    git remote add $remote_name \\"
        echo "        $repo_url"
        git remote add $remote_name $repo_url
    fi
    git fetch $remote_name master
}

if [ "$op" = "pull" ]; then
    remote_add_and_fetch
    git subtree pull --prefix $path $remote_name "$ref" --squash -m "$merge_commit_msg"
elif [ "$op" = "pick" ]; then
    remote_add_and_fetch
    git cherry-pick -x --strategy=subtree -Xsubtree=$path/ "$ref"
else
    print_usage
    exit 1
fi

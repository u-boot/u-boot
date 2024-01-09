#!/bin/sh
# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2024 Linaro Ltd.
#
# Usage: from the top level U-Boot source tree, run:
# $ ./dts/update-dts-subtree.sh pull <release-tag>
# $ ./dts/update-dts-subtree.sh pick <commit-id>
#
# The script will pull changes from devicetree-rebasing repo into U-Boot
# as a subtree located as <U-Boot>/dts/upstream sub-directory. It will
# automatically create a squash/merge commit listing the commits imported.

set -e

merge_commit_msg=$(cat << EOF
Subtree merge tag '$2' of devicetree-rebasing repo [1] into dts/upstream

[1] https://git.kernel.org/pub/scm/linux/kernel/git/devicetree/devicetree-rebasing.git/
EOF
)

remote_add() {
    if ! git remote | grep -w devicetree-rebasing
    then
        git remote add devicetree-rebasing \
            https://git.kernel.org/pub/scm/linux/kernel/git/devicetree/devicetree-rebasing.git
    fi
    git fetch devicetree-rebasing master
}

if [ ! -z $1 ] && [ $1 = "pull" ]
then
    remote_add
    git subtree pull --prefix dts/upstream devicetree-rebasing \
        $2 --squash -m "${merge_commit_msg}"
elif [ ! -z $1 ] && [ $1 = "pick" ]
then
    remote_add
    git cherry-pick -x --strategy=subtree -Xsubtree=dts/upstream/ $2
else
    echo "usage: $0 param1 param2"
    echo "  param1    pull or pick"
    echo "  param2    release tag [pull] or commit id [pick]"
fi

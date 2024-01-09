#!/bin/sh
# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2024 Linaro Ltd.
#
# Usage: from the top level U-Boot source tree, run:
# $ ./dts/update-dts-subtree.sh <release-tag>
#
# The script will pull changes from devicetree-rebasing repo into U-Boot
# as a subtree located as <U-Boot>/dts/upstream sub-directory. It will
# automatically create a squash/merge commit listing the commits imported.

set -e

merge_commit_msg=$(cat << EOF
Subtree merge tag '$1' of devicetree-rebasing repo [1] into dts/upstream

[1] https://git.kernel.org/pub/scm/linux/kernel/git/devicetree/devicetree-rebasing.git/
EOF
)

git subtree pull --prefix dts/upstream \
    git://git.kernel.org/pub/scm/linux/kernel/git/devicetree/devicetree-rebasing.git \
    $1 --squash -m "${merge_commit_msg}"

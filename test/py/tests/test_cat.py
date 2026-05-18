# SPDX-License-Identifier:      GPL-2.0+

""" Unit test for cat command
"""

import pytest
from tests.fs_helper import FsHelper

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_cat')
def test_cat(ubman):
    """ Unit test for cat

    Args:
        ubman -- U-Boot console
    """
    with FsHelper(ubman.config, 'vfat', 1, 'test_cat') as fsh:
        with open(f'{fsh.srcdir}/hello', 'w', encoding = 'ascii') as outf:
            outf.write('hello world\n')
        fsh.mk_fs()

        response = ubman.run_command_list([f'host bind 0 {fsh.fs_img}',
                                           'cat host 0 hello'])
        assert 'hello world' in response

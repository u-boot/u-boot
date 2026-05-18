# SPDX-License-Identifier:      GPL-2.0+

""" Unit test for xxd command
"""

import pytest
from tests.fs_helper import FsHelper

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_xxd')
def test_xxd(ubman):
    """ Unit test for xxd

    Args:
        ubman -- U-Boot console
    """
    with FsHelper(ubman.config, 'vfat', 1, 'test_xxd') as fsh:
        with open(f'{fsh.srcdir}/hello', 'w', encoding = 'ascii') as outf:
            outf.write('hello world\n\x00\x01\x02\x03\x04\x05')
        fsh.mk_fs()
        response = ubman.run_command_list([f'host bind 0 {fsh.fs_img}',
                                           'xxd host 0 hello'])

        assert '00000000: 68 65 6c 6c 6f 20 77 6f 72 6c 64 0a 00 01 02 03  hello world.....\r\r\n' + \
               '00000010: 04 05                                            ..' \
               in response

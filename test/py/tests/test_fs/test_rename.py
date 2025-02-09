# SPDX-License-Identifier:      GPL-2.0+
# Copyright 2025 Gabriel Dalimonte <gabriel.dalimonte@gmail.com>
#
# U-Boot File System:rename Test


import pytest

from fstest_defs import *
from fstest_helpers import assert_fs_integrity

@pytest.mark.boardspec('sandbox')
@pytest.mark.slow
class TestRename(object):
    def test_rename1(self, ubman, fs_obj_rename):
        """
        Test Case 1 - rename a file (successful mv)
        """
        fs_type, fs_img, md5val = fs_obj_rename
        with ubman.log.section('Test Case 1 - rename a file'):
            d = 'test1'
            src = '%s/file1' % d
            dst = '%s/file2' % d
            output = ubman.run_command_list([
                'host bind 0 %s' % fs_img,
                'setenv filesize',
                'mv host 0:0 %s %s' % (src, dst),
            ])
            assert('' == ''.join(output))

            output = ubman.run_command_list([
                'load host 0:0 %x /%s' % (ADDR, dst),
                'printenv filesize'])
            assert('filesize=400' in output)

            output = ubman.run_command_list([
                'ls host 0:0 %s' % (d),
            ])
            assert('file1' not in ''.join(output))

            output = ubman.run_command_list([
                'md5sum %x $filesize' % ADDR,
                'setenv filesize'])
            assert(md5val['test1'] in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)

    def test_rename2(self, ubman, fs_obj_rename):
        """
        Test Case 2 - rename a file to an existing file (successful mv)
        """
        fs_type, fs_img, md5val = fs_obj_rename
        with ubman.log.section('Test Case 2 - rename a file to an existing file'):
            d = 'test2'
            src = '%s/file1' % d
            dst = '%s/file_exist' % d
            output = ubman.run_command_list([
                'host bind 0 %s' % fs_img,
                'setenv filesize',
                'mv host 0:0 %s %s' % (src, dst),
            ])
            assert('' == ''.join(output))

            output = ubman.run_command_list([
                'load host 0:0 %x /%s' % (ADDR, dst),
                'printenv filesize'])
            assert('filesize=400' in output)

            output = ubman.run_command_list([
                'ls host 0:0 %s' % (d),
            ])
            assert('file1' not in ''.join(output))

            output = ubman.run_command_list([
                'md5sum %x $filesize' % ADDR,
                'setenv filesize'])
            assert(md5val['test2'] in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)

    def test_rename3(self, ubman, fs_obj_rename):
        """
        Test Case 3 - rename a directory (successful mv)
        """
        fs_type, fs_img, md5val = fs_obj_rename
        with ubman.log.section('Test Case 3 - rename a directory'):
            d = 'test3'
            src = '%s/dir1' % d
            dst = '%s/dir2' % d
            output = ubman.run_command_list([
                'host bind 0 %s' % fs_img,
                'setenv filesize',
                'mv host 0:0 %s %s' % (src, dst),
            ])
            assert('' == ''.join(output))

            output = ubman.run_command_list([
                'load host 0:0 %x /%s/file1' % (ADDR, dst),
                'printenv filesize'])
            assert('filesize=400' in output)

            output = ubman.run_command_list([
                'ls host 0:0 %s' % (d),
            ])
            assert('dir1' not in ''.join(output))

            output = ubman.run_command_list([
                'md5sum %x $filesize' % ADDR,
                'setenv filesize'])
            assert(md5val['test3'] in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)

    def test_rename4(self, ubman, fs_obj_rename):
        """
        Test Case 4 - rename a directory to an existing directory (successful
        mv)
        """
        fs_type, fs_img, md5val = fs_obj_rename
        with ubman.log.section('Test Case 4 - rename a directory to an existing directory'):
            d = 'test4'
            src = '%s/dir1' % d
            dst = '%s/dir2' % d
            output = ubman.run_command_list([
                'host bind 0 %s' % fs_img,
                'setenv filesize',
                'mv host 0:0 %s %s' % (src, dst),
            ])
            assert('' == ''.join(output))

            output = ubman.run_command_list([
                'load host 0:0 %x /%s/dir1/file1' % (ADDR, dst),
                'printenv filesize'])
            assert('filesize=400' in output)

            output = ubman.run_command_list([
                'ls host 0:0 %s' % (d),
            ])
            assert('dir1' not in ''.join(output))

            output = ubman.run_command_list([
                'md5sum %x $filesize' % ADDR,
                'setenv filesize'])
            assert(md5val['test4'] in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)

    def test_rename5(self, ubman, fs_obj_rename):
        """
        Test Case 5 - rename a directory to an existing file (failed mv)
        """
        fs_type, fs_img, md5val = fs_obj_rename
        with ubman.log.section('Test Case 5 - rename a directory to an existing file'):
            d = 'test5'
            src = '%s/dir1' % d
            dst = '%s/file2' % d
            output = ubman.run_command_list([
                'host bind 0 %s' % fs_img,
                'setenv filesize',
                'mv host 0:0 %s %s' % (src, dst),
            ])
            assert('' == ''.join(output))

            output = ubman.run_command_list([
                'ls host 0:0 %s' % (d),
            ])
            assert('dir1' in ''.join(output))
            assert('file2' in ''.join(output))

            output = ubman.run_command_list([
                'load host 0:0 %x /%s' % (ADDR, dst),
                'printenv filesize'])
            assert('filesize=400' in output)

            output = ubman.run_command_list([
                'md5sum %x $filesize' % ADDR,
                'setenv filesize'])
            assert(md5val['test5'] in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)

    def test_rename6(self, ubman, fs_obj_rename):
        """
        Test Case 6 - rename a file to an existing empty directory (failed mv)
        """
        fs_type, fs_img, md5val = fs_obj_rename
        with ubman.log.section('Test Case 6 - rename a file to an existing empty directory'):
            d = 'test6'
            src = '%s/existing' % d
            dst = '%s/dir2' % d
            output = ubman.run_command_list([
                'host bind 0 %s' % fs_img,
                'setenv filesize',
                'mv host 0:0 %s %s' % (src, dst),
            ])
            assert('' == ''.join(output))

            output = ubman.run_command_list([
                'load host 0:0 %x /%s' % (ADDR, src),
                'printenv filesize'])
            assert('filesize=400' in output)

            output = ubman.run_command_list([
                'ls host 0:0 %s' % (d),
            ])
            assert('dir2' in ''.join(output))
            assert('existing' in ''.join(output))

            output = ubman.run_command_list([
                'md5sum %x $filesize' % ADDR,
                'setenv filesize'])
            assert(md5val['test6'] in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)

    def test_rename7(self, ubman, fs_obj_rename):
        """
        Test Case 7 - rename a directory to a non-empty directory (failed mv)
        """
        fs_type, fs_img, md5val = fs_obj_rename
        with ubman.log.section('Test Case 7 - rename a directory to a non-empty directory'):
            d = 'test7'
            src = '%s/dir1' % d
            dst = '%s/dir2' % d
            output = ubman.run_command_list([
                'host bind 0 %s' % fs_img,
                'setenv filesize',
                'mv host 0:0 %s %s' % (src, dst),
            ])
            assert('' == ''.join(output))

            output = ubman.run_command_list([
                'load host 0:0 %x /%s/dir1/file1' % (ADDR, dst),
                'printenv filesize'])
            assert('filesize=400' in output)

            output = ubman.run_command_list([
                'ls host 0:0 %s' % (d),
            ])
            assert('dir1' in ''.join(output))
            assert('dir2' in ''.join(output))

            output = ubman.run_command_list([
                'md5sum %x $filesize' % ADDR,
                'setenv filesize'])
            assert(md5val['test7'] in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)

    def test_rename8(self, ubman, fs_obj_rename):
        """
        Test Case 8 - rename a directory inside itself (failed mv)
        """
        fs_type, fs_img, md5val = fs_obj_rename
        with ubman.log.section('Test Case 8 - rename a directory inside itself'):
            d = 'test8'
            src = '%s/dir1' % d
            dst = '%s/dir1/dir1' % d
            output = ubman.run_command_list([
                'host bind 0 %s' % fs_img,
                'setenv filesize',
                'mv host 0:0 %s %s' % (src, dst),
            ])
            assert('' == ''.join(output))

            output = ubman.run_command_list([
                'load host 0:0 %x /%s/file1' % (ADDR, src),
                'printenv filesize'])
            assert('filesize=400' in output)

            output = ubman.run_command_list([
                'ls host 0:0 %s' % (d),
            ])
            assert('dir1' in ''.join(output))

            output = ubman.run_command_list([
                'ls host 0:0 %s' % (src),
            ])
            assert('file1' in ''.join(output))
            assert('dir1' not in ''.join(output))

            output = ubman.run_command_list([
                'md5sum %x $filesize' % ADDR,
                'setenv filesize'])
            assert(md5val['test8'] in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)

    def test_rename9(self, ubman, fs_obj_rename):
        """
        Test Case 9 - rename a directory inside itself with backtracks (failed
        mv)
        """
        fs_type, fs_img, md5val = fs_obj_rename
        with ubman.log.section('Test Case 9 - rename a directory inside itself with backtracks'):
            d = 'test9'
            src = '%s/dir1/nested' % d
            dst = '%s/dir1/nested/inner/./../../../dir1/nested/inner/another' % d
            output = ubman.run_command_list([
                'host bind 0 %s' % fs_img,
                'setenv filesize',
                'mv host 0:0 %s %s' % (src, dst),
            ])
            assert('' == ''.join(output))

            output = ubman.run_command_list([
                'ls host 0:0 %s/dir1' % (d),
            ])
            assert('nested' in ''.join(output))

            output = ubman.run_command_list([
                'ls host 0:0 %s' % (src),
            ])
            assert('inner' in ''.join(output))
            assert('nested' not in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)

    def test_rename10(self, ubman, fs_obj_rename):
        """
        Test Case 10 - rename a file to itself (successful mv)
        """
        fs_type, fs_img, md5val = fs_obj_rename
        with ubman.log.section('Test Case 10 - rename a file to itself'):
            d = 'test10'
            src = '%s/file1' % d
            output = ubman.run_command_list([
                'host bind 0 %s' % fs_img,
                'setenv filesize',
                'mv host 0:0 %s %s' % (src, src),
            ])
            assert('' == ''.join(output))

            output = ubman.run_command_list([
                'load host 0:0 %x /%s' % (ADDR, src),
                'printenv filesize'])
            assert('filesize=400' in output)

            output = ubman.run_command_list([
                'ls host 0:0 %s' % (d),
            ])
            assert('file1' in ''.join(output))

            output = ubman.run_command_list([
                'md5sum %x $filesize' % ADDR,
                'setenv filesize'])
            assert(md5val['test10'] in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)

    def test_rename11(self, ubman, fs_obj_rename):
        """
        Test Case 11 - rename a directory to itself (successful mv)
        """
        fs_type, fs_img, md5val = fs_obj_rename
        with ubman.log.section('Test Case 11 - rename a directory to itself'):
            # / at the end here is intentional. Ensures trailing / doesn't
            # affect mv producing an updated dst path for fs_rename
            d = 'test11/'
            src = '%sdir1' % d
            output = ubman.run_command_list([
                'host bind 0 %s' % fs_img,
                'setenv filesize',
                'mv host 0:0 %s %s' % (src, d),
            ])
            assert('' == ''.join(output))

            output = ubman.run_command_list([
                'load host 0:0 %x /%s/file1' % (ADDR, src),
                'printenv filesize'])
            assert('filesize=400' in output)

            output = ubman.run_command_list([
                'ls host 0:0 %s' % (d),
            ])
            assert('dir1' in ''.join(output))

            output = ubman.run_command_list([
                'md5sum %x $filesize' % ADDR,
                'setenv filesize'])
            assert(md5val['test11'] in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)

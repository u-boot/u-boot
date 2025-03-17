# SPDX-License-Identifier:      GPL-2.0+
# Copyright (c) 2018, Linaro Limited
# Author: Takahiro Akashi <takahiro.akashi@linaro.org>
#
# U-Boot File System:Exntented Test

"""
This test verifies extended write operation on file system.
"""

import os.path
import pytest
import re
from subprocess import check_output
from fstest_defs import *
from fstest_helpers import assert_fs_integrity

PLAIN_FILE='abcdefgh.txt'
MANGLE_FILE='abcdefghi.txt'

def str2fat(long_filename):
    splitext = os.path.splitext(long_filename.upper())
    name = splitext[0]
    ext = splitext[1][1:]
    if len(name) > 8:
        name = '%s~1' % name[:6]
    return '%-8s %s' % (name, ext)

@pytest.mark.boardspec('sandbox')
@pytest.mark.slow
class TestFsExt(object):
    def test_fs_ext1(self, ubman, fs_obj_ext):
        """
        Test Case 1 - write a file with absolute path
        """
        fs_type,fs_cmd_prefix,fs_cmd_write,fs_img,md5val = fs_obj_ext
        with ubman.log.section('Test Case 1 - write with abs path'):
            # Test Case 1a - Check if command successfully returned
            output = ubman.run_command_list([
                'host bind 0 %s' % fs_img,
                '%sload host 0:0 %x /%s' % (fs_cmd_prefix, ADDR, MIN_FILE),
                '%s%s host 0:0 %x /dir1/%s.w1 $filesize'
                    % (fs_cmd_prefix, fs_cmd_write, ADDR, MIN_FILE)])
            assert('20480 bytes written' in ''.join(output))

            # Test Case 1b - Check md5 of file content
            output = ubman.run_command_list([
                'mw.b %x 00 100' % ADDR,
                '%sload host 0:0 %x /dir1/%s.w1' % (fs_cmd_prefix, ADDR, MIN_FILE),
                'md5sum %x $filesize' % ADDR,
                'setenv filesize'])
            assert(md5val[0] in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)

    def test_fs_ext2(self, ubman, fs_obj_ext):
        """
        Test Case 2 - write to a file with relative path
        """
        fs_type,fs_cmd_prefix,fs_cmd_write,fs_img,md5val = fs_obj_ext
        with ubman.log.section('Test Case 2 - write with rel path'):
            # Test Case 2a - Check if command successfully returned
            output = ubman.run_command_list([
                'host bind 0 %s' % fs_img,
                '%sload host 0:0 %x /%s' % (fs_cmd_prefix, ADDR, MIN_FILE),
                '%s%s host 0:0 %x dir1/%s.w2 $filesize'
                    % (fs_cmd_prefix, fs_cmd_write, ADDR, MIN_FILE)])
            assert('20480 bytes written' in ''.join(output))

            # Test Case 2b - Check md5 of file content
            output = ubman.run_command_list([
                'mw.b %x 00 100' % ADDR,
                '%sload host 0:0 %x dir1/%s.w2' % (fs_cmd_prefix, ADDR, MIN_FILE),
                'md5sum %x $filesize' % ADDR,
                'setenv filesize'])
            assert(md5val[0] in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)

    def test_fs_ext3(self, ubman, fs_obj_ext):
        """
        Test Case 3 - write to a file with invalid path
        """
        fs_type,fs_cmd_prefix,fs_cmd_write,fs_img,md5val = fs_obj_ext
        with ubman.log.section('Test Case 3 - write with invalid path'):
            # Test Case 3 - Check if command expectedly failed
            output = ubman.run_command_list([
                'host bind 0 %s' % fs_img,
                '%sload host 0:0 %x /%s' % (fs_cmd_prefix, ADDR, MIN_FILE),
                '%s%s host 0:0 %x /dir1/none/%s.w3 $filesize'
                    % (fs_cmd_prefix, fs_cmd_write, ADDR, MIN_FILE)])
            assert('Unable to write file /dir1/none/' in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)

    def test_fs_ext4(self, ubman, fs_obj_ext):
        """
        Test Case 4 - write at non-zero offset, enlarging file size
        """
        fs_type,fs_cmd_prefix,fs_cmd_write,fs_img,md5val = fs_obj_ext
        with ubman.log.section('Test Case 4 - write at non-zero offset, enlarging file size'):
            # Test Case 4a - Check if command successfully returned
            output = ubman.run_command_list([
                'host bind 0 %s' % fs_img,
                '%sload host 0:0 %x /%s' % (fs_cmd_prefix, ADDR, MIN_FILE),
                '%s%s host 0:0 %x /dir1/%s.w4 $filesize'
                    % (fs_cmd_prefix, fs_cmd_write, ADDR, MIN_FILE)])
            output = ubman.run_command(
                '%s%s host 0:0 %x /dir1/%s.w4 $filesize 0x1400'
                    % (fs_cmd_prefix, fs_cmd_write, ADDR, MIN_FILE))
            assert('20480 bytes written' in output)

            # Test Case 4b - Check size of written file
            output = ubman.run_command_list([
                '%ssize host 0:0 /dir1/%s.w4' % (fs_cmd_prefix, MIN_FILE),
                'printenv filesize',
                'setenv filesize'])
            assert('filesize=6400' in ''.join(output))

            # Test Case 4c - Check md5 of file content
            output = ubman.run_command_list([
                'mw.b %x 00 100' % ADDR,
                '%sload host 0:0 %x /dir1/%s.w4' % (fs_cmd_prefix, ADDR, MIN_FILE),
                'md5sum %x $filesize' % ADDR,
                'setenv filesize'])
            assert(md5val[1] in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)

    def test_fs_ext5(self, ubman, fs_obj_ext):
        """
        Test Case 5 - write at non-zero offset, shrinking file size
        """
        fs_type,fs_cmd_prefix,fs_cmd_write,fs_img,md5val = fs_obj_ext
        with ubman.log.section('Test Case 5 - write at non-zero offset, shrinking file size'):
            # Test Case 5a - Check if command successfully returned
            output = ubman.run_command_list([
                'host bind 0 %s' % fs_img,
                '%sload host 0:0 %x /%s' % (fs_cmd_prefix, ADDR, MIN_FILE),
                '%s%s host 0:0 %x /dir1/%s.w5 $filesize'
                    % (fs_cmd_prefix, fs_cmd_write, ADDR, MIN_FILE)])
            output = ubman.run_command(
                '%s%s host 0:0 %x /dir1/%s.w5 0x1400 0x1400'
                    % (fs_cmd_prefix, fs_cmd_write, ADDR, MIN_FILE))
            assert('5120 bytes written' in output)

            # Test Case 5b - Check size of written file
            output = ubman.run_command_list([
                '%ssize host 0:0 /dir1/%s.w5' % (fs_cmd_prefix, MIN_FILE),
                'printenv filesize',
                'setenv filesize'])
            assert('filesize=2800' in ''.join(output))

            # Test Case 5c - Check md5 of file content
            output = ubman.run_command_list([
                'mw.b %x 00 100' % ADDR,
                '%sload host 0:0 %x /dir1/%s.w5' % (fs_cmd_prefix, ADDR, MIN_FILE),
                'md5sum %x $filesize' % ADDR,
                'setenv filesize'])
            assert(md5val[2] in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)

    def test_fs_ext6(self, ubman, fs_obj_ext):
        """
        Test Case 6 - write nothing at the start, truncating to zero
        """
        fs_type,fs_cmd_prefix,fs_cmd_write,fs_img,md5val = fs_obj_ext
        with ubman.log.section('Test Case 6 - write nothing at the start, truncating to zero'):
            # Test Case 6a - Check if command successfully returned
            output = ubman.run_command_list([
                'host bind 0 %s' % fs_img,
                '%sload host 0:0 %x /%s' % (fs_cmd_prefix, ADDR, MIN_FILE),
                '%s%s host 0:0 %x /dir1/%s.w6 $filesize'
                    % (fs_cmd_prefix, fs_cmd_write, ADDR, MIN_FILE)])
            output = ubman.run_command(
                '%s%s host 0:0 %x /dir1/%s.w6 0 0'
                    % (fs_cmd_prefix, fs_cmd_write, ADDR, MIN_FILE))
            assert('0 bytes written' in output)

            # Test Case 6b - Check size of written file
            output = ubman.run_command_list([
                '%ssize host 0:0 /dir1/%s.w6' % (fs_cmd_prefix, MIN_FILE),
                'printenv filesize',
                'setenv filesize'])
            assert('filesize=0' in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)

    def test_fs_ext7(self, ubman, fs_obj_ext):
        """
        Test Case 7 - write at the end (append)
        """
        fs_type,fs_cmd_prefix,fs_cmd_write,fs_img,md5val = fs_obj_ext
        with ubman.log.section('Test Case 7 - write at the end (append)'):
            # Test Case 7a - Check if command successfully returned
            output = ubman.run_command_list([
                'host bind 0 %s' % fs_img,
                '%sload host 0:0 %x /%s' % (fs_cmd_prefix, ADDR, MIN_FILE),
                '%s%s host 0:0 %x /dir1/%s.w7 $filesize'
                    % (fs_cmd_prefix, fs_cmd_write, ADDR, MIN_FILE)])
            output = ubman.run_command(
                '%s%s host 0:0 %x /dir1/%s.w7 $filesize $filesize'
                    % (fs_cmd_prefix, fs_cmd_write, ADDR, MIN_FILE))
            assert('20480 bytes written' in output)

            # Test Case 7b - Check size of written file
            output = ubman.run_command_list([
                '%ssize host 0:0 /dir1/%s.w7' % (fs_cmd_prefix, MIN_FILE),
                'printenv filesize',
                'setenv filesize'])
            assert('filesize=a000' in ''.join(output))

            # Test Case 7c - Check md5 of file content
            output = ubman.run_command_list([
                'mw.b %x 00 100' % ADDR,
                '%sload host 0:0 %x /dir1/%s.w7' % (fs_cmd_prefix, ADDR, MIN_FILE),
                'md5sum %x $filesize' % ADDR,
                'setenv filesize'])
            assert(md5val[3] in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)

    def test_fs_ext8(self, ubman, fs_obj_ext):
        """
        Test Case 8 - write at offset beyond the end of file
        """
        fs_type,fs_cmd_prefix,fs_cmd_write,fs_img,md5val = fs_obj_ext
        with ubman.log.section('Test Case 8 - write beyond the end'):
            # Test Case 8a - Check if command expectedly failed
            output = ubman.run_command_list([
                'host bind 0 %s' % fs_img,
                '%sload host 0:0 %x /%s' % (fs_cmd_prefix, ADDR, MIN_FILE),
                '%s%s host 0:0 %x /dir1/%s.w8 $filesize'
                    % (fs_cmd_prefix, fs_cmd_write, ADDR, MIN_FILE)])
            output = ubman.run_command(
                '%s%s host 0:0 %x /dir1/%s.w8 0x1400 %x'
                    % (fs_cmd_prefix, fs_cmd_write, ADDR, MIN_FILE, 0x100000 + 0x1400))
            assert('Unable to write file /dir1' in output)
            assert_fs_integrity(fs_type, fs_img)

    def test_fs_ext9(self, ubman, fs_obj_ext):
        """
        Test Case 9 - write to a non-existing file at non-zero offset
        """
        fs_type,fs_cmd_prefix,fs_cmd_write,fs_img,md5val = fs_obj_ext
        with ubman.log.section('Test Case 9 - write to non-existing file with non-zero offset'):
            # Test Case 9a - Check if command expectedly failed
            output = ubman.run_command_list([
                'host bind 0 %s' % fs_img,
                '%sload host 0:0 %x /%s' % (fs_cmd_prefix, ADDR, MIN_FILE),
                '%s%s host 0:0 %x /dir1/%s.w9 0x1400 0x1400'
                    % (fs_cmd_prefix, fs_cmd_write, ADDR, MIN_FILE)])
            assert('Unable to write file /dir1' in ''.join(output))
            assert_fs_integrity(fs_type, fs_img)

    def test_fs_ext10(self, ubman, fs_obj_ext):
        """
        'Test Case 10 - create/delete as many directories under root directory
        as amount of directory entries goes beyond one cluster size)'
        """
        fs_type,fs_cmd_prefix,fs_cmd_write,fs_img,md5val = fs_obj_ext
        with ubman.log.section('Test Case 10 - create/delete (many)'):
            # Test Case 10a - Create many files
            #   Please note that the size of directory entry is 32 bytes.
            #   So one typical cluster may holds 64 (2048/32) entries.
            output = ubman.run_command(
                'host bind 0 %s' % fs_img)

            for i in range(0, 66):
                output = ubman.run_command(
                    '%s%s host 0:0 %x /FILE0123456789_%02x 100'
                    % (fs_cmd_prefix, fs_cmd_write, ADDR, i))
            output = ubman.run_command('%sls host 0:0 /' % fs_cmd_prefix)
            assert('FILE0123456789_00' in output)
            assert('FILE0123456789_41' in output)

            # Test Case 10b - Delete many files
            for i in range(0, 66):
                output = ubman.run_command(
                    '%srm host 0:0 /FILE0123456789_%02x'
                    % (fs_cmd_prefix, i))
            output = ubman.run_command('%sls host 0:0 /' % fs_cmd_prefix)
            assert(not 'FILE0123456789_00' in output)
            assert(not 'FILE0123456789_41' in output)

            # Test Case 10c - Create many files again
            # Please note no.64 and 65 are intentionally re-created
            for i in range(64, 128):
                output = ubman.run_command(
                    '%s%s host 0:0 %x /FILE0123456789_%02x 100'
                    % (fs_cmd_prefix, fs_cmd_write, ADDR, i))
            output = ubman.run_command('%sls host 0:0 /' % fs_cmd_prefix)
            assert('FILE0123456789_40' in output)
            assert('FILE0123456789_79' in output)

            assert_fs_integrity(fs_type, fs_img)

    def test_fs_ext11(self, ubman, fs_obj_ext):
        """
        'Test Case 11 - create/delete as many directories under non-root
        directory as amount of directory entries goes beyond one cluster size)'
        """
        fs_type,fs_cmd_prefix,fs_cmd_write,fs_img,md5val = fs_obj_ext
        with ubman.log.section('Test Case 11 - create/delete (many)'):
            # Test Case 11a - Create many files
            #   Please note that the size of directory entry is 32 bytes.
            #   So one typical cluster may holds 64 (2048/32) entries.
            output = ubman.run_command(
                'host bind 0 %s' % fs_img)

            for i in range(0, 66):
                output = ubman.run_command(
                    '%s%s host 0:0 %x /dir1/FILE0123456789_%02x 100'
                    % (fs_cmd_prefix, fs_cmd_write, ADDR, i))
            output = ubman.run_command('%sls host 0:0 /dir1' % fs_cmd_prefix)
            assert('FILE0123456789_00' in output)
            assert('FILE0123456789_41' in output)

            # Test Case 11b - Delete many files
            for i in range(0, 66):
                output = ubman.run_command(
                    '%srm host 0:0 /dir1/FILE0123456789_%02x'
                    % (fs_cmd_prefix, i))
            output = ubman.run_command('%sls host 0:0 /dir1' % fs_cmd_prefix)
            assert(not 'FILE0123456789_00' in output)
            assert(not 'FILE0123456789_41' in output)

            # Test Case 11c - Create many files again
            # Please note no.64 and 65 are intentionally re-created
            for i in range(64, 128):
                output = ubman.run_command(
                    '%s%s host 0:0 %x /dir1/FILE0123456789_%02x 100'
                    % (fs_cmd_prefix, fs_cmd_write, ADDR, i))
            output = ubman.run_command('%sls host 0:0 /dir1' % fs_cmd_prefix)
            assert('FILE0123456789_40' in output)
            assert('FILE0123456789_79' in output)

            assert_fs_integrity(fs_type, fs_img)

    def test_fs_ext12(self, ubman, fs_obj_ext):
        """
        Test Case 12 - write plain and mangle file
        """
        fs_type,fs_cmd_prefix,fs_cmd_write,fs_img,md5val = fs_obj_ext
        with ubman.log.section('Test Case 12 - write plain and mangle file'):
            # Test Case 12a - Check if command successfully returned
            output = ubman.run_command_list([
                'host bind 0 %s' % fs_img,
                '%s%s host 0:0 %x /%s 0'
                    % (fs_cmd_prefix, fs_cmd_write, ADDR, PLAIN_FILE),
                '%s%s host 0:0 %x /%s 0'
                    % (fs_cmd_prefix, fs_cmd_write, ADDR, MANGLE_FILE)])
            assert('0 bytes written' in ''.join(output))
            if fs_type == 'exfat':
                # Test Case 12b - Read file system content
                output = check_output('fattools ls %s' % fs_img, shell=True).decode()
                # Test Case 12c - Check if short filename is not mangled
                assert(PLAIN_FILE in ''.join(output))
                # Test Case 12d - Check if long filename is mangled
                assert(MANGLE_FILE in ''.join(output))
            else:
                # Test Case 12b - Read file system content
                output = check_output('mdir -i %s' % fs_img, shell=True).decode()
                # Test Case 12c - Check if short filename is not mangled
                assert(str2fat(PLAIN_FILE) in ''.join(output))
                # Test Case 12d - Check if long filename is mangled
                assert(str2fat(MANGLE_FILE) in ''.join(output))

            assert_fs_integrity(fs_type, fs_img)

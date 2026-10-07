# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2026, Kuan-Wei Chiu <visitorckw@gmail.com>

import os
import pytest
import utils
import zlib

@pytest.mark.buildconfigspec("fs_9p")
@pytest.mark.buildconfigspec("cmd_virtio")
def test_fs_9p(ubman):
    """Test the 9P filesystem commands."""

    test_file_name = "9p_test_small.txt"
    test_file_content = b"9PFS_AUTOMATED_TEST_CONTENT_1234567890\n" * 10
    test_file_size = len(test_file_content)
    test_file_crc = zlib.crc32(test_file_content) & 0xffffffff
    test_file_path = os.path.join(ubman.config.build_dir, test_file_name)

    with open(test_file_path, "wb") as f:
        f.write(test_file_content)

    large_file_name = "9p_test_large.bin"
    large_file_content = b"".join(bytes([i % 256]) for i in range(128 * 1024))
    large_file_size = len(large_file_content)
    large_file_crc = zlib.crc32(large_file_content) & 0xffffffff
    large_file_path = os.path.join(ubman.config.build_dir, large_file_name)

    with open(large_file_path, "wb") as f:
        f.write(large_file_content)

    sub_dir_name = "9p_subdir"
    sub_dir_path = os.path.join(ubman.config.build_dir, sub_dir_name)
    os.makedirs(sub_dir_path, exist_ok=True)
    nested_file_name = "nested.txt"
    nested_file_content = b"NESTED_FILE_DATA_OK\n"
    nested_file_path = os.path.join(sub_dir_path, nested_file_name)

    with open(nested_file_path, "wb") as f:
        f.write(nested_file_content)

    ubman.run_command("virtio scan")
    addr = utils.find_ram_base(ubman)

    output = ubman.run_command("ls 9p - /")
    assert test_file_name in output
    assert sub_dir_name in output

    output = ubman.run_command("ls 9p rootfs /")
    assert test_file_name in output
    assert large_file_name in output

    output = ubman.run_command("ls 9p 0 /")
    assert test_file_name in output

    output = ubman.run_command(f"ls 9p rootfs /{sub_dir_name}")
    assert nested_file_name in output

    output = ubman.run_command(f"size 9p rootfs /{test_file_name}")
    output = ubman.run_command("printenv filesize")
    assert f"filesize={test_file_size:x}" in output

    output = ubman.run_command(f"load 9p rootfs {addr:x} /{test_file_name}")
    assert f"{test_file_size} bytes read" in output

    output = ubman.run_command(f"crc32 {addr:x} $filesize")
    assert f"{test_file_crc:08x}" in output

    output = ubman.run_command(f"load 9p rootfs {addr:x} /{large_file_name}")
    assert f"{large_file_size} bytes read" in output

    output = ubman.run_command(f"crc32 {addr:x} $filesize")
    assert f"{large_file_crc:08x}" in output

    output = ubman.run_command(f"load 9p rootfs {addr:x} /{sub_dir_name}/{nested_file_name}")
    assert f"{len(nested_file_content)} bytes read" in output

    output = ubman.run_command(f"load 9p rootfs {addr:x} /non_existent_file_9p.txt")
    assert "Failed to load" in output

    output = ubman.run_command("ls 9p invalid_tag /")
    assert "No matching 9P transport device found" in output

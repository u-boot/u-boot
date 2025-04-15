# SPDX-License-Identifier: GPL-2.0
# Copyright (C) 2020
# Niel Fourie, DENX Software Engineering, lusus@denx.de

import pytest

@pytest.mark.buildconfigspec('cmd_part')
@pytest.mark.buildconfigspec('partitions')
@pytest.mark.buildconfigspec('efi_partition')
def test_part_types(ubman):
    """Test that `part types` prints a result which includes `EFI`."""
    output = ubman.run_command('part types')
    assert "Supported partition tables:" in output
    assert "EFI" in output

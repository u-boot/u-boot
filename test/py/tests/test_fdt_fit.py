# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2026 BayLibre SAS

"""Test the 'fdt fit' command

Build a FIT holding a base devicetree and two overlays, with configurations
which combine them, then check the devicetree produced for each
configuration and that images failing to load or verify are rejected.
"""

import gzip
import os
import shutil

import pytest
import fit_util
import utils

FIT_ADDR = 0x1000000
DEST_ADDR = 0x2000000

BASE_DTS = '''
/dts-v1/;

/ {
	model = "fdt fit test";

	test-node {
		base-prop = "base-value";
		order = "base";
	};
};
'''

OVERLAY_DTS = '''
/dts-v1/;
/plugin/;

&{/test-node} {
	%(name)s-prop = "%(name)s-value";
	order = "%(name)s";
};
'''

FDT_IMAGE = '''
		%(name)s {
			data = /incbin/("%(fname)s");
			type = "flat_dt";
			arch = "sandbox";
			compression = "%(compression)s";
			hash-1 {
				algo = "sha256";
			};
		};
'''

CONFIG = '''
		%(name)s {
			fdt = %(fdt)s;
			%(sig)s
		};
'''

SIGNATURE = '''signature {
				algo = "sha256,rsa2048";
				key-name-hint = "dev";
			};'''

def make_test_fit(ubman, basename, sig, compress_base=False):
    """Create a FIT with a base devicetree, two overlays and configurations

    Args:
        ubman (ConsoleBase): U-Boot console
        basename (str): Base name of the FIT to create
        sig (str): Signature node to add to each configuration, or ''
        compress_base (bool): Compress the base devicetree with gzip

    Returns:
        str: Filename of the FIT
    """
    images = {'base': fit_util.make_dtb(ubman, BASE_DTS, 'fdt-fit-base')}
    if compress_base:
        compressed = images['base'] + '.gz'
        with open(images['base'], 'rb') as inf, gzip.open(compressed, 'wb') as outf:
            shutil.copyfileobj(inf, outf)
        images['base'] = compressed
    for name in ('ov-a', 'ov-b'):
        images[name] = fit_util.make_dtb(ubman, OVERLAY_DTS % {'name': name},
                                         f'fdt-fit-{name}')

    configs = {
        'conf-base': '"base"',
        'conf-a': '"base", "ov-a"',
        'conf-ab': '"base", "ov-a", "ov-b"',
        'conf-b': '"ov-b"',
    }

    its = '/dts-v1/;\n\n/ {\n\tdescription = "fdt fit test";\n'
    its += '\t#address-cells = <1>;\n\n\timages {'
    for name, fname in images.items():
        compression = 'gzip' if name == 'base' and compress_base else 'none'
        its += FDT_IMAGE % {'name': name, 'fname': fname,
                           'compression': compression}
    its += '\t};\n\n\tconfigurations {\n\t\tdefault = "conf-base";\n'
    for name, fdt in configs.items():
        its += CONFIG % {'name': name, 'fdt': fdt, 'sig': sig}
    its += '\t};\n};\n'

    its_fname = fit_util.make_its(ubman, its, {}, f'{basename}.its')
    fit = fit_util.make_fname(ubman, f'{basename}.fit')
    mkimage = os.path.join(ubman.config.build_dir, 'tools/mkimage')
    utils.run_and_log(ubman, [mkimage, '-f', its_fname, fit])

    return fit

def corrupt_overlay(ubman, fit, basename):
    """Copy a FIT, changing the data of the ov-b image but not its hash

    Args:
        ubman (ConsoleBase): U-Boot console
        fit (str): Filename of the FIT to copy
        basename (str): Base name of the copy

    Returns:
        str: Filename of the copy
    """
    with open(fit, 'rb') as inf:
        data = inf.read()
    assert data.count(b'ov-b-value') == 1
    out = fit_util.make_fname(ubman, basename)
    with open(out, 'wb') as outf:
        outf.write(data.replace(b'ov-b-value', b'ov-b-evil!'))

    return out

def load_fit(ubman, fit):
    """Load a file into memory at FIT_ADDR and clear the destination"""
    output = ubman.run_command(f'host load hostfs - {FIT_ADDR:x} {fit}')
    assert 'bytes read' in output
    ubman.run_command(f'mw.l {DEST_ADDR:x} 0 400')

def fdt_fit(ubman, args):
    """Run 'fdt fit' and report whether it succeeded

    Args:
        ubman (ConsoleBase): U-Boot console
        args (str): Arguments for the command

    Returns:
        tuple:
            bool: True if the command succeeded
            str: Command output
    """
    output = ubman.run_command(f'fdt fit {args}; echo rc=$?')
    return output.rstrip().endswith('rc=0'), output

def get_test_node(ubman):
    """Return the test node of the devicetree at DEST_ADDR"""
    ubman.run_command(f'fdt addr {DEST_ADDR:x}')
    return ubman.run_command('fdt print /test-node')

def check_dest_untouched(ubman, addr=DEST_ADDR):
    """Check that nothing was written to a cleared destination"""
    output = ubman.run_command(f'md.l {addr:x} 4')
    assert '00000000 00000000 00000000 00000000' in output

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_fdt_fit')
@pytest.mark.requiredtool('dtc')
@pytest.mark.requiredtool('fdtput')
def test_fdt_fit(ubman):
    """Test loading the devicetree of a FIT configuration"""
    fit = make_test_fit(ubman, 'fdt-fit', '')

    with ubman.log.section('Default configuration'):
        load_fit(ubman, fit)
        ok, output = fdt_fit(ubman, f'{FIT_ADDR:x} {DEST_ADDR:x}')
        assert ok, output
        assert "Using 'conf-base' configuration" in output
        node = get_test_node(ubman)
        assert 'base-prop = "base-value"' in node
        assert 'ov-a-prop' not in node

    with ubman.log.section('Overlays applied in order'):
        load_fit(ubman, fit)
        ok, output = fdt_fit(ubman, f'{FIT_ADDR:x}#conf-ab {DEST_ADDR:x}')
        assert ok, output
        node = get_test_node(ubman)
        assert 'base-prop = "base-value"' in node
        assert 'ov-a-prop = "ov-a-value"' in node
        assert 'ov-b-prop = "ov-b-value"' in node
        assert 'order = "ov-b"' in node

    with ubman.log.section('Chained configuration'):
        load_fit(ubman, fit)
        ok, output = fdt_fit(ubman,
                             f'{FIT_ADDR:x}#conf-a#conf-b {DEST_ADDR:x}')
        assert not ok
        assert 'Chained configurations are not supported' in output
        check_dest_untouched(ubman)

    with ubman.log.section('Length'):
        load_fit(ubman, fit)
        ok, output = fdt_fit(ubman, f'{FIT_ADDR:x}#conf-a {DEST_ADDR:x} 4000')
        assert ok, output
        ubman.run_command(f'fdt addr {DEST_ADDR:x}')
        output = ubman.run_command('fdt header get size totalsize; echo $size')
        assert output.splitlines()[-1] == '4000'

        load_fit(ubman, fit)
        ok, output = fdt_fit(ubman, f'{FIT_ADDR:x}#conf-a {DEST_ADDR:x} 10')
        assert not ok
        assert 'aborting' in output
        check_dest_untouched(ubman)

    with ubman.log.section('Missing configuration'):
        load_fit(ubman, fit)
        ok, output = fdt_fit(ubman, f'{FIT_ADDR:x}#conf-none {DEST_ADDR:x}')
        assert not ok
        assert "Could not find configuration node 'conf-none'" in output
        check_dest_untouched(ubman)

    with ubman.log.section('Corrupted overlay'):
        bad_fit = corrupt_overlay(ubman, fit, 'fdt-fit-bad-overlay.fit')
        load_fit(ubman, bad_fit)
        ok, output = fdt_fit(ubman, f'{FIT_ADDR:x}#conf-ab {DEST_ADDR:x}')
        assert not ok
        assert 'Bad hash value' in output
        assert 'load of ov-b failed' in output
        check_dest_untouched(ubman)

    with ubman.log.section('Missing overlay'):
        missing_fit = fit_util.make_fname(ubman, 'fdt-fit-missing.fit')
        shutil.copyfile(fit, missing_fit)
        utils.run_and_log(ubman, ['fdtput', '-t', 's', missing_fit,
                                  '/configurations/conf-a', 'fdt', 'base',
                                  'ov-missing'])
        load_fit(ubman, missing_fit)
        ok, output = fdt_fit(ubman, f'{FIT_ADDR:x}#conf-a {DEST_ADDR:x}')
        assert not ok
        assert 'load of ov-missing failed' in output
        check_dest_untouched(ubman)

    with ubman.log.section('Not a FIT'):
        load_fit(ubman, fit_util.make_fname(ubman, 'fdt-fit-base.dtb'))
        ok, output = fdt_fit(ubman, f'{FIT_ADDR:x} {DEST_ADDR:x}')
        assert not ok
        check_dest_untouched(ubman)

    with ubman.log.section('Usage'):
        ok, output = fdt_fit(ubman, f'{FIT_ADDR:x}')
        assert not ok
        assert 'Usage:' in output

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_fdt_fit')
@pytest.mark.requiredtool('dtc')
@pytest.mark.requiredtool('fdtput')
@pytest.mark.parametrize('compress_base', [False, True])
def test_fdt_fit_load_addr(ubman, compress_base):
    """Ignore FIT load addresses, including before an overlay failure"""
    if compress_base and not ubman.config.buildconfig.get('config_gzip'):
        pytest.skip('gzip support not enabled')

    fit = make_test_fit(ubman, 'fdt-fit-load', '', compress_base)
    utils.run_and_log(ubman, ['fdtput', '-t', 'x', fit, '/images/base',
                              'load', f'{DEST_ADDR:x}'])

    with ubman.log.section('Load address equals destination, bad overlay'):
        bad_fit = corrupt_overlay(ubman, fit, 'fdt-fit-load-bad.fit')
        load_fit(ubman, bad_fit)
        ok, output = fdt_fit(ubman, f'{FIT_ADDR:x}#conf-ab {DEST_ADDR:x}')
        assert not ok
        assert 'load of ov-b failed' in output
        check_dest_untouched(ubman)

    with ubman.log.section('Load address equals destination, short length'):
        load_fit(ubman, fit)
        ok, output = fdt_fit(ubman, f'{FIT_ADDR:x}#conf-base {DEST_ADDR:x} 10')
        assert not ok
        assert 'aborting' in output
        check_dest_untouched(ubman)

    with ubman.log.section('Only the requested destination is written'):
        load_addr = 0x3000000
        utils.run_and_log(ubman, ['fdtput', '-t', 'x', fit, '/images/base',
                                  'load', f'{load_addr:x}'])
        for conf in ('conf-base', 'conf-ab'):
            load_fit(ubman, fit)
            ubman.run_command(f'mw.l {load_addr:x} 0 400')
            ok, output = fdt_fit(ubman, f'{FIT_ADDR:x}#{conf} {DEST_ADDR:x}')
            assert ok, output
            node = get_test_node(ubman)
            assert 'base-prop = "base-value"' in node
            order = 'base' if conf == 'conf-base' else 'ov-b'
            assert f'order = "{order}"' in node
            check_dest_untouched(ubman, load_addr)

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_fdt_fit')
@pytest.mark.buildconfigspec('fit_signature')
@pytest.mark.requiredtool('dtc')
@pytest.mark.requiredtool('openssl')
def test_fdt_fit_signed(ubman):
    """Test 'fdt fit' with a required configuration signature

    This restarts U-Boot with a control devicetree holding the public key,
    so it must run on sandbox.
    """
    keydir = fit_util.make_fname(ubman, 'fdt-fit-keys')
    os.makedirs(keydir, exist_ok=True)
    key = os.path.join(keydir, 'dev')
    utils.run_and_log(ubman, f'openssl genpkey -algorithm RSA -out {key}.key '
                      '-pkeyopt rsa_keygen_bits:2048 '
                      '-pkeyopt rsa_keygen_pubexp:65537')
    utils.run_and_log(ubman, f'openssl req -batch -new -x509 -key {key}.key '
                      f'-out {key}.crt')

    vboot_dts = os.path.join(ubman.config.source_dir,
                             'test/py/tests/vboot/sandbox-u-boot.dts')
    dtb = fit_util.make_fname(ubman, 'fdt-fit-u-boot.dtb')
    utils.run_and_log(ubman, ['dtc', vboot_dts, '-O', 'dtb', '-o', dtb])

    unsigned_fit = make_test_fit(ubman, 'fdt-fit-unsigned', '')
    fit = make_test_fit(ubman, 'fdt-fit-signed', SIGNATURE)
    mkimage = os.path.join(ubman.config.build_dir, 'tools/mkimage')
    utils.run_and_log(ubman, [mkimage, '-F', '-k', keydir, '-K', dtb, '-r',
                              fit])
    bad_fit = corrupt_overlay(ubman, fit, 'fdt-fit-signed-bad-overlay.fit')

    old_dtb = ubman.config.dtb
    try:
        ubman.config.dtb = dtb
        ubman.restart_uboot()
        ubman.run_command('setenv verify no')

        with ubman.log.section('Signed configuration'):
            load_fit(ubman, fit)
            ok, output = fdt_fit(ubman, f'{FIT_ADDR:x}#conf-ab {DEST_ADDR:x}')
            assert ok, output
            assert 'sha256,rsa2048:dev+' in output
            node = get_test_node(ubman)
            assert 'order = "ov-b"' in node

        with ubman.log.section('Chained signed configurations'):
            # Each is signed, but no configuration lists base plus only ov-b
            for first in ('conf-a', 'conf-base'):
                load_fit(ubman, fit)
                ok, output = fdt_fit(ubman,
                                     f'{FIT_ADDR:x}#{first}#conf-b {DEST_ADDR:x}')
                assert not ok
                assert 'Chained configurations are not supported' in output
                assert 'dev+' not in output
                check_dest_untouched(ubman)

        with ubman.log.section('Unsigned configuration'):
            load_fit(ubman, unsigned_fit)
            ok, output = fdt_fit(ubman, f'{FIT_ADDR:x}#conf-ab {DEST_ADDR:x}')
            assert not ok
            check_dest_untouched(ubman)

        with ubman.log.section('Signed configuration, corrupted overlay'):
            load_fit(ubman, bad_fit)
            ok, output = fdt_fit(ubman, f'{FIT_ADDR:x}#conf-ab {DEST_ADDR:x}')
            assert not ok
            assert 'sha256,rsa2048:dev+' in output
            assert 'load of ov-b failed' in output
            check_dest_untouched(ubman)
    finally:
        ubman.config.dtb = old_dtb
        ubman.restart_uboot()

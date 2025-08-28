// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 */

#include <command.h>
#include <cpu.h>
#include <cpu_func.h>
#include <dm.h>
#include <dm/lists.h>
#include <event.h>
#include <hang.h>
#include <init.h>
#include <irq_func.h>
#include <log.h>
#include <asm/encoding.h>
#include <asm/system.h>
#include <asm/hwcap.h>
#include <asm/cpufeature.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <dm/uclass-internal.h>
#include <linux/bitops.h>
#include <linux/log2.h>
#include <linux/ctype.h>

/*
 * The variables here must be stored in the data section since they are used
 * before the bss section is available.
 */
#if !CONFIG_IS_ENABLED(XIP)
u32 hart_lottery __section(".data") = 0;

#ifdef CONFIG_AVAILABLE_HARTS
/*
 * The main hart running U-Boot has acquired available_harts_lock until it has
 * finished initialization of global data.
 */
u32 available_harts_lock = 1;
#endif
#endif

/* Host ISA bitmap */
static DECLARE_BITMAP(riscv_isa, RISCV_ISA_EXT_MAX) __section(".data");

static unsigned int riscv_cbom_block_size __section(".data");
static unsigned int riscv_cboz_block_size __section(".data");
/**
 * __riscv_isa_extension_available() - Check whether given extension
 * is available or not
 *
 * @bit: bit position of the desired extension
 * Return: true or false
 *
 */
static bool __riscv_isa_extension_available(unsigned int bit)
{
	if (bit >= RISCV_ISA_EXT_MAX)
		return false;

	return test_bit(bit, riscv_isa) ? true : false;
}

inline unsigned int riscv_get_cbom_block_size(void)
{
	return riscv_cbom_block_size;
}

inline unsigned int riscv_get_cboz_block_size(void)
{
	return riscv_cboz_block_size;
}

static int riscv_ext_zicbom_validate(const struct riscv_isa_ext_data *data,
				     const unsigned long *isa_bitmap)
{
	struct udevice *dev;

	if (!CONFIG_IS_ENABLED(RISCV_ISA_ZICBOM) || riscv_cbom_block_size)
		return 0;

	uclass_first_device(UCLASS_CPU, &dev);
	if (!dev) {
		log_info("Failed to get cpu device!\n");
		return -ENXIO;
	}

	if (!dev_read_u32(dev, "riscv,cbom-block-size",
			  &riscv_cbom_block_size)) {
		if (!riscv_cbom_block_size) {
			log_err("Zicbom detected in ISA string, disabling as no cbom-block-size found\n");
			return -EINVAL;
		}
		if (!is_power_of_2(riscv_cbom_block_size)) {
			log_err("Zicbom disabled as cbom-block-size present, but is not a power-of-2\n");
			return -EINVAL;
		}
		return 0;
	} else {
		return -EINVAL;
	}
}

static int riscv_ext_zicboz_validate(const struct riscv_isa_ext_data *data,
				     const unsigned long *isa_bitmap)
{
	struct udevice *dev;

	if (!CONFIG_IS_ENABLED(RISCV_ISA_ZICBOM) || riscv_cboz_block_size)
		return 0;

	uclass_first_device(UCLASS_CPU, &dev);
	if (!dev) {
		log_debug("Failed to get cpu device!\n");
		return -ENXIO;
	}

	if (!dev_read_u32(dev, "riscv,cboz-block-size",
			  &riscv_cboz_block_size)) {
		if (!riscv_cboz_block_size) {
			log_err("Zicboz detected in ISA string, disabling as no cboz-block-size found\n");
			return -EINVAL;
		}
		if (!is_power_of_2(riscv_cboz_block_size)) {
			log_err("Zicboz disabled as cboz-block-size present, but is not a power-of-2\n");
			return -EINVAL;
		}
		return 0;
	} else {
		return -EINVAL;
	}
}

static int riscv_ext_zca_depends(const struct riscv_isa_ext_data *data,
				 const unsigned long *isa_bitmap)
{
	if (__riscv_isa_extension_available(RISCV_ISA_EXT_ZCA))
		return 0;

	return -EINVAL;
}

static int riscv_ext_zcd_validate(const struct riscv_isa_ext_data *data,
				  const unsigned long *isa_bitmap)
{
	if (__riscv_isa_extension_available(RISCV_ISA_EXT_ZCA) &&
	    __riscv_isa_extension_available(RISCV_ISA_EXT_d))
		return 0;

	return -EINVAL;
}

static int riscv_ext_zcf_validate(const struct riscv_isa_ext_data *data,
				  const unsigned long *isa_bitmap)
{
	if (IS_ENABLED(CONFIG_64BIT))
		return -EINVAL;

	if (__riscv_isa_extension_available(RISCV_ISA_EXT_ZCA) &&
	    __riscv_isa_extension_available(RISCV_ISA_EXT_f))
		return 0;

	return -EINVAL;
}

static const unsigned int riscv_zk_bundled_exts[] = {
	RISCV_ISA_EXT_ZBKB,
	RISCV_ISA_EXT_ZBKC,
	RISCV_ISA_EXT_ZBKX,
	RISCV_ISA_EXT_ZKND,
	RISCV_ISA_EXT_ZKNE,
	RISCV_ISA_EXT_ZKR,
	RISCV_ISA_EXT_ZKT,
};

static const unsigned int riscv_zkn_bundled_exts[] = {
	RISCV_ISA_EXT_ZBKB,
	RISCV_ISA_EXT_ZBKC,
	RISCV_ISA_EXT_ZBKX,
	RISCV_ISA_EXT_ZKND,
	RISCV_ISA_EXT_ZKNE,
	RISCV_ISA_EXT_ZKNH,
};

static const unsigned int riscv_zks_bundled_exts[] = {
	RISCV_ISA_EXT_ZBKB,
	RISCV_ISA_EXT_ZBKC,
	RISCV_ISA_EXT_ZKSED,
	RISCV_ISA_EXT_ZKSH
};

#define RISCV_ISA_EXT_ZVKN	\
	RISCV_ISA_EXT_ZVKNED,	\
	RISCV_ISA_EXT_ZVKNHB,	\
	RISCV_ISA_EXT_ZVKB,	\
	RISCV_ISA_EXT_ZVKT

static const unsigned int riscv_zvkn_bundled_exts[] = {
	RISCV_ISA_EXT_ZVKN
};

static const unsigned int riscv_zvknc_bundled_exts[] = {
	RISCV_ISA_EXT_ZVKN,
	RISCV_ISA_EXT_ZVBC
};

static const unsigned int riscv_zvkng_bundled_exts[] = {
	RISCV_ISA_EXT_ZVKN,
	RISCV_ISA_EXT_ZVKG
};

#define RISCV_ISA_EXT_ZVKS	\
	RISCV_ISA_EXT_ZVKSED,	\
	RISCV_ISA_EXT_ZVKSH,	\
	RISCV_ISA_EXT_ZVKB,	\
	RISCV_ISA_EXT_ZVKT

static const unsigned int riscv_zvks_bundled_exts[] = {
	RISCV_ISA_EXT_ZVKS
};

static const unsigned int riscv_zvksc_bundled_exts[] = {
	RISCV_ISA_EXT_ZVKS,
	RISCV_ISA_EXT_ZVBC
};

static const unsigned int riscv_zvksg_bundled_exts[] = {
	RISCV_ISA_EXT_ZVKS,
	RISCV_ISA_EXT_ZVKG
};

static const unsigned int riscv_zvbb_exts[] = {
	RISCV_ISA_EXT_ZVKB
};

#define RISCV_ISA_EXT_ZVE64F_IMPLY_LIST	\
	RISCV_ISA_EXT_ZVE64X,		\
	RISCV_ISA_EXT_ZVE32F,		\
	RISCV_ISA_EXT_ZVE32X

#define RISCV_ISA_EXT_ZVE64D_IMPLY_LIST	\
	RISCV_ISA_EXT_ZVE64F,		\
	RISCV_ISA_EXT_ZVE64F_IMPLY_LIST

#define RISCV_ISA_EXT_V_IMPLY_LIST	\
	RISCV_ISA_EXT_ZVE64D,		\
	RISCV_ISA_EXT_ZVE64D_IMPLY_LIST

static const unsigned int riscv_zve32f_exts[] = {
	RISCV_ISA_EXT_ZVE32X
};

static const unsigned int riscv_zve64f_exts[] = {
	RISCV_ISA_EXT_ZVE64F_IMPLY_LIST
};

static const unsigned int riscv_zve64d_exts[] = {
	RISCV_ISA_EXT_ZVE64D_IMPLY_LIST
};

static const unsigned int riscv_v_exts[] = {
	RISCV_ISA_EXT_V_IMPLY_LIST
};

static const unsigned int riscv_zve64x_exts[] = {
	RISCV_ISA_EXT_ZVE32X,
	RISCV_ISA_EXT_ZVE64X
};

/*
 * While the [ms]envcfg CSRs were not defined until version 1.12 of the RISC-V
 * privileged ISA, the existence of the CSRs is implied by any extension which
 * specifies [ms]envcfg bit(s). Hence, we define a custom ISA extension for the
 * existence of the CSR, and treat it as a subset of those other extensions.
 */
static const unsigned int riscv_xlinuxenvcfg_exts[] = {
	RISCV_ISA_EXT_XLINUXENVCFG
};

/*
 * Zc* spec states that:
 * - C always implies Zca
 * - C+F implies Zcf (RV32 only)
 * - C+D implies Zcd
 *
 * These extensions will be enabled and then validated depending on the
 * availability of F/D RV32.
 */
static const unsigned int riscv_c_exts[] = {
	RISCV_ISA_EXT_ZCA,
	RISCV_ISA_EXT_ZCF,
	RISCV_ISA_EXT_ZCD,
};

/*
 * The canonical order of ISA extension names in the ISA string is defined in
 * chapter 27 of the unprivileged specification.
 *
 * Ordinarily, for in-kernel data structures, this order is unimportant but
 * isa_ext_arr defines the order of the ISA string in /proc/cpuinfo.
 *
 * The specification uses vague wording, such as should, when it comes to
 * ordering, so for our purposes the following rules apply:
 *
 * 1. All multi-letter extensions must be separated from other extensions by an
 *    underscore.
 *
 * 2. Additional standard extensions (starting with 'Z') must be sorted after
 *    single-letter extensions and before any higher-privileged extensions.
 *
 * 3. The first letter following the 'Z' conventionally indicates the most
 *    closely related alphabetical extension category, IMAFDQLCBKJTPVH.
 *    If multiple 'Z' extensions are named, they must be ordered first by
 *    category, then alphabetically within a category.
 *
 * 3. Standard supervisor-level extensions (starting with 'S') must be listed
 *    after standard unprivileged extensions.  If multiple supervisor-level
 *    extensions are listed, they must be ordered alphabetically.
 *
 * 4. Standard machine-level extensions (starting with 'Zxm') must be listed
 *    after any lower-privileged, standard extensions.  If multiple
 *    machine-level extensions are listed, they must be ordered
 *    alphabetically.
 *
 * 5. Non-standard extensions (starting with 'X') must be listed after all
 *    standard extensions. If multiple non-standard extensions are listed, they
 *    must be ordered alphabetically.
 *
 * An example string following the order is:
 *    rv64imadc_zifoo_zigoo_zafoo_sbar_scar_zxmbaz_xqux_xrux
 *
 * New entries to this struct should follow the ordering rules described above.
 */
const struct riscv_isa_ext_data riscv_isa_ext[] = {
	__RISCV_ISA_EXT_DATA(i, RISCV_ISA_EXT_i),
	__RISCV_ISA_EXT_DATA(m, RISCV_ISA_EXT_m),
	__RISCV_ISA_EXT_DATA(a, RISCV_ISA_EXT_a),
	__RISCV_ISA_EXT_DATA(f, RISCV_ISA_EXT_f),
	__RISCV_ISA_EXT_DATA(d, RISCV_ISA_EXT_d),
	__RISCV_ISA_EXT_DATA(q, RISCV_ISA_EXT_q),
	__RISCV_ISA_EXT_SUPERSET(c, RISCV_ISA_EXT_c, riscv_c_exts),
	__RISCV_ISA_EXT_SUPERSET(v, RISCV_ISA_EXT_v, riscv_v_exts),
	__RISCV_ISA_EXT_DATA(h, RISCV_ISA_EXT_h),
	__RISCV_ISA_EXT_SUPERSET_VALIDATE(zicbom, RISCV_ISA_EXT_ZICBOM, riscv_xlinuxenvcfg_exts,
					  riscv_ext_zicbom_validate),
	__RISCV_ISA_EXT_SUPERSET_VALIDATE(zicboz, RISCV_ISA_EXT_ZICBOZ, riscv_xlinuxenvcfg_exts,
					  riscv_ext_zicboz_validate),
	__RISCV_ISA_EXT_DATA(zicntr, RISCV_ISA_EXT_ZICNTR),
	__RISCV_ISA_EXT_DATA(zicond, RISCV_ISA_EXT_ZICOND),
	__RISCV_ISA_EXT_DATA(zicsr, RISCV_ISA_EXT_ZICSR),
	__RISCV_ISA_EXT_DATA(zifencei, RISCV_ISA_EXT_ZIFENCEI),
	__RISCV_ISA_EXT_DATA(zihintntl, RISCV_ISA_EXT_ZIHINTNTL),
	__RISCV_ISA_EXT_DATA(zihintpause, RISCV_ISA_EXT_ZIHINTPAUSE),
	__RISCV_ISA_EXT_DATA(zihpm, RISCV_ISA_EXT_ZIHPM),
	__RISCV_ISA_EXT_DATA(zimop, RISCV_ISA_EXT_ZIMOP),
	__RISCV_ISA_EXT_DATA(zacas, RISCV_ISA_EXT_ZACAS),
	__RISCV_ISA_EXT_DATA(zawrs, RISCV_ISA_EXT_ZAWRS),
	__RISCV_ISA_EXT_DATA(zfa, RISCV_ISA_EXT_ZFA),
	__RISCV_ISA_EXT_DATA(zfh, RISCV_ISA_EXT_ZFH),
	__RISCV_ISA_EXT_DATA(zfhmin, RISCV_ISA_EXT_ZFHMIN),
	__RISCV_ISA_EXT_DATA(zca, RISCV_ISA_EXT_ZCA),
	__RISCV_ISA_EXT_DATA_VALIDATE(zcb, RISCV_ISA_EXT_ZCB, riscv_ext_zca_depends),
	__RISCV_ISA_EXT_DATA_VALIDATE(zcd, RISCV_ISA_EXT_ZCD, riscv_ext_zcd_validate),
	__RISCV_ISA_EXT_DATA_VALIDATE(zcf, RISCV_ISA_EXT_ZCF, riscv_ext_zcf_validate),
	__RISCV_ISA_EXT_DATA_VALIDATE(zcmop, RISCV_ISA_EXT_ZCMOP, riscv_ext_zca_depends),
	__RISCV_ISA_EXT_DATA(zba, RISCV_ISA_EXT_ZBA),
	__RISCV_ISA_EXT_DATA(zbb, RISCV_ISA_EXT_ZBB),
	__RISCV_ISA_EXT_DATA(zbc, RISCV_ISA_EXT_ZBC),
	__RISCV_ISA_EXT_DATA(zbkb, RISCV_ISA_EXT_ZBKB),
	__RISCV_ISA_EXT_DATA(zbkc, RISCV_ISA_EXT_ZBKC),
	__RISCV_ISA_EXT_DATA(zbkx, RISCV_ISA_EXT_ZBKX),
	__RISCV_ISA_EXT_DATA(zbs, RISCV_ISA_EXT_ZBS),
	__RISCV_ISA_EXT_BUNDLE(zk, riscv_zk_bundled_exts),
	__RISCV_ISA_EXT_BUNDLE(zkn, riscv_zkn_bundled_exts),
	__RISCV_ISA_EXT_DATA(zknd, RISCV_ISA_EXT_ZKND),
	__RISCV_ISA_EXT_DATA(zkne, RISCV_ISA_EXT_ZKNE),
	__RISCV_ISA_EXT_DATA(zknh, RISCV_ISA_EXT_ZKNH),
	__RISCV_ISA_EXT_DATA(zkr, RISCV_ISA_EXT_ZKR),
	__RISCV_ISA_EXT_BUNDLE(zks, riscv_zks_bundled_exts),
	__RISCV_ISA_EXT_DATA(zkt, RISCV_ISA_EXT_ZKT),
	__RISCV_ISA_EXT_DATA(zksed, RISCV_ISA_EXT_ZKSED),
	__RISCV_ISA_EXT_DATA(zksh, RISCV_ISA_EXT_ZKSH),
	__RISCV_ISA_EXT_DATA(ztso, RISCV_ISA_EXT_ZTSO),
	__RISCV_ISA_EXT_SUPERSET(zvbb, RISCV_ISA_EXT_ZVBB, riscv_zvbb_exts),
	__RISCV_ISA_EXT_DATA(zvbc, RISCV_ISA_EXT_ZVBC),
	__RISCV_ISA_EXT_SUPERSET(zve32f, RISCV_ISA_EXT_ZVE32F, riscv_zve32f_exts),
	__RISCV_ISA_EXT_DATA(zve32x, RISCV_ISA_EXT_ZVE32X),
	__RISCV_ISA_EXT_SUPERSET(zve64d, RISCV_ISA_EXT_ZVE64D, riscv_zve64d_exts),
	__RISCV_ISA_EXT_SUPERSET(zve64f, RISCV_ISA_EXT_ZVE64F, riscv_zve64f_exts),
	__RISCV_ISA_EXT_SUPERSET(zve64x, RISCV_ISA_EXT_ZVE64X, riscv_zve64x_exts),
	__RISCV_ISA_EXT_DATA(zvfh, RISCV_ISA_EXT_ZVFH),
	__RISCV_ISA_EXT_DATA(zvfhmin, RISCV_ISA_EXT_ZVFHMIN),
	__RISCV_ISA_EXT_DATA(zvkb, RISCV_ISA_EXT_ZVKB),
	__RISCV_ISA_EXT_DATA(zvkg, RISCV_ISA_EXT_ZVKG),
	__RISCV_ISA_EXT_BUNDLE(zvkn, riscv_zvkn_bundled_exts),
	__RISCV_ISA_EXT_BUNDLE(zvknc, riscv_zvknc_bundled_exts),
	__RISCV_ISA_EXT_DATA(zvkned, RISCV_ISA_EXT_ZVKNED),
	__RISCV_ISA_EXT_BUNDLE(zvkng, riscv_zvkng_bundled_exts),
	__RISCV_ISA_EXT_DATA(zvknha, RISCV_ISA_EXT_ZVKNHA),
	__RISCV_ISA_EXT_DATA(zvknhb, RISCV_ISA_EXT_ZVKNHB),
	__RISCV_ISA_EXT_BUNDLE(zvks, riscv_zvks_bundled_exts),
	__RISCV_ISA_EXT_BUNDLE(zvksc, riscv_zvksc_bundled_exts),
	__RISCV_ISA_EXT_DATA(zvksed, RISCV_ISA_EXT_ZVKSED),
	__RISCV_ISA_EXT_DATA(zvksh, RISCV_ISA_EXT_ZVKSH),
	__RISCV_ISA_EXT_BUNDLE(zvksg, riscv_zvksg_bundled_exts),
	__RISCV_ISA_EXT_DATA(zvkt, RISCV_ISA_EXT_ZVKT),
	__RISCV_ISA_EXT_DATA(smaia, RISCV_ISA_EXT_SMAIA),
	__RISCV_ISA_EXT_DATA(smmpm, RISCV_ISA_EXT_SMMPM),
	__RISCV_ISA_EXT_SUPERSET(smnpm, RISCV_ISA_EXT_SMNPM, riscv_xlinuxenvcfg_exts),
	__RISCV_ISA_EXT_DATA(smstateen, RISCV_ISA_EXT_SMSTATEEN),
	__RISCV_ISA_EXT_DATA(ssaia, RISCV_ISA_EXT_SSAIA),
	__RISCV_ISA_EXT_DATA(sscofpmf, RISCV_ISA_EXT_SSCOFPMF),
	__RISCV_ISA_EXT_SUPERSET(ssnpm, RISCV_ISA_EXT_SSNPM, riscv_xlinuxenvcfg_exts),
	__RISCV_ISA_EXT_DATA(sstc, RISCV_ISA_EXT_SSTC),
	__RISCV_ISA_EXT_DATA(svinval, RISCV_ISA_EXT_SVINVAL),
	__RISCV_ISA_EXT_DATA(svnapot, RISCV_ISA_EXT_SVNAPOT),
	__RISCV_ISA_EXT_DATA(svpbmt, RISCV_ISA_EXT_SVPBMT),
	__RISCV_ISA_EXT_DATA(svvptc, RISCV_ISA_EXT_SVVPTC),
};

const size_t riscv_isa_ext_count = ARRAY_SIZE(riscv_isa_ext);

static void riscv_isa_set_ext(const struct riscv_isa_ext_data *ext, unsigned long *bitmap)
{
	if (ext->id != RISCV_ISA_EXT_INVALID)
		__set_bit(ext->id, bitmap);

	for (int i = 0; i < ext->subset_ext_size; i++) {
		if (ext->subset_ext_ids[i] != RISCV_ISA_EXT_INVALID)
			__set_bit(ext->subset_ext_ids[i], bitmap);
	}
}

static void match_isa_ext(const char *name, const char *name_end)
{
	for (int i = 0; i < riscv_isa_ext_count; i++) {
		const struct riscv_isa_ext_data *ext = &riscv_isa_ext[i];

		if ((name_end - name == strlen(ext->name)) &&
		    !strncasecmp(name, ext->name, name_end - name)) {
			if (ext->validate && !ext->validate(ext, riscv_isa))
				riscv_isa_set_ext(ext, riscv_isa);
			break;
		}
	}
}

static void riscv_parse_isa_string(const char *isa)
{
	/*
	 * For all possible cpus, we have already validated in
	 * the boot process that they at least contain "rv" and
	 * whichever of "32"/"64" this kernel supports, and so this
	 * section can be skipped.
	 */
	isa += 4;

	while (*isa) {
		const char *ext = isa++;
		const char *ext_end = isa;
		bool ext_err = false;

		switch (*ext) {
		case 'x':
		case 'X':
			log_warning("Vendor extensions are ignored in riscv,isa. Use riscv,isa-extensions instead.");
			/*
			 * To skip an extension, we find its end.
			 * As multi-letter extensions must be split from other multi-letter
			 * extensions with an "_", the end of a multi-letter extension will
			 * either be the null character or the "_" at the start of the next
			 * multi-letter extension.
			 */
			for (; *isa && *isa != '_'; ++isa)
				;
			ext_err = true;
			break;
		case 's':
			/*
			 * Workaround for invalid single-letter 's' & 'u' (QEMU).
			 * No need to set the bit in riscv_isa as 's' & 'u' are
			 * not valid ISA extensions. It works unless the first
			 * multi-letter extension in the ISA string begins with
			 * "Su" and is not prefixed with an underscore.
			 */
			if (ext[-1] != '_' && ext[1] == 'u') {
				++isa;
				ext_err = true;
				break;
			}
			fallthrough;
		case 'S':
		case 'z':
		case 'Z':
			/*
			 * Before attempting to parse the extension itself, we find its end.
			 * As multi-letter extensions must be split from other multi-letter
			 * extensions with an "_", the end of a multi-letter extension will
			 * either be the null character or the "_" at the start of the next
			 * multi-letter extension.
			 *
			 * Next, as the extensions version is currently ignored, we
			 * eliminate that portion. This is done by parsing backwards from
			 * the end of the extension, removing any numbers. This may be a
			 * major or minor number however, so the process is repeated if a
			 * minor number was found.
			 *
			 * ext_end is intended to represent the first character *after* the
			 * name portion of an extension, but will be decremented to the last
			 * character itself while eliminating the extensions version number.
			 * A simple re-increment solves this problem.
			 */
			for (; *isa && *isa != '_'; ++isa)
				if (unlikely(!isalnum(*isa)))
					ext_err = true;

			ext_end = isa;
			if (unlikely(ext_err))
				break;

			if (!isdigit(ext_end[-1]))
				break;

			while (isdigit(*--ext_end))
				;

			if (tolower(ext_end[0]) != 'p' || !isdigit(ext_end[-1])) {
				++ext_end;
				break;
			}

			while (isdigit(*--ext_end))
				;

			++ext_end;
			break;
		default:
			/*
			 * Things are a little easier for single-letter extensions, as they
			 * are parsed forwards.
			 *
			 * After checking that our starting position is valid, we need to
			 * ensure that, when isa was incremented at the start of the loop,
			 * that it arrived at the start of the next extension.
			 *
			 * If we are already on a non-digit, there is nothing to do. Either
			 * we have a multi-letter extension's _, or the start of an
			 * extension.
			 *
			 * Otherwise we have found the current extension's major version
			 * number. Parse past it, and a subsequent p/minor version number
			 * if present. The `p` extension must not appear immediately after
			 * a number, so there is no fear of missing it.
			 *
			 */
			if (unlikely(!isalpha(*ext))) {
				ext_err = true;
				break;
			}

			if (!isdigit(*isa))
				break;

			while (isdigit(*++isa))
				;

			if (tolower(*isa) != 'p')
				break;

			if (!isdigit(*++isa)) {
				--isa;
				break;
			}

			while (isdigit(*++isa))
				;

			break;
		}

		/*
		 * The parser expects that at the start of an iteration isa points to the
		 * first character of the next extension. As we stop parsing an extension
		 * on meeting a non-alphanumeric character, an extra increment is needed
		 * where the succeeding extension is a multi-letter prefixed with an "_".
		 */
		if (*isa == '_')
			++isa;

		if (unlikely(ext_err))
			continue;
		match_isa_ext(ext, ext_end);
	}
}

static inline bool supports_extension(char ext)
{
#if CONFIG_IS_ENABLED(RISCV_MMODE)
	return csr_read(CSR_MISA) & (1 << (ext - 'a'));
#elif CONFIG_CPU
	return __riscv_isa_extension_available(ext);
#else  /* !CONFIG_CPU */
#warning "There is no way to determine the available extensions in S-mode."
#warning "Please convert your board to use the RISC-V CPU driver."
	return false;
#endif /* CONFIG_CPU */
}

static int riscv_cpu_probe(void)
{
	if (CONFIG_IS_ENABLED(CPU)) {
		int ret;

		/* probe cpus so that RISC-V timer can be bound */
		ret = cpu_probe_all();
		if (ret)
			return log_msg_ret("RISC-V cpus probe failed\n", ret);
	}

	return 0;
}
EVENT_SPY_SIMPLE(EVT_DM_POST_INIT_R, riscv_cpu_probe);

/*
 * This is called on secondary harts just after the IPI is init'd. Currently
 * there's nothing to do, since we just need to clear any existing IPIs, and
 * that is handled by the sending of an ipi itself.
 */
#if CONFIG_IS_ENABLED(SMP)
static void dummy_pending_ipi_clear(ulong hart, ulong arg0, ulong arg1)
{
}
#endif

int riscv_cpu_setup(void)
{
	int ret = -ENODEV, ext_count, i;
	const char *isa, **exts;
	struct udevice *dev;

	uclass_find_first_device(UCLASS_CPU, &dev);
	if (!dev) {
		debug("unable to find the RISC-V cpu device\n");
		return ret;
	}

	ext_count = dev_read_string_list(dev, "riscv,isa-extensions", &exts);
	if (ext_count > 0) {
		for (i = 0; i < ext_count; i++)
			match_isa_ext(exts[i], exts[i] + strlen(exts[i]));
	} else {
		isa = dev_read_string(dev, "riscv,isa");
		if (!isa)
			return ret;
		riscv_parse_isa_string(isa);
	}

	/* Enable FPU */
	if (supports_extension('d') || supports_extension('f')) {
		csr_set(MODE_PREFIX(status), MSTATUS_FS);
		csr_write(CSR_FCSR, 0);
	}

	if (CONFIG_IS_ENABLED(RISCV_MMODE)) {
		/*
		 * Enable perf counters for cycle, time,
		 * and instret counters only
		 */
		if (supports_extension('u')) {
#ifdef CONFIG_RISCV_PRIV_1_9
			csr_write(CSR_MSCOUNTEREN, GENMASK(2, 0));
			csr_write(CSR_MUCOUNTEREN, GENMASK(2, 0));
#else
			csr_write(CSR_MCOUNTEREN, GENMASK(2, 0));
#endif
		}

		/* Disable paging */
		if (supports_extension('s'))
#ifdef CONFIG_RISCV_PRIV_1_9
			csr_read_clear(CSR_MSTATUS, SR_VM);
#else
			csr_write(CSR_SATP, 0);
#endif
	}

#if CONFIG_IS_ENABLED(SMP)
	ret = riscv_init_ipi();
	if (ret)
		return ret;

	/*
	 * Clear all pending IPIs on secondary harts. We don't do anything on
	 * the boot hart, since we never send an IPI to ourselves, and no
	 * interrupts are enabled
	 */
	ret = smp_call_function((ulong)dummy_pending_ipi_clear, 0, 0, 0);
	if (ret)
		return ret;
#endif

	return 0;
}
EVENT_SPY_SIMPLE(EVT_DM_POST_INIT_F, riscv_cpu_setup);

int arch_early_init_r(void)
{
	if (IS_ENABLED(CONFIG_SYSRESET_SBI))
		device_bind_driver(gd->dm_root, "sbi-sysreset",
				   "sbi-sysreset", NULL);

	return 0;
}

/**
 * harts_early_init() - A callback function called by start.S to configure
 * feature settings of each hart.
 *
 * In a multi-core system, memory access shall be careful here, it shall
 * take care of race conditions.
 */
__weak void harts_early_init(void)
{
}

#if !CONFIG_IS_ENABLED(SYSRESET)
void reset_cpu(void)
{
	printf("resetting ...\n");

	printf("reset not supported yet\n");
	hang();
}
#endif

/*
 * cleanup_before_linux() is called just before we call linux, which prepares
 * the processor for linux.
 * this weak implementation is used by default. we disable interrupts and flush
 * the cache.
 */
__weak int cleanup_before_linux(void)
{
	disable_interrupts();

	cache_flush();

	return 0;
}

void arch_setup_gd(gd_t *new_gd)
{
	set_gd(new_gd);
}

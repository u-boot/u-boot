/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_CPUFEATURE_H
#define _ASM_CPUFEATURE_H
struct riscv_isa_ext_data {
	const unsigned int id;
	const char *name;
	const char *property;
	const unsigned int *subset_ext_ids;
	const unsigned int subset_ext_size;
	int (*validate)(const struct riscv_isa_ext_data *data, const unsigned long *isa_bitmap);
};

#define _RISCV_ISA_EXT_DATA(_name, _id, _subset_exts, _subset_exts_size, _validate) {	\
	.name = #_name,									\
	.property = #_name,								\
	.id = _id,									\
	.subset_ext_ids = _subset_exts,							\
	.subset_ext_size = _subset_exts_size,						\
	.validate = _validate								\
}

#define __RISCV_ISA_EXT_DATA(_name, _id) _RISCV_ISA_EXT_DATA(_name, _id, NULL, 0, NULL)
#define __RISCV_ISA_EXT_DATA_VALIDATE(_name, _id, _validate) \
			_RISCV_ISA_EXT_DATA(_name, _id, NULL, 0, _validate)

/* Used to declare pure "lasso" extension (Zk for instance) */
#define __RISCV_ISA_EXT_BUNDLE(_name, _bundled_exts) \
	_RISCV_ISA_EXT_DATA(_name, RISCV_ISA_EXT_INVALID, _bundled_exts, \
			    ARRAY_SIZE(_bundled_exts), NULL)

/* Used to declare extensions that are a superset of other extensions (Zvbb for instance) */
#define __RISCV_ISA_EXT_SUPERSET(_name, _id, _sub_exts) \
	_RISCV_ISA_EXT_DATA(_name, _id, _sub_exts, ARRAY_SIZE(_sub_exts), NULL)
#define __RISCV_ISA_EXT_SUPERSET_VALIDATE(_name, _id, _sub_exts, _validate) \
	_RISCV_ISA_EXT_DATA(_name, _id, _sub_exts, ARRAY_SIZE(_sub_exts), _validate)
#endif

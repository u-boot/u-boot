// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_variables_runtime
 *
 * Copyright (c) 2019 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * This unit test checks the runtime services for variables after
 * ExitBootServices():
 * GetVariable, GetNextVariableName, SetVariable, QueryVariableInfo.
 */

#include <efi_selftest.h>
#include <efi_variable.h>
#include <u-boot/crc.h>

#define EFI_ST_MAX_DATA_SIZE 16
#define EFI_ST_MAX_VARNAME_SIZE 40

static struct efi_boot_services *boottime;
static struct efi_runtime_services *runtime;
static const efi_guid_t guid_vendor0 = EFI_GLOBAL_VARIABLE_GUID;
static const efi_guid_t __efi_runtime_data efi_rt_var_guid =
						U_BOOT_EFI_RT_VAR_FILE_GUID;

/*
 * Setup unit test.
 *
 * @handle	handle of the loaded image
 * @systable	system table
 */
static int setup(const efi_handle_t img_handle,
		 const struct efi_system_table *systable)
{
	boottime = systable->boottime;
	runtime = systable->runtime;

	return EFI_ST_SUCCESS;
}

/**
 * execute() - execute unit test
 *
 * As runtime support is not implmented expect EFI_UNSUPPORTED to be returned.
 */
static int execute(void)
{
	efi_status_t ret;
	efi_uintn_t len, avail, append_len = 17;
	u32 attr;
	u8 v[16] = {0x5d, 0xd1, 0x5e, 0x51, 0x5a, 0x05, 0xc7, 0x0c,
		    0x35, 0x4a, 0xae, 0x87, 0xa5, 0xdf, 0x0f, 0x65,};
	u8 v2[CONFIG_EFI_VAR_BUF_SIZE];
	u8 data[EFI_ST_MAX_DATA_SIZE];
	u8 data2[CONFIG_EFI_VAR_BUF_SIZE];
	u16 varname[EFI_ST_MAX_VARNAME_SIZE];
	efi_guid_t guid;
	u64 max_storage, rem_storage, max_size;
	int test_ret;

	memset(v2, 0x1, sizeof(v2));

	if (IS_ENABLED(CONFIG_EFI_VARIABLE_FILE_STORE)) {
		test_ret = efi_st_query_variable_common(runtime, EFI_VARIABLE_BOOTSERVICE_ACCESS |
								 EFI_VARIABLE_RUNTIME_ACCESS);
		if (test_ret != EFI_ST_SUCCESS) {
			efi_st_error("QueryVariableInfo failed\n");
			return EFI_ST_FAILURE;
		}
	} else {
		ret = runtime->query_variable_info(EFI_VARIABLE_BOOTSERVICE_ACCESS,
					   &max_storage, &rem_storage,
					   &max_size);
		if (ret != EFI_UNSUPPORTED) {
			efi_st_error("QueryVariableInfo failed\n");
			return EFI_ST_FAILURE;
		}
	}

	ret = runtime->set_variable(u"efi_st_var0", &guid_vendor0,
				    EFI_VARIABLE_BOOTSERVICE_ACCESS |
				    EFI_VARIABLE_RUNTIME_ACCESS,
				    3, v + 4);
	if (IS_ENABLED(CONFIG_EFI_RT_VOLATILE_STORE)) {
		efi_uintn_t prev_len, delta;
		struct efi_var_entry *var;
		struct efi_var_file *hdr;

		/* At runtime only non-volatile variables may be set. */
		if (ret != EFI_INVALID_PARAMETER) {
			efi_st_error("SetVariable failed\n");
			return EFI_ST_FAILURE;
		}

		/* runtime atttribute must be set */
		ret = runtime->set_variable(u"efi_st_var0", &guid_vendor0,
					    EFI_VARIABLE_BOOTSERVICE_ACCESS |
					    EFI_VARIABLE_NON_VOLATILE,
					    3, v + 4);
		if (ret != EFI_INVALID_PARAMETER) {
			efi_st_error("SetVariable failed\n");
			return EFI_ST_FAILURE;
		}

		len = sizeof(data);
		ret = runtime->get_variable(u"RTStorageVolatile",
					    &efi_rt_var_guid,
					    &attr, &len, data);
		if (ret != EFI_SUCCESS) {
			efi_st_error("GetVariable failed\n");
			return EFI_ST_FAILURE;
		}

		if (len != sizeof(EFI_VAR_FILE_NAME) ||
		    memcmp(data, EFI_VAR_FILE_NAME, sizeof(EFI_VAR_FILE_NAME))) {
			data[len - 1] = 0;
			efi_st_error("RTStorageVolatile = %s\n", data);
			return EFI_ST_FAILURE;
		}

		len = sizeof(data2);
		ret = runtime->get_variable(u"VarToFile", &efi_rt_var_guid,
					    &attr, &len, data2);
		if (ret != EFI_SUCCESS) {
			efi_st_error("GetVariable failed\n");
			return EFI_ST_FAILURE;
		}
		/*
		 * VarToFile size must change once a variable is inserted
		 * Store it now, we'll use it later
		 */
		prev_len = len;
		ret = runtime->set_variable(u"efi_st_var0", &guid_vendor0,
					    EFI_VARIABLE_BOOTSERVICE_ACCESS |
					    EFI_VARIABLE_RUNTIME_ACCESS |
					    EFI_VARIABLE_NON_VOLATILE,
					    sizeof(v2),
					    v2);
		/*
		 * This will try to update VarToFile as well and must fail,
		 * without changing or deleting VarToFile
		 */
		if (ret != EFI_OUT_OF_RESOURCES) {
			efi_st_error("SetVariable failed\n");
			return EFI_ST_FAILURE;
		}
		len = sizeof(data2);
		ret = runtime->get_variable(u"VarToFile", &efi_rt_var_guid,
					    &attr, &len, data2);
		if (ret != EFI_SUCCESS || prev_len != len) {
			efi_st_error("Get/SetVariable failed\n");
			return EFI_ST_FAILURE;
		}

		/* Add an 8byte aligned variable */
		ret = runtime->set_variable(u"efi_st_var0", &guid_vendor0,
					    EFI_VARIABLE_BOOTSERVICE_ACCESS |
					    EFI_VARIABLE_RUNTIME_ACCESS |
					    EFI_VARIABLE_NON_VOLATILE,
					    sizeof(v), v);
		if (ret != EFI_SUCCESS) {
			efi_st_error("SetVariable failed\n");
			return EFI_ST_FAILURE;
		}

		/* Delete it by setting the attrs to 0 */
		ret = runtime->set_variable(u"efi_st_var0", &guid_vendor0,
					    0, sizeof(v), v);
		if (ret != EFI_SUCCESS) {
			efi_st_error("SetVariable failed\n");
			return EFI_ST_FAILURE;
		}

		/* Add it back */
		ret = runtime->set_variable(u"efi_st_var0", &guid_vendor0,
					    EFI_VARIABLE_BOOTSERVICE_ACCESS |
					    EFI_VARIABLE_RUNTIME_ACCESS |
					    EFI_VARIABLE_NON_VOLATILE,
					    sizeof(v), v);
		if (ret != EFI_SUCCESS) {
			efi_st_error("SetVariable failed\n");
			return EFI_ST_FAILURE;
		}

		/* Delete it again by setting the size to 0 */
		ret = runtime->set_variable(u"efi_st_var0", &guid_vendor0,
					    EFI_VARIABLE_BOOTSERVICE_ACCESS |
					    EFI_VARIABLE_RUNTIME_ACCESS |
					    EFI_VARIABLE_NON_VOLATILE,
					    0, NULL);
		if (ret != EFI_SUCCESS) {
			efi_st_error("SetVariable failed\n");
			return EFI_ST_FAILURE;
		}

		/* Delete it again and make sure it's not there */
		ret = runtime->set_variable(u"efi_st_var0", &guid_vendor0,
					    EFI_VARIABLE_BOOTSERVICE_ACCESS |
					    EFI_VARIABLE_RUNTIME_ACCESS |
					    EFI_VARIABLE_NON_VOLATILE,
					    0, NULL);
		if (ret != EFI_NOT_FOUND) {
			efi_st_error("SetVariable failed\n");
			return EFI_ST_FAILURE;
		}

		/*
		 * Add a non-aligned variable
		 * VarToFile updates must include efi_st_var0
		 */
		ret = runtime->set_variable(u"efi_st_var0", &guid_vendor0,
					    EFI_VARIABLE_BOOTSERVICE_ACCESS |
					    EFI_VARIABLE_RUNTIME_ACCESS |
					    EFI_VARIABLE_NON_VOLATILE,
					    9, v + 4);
		if (ret != EFI_SUCCESS) {
			efi_st_error("SetVariable failed\n");
			return EFI_ST_FAILURE;
		}
		var = efi_var_mem_find(&guid_vendor0, u"efi_st_var0", NULL);
		if (!var) {
			efi_st_error("GetVariable failed\n");
			return EFI_ST_FAILURE;
		}
		delta = efi_var_entry_len(var);
		len = sizeof(data2);
		ret = runtime->get_variable(u"VarToFile", &efi_rt_var_guid,
					    &attr, &len, data2);
		if (ret != EFI_SUCCESS || prev_len + delta != len) {
			efi_st_error("Get/SetVariable failed\n");
			return EFI_ST_FAILURE;
		}

		/*
		 * Append on an existing variable must update VarToFile
		 * Our variable entries are 8-byte aligned.
		 * Adding a single byte will fit on the existing space
		 */
		prev_len = len;
		avail = efi_var_entry_len(var) -
			(sizeof(u16) * (u16_strlen(var->name) + 1) + sizeof(*var)) -
			var->length;
		if (avail >= append_len)
			delta = 0;
		else
			delta = ALIGN(append_len - avail, 8);
		ret = runtime->set_variable(u"efi_st_var0", &guid_vendor0,
					    EFI_VARIABLE_BOOTSERVICE_ACCESS |
					    EFI_VARIABLE_RUNTIME_ACCESS |
					    EFI_VARIABLE_APPEND_WRITE |
					    EFI_VARIABLE_NON_VOLATILE,
					    append_len, v2);
		if (ret != EFI_SUCCESS) {
			efi_st_error("SetVariable failed\n");
			return EFI_ST_FAILURE;
		}
		len = sizeof(data2);
		ret = runtime->get_variable(u"VarToFile", &efi_rt_var_guid,
					    &attr, &len, data2);
		if (ret != EFI_SUCCESS) {
			efi_st_error("GetVariable failed\n");
			return EFI_ST_FAILURE;
		}
		if (prev_len + delta != len) {
			efi_st_error("Unexpected VarToFile size");
			return EFI_ST_FAILURE;
		}

		/* Make sure that variable contains a valid file */
		hdr = (struct efi_var_file *)data2;
		if (hdr->magic != EFI_VAR_FILE_MAGIC ||
		    len != hdr->length ||
		    hdr->crc32 != crc32(0, (u8 *)((uintptr_t)data2 + sizeof(struct efi_var_file)),
					len - sizeof(struct efi_var_file))) {
			efi_st_error("VarToFile invalid header\n");
			return EFI_ST_FAILURE;
		}

		/* Variables that are BS, RT and volatile are RO after EBS */
		ret = runtime->set_variable(u"VarToFile", &efi_rt_var_guid,
					    EFI_VARIABLE_BOOTSERVICE_ACCESS |
					    EFI_VARIABLE_RUNTIME_ACCESS |
					    EFI_VARIABLE_NON_VOLATILE,
					    sizeof(v), v);
		if (ret != EFI_WRITE_PROTECTED) {
			efi_st_error("Get/SetVariable failed\n");
			return EFI_ST_FAILURE;
		}
	} else {
		if (ret != EFI_UNSUPPORTED) {
			efi_st_error("SetVariable failed\n");
			return EFI_ST_FAILURE;
		}
	}
	len = EFI_ST_MAX_DATA_SIZE;
	ret = runtime->get_variable(u"PlatformLangCodes", &guid_vendor0,
				    &attr, &len, data);
	if (ret != EFI_SUCCESS) {
		efi_st_error("GetVariable failed\n");
		return EFI_ST_FAILURE;
	}
	memset(&guid, 0, 16);
	*varname = 0;
	len = 2 * EFI_ST_MAX_VARNAME_SIZE;
	ret = runtime->get_next_variable_name(&len, varname, &guid);
	if (ret != EFI_SUCCESS) {
		efi_st_error("GetNextVariableName failed\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(variables_run) = {
	.name = "variables at runtime",
	.phase = EFI_SETUP_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
};

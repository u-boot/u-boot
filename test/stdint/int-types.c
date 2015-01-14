#include <common.h>
#include <inttypes.h>

int test_types(void)
{
	uintptr_t uintptr = 0;
	uint64_t uint64 = 0;
	u64 u64_val = 0;

	printf("uintptr = %" PRIuPTR "\n", uintptr);
	printf("uint64 = %" PRIu64 "\n", uint64);
	printf("u64 = %" PRIu64 "\n", u64_val);
}

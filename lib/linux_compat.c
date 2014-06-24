
#include <common.h>
#include <linux/compat.h>

struct p_current cur = {
	.pid = 1,
};
__maybe_unused struct p_current *current = &cur;

unsigned long copy_from_user(void *dest, const void *src,
		     unsigned long count)
{
	memcpy((void *)dest, (void *)src, count);
	return 0;
}

void *kmalloc(size_t size, int flags)
{
	return memalign(ARCH_DMA_MINALIGN, size);
}

void *kzalloc(size_t size, int flags)
{
	void *ptr = kmalloc(size, flags);
	memset(ptr, 0, size);
	return ptr;
}

void *vzalloc(unsigned long size)
{
	return kzalloc(size, 0);
}

struct kmem_cache *get_mem(int element_sz)
{
	struct kmem_cache *ret;

	ret = memalign(ARCH_DMA_MINALIGN, sizeof(struct kmem_cache));
	ret->sz = element_sz;

	return ret;
}

void *kmem_cache_alloc(struct kmem_cache *obj, int flag)
{
	return memalign(ARCH_DMA_MINALIGN, obj->sz);
}

#include <Buffer.hpp>

#ifndef MAP_HUGETLB
#define MAP_HUGETLB 0x40000 /* arch specific */
#endif

/* Only ia64 requires this */
#ifdef __ia64__
#define ADDR (void *)(0x8000000000000000UL)
#define FLAGS (MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB | MAP_FIXED)
#else
#define ADDR (void *)(0x0UL)
#define FLAGS (MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB)
#endif

/* My Customs */
#define PROTECTION (PROT_READ | PROT_WRITE)

Large_buffer::Large_buffer( size_t Buffer_size, bool Contiguous ) : Buffer( Buffer_size, Linux_attributes::POOL_SIZE_2MB, PAGE_2MB, Contiguous )
{
	Error_code status;
	if( ( status = Allocate_memory() ) != OK )
		exit(status);
	if( ( status = Initialize() ) != OK )
		exit(status);
}

Buffer::Error_code
Large_buffer::Allocate_memory()
{
	void * Allocd_buffer = mmap( ADDR, Get_allocated_size(), PROTECTION, FLAGS, 0, 0);
	if( Allocd_buffer == MAP_FAILED )
	{
		perror("Failed to mmap hugepage");
		return FAILED_GET_MEMORY_FROM_OS;
	}
	
	M_Buffer = reinterpret_cast<uintptr_t>( Allocd_buffer );
	return OK;
}

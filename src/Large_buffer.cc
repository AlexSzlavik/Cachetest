#include<Buffer.hpp>

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
	void * Allocd_buffer = mmap( 0x0, Get_allocated_size(), PROTECTION, FLAGS, -1, 0);
	if( Allocd_buffer == MAP_FAILED )
		return FAILED_GET_MEMORY_FROM_OS;
	
	M_Buffer = reinterpret_cast<uintptr_t>( Allocd_buffer );
	return OK;
}

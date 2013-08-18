#include <Buffer.hpp>
/* 
 * Buffer Constructor
 */
Small_buffer::Small_buffer( size_t Buffer_size, bool Contiguous ) : Buffer( Buffer_size, Linux_attributes::POOL_SIZE_4KB, getpagesize(), Contiguous )
{
	Error_code status;
	Allocate_memory();
	if( ( status = Initialize() ) != OK )
		exit(status);
}

/* Initialize
 * Allocate the physical buffer and request it from the OS
 * This is the entire buffer + contiguous search regions 
 */
Buffer::Error_code 
Small_buffer::Allocate_memory()
{
    M_Buffer = reinterpret_cast<uintptr_t>( malloc( Get_allocated_size() ) );
	if( M_Buffer == 0x0 )
		return FAILED_GET_MEMORY_FROM_OS;
	return OK;
}

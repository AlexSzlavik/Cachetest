/*
 * Buffer.cc
 * Here we implement the various ways of allocating the bufer.
 */

#include "Buffer.hpp"

namespace
{
	inline size_t
	Round_to_alignment( size_t Address, size_t Alignment )
	{
		return ( Address + ( Alignment - 1 ) ) & ~( Alignment - 1 );
	}
	
	inline size_t
	Page_number( size_t Address )
	{
		size_t Offset = 0;
		while( Address != 1 )
		{
			Address /= 2;
			Offset += 1;
		}
		return Offset;
	}
	Buffer::Error_code
	Open_proc_files( int * File_pagemap, int * File_kpageflags )
	{
		std::stringstream	Pagemap_filename;

		Pagemap_filename << "/proc/" << getpid() << "/pagemap";

		// Open the Proc files
		*File_pagemap = open( Pagemap_filename.str().c_str(), O_RDONLY, NULL );
		if( *File_pagemap < 0 )
		{ 
			perror("Opening /proc/../pagemap");
			return Buffer::FAILED_TO_OPEN_PAGEMAP;
		}

		*File_kpageflags = open("/proc/kpageflags", O_RDONLY, NULL );
		if( *File_kpageflags  < 0 )
		{
			perror("Opening kpageflags");
			return Buffer::FAILED_TO_OPEN_KPAGEFLAGS;
		}
	}
};

/*
 * Abstract_Buffer constructor
 */
Buffer::Buffer( size_t Buffer_size, size_t Pool_size, size_t Logical_page_size, bool Contiguous ) : 	M_Buffer_size( Buffer_size ),
																										M_Contiguous( Contiguous ),
																										M_Logical_page_size( Logical_page_size ),
																										M_Page_size( getpagesize() )
{
	if( Contiguous )
		M_Pool_size = Round_to_alignment( Pool_size + Buffer_size , M_Logical_page_size );
	else
		M_Pool_size = M_Buffer_size;	//Buffer can be much smaller if it's not contiguous
}

Buffer::Error_code
Buffer::Populate_frame_to_page_map( Frame_to_page_map & Map )
{
	// This is platform (OS) specific. Currently we assume Linux
	int 		File_pagemap, File_kpageflags;
    uint64_t	idx = 1, Frames_to_dump = Get_allocated_size()/M_Page_size;
	size_t		Pagemap_offset;
	uint64_t	Dev_data = 0, Dev_flags = 0, PFN = 0;

	Pagemap_offset = ( M_Buffer >> Page_number( M_Page_size ) ) * PAGEMAP_ENTRY_SIZE;

	Open_proc_files( &File_pagemap, &File_kpageflags );

	// Seek to the page corresponding to our buffer
    if( lseek( File_pagemap, Pagemap_offset, SEEK_SET) < 0 )
	{
        perror("Pagemap seek");
		return FAILED_TO_SEEK_PAGEMAP;
    }

	// For every page, get the PFN, look up the PFN in the pageflags and get the pages flags
	// The flags tell us about contiguouty 
    for( idx=0 ; idx < Frames_to_dump; idx++ )
	{
        if( read( File_pagemap, &Dev_data, sizeof( Dev_data ) ) <= 0 ) {
            perror("read pagemap:");
            exit(1);
        }
        if( lseek( File_kpageflags, ( Dev_data & Linux_attributes::PAGEMAP_PFN_MASK ) * 8 ,SEEK_SET ) < 0)
		{
            perror("Seek Flags:");
            exit(1);
        }
        if( read( File_kpageflags, &Dev_flags, sizeof( Dev_flags ) ) < 0 )
		{
            perror("read kpageflags:");
            exit(1);
        }

        //Skip non hugepage heads
	    if( Is_large_buffer() && ! ( Dev_flags & (1 << KPF_COMPOUND_HEAD ) ) )
            continue;

        PFN = Dev_data & Linux_attributes::PAGEMAP_PFN_MASK;   //PFN
        Map[PFN] = M_Buffer + idx * M_Page_size;

#ifdef DEBUG_CREATE
        std::cout << "IDX: " << idx << std::endl;
        std::cout << "PFN->VPN:\t" << std::hex << "0x" << PFN << " -> 0x" << Map[PFN] << std::endl;
        std::cout << std::dec;
        printf("PageMap:\t0x%016llx\n",Dev_data);
        printf("Flags:\t\t0x%016llx\n",Dev_flags);
	    if(Dev_flags & (1 << KPF_COMPOUND_HEAD))
	    	printf("COMPOUND_HEAD SET\n");
	    if(Dev_flags & (1 << KPF_COMPOUND_TAIL))
	    	printf("COMPOUND_TAIL SET\n");
	    if(Dev_flags & (1 << KPF_HUGE))
	    	printf("HUGE SET\n");
	    printf("\n");
#endif
    }

    close( File_pagemap );
    close( File_kpageflags );
}

Buffer::Error_code
Buffer::Find_contiguous_range()
{
	Buffer::Error_code 			status;
	Frame_to_page_map Tmp_slabs;
	if( ( status = Populate_frame_to_page_map( Tmp_slabs ) ) != OK )
		return status;

	// Find contiguous range in slabs
	//TODO: We could make this cache aware and loosen the requirement on the contiguous pages,
	//		to only be cache-wise contiguous. This will require knowledge of the cache configuration
	size_t 							Required_pages;
	uintptr_t						Last_PFN = 0;
	Frame_to_page_map::iterator		Map_iterator;

	Required_pages = 				( Round_to_alignment( M_Buffer_size, M_Logical_page_size )  / M_Logical_page_size ) - 1;
	M_Buffer_slabs.push_back( Tmp_slabs.begin()->second );

	for( Map_iterator = Tmp_slabs.begin(); Map_iterator != Tmp_slabs.end(); Map_iterator++ )
	{
		uintptr_t PFN = Map_iterator->first;
		uintptr_t VPN = Map_iterator->second;

		if( Required_pages == 0 )
			return OK;

		if( PFN == Last_PFN + 1 )
		{
			Required_pages -= 1;
			M_Buffer_slabs.push_back( VPN );
		}
		else
		{
			Required_pages = ( Round_to_alignment( M_Buffer_size, M_Logical_page_size )  / M_Logical_page_size ) - 1;
			M_Buffer_slabs.clear();
			M_Buffer_slabs.push_back( VPN );
		}
		Last_PFN = PFN;
	}

	if( Required_pages == 0 )
		return OK;
	else
		return NO_CONTIGUOUS_RANGE_FOUND;
}

Buffer::Error_code
Buffer::Initialize()
{
	if( M_Buffer == 0 )
		return BUFFER_NOT_ALLOCATED;

    memset( reinterpret_cast<void*>( M_Buffer ), 0x0 , Get_allocated_size() );

	if( M_Buffer % ~( M_Page_size - 1 ) != 0 )	//Ensure that the buffer is page aligned
		M_Buffer = Round_to_alignment( M_Buffer, M_Logical_page_size );

	if( M_Contiguous )
		if( Find_contiguous_range() != OK )
			return FAILED_CONTIGUOUS_LOOKUP;

	M_Start_address = M_Buffer;

	return OK;
}

size_t
Buffer::Get_size()
{
	return M_Buffer_size;
}

size_t
Buffer::Get_allocated_size()
{
	return M_Pool_size;
}

unsigned char *
Buffer::Get_buffer_pointer()
{
	return reinterpret_cast<unsigned char*>(M_Buffer);
}

unsigned char *
Buffer::Get_start_address()
{
	return reinterpret_cast<unsigned char*>(M_Start_address);
}

void
Buffer::Set_buffer_offset( size_t Offset )
{
	M_Start_address += Offset;
}

void
Buffer::Dump_frames( std::string Filename )
{

}

bool
Buffer::Is_large_buffer() 
{
	return false;
}

std::vector< uintptr_t > &
Buffer::Get_slabs()
{
	return M_Buffer_slabs;
}

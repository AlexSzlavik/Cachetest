/*
 * Buffer.h
 */

#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <cstdlib>
#include <csignal>
#include <cstring>
#include <cstdio>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <iostream>
#include <string>
#include <map>
#include <iomanip>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <linux/kernel-page-flags.h>

namespace Linux_attributes
{
	const uint64_t PAGEMAP_PFN_MASK 		= ( 1ULL << 55 ) - 1;
	const uint64_t PAGEMAP_MAPPED_MASK 		= ( 1ULL << 63 );
	const uint64_t POOL_SIZE_4KB			= ( 1 << 25 );
	const uint64_t POOL_SIZE_2MB 			= ( 1 << 26 );
};

enum
{
	PAGEMAP_ENTRY_SIZE 		= 8,
	PAGE_4KB				= ( 1 << 12 ),
	PAGE_2MB				= ( 1 << 21 ),
	PAGE_1GB				= ( 1 << 30 ),
};

typedef std::map< uintptr_t, uintptr_t > Frame_to_page_map;

/*
 * class Buffer
 * The class represents the allocated buffer for our experiment
 * This container was creted to support contiguous huge pages
 */

class Buffer
{

	public:
		Buffer( size_t Buffer_size, size_t Pool_size, size_t Alignment, bool contiguous = false );
			
		size_t							Get_size();
		size_t							Get_allocated_size();
		unsigned char *					Get_buffer_pointer();
		unsigned char *					Get_start_address();
		std::vector<uintptr_t>&			Get_slabs();

		void							Set_buffer_offset( size_t Offset );

		void							Dump_frames( std::string Filename );
		virtual bool					Is_large_buffer();

		enum Error_code
		{
			OK = 0,
			FAILED_GET_MEMORY_FROM_OS,
			FAILED_ALLOCATE_BUFFER,
			FAILED_CONTIGUOUS_LOOKUP,
			BUFFER_NOT_ALLOCATED,
			FAILED_TO_OPEN_PAGEMAP,
			FAILED_TO_OPEN_KPAGEFLAGS,
			FAILED_TO_SEEK_PAGEMAP,
			FAILED_TO_SEEK_KPAGEFLAGS,
			NOT_IMPLEMENTED,
			NO_CONTIGUOUS_RANGE_FOUND,
		};

	protected:
	    const size_t 					M_Buffer_size;
		const bool						M_Contiguous;
		const size_t					M_Logical_page_size;
		const size_t					M_Page_size;

		uintptr_t 						M_Buffer;
		uintptr_t 						M_Start_address;

		std::vector<uintptr_t>			M_Buffer_slabs;

		Error_code						Initialize();

	private:
		size_t							M_Pool_size;

		Error_code						Find_contiguous_range();
		virtual Error_code				Populate_frame_to_page_map( Frame_to_page_map & Map );
		
		virtual Error_code				Allocate_memory() = 0;	//Must be implemented
};

class Small_buffer : public Buffer
{
	public:
		Small_buffer( size_t Buffer_size, bool Contiguous );

		bool				Is_large_buffer() { return false; }

	protected:

	private:
		Buffer::Error_code	Allocate_memory();
};

class Large_buffer : public Buffer
{
	public:
		Large_buffer( size_t Buffer_size, bool Contiguous );
		bool							Is_large_buffer() { return true; }

	private:
		virtual Buffer::Error_code		Allocate_memory();
};

//class Buffer : public Abstract_buffer
//{
//
//	public:
//	    Buffer();
//	    Buffer( size_t size );
//	    Buffer( size_t size, bool cont = false );
//	    ~Buffer();
//	    
//	    bool isHuge() 					{ return huge; }
//	    unsigned char* 					getPtr() { return this->theMemory; }
//	    unsigned char* 					getStartAddr() { return this->startAddr;}
//	    unsigned long 					getSize() { return this->size;}
//	    unsigned long 					getActualSize() { return this->actualSize;}
//	    std::vector<unsigned long long> &getSlabs() { return this->slabs;}
//	    void 							setBuffer(unsigned char* buff,unsigned int size) { this->theMemory = buff; this->actualSize = size;}
//	    void 							setStartAddr(unsigned char *start) {this->startAddr = start;};
//	    void 							setContRange(std::map<unsigned long long, unsigned long long>::iterator it, unsigned int pages);
//	    void 							setHuge(bool isHuge) { this->huge = isHuge;};
//	    void 							offsetBuffer(int offset);
//	    void 							dumpFrames(std::string filename);
//	
//		unsigned char *					Get_pointer() { return 0; }
//		unsigned char *					Get_start_address() { return 0; }
//		std::vector<uint64_t>			Get_slabs() { new std::vector<uint64_t>; }
//	private:
//		Error_code	Allocate_buffer();
//		void		Find_contiguous_range();
//};

//class Huge_Buffer : public Abstract_buffer
//{
//
//}

//Buffer& createBuffer(unsigned int size,int reqcont);
//Buffer& createLargeBuffer(unsigned int size, int reqcont);

#endif

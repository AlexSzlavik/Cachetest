/*
 * Buffer.h
 */

#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <csignal>
#include <cstring>
#include <unistd.h>
#include <iomanip>
#include <sched.h>
#include <sys/time.h>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdio>
#include <sys/mman.h>
#include <map>
#include <math.h>

#ifndef MAP_HUGETLB
#define MAP_HUGETLB 0x40000 /* arch specific */
#endif

#define KPF_LOCKED		0
#define KPF_ERROR		1
#define KPF_REFERENCED		2
#define KPF_UPTODATE		3
#define KPF_DIRTY		4
#define KPF_LRU			5
#define KPF_ACTIVE		6
#define KPF_SLAB		7
#define KPF_WRITEBACK		8
#define KPF_RECLAIM		9
#define KPF_BUDDY		10

/* 11-20: new additions in 2.6.31 */
#define KPF_MMAP		11
#define KPF_ANON		12
#define KPF_SWAPCACHE		13
#define KPF_SWAPBACKED		14
#define KPF_COMPOUND_HEAD	15
#define KPF_COMPOUND_TAIL	16
#define KPF_HUGE		17
#define KPF_UNEVICTABLE		18
#define KPF_HWPOISON		19
#define KPF_NOPAGE		20

#define KPF_KSM			21

/* kernel hacking assistances
 * WARNING: subject to change, never rely on them!
 */
#define KPF_RESERVED		32
#define KPF_MLOCKED		33
#define KPF_MAPPEDTODISK	34
#define KPF_PRIVATE		35
#define KPF_PRIVATE_2		36
#define KPF_OWNER_PRIVATE	37
#define KPF_ARCH		38
#define KPF_UNCACHED		39

/* My Customs */
#define PAGEMAP_ENTRY_SIZE 8
#define PAGEMAP_PFN_MASK    0x00FFFFFFFFFFFFFFULL
#define PAGEMAP_MAPPED_MASK 0x8000000000000000ULL
#define FLAGS (MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB )
#define PROTECTION (PROT_READ | PROT_WRITE)

/*
 * class Buffer
 * The class represents the allocated buffer for our experiment
 * This container was creted to support contiguous huge pages
 */
class Buffer {
    unsigned int size;
    unsigned int actualSize;
    bool huge;
    unsigned char * theMemory;
    unsigned char * startAddr;
    std::vector<unsigned long long> slabs;
    
public:
    Buffer() {huge=false;};
    Buffer(unsigned int size) { this->size = size; huge = false;}
    Buffer(unsigned int size, bool cont);
    Buffer(unsigned int size, bool cont, bool isHuge);
    ~Buffer();
    
    bool isHuge() { return huge;}
    unsigned char* getPtr() { return this->theMemory;}
    unsigned char* getStartAddr() { return this->startAddr;}
    unsigned long getSize() { return this->size;}
    unsigned long getActualSize() { return this->actualSize;}
    std::vector<unsigned long long> &getSlabs() { return this->slabs;}
    void setBuffer(unsigned char* buff,unsigned int size) { this->theMemory = buff; this->actualSize = size;}
    void setStartAddr(unsigned char *start) {this->startAddr = start;};
    void setContRange(std::map<unsigned long long, unsigned long long>::iterator it, unsigned int pages);
    void setHuge(bool isHuge) { this->huge = isHuge;};
    void offsetBuffer(int offset);
    void dumpFrames(std::string filename);
};

Buffer& createBuffer(unsigned int size,int reqcont);
Buffer& createLargeBuffer(unsigned int size, int reqcont);

#endif

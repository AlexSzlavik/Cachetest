/*
 * Buffer.cc
 * Here we implement the various ways of allocating the bufer.
 */

#include "Buffer.h"

Buffer& createLargeBuffer(unsigned int buffersize, int reqcont) {
    int page_size = getpagesize();
    int page_offset = 0;
    int fp,fp2;
    unsigned long long page_mask = 0x1;
    //Don't request more then 1 Gig, or increase nr_hugepages
    unsigned long long newBufferSize = buffersize + (1U << 28); //Allocate an extra 1GB
    unsigned char *buffer;
    unsigned long long addr;
	std::map<unsigned long long, unsigned long long> phys_to_virt;	 
    //Read out values
    unsigned long long data = 0;
    unsigned long long flags = 0;
    unsigned long long prev = 0;
    Buffer *bufferCl = new Buffer(buffersize);

    newBufferSize += (1UL << 21) - (newBufferSize % (1UL << 21)); //Must be huge page aligned (2 MB on Douze)
    buffer = (unsigned char*)mmap(0x0,newBufferSize,PROTECTION,FLAGS,-1,0);
    bufferCl->setBuffer(buffer,newBufferSize);
    bufferCl->setHuge(true);
    bufferCl->setStartAddr(buffer);
    if(buffer == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    memset(buffer,0x0,newBufferSize);

    while(page_size!=1) {
        page_size /= 2;
        page_offset += 1;
        page_mask = (page_mask << 1) | 0x1;
    }
    page_mask = page_mask >> 1;
    page_size = getpagesize();
#ifdef DEBUG_CREATE
    std::cout << "PageSize / Offset: " << page_size << " / " << page_offset << std::endl;
#endif

    //We don't care for a physically contigous buffer, so just return
    if(reqcont == 0)
        return *bufferCl;

    addr = (unsigned long long)buffer;
    addr = addr >> page_offset;
    addr = addr * PAGEMAP_ENTRY_SIZE;

    snprintf((char*)buffer,sizeof(char)*100,"/proc/%d/pagemap",getpid());
    if((fp = open((char*)buffer,O_RDONLY,NULL)) < 0) { perror("Opening /proc/../pagemap");exit(1);}
    if((fp2 = open("/proc/kpageflags",O_RDONLY,NULL)) < 0) {perror("Open");exit(1);}

    //Lookup part
    unsigned int idx = 1;
    if(lseek(fp,addr,SEEK_SET) < 0) {
        perror("Seek1:");
        exit(1);
    }
    for(idx=0;idx<newBufferSize/(1<<12);idx++){
        if(read(fp,&data,sizeof(data)) <= 0) {
            perror("read pagemap:");
            exit(1);
        }
        if(lseek(fp2,(data & (unsigned long long)0x000FFFFFFFFFFFFF)*8,SEEK_SET) < 0) {
            perror("Seek Flags:");
            exit(1);
        }
        if(read(fp2,&flags,sizeof(flags)) < 0) {
            perror("read kpageflags:");
            exit(1);
        }

        //Skip non hugepage heads
	    if(!(flags & (1 << KPF_COMPOUND_HEAD)))
            continue;

        prev = data & (unsigned long long)0x000FFFFFFFFFFFFF;   //PFN
        phys_to_virt[prev] = (unsigned long long)(buffer) + (unsigned long long)idx*(1 << 12);

#ifdef DEBUG_CREATE
        std::cout << idx << std::endl;
        std::cout << std::hex << prev << " -> " << phys_to_virt[prev] << std::endl;
        std::cout << std::dec;
        printf("PageMap: 0x%016llx\n",data);
        printf("Flags: 0x%016llx\n",flags);
	    if(flags & (1 << KPF_COMPOUND_HEAD))
	    	printf("COMPOUND_HEAD SET\n");
	    if(flags & (1 << KPF_COMPOUND_TAIL))
	    	printf("COMPOUND_TAIL SET\n");
	    if(flags & (1 << KPF_HUGE))
	    	printf("HUGE SET\n");
	    printf("\n");
#endif
    }

    close(fp);
    close(fp2);

    //Search for some number of contiguous large pages (assumed 2 MB)
    //need buffersize/(1<<21) pages to be contiguous
    int requiredPages = ceil((float)buffersize / (1U<<21));
    prev = 0x0;
    unsigned int nFound = 0;
    unsigned int nextOff = 0;
#ifdef DEBUG_CREATE
    std::cout << "Buffer base: 0x" << std::hex << (unsigned long long)buffer << std::dec << std::endl;
#endif
    std::map<unsigned long long, unsigned long long>::iterator lastGoodIt, it;
	std::map<unsigned long long, unsigned long long> found_pages;
    for(lastGoodIt = it = phys_to_virt.begin(); it != phys_to_virt.end(); it++){
        if((it->first == prev+(1U<<9))|| (it->first - prev) % (1U<<11) == 0) { //The distance from the last page we accepted must be in the 8MB congruence class
            nFound += 1;
            prev = it->first;
            found_pages[prev] = it->second;
        }
        /*if(it->first == prev+(0x200)) { //prev + 0x200 is 2 MB worth of 4 KB frames (0x200000)
            nFound += 1;
        }else{
            nFound = 1;
            lastGoodIt = it;
        }
        */
        //printf("0x%llx <- 0x%llx\n",it->first, it->second);
        if(nFound == requiredPages)
            break;
    }
    if(nFound < requiredPages) {
        std::cout << "No Contiguous range found" << std::endl;
        exit(1);
    }
    bufferCl->setContRange(found_pages.begin(),requiredPages);
    return *bufferCl;
}


Buffer& createBuffer(unsigned int buffersize, int reqcont) {
    int retry = 0;
    std::vector<unsigned char*> buffers_vector;
    Buffer *bufferCl = new Buffer(buffersize);
    unsigned long long startAddr = 0;
testlbl:
    int page_size = getpagesize();
    int page_offset = 0;
    unsigned long long page_mask = 0x1;
    unsigned long long newBufferSize = buffersize + (1U << 29); //Allocate an extra 512 MB
    if(reqcont == 0)
        newBufferSize = buffersize + page_size;

    unsigned char *buffer;
    unsigned long long addr;
    int fp;

    bufferCl->setBuffer((unsigned char*)malloc(newBufferSize),newBufferSize);
    buffer = bufferCl->getPtr();
    memset(buffer,0x0,newBufferSize);

    while(page_size!=1) {
        page_size /= 2;
        page_offset += 1;
        page_mask = (page_mask << 1) | 0x1;
    }
    page_mask = page_mask >> 1;
    page_size = getpagesize();
#ifdef DEBUG_CREATE
    std::cout << "PageSize / Offset: " << page_size << " / " << page_offset << std::endl;
#endif

    //Let's ensure that we start with a Page aligned section of memory
    if((unsigned long long)buffer % page_size == 0)
        bufferCl->setStartAddr(buffer);
    else 
        bufferCl->setStartAddr((unsigned char*)(((unsigned long long)(buffer) + page_size) & ~page_mask)); 

#ifdef DEBUG_CREATE
    std::cout << "Start Addr:" << std::hex << (void*)bufferCl->getStartAddr() << std::endl << std::dec;
#endif

    //We don't care for a physically contigous buffer, so just return
    if(reqcont == 0)
        return *bufferCl;


    addr = (unsigned long long)bufferCl->getStartAddr();
    addr = addr >> page_offset;
    addr = addr * PAGEMAP_ENTRY_SIZE;

    snprintf((char*)buffer,sizeof(char)*100,"/proc/%d/pagemap",getpid());
    if((fp = open((char*)buffer,O_RDONLY,NULL)) < 0) { perror("Opening /proc/../pagemap");exit(1);}

    unsigned int pages_in_buffer = newBufferSize / page_size;
    if(newBufferSize % page_size != 0) pages_in_buffer += 1; //Round up

    //Iterate over buffersize make sure it's contiguous in RAM
    int i;
    unsigned long long entry=0, lastPFN=0;
    bool good = true;
    int cont = 0;
    startAddr = (unsigned long long)bufferCl->getStartAddr();
    if(lseek(fp,addr,SEEK_SET) < 0 ) { perror("Seeking"); exit(1); }

    while(((unsigned long long)buffer + newBufferSize - 1) - (startAddr) >= buffersize && cont < buffersize){
        //Iterate until the window is too small
        //Keep shifting startAddr and check if the entry is contiguous
        //If it is, increase the cont counter
        //else, reset the cont counter and shift startAddr to start of next page (we want to stay page aligned)
        //Also break the while loop if cont == buffersize
#ifdef DEBUG_CREATE
        std::cout << (buffer + newBufferSize - 1) - (startAddr) << "/" << buffersize << std::endl;
#endif
        if(read(fp,&entry,sizeof(entry)) < 0) { perror("Reading Entry");exit(1); }  //Read the entry from /proc
#ifdef DEBUG_CREATE
        //std::cout << std::hex << (unsigned long long)(*startAddr)+cont << " -> " << entry << std::endl << std::dec;
#endif

        if(!(entry & PAGEMAP_MAPPED_MASK)) { 
            std::cerr << ("Page not mapped into memory") << std::endl;  //Ensure that the page is mapped
            startAddr = startAddr + cont + page_size;
            cont = 0;
            //exit(1); 
        } 
        else if(lastPFN == 0) {         //Corner case, start
            lastPFN = entry & PAGEMAP_PFN_MASK;    
            cont += page_size;
        }
        else if(lastPFN + 1 != (entry & PAGEMAP_PFN_MASK)) {      //The Physical address is not contiguous with the previous one      
            lastPFN = 0;
            startAddr = startAddr + cont + page_size;
            cont = 0;
        }
        else {
            lastPFN = entry & PAGEMAP_PFN_MASK;                        //Not above
            cont += page_size;
#ifdef DEBUG_CREATE
            std::cout << "Found " << cont << " contiguous bytes out of " << buffersize << std::endl;
#endif
        }
    }//while

    close(fp);
    if(cont < buffersize) {
        if(retry > 50)
            exit(1);
        retry += 1;
        std::cerr << "No contiguous range found" << std::endl;
        startAddr = (unsigned long long)buffer;
        std::cerr << "Trying again" << std::endl;
        buffers_vector.push_back(buffer);
        goto testlbl;
    }
    std::cerr << "Found buffer" << std::endl;
#ifdef DEBUG_CREATE
    std::cout << std::hex << "Buffer starts at: " << (unsigned long long)buffer << std::dec << std::endl;
    std::cout << std::hex << "Buffer: [" << (unsigned long long)startAddr << "-" << (unsigned long long)startAddr + buffersize
        << "] " << std::dec << std::endl;
#endif
    for(std::vector<unsigned char*>::iterator it=buffers_vector.begin();it!=buffers_vector.end();it++)
        free(*it);
    return *bufferCl;
}

void Buffer::dumpFrames(std::string filename) {
    char buffer[100];
    int fp;
    FILE *dumpFile;
    unsigned int page_size = getpagesize();
    unsigned int page_offset = 0x0 ,page_mask = 0x0;

    unsigned int pages_in_buffer = this->actualSize / page_size;
    if(this->actualSize % page_size != 0) pages_in_buffer += 1; //Round up

    //Iterate over buffersize make sure it's contiguous in RAM
    int i;
    unsigned long long entry=0;
    unsigned long long addr;
    unsigned long long startAddr = (unsigned long long)getStartAddr();

    while(page_size!=1) {
        page_size /= 2;
        page_offset += 1;
        page_mask = (page_mask << 1) | 0x1;
    }
    page_mask = page_mask >> 1;
    page_size = getpagesize();

    addr = startAddr;
    addr >>= page_offset;
    addr *= PAGEMAP_ENTRY_SIZE;

    snprintf((char*)buffer,sizeof(char)*100,"/proc/%d/pagemap",getpid());
    if((fp = open((char*)buffer,O_RDONLY,NULL)) < 0) { perror("Opening /proc/../pagemap");exit(1);}

    if(lseek(fp,addr,SEEK_SET) < 0 ) { perror("Seeking in dumpFrames"); exit(1); }

    std::map<unsigned int,unsigned int> pfn_map;
    unsigned int offset = 0;
    unsigned int PFNa = 0;

    while(offset < pages_in_buffer) {
        if(read(fp,&entry,sizeof(entry)) < 0) { perror("Reading Entry");exit(1); }  //Read the entry from /proc

        if(!(entry & PAGEMAP_MAPPED_MASK)) { 
            std::cerr << ("Page not mapped into memory") << std::endl;  //Ensure that the page is mapped
        }

        PFNa = entry & PAGEMAP_PFN_MASK;
        pfn_map[addr+offset] = PFNa;
        offset += 1;
    }

    close(fp);

    if((dumpFile = fopen(filename.c_str(),"a+")) == NULL) {perror("Creating frames dump file");exit(1);}
    //First write the start of the buffer
    unsigned int numArgs = pfn_map.size();
    if(fwrite((void*)&numArgs,sizeof(unsigned int),1,dumpFile) < 0) {}
    for(std::map<unsigned int,unsigned int>::iterator it=pfn_map.begin();it != pfn_map.end(); it++)
        if(fwrite((void*)&(it->second),sizeof(unsigned int),1,dumpFile) < 0){
            perror("Writing the frames dump"); exit(1);
        }
    fclose(dumpFile);

}

void Buffer::offsetBuffer(int offset) {
    this->setStartAddr(this->getStartAddr() + offset);
}

void Buffer::setContRange(std::map<unsigned long long, unsigned long long>::iterator it, unsigned int pages) {
    this->setStartAddr((unsigned char*)(it->second));  
    for(;pages > 0;pages--) {
        this->slabs.push_back(it->second); //Push the virtual address
        it++;
    }
}

Buffer::~Buffer() {
    if(huge) {
        munmap(theMemory,actualSize);
    }else{
        delete [] theMemory;
    }
}

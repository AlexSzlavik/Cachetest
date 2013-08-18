#include "Distribution.h"

Distribution* 
Distribution::createDistribution(Distribution::TYPE t,
                                Buffer *buffer,
                                int cacheline,
                                size_t element_size,
                                int seed)
{
    Distribution *ptr = NULL;
    switch (t) {
        case UNIFORM:
            ptr = new UniformDistribution;
            break;;
        case ZIPF:
            ptr = new ZipfDistribution;
            break;;
        case LINEAR:
            ptr = new LinearDistribution;
            break;;
        case WUNI:
            ptr = new WeightedUniform;
            break;;
        default:
            return NULL;
            break;;
    }
 
    ptr->setup(buffer,element_size,seed,cacheline);
    return ptr;
}

Distribution::Distribution() {
    this->buffer = NULL;
    this->entries = 0;
}

void Distribution::distribute() {
    this->doDistribute();
}

void Distribution::setup(Buffer *buffer, size_t esize, int seed, int cacheline) {
    this->buffer = buffer;
    //this->buffersize = size;
    this->element_size = esize;
    this->seed = seed;
    this->cacheline = cacheline;
    this->num_elements = buffer->Get_size()/ element_size;
    this->num_elements_in_cacheline = cacheline / element_size;
    this->entries = 0;
}

unsigned int Distribution::getEntries() {
    return this->entries;
}

unsigned int Distribution::getNumElements(){
    return this->num_elements;
}

double Distribution::getBufferUtilization() {
    return (double)entries / num_elements;
}

//Dump the contents of our buffer to fd
void Distribution::dumpBuffer(int fd) {
    if(write(fd,buffer->Get_buffer_pointer(),buffer->Get_allocated_size()) < 0) {
        perror("Dumping Buffer");
    }
}

void Distribution::dumpSequence(int fd) {
    std::vector<unsigned int>::iterator it = this->sequence.begin();
    for(;it!=sequence.end();it++) {
        if(write(fd,&(*it),sizeof(unsigned int)) < 0 ) {
            perror("Dumping Sequence");
        }
    }
}
///End Base Class

void ZipfDistribution::setParameters(double alpha, unsigned int items) {
    if(items == 0)
        return; //Need an error here
    this->alpha = alpha;
    this->num_items = items;
}

//Actual implementations of Distributions
//
void LinearDistribution::doDistribute() {
    int datalines = buffer->Get_size()/ cacheline;
    unsigned int offset = buffer->Get_start_address() - buffer->Get_buffer_pointer();
    this->sequence.clear();
    for ( int i = 0; i < datalines; i += 1 ) {
        *(int*)(buffer->Get_start_address() + i * cacheline) = ((i + 1) % datalines) * cacheline + offset;
        this->sequence.push_back((i+1)%datalines);
        entries += 1;
    }
}

void UniformDistribution::doDistribute() {
    int bufferlines = buffer->Get_size()/ cacheline;
    int num_line_elements = cacheline / element_size;
    int offset;
    int element = 0;
    int previdx = buffer->Get_start_address() - buffer->Get_buffer_pointer();
    bool done = false;
    this->sequence.clear();
    MTRand mr(seed);

    if(buffer->Is_large_buffer() && buffer->Get_slabs().size() > 0)
        return this->doHugeDistribution();

    /* New algorithm:
	 * Init the access array, such that we randomly sample a new place to go to,
	 * but each entry has cacheline / sizeof(int) entries. This is sort of 
	 * analogous to buckets in hashing. When the front entry is full, find 
	 * one empty one in the next 15 entries. If all of them are full, 
	 * remember that this occured and draw a new number element.
	 * Then the actual memory traversal should simply follow the pointers again
	 * and all is swell
	 */
    for ( int i = 0; i < num_elements; i += 1 ) {
        int idx = mr.randInt() % bufferlines;
        offset = 0;
        while (true) {
            element = idx * cacheline + offset*element_size + (buffer->Get_start_address() - buffer->Get_buffer_pointer());
            if(*(element_size_t*)(buffer->Get_buffer_pointer() + element)) {
                offset += 1;
            }else{
                break;
            }
            if(offset == num_line_elements - 1) { done=true; break; }
        }
        if(done) break;
        this->sequence.push_back(idx);
        *(element_size_t*)(buffer->Get_buffer_pointer() + previdx) = element;
        previdx = element;
        entries += 1;
    }
    *(int*)(buffer->Get_buffer_pointer() + previdx) = buffer->Get_start_address() - buffer->Get_buffer_pointer();
}

//Mainly duplicate from above.
//For now we assume that a huge page can only be
//2MB (1 << 21). This assumption is also hardcoded 
//inside Buffer.cc
//The main difference is that we want to distribute across multiple pages, 
//but treat them as though they were contiguous. So we need to do some fancy 
//address translation
void UniformDistribution::doHugeDistribution() {
    unsigned int bufferlines = buffer->Get_size()/ cacheline;
    unsigned int num_line_elements = cacheline / element_size;
    unsigned int num_lines_on_page = ((1 << 21) / (cacheline));
    int offset;
    int element = 0;
    unsigned long long  previdx = 0;
    bool done = false;
    MTRand mr(seed);
    this->sequence.clear();

    previdx = buffer->Get_start_address() - buffer->Get_buffer_pointer();

    assert((buffer->Get_slabs()).size() > 0);    //Ensure that the slabs have been setup
    for ( int i = 0; i < num_elements; i += 1 ) {
        int idx = mr.randInt() % bufferlines;
        this->sequence.push_back(idx);
        unsigned int page = idx / num_lines_on_page; //Find the logical page idx mod (2 MB/64 b)
        unsigned int pageIDX = idx - page*num_lines_on_page; //Offset into page (line)
        unsigned long long virtPageStart = buffer->Get_slabs()[page];
        unsigned long long virtBufferStart = (unsigned long long)buffer->Get_buffer_pointer();

        offset = 0;
        while (true) {
            element = (virtPageStart-virtBufferStart) + pageIDX*cacheline + offset*element_size;
            if(*(element_size_t*)(buffer->Get_buffer_pointer() + element)) {
                offset += 1;
            }else{
                break;
            }
            if(offset == num_line_elements - 1) { done=true; break; }
        }
        if(done) break;
        *(element_size_t*)(buffer->Get_buffer_pointer() + previdx) = element;
        previdx = element;
        entries += 1;
    }
    *(int*)(buffer->Get_buffer_pointer() + previdx) = (buffer->Get_start_address()-buffer->Get_buffer_pointer());
}

/*
 * Zipf Distribution
 * Algorithm: Seed Zipf.
 * Distribute memory slots uniform random (create ranks)
 * Then apply Zipf distribution amongst the slots and create
 * linked list of access pattern. 
 */
void ZipfDistribution::doDistribute() {
    int offset;
    MTRand mr(seed);

    if(init_zipf(this->seed) < 0){
        std::cerr << "Seeding Zipf failed" << std::endl;
        exit(1);
    }

    //Randomize the slots
    //Doing Sattolo's Alrgorithm for an in-place shuffle of an array
    //This is the Fisher-Yates shuffler
    int *slots = new int[num_elements]; //This is the rank array
    int i=0;
    for(i=0;i<num_elements;i++) slots[i]=0;
    i = num_elements;
    while (i > 0) {
        i -= 1;
        int j = mr.randInt(i-1);
        int tmp = slots[j];
        slots[j]=slots[i];
        slots[i]=tmp;
    }

    //The element at idx in rank array is the bufferslot of rank idx
    //
    //Now insert the zipf pattern

    /*for ( int i = 0; i < num_elements; i += 1 ) {
        int idx = mr.randInt() % bufferlines;
        offset = 0;
        while (true) {
            element = idx * cacheline + offset*element_size;
            if(*(element_size_t*)(buffer + element)) {
                offset += 1;
            }else{
                break;
            }
            if(offset == num_line_elements - 1) { done=true; break; }
        }
        if(done) break;
        *(element_size_t*)(buffer + previdx) = element;
        previdx = element;
        entries += 1;
    }
    *(int*)(buffer + previdx) = 0;*/
}

void WeightedUniform::calculateCDF() {
    unsigned long long running_total = 0;

    for(int i=0;i<buffer->Get_size()/cacheline;i++){
        running_total += i;
        cdf.push_back(running_total);
    }

    std::sort(cdf.begin(),cdf.end());
    maxValinCDF = running_total;
}

/*
 * Draw a number from the weighted distribution
 * via binary search on the slots
 */
unsigned int WeightedUniform::getNextIDX(MTRand &mr) {
    unsigned res = 0;
    int imax = cdf.size();
    int imin = 0;
    unsigned long long key = mr.randExc()*maxValinCDF;

    //Binary search for slot
    int max,min,mid;
    max = cdf.size();
    min = 0;
    while(min<max) {
        mid = ((max + min)/2);
        if(key < cdf[mid])
            max = mid;
        else
            min = mid+1;
    }
    return mid;
}

void WeightedUniform::doDistribute() {
    int bufferlines = buffer->Get_size()/ cacheline;
    int num_line_elements = cacheline / element_size;
    int offset;
    int element = 0;
    int previdx = buffer->Get_start_address() - buffer->Get_buffer_pointer();
    bool done = false;
    this->sequence.clear();
    MTRand mr(seed);

    if(buffer->Is_large_buffer() && buffer->Get_slabs().size() > 0)
        assert(false);

    calculateCDF();

    /* New algorithm:
	 * Init the access array, such that we randomly sample a new place to go to,
	 * but each entry has cacheline / sizeof(int) entries. This is sort of 
	 * analogous to buckets in hashing. When the front entry is full, find 
	 * one empty one in the next 15 entries. If all of them are full, 
	 * remember that this occured and draw a new number element.
	 * Then the actual memory traversal should simply follow the pointers again
	 * and all is swell
	 */
    for ( int i = 0; i < num_elements; i += 1 ) {
        unsigned idx = getNextIDX(mr);
        assert(idx < bufferlines);
        offset = 0;
        while (true) {
            element = idx * cacheline + offset*element_size + (buffer->Get_start_address() - buffer->Get_buffer_pointer());
            if(*(element_size_t*)(buffer->Get_buffer_pointer() + element)) {
                offset += 1;
            }else{
                break;
            }
            if(offset == num_line_elements - 1) { done=true; break; }
        }
        if(done) break;
        this->sequence.push_back(idx);
        *(element_size_t*)(buffer->Get_buffer_pointer() + previdx) = element;
        previdx = element;
        entries += 1;
    }
    *(int*)(buffer->Get_buffer_pointer() + previdx) = buffer->Get_start_address() - buffer->Get_buffer_pointer();
}

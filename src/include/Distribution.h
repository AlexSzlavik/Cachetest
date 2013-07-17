#include <stdlib.h>
#include "MersenneTwister.h"
#include "zipf.h"
#include "Buffer.h"
#include <algorithm>
#include <vector>

typedef unsigned int element_size_t;

class Distribution {
    private:
    protected:
        int cacheline;
        size_t element_size;
        int seed;
        Buffer *buffer;
        std::vector<unsigned int> sequence;

        unsigned int entries, num_elements, num_elements_in_cacheline;
    public:
        enum TYPE {UNIFORM, ZIPF, LINEAR, WUNI};
        
        //Factory method
        static Distribution* createDistribution(TYPE t,Buffer* buffer , int cacheline, size_t element_size, int seed); 

        void setup(Buffer*,size_t,int,int cacheline);
        void distribute();  //Public start point

        Distribution();
        virtual ~Distribution() {}

        virtual void doDistribute() = 0;    //Implement this
        unsigned int getEntries();  //Retrieve the # of entries created
        unsigned int getNumElements();  //Retrieve the # of entries created
        double getBufferUtilization();  //Retrieve the # of entries created
        void dumpBuffer(int dumpfd);    //Dump the data to the given file
        void dumpSequence(int dumpfd);  //Dump the generated sequence
};

class UniformDistribution : public Distribution {
    private:
        void doHugeDistribution();
    public:
        virtual void doDistribute();
};

class WeightedUniform : public Distribution {
    public:
        WeightedUniform() { weightFactor = 1;}
        virtual void doDistribute();
    private:
        unsigned int weightFactor; //Two connected items are 1 unit of probability apart
        std::vector<unsigned long long> cdf;
        unsigned long long maxValinCDF;

        void calculateCDF();    //Precomputes the distribution probabilities for WUNI
        unsigned int getNextIDX(MTRand &);
};

class ZipfDistribution: public Distribution {
    private:
        double alpha;
        unsigned int num_items;
    public:
        ZipfDistribution() { alpha = 0; num_items = 0;}
        void setParameters(double alpha, unsigned int num_items);
        virtual void doDistribute();
};

class LinearDistribution: public Distribution {
    public:
        virtual void doDistribute();
};


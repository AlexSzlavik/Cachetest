#include <vector>
#include <Perf.hpp>

struct Options
{
    unsigned int            bufferfactor;       /*Buffer factor*/
    unsigned int            duration;           /*Experiment Duration in seconds*/
    unsigned int            loopfactor;         /*Additional loops*/
    unsigned int            cacheline;          /*Size of cachelines*/
    unsigned int            bufferoffset;       /*Start experiment at buffer offset*/
    unsigned int            dataset;            /*Size of the dataset*/

    int                     seed;               /*RNG Seed*/
    int                     cpu;                
    
    bool                    cont;               /*Contiguous memory allocation*/
    bool                    huge;               /*Use Hugepages*/
    bool                    append;             /*Append data to output files, rather than overwrite*/
    bool                    wait;               /*Synchronized execution*/
    bool                    dumpSeq;            /*Dump the generated access pattern*/

    std::vector<Event>      events;             /*Vector for Custom Perf Events*/

    Options()
    {
        bufferfactor    = 1;
        duration        = 1;
        loopfactor      = 1;
        cacheline       = 64;
        seed            = -1;
        cpu             = -1;
        cont            = false;
        huge            = false;
        append          = false;
        wait            = false;
        dumpSeq         = false;
    } 
};

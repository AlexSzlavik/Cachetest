#ifndef __PERF_PAPI_H__
#define __PERF_PAPI_H__
#include <Perf.h>
#include <papi.h>
#include <string>
#include <sstream>

#define MAX_PERF_COUNTERS   4

class Perf_PAPI : public Perf
{
    private:
        int         Event_set;
        Result_t*   result_vector;
        uint32_t    available_counters;

        bool        running;

        bool        error( std::string, int errorCode );

    public:
        Perf_PAPI();

        bool        addEvent( std::string event );

        bool        start();
        bool        stop();

        bool        read_results( std::vector<Result_t> & );
};

#endif

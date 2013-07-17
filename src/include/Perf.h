#ifndef __A_PERF_H__
#define __A_PERF_H__

#include <sys/types.h>
#include <asm/unistd.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <linux/perf_event.h>
#include <string.h>
#include <sys/ioctl.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <stdint.h>
#include <string>

#include <config.h>

extern std::ostream*     error_out;
extern std::ostream*     output;

typedef long long Result_t;

typedef struct {
    unsigned int eventid;
    unsigned int unitmask;

    std::string string_event;
} Event;

template <class T>
bool from_string(T& t, const std::string& s, std::ios_base& (*f)(std::ios_base&))
{
    std::istringstream iss(s);
    return !(iss >> f >> t).fail();
}

class Perf {
    private:
        static Perf*            instance;

    protected:

    public:
        Perf();

        virtual std::vector<Event>*  		parseEvents( char *str );

        virtual bool                addEvent( uint64_t rawCode ) { return false; }
        virtual bool                addEvent( uint32_t event_id, uint32_t unit_mask ) { return false; }
        virtual bool                addEvent( std::string event ) { return false; }
        virtual bool                addEvent( Event event ) { return false; }

        virtual bool                start()     = 0;
        virtual bool                stop()      = 0;

        virtual bool                read_results( std::vector<Result_t> & ) { return false; }

        static Perf *               get_instance( void );
};

#endif

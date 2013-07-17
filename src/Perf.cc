#include <Perf.h>

#ifdef HAVE_PAPI_H
#include "Perf_PAPI.h"
#else
#include "Perf_Cachetest.h"
#endif

Perf* Perf::instance = NULL;

Perf* Perf::get_instance( void ) 
{
    if(instance == NULL)
    {
#ifdef HAVE_PAPI_H
        instance = new Perf_PAPI();
#else
        instance = new Perf_Cachetest();
#endif
    }

    return instance;
}

Perf::Perf( ) 
{
}

std::vector<Event>*
Perf::parseEvents(char* str)
{
    char* idx   = NULL;
    std::vector<Event>* Events = new std::vector<Event>;
    while((idx = strtok(str, ",")) != NULL)
    {
        str = NULL;

        Event new_event;
        new_event.string_event.assign(idx);
        Events->push_back(new_event);
    }

	if ( Events->size() != 0 )
	    return Events;
	else
	{
		delete Events;
		return NULL;
	}
}

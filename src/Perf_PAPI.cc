#include "Perf_PAPI.h"

Perf_PAPI::Perf_PAPI()
{
    Event_set = PAPI_NULL;

    PAPI_library_init(PAPI_VER_CURRENT);

    available_counters = PAPI_num_counters();

    PAPI_create_eventset( &Event_set );

	result_vector = NULL;

    running = false;
}

bool
Perf_PAPI::addEvent( Event event )
{
	return addEvent( event.string_event );
}

bool
Perf_PAPI::addEvent( std::string event ) 
{
    int event_id = PAPI_NULL;
    char* event_string = (char*)event.c_str();
    int return_code;

    if( return_code = PAPI_event_name_to_code( event_string, &event_id ) != PAPI_OK )
    {
        error("Adding Event", return_code );
        return false;
    }

    if ( return_code = PAPI_add_event(Event_set, event_id ) != PAPI_OK )
    {
        error("Adding Event", return_code );
        return false;
    }

    return true;
}

bool
Perf_PAPI::start()
{
    int return_code     = PAPI_NULL;

    if( PAPI_num_events( Event_set ) == 0 )
        return false;

    return_code = PAPI_start(Event_set);
    if(error("Starting Counter",return_code))
        exit(1);

	running = true;

    return true;
}

bool
Perf_PAPI::stop()
{
	unsigned int number_of_results = 0;
	number_of_results = available_counters < PAPI_num_events(Event_set) ? available_counters : PAPI_num_events(Event_set);
    if(result_vector == NULL && running && number_of_results)
    {
        result_vector = new Result_t[number_of_results];
        PAPI_stop(Event_set, result_vector);
    }
	else
	{
		return false;
	}

    running = false;

    return true;
}

bool
Perf_PAPI::read_results( std::vector<Result_t>&  result_vec )
{
	unsigned int number_of_results = 0;
	number_of_results = available_counters < PAPI_num_events(Event_set) ? available_counters : PAPI_num_events(Event_set);
    for(int counter=0;counter<number_of_results;++counter)
        result_vec.push_back(result_vector[counter]);

    return true;
}

bool
Perf_PAPI::error( std::string msg, int errorCode )
{
    std::stringstream ss;

    ss << "PAPI: " << msg << ": ";

    switch( errorCode )
    {
        case PAPI_OK:
            return false;
            break;
        case PAPI_EINVAL:
            ss << "Invalid argument";
            break;
        case PAPI_ESYS:
            ss << "System or C library error inside PAPI";
            break;
        case PAPI_ENOEVST:
            ss << "The EventSet specified does not exist";
            break;
        case PAPI_EISRUN:
            ss << "The EventSet is already counting events";
            break;
        case PAPI_ECNFLCT:
            ss << "Counter conflict, cannot count these events simultaneously";
            break;
        case PAPI_ENOEVNT:
            ss << "The PAPI preset is not available on the underlying hardware";
            break;
        default:
            ss << "Unknown error code";
            break;
    }

    ss << std::endl;
    *error_out << ss.str();

    return true;
}


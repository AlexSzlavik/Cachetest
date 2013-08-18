void		Initialize( void );
void        pinExperiment( int cpu );
bool        Parse_options( int, char * const *, Options &);
bool        Configure_experiment( void );
bool        Setup_buffer( void );
bool        Setup_perf( void );
bool        Setup_distribution( void );
bool        Synchronize_instances( void );
bool        Setup_timeout( void );
bool        Perform_experiment( void );
bool        Dump_results( void );

static void usage( const char *);
static void alarm_handler( int );
static void start_handler( int );

struct Result_vector_t
{
    Result_t    Measured_iterations;
};

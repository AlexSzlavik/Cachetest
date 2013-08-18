//Author:	Alex Szlavik
//Perf_Cachetest.cc

#include <Perf_Cachetest.hpp>

Perf_Cachetest::Perf_Cachetest() {
    fd_leader = -1;
}

Perf_Cachetest::~Perf_Cachetest() {
    std::vector<int>::iterator it = fds.begin();
    for(;it!=fds.end();it++)
        close(*it);
}
//
//bool 
//Perf_Cachetest::addEvent(__u64 rawCode) {
//    struct perf_event_attr attr;
//    memset(&attr,0x0,sizeof(attr));
//    attr.type = PERF_TYPE_RAW;
//
//    attr.config = rawCode;
//}

bool 
Perf_Cachetest::addEvent(__u32 eventId, __u32 UnitMask) {
    struct perf_event_attr attr;
    int fd;
    memset(&attr,0x0,sizeof(attr));
    attr.type = PERF_TYPE_RAW;
    attr.exclude_kernel = 1;
    attr.config |= (1<<16);

#if (defined __x86_64__ || defined __IA64__)
    attr.config |= ((uint64_t)eventId << 24ULL) & 0xF00000000ULL;
    attr.config |= (eventId & 0xFF);
    attr.config |= (UnitMask << 8) & 0xFF00;
#elif __powerpc__
    attr.config |= eventId;
#else
    #error : Arch not supported by Perf implementation
#endif

    if((fd = perf_event_open(&attr,0,-1,fd_leader,0)) < 0) {
        perror("Opening perf");
        exit(1);
    }

    if(fd_leader < 0)
        fd_leader = fd;

    fds.push_back(fd);

    return true;
}

bool 
Perf_Cachetest::addEvent(Event event) {
    return addEvent(event.eventid, event.unitmask);
}

bool 
Perf_Cachetest::addEvents(std::vector<Event> &event) {
    std::vector<Event>::iterator it;
    for(it=event.begin();it!=event.end();it++)
        if(!addEvent(*it)) return false;
    return true;
}

bool
Perf_Cachetest::start() {
    if(fd_leader > 0)
	{
        ioctl(fd_leader,PERF_EVENT_IOC_RESET, 0 );
        ioctl(fd_leader,PERF_EVENT_IOC_ENABLE, 0 );
	}
    else
        return false;
	return true;
}

bool
Perf_Cachetest::stop() {
    if(fd_leader > 0)
        ioctl(fd_leader,PERF_EVENT_IOC_DISABLE);
    else
        return false;
	return true;
}

int 
Perf_Cachetest::perf_event_open(struct perf_event_attr *attr, pid_t pid, int cpu, int group_fd, unsigned long flags) {
    attr->size = sizeof(*attr);
    return syscall(__NR_perf_event_open, attr, pid, cpu,group_fd, flags);
}

bool
Perf_Cachetest::read_results( std::vector<Result_t> & results )
{
    std::vector<int>::iterator it = fds.begin();
    for(;it!=fds.end();it++){
        unsigned long long data;
        if(read(*it,&data,sizeof(data)) < 0) {
            perror("Reading Counter");
            return false;
        }
        results.push_back(data);
    }
    return true;
}

int 
Perf_Cachetest::getEventFd(int idx) {
    return fds[idx];
}

/*
 * void parseEvents method
 * This method parses the passed in events string (due to -e)
 * and returns an array of events and unit masks.
 * Two formats are acceptable: rx0000UUxx, where xxx is the 
 * 12 bit event id and UU is the 8 bit unti mask.
 * The second format is -r xxx,uUU where xxx and uu are the same as before.
 * In this format we use the u prefix to pass shorter strings.
 */
std::vector<Event>*
Perf_Cachetest::parseEvents(char *string) {
    char *idx;
    int num_events = 0;
    unsigned long long code;
	std::vector<Event>* events = new std::vector<Event>;

    while((idx = strtok(string, ",")) != NULL) {    //split into individual events
        string=NULL;
        skip:
        if((num_events += 1) > 4) {
            std::cerr << "We don't want more then 4 events simultaneously\nThis leads to multiplexing" << std::endl;
            std::cerr << "Exiting" << std::endl;
            exit(1);
        }
        if(*idx == 'r') { //Method 1
            std::string str(idx+1);
            if(!from_string<unsigned long long>(code,str,std::hex)) {
                std::cerr << idx << " is an ill-formed event id string" << std::endl;
                exit(1);
            }
            Event event;
#if (defined __x86_64__ || defined __IA64__)
            event.unitmask = (code >> 8) & 0xFF;
            event.eventid = (code >> 32) & 0xF;
            event.eventid = (event.eventid << 8) | (0xFF & code);
            events->push_back(event);
#elif __powerpc__
            event.eventid = code;
    	    events->push_back(event);
#else
    #error : Arch not supported by Perf implementation
#endif

#ifdef DEBUG            
            std::cout << code << std::endl;
            std::cout << std::hex << event.eventid << std::endl;
            std::cout << std::hex << event.unitmask << std::endl;
#endif
        } 
        else {      //Method 2
            Event event;
            std::string str(idx);
            if(!from_string<unsigned int>(event.eventid,str,std::hex)){
                std::cerr << idx << " is an ill-formed event id string" << std::endl;
                exit(1);
            }

            bool cont = true;
            idx = strtok(string, ",");
            if(idx == NULL) { events->push_back(event);break;}
            if(*idx != 'u') cont = false;
            else {
                std::string maskStr(idx+1);
                if(!from_string<unsigned int>(event.unitmask,maskStr,std::hex)){
                    std::cerr << idx << " is an ill-formed event id string" << std::endl;
                    exit(1);
                }
            }
            events->push_back(event);
#ifdef DEBUG            
            std::cout << "Method 2" << std::endl;
            std::cout << std::hex << event.eventid << std::endl;
            std::cout << std::hex << event.unitmask << std::endl;
#endif
            if(!cont) goto skip;
        }
    }

	if( events->size() != 0 )
	    return events;
	else
	{
		delete events;
		return NULL;
	}
}


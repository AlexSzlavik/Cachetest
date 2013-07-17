#ifndef __PERF_CACHETEST_H__
#define __PERF_CACHETEST_H__
#include <Perf.h>

class Perf_Cachetest : public Perf
{
    private:
        int fd_leader;
        std::vector<int> fds;
        std::vector<Result_t> results;

        int perf_event_open(struct perf_event_attr *attr,pid_t pid, 
                        int cpu, int group_fd,unsigned long flags);
    public:
        Perf_Cachetest();
        ~Perf_Cachetest();

        bool 							addEvent(__u64 rawCode);
        bool 							addEvent(__u32 eventId, __u32 UnitMask);
        bool 							addEvent(Event event);
        bool 							addEvents(std::vector<Event> &events);
        bool 							start();       //Start counting
        bool							stop();        //Stop counting

        void 							setCPU(int cpuMask);

        std::vector<Result_t>& 			getResults();
        int 							getEventFd(int);   //Return the Fd for the group leader

		std::vector<Event>* 			parseEvents(char *string);

        bool                            read_results(std::vector<Result_t>&);
};

#endif

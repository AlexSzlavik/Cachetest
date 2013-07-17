#include <unistd.h>
#include <string.h>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sched.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <cstdio>

#define INCREASE_AMOUNT -10
#define MAX_CPUS 3

static cpu_set_t *cpuset;

static void start_handler(int) {
}

static void usage(const char *program) {
    std::cerr << "Usage: " << program << " [Options] -- <program path> [program options] " << std::endl;
    std::cerr << "This tool requires privileged proccess status" << std::endl;
    exit(1);
}

static void pinExperiment(std::vector<int> &cpus) {
    size_t size;

    //FIXME this shouldn't be hardcoded here?
    cpuset = CPU_ALLOC(MAX_CPUS);
    if(cpuset == NULL) {
        perror("CPU_ALLOC");
        exit(EXIT_FAILURE);
    }

    size = CPU_ALLOC_SIZE(MAX_CPUS);

    CPU_ZERO_S(size, cpuset);
    for(std::vector<int>::iterator it=cpus.begin(); it != cpus.end(); it++) {
        //std::cerr << "CPUS are: " << *it << std::endl;
        CPU_SET_S(*it, size,cpuset);
    }

    if(sched_setaffinity(getpid(),size,cpuset) == -1) {
        perror("Setting affinity");
    }
}

//Entry
int main ( int argc, char ** argv) {

    char **execv_args;
    std::vector<int> cpus;
    cpus.push_back(1);

    for (;;) {
        int option = getopt (argc, argv, "ahc:");
        if ( option < 0 ) { break; }
            switch(option) {
                case 'a': break;
                case 'h': usage (argv[0]); break;;
                case 'c': {
                        std::string ss(optarg);
                        cpus.clear();
                        int last=0;
                        int next=0;
                        cpus.push_back(atoi(optarg));
                        //while((next=ss.find_first_of(",",last)) != std::string::npos){
                        //    cpus.push_back(atoi(ss.substr(next-last,next).c_str()));
                        //    last=next+1;
                        //}
                        break;;
                    }
                case '?': 
                    std::cerr << "Unknown option: " << optopt << std::endl; 
                    usage(argv[0]); 
                    break;
                default: usage (argv[0]); break;
            }
    }

    if ( optind == argc ) {
        std::cerr << "Need application to launch" << std::endl;
        usage(argv[0]);
    }
    execv_args = (char**)malloc((argc - optind + 2)*sizeof(char*));
    memset(execv_args, 0x0, sizeof(execv_args));
    for (int index = optind; index < argc; index ++) {
        //std::cout << "Non-opt option: " << argv[index] << std::endl;
        execv_args[index - optind] = argv[index];
    }

    pid_t pid_ret = fork();
    if (pid_ret == 0) {    //child
        ////Change scheduler at this point
        struct sched_param param;
        param.sched_priority = sched_get_priority_max(SCHED_FIFO);
        int ret = sched_setscheduler(getpid(), SCHED_FIFO, &param);
        if(ret != 0) {
            perror("sched_setscheduler");
            exit(1);
        }

        //Pin the experiment
        pinExperiment(cpus);

        //Move everything else off the CPU we are testing
        //NOTE: Done in script at this time

        //Renice
        if (nice(INCREASE_AMOUNT) == -1) {
            perror("nice");
            exit(1);
        }
        
        //Launch the actual task
        execv(execv_args[0], execv_args);
        

    }else{    //Parent
        int child_ret;
        //signal(SIGUSR1, start_handler);
        //pause();
        //kill(pid_ret, SIGUSR1);
        waitpid(pid_ret, &child_ret, 0);

        return child_ret;
        //Change scheduler back, no need
        //Undo pinnings?, no need
    }

    return 0;
}


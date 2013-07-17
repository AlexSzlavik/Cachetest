verify_constraints() {
    #Pick at least one of -s -r
    if [ -z "$TESTTYPE" ]; then
        echo "ERROR: Must pick random or sequential access" >&2
        usage
        clean_exit 1
    fi
    if [ -z "$from" ]; then
        echo "ERROR: Must specify default cache level or custom memory size range" >&2
        usage
    fi
    if [ -z "$line" ]; then    #Need a line size
        echo "ERROR: No Line Size or Cache Level provided" >&2
        usage
    fi

}

#Function to dump all configuration parameters to RESDIR
function dump_config() {

    #Get the system info and dump it
    #$(dirname $0)/systeminfo.sh ${RESDIR}/${SYS_DUMP_FILE}
    lscpu &> ${RESDIR}/${SYS_DUMP_FILE}
    
    #Dump the execution parameters
    echo "Hostname: `hostname`" >> ${RESDIR}/${CONFIG_DUMP_FILE}
    echo "Scan Type: $TESTTYPE" >> ${RESDIR}/${CONFIG_DUMP_FILE}
    echo "Memory Range: $from - $to" >> ${RESDIR}/${CONFIG_DUMP_FILE}
    echo "Memory Step: $incr" >> ${RESDIR}/${CONFIG_DUMP_FILE}
    echo "Line Size: $line" >> ${RESDIR}/${CONFIG_DUMP_FILE}
    echo "Buffer Size: $buffersize" >> ${RESDIR}/${CONFIG_DUMP_FILE}
    echo "Iterations per test: $iterations" >>${RESDIR}/${CONFIG_DUMP_FILE}
    echo "Execution Time: $duration" >> ${RESDIR}/${CONFIG_DUMP_FILE}
    echo "Nice Level: $NICECMD" >> ${RESDIR}/${CONFIG_DUMP_FILE}
    echo "Loopfactor: $loopfactor" >> ${RESDIR}/${CONFIG_DUMP_FILE}
    echo "Writeback: $write_set" >> ${RESDIR}/${CONFIG_DUMP_FILE}
    echo "Physically Con: $PHYS_CONT" >> ${RESDIR}/${CONFIG_DUMP_FILE}
    echo "Huge Pages: $HUGE" >> ${RESDIR}/${CONFIG_DUMP_FILE}
    
    #SHA1 Will sum up configuration details
    echo "SHA1: `sha1sum ${RESDIR}/${CONFIG_DUMP_FILE}`" >> ${RESDIR}/${CONFIG_DUMP_FILE}
    #Now we echo the stuff that shouldn't be in the SHA1 Hash
    echo "Number of CPUs used: ${#CPUIDS[@]}" >> ${RESDIR}/${CONFIG_DUMP_FILE}
    echo "CPUIDs used: ${CPUIDS[@]}" >> ${RESDIR}/${CONFIG_DUMP_FILE}
    echo "Experiment Name: $expname" >> ${RESDIR}/${CONFIG_DUMP_FILE}
    echo "CPU Frequency: $FREQUENCY" >> ${RESDIR}/${CONFIG_DUMP_FILE}

}


#Setup CPUs
function configure_cpus() {

    #cpu settings 
    #First pin all IRQs to the "boot" cpu
    source cpuconf.sh
    set_IRQ_core $IRQ_CORE_ID
    if [ $? -ne 0 ]; then
        echo "ERROR: Pinning IRQ"
        clean_exit 1
    fi
    move_PIDs_to_core $IRQ_CORE_ID
    if [ $? -ne 0 ]; then
        echo "ERROR: Move PIDs"
        clean_exit 1;
    fi
    #update cpu frequency if -f was passed
    if [ $FREQUENCY -ne 0 ]
    then
        #include cpuconf.sh to configure cpu 
        #for (( cpus = $CPU_PIN; cpus < `expr $CPU_PIN \+ $PARALLEL`; cpus = cpus + 1 )); do
        for cpus in ${CPUIDS[@]}; do
            set_cpu_freq $cpus $FREQUENCY
        done
    fi


}

function setup_resdir() {

    #Directory setup for results
    if [ ! -e results ]; then
        mkdir $(dirname $0)/results
    fi

    #Top level, results->mem_range->#cpus->r/s->unique_ID
    RESDIR=results/${from}_${to}/${#CPUIDS[@]}/${TESTTYPE}
    mkdir -p $(dirname $0)/$RESDIR
    
    RESDIR=${RESDIR}/`ls -d ${RESDIR}/* 2> /dev/null | awk -F "/" 'BEGIN{max=0};{if ($NF > max) max=$NF};END{print max+1}'`
    
    if [ ! -e $RESDIR ]; then
        mkdir $RESDIR
    else
        echo "Something is wrong here"
        echo "$RESDIR already exists"
        clean_exit 1
    fi
}

clean_exit() {
    if [ $# -ne 1 ]; then
        echo "WARNING: No exit code given, default 1"
        excode=1
    else
        excode=$1
    fi

    #bash ./../exp_lockfile.sh -r

    exit $excode
}

#Usage messages
usage() {
    echo "usage: $0 -{s,r} -[dclptRn]"
    echo "Options:"
    echo -e "\t-s\t\t\tSequential Memory access test"
    echo -e "\t-r\t\t\tRandom Memory access test"
    echo -e "\t-d [cache level]\tDefault Cache Sizes (Read from getconf or /sys/devices"
    echo -e "\t-b [buffer size]\tThe multiplier to the in memory buffer"
    #echo -e "\t-c [memory footprint]\t(Deprecated) Specify a footprint to work with"
    echo -e "\t-l [line size]\t\tOverwrite line size"
    #echo -e "\t-p [# of CPUs]\t(Deprecated)Start p copies of the test program in parallel"
    echo -e "\t-n [# of tests]\t\tNumber of times a particular test should be performed"
    echo -e "\t-f [loopFactor]\t\tNumber of intermediate operations between consecutive loads"
    echo -e "\t-R from,to,incr\t\tRange of memory foorprint to test"
    echo -e "\t-t [Time n Sec]\t\tDuration of each test"
    echo -e "\t-N [Nice level]\t\tDefault 0 (no renice performed)"
    echo -e "\t-P [# of curves]\tCreate Plot of data, optionally with multiple curves"
    echo -e "\t-e [exp name]\t\tName of the experiment to show in the final plot"
    echo -e "\t-q [CPU frequency]\tDesired CPU frequency, this will apply to all the involved cores"
    echo -e "\t-C [CPU list/range]\tSpecify a list of CPUs on which experiments are to be performed"
    echo -e "\t-a\t\t\tRequire physically contigous memory"
    echo -e "\t-H\t\t\tUse Huge pages"
    echo -e "NOTE: Mind the order in which arguments are passed."
    clean_exit 1
}

#Parse the -C parameter and return a list of cpuids
parse_cpu_list() {
    #Check the validity of the argument
    if [ $# -ne 1 ]; then
        echo "a"
        echo "ERROR: The CPU list must in the form of cpu1,cpu2,cpu3-cpu4,cpu5 etc"
        clean_exit 1
    fi

    if [[ ! "$1" =~ ^([0-9]+,|[0-9]+-[0-9]+,)*([0-9]+|[0-9]+-[0-9]+)$ ]]; then
        echo "b"
        echo "ERROR: The CPU list must in the form of cpu1,cpu2,cpu3-cpu4,cpu5 etc"
        clean_exit 1
    fi

    #Extract the list items
    CPUIDS=(`echo $1 | awk -F "," '{for(i=1;i<=NF;i++){print $i}}' \
        | awk '/^[0-9]+-[0-9]+$/{print $1}' \
        | awk -F "-" '{for(i=$1;i<=$2;i++){printf("%d ",i)}}'`)
    CPUIDS=("${CPUIDS[@]}" `echo $1 | awk -F "," '{for(i=1;i<=NF;i++){print $i}}' \
        | awk '/^[0-9]+$/{print $1}'`)

    #For each list item, either add it to the final list or 
    #generate a sub range.

    #Remove duplicates?
    CPUIDS=(`(for i in ${CPUIDS[@]}; do echo $i; done) | sort | uniq`)
}

load_mem_info() {

    #Check for a valid Cache Level parameter
    if [ $1 -gt 4 ] || [ $1 -lt 1 ]; then
        usage
        clean_exit 1
    fi

    #If we only want the cache line size
    if [ -n "$2" ]; then
        return    
    fi

    size=0
    
    #NOTE 
    #The getconf method is not supported on all platforms
    #while sysfs is supported on all 2.6 and up kernels. 
    #We should probably just remove this...

    # try to retrieve cache information from 'getconf'
    #if [ -x $(type -p getconf) ]; then
    #    idx=$(expr $1 - 1)
    #    conf=( 1_D 2_ 3_ 4_ )
    #    size=$(getconf LEVEL${conf[idx]}\CACHE_SIZE 2>/dev/null)
    #    line=$(getconf LEVEL${conf[idx]}\CACHE_LINESIZE 2>/dev/null)
    #    [ -z "$size" ] && size=0
    #fi
    
    # try to retrieve cache information from /sys hierarchy
    if [ $size -eq 0 ]; then
        path=/sys/devices/system/cpu/cpu0/cache
        idx=$(for i in $(fgrep -l $1 $path/index*/level); do
            fgrep -lv Instruction $(dirname $i)/type;
        done|cut -c41)
        if [ ! -z "$idx" ]; then
            size=$(cat $path/index$idx/size|sed -e "/K/s///")
            line=$(cat $path/index$idx/coherency_line_size)
            size=$(expr $size \* 1024)
        else 
            echo "ERROR: This system does not have a level $1 cache" >&2
            clean_exit 1
        fi
    fi
    
    to=$(expr $size / 512)        # go up to (2 * cache size)
    incr=$(expr $size / 8192)            # divide cache size into 8 steps
    from=$incr

    echo "Loaded memory info"
    echo "$from,$to,$incr"
}

#Function to open and parse the output from perf. 
#Currently we are only interested in L1 and LLC cache miss rates.
#This information should be available to be plotted
#Dump into a file? avg_perf.dat?
#
perf_statistics() {
    if [ $# -ne 3 ]; then
        return 1;
    fi

    raw_data=$1
    count=$2
    cpuid=$3
    #We also use $from, $to, $incr to add the dataset size to the perf data for plotting
    #This is an outside dependency to this function and thus bad practice! TODO

    #This could be nicer. Strip the commas from the perf stat output and compute misses/total 
    #with awk. Finally average the numbers

    raw_prefix=$(fgrep -q raw $raw_data && echo "raw.*0x" || echo r)

    events=$(grep -E "^PERF_PARAM" experiment.config | grep -oE "r[0-9a-zA-Z\-]{3,10}" | sed 's/^r0*//')
    offset=2
    for i in $events; do
        cat $raw_data | sed 's/^Perf //' | awk -v idx=$offset '{print $1,$idx}' \
            | tee $RESDIR/${PERF_PREFIX}_${cpuid}_${i} \
            | ./avg.awk -v count=$count > $RESDIR/${AVG_PERF_PREFIX}_${cpuid}_${i}
        offset=`expr $offset \+ 1`
    done
}

# split_output arg_file
#This function splits the output from cachetest into seperate files.
#Notably, we want the perf stats in a seperate file from the measurements.

split_output() {
    if [ $# -ne 3 ]; then
        return 1;
    fi

    input=$1
    measure=$2
    perf=$3

    awk 'NR%2==0' $1 > $perf
    awk 'NR%2==1' $1 > $measure

}

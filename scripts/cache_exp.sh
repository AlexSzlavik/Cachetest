#!/bin/bash

#Overview of Script
##This script automates/faciliates cache testing and exposes additional parameters to the user, which are not present
##in the actual c++ program, cachetest.cc . The program also performs a number of sanity checks on parameters passed
##in and system parameters which are to be assumed during testing.
##Start by using getopts to parse the arguments passed in. -h always supersedes all other parameters.
##There is some fancy regex to parse a Range string for validity. The string should be of the form
##-R from,to,incr . Next we check and create the results directory structure as required.
##systeminfo.sh is invoked to dump the system information as it is found in /sys/devices/system/
##Additional information is dumped into a file which tells us the parameters that were used during a test run
##Now it is time to fork the processes. For every CPU specified via the options, we fork one process of run.sh .
##The output from run.sh is tee'd into the results raw .dat files, which is later averaged and plotted.
##To allow the system to be haltable, we use trap to catch a CTRL-C interrupt request, which we translate into a killall command for run.sh.
##This approach is suboptimal, originally we kept track of all the PIDs used in the PIDS array. However, kill doesn't like the array argument. FIXME
##The script synchronizes with all the child PIDs via a barrier. Only once the barrier is reached do we continue with the optional plot step.


#TODO List
##Want to include OProfile Options (or make it default if oprofile already exists)
##Want to add an option (or new script) to perform drawing of graphs with more information

GETOPTSTRING="Hahsrd:c:l:n:R:N:t:P:f:b:e:q:C:"

#Default values
PARALLEL=1
PLOT=0
PHYS_CONT=0
HUGE=0
FREQUENCY=0
IRQ_CORE_ID=0
iterations=1
duration=5
nicelevel=0
loopfactor=1
buffersize=1
CPU_PIN=1
CPUIDS=(1)
#End of Default Values

#Get the global config parameters
source $(dirname $0)/experiment.config
source $(dirname $0)/functions.sh


[ $# -lt 1 ] && usage
[ -e $PWD/cachtest ] && echo "Must be in parent dir of ./cachetest"

#First we need to decipher the arguments
#Parse the list of arguments
while getopts "$GETOPTSTRING" opt; do
    case $opt in
        (a)
            PHYS_CONT=1
            ;;
        (H)
            HUGE=1
            ;;
        (h)
            usage
            ;;
        (s)
            TESTTYPE="s"
            ;;
        (t)
            duration=$OPTARG
            ;;
        (r)
            TESTTYPE="r"
            SEQ_TYPE='-r $RANDOM'
            ;;
        (b)
            buffersize=$OPTARG
            ;;
        (d)
            LEVEL=$OPTARG
            load_mem_info "$OPTARG"
            ;;
        (c)
            target=$OPTARG
            ;;
        (l)
            line=$OPTARG
            ;;
        (f)
            loopfactor=$OPTARG
            ;;
        (n)
            iterations=$OPTARG
            ;;
        (R)
            #figure out a range
            tmpval=`echo $OPTARG | awk -F"," '/^[0-9]+,[0-9]+,[0-9]+$/{print "Found"}'`
            if [ -z "$tmpval" ]; then
                echo "ERROR:Invalid Range arguments. Must be of form from,to,incr" >&2
                usage
            fi
            from=$(echo $OPTARG | sed "s/,[0-9]*,[0-9]*$//")
            to=$(echo $OPTARG | sed "s/^[0-9]*,//" | sed "s/,[0-9]*$//")
            incr=$(echo $OPTARG | sed "s/^[0-9]*,[0-9]*,//")

            if [ $from -gt $to ] || [ $incr -lt 1 ]; then
                echo "ERROR:Invalid Range" >&2
                usage
            fi
            echo "$from,$to,$incr"
            ;;
        (P)
            #Check to see if gnuplot exists
            if [ -z "$OPTARG" ]; then
                PLOT=1
            else
                PLOT=$OPTARG
            fi
            ;;
        (N)
            nicelevel=$OPTARG
            NICECMD="nice -$nicelevel"
            ;;
        (e)
            expname=$OPTARG
            ;;
        (q)
            FREQUENCY=$OPTARG
            ;;
        (C)
            parse_cpu_list $OPTARG
            ;;

        (*)
            echo "Invalid option: -$OPTARG"
            ;;
    esac
done

#We must verify that some contraints are met
verify_constraints

#Find out if Writeback was set
TESTA=`nm $BENCHMARK | grep WRITEBACK_VAR`    #Check for the special variable that is set
if [ -z "$TESTA" ]; then
    write_set=0
else
    write_set=1
fi

#Setup Results dir $RESDIR
setup_resdir

#Setup CPUs
configure_cpus

#Dump Info
dump_config



###################
##LOAD EXPERIMENT##
###################

EXECDIR="`dirname $0`"
PIDS=()
#Fork multiple copies if required
#for (( cpus = 0; cpus < $PARALLEL; cpus = cpus + 1 )); do
if [ ${OLD_VERSION:-0} -eq 1 ]; then
    for cpus in ${CPUIDS[@]}; do
        echo "$EXECDIR/run.sh $TESTTYPE $from $to $incr $duration $iterations $line $RANDOM $loopfactor $buffersize $cpus $PHYS_CONT $HUGE | tee $RESDIR/cpu_${cpus}_result.dat &"
    
        $EXECDIR/run.sh $TESTTYPE $from $to $incr $duration $iterations $line $RANDOM $loopfactor $buffersize $cpus $PHYS_CONT $HUGE 2> $RESDIR/${ERROR_DUMP_FILE}_${cpus}.dat | tee $RESDIR/raw_${cpus}.dat &
    
        PIDS=(${PIDS[@]} $!)    #Array of PIDS, ideally we want the above and this op to be atomic :3
    done
    
    #Trap a CTRL-C Int and kill all our children
    #TODO Make cachetest multithreaded, so we don't have to deal with this
    trap "killall run.sh; clean_exit 0" SIGINT
    echo "PIDs are: ${PIDS[@]}"
    wait ${PIDS[@]}
else
    #In the new version, we'll rely on a multi-threaded cachetest application
    #Futhermore, we will launch the mutliple iterations from here

    EXECUTER_STATIC="$(dirname $0)/$LAUNCHER_PROGRAM -c `echo ${CPUIDS[@]} | sed 's/ /,/g'` --"
    EXECUTER_STATIC="${EXECUTER_STATIC} $BENCHMARK $PERF_PARAM"
    EXECUTER_STATIC="${EXECUTER_STATIC} -d $duration"
    EXECUTER_STATIC="${EXECUTER_STATIC} -l $line"
    #EXECUTER_STATIC="${EXECUTER_STATIC} ${SEQ_TYPE}"
    EXECUTER_STATIC="${EXECUTER_STATIC} -f $loopfactor"
    EXECUTER_STATIC="${EXECUTER_STATIC} -b $buffersize"
    EXECUTER_STATIC="${EXECUTER_STATIC} `if [ "${PHYS_CONT}" == "1" ];then echo -a;fi`"
    EXECUTER_STATIC="${EXECUTER_STATIC} `if [ "${HUGE}" == "1" ];then echo -h;fi`"
    EXECUTER_STATIC="${EXECUTER_STATIC} -o 0"
    EXECUTER_STATIC="${EXECUTER_STATIC} -c `echo ${CPUIDS[@]} | sed 's/ /,/g'`"

    for (( ts = $from; ts <= $to; ts = ts + $incr )); do
        for (( s = 1; s <= $iterations; s = s + 1 )); do

            EXECUTER_VARIABLE=""
            if [ $ts -ne $from -o $s -ne 1 ]; then
                EXECUTER_VARIABLE="${EXECUTER_VARIABLE} -A"
            fi
            if [ $TESTTYPE == "r" ]; then
                EXECUTER_VARIABLE="${EXECUTER_VARIABLE} -r $RANDOM"
            fi
            if [ $s -eq $iterations ];then
                EXECUTER_VARIABLE="${EXECUTER_VARIABLE} -s"
            fi
            EXECUTER_VARIABLE="${EXECUTER_VARIABLE} $ts $RESDIR/raw "

            DEBUG=1
            if [ ${DEBUG:-0} -eq 1 ]; then
                echo "$EXECUTER_STATIC $EXECUTER_VARIABLE 2>> ${RESDIR}/${ERROR_DUMP_FILE}"
            fi

            #####
            #Launch experiment
            #####
            $EXECUTER_STATIC $EXECUTER_VARIABLE 2>> ${RESDIR}/${ERROR_DUMP_FILE}
            mv sequencedump_${CPUIDS[0]}.dat $RESDIR/sequencedump_${cpus}_$ts.dat
        done
    done
fi;

#Finally, we need to average our results and perform the plotting
#if it has been requested
#for (( cpus = 0; cpus < $PARALLEL; cpus = cpus + 1 )); do
for cpus in ${CPUIDS[@]};do
    mv frameDump_${cpus}.dat $RESDIR
    if [ -s ${RESDIR}/${ERROR_DUMP_FILE}_${cpus}.dat ]; then
        echo "There was an error running CPU $cpus"
        cat ${RESDIR}/${ERROR_DUMP_FILE}_${cpus}.dat
        echo "----END ERRORS----"
    fi
    split_output $RESDIR/raw_${cpus}.dat $RESDIR/cpu_${cpus}_result.dat $RESDIR/${PERF_DUMP_FILE}_${cpus}.dat
    $(dirname $0)/avg.awk -v count=$iterations $RESDIR/cpu_${cpus}_result.dat > $RESDIR/avg_cpu_${cpus}.dat
    perf_statistics    $RESDIR/${PERF_DUMP_FILE}_${cpus}.dat $iterations $cpus
    if [ $PLOT -eq 1 ];then
        #do the plotting too
        echo "Do Plotting"
        $(dirname $0)/plot.sh $RESDIR/avg_cpu_${cpus}
    fi
done

#Create a sym link to the last Experiment run
rm -f ./newRes
ln -sf $RESDIR ./newRes

#Clear the EXP Lock
clean_exit 0


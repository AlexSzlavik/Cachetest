#!/bin/bash

usage() {
    echo "usage: $0 r|s <from> <to> <step> <duration> <repeats> <linesize> <rseed> <loopfactor> <cont?> <huge?>"
    exit 1
}

source $(dirname $0)/experiment.config

BENCHMARK_PROGRAM="$BENCHMARK $PERF_PARAM"
#BENCHMARK_PROGRAM="${PERF_PARAM} $BENCHMARK"
#if file $PERF | fgrep -q "shell script"; then
#    BENCHMARK_PROGRAM="$SHELL $BENCHMARK_PROGRAM"
#fi

[ $# -ne 13 ] && usage
[ "$1" != "r" -a "$1" != "s" ] && usage

#if [ $(expr $2 : [1234]) -eq 0 ]; then
#    echo choose index between 1 and 4 && exit 1
#elif [ $2 -lt 1 -o $2 -gt 4 ]; then
#    echo choose index between 1 and 4 && exit 1
#elif [ $3 -lt 1 ]; then
#    echo choose duration of at least 1 && exit 1
#elif [ $4 -lt 1 ]; then
#    echo choose repeats of at least 1 && exit 1
#fi

from=$2
target=$3
step=$4
duration=$5
repeats=$6
line=$7
s=$8
loopfactor=$9
buffersize=${10}
cpu_pin=${11}
phys_cont=`if [ "${12}" == "1" ];then echo -a;fi`
huge=`if [ "${13}" == "1" ];then echo -h;fi`
offset=$target
offset=0

if [ "$1" = "r" ]; then
    for (( ts = $from; ts <= $target; ts = ts + $step )); do
        for (( s = 1; s <= $repeats; s = s + 1 )); do
            #echo $(dirname $0)/$LAUNCHER_PROGRAM -c $cpu_pin -- $BENCHMARK_PROGRAM $ts -d $duration -l $line -b 16 -r $RANDOM -f $loopfactor -b $buffersize $phys_cont $huge -o $offset -c $cpu_pin
            if [ $cpu_pin -eq 4 ]; then offset=0; fi #This line enables offset
            $(dirname $0)/$LAUNCHER_PROGRAM -c $cpu_pin -- $BENCHMARK_PROGRAM $ts -d $duration -l $line -r $RANDOM -f $loopfactor -b $buffersize $phys_cont $huge -o $offset -c $cpu_pin
        done
    done
else
    for (( ts = $from; ts <= $target; ts = ts + $step )); do
        for (( s = 1; s <= $repeats; s = s + 1 )); do
            #echo $(dirname $0)/$LAUNCHER_PROGRAM -c $cpu_pin -- $BENCHMARK_PROGRAM $ts -d $duration -l $line -f $loopfactor -b $buffersize -c $cpu_pin -b $buffersize $phys_cont $huge
            $(dirname $0)/$LAUNCHER_PROGRAM -c $cpu_pin -- $BENCHMARK_PROGRAM $ts -d $duration -l $line -f $loopfactor -b $buffersize $phys_cont $huge
        done
    done
fi

exit 0

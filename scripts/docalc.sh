#!/bin/bash

if [ $# -ne 2 ]; then
    echo "Usage: $0 <target folder> <experiment iterations>"
    exit 1
fi

resdir=$1
iterations=$2


for i in `ls $resdir/avg_cpu_[0-9]*.dat`;do
    cpunum=$(basename $i|cut -f3 -d_|cut -f1 -d.)
    rows=`wc $i | awk '{print $1}'`

    #L1 Hits
    #awk -f avg_diff.awk -v maxr=$rows $resdir/avg_perf_${cpunum}_40 $resdir/avg_perf_${cpunum}_1f42 | tee $resdir/l1_hits_${cpunum}.dat
    awk -v maxr=$(wc -l < $resdir/cpu_${cpunum}_result.dat) \
        'NR<=maxr{suba[NR]=$3};NR>maxr{print $1,suba[NR-maxr]-$2}' \
        $resdir/cpu_${cpunum}_result.dat $resdir/perf__${cpunum}_l1-misses \
        > $resdir/l1_hits_${cpunum}.dat
    awk -f avg.awk -v count=$iterations \
        $resdir/l1_hits_${cpunum}.dat > $resdir/l1_avg_hits_${cpunum}.dat
   
    #L2 Hits
    awk -v maxr=$(wc -l < $resdir/cpu_${cpunum}_result.dat) \
        'NR<=maxr{suba[NR]=$2};NR>maxr{print $1,suba[NR-maxr]-$2}' \
        $resdir/perf__${cpunum}_l1-misses $resdir/perf__${cpunum}_l2-misses \
        > $resdir/l2_hits_${cpunum}.dat
    awk -f avg.awk -v count=$iterations \
        $resdir/l2_hits_${cpunum}.dat > $resdir/l2_avg_hits_${cpunum}.dat
    
    #L3 hits
    awk -v maxr=$(wc -l < $resdir/cpu_${cpunum}_result.dat) \
        'NR<=maxr{suba[NR]=$2};NR>maxr{print $1,suba[NR-maxr]-$2}' \
        $resdir/perf__${cpunum}_l2-misses $resdir/perf__${cpunum}_l3-misses \
        > $resdir/l3_hits_${cpunum}.dat
    awk -f avg.awk -v count=$iterations \
        $resdir/l3_hits_${cpunum}.dat > $resdir/l3_avg_hits_${cpunum}.dat
    
    #RAM Reloads
    #cp $resdir/avg_perf_${cpunum}_4000037e1 $resdir/dram_hits_${cpunum}.dat
    cp $resdir/perf__${cpunum}_l3-misses $resdir/dram_hits_${cpunum}.dat
    awk -f avg.awk -v count=$iterations \
        $resdir/dram_hits_${cpunum}.dat > $resdir/dram_avg_hits_${cpunum}.dat
    
    #Calculate the percentages (Python script)
    python perf_stat.py $resdir $cpunum
    awk -f avg.awk -v count=$iterations \
        $resdir/percentages_${cpunum}.dat > $resdir/percentages_avg_${cpunum}.dat

    awk -v maxr=$(wc -l < $resdir/cpu_${cpunum}_result.dat) \
        'NR<=maxr{suba[NR]=$2};NR>maxr{print $1,suba[NR-maxr]/$3}' \
        $resdir/perf__${cpunum}_l3-misses $resdir/cpu_${cpunum}_result.dat \
        > $resdir/l3_miss_rate_${cpunum}.dat
    awk -f avg.awk -v count=$iterations \
        $resdir/l3_miss_rate_${cpunum}.dat > $resdir/l3_avg_miss_rate_${cpunum}.dat 

done

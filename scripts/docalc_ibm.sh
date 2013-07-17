#!/bin/bash

if [ $# -ne 2 ]; then
    echo "Usage: $0 <target folder> <experiment iterations>"
    exit 1
fi

resdir=$1
iterations=$2

for i in `ls $resdir/avg_cpu_[0-9]*.dat`;do
    echo "Processing $i"
    cpunum=$(basename $i|cut -f3 -d_|cut -f1 -d.)
    rows=`wc $i | awk '{print $1}'`

    #L1 Hits
    #awk -f avg_diff.awk -v maxr=$rows -v sp=1 $resdir/avg_cpu_${cpunum}.dat $resdir/avg_perf_${cpunum}_400f0 | tee $resdir/l1_hits_${cpunum}.dat
    awk -v maxr=$(wc -l < $resdir/cpu_${cpunum}_result.dat) \
        'NR<=maxr{suba[NR]=$3};NR>maxr{print $1,suba[NR-maxr]-$2}' \
        $resdir/cpu_${cpunum}_result.dat $resdir/perf__${cpunum}_400f0 \
        > $resdir/l1_hits_${cpunum}.dat
    awk -f avg.awk -v count=$iterations \
        $resdir/l1_hits_${cpunum}.dat > $resdir/l1_avg_hits_${cpunum}.dat
    
    #L2 Hits
    cp $resdir/perf__${cpunum}_1c040 $resdir/l2_hits_${cpunum}.dat
    awk -f avg.awk -v count=$iterations \
        $resdir/l2_hits_${cpunum}.dat > $resdir/l2_avg_hits_${cpunum}.dat
    
    #L3 hits
    cp $resdir/perf__${cpunum}_2c040 $resdir/l3_hits_${cpunum}.dat
    awk -f avg.awk -v count=$iterations \
        $resdir/l3_hits_${cpunum}.dat > $resdir/l3_avg_hits_${cpunum}.dat
    
    #RAM Reloads
    cp $resdir/perf__${cpunum}_300fe $resdir/dram_hits_${cpunum}.dat
    awk -f avg.awk -v count=$iterations \
        $resdir/dram_hits_${cpunum}.dat > $resdir/dram_avg_hits_${cpunum}.dat
    
    #Calculate the percentages (Python script)
    python perf_stat.py $resdir $cpunum
    awk -f avg.awk -v count=$iterations \
        $resdir/percentages_${cpunum}.dat > $resdir/percentages_avg_${cpunum}.dat


    #SPECIAL
    #We also want the misses from L3
    awk -v maxr=$(wc -l < $resdir/cpu_${cpunum}_result.dat) \
        'NR<=maxr{suba[NR]=$3};NR>maxr{print $1,$2/suba[NR-maxr]}' \
        $resdir/cpu_${cpunum}_result.dat $resdir/perf__${cpunum}_300fe \
        > $resdir/l3_miss_rate_${cpunum}.dat
 
    awk -f avg.awk -v count=$iterations \
        $resdir/l3_miss_rate_${cpunum}.dat > $resdir/l3_avg_miss_rate_${cpunum}.dat
done

#!/bin/bash

#Systeminfo.sh
#This script dumps the system information to a dump file from sysfs /sys.
#TODO
#The CPU information is not necessarily correct on all platforms.
#Should consider revamping it.

if [ $# -lt 1 ]; then
	echo "Need Dump file name, and folder to store"
	echo "Usage: $0 dump_file_name"
	exit 1
fi

SYS_DUMP_FILE=$1

#Sanity
if [ ! -e `dirname $SYS_DUMP_FILE` ]; then
	echo "ERROR: Result dir `dirname $SYS_DUMP_FILE` does not exist"
	exit 1
fi

#Drop system info
touch $SYS_DUMP_FILE
echo "#CPUs: `cat /proc/cpuinfo | awk '/physical id/{print $4}' | uniq | wc -l`" >> $SYS_DUMP_FILE
echo "#Cores/CPU: `cat /proc/cpuinfo | grep "cpu cores" | uniq | awk '{print $4}'`" >> $SYS_DUMP_FILE
echo "#Cores: `cat /sys/devices/system/cpu/cpu*/topology/core_id | sort | uniq | wc -l`" >> $SYS_DUMP_FILE

#Determine the cache Assosciativity
lca=()
for i in `grep -lv Instruction /sys/devices/system/cpu/cpu0/cache/index?/type`; do
	LEVEL=`cat $(dirname $i)/level`
	ASOC=$(cat `dirname $i`/ways_of_associativity)
	lca=(${lca[@]} `echo "L$LEVEL: $ASOC"`)
done
echo "Assosciativity: ${lca[@]}" >> $SYS_DUMP_FILE


for ((level=1; level<4; level=level+1)); do
	size=""
	line=""
	# try to retrieve cache information from 'getconf'
	#if [ -x $(type -p getconf) ]; then
	#	conf=( 1_D 2_ 3_ 4_ )
	#	size=$(getconf LEVEL${conf[level]}\CACHE_SIZE 2>/dev/null)
	#	line=$(getconf LEVEL${conf[level]}\CACHE_LINESIZE 2>/dev/null)

	## try to retrieve cache information from /sys hierarchy
	#else
		path=/sys/devices/system/cpu/cpu0/cache
		idx=$(for i in $(fgrep -l $level $path/index*/level); do
			fgrep -lv Instruction $(dirname $i)/type;
		done|cut -c41)
		if [ ! -z "$idx" ]; then
			size=$(cat $path/index$idx/size|sed -e "/K/s///")
			line=$(cat $path/index$idx/coherency_line_size)
			size=$(expr $size \* 1024)
		fi
	#fi
	if [ -z "$size" ]; then
		size="N/A"
		line="N/A"
	fi
	echo "L$level Cache: $size,$line" >> $SYS_DUMP_FILE
done
######################################


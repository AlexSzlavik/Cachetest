#!/bin/bash

FILENAME=out
GP_COMMAND=""
MAX_UPPER_BOUND="*"
num_plots=0
num_args=0
IGNORE_X_VALUES=0

#Usage method
usage() {
	echo "Usage: -o [output name] -P <result dir 1> <result dir 2> ... <result dir n>"
	echo "This tool is used to create a plot containing n errorlines. "
	echo "The data files are passed in (order matters), along with a config file, which provides" 
	echo "additional info wich should be printed on the graph."
	echo "-P    Include a second axis and performance counter data on plot"
	exit 1
}

#Sanity checks
#Get the output name
while getopts "Po:u:" opt; do
  case $opt in
    o)
	FILENAME=$OPTARG
	#get rid of -o arg in $@
  	num_args=`expr $num_args \+ 2`
      ;;
    u)
    MAX_UPPER_BOUND=$OPTARG
  	num_args=`expr $num_args \+ 2`
      ;;
	P)
	WITH_PERF=1
  	num_args=`expr $num_args \+ 1`
	;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      ;;
  esac
done

for i in `seq 1 1 $num_args`;do shift;done

#The arguments are used to drive the output of the plot. 
source $(dirname $0)/experiment.config

#Setting the required parameters by checking the first directory passed
if [ ! -e $1 ]; then
	echo "ERROR:$1 does not exist"
	exit 1
fi

TESTTYPE=`cat $1/$CONFIG_DUMP_FILE | grep "Type" | grep -oE ".$"`
HOSTNAME=`cat $1/$CONFIG_DUMP_FILE | grep "Hostname" | sed 's/.*: //'`
#SHA1 check is missing here :( 
if [ "$TESTTYPE" = "s" ]; then
	TESTTYPE="Sequential"
else
	TESTTYPE="Random"
fi

SHA1_TMP=`cat $1/$CONFIG_DUMP_FILE | grep "SHA1" | awk 'END{print $2}'`
SHA1_CUR=""



#iterating through list of directories
while [ "$1" ]; do
	if [ ! -e $1 ]; then
		echo "ERROR: $1 does not exist"
		exit 1
	fi

	SHA1_CUR=`cat $1/$CONFIG_DUMP_FILE | grep "SHA1" | awk 'END{print $2}'`

	if [ $SHA1_TMP != $SHA1_CUR ]; then
		echo "ERROR:"
		echo "The SHA1 hashes of 2 of the experiments don't match."
		echo "You are likely comparing two different experiments."
		echo "Aborting"
		exit 1
	fi
	SHA1_TMP=$SHA1_CUR
  TITLE=`cat $1/$CONFIG_DUMP_FILE | grep "Experiment Name:" | sed 's/.*: //'`
  if [ -z "$TITLE" ]; then
	FREQ=`cat $1/$CONFIG_DUMP_FILE | grep "CPU Frequency:" | sed 's/.*: //'`
	CORES=`cat $1/$CONFIG_DUMP_FILE | grep "Number of CPUs used:" | sed 's/.*: //'`
	TITLE="$CORES Core(s) @ $FREQ"
  fi

  GP_COMMAND="$GP_COMMAND plot \"$1/avg_cpu_3.dat\" using (\$1<=$IGNORE_X_VALUES?1/0:\$1):(\$5/10**6):(\$6/10**6) with errorlines title \"Loop Iterations\" lw 2 ps 2"
  if [ -n "$WITH_PERF" ]; then
    GP_COMMAND="$GP_COMMAND ,\"$1/percentages_avg_3.dat\" using (\$1<=$IGNORE_X_VALUES?1/0:\$1):3:4 with errorlines title \"L1 Hits\" lw 2 ps 2 axes x1y2"
    GP_COMMAND="$GP_COMMAND ,\"$1/percentages_avg_3.dat\" using (\$1<=$IGNORE_X_VALUES?1/0:\$1):5:6 with errorlines title \"L2 Hits\" lw 2 ps 2 axes x1y2"
    GP_COMMAND="$GP_COMMAND ,\"$1/percentages_avg_3.dat\" using (\$1<=$IGNORE_X_VALUES?1/0:\$1):7:8 with errorlines title \"L3 Hits\" lw 2 ps 2 axes x1y2"
    GP_COMMAND="$GP_COMMAND ,\"$1/percentages_avg_3.dat\" using (\$1<=$IGNORE_X_VALUES?1/0:\$1):9:10 with errorlines title \"DRAM Loads\" lw 2 ps 2 axes x1y2"
  fi

  #Remove this and change GP_COMMAND to plot the given data on the same axes
  GP_COMMAND="${GP_COMMAND};"
  num_plots=`expr $num_plots \+ 1`

  shift
done
#remove the leading comma 
GP_COMMAND="${GP_COMMAND%,}"

#update filename to include the current date
FILENAME="$FILENAME-`date +%Y-%m-%d`"

gnuplot << EOF

set xlabel "Working Set Size (kB)"
set ylabel "Memory References (10^6)"
set y2label "% Fetch from Cachelevel"

set yr [0:$MAX_UPPER_BOUND]
set y2r [0:1]
set ytics nomirror
set y2tics
set terminal postscript eps enhanced monochrome
set output "$FILENAME.eps"
set title "Cache Access Iterations \n AMD Opteron  - $TESTTYPE"

set multiplot layout $num_plots,1 rowsfirst
$GP_COMMAND
unset multiplot
 
set terminal png size 800,800
set output "$FILENAME.png"

set multiplot layout $num_plots,1
$GP_COMMAND
unset multiplot


EOF

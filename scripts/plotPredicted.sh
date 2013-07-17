#!/bin/bash

FILENAME=$1
INPUT=$2

gnuplot <<EOF

set xlabel "Dataset (KB)"
set ylabel "Miss Rate"

set title "Predicted Miss Rate"
set style line 1 lw 2 pt 1 lc rgb "#aa0000"
set style line 2 lw 2 pt 6 lc rgb "#00aa00"
set style line 3 lt 0 lw 1 lc rgb "dark-grey"

set grid ls 3

set yr [0:1]
set xtics nomirror
set ytics nomirror

set terminal png size 800,800
set terminal postscript eps enhanced color
set output "$(dirname $INPUT)/$FILENAME.eps"

plot "$INPUT" using 1:2:(\$3/sqrt(20)) with errorlines ls 1 ti "Predicted Miss-rate", "$(dirname $INPUT)/l3_avg_miss_rate_20.dat" using 1:3:4 with errorlines ls 2 ti "Measured Miss-rate"

EOF

#!/bin/bash
if [ $# -ne 1 ]; then
	gnuplot <<EOF

set xlabel "data set (KB)"
set ylabel "access count (mio)"
#set y2label "time (ms)"
#set y2tics autofreq

set terminal postscript color
set output "sumr.ps"
plot \
"L1r.out" using 1:5 with linespoints title "L1r (work)" axes x1y2,\
"L2r.out" using 1:5 with linespoints title "L2r (work)" axes x1y2,\
"L3r.out" using 1:5 with linespoints title "L3r (work)" axes x1y2,\
"L4r.out" using 1:5 with linespoints title "L4r (work)" axes x1y2
#"L1r.out" using 1:3 with linespoints title "L1r (time)",\
#"L2r.out" using 1:3 with linespoints title "L2r (time)",\
#"L3r.out" using 1:3 with linespoints title "L3r (time)",\
#"L4r.out" using 1:3 with linespoints title "L4r (time)",\
set output

set output "sums.ps"
plot \
"L1s.out" using 1:5 with linespoints title "L1s (work)" axes x1y2,\
"L2s.out" using 1:5 with linespoints title "L2s (work)" axes x1y2,\
"L3s.out" using 1:5 with linespoints title "L3s (work)" axes x1y2,\
"L4s.out" using 1:5 with linespoints title "L4s (work)" axes x1y2
#"L1s.out" using 1:3 with linespoints title "L1s (time)",\
#"L2s.out" using 1:3 with linespoints title "L2s (time)",\
#"L3s.out" using 1:3 with linespoints title "L3s (time)",\
#"L4s.out" using 1:3 with linespoints title "L4s (time)",\
set output

EOF

elif [ -s $1.dat ]; then
	gnuplot <<EOF

set xlabel "data set (KB)"
set ylabel "access count (mio)"
#set y2label "time (ms)"
#set y2tics autofreq
set terminal postscript color
set output "$1.ps"
plot \
"$1.dat" using 1:5 with linespoints title "$1 (work)" axes x1y2
#"$1.dat" using 1:3 with linespoints title "$1 (time)"
set output

EOF

else
	echo $1.dat does not exist
fi

exit 0

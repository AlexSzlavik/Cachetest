gnuplot <<EOF

set term png size 600,600
set output "missPlot.png"

plot './results/512_8192/1/s/2/frameProbMiss.plot' with errorlines title "Prob. Miss via Frames", './results/512_8192/1/s/2/percentages_avg_96.dat' using 1:9:10 with errorlines title "L3 Misses"

EOF

#This is only useful if used with the dome.sh script on douze.
#It will create a bunch of graphs. The value for L3 under -u 
#is only correct for douze

#L1
./multiple_plot.sh -P -o rand-cont-nopf-l1 results/8_128/1/r/1
./multiple_plot.sh -P -o rand-cont-withpf-l1 results/8_128/1/r/2
./multiple_plot.sh -P -o rand-notcont-nopf-l1 results/8_128/1/r/3
./multiple_plot.sh -P -o rand-notcont-withpf-l1 results/8_128/1/r/4

./multiple_plot.sh -P -o seq-cont-nopf-l1 results/8_128/1/s/1
./multiple_plot.sh -P -o seq-cont-withpf-l1 results/8_128/1/s/2
./multiple_plot.sh -P -o seq-notcont-nopf-l1 results/8_128/1/s/3
./multiple_plot.sh -P -o seq-notcont-withpf-l1 results/8_128/1/s/4


#L2
./multiple_plot.sh -P -o rand-cont-nopf-l2 results/64_1024/1/r/1
./multiple_plot.sh -P -o rand-cont-withpf-l2 results/64_1024/1/r/2
./multiple_plot.sh -P -o rand-notcont-nopf-l2 results/64_1024/1/r/3
./multiple_plot.sh -P -o rand-notcont-withpf-l2 results/64_1024/1/r/4

./multiple_plot.sh -P -o seq-cont-nopf-l2 results/64_1024/1/s/1
./multiple_plot.sh -P -o seq-cont-withpf-l2 results/64_1024/1/s/2
./multiple_plot.sh -P -o seq-notcont-nopf-l2 results/64_1024/1/s/3
./multiple_plot.sh -P -o seq-notcont-withpf-l2 results/64_1024/1/s/4


#L3
./multiple_plot.sh -P -u 1400 -o rand-cont-nopf-l3 results/768_12288/1/r/1
./multiple_plot.sh -P -u 1400 -o rand-cont-withpf-l3 results/768_12288/1/r/2
./multiple_plot.sh -P -u 1400 -o rand-notcont-nopf-l3 results/768_12288/1/r/3
./multiple_plot.sh -P -u 1400 -o rand-notcont-withpf-l3 results/768_12288/1/r/4

./multiple_plot.sh -P -u 1400 -o seq-cont-nopf-l3 results/768_12288/1/s/1
./multiple_plot.sh -P -u 1400 -o seq-cont-withpf-l3 results/768_12288/1/s/2
./multiple_plot.sh -P -u 1400 -o seq-notcont-nopf-l3 results/768_12288/1/s/3
./multiple_plot.sh -P -u 1400 -o seq-notcont-withpf-l3 results/768_12288/1/s/4

for i in ibm_results_2012_10_4/**/**/**/**;do ./docalc_ibm.sh $i 20;done
for i in ibm_results_2012_10_4/**/**/**/**;do ./multiple_plot.sh -P -o $i/{plot,};done
for i in ibm_results_2012_10_4/512_8192/**/**/**/extractedFrames;do cat $i | awk '/ProbMiss/{print $2}' | awk -f avg.awk -v count=20 | awk 'BEGIN{i=512};{print i,$0;i+=512}' > $(dirname $i)/predictedMiss.plot; done
for i in ibm_results_2012_10_4/32_512/**/**/**/extractedFrames;do cat $i | awk '/ProbMiss/{print $2}' | awk -f avg.awk -v count=20 | awk 'BEGIN{i=32};{print i,$0;i+=32}' > $(dirname $i)/predictedMiss.plot; done
eog ibm_results_2012_10_4/512_8192/**/**/**
for i in ibm_results_2012_10_4/**/**/**/**/predictedMiss.plot;do ./plotPredicted.sh predicted $i; done

for i in amd_results_2012_10_26/*/*/*/*/plot-2012-12-11.eps;do cp $i /tmp/$(grep "Experiment" $(dirname $i)/used_config.txt | sed 's/^.*: //' | sed 's/ /_/g')_$(grep "Scan Type" $(dirname $i)/used_config.txt | sed 's/^.*: //').eps;done

#Do all the dirs in result
for i in `ls results/**/**/**/** | grep : | sed 's/://'`;do ./docalc_ibm.sh $i;done

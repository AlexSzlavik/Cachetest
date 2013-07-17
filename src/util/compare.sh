./phys_dumper;for i in `seq 5 1 15`;do awk -v var=$i '/Normal/{if(!first){first=$var}else{print (first-$var)*(2^(var-3));first=0}}' out1 out2;done
rm out1 out2

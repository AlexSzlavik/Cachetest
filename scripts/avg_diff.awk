#!/usr/bin/awk -f

BEGIN {
    i=1;
    if (maxr == 0) {
        printf("Need maxr argument\n");
        printf("maxr should indicate the number of rows in each file\n");
        exit 1
    }
}

{ 
    if (NR<=maxr) {
        if(sp==1) {
            large[i++]=$5
            #printf("a:%f\n",(large[i-1]));
        }else{
            large[i++]=$3
        }
    } else {
	printf("%d %d %.2f 0\n",$1,$2,large[NR - maxr]-$3)
    } 
} 

BEGIN {
	if ( count == 0 ) {
		printf( "use option -v count=<rowcount>\n" )
		exit 1
	}
}
{
    if(NR==1)min=max=$1;

    nums[NR % count] = $1;
    if(max<$1)max=$1;
    if(min>$1)min=$1;
    if(NR%(count) == 0){
        asort(nums)
        if(count%2) {
            print nums[(count+1)/2],min,max;
        }else
        {
            print (nums[(count/2)]+nums[(count/2)+1]) / 2,min,max;
        }
        max=0;
    }
}

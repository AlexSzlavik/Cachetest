#!/usr/bin/awk -f
BEGIN {
	if ( count == 0 ) {
		printf( "use option -v count=<rowcount>\n" )
		exit 1
	}
}
{
	for (i = 1; i <= NF; i++) {
		sum[i] += $i;
		sqsum[i] += $i * $i;
	}
	if ( NF > maxf ) maxf = NF;
	line = line + 1
	if ( line >= count ) {
		for (i = 1; i <= maxf; i++) {
			avg = sum[i] / count;
			var = sqsum[i] / count - avg**2
			printf( "%.6f %.6f ", avg, (var>0)?sqrt(var):0 );
			sum[i] = 0;
			sqsum[i] = 0;
		}
		printf( "\n" );
		line = 0;
	}
}

#ifdef CAR_DEBUG
#include <cardinal.h>
#include <stdio.h>
#include <stdlib.h>

void car_report_unreachable(const char *file, int line, const char *function) {
	fprintf(stderr,"Internal error: An UNREACHABLE(..) statement was reached.\n");
	fprintf(stderr,"This is a bug in Cardinal; please report it to the devs.\n");
	fprintf(stderr,"(File `%s`, line `%d`, in function `%d(..)`)\n",file,line,function);
	exit(1);
}

#endif

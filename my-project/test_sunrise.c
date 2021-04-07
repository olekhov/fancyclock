/* Test sunrise calculations. This meant to be run on PC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "sunrise.h"

void test_trig()
{
	float x;
/*
	for(x=-3*M_PI;x<3*M_PI;x+=0.01) {
		printf("%6.4lf %+8.6lf %+8.6le %8.6lf %+8.6le %+8.6le\n",
		       x, mysin(x), mysin(x)-sin(x),
		       mycos(x), mycos(x)-cos(x), mysqrt(x)-sqrt(x));
	}
*/
	//x=-0.61;	printf("X: %+6.4lf %+8.6lf\n", x, myacos(x));


	for(x=-1;x<1;x+=0.01) {
		printf("%+6.4lf %+8.6lf %+8.6le\n", x, myacos(x), acos(x) - myacos(x));
	}

}

int main()
{
	struct tm tm;
	time_t t;

	t=time(NULL);
	localtime_r(&t, &tm);

//	 test_trig();	return 0;

	/*
	printf("%04d.%02d.%02d %02d:%02d:%02d\n",
		tm.tm_year+1900,
		tm.tm_mon+1,
		tm.tm_mday,
		tm.tm_hour,
		tm.tm_min,
		tm.tm_sec);
*/
	int i;
	for(i=0;i<240;i++) {
		tm.tm_mday+=1;
		mktime(&tm);

		printf("%04d.%02d.%02d  ",
		tm.tm_year+1900,
		tm.tm_mon+1,
		tm.tm_mday);

		uint32_t rise=0, set=0;

		calc_sun_times(&tm, &rise, &set);
		printf("Rise: %05d Set: %05d Dur:%06d ", rise, set, set-rise);

		tm.tm_sec = rise;
		tm.tm_hour = 3;
		tm.tm_min = 0;
		mktime(&tm);

		char datestr[16]={0}, timestr[16]={0};

		strftime(datestr, sizeof(datestr), "%Y.%m.%d", &tm);
		strftime(timestr, sizeof(timestr), "%H.%M.%S  ", &tm);
		printf("%s -- ", timestr);


		tm.tm_sec = set;
		tm.tm_hour = 3;
		tm.tm_min = 0;
		mktime(&tm);

		strftime(datestr, sizeof(datestr), "%Y.%m.%d", &tm);
		strftime(timestr, sizeof(timestr), "%H.%M.%S  ", &tm);

		printf("%s\n", timestr);

	}


	return 0;
}

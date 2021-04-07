#include "sunrise.h"

#define M_PI 3.1415926

float myfabs(float x)
{
	return x>0? x: -x;
}

/** @brief sine function by Taylor series
  For |theta| < 0.5 this sums up to 5 terms
  @param[in] theta Argument in radians. Should be as close to zero as possible
  @returns sin(theta)
  */
static float sin_taylor(float theta)
{
	float s=0, term=theta;
	int n=2;
	do {
		s+=term;
		term*=-theta*theta/n/(n+1);
		n+=2;
	} while(myfabs(term)>1e-8);
	return s;
}

/** @brief cosine function by Taylor series
  For |theta| < 0.5 this sums up to 5 terms
  @param[in] theta Argument in radians. Should be as close to zero as possible
  @returns cos(theta)
  */
static float cos_taylor(float theta)
{
	float s=1, term=-theta*theta/2;
	int n=3;
	do {
		s+=term;
		term*=-theta*theta/n/(n+1);
		n+=2;
	} while(myfabs(term)>1e-8);
	return s;
}


/** @brief sine function
  shifts argument to 0..pi and the computes as 1/third angle
  @param[in] theta Argument in radians
  @returns sin(theta)
  */
float mysin(float theta)
{
	float res=0, s=1;

	theta = myfmod(theta, 2*M_PI);

	if(theta>M_PI) {
		theta -= M_PI;
		s=-1;
	}

	theta/=3;
	res = sin_taylor(theta);
	res = 3*res-4*res*res*res;
	return s*res;
}

/** @brief cosine function
  shifts argument to 0..pi and the computes as 1/third angle
  @param[in] theta Argument in radians
  @returns sin(theta)
  */
float mycos(float theta)
{
	float res=0, s=1;

	theta = myfmod(theta, 2*M_PI);

	if(theta>M_PI) {
		theta -= M_PI;
		s=-1;
	}

	theta/=3;
	res = cos_taylor(theta);
	res = 4*res*res*res-3*res;
	return s*res;
}

/** @brief square root function
  solves equation y=x*x for given x by division algorithm
  */

float mysqrt(float x)
{

	float a = 0, b = x, c=(a+b)/2;

	while( (b-a)>1e-6*x) {
		c=(a+b)/2;
		if(c*c>x)
			b=c;
		else
			a=c;
	}
	return c;
}

/** @brief arctan by Taylor series
  */
float myatan(float x)
{
	float s=0, term=x;
	int n=1;
	do {
		s+=term;
		term*=n*x/(n+2);
		n+=2;
	} while(myfabs(term)>1e-8);
	return s;
}

/** @brief arccos function
  solves y=cos(x) for given y by division algorithm
  */
float myacos(float x)
{
	float a,b,c;
	a=0; b=M_PI; c = (a+b)/2;
	if(x<=-1) return M_PI;
	if(x>=1) return 0;

	while(b-a > 1e-6) {
		if(mycos(c)>x) {
			a=c;
		} else {
			b=c;
		}
		c=(a+b)/2;
	}
	return c;

}

/** @brief fmod - float modulus
  calculates r for x,d such as: x=d*n+r, where n is integer
  */
float myfmod(float x, float d)
{
	if(x>d) {
		int r = x/d;
		return x-r*d;
	} else if (x<0) {
		int r = (-x+d)/d;
		return x+r*d;
	}
	return x;
}

/** @brief calculate sunrise and sunset times
  https://en.wikipedia.org/wiki/Sunrise_equation

  @param[in] tm date-time (day) for which sunrise and sunset are calculated
  @param[in] n_lat latitude, north is positive
  @param[in] e_long longitude, east is positive
  @param[out] secs_rise seconds since midnight to sunrise
  @param[out] secs_set seconds since midnight to sunset
  */
void calc_sun_times(struct tm *tm, float n_lat, float e_long, uint32_t *secs_rise, uint32_t *secs_set)
{
	int days_since_01012000 = 0;
	int i;

	/* Wikipedia formulae count form 01.01.2000 */
	for( i=2000; i < tm->tm_year + 1900; i++) {
		days_since_01012000 += 365;

		/* add leap years */
		if(i%4 == 0)
			days_since_01012000 ++;
	}

	/* add days since beginning of the year */
	days_since_01012000 += tm->tm_yday;

	/* Calculate Julian day. This can't be handled by 'float' type.
	   Since we do not need 2million days since Julian epoch, this bias
	   is discarded. */
	int j_day = days_since_01012000 /*+ 2451545*/;

	/* Mean Solar Time */
	float j_star = days_since_01012000 - e_long/360;

	/* Mean Solar Anomaly */
	float M = myfmod(357.5291+0.98560028*j_star, 360);

	/* Equation of the center */
	float C = 1.9148*mysin(M/180*M_PI) + 0.02*mysin(2*M/180*M_PI) + 0.003*mysin(3*M/180*M_PI);

	/* Elliptic latitude */
	float lambda = myfmod(M+C+180+102.9372, 360);

	/* Solar transit. Remember discarded bias of 2.45 million days before? */
	float j_transit=/*2451545.0+*/j_star + 0.0053*mysin(M/180*M_PI) - 0.0069*mysin(2*lambda/180*M_PI);

	/* Declination delta */
	float sindelta = mysin(lambda/180*M_PI)*mysin(23.44/180*M_PI);
	float cosdelta = mysqrt(1-sindelta*sindelta);

	/* Hour angle */
	float cosw0 = mysin(-0.83/180*M_PI)-mysin(n_lat/180*M_PI)*sindelta/(mycos(n_lat/180*M_PI)*cosdelta);

	/* Omega_0 */
	float acosw0 = myacos(cosw0)/M_PI*180/360;

	/* Julian dates of sunrise and sunset. In fractions of day */
	float j_rise = j_transit - acosw0 ;
	float j_set = j_transit + acosw0 ;

	/* In Julian dates the day's zero is noon.
	   So we need to shift half a day (0.5).
	   The result is needed in seconds. 86400 seconds in a day
	   */

	*secs_rise = (j_rise - j_day + 0.5)*86400;
	*secs_set = (j_set - j_day + 0.5)*86400;
}


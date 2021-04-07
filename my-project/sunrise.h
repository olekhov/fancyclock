#pragma once
#include <stdint.h>
#include <time.h>

/** @brief calculate sunrise and sunset times
  https://en.wikipedia.org/wiki/Sunrise_equation

  @param[in] tm date-time (day) for which sunrise and sunset are calculated
  @param[in] n_lat latitude, north is positive
  @param[in] e_long longitude, east is positive
  @param[out] secs_rise seconds since midnight to sunrise
  @param[out] secs_set seconds since midnight to sunset
  */
void calc_sun_times(struct tm *tm, float n_lat, float e_long, uint32_t *secs_rise, uint32_t *secs_set);

/** @brief mathematic functions implementation. No math on STM32F103CBT6 :( */
float myfmod(float x, float d);
float myfabs(float x);
float mysin(float x);
float mycos(float x);
float mysqrt(float x);
float myacos(float x);
float myatan(float x);

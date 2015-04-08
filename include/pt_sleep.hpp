#ifndef _PT_SLEEP_HPP_
#define _PT_SLEEP_HPP_

#include <pthread.h>
#include <sys/time.h>
#include <math.h>

// Replacement for usleep() that is compatible 
// with pthreads
void pt_sleep_us(float micro_seconds);

void pt_sleep_ms(float milli_seconds);

void pt_sleep_s(float seconds);

#endif

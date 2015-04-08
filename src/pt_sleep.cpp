#include <pthread.h>
#include <sys/time.h>
#include <math.h>

void pt_sleep_us(float micro_seconds) {
    // For creating a delay instead of sleep()
    struct timeval sleep_begin_time;
    double sleep_end_time_ns;
    double sleep_end_time_spart;
    double sleep_end_time_nspart;
    struct timespec sleep_end_time;
    pthread_mutex_t sleep_mutex;
    pthread_cond_t sleep_cond;

    // Initialize mutex and cond
    pthread_mutex_init(&sleep_mutex, NULL);
    pthread_cond_init(&sleep_cond,   NULL);

    // Get current time of day
    gettimeofday(&sleep_begin_time, NULL);
    // Calculate sleep time in nanoseconds^M
    sleep_end_time_ns = sleep_begin_time.tv_usec*1e3+sleep_begin_time.tv_sec*1e9+micro_seconds*1e3;
    // Convert timeout time to s and ns parts
    sleep_end_time_nspart = modf(sleep_end_time_ns/1e9, &sleep_end_time_spart);
    // Put timeout time into timespec struct
    sleep_end_time.tv_sec = sleep_end_time_spart;
    sleep_end_time.tv_nsec = sleep_end_time_nspart;

    // Wait until delay passes
    pthread_mutex_lock(&sleep_mutex);
    pthread_cond_timedwait(&sleep_cond, &sleep_mutex, &sleep_end_time);

}

void pt_sleep_ms(float milli_seconds) {
    pt_sleep_us(milli_seconds*1e3);
}

void pt_sleep_s(float seconds) {
    pt_sleep_s(seconds*1e6);
}

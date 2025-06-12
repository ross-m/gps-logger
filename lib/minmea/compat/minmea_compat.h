#ifndef MINMEA_COMPAT_H_
#define MINMEA_COMPAT_H_

#if !defined(HAVE_STRUCT_TIMESPEC)
struct timespec {
    time_t tv_sec;
    time_t tv_nsec;
};
#endif
#endif

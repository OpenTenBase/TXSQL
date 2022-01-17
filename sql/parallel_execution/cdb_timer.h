#ifndef CDB_TIMER_INCLUDED
#define CDB_TIMER_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(__i386__) && !defined(__x86_64__) && !defined(__sparc__)
#warning No supported architecture found -- timers will return junk.
#endif

#define CPU_FRE 2500000000   /* set CPU_FRE if needed. */

static __inline__ unsigned long long curtick() {
        unsigned long long tick;
#if defined(__i386__)
        unsigned long lo, hi;
        __asm__ __volatile__ (".byte 0x0f, 0x31" : "=a" (lo), "=d" (hi));
        tick = (unsigned long long) hi << 32 | lo;
#elif defined(__x86_64__)
        unsigned long lo, hi;
        __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
        tick = (unsigned long long) hi << 32 | lo;
#elif defined(__sparc__)
        __asm__ __volatile__ ("rd %%tick, %0" : "=r" (tick));
#endif
        return tick;
}

static __inline__ void startTimer(unsigned long long* t) {
        *t = curtick();
}
static __inline__ void stopTimer(unsigned long long* t) {
        *t = curtick() - *t;
}
static __inline__ double getSecond(unsigned long long start_time) {
        return (curtick()-start_time)/(double)CPU_FRE;
}
static __inline__ double getSecondDuratoin(unsigned long long start_time, 
                                           unsigned long long end) {
        return (end-start_time)/(double)CPU_FRE;
}
static __inline__ double getMilliSecond(unsigned long long start_time) {
        return (curtick()-start_time)/(double)CPU_FRE*1000;
}
static __inline__ double getMicroSecond(unsigned long long start_time) {
        return (curtick()-start_time)/(double)CPU_FRE*1000000;
}
static __inline__ double convertCyclesToSecond(unsigned long long tick){
        return tick/(double)CPU_FRE;
}

#ifdef __cplusplus
}
#endif

#endif /* CDB_TIMER_INCLUDED */
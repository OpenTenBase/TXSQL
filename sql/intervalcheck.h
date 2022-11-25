/*
 * intervalcheck.h
 *
 *  Created on: 2015.6.26
 *      Author: harlylei
 */

#ifndef INTERVALCHECK_H_
#define INTERVALCHECK_H_

#include <time.h>
#include <sys/time.h>
#include <stdlib.h>

class CIntervalCheck {
public:
    CIntervalCheck( time_t lastBeginTime, time_t interval ) :
                    m_lastBeginTime (lastBeginTime), m_interval (interval) {
    }

    bool check() {
        time_t cur = time (NULL);
        /*
          There is a potential mistake to check like this:
          Suppose stall limit is 5 secs, and system time is modified to 5 secs
          earlier right after the timer thread checks for stall, then 5 seconds
          later this is called, but cur==m_lastBeginTime, so timer thread
          doesn't timeout and won't check for stall. Next time this is called,
          it will return true. so mysqld will stall for twice the stall limit,
          but only for one time, so that's not too bad.
        */
        if (labs (cur - m_lastBeginTime) > m_interval) {
            m_lastBeginTime = cur;
            return true;
        }
        else {
            return false;
        }
    }

    void setInterval(time_t interval ) {
        m_interval = interval;
    }

    void setLastBeginTime(time_t lastBeginTime ) {
        m_lastBeginTime = lastBeginTime;
    }
    time_t getInterval() const { return m_interval; }
private:
    time_t m_lastBeginTime;
    time_t m_interval;
};

// Check whether timeout (specified in milliseconds) happens.
// See the comment for forceSignalTimer() for more information.
class CMSecIntervalCheck {
    // HIDE default constructor and assignment operator.
    CMSecIntervalCheck();
    CMSecIntervalCheck &operator=(const CMSecIntervalCheck &);
public:
    CMSecIntervalCheck(time_t lastBeginTime, uint64_t msecInterval) :
                    m_lastBeginMsec(((uint64_t) lastBeginTime) * 1000), m_msecInterval (msecInterval) {
    }
    
    // Check whether current time is m_msecInterval milliseconds later than
    // m_lastBeginMsec, i.e. timeout happened.
    bool check () {
        struct timeval tm;
        gettimeofday(&tm, NULL);
        uint64_t cur = ((uint64_t) tm.tv_sec) * 1000 + tm.tv_usec / 1000;
        /*
          The 1st check() call may prematually return true(timeout) because
          m_lastBeginMsec is 0 in current usage when this instance is created.
          This is not a big problem at least for now since it's only used in
          thread pool checking and a premature check for
          thread pool stall isn't halmful.
        */
        if ((uint64_t)labs(cur - m_lastBeginMsec) > m_msecInterval) {
            m_lastBeginMsec = cur;
            return true;
        }
        else {
            return false;
        }
    }

    void setInterval ( uint64_t interval ) {
        m_msecInterval = interval;
    }

    void setLastBeginTime ( time_t lastBeginTime ) {
        m_lastBeginMsec = (((uint64_t) lastBeginTime) * 1000);
    }

    uint64_t getInterval() const { return m_msecInterval; }
private:
    uint64_t m_lastBeginMsec; //microseconds of beginning
    uint64_t m_msecInterval;
};

#endif /* INTERVALCHECK_H_ */

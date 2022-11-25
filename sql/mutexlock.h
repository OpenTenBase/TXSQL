#ifndef _mutexlock_H
#define _mutexlock_H

#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <deque>
#include <arpa/inet.h>
#include <sys/time.h>
#include <cassert>

static const size_t g_maxqueuesize = 20000000;

class CTMutex {
protected:
    pthread_mutex_t lock_;
    // HIDE copy-constructor, operator=.
    CTMutex(const CTMutex &);
    CTMutex &operator=(const CTMutex &);
public:
    explicit CTMutex(pthread_mutexattr_t *attrs = 0 ) {
        pthread_mutex_init(&lock_, attrs);
    }

    ~CTMutex() {
        pthread_mutex_destroy(&lock_);
    }

    int acquire() {
        return pthread_mutex_lock(&lock_);
    }

    int acquire(const struct timespec *_time ) {
        return pthread_mutex_timedlock(&lock_, _time);
    }

    int release() {
        return pthread_mutex_unlock(&lock_);
    }

    pthread_mutex_t& lock() { return lock_; }
};

template<class Tlock>
class CTGuard {
private:
    Tlock* m_lock;
    int m_owner;
public:
    CTGuard(Tlock& lock) :
                    m_lock (&lock), m_owner (-1) {
        acquire();
    }

    CTGuard(Tlock *lock) :
      m_lock (lock), m_owner (-1) {
        acquire();
    }

    ~CTGuard() {
        release();
    }

    int acquire() {
        if (m_lock) {
            return m_owner = m_lock->acquire();
        } else {
            return 0;
        }
    }

    int release() {
        if (m_owner == -1) {
            return 0;
        } else {
            m_owner = -1;
            if (m_lock) {
                return m_lock->release();
            } else {
                return 0;
            }
        }
    }

    int locked() const {
        return m_owner != -1;
    }

private:
    CTGuard();
    CTGuard(const CTGuard<Tlock> &);
    void operator= (const CTGuard<Tlock> &);
};

template<class MUTEX>
class CTCondition {
public:
    // = Initialiation and termination methods.
    // Initialize the condition variable.
    CTCondition(MUTEX &m) :
                    mutex_(m) {
        pthread_condattr_t attr;
        pthread_condattr_init(&attr);

        if (pthread_cond_init(&this->cond_, &attr) != 0) {
            fprintf (stderr, "call pthread_cond_init failed\n");
            assert(false);
        }

        pthread_condattr_destroy(&attr);
    }

    // Implicitly destroy the condition variable.
    ~CTCondition(void) {
        if (this->remove () == -1) {
            fprintf (stderr, "remove CCondition failed\n");
            assert(false);
        }
    }

    // Block on condition.
    int wait(void) {
        return pthread_cond_wait(&this->cond_, &this->mutex_.lock());
    }

    // Block on condition.wait for a second time
    int wait(const struct timespec* _time) {
        return pthread_cond_timedwait(&this->cond_, &this->mutex_.lock (), _time);
    }

    // Block on condition.wait for a second time
    int wait(int msec) {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        uint64_t curMsec = tv.tv_sec * 1000L + tv.tv_usec / 1000;
        curMsec += msec;

        struct timespec _time;
        _time.tv_sec = curMsec / 1000;
        _time.tv_nsec = (curMsec % 1000) * 1000 * 1000;	//sleep msec

        return pthread_cond_timedwait(&this->cond_, &this->mutex_.lock (), &_time);
    }

    // Signal one waiting thread.
    int signal(void) {
        return pthread_cond_signal(&this->cond_);
    }

    // Signal *all* waiting threads.
    int broadcast(void) {
        return pthread_cond_broadcast(&this->cond_);
    }

    // = Utility methods.
    // Explicitly destroy the condition variable.
    int remove(void) {
        int result = 0;

        while ((result = pthread_cond_destroy(&this->cond_)) == -1 && errno == EBUSY) {
            pthread_cond_broadcast(&this->cond_);
            usleep (100);
        }

        return result;
    }

    // Returns a reference to the underlying mutex_; DZW: why is this needed? at least return const reference.
    MUTEX &mutex(void) {
        return this->mutex_;
    }

protected:
    // Condition variable.
    pthread_cond_t cond_;

    // Reference to mutex lock.
    MUTEX &mutex_;

private:
    // = Prevent assignment and initialization.
    void operator=(const CTCondition<MUTEX> & );
    CTCondition(const CTCondition<MUTEX> & );
    CTCondition();
};

template<class MTYPE, class MUTEX = CTMutex>
class CTMsgQueue {
private:
    std::deque<MTYPE*> m_queue;
    MUTEX m_lock;

public:
    CTMsgQueue() {}

    ~CTMsgQueue() {
        CTGuard<MUTEX> guard(m_lock);
        while (m_queue.size() > 0) {
            MTYPE* _pvalue = m_queue.front();
            if (_pvalue != NULL) {
                delete _pvalue;
                m_queue.pop_front();
            } else {
                break;
            }
        }
    }

    bool postmsg(MTYPE* msg) {
        CTGuard<MUTEX> guard(m_lock);
        if (m_queue.size () > g_maxqueuesize) {
            return false;
        }
        m_queue.push_back(msg);
        return true;
    }

    MTYPE* getmsg() {
        CTGuard<MUTEX> guard(m_lock);

        if (m_queue.size() <= 0)
            return NULL;

        MTYPE* _pvalue = m_queue.front();
        if (_pvalue != NULL) {
            m_queue.pop_front();
            return _pvalue;
        } else {
            return NULL;
        }
    }
    
    int size() {
        CTGuard<MUTEX> guard(m_lock);
        return m_queue.size();
    }
};

template<class MTYPE, class MUTEX = CTMutex>
class CTMsgQueueCond {
private:
    std::deque<MTYPE*> m_queue;

    MUTEX m_lock;
    CTCondition<MUTEX> cond_;

public:
    CTMsgQueueCond() : cond_ (m_lock) {}

    ~CTMsgQueueCond() {
        CTGuard<MUTEX> guard (m_lock);
        while (m_queue.size () > 0) {
            MTYPE* _pvalue = m_queue.front ();
            if (_pvalue != NULL) {
                delete _pvalue;
                m_queue.pop_front ();
            } else {
                break;
            }
        }
    }

    bool postmsg(MTYPE *msg) {
        CTGuard<MUTEX> guard (m_lock);
        if (m_queue.size() > (size_t) g_maxqueuesize) {
            return false;
        }
        m_queue.push_back(msg);
        cond_.signal();
        return true;
    }

    // Wait infinitely for a message.
    MTYPE* getmsg() {
        CTGuard<MUTEX> guard(m_lock);

        if (m_queue.size() <= 0) {
            cond_.wait();
            if (m_queue.size() <= 0) {
                return NULL;
            }
        }

        MTYPE* _pvalue = m_queue.front();
        if (_pvalue != NULL) {
            m_queue.pop_front();
            if (m_queue.size() + 10 >= (size_t) g_maxqueuesize) {
                cond_.signal();// DZW: we'd better always notify here.
            }

            return _pvalue;
        } else {
            return NULL;
        }
    }

    //wait for '_time' and if still none, return
    //NULL, otherwise return the head of the queue.
    MTYPE* getmsg(const struct timespec* _time) {
        CTGuard<MUTEX> guard(m_lock);

        if (m_queue.size() <= 0) {
            cond_.wait(_time);
            if (m_queue.size() <= 0) {
                return NULL;
            }
        }

        MTYPE* _pvalue = m_queue.front();
        if (_pvalue != NULL) {
            m_queue.pop_front();
            if (m_queue.size() + 10 >= (size_t) g_maxqueuesize)
            {
                cond_.signal();// about to be full, notify.
            }
            return _pvalue;
        } else {
            return NULL;
        }
    }

    //Timeout waiting
    MTYPE* getmsg(int msec) {
        struct timeval tv;
        gettimeofday (&tv, NULL);
        uint64_t curMsec = tv.tv_sec * 1000L + tv.tv_usec / 1000;
        curMsec += msec;

        struct timespec _time;
        _time.tv_sec = curMsec / 1000;
        _time.tv_nsec = (curMsec % 1000) * 1000 * 1000;

        return getmsg(&_time);
    }

    int size () {
        CTGuard<MUTEX> guard (m_lock);
        return m_queue.size ();
    }
};

template<class MTYPE, class MUTEX = CTMutex>
class CTMsgObjQueueCond {
private:
    std::deque<MTYPE> m_queue;

    MUTEX m_lock;
    CTCondition<MUTEX> cond_;

public:
    CTMsgObjQueueCond() :
                    cond_ (m_lock) {}
    ~CTMsgObjQueueCond() {}
    
    bool postmsg(const MTYPE &msg ) {
        CTGuard<MUTEX> guard (m_lock);
        if (m_queue.size() > (size_t) g_maxqueuesize) {
            // no wait and directly returns, so getmsg() don't need to notify.
            return false;
        }

        m_queue.push_back(msg);
        cond_.signal();
        return true;
    }

    /*
      Wait infinitely for a message. returns true if msg is got, and 'out'
      takes back the msg; returns false if no msg, and 'out' is intact.
    */
    bool getmsg(MTYPE &out) {
        CTGuard<MUTEX> guard(m_lock);

        if (m_queue.size() <= 0) {
            cond_.wait();
            if (m_queue.size() <= 0) {
                return false;
            }
        }

        out= m_queue.front();
        m_queue.pop_front();
        return true;
    }

    //wait for '_time' and if still none, return
    //NULL, otherwise return the head of the queue.
    bool getmsg( const struct timespec* _time, MTYPE &out) {
        CTGuard<MUTEX> guard(m_lock);

        if (m_queue.size() <= 0) {
            cond_.wait(_time);
            if (m_queue.size () <= 0) {
                return false;
            }
        }

        out= m_queue.front();
        m_queue.pop_front();

        return true;
    }

    bool getmsg(int msec, MTYPE &out) {
        struct timeval tv;
        gettimeofday (&tv, NULL);
        uint64_t curMsec = tv.tv_sec * 1000L + tv.tv_usec / 1000;
        curMsec += msec;

        struct timespec _time;
        _time.tv_sec = curMsec / 1000;
        _time.tv_nsec = (curMsec % 1000) * 1000 * 1000;	

        return getmsg(&_time, out);
    }

    int size() {
        CTGuard<MUTEX> guard (m_lock);
        return m_queue.size();
    }
};
#endif


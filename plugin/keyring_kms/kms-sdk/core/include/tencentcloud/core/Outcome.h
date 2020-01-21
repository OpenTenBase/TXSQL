
#ifndef TENCENTCLOUD_CORE_OUTCOME_H_
#define TENCENTCLOUD_CORE_OUTCOME_H_

#include <utility>

namespace TencentCloud
{
    template<typename E, typename R>
    class Outcome
    {
    public:
        Outcome() : m_success(true)
        {
        }
        explicit Outcome(const E &e) : m_error(e), m_success(false)
        {
        }
        explicit Outcome(const R &r) : m_result(r), m_success(true)
        {
        }
        Outcome(const Outcome &o) :
            m_error(o.m_error),
            m_result(o.m_result),
			m_success(o.m_success)
        {
        }
        Outcome(Outcome &&o)
        {
          *this = std::move(o);
        }
        Outcome & operator=(const Outcome &o)
        {
            if (this != &o)
            {
                m_success = o.m_success;
                m_error = o.m_error;
                m_result = o.m_result;
            }
            return *this;
        }
        Outcome & operator=(Outcome &&o)
        {
            if (this != &o)
            {
                m_success = o.m_success;
                m_error = o.m_error;
                m_result = o.m_result;
            }
            return *this;
        }

        inline bool IsSuccess() const
        {
            return m_success;
        }

        inline E& GetError()
        {
            return m_error;
        }

        inline R GetResult() const
        {
            return m_result;
        }

    private:
        E m_error;
        R m_result;
		bool m_success;
    };
}
#endif // !TENCENTCLOUD_CORE_OUTCOME_H_


#ifndef TENCENTCLOUD_CORE_ERROR_H_
#define TENCENTCLOUD_CORE_ERROR_H_

#include <string>
#include <sstream>

namespace TencentCloud
{
    class Error
    {
    public:
        Error() = default;
        Error(const std::string message) :
            m_errorCode("CoreInternalError"),
            m_message(message)
        {
        }
        Error(std::string code, const std::string message) :
            m_errorCode(code),
            m_message(message)
        {
        }
        ~Error() = default;

        std::string GetErrorCode() const
        {
            return m_errorCode;
        }
        std::string GetErrorMessage() const
        {
            return m_message;
        }
        std::string GetRequestId() const
        {
            return m_requestId;
        }
        Error& SetErrorCode(const std::string &code)
        {
            m_errorCode = code;
            return *this;
        }
        Error& SetErrorMessage(const std::string& message)
        {
            m_message = message;
            return *this;
        }
        Error& SetRequestId(const std::string& requestId)
        {
            m_requestId = requestId;
            return *this;
        }

        std::string PrintAll() const
        {
            std::ostringstream out;
            out << "RequestId=" << m_requestId << ", ";
            out << "ErrorCode=" << m_errorCode << ", ";
            out << "ErrorMessage=" << m_message;
            return out.str();
        }

    private:
        std::string m_errorCode;
        std::string m_message;
        std::string m_requestId;
    };
}

#endif // !TENCENTCLOUD_CORE_ERROR_H_

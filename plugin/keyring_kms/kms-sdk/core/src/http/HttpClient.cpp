/*
 * Copyright (c) 2017-2019 THL A29 Limited, a Tencent company. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <tencentcloud/core/http/HttpClient.h>
#include <tencentcloud/core/utils/Utils.h>
#include <cassert>
#include <sstream>
#include <vector>

using namespace TencentCloud;

namespace
{
    size_t recvBody(char *ptr, size_t size, size_t nmemb, void *userdata)
    {
        std::ostringstream &out = *static_cast<std::ostringstream*>(userdata);
        out << std::string(ptr, nmemb*size);
        return nmemb * size;
    }

    size_t recvHeaders(char *buffer, size_t size, size_t nitems, void *userdata)
    {
        HttpResponse *response = static_cast<HttpResponse*>(userdata);
        std::string line(buffer);
        auto pos = line.find(':');
        if (pos != line.npos)
        {
            std::string name = line.substr(0, pos);
            std::string value = line.substr(pos + 2);
            size_t p = 0;
            if ((p = value.rfind('\r')) != value.npos)
                value[p] = '\0';
            response->SetHeader(name, value);
        }
        return nitems * size;
    }

    void setCUrlProxy(CURL *curlHandle, const NetworkProxy &proxy)
    {
        if (proxy.GetType() == NetworkProxy::Type::NONE)
            return;

        int64_t type;
        switch (proxy.GetType())
        {
            case NetworkProxy::Type::SOCKS5:
                type = CURLPROXY_SOCKS5;
                break;
            /*case NetworkProxy::Type::HTTPS:
                type = CURLPROXY_HTTPS;
                break;*/
            case NetworkProxy::Type::HTTP:
            default:
                type = CURLPROXY_HTTP;
                break;
        }
        curl_easy_setopt(curlHandle, CURLOPT_PROXYTYPE, type);

        std::ostringstream out;
        out << proxy.GetHostName() << ":" << proxy.GetPort();
        curl_easy_setopt(curlHandle, CURLOPT_PROXY, out.str().c_str());

        if (!proxy.GetUser().empty()) {
            out.clear();
            out << proxy.GetUser() << ":" << proxy.GetPassword();
            curl_easy_setopt(curlHandle, CURLOPT_PROXYUSERPWD, out.str().c_str());
        }
    }
}

HttpClient::HttpClient() :
    m_curlHandle(curl_easy_init())
{
}

HttpClient::~HttpClient()
{
    curl_easy_cleanup(m_curlHandle);
}

void HttpClient::InitGlobalState()
{
    curl_global_init(CURL_GLOBAL_ALL);
}

void HttpClient::CleanupGlobalState()
{
    curl_global_cleanup();
}

void HttpClient::SetReqTimeout(int64_t timeoutOfMs)
{
    m_reqTimeout = timeoutOfMs;
}

void HttpClient::SetConnectTimeout(int64_t timeoutOfMs)
{
    m_connectTimeout = timeoutOfMs;
}

void HttpClient::SetProxy(const NetworkProxy &proxy)
{
    m_proxy = proxy;
}

HttpClient::HttpResponseOutcome HttpClient::SendRequest(const HttpRequest &request)
{
    curl_easy_reset(m_curlHandle);
    HttpResponse response;

    std::string url = request.GetUrl().ToString();
    switch (request.GetMethod())
    {
    case HttpRequest::Method::POST:
        {
            if (request.BodySize() > 0)
            {
                curl_easy_setopt(m_curlHandle, CURLOPT_POSTFIELDS, request.Body());
            }
            else
            {
                curl_easy_setopt(m_curlHandle, CURLOPT_POSTFIELDS, "");
            }
        }
        break;
    default:
        break;
    }

    curl_easy_setopt(m_curlHandle, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(m_curlHandle, CURLOPT_TIMEOUT_MS, m_reqTimeout);
    curl_easy_setopt(m_curlHandle, CURLOPT_CONNECTTIMEOUT_MS, m_connectTimeout);

    curl_easy_setopt(m_curlHandle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(m_curlHandle, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(m_curlHandle, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(m_curlHandle, CURLOPT_HEADERDATA, &response);
    curl_easy_setopt(m_curlHandle, CURLOPT_HEADERFUNCTION, recvHeaders);

    curl_slist *header_list = nullptr;
    auto headers = request.Headers();
    for (auto &p : headers)
    {
        std::string str = p.first;
        str.append(": ").append(p.second);
        header_list = curl_slist_append(header_list, str.c_str());
    }
    curl_easy_setopt(m_curlHandle, CURLOPT_HTTPHEADER, header_list);
    std::ostringstream out;
    curl_easy_setopt(m_curlHandle, CURLOPT_WRITEDATA, &out);
    curl_easy_setopt(m_curlHandle, CURLOPT_WRITEFUNCTION, recvBody);
    setCUrlProxy(m_curlHandle, m_proxy);

    char errbuf[CURL_ERROR_SIZE];
    curl_easy_setopt(m_curlHandle, CURLOPT_ERRORBUFFER, errbuf);

    CURLcode res = curl_easy_perform(m_curlHandle);
    if (header_list)
    {
        curl_slist_free_all(header_list);
    }
    switch (res)
    {
    case CURLE_OK:
    {
        int64_t response_code;
        curl_easy_getinfo(m_curlHandle, CURLINFO_RESPONSE_CODE, &response_code);
        response.SetStatusCode(response_code);
        response.SetBody(out.str());

        if (response_code != 200)
        {
            std::string errorMsg = "status=" + Utils::int2str(response_code) + ", " + out.str();
            return HttpResponseOutcome(Error("ServiceNetworkError", errorMsg));
        }

        return HttpResponseOutcome(response);
    }
    default:
        return HttpResponseOutcome(Error("NetworkError", std::string(errbuf)));
    }
}

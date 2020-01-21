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

#include <tencentcloud/core/http/Http.h>
#include <algorithm>

#if defined(WIN32) && defined(_MSC_VER)
#  define strcasecmp _stricmp
#else
#  include <strings.h>
#endif

using namespace TencentCloud;
using namespace std;

Http::Http() :
    m_headers(),
    m_body("")
{
}

Http::Http(const Http &other) :
    m_headers(other.m_headers),
    m_body(other.m_body)
{
}

Http::Http(Http &&other)
{
    *this = std::move(other);
}

Http& Http::operator=(const Http &other)
{
    if (this != &other) {
        m_headers = other.m_headers;
        m_body = other.m_body;
    }
    return *this;
}

Http& Http::operator=(Http &&other)
{
  if (this != &other)
      *this = std::move(other);
  return *this;
}

void Http::AddHeader(const string & name, const string & value)
{
    SetHeader(name, value);
}

string Http::Header(const string & name) const
{
    auto it = m_headers.find(name);
    if (it != m_headers.end())
        return it->second;
    else
        return std::string();
}

Http::HeaderCollection Http::Headers() const
{
    return m_headers;
}

void Http::RemoveHeader(const string & name)
{
    m_headers.erase(name);
}

void Http::SetHeader(const string & name, const string & value)
{
    m_headers[name] = value;
}

Http::~Http()
{
}

const char* Http::Body() const
{
    return m_body.c_str();
}

size_t Http::BodySize() const
{
    return m_body.size();
}

bool Http::HasBody() const
{
    return (m_body.size() != 0);
}

void Http::SetBody(const string &body)
{
    m_body = body;
}

bool Http::NocaseLess::operator() (const std::string & s1, const std::string & s2) const
{
    return strcasecmp(s1.c_str(), s2.c_str()) < 0;
}

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

#include <tencentcloud/core/AbstractClient.h>
#include <tencentcloud/core/utils/Utils.h>
#include <tencentcloud/core/Sign.h>
#include <tencentcloud/core/Config.h>

using namespace TencentCloud;
using std::string;

namespace
{
    const string SDK_VERSION_PREFIX = "SDK_CPP_";
}

AbstractClient::AbstractClient(const string &endpoint, const string &version, const Credential &credential, const string &region) :
    AbstractClient(endpoint, version, credential, region, ClientProfile())
{
}

AbstractClient::AbstractClient(const string &endpoint, const string &version, const Credential &credential, const string &region, const ClientProfile &profile) :
    m_credential(credential),
    m_clientProfile(profile),
    m_endpoint(endpoint),
    m_region(region),
    m_sdkVersion(SDK_VERSION_PREFIX + TENCENTCLOUD_VERSION_STR),
    m_apiVersion(version),
    m_httpClient(new HttpClient()),
    m_service("")
{
}

AbstractClient::~AbstractClient()
{
    delete m_httpClient;
}

void AbstractClient::SetNetworkProxy(const NetworkProxy &proxy)
{
    m_httpClient->SetProxy(proxy);
}

void AbstractClient::SetEndpoint(const std::string &endpoint)
{
    m_endpoint = endpoint;
}

std::string AbstractClient::GetEndpoint() const
{
    return m_endpoint;
}

void AbstractClient::SetRegion(const string &region)
{
    m_region = region;
}

string AbstractClient::GetRegion() const
{
    return m_region;
}

void AbstractClient::SetClientProfile(const ClientProfile &profile)
{
    m_clientProfile = profile;
}

ClientProfile AbstractClient::GetClientProfile() const
{
    return m_clientProfile;
}

void AbstractClient::SetCredential(const Credential &credential)
{
    m_credential = credential;
}

void AbstractClient::SetEndPoint(const std::string &endpoint)
{
    m_endpoint = endpoint;
}

std::string AbstractClient::GetEndPoint() const
{
    return m_endpoint;
}

Credential AbstractClient::GetCredential() const
{
    return m_credential;
}

HttpClient::HttpResponseOutcome AbstractClient::MakeRequest(const AbstractModel& request, const string &actionName)
{
    string body = request.ToJsonString();

    HttpProfile httpProfile = m_clientProfile.GetHttpProfile();
    string endpoint = httpProfile.GetEndpoint();
    if (endpoint == "")
        endpoint = m_endpoint;

    string::size_type pos = endpoint.find_first_of(".");
    if (pos != string::npos)
        m_service = endpoint.substr(0, pos);
    else
    {
        m_service = "unknown";
        return HttpClient::HttpResponseOutcome(Error("ClientError", "endpoint `"+ endpoint + "` is not valid"));
    }

    Url url;
    url.SetHost(endpoint);
    HttpRequest httpRequest(url);
    httpRequest.SetMethod(HttpRequest::Method::POST);
    httpRequest.SetBody(body);
    httpRequest.AddHeader("Host", url.GetHost());
    httpRequest.AddHeader("Content-Type", "application/json");
    httpRequest.AddHeader("X-TC-Action", actionName);
    httpRequest.AddHeader("X-TC-Version", m_apiVersion);
    if (m_region != "")
        httpRequest.AddHeader("X-TC-Region", m_region);
    string token = m_credential.GetToken();
    if (token != "")
        httpRequest.AddHeader("X-TC-Token", token);
    httpRequest.AddHeader("X-TC-RequestClient", m_sdkVersion);
    if (httpProfile.IsKeepAlive())
        httpRequest.AddHeader("Connection", "Keep-Alive");
    else
        httpRequest.AddHeader("Connection", "Close");
    GenerateSignature(httpRequest);

    m_httpClient->SetReqTimeout(httpProfile.GetReqTimeout()*1000);
    m_httpClient->SetConnectTimeout(httpProfile.GetConnectTimeout()*1000);

    return m_httpClient->SendRequest(httpRequest);
}

void AbstractClient::GenerateSignature(HttpRequest &request)
{
    int64_t currentTime;
    string credDate;
    Utils::GetCurrentTimeAndUtcDate(currentTime, credDate);
    request.AddHeader("X-TC-Timestamp", Utils::int2str(currentTime));
    string credentialScope = credDate + "/" + m_service + "/tc3_request";

    string serviceName = m_service;
    string canonicalUri = request.GetUrl().GetPath();
    string canonicalQueryString = "";
    string payloadHash;
    string method;

    if (request.GetMethod() == HttpRequest::Method::POST)
    {
        method = "POST";
        payloadHash = Utils::HashSha256(string(request.Body(), request.BodySize()));
    }
    Http::HeaderCollection headers = request.Headers();
    string canonicalHeaders = "content-type:" + headers[string("Content-Type")] + "\nhost:"
                        + headers["Host"] + "\n";
    string signedHeaders = "content-type;host";
    string canonicalRequest = method + "\n" + canonicalUri + "\n" + canonicalQueryString
                       + "\n" + canonicalHeaders + "\n" + signedHeaders + "\n" + payloadHash;

    string algorithm = "TC3-HMAC-SHA256";
    string cannonicalRequestHash = Utils::HashSha256(canonicalRequest);
    string stringToSign = algorithm + "\n" +  headers["X-TC-Timestamp"] +
                            "\n" +  credentialScope + "\n" + cannonicalRequestHash;

    string secretId = m_credential.GetSecretId();
    string secretKey = m_credential.GetSecretKey();
    Sign signHandler(secretId, secretKey);
    string signature = signHandler.Tc3Sign(credDate, serviceName, stringToSign);

    string authorization = "TC3-HMAC-SHA256 Credential=" + secretId + "/" + credentialScope
                           + ", SignedHeaders=content-type;host" + ", Signature=" + signature;
    request.AddHeader("Authorization", authorization);
}


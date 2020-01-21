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

#include <tencentcloud/kms/v20190118/KmsClient.h>
#include <tencentcloud/core/Executor.h>
#include <tencentcloud/core/Runnable.h>

using namespace TencentCloud;
using namespace TencentCloud::Kms::V20190118;
using namespace TencentCloud::Kms::V20190118::Model;
using namespace std;

namespace
{
    const string VERSION = "2019-01-18";
    const string ENDPOINT = "kms.internal.tencentcloudapi.com";
}

KmsClient::KmsClient(const Credential &credential, const string &region) :
    KmsClient(credential, region, ClientProfile())
{
}

KmsClient::KmsClient(const Credential &credential, const string &region, const ClientProfile &profile) :
    AbstractClient(ENDPOINT, VERSION, credential, region, profile)
{
}


KmsClient::CancelKeyDeletionOutcome KmsClient::CancelKeyDeletion(const CancelKeyDeletionRequest &request)
{
    auto outcome = MakeRequest(request, "CancelKeyDeletion");
    if (outcome.IsSuccess())
    {
        auto r = outcome.GetResult();
        string payload = string(r.Body(), r.BodySize());
        CancelKeyDeletionResponse rsp = CancelKeyDeletionResponse();
        auto o = rsp.Deserialize(payload);
        if (o.IsSuccess())
            return CancelKeyDeletionOutcome(rsp);
        else
            return CancelKeyDeletionOutcome(o.GetError());
    }
    else
    {
        return CancelKeyDeletionOutcome(outcome.GetError());
    }
}

void KmsClient::CancelKeyDeletionAsync(const CancelKeyDeletionRequest& request, const CancelKeyDeletionAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context)
{
    auto fn = [this, request, handler, context]()
    {
        handler(this, request, this->CancelKeyDeletion(request), context);
    };

    Executor::GetInstance()->Submit(new Runnable(fn));
}

KmsClient::CancelKeyDeletionOutcomeCallable KmsClient::CancelKeyDeletionCallable(const CancelKeyDeletionRequest &request)
{
    auto task = std::make_shared<std::packaged_task<CancelKeyDeletionOutcome()>>(
        [this, request]()
        {
            return this->CancelKeyDeletion(request);
        }
    );

    Executor::GetInstance()->Submit(new Runnable([task]() { (*task)(); }));
    return task->get_future();
}

KmsClient::CreateKeyOutcome KmsClient::CreateKey(const CreateKeyRequest &request)
{
    auto outcome = MakeRequest(request, "CreateKey");
    if (outcome.IsSuccess())
    {
        auto r = outcome.GetResult();
        string payload = string(r.Body(), r.BodySize());
        CreateKeyResponse rsp = CreateKeyResponse();
        auto o = rsp.Deserialize(payload);
        if (o.IsSuccess())
            return CreateKeyOutcome(rsp);
        else
            return CreateKeyOutcome(o.GetError());
    }
    else
    {
        return CreateKeyOutcome(outcome.GetError());
    }
}

void KmsClient::CreateKeyAsync(const CreateKeyRequest& request, const CreateKeyAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context)
{
    auto fn = [this, request, handler, context]()
    {
        handler(this, request, this->CreateKey(request), context);
    };

    Executor::GetInstance()->Submit(new Runnable(fn));
}

KmsClient::CreateKeyOutcomeCallable KmsClient::CreateKeyCallable(const CreateKeyRequest &request)
{
    auto task = std::make_shared<std::packaged_task<CreateKeyOutcome()>>(
        [this, request]()
        {
            return this->CreateKey(request);
        }
    );

    Executor::GetInstance()->Submit(new Runnable([task]() { (*task)(); }));
    return task->get_future();
}

KmsClient::DecryptOutcome KmsClient::Decrypt(const DecryptRequest &request)
{
    auto outcome = MakeRequest(request, "Decrypt");
    if (outcome.IsSuccess())
    {
        auto r = outcome.GetResult();
        string payload = string(r.Body(), r.BodySize());
        DecryptResponse rsp = DecryptResponse();
        auto o = rsp.Deserialize(payload);
        if (o.IsSuccess())
            return DecryptOutcome(rsp);
        else
            return DecryptOutcome(o.GetError());
    }
    else
    {
        return DecryptOutcome(outcome.GetError());
    }
}

void KmsClient::DecryptAsync(const DecryptRequest& request, const DecryptAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context)
{
    auto fn = [this, request, handler, context]()
    {
        handler(this, request, this->Decrypt(request), context);
    };

    Executor::GetInstance()->Submit(new Runnable(fn));
}

KmsClient::DecryptOutcomeCallable KmsClient::DecryptCallable(const DecryptRequest &request)
{
    auto task = std::make_shared<std::packaged_task<DecryptOutcome()>>(
        [this, request]()
        {
            return this->Decrypt(request);
        }
    );

    Executor::GetInstance()->Submit(new Runnable([task]() { (*task)(); }));
    return task->get_future();
}

KmsClient::DescribeKeyOutcome KmsClient::DescribeKey(const DescribeKeyRequest &request)
{
    auto outcome = MakeRequest(request, "DescribeKey");
    if (outcome.IsSuccess())
    {
        auto r = outcome.GetResult();
        string payload = string(r.Body(), r.BodySize());
        DescribeKeyResponse rsp = DescribeKeyResponse();
        auto o = rsp.Deserialize(payload);
        if (o.IsSuccess())
            return DescribeKeyOutcome(rsp);
        else
            return DescribeKeyOutcome(o.GetError());
    }
    else
    {
        return DescribeKeyOutcome(outcome.GetError());
    }
}

void KmsClient::DescribeKeyAsync(const DescribeKeyRequest& request, const DescribeKeyAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context)
{
    auto fn = [this, request, handler, context]()
    {
        handler(this, request, this->DescribeKey(request), context);
    };

    Executor::GetInstance()->Submit(new Runnable(fn));
}

KmsClient::DescribeKeyOutcomeCallable KmsClient::DescribeKeyCallable(const DescribeKeyRequest &request)
{
    auto task = std::make_shared<std::packaged_task<DescribeKeyOutcome()>>(
        [this, request]()
        {
            return this->DescribeKey(request);
        }
    );

    Executor::GetInstance()->Submit(new Runnable([task]() { (*task)(); }));
    return task->get_future();
}

KmsClient::DescribeKeysOutcome KmsClient::DescribeKeys(const DescribeKeysRequest &request)
{
    auto outcome = MakeRequest(request, "DescribeKeys");
    if (outcome.IsSuccess())
    {
        auto r = outcome.GetResult();
        string payload = string(r.Body(), r.BodySize());
        DescribeKeysResponse rsp = DescribeKeysResponse();
        auto o = rsp.Deserialize(payload);
        if (o.IsSuccess())
            return DescribeKeysOutcome(rsp);
        else
            return DescribeKeysOutcome(o.GetError());
    }
    else
    {
        return DescribeKeysOutcome(outcome.GetError());
    }
}

void KmsClient::DescribeKeysAsync(const DescribeKeysRequest& request, const DescribeKeysAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context)
{
    auto fn = [this, request, handler, context]()
    {
        handler(this, request, this->DescribeKeys(request), context);
    };

    Executor::GetInstance()->Submit(new Runnable(fn));
}

KmsClient::DescribeKeysOutcomeCallable KmsClient::DescribeKeysCallable(const DescribeKeysRequest &request)
{
    auto task = std::make_shared<std::packaged_task<DescribeKeysOutcome()>>(
        [this, request]()
        {
            return this->DescribeKeys(request);
        }
    );

    Executor::GetInstance()->Submit(new Runnable([task]() { (*task)(); }));
    return task->get_future();
}

KmsClient::DisableKeyOutcome KmsClient::DisableKey(const DisableKeyRequest &request)
{
    auto outcome = MakeRequest(request, "DisableKey");
    if (outcome.IsSuccess())
    {
        auto r = outcome.GetResult();
        string payload = string(r.Body(), r.BodySize());
        DisableKeyResponse rsp = DisableKeyResponse();
        auto o = rsp.Deserialize(payload);
        if (o.IsSuccess())
            return DisableKeyOutcome(rsp);
        else
            return DisableKeyOutcome(o.GetError());
    }
    else
    {
        return DisableKeyOutcome(outcome.GetError());
    }
}

void KmsClient::DisableKeyAsync(const DisableKeyRequest& request, const DisableKeyAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context)
{
    auto fn = [this, request, handler, context]()
    {
        handler(this, request, this->DisableKey(request), context);
    };

    Executor::GetInstance()->Submit(new Runnable(fn));
}

KmsClient::DisableKeyOutcomeCallable KmsClient::DisableKeyCallable(const DisableKeyRequest &request)
{
    auto task = std::make_shared<std::packaged_task<DisableKeyOutcome()>>(
        [this, request]()
        {
            return this->DisableKey(request);
        }
    );

    Executor::GetInstance()->Submit(new Runnable([task]() { (*task)(); }));
    return task->get_future();
}

KmsClient::DisableKeyRotationOutcome KmsClient::DisableKeyRotation(const DisableKeyRotationRequest &request)
{
    auto outcome = MakeRequest(request, "DisableKeyRotation");
    if (outcome.IsSuccess())
    {
        auto r = outcome.GetResult();
        string payload = string(r.Body(), r.BodySize());
        DisableKeyRotationResponse rsp = DisableKeyRotationResponse();
        auto o = rsp.Deserialize(payload);
        if (o.IsSuccess())
            return DisableKeyRotationOutcome(rsp);
        else
            return DisableKeyRotationOutcome(o.GetError());
    }
    else
    {
        return DisableKeyRotationOutcome(outcome.GetError());
    }
}

void KmsClient::DisableKeyRotationAsync(const DisableKeyRotationRequest& request, const DisableKeyRotationAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context)
{
    auto fn = [this, request, handler, context]()
    {
        handler(this, request, this->DisableKeyRotation(request), context);
    };

    Executor::GetInstance()->Submit(new Runnable(fn));
}

KmsClient::DisableKeyRotationOutcomeCallable KmsClient::DisableKeyRotationCallable(const DisableKeyRotationRequest &request)
{
    auto task = std::make_shared<std::packaged_task<DisableKeyRotationOutcome()>>(
        [this, request]()
        {
            return this->DisableKeyRotation(request);
        }
    );

    Executor::GetInstance()->Submit(new Runnable([task]() { (*task)(); }));
    return task->get_future();
}

KmsClient::DisableKeysOutcome KmsClient::DisableKeys(const DisableKeysRequest &request)
{
    auto outcome = MakeRequest(request, "DisableKeys");
    if (outcome.IsSuccess())
    {
        auto r = outcome.GetResult();
        string payload = string(r.Body(), r.BodySize());
        DisableKeysResponse rsp = DisableKeysResponse();
        auto o = rsp.Deserialize(payload);
        if (o.IsSuccess())
            return DisableKeysOutcome(rsp);
        else
            return DisableKeysOutcome(o.GetError());
    }
    else
    {
        return DisableKeysOutcome(outcome.GetError());
    }
}

void KmsClient::DisableKeysAsync(const DisableKeysRequest& request, const DisableKeysAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context)
{
    auto fn = [this, request, handler, context]()
    {
        handler(this, request, this->DisableKeys(request), context);
    };

    Executor::GetInstance()->Submit(new Runnable(fn));
}

KmsClient::DisableKeysOutcomeCallable KmsClient::DisableKeysCallable(const DisableKeysRequest &request)
{
    auto task = std::make_shared<std::packaged_task<DisableKeysOutcome()>>(
        [this, request]()
        {
            return this->DisableKeys(request);
        }
    );

    Executor::GetInstance()->Submit(new Runnable([task]() { (*task)(); }));
    return task->get_future();
}

KmsClient::EnableKeyOutcome KmsClient::EnableKey(const EnableKeyRequest &request)
{
    auto outcome = MakeRequest(request, "EnableKey");
    if (outcome.IsSuccess())
    {
        auto r = outcome.GetResult();
        string payload = string(r.Body(), r.BodySize());
        EnableKeyResponse rsp = EnableKeyResponse();
        auto o = rsp.Deserialize(payload);
        if (o.IsSuccess())
            return EnableKeyOutcome(rsp);
        else
            return EnableKeyOutcome(o.GetError());
    }
    else
    {
        return EnableKeyOutcome(outcome.GetError());
    }
}

void KmsClient::EnableKeyAsync(const EnableKeyRequest& request, const EnableKeyAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context)
{
    auto fn = [this, request, handler, context]()
    {
        handler(this, request, this->EnableKey(request), context);
    };

    Executor::GetInstance()->Submit(new Runnable(fn));
}

KmsClient::EnableKeyOutcomeCallable KmsClient::EnableKeyCallable(const EnableKeyRequest &request)
{
    auto task = std::make_shared<std::packaged_task<EnableKeyOutcome()>>(
        [this, request]()
        {
            return this->EnableKey(request);
        }
    );

    Executor::GetInstance()->Submit(new Runnable([task]() { (*task)(); }));
    return task->get_future();
}

KmsClient::EnableKeyRotationOutcome KmsClient::EnableKeyRotation(const EnableKeyRotationRequest &request)
{
    auto outcome = MakeRequest(request, "EnableKeyRotation");
    if (outcome.IsSuccess())
    {
        auto r = outcome.GetResult();
        string payload = string(r.Body(), r.BodySize());
        EnableKeyRotationResponse rsp = EnableKeyRotationResponse();
        auto o = rsp.Deserialize(payload);
        if (o.IsSuccess())
            return EnableKeyRotationOutcome(rsp);
        else
            return EnableKeyRotationOutcome(o.GetError());
    }
    else
    {
        return EnableKeyRotationOutcome(outcome.GetError());
    }
}

void KmsClient::EnableKeyRotationAsync(const EnableKeyRotationRequest& request, const EnableKeyRotationAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context)
{
    auto fn = [this, request, handler, context]()
    {
        handler(this, request, this->EnableKeyRotation(request), context);
    };

    Executor::GetInstance()->Submit(new Runnable(fn));
}

KmsClient::EnableKeyRotationOutcomeCallable KmsClient::EnableKeyRotationCallable(const EnableKeyRotationRequest &request)
{
    auto task = std::make_shared<std::packaged_task<EnableKeyRotationOutcome()>>(
        [this, request]()
        {
            return this->EnableKeyRotation(request);
        }
    );

    Executor::GetInstance()->Submit(new Runnable([task]() { (*task)(); }));
    return task->get_future();
}

KmsClient::EnableKeysOutcome KmsClient::EnableKeys(const EnableKeysRequest &request)
{
    auto outcome = MakeRequest(request, "EnableKeys");
    if (outcome.IsSuccess())
    {
        auto r = outcome.GetResult();
        string payload = string(r.Body(), r.BodySize());
        EnableKeysResponse rsp = EnableKeysResponse();
        auto o = rsp.Deserialize(payload);
        if (o.IsSuccess())
            return EnableKeysOutcome(rsp);
        else
            return EnableKeysOutcome(o.GetError());
    }
    else
    {
        return EnableKeysOutcome(outcome.GetError());
    }
}

void KmsClient::EnableKeysAsync(const EnableKeysRequest& request, const EnableKeysAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context)
{
    auto fn = [this, request, handler, context]()
    {
        handler(this, request, this->EnableKeys(request), context);
    };

    Executor::GetInstance()->Submit(new Runnable(fn));
}

KmsClient::EnableKeysOutcomeCallable KmsClient::EnableKeysCallable(const EnableKeysRequest &request)
{
    auto task = std::make_shared<std::packaged_task<EnableKeysOutcome()>>(
        [this, request]()
        {
            return this->EnableKeys(request);
        }
    );

    Executor::GetInstance()->Submit(new Runnable([task]() { (*task)(); }));
    return task->get_future();
}

KmsClient::EncryptOutcome KmsClient::Encrypt(const EncryptRequest &request)
{
    auto outcome = MakeRequest(request, "Encrypt");
    if (outcome.IsSuccess())
    {
        auto r = outcome.GetResult();
        string payload = string(r.Body(), r.BodySize());
        EncryptResponse rsp = EncryptResponse();
        auto o = rsp.Deserialize(payload);
        if (o.IsSuccess())
            return EncryptOutcome(rsp);
        else
            return EncryptOutcome(o.GetError());
    }
    else
    {
        return EncryptOutcome(outcome.GetError());
    }
}

void KmsClient::EncryptAsync(const EncryptRequest& request, const EncryptAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context)
{
    auto fn = [this, request, handler, context]()
    {
        handler(this, request, this->Encrypt(request), context);
    };

    Executor::GetInstance()->Submit(new Runnable(fn));
}

KmsClient::EncryptOutcomeCallable KmsClient::EncryptCallable(const EncryptRequest &request)
{
    auto task = std::make_shared<std::packaged_task<EncryptOutcome()>>(
        [this, request]()
        {
            return this->Encrypt(request);
        }
    );

    Executor::GetInstance()->Submit(new Runnable([task]() { (*task)(); }));
    return task->get_future();
}

KmsClient::GenerateDataKeyOutcome KmsClient::GenerateDataKey(const GenerateDataKeyRequest &request)
{
    auto outcome = MakeRequest(request, "GenerateDataKey");
    if (outcome.IsSuccess())
    {
        auto r = outcome.GetResult();
        string payload = string(r.Body(), r.BodySize());
        GenerateDataKeyResponse rsp = GenerateDataKeyResponse();
        auto o = rsp.Deserialize(payload);
        if (o.IsSuccess())
            return GenerateDataKeyOutcome(rsp);
        else
            return GenerateDataKeyOutcome(o.GetError());
    }
    else
    {
        return GenerateDataKeyOutcome(outcome.GetError());
    }
}

void KmsClient::GenerateDataKeyAsync(const GenerateDataKeyRequest& request, const GenerateDataKeyAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context)
{
    auto fn = [this, request, handler, context]()
    {
        handler(this, request, this->GenerateDataKey(request), context);
    };

    Executor::GetInstance()->Submit(new Runnable(fn));
}

KmsClient::GenerateDataKeyOutcomeCallable KmsClient::GenerateDataKeyCallable(const GenerateDataKeyRequest &request)
{
    auto task = std::make_shared<std::packaged_task<GenerateDataKeyOutcome()>>(
        [this, request]()
        {
            return this->GenerateDataKey(request);
        }
    );

    Executor::GetInstance()->Submit(new Runnable([task]() { (*task)(); }));
    return task->get_future();
}

KmsClient::GetKeyRotationStatusOutcome KmsClient::GetKeyRotationStatus(const GetKeyRotationStatusRequest &request)
{
    auto outcome = MakeRequest(request, "GetKeyRotationStatus");
    if (outcome.IsSuccess())
    {
        auto r = outcome.GetResult();
        string payload = string(r.Body(), r.BodySize());
        GetKeyRotationStatusResponse rsp = GetKeyRotationStatusResponse();
        auto o = rsp.Deserialize(payload);
        if (o.IsSuccess())
            return GetKeyRotationStatusOutcome(rsp);
        else
            return GetKeyRotationStatusOutcome(o.GetError());
    }
    else
    {
        return GetKeyRotationStatusOutcome(outcome.GetError());
    }
}

void KmsClient::GetKeyRotationStatusAsync(const GetKeyRotationStatusRequest& request, const GetKeyRotationStatusAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context)
{
    auto fn = [this, request, handler, context]()
    {
        handler(this, request, this->GetKeyRotationStatus(request), context);
    };

    Executor::GetInstance()->Submit(new Runnable(fn));
}

KmsClient::GetKeyRotationStatusOutcomeCallable KmsClient::GetKeyRotationStatusCallable(const GetKeyRotationStatusRequest &request)
{
    auto task = std::make_shared<std::packaged_task<GetKeyRotationStatusOutcome()>>(
        [this, request]()
        {
            return this->GetKeyRotationStatus(request);
        }
    );

    Executor::GetInstance()->Submit(new Runnable([task]() { (*task)(); }));
    return task->get_future();
}

KmsClient::GetServiceStatusOutcome KmsClient::GetServiceStatus(const GetServiceStatusRequest &request)
{
    auto outcome = MakeRequest(request, "GetServiceStatus");
    if (outcome.IsSuccess())
    {
        auto r = outcome.GetResult();
        string payload = string(r.Body(), r.BodySize());
        GetServiceStatusResponse rsp = GetServiceStatusResponse();
        auto o = rsp.Deserialize(payload);
        if (o.IsSuccess())
            return GetServiceStatusOutcome(rsp);
        else
            return GetServiceStatusOutcome(o.GetError());
    }
    else
    {
        return GetServiceStatusOutcome(outcome.GetError());
    }
}

void KmsClient::GetServiceStatusAsync(const GetServiceStatusRequest& request, const GetServiceStatusAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context)
{
    auto fn = [this, request, handler, context]()
    {
        handler(this, request, this->GetServiceStatus(request), context);
    };

    Executor::GetInstance()->Submit(new Runnable(fn));
}

KmsClient::GetServiceStatusOutcomeCallable KmsClient::GetServiceStatusCallable(const GetServiceStatusRequest &request)
{
    auto task = std::make_shared<std::packaged_task<GetServiceStatusOutcome()>>(
        [this, request]()
        {
            return this->GetServiceStatus(request);
        }
    );

    Executor::GetInstance()->Submit(new Runnable([task]() { (*task)(); }));
    return task->get_future();
}

KmsClient::ListKeyDetailOutcome KmsClient::ListKeyDetail(const ListKeyDetailRequest &request)
{
    auto outcome = MakeRequest(request, "ListKeyDetail");
    if (outcome.IsSuccess())
    {
        auto r = outcome.GetResult();
        string payload = string(r.Body(), r.BodySize());
        ListKeyDetailResponse rsp = ListKeyDetailResponse();
        auto o = rsp.Deserialize(payload);
        if (o.IsSuccess())
            return ListKeyDetailOutcome(rsp);
        else
            return ListKeyDetailOutcome(o.GetError());
    }
    else
    {
        return ListKeyDetailOutcome(outcome.GetError());
    }
}

void KmsClient::ListKeyDetailAsync(const ListKeyDetailRequest& request, const ListKeyDetailAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context)
{
    auto fn = [this, request, handler, context]()
    {
        handler(this, request, this->ListKeyDetail(request), context);
    };

    Executor::GetInstance()->Submit(new Runnable(fn));
}

KmsClient::ListKeyDetailOutcomeCallable KmsClient::ListKeyDetailCallable(const ListKeyDetailRequest &request)
{
    auto task = std::make_shared<std::packaged_task<ListKeyDetailOutcome()>>(
        [this, request]()
        {
            return this->ListKeyDetail(request);
        }
    );

    Executor::GetInstance()->Submit(new Runnable([task]() { (*task)(); }));
    return task->get_future();
}

KmsClient::ListKeysOutcome KmsClient::ListKeys(const ListKeysRequest &request)
{
    auto outcome = MakeRequest(request, "ListKeys");
    if (outcome.IsSuccess())
    {
        auto r = outcome.GetResult();
        string payload = string(r.Body(), r.BodySize());
        ListKeysResponse rsp = ListKeysResponse();
        auto o = rsp.Deserialize(payload);
        if (o.IsSuccess())
            return ListKeysOutcome(rsp);
        else
            return ListKeysOutcome(o.GetError());
    }
    else
    {
        return ListKeysOutcome(outcome.GetError());
    }
}

void KmsClient::ListKeysAsync(const ListKeysRequest& request, const ListKeysAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context)
{
    auto fn = [this, request, handler, context]()
    {
        handler(this, request, this->ListKeys(request), context);
    };

    Executor::GetInstance()->Submit(new Runnable(fn));
}

KmsClient::ListKeysOutcomeCallable KmsClient::ListKeysCallable(const ListKeysRequest &request)
{
    auto task = std::make_shared<std::packaged_task<ListKeysOutcome()>>(
        [this, request]()
        {
            return this->ListKeys(request);
        }
    );

    Executor::GetInstance()->Submit(new Runnable([task]() { (*task)(); }));
    return task->get_future();
}

KmsClient::ReEncryptOutcome KmsClient::ReEncrypt(const ReEncryptRequest &request)
{
    auto outcome = MakeRequest(request, "ReEncrypt");
    if (outcome.IsSuccess())
    {
        auto r = outcome.GetResult();
        string payload = string(r.Body(), r.BodySize());
        ReEncryptResponse rsp = ReEncryptResponse();
        auto o = rsp.Deserialize(payload);
        if (o.IsSuccess())
            return ReEncryptOutcome(rsp);
        else
            return ReEncryptOutcome(o.GetError());
    }
    else
    {
        return ReEncryptOutcome(outcome.GetError());
    }
}

void KmsClient::ReEncryptAsync(const ReEncryptRequest& request, const ReEncryptAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context)
{
    auto fn = [this, request, handler, context]()
    {
        handler(this, request, this->ReEncrypt(request), context);
    };

    Executor::GetInstance()->Submit(new Runnable(fn));
}

KmsClient::ReEncryptOutcomeCallable KmsClient::ReEncryptCallable(const ReEncryptRequest &request)
{
    auto task = std::make_shared<std::packaged_task<ReEncryptOutcome()>>(
        [this, request]()
        {
            return this->ReEncrypt(request);
        }
    );

    Executor::GetInstance()->Submit(new Runnable([task]() { (*task)(); }));
    return task->get_future();
}

KmsClient::ScheduleKeyDeletionOutcome KmsClient::ScheduleKeyDeletion(const ScheduleKeyDeletionRequest &request)
{
    auto outcome = MakeRequest(request, "ScheduleKeyDeletion");
    if (outcome.IsSuccess())
    {
        auto r = outcome.GetResult();
        string payload = string(r.Body(), r.BodySize());
        ScheduleKeyDeletionResponse rsp = ScheduleKeyDeletionResponse();
        auto o = rsp.Deserialize(payload);
        if (o.IsSuccess())
            return ScheduleKeyDeletionOutcome(rsp);
        else
            return ScheduleKeyDeletionOutcome(o.GetError());
    }
    else
    {
        return ScheduleKeyDeletionOutcome(outcome.GetError());
    }
}

void KmsClient::ScheduleKeyDeletionAsync(const ScheduleKeyDeletionRequest& request, const ScheduleKeyDeletionAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context)
{
    auto fn = [this, request, handler, context]()
    {
        handler(this, request, this->ScheduleKeyDeletion(request), context);
    };

    Executor::GetInstance()->Submit(new Runnable(fn));
}

KmsClient::ScheduleKeyDeletionOutcomeCallable KmsClient::ScheduleKeyDeletionCallable(const ScheduleKeyDeletionRequest &request)
{
    auto task = std::make_shared<std::packaged_task<ScheduleKeyDeletionOutcome()>>(
        [this, request]()
        {
            return this->ScheduleKeyDeletion(request);
        }
    );

    Executor::GetInstance()->Submit(new Runnable([task]() { (*task)(); }));
    return task->get_future();
}

KmsClient::UpdateAliasOutcome KmsClient::UpdateAlias(const UpdateAliasRequest &request)
{
    auto outcome = MakeRequest(request, "UpdateAlias");
    if (outcome.IsSuccess())
    {
        auto r = outcome.GetResult();
        string payload = string(r.Body(), r.BodySize());
        UpdateAliasResponse rsp = UpdateAliasResponse();
        auto o = rsp.Deserialize(payload);
        if (o.IsSuccess())
            return UpdateAliasOutcome(rsp);
        else
            return UpdateAliasOutcome(o.GetError());
    }
    else
    {
        return UpdateAliasOutcome(outcome.GetError());
    }
}

void KmsClient::UpdateAliasAsync(const UpdateAliasRequest& request, const UpdateAliasAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context)
{
    auto fn = [this, request, handler, context]()
    {
        handler(this, request, this->UpdateAlias(request), context);
    };

    Executor::GetInstance()->Submit(new Runnable(fn));
}

KmsClient::UpdateAliasOutcomeCallable KmsClient::UpdateAliasCallable(const UpdateAliasRequest &request)
{
    auto task = std::make_shared<std::packaged_task<UpdateAliasOutcome()>>(
        [this, request]()
        {
            return this->UpdateAlias(request);
        }
    );

    Executor::GetInstance()->Submit(new Runnable([task]() { (*task)(); }));
    return task->get_future();
}

KmsClient::UpdateKeyDescriptionOutcome KmsClient::UpdateKeyDescription(const UpdateKeyDescriptionRequest &request)
{
    auto outcome = MakeRequest(request, "UpdateKeyDescription");
    if (outcome.IsSuccess())
    {
        auto r = outcome.GetResult();
        string payload = string(r.Body(), r.BodySize());
        UpdateKeyDescriptionResponse rsp = UpdateKeyDescriptionResponse();
        auto o = rsp.Deserialize(payload);
        if (o.IsSuccess())
            return UpdateKeyDescriptionOutcome(rsp);
        else
            return UpdateKeyDescriptionOutcome(o.GetError());
    }
    else
    {
        return UpdateKeyDescriptionOutcome(outcome.GetError());
    }
}

void KmsClient::UpdateKeyDescriptionAsync(const UpdateKeyDescriptionRequest& request, const UpdateKeyDescriptionAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context)
{
    auto fn = [this, request, handler, context]()
    {
        handler(this, request, this->UpdateKeyDescription(request), context);
    };

    Executor::GetInstance()->Submit(new Runnable(fn));
}

KmsClient::UpdateKeyDescriptionOutcomeCallable KmsClient::UpdateKeyDescriptionCallable(const UpdateKeyDescriptionRequest &request)
{
    auto task = std::make_shared<std::packaged_task<UpdateKeyDescriptionOutcome()>>(
        [this, request]()
        {
            return this->UpdateKeyDescription(request);
        }
    );

    Executor::GetInstance()->Submit(new Runnable([task]() { (*task)(); }));
    return task->get_future();
}


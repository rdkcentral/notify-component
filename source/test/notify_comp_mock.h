/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2024 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#ifndef NOTFIY_COMP_MOCK_H
#define NOTFIY_COMP_MOCK_H

#define EVENT_QUEUE_NAME  "/Notify_queue"
#define MAX_SIZE    2048
#define NotifyMask_WEBPA 0x00000001
#define NotifyMask_DMCLI 0x00000002
#define NotifyMask_SNMP 0x00000004
#define NotifyMask_TR069 0x00000008
#define NotifyMask_WIFI 0x00000010
#define CONNECTED_CLIENT_STR "Connected-Client"
#define NOTIFY_PARAM_FILE "/tmp/.NotifyParamListCache"


#include "gtest/gtest.h"

#include <mocks/mock_safec_lib.h>
#include <mocks/mock_usertime.h>
#include <mocks/mock_rbus.h>
#include <mocks/mock_trace.h>
#include <mocks/mock_ansc_memory.h>
#include <mocks/mock_base_api.h>
#include <mocks/mock_mqhandler.h>
#include <mocks/mock_pthread.h>
#include <mocks/mock_file_io.h>
#include <ansc_debug_wrapper_base.h>

extern ANSC_HANDLE bus_handle;
extern char* g_NotifyCompName;
extern FileIOMock *g_fileIOMock;

class NotifyCompBaseTestFixture : public ::testing::Test {
    protected:
        SafecLibMock mockedSafecLibMock;
        UserTimeMock mockedUserTime;
        TraceMock mockedTrace;
        rbusMock mockedRbus;
        AnscMemoryMock mockedAnscMemory;
        BaseAPIMock mockedBaseAPI;
        PtdHandlerMock mockedPtdHandler;

        NotifyCompBaseTestFixture();
        virtual ~NotifyCompBaseTestFixture();
        virtual void SetUp() override;
        virtual void TearDown() override;

        void TestBody() override;

};

#endif //NOTFIY_COMP_MOCK_H
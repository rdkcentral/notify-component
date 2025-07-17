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

#include <gmock/gmock.h>
#include "notify_comp_mock.h"

SafecLibMock *g_safecLibMock = NULL;
UserTimeMock *g_usertimeMock = NULL;
TraceMock * g_traceMock = NULL;
rbusMock *g_rbusMock = NULL;
AnscMemoryMock * g_anscMemoryMock = NULL;
BaseAPIMock * g_baseapiMock = NULL;
MQHandlerMock* g_MQHandlerMock = NULL;
PtdHandlerMock* g_PtdHandlerMock = NULL;
FileIOMock * g_fileIOMock = NULL;

ANSC_HANDLE bus_handle = NULL;
char* g_NotifyCompName = {};

NotifyCompBaseTestFixture::NotifyCompBaseTestFixture() {
    g_safecLibMock = new SafecLibMock;
    g_usertimeMock = new UserTimeMock;
    g_traceMock = new TraceMock;
    g_rbusMock    = new rbusMock;
    g_anscMemoryMock = new ::testing::NiceMock<AnscMemoryMock>();
    g_baseapiMock  = new BaseAPIMock;
    g_MQHandlerMock = new MQHandlerMock;
    g_fileIOMock = new FileIOMock;
    g_PtdHandlerMock = new ::testing::NiceMock<PtdHandlerMock>();
}

NotifyCompBaseTestFixture::~NotifyCompBaseTestFixture() {
    delete g_safecLibMock;
    delete g_usertimeMock;
    delete g_traceMock;
    delete g_rbusMock;
    delete g_anscMemoryMock;
    delete g_baseapiMock;
    delete g_MQHandlerMock;
    delete g_PtdHandlerMock;
    delete g_fileIOMock;

    g_safecLibMock = nullptr;
    g_usertimeMock = nullptr;
    g_rbusMock = nullptr;
    g_anscMemoryMock = nullptr;
    g_baseapiMock = nullptr;
    g_MQHandlerMock = nullptr;
    g_PtdHandlerMock= nullptr;
    g_fileIOMock = nullptr;
}
void NotifyCompBaseTestFixture::SetUp() {}
void NotifyCompBaseTestFixture::TearDown() {}
void NotifyCompBaseTestFixture::TestBody() {}
// end of file
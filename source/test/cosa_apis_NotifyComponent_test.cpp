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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <mqueue.h>
#include <sstream>
#include <cstdio>  // for freopen
#include <mocks/mock_file_io.h>
#include "notify_comp_mock.h"

using ::testing::Return;
using ::testing::DoAll;
using ::testing::StrEq;
using ::testing::SetArgPointee;
using ::testing::_;
using ::testing::Invoke;
using ::testing::WithArg;

extern "C"
{
    #include "cosa_apis_NotifyComponent.h"

}


extern SafecLibMock* g_safecLibMock;
extern MQHandlerMock* g_MQHandlerMock;
extern PtdHandlerMock * g_PtdHandlerMock;
extern BaseAPIMock * g_baseapiMock;
extern FileIOMock *g_fileIOMock;

int numLoops;
int NullTests;
// In-file Fopen Mock implementation
class FopenMock {
public:
    MOCK_METHOD(FILE*, fopen_mock, (const char* filename, const char* mode), ());
};
FopenMock* g_fopenMock = nullptr;
extern "C" FILE* fopen_mock(const char* filename, const char* mode)
{
    if (g_fopenMock) {
        return g_fopenMock->fopen_mock(filename, mode);
    }
    return std::fopen(filename, mode);
}

class NotifyCompTestFixture : public NotifyCompBaseTestFixture {
protected:
    void SetUp() {
        g_fopenMock = new FopenMock();
        numLoops = 1;
    }
    void TearDown() {
        delete g_fopenMock;

        // Command to remove all .gcda files in the project directory and subdirectories
        int result = std::system("find . -name '*.gcda' -exec rm -f {} +");

        if (result != 0) {
            std::cerr << "Error: Failed to clean up .gcda files!" << std::endl;
        }

    }
};

TEST(NotifyCompTests, NotifyComponent_GetParamUlongValue) {

    ANSC_HANDLE hInsContext = nullptr;
    char ParamName[] = "TestParam";
    ULONG puLong = 0;

    BOOL result = NotifyComponent_GetParamUlongValue(hInsContext, ParamName, &puLong);
    EXPECT_TRUE(result);
}

TEST(NotifyCompTests, NotifyComponent_SetParamUlongValue) {

    ANSC_HANDLE hInsContext = nullptr;
    char ParamName[] = "TestParam";
    ULONG puLong = 0;

    BOOL result = NotifyComponent_SetParamUlongValue(hInsContext, ParamName, puLong);
    EXPECT_TRUE(result);
}

TEST_F(NotifyCompTestFixture, SetParamStringValue_SetNotifiParamName) {
    ANSC_HANDLE hInsContext = nullptr;
    char ParamName[] = "SetNotifi_ParamName";
    char pString[] = "TestString";
    int ind = 0;

    // Create the message queue
    mqd_t mq;
    struct mq_attr attr;


    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(_, _, _, _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(ind), Return(EOK)));

    // Set up the EXPECT_CALL for mq_open
    EXPECT_CALL(*g_MQHandlerMock, mq_open(StrEq(EVENT_QUEUE_NAME),_, _, _))
        .Times(2)
        .WillRepeatedly(Return((mqd_t)0)); // Mock a valid message queue descriptor

    // Add EXPECT_CALL for strcpy_s in MsgPosttoQueue
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
        .WillOnce(Return(EOK)); // Mock successful strcpy_s

    EXPECT_CALL(*g_MQHandlerMock, mq_send(_, _, _, _))
        .WillOnce(Return(0)); // Mock successful message send

    EXPECT_CALL(*g_MQHandlerMock, mq_close(_))
        .Times(2)
        .WillRepeatedly(Return(0)); // Mock successful message queue close

    EXPECT_CALL(*g_MQHandlerMock, mq_unlink(StrEq(EVENT_QUEUE_NAME)))
        .WillOnce(Return(0)); // Mock successful message queue close

    EXPECT_TRUE(NotifyComponent_SetParamStringValue(hInsContext, ParamName, pString));


    // Second test case: Error in strcpy_s (EINVAL)
    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(_, _, _, _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(ind), Return(EOK)));

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
        .WillOnce(Return(EINVAL));  // Mock strcpy_s returning an error (EINVAL)

    EXPECT_TRUE(NotifyComponent_SetParamStringValue(hInsContext, ParamName, pString));

    // Clean up the message queue
    mq_close(mq);
    mq_unlink(EVENT_QUEUE_NAME);
}

TEST_F(NotifyCompTestFixture, SetParamStringValue_NotifiParamName_AddNotifyParam) {
    ANSC_HANDLE hInsContext = nullptr;
    char ParamName[] = "Notifi_ParamName";
    char pString[] = "param1,eRT.com.cisco.spvtg.ccsp.webpaagent,true";
    char p_notify_param_name[] = "param1";
    char p_write_id[] = "eRT.com.cisco.spvtg.ccsp.webpaagent";
    char PA_Name[]= "eRT.com.cisco.spvtg.ccsp.webpaagent";
    int ind = 0;

    FILE* mockFile = tmpfile();
    ASSERT_NE(mockFile, nullptr) << "tmpfile() Failed : Couldn't create temporary file";

    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq("SetNotifi_ParamName"), _, StrEq(ParamName), _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(1), Return(EOK)));

    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq("Notifi_ParamName"), _, StrEq(ParamName), _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(0), Return(EOK)));


    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq("true"), _, StrEq("true"), _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(0), Return(EOK)));

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
        .WillOnce(Return(EOK));

    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(_, _, StrEq(PA_Name), _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(0), Return(EOK)));

    EXPECT_CALL(*g_fopenMock, fopen_mock(StrEq(NOTIFY_PARAM_FILE), StrEq("w+")))
        .Times(1)
        .WillOnce(Return(mockFile));

    EXPECT_CALL(*g_fileIOMock, fclose(mockFile))
        .Times(1)
        .WillOnce(Return(0));

    // Act
    BOOL result = NotifyComponent_SetParamStringValue(hInsContext, ParamName, pString);

    // Assert
    EXPECT_TRUE(result);

}

TEST_F(NotifyCompTestFixture, SetParamStringValue_NotifiParamName_AddNotifyParam_TempNotNull) {
    ANSC_HANDLE hInsContext = nullptr;
    char ParamName[] = "Notifi_ParamName";
    char pString[] = "param1,eRT.com.cisco.spvtg.ccsp.webpaagent,true";
    char p_notify_param_name[] = "param1";
    char p_write_id[] = "eRT.com.cisco.spvtg.ccsp.webpaagent";
    char PA_Name[]= "eRT.com.cisco.spvtg.ccsp.webpaagent";
    int ind = 0;

    FILE* mockFile = tmpfile();
    ASSERT_NE(mockFile, nullptr) << "tmpfile() Failed : Couldn't create temporary file";

    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq("SetNotifi_ParamName"), _, StrEq(ParamName), _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(1), Return(EOK)));

    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq("Notifi_ParamName"), _, StrEq(ParamName), _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(0), Return(EOK)));


    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq("true"), _, StrEq("true"), _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(0), Return(EOK)));

    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq(p_notify_param_name), _, _, _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(0), Return(EOK)));

    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq(p_write_id), _, _, _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(0), Return(EOK)));

    EXPECT_CALL(*g_fopenMock, fopen_mock(StrEq(NOTIFY_PARAM_FILE), StrEq("w+")))
        .Times(1)
        .WillOnce(Return(mockFile));

    EXPECT_CALL(*g_fileIOMock, fclose(mockFile))
        .Times(1)
        .WillOnce(Return(0));

    // Act
    BOOL result = NotifyComponent_SetParamStringValue(hInsContext, ParamName, pString);

    // Assert
    EXPECT_TRUE(result);

}

TEST_F(NotifyCompTestFixture, SetParamStringValue_NotifiParamName_AddNotifyParam_TempNotNull2) {
    ANSC_HANDLE hInsContext = nullptr;
    char ParamName[] = "Notifi_ParamName";
    char pString[] = "param1,eRT.com.cisco.spvtg.ccsp.webpaagent,true";
    char p_notify_param_name[] = "param1";
    char p_write_id[] = "eRT.com.cisco.spvtg.ccsp.webpaagent";
    char PA_Name[]= "eRT.com.cisco.spvtg.ccsp.webpaagent";
    int ind = 0;

    FILE* mockFile = tmpfile();
    ASSERT_NE(mockFile, nullptr) << "tmpfile() Failed : Couldn't create temporary file";

    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq("SetNotifi_ParamName"), _, StrEq(ParamName), _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(1), Return(EOK)));

    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq("Notifi_ParamName"), _, StrEq(ParamName), _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(0), Return(EOK)));


    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq("true"), _, StrEq("true"), _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(0), Return(EOK)));

    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq(p_notify_param_name), _, _, _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(1), Return(EOK)));

    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq(p_write_id), _, _, _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(0), Return(EOK)));

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
        .WillOnce(Return(EOK));

    EXPECT_CALL(*g_fopenMock, fopen_mock(StrEq(NOTIFY_PARAM_FILE), StrEq("w+")))
        .Times(1)
        .WillOnce(Return(mockFile));

    EXPECT_CALL(*g_fileIOMock, fclose(mockFile))
        .Times(1)
        .WillOnce(Return(0));

    // Act
    BOOL result = NotifyComponent_SetParamStringValue(hInsContext, ParamName, pString);

    // Assert
    EXPECT_TRUE(result);

}

TEST_F(NotifyCompTestFixture, SetParamStringValue_NotifiParamName_AddNotifyParam_Found0) {
    ANSC_HANDLE hInsContext = nullptr;
    char ParamName[] = "Notifi_ParamName";
    char pString[] = "param2,eRT.com.cisco.spvtg.ccsp.webpaagent,true";
    char p_notify_param_name[] = "param2";
    char p_write_id[] = "eRT.com.cisco.spvtg.ccsp.webpaagent";
    char PA_Name[]= "eRT.com.cisco.spvtg.ccsp.webpaagent";
    int ind = 0;

    FILE* mockFile = tmpfile();
    ASSERT_NE(mockFile, nullptr) << "tmpfile() Failed : Couldn't create temporary file";

    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq("SetNotifi_ParamName"), _, StrEq(ParamName), _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(1), Return(EOK)));

    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq("Notifi_ParamName"), _, StrEq(ParamName), _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(0), Return(EOK)));


    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq("true"), _, StrEq("true"), _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(0), Return(EOK)));

    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq(p_notify_param_name), _, _, _, _, _))
        .Times(testing::AnyNumber())
        .WillRepeatedly(DoAll(SetArgPointee<3>(1), Return(EOK)));

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
        .WillOnce(Return(ESNULLP));

    // Act
    BOOL result = NotifyComponent_SetParamStringValue(hInsContext, ParamName, pString);

    // Assert
    EXPECT_TRUE(result);

}

TEST_F(NotifyCompTestFixture, SetParamStringValue_NotifiParamName_AddNotifyParam_PA_To_Mask_Default) {
    ANSC_HANDLE hInsContext = nullptr;
    char ParamName[] = "Notifi_ParamName";
    char pString[] = "param3,InvalidWriteID,true";
    char p_notify_param_name[] = "param3";
    char p_write_id[] = "InvalidWriteID";
    int ind = 0;

    FILE* mockFile = tmpfile();
    ASSERT_NE(mockFile, nullptr) << "tmpfile() Failed : Couldn't create temporary file";

    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq("SetNotifi_ParamName"), _, StrEq(ParamName), _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(1), Return(EOK)));

    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq("Notifi_ParamName"), _, StrEq(ParamName), _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(0), Return(EOK)));


    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq("true"), _, StrEq("true"), _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(0), Return(EOK)));

    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq(p_notify_param_name), _, _, _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(0), Return(EOK)));

    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq(p_write_id), _, _, _, _, _))
        .WillRepeatedly(DoAll(SetArgPointee<3>(1), Return(EOK)));

    EXPECT_CALL(*g_fopenMock, fopen_mock(StrEq(NOTIFY_PARAM_FILE), StrEq("w+")))
        .Times(1)
        .WillOnce(Return(mockFile));

    EXPECT_CALL(*g_fileIOMock, fclose(mockFile))
        .Times(1)
        .WillOnce(Return(0));

    // Act
    BOOL result = NotifyComponent_SetParamStringValue(hInsContext, ParamName, pString);

    // Assert
    EXPECT_TRUE(result);

}

TEST_F(NotifyCompTestFixture, Find_Param_ParameterNotFoundInDynamicList) {
    char param_name[] = "param1";
    char msg_str[] = "Connected-Client";

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , testing::_ , testing::_)).WillRepeatedly(testing::Return(EOK));

    Find_Param(param_name, msg_str);
}

TEST_F(NotifyCompTestFixture, SetParamStringValue_NotifiParamName_DelNotifyParam) {
    ANSC_HANDLE hInsContext = nullptr;
    char ParamName[] = "Notifi_ParamName";
    char pString[] = "param1,eRT.com.cisco.spvtg.ccsp.webpaagent,false";
    char p_notify_param_name[] = "param1";
    char p_write_id[] = "eRT.com.cisco.spvtg.ccsp.webpaagent";
    char add[] = "false";
    int ind = 0;
    FILE* mockFile = tmpfile();
    ASSERT_NE(mockFile, nullptr) << "tmpfile() Failed : Couldn't create temporary file";

    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq("SetNotifi_ParamName"), _, StrEq(ParamName), _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(1), Return(EOK)));

    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq("Notifi_ParamName"), _, StrEq(ParamName), _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(0), Return(EOK)));


    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq("true"), _, StrEq(add), _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(1), Return(EOK)));

    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq(p_notify_param_name), _, _, _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(0), Return(EOK)));

    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq(p_write_id), _, _, _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(0), Return(EOK)));

    EXPECT_CALL(*g_fopenMock, fopen_mock(StrEq(NOTIFY_PARAM_FILE), StrEq("w+")))
        .Times(1)
        .WillOnce(Return(mockFile));

    EXPECT_CALL(*g_fileIOMock, fclose(mockFile))
        .Times(1)
        .WillOnce(Return(0));

    BOOL result = NotifyComponent_SetParamStringValue(hInsContext, ParamName, pString);

    // Assert
    EXPECT_TRUE(result);
}

TEST_F(NotifyCompTestFixture, SetParamStringValue_NotifiParamName_DelNotifyParam_strcmp_err) {
    ANSC_HANDLE hInsContext = nullptr;
    char ParamName[] = "Notifi_ParamName";
    char pString[] = "param1,eRT.com.cisco.spvtg.ccsp.webpaagent,false";
    char p_notify_param_name[] = "param1";
    char p_write_id[] = "eRT.com.cisco.spvtg.ccsp.webpaagent";
    char add[] = "false";
    int ind = 0;
    FILE* mockFile = tmpfile();
    ASSERT_NE(mockFile, nullptr) << "tmpfile() Failed : Couldn't create temporary file";

    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq("SetNotifi_ParamName"), _, StrEq(ParamName), _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(1), Return(EOK)));

    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq("Notifi_ParamName"), _, StrEq(ParamName), _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(0), Return(EOK)));


    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq("true"), _, StrEq(add), _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(1), Return(EOK)));

    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq(p_notify_param_name), _, _, _, _, _))
        .Times(testing::AnyNumber())
        .WillRepeatedly(DoAll(SetArgPointee<3>(1), Return(EOK)));

    BOOL result = NotifyComponent_SetParamStringValue(hInsContext, ParamName, pString);

    // Assert
    EXPECT_TRUE(result);
}

TEST_F(NotifyCompTestFixture, UpdateNotifyParamFile_nullfptr) {
    EXPECT_CALL(*g_fopenMock, fopen_mock(_, _))
        .Times(1)
        .WillOnce(Return(nullptr));

    UpdateNotifyParamFile();
}

TEST_F(NotifyCompTestFixture, SetParamStringValue_UnsupportedParamName) {
    ANSC_HANDLE hInsContext = nullptr;
    char ParamName[] = "Unsupported_ParamName";
    char pString[] = "param1,eRT.com.cisco.spvtg.ccsp.webpaagent,false";

    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq("SetNotifi_ParamName"), _, StrEq(ParamName), _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(1), Return(EOK)));

    EXPECT_CALL(*g_safecLibMock, _strcmp_s_chk(StrEq("Notifi_ParamName"), _, StrEq(ParamName), _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(1), Return(EOK)));

    BOOL result = NotifyComponent_SetParamStringValue(hInsContext, ParamName, pString);

    // Assert
    EXPECT_FALSE(result);

}

TEST_F(NotifyCompTestFixture, NotifyComponent_GetParamStringValue) {
    ANSC_HANDLE hInsContext = nullptr;
    char ParamName[] = "TestParam";
    char pValue[] = "TestValue";  // Buffer to hold the string value
    ULONG ulSize = sizeof(pValue);  // Size of the buffer

    // Call the function
    BOOL result = NotifyComponent_GetParamStringValue(hInsContext, ParamName, pValue, &ulSize);

    // Check that the function returns TRUE
    EXPECT_TRUE(result);
    EXPECT_EQ(ulSize, sizeof(pValue));  // The size shouldn't change
    EXPECT_STREQ(pValue, "TestValue");  // The value should remain unchanged

    strcpy(pValue, "");
    ulSize = 0;

    result = NotifyComponent_GetParamStringValue(hInsContext, ParamName, pValue, &ulSize);

    EXPECT_TRUE(result);
    EXPECT_EQ(ulSize, 0);  // The size shouldn't change
}

TEST(NotifyCompTests, NotifyComponent_Commit) {
    ANSC_HANDLE hInsContext = nullptr;

    // Call the function
    BOOL result = NotifyComponent_Commit(hInsContext);

    // Check that the function returns 0
    EXPECT_EQ(result, 0);  // The size shouldn't change
}

TEST(NotifyCompTests, NotifyComponent_GetParamBoolValue) {
    ANSC_HANDLE hInsContext = nullptr;
    char ParamName[] = "TestParam";
    BOOL boolValue = FALSE;

    // Call the function
    BOOL result = NotifyComponent_GetParamBoolValue(hInsContext, ParamName, &boolValue);

    // Check that the function returns TRUE
    EXPECT_TRUE(result);
    // Verify the bool value remains unchanged (since the function does not modify it)
    EXPECT_EQ(boolValue, FALSE);
}

TEST(NotifyCompTests, NotifyComponent_SetParamBoolValue) {
    ANSC_HANDLE hInsContext = nullptr;
    char ParamName[] = "TestParam";
    BOOL boolValue = TRUE;

    // Call the function
    BOOL result = NotifyComponent_SetParamBoolValue(hInsContext, ParamName, boolValue);

    // Check that the function returns TRUE
    EXPECT_TRUE(result);
}

TEST_F(NotifyCompTestFixture, Notify_To_PAs_HandleWEBPANotification_ConnectedClient) {
    char msg_str[] = "Connected-Client";
    UINT PA_Bits = NotifyMask_WEBPA;

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , testing::_ , testing::_)).WillRepeatedly(testing::Return(EOK));
    EXPECT_CALL(*g_baseapiMock, CcspBaseIf_setParameterValues(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(CCSP_SUCCESS));

    Notify_To_PAs(PA_Bits, msg_str);
}

TEST_F(NotifyCompTestFixture, Notify_To_PAs_HandleWEBPANotification_PresenceNotification) {
    char msg_str[] = "PresenceNotification";
    UINT PA_Bits = NotifyMask_WEBPA;

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , testing::_ , testing::_)).WillRepeatedly(testing::Return(EOK));
    EXPECT_CALL(*g_baseapiMock, CcspBaseIf_setParameterValues(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(CCSP_FAILURE));

    Notify_To_PAs(PA_Bits, msg_str);
}

TEST(NotifyCompTests, Notify_To_PAs_HandleDMCLINotification) {
    char msg_str[] = "Test-Client";
    UINT PA_Bits = NotifyMask_DMCLI;

    Notify_To_PAs(PA_Bits, msg_str);
}

TEST(NotifyCompTests, Notify_To_PAs_HandleSNMPNotification) {
    char msg_str[] = "Test-Client";
    UINT PA_Bits = NotifyMask_SNMP;

    Notify_To_PAs(PA_Bits, msg_str);
}

TEST_F(NotifyCompTestFixture, Notify_To_PAs_HandleTR069Notification_ConnectedClient) {
    char msg_str[] = CONNECTED_CLIENT_STR;
    UINT PA_Bits = NotifyMask_TR069;

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , testing::_ , testing::_)).WillRepeatedly(testing::Return(EOK));
    EXPECT_CALL(*g_baseapiMock, CcspBaseIf_setParameterValues(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(CCSP_SUCCESS));

    Notify_To_PAs(PA_Bits, msg_str);
}

TEST_F(NotifyCompTestFixture, Notify_To_PAs_HandleTR069Notification_NotConnectedClient) {
    char msg_str[] = "Not-ConnectedClient";
    UINT PA_Bits = NotifyMask_TR069;

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , testing::_ , testing::_)).WillRepeatedly(testing::Return(EOK));
    EXPECT_CALL(*g_baseapiMock, CcspBaseIf_setParameterValues(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(CCSP_FAILURE));

    Notify_To_PAs(PA_Bits, msg_str);
}

TEST_F(NotifyCompTestFixture, Notify_To_PAs_HandleWIFINotification_ConnectedClient) {
    char msg_str[] = "Connected-Client";
    UINT PA_Bits = NotifyMask_WIFI;

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , testing::_ , testing::_)).WillRepeatedly(testing::Return(EOK));
    EXPECT_CALL(*g_baseapiMock, CcspBaseIf_setParameterValues(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(CCSP_SUCCESS));

    Notify_To_PAs(PA_Bits, msg_str);
}

TEST_F(NotifyCompTestFixture, Notify_To_PAs_HandleWIFINotification_NotConnectedClient) {
    char msg_str[] = "Not-ConnectedClient";
    UINT PA_Bits = NotifyMask_WIFI;

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , testing::_ , testing::_)).WillRepeatedly(testing::Return(EOK));
    EXPECT_CALL(*g_baseapiMock, CcspBaseIf_setParameterValues(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(CCSP_FAILURE));

    Notify_To_PAs(PA_Bits, msg_str);
}

TEST_F(NotifyCompTestFixture, Notify_To_PAs_HandleErrorInStrcpy_s) {
    char msg_str[] = "Connected-Client";
    UINT PA_Bits = NotifyMask_WEBPA;

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , testing::_ , testing::_)).WillOnce(testing::Return(ESNULLP));
    EXPECT_CALL(*g_baseapiMock, CcspBaseIf_setParameterValues(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .Times(0);

    Notify_To_PAs(PA_Bits, msg_str);
}

TEST_F(NotifyCompTestFixture, Notify_To_PAs_HandleErrorInStrcpy_s2) {
    char msg_str[] = "Connected-Client";
    UINT PA_Bits = NotifyMask_WEBPA;

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , "eRT.com.cisco.spvtg.ccsp.webpaagent", testing::_)).WillOnce(Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , "/com/cisco/spvtg/ccsp/webpaagent", testing::_)).WillOnce(Return(ESNULLP));
    EXPECT_CALL(*g_baseapiMock, CcspBaseIf_setParameterValues(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .Times(0);

    Notify_To_PAs(PA_Bits, msg_str);

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , "eRT.com.cisco.spvtg.ccsp.webpaagent", testing::_)).WillOnce(Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , "/com/cisco/spvtg/ccsp/webpaagent", testing::_)).WillOnce(Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , "Device.Webpa.X_RDKCENTRAL-COM_Connected-Client", testing::_)).WillOnce(Return(ESNULLP));
    EXPECT_CALL(*g_baseapiMock, CcspBaseIf_setParameterValues(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .Times(0);

    Notify_To_PAs(PA_Bits, msg_str);
}

TEST_F(NotifyCompTestFixture, Notify_To_PAs_HandleErrorInStrcpy_s3) {
    char msg_str[] = "PresenceNotification";
    UINT PA_Bits = NotifyMask_WEBPA;

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , "eRT.com.cisco.spvtg.ccsp.webpaagent", testing::_)).WillOnce(Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , "/com/cisco/spvtg/ccsp/webpaagent", testing::_)).WillOnce(Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , "Device.Webpa.X_RDKCENTRAL-COM_WebPA_Notification", testing::_)).WillOnce(Return(ESNULLP));
    EXPECT_CALL(*g_baseapiMock, CcspBaseIf_setParameterValues(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .Times(0);

    Notify_To_PAs(PA_Bits, msg_str);

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , "eRT.com.cisco.spvtg.ccsp.webpaagent", testing::_)).WillOnce(Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , "/com/cisco/spvtg/ccsp/webpaagent", testing::_)).WillOnce(Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , "Device.Webpa.X_RDKCENTRAL-COM_WebPA_Notification", testing::_)).WillOnce(Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , "Device.Webpa.X_RDKCENTRAL-COM_Connected-Client", testing::_)).WillOnce(Return(ESNULLP));
    EXPECT_CALL(*g_baseapiMock, CcspBaseIf_setParameterValues(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .Times(0);

    Notify_To_PAs(PA_Bits, msg_str);
}

TEST_F(NotifyCompTestFixture, Notify_To_PAs_HandleTR069Notification_ConnectedClient_StrcpyFail) {
    char msg_str[64] = CONNECTED_CLIENT_STR;
    UINT PA_Bits = NotifyMask_TR069;

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , "eRT.com.cisco.spvtg.ccsp.tr069pa" , testing::_)).WillOnce(testing::Return(ESNULLP));
    EXPECT_CALL(*g_baseapiMock, CcspBaseIf_setParameterValues(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .Times(0);

    Notify_To_PAs(PA_Bits, msg_str);

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , "eRT.com.cisco.spvtg.ccsp.tr069pa" , testing::_)).WillOnce(testing::Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , "/com/cisco/spvtg/ccsp/MS" , testing::_)).WillOnce(testing::Return(ESNULLP));
    EXPECT_CALL(*g_baseapiMock, CcspBaseIf_setParameterValues(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .Times(0);

    Notify_To_PAs(PA_Bits, msg_str);

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , "eRT.com.cisco.spvtg.ccsp.tr069pa" , testing::_)).WillOnce(testing::Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , "/com/cisco/spvtg/ccsp/MS" , testing::_)).WillOnce(testing::Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , "Device.TR069Notify.X_RDKCENTRAL-COM_Connected-Client" , testing::_)).WillRepeatedly(testing::Return(ESNULLP));
    EXPECT_CALL(*g_baseapiMock, CcspBaseIf_setParameterValues(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .Times(0);

    Notify_To_PAs(PA_Bits, msg_str);

    strcpy(msg_str, "PresenceNotification");

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , "eRT.com.cisco.spvtg.ccsp.tr069pa" , testing::_)).WillOnce(testing::Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , "/com/cisco/spvtg/ccsp/MS" , testing::_)).WillOnce(testing::Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , "Device.TR069Notify.X_RDKCENTRAL-COM_TR069_Notification" , testing::_)).WillRepeatedly(testing::Return(ESNULLP));
    EXPECT_CALL(*g_baseapiMock, CcspBaseIf_setParameterValues(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .Times(0);

    Notify_To_PAs(PA_Bits, msg_str);
}

TEST_F(NotifyCompTestFixture, Notify_To_PAs_HandleWIFINotification_ConnectedClient_StrcpyFail) {
    char msg_str[64] = "Connected-Client";
    UINT PA_Bits = NotifyMask_WIFI;

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , "eRT.com.cisco.spvtg.ccsp.wifi" , testing::_)).WillOnce(testing::Return(ESNULLP));
    EXPECT_CALL(*g_baseapiMock, CcspBaseIf_setParameterValues(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .Times(0);

    Notify_To_PAs(PA_Bits, msg_str);

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , "eRT.com.cisco.spvtg.ccsp.wifi" , testing::_)).WillOnce(testing::Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , "/com/cisco/spvtg/ccsp/wifi" , testing::_)).WillOnce(testing::Return(ESNULLP));
    EXPECT_CALL(*g_baseapiMock, CcspBaseIf_setParameterValues(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .Times(0);

    Notify_To_PAs(PA_Bits, msg_str);

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , "eRT.com.cisco.spvtg.ccsp.wifi" , testing::_)).WillOnce(testing::Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , "/com/cisco/spvtg/ccsp/wifi" , testing::_)).WillOnce(testing::Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , "Device.WiFi.X_RDKCENTRAL-COM_Connected-Client" , testing::_)).WillRepeatedly(testing::Return(ESNULLP));
    EXPECT_CALL(*g_baseapiMock, CcspBaseIf_setParameterValues(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .Times(0);

    Notify_To_PAs(PA_Bits, msg_str);

    strcpy(msg_str, "PresenceNotification");

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , "eRT.com.cisco.spvtg.ccsp.wifi" , testing::_)).WillOnce(testing::Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , "/com/cisco/spvtg/ccsp/wifi" , testing::_)).WillOnce(testing::Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(testing::_ , testing::_ , "Device.WiFi.X_RDKCENTRAL-COM_WiFi_Notification" , testing::_)).WillRepeatedly(testing::Return(ESNULLP));
    EXPECT_CALL(*g_baseapiMock, CcspBaseIf_setParameterValues(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .Times(0);

    Notify_To_PAs(PA_Bits, msg_str);
}

TEST_F(NotifyCompTestFixture, ReloadNotifyParam_fopen_fail) {

    EXPECT_CALL(*g_fopenMock, fopen_mock(_, _))
        .Times(1)
        .WillOnce(Return(nullptr));

    ReloadNotifyParam();
}

TEST_F(NotifyCompTestFixture, ReloadNotifyParam_fopen_success) {

    FILE* mockFile = tmpfile();
    ASSERT_NE(mockFile, nullptr) << "tmpfile() Failed : Couldn't create temporary file";

    EXPECT_CALL(*g_fopenMock, fopen_mock(_, _))
        .Times(1)
        .WillOnce(Return(mockFile));

    EXPECT_CALL(*g_fileIOMock, fgets(_, _, _))
        .Times(2)
        .WillOnce(Invoke([](char* buf, int size, FILE* file) {
            strncpy(buf, "0x00000001:eRT.com.cisco.spvtg.ccsp.webpaagent\n0x00000002:ccsp.busclient\n0x00000004:SNMP\n0x00000008:eRT.com.cisco.spvtg.ccsp.tr069pa\n0x00000010:eRT.com.cisco.spvtg.ccsp.wifi\n", size);
            return buf;
        }))
        .WillOnce(Invoke([](char* buf, int size, FILE* file) {
            strncpy(buf, "0x00000001:eRT.com.cisco.spvtg.ccsp.webpaagent\n0x00000002:ccsp.busclient\n0x00000004:SNMP\n0x00000008:eRT.com.cisco.spvtg.ccsp.tr069pa\n0x00000010:eRT.com.cisco.spvtg.ccsp.wifi\n", size);
            return nullptr;
        }));

    EXPECT_CALL(*g_fileIOMock, fclose(mockFile))
        .Times(1)
        .WillOnce(Return(0));

    ReloadNotifyParam();

    EXPECT_CALL(*g_fopenMock, fopen_mock(_, _))
        .Times(1)
        .WillOnce(Return(mockFile));

    EXPECT_CALL(*g_fileIOMock, fgets(_, _, _))
        .Times(2)
        .WillOnce(Invoke([](char* buf, int size, FILE* file) {
            strncpy(buf, "0x00000001:eRT.com.cisco.spvtg.ccsp.webpaagent\n0x00000002:ccsp.busclient\n0x00000004:SNMP\n0x00000008:eRT.com.cisco.spvtg.ccsp.tr069pa\n0x00000010:eRT.com.cisco.spvtg.ccsp.wifi\n", size);
            return buf;
        }))
        .WillOnce(Invoke([](char* buf, int size, FILE* file) {
            strncpy(buf, "0x00000001:eRT.com.cisco.spvtg.ccsp.webpaagent\n0x00000002:ccsp.busclient\n0x00000004:SNMP\n0x00000008:eRT.com.cisco.spvtg.ccsp.tr069pa\n0x00000010:eRT.com.cisco.spvtg.ccsp.wifi\n", size);
            return nullptr;
        }));

    EXPECT_CALL(*g_fileIOMock, fclose(mockFile))
        .Times(1)
        .WillOnce(Return(0));

    ReloadNotifyParam();
}

TEST_F(NotifyCompTestFixture, EventHandlerThread_mqopen_fail) {

    FILE *original_stderr = stderr;
    stderr = fopen("/dev/null", "w");

    pthread_t Event_HandlerThreadID;

    // Set up the EXPECT_CALL for mq_open
    EXPECT_CALL(*g_MQHandlerMock, mq_open(_,_, _, _))
        .WillOnce(Return((mqd_t)-1)); // Mock a valid message queue descriptor

    Event_HandlerThread((void*)&Event_HandlerThreadID);

    stderr = original_stderr;
}

TEST_F(NotifyCompTestFixture, EventHandlerThread_mqopen_success) {
    pthread_t Event_HandlerThreadID;
    numLoops = 0;

    EXPECT_CALL(*g_MQHandlerMock, mq_open(_,_, _, _))
        .WillOnce(Return((mqd_t)1)); // Mock a valid message queue descriptor

    EXPECT_CALL(*g_MQHandlerMock, mq_receive(_,_, _, _))
        .Times(1)
        .WillRepeatedly(Return((mqd_t)0)); // Mock a valid message queue descriptor

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
        .Times(1)
        .WillRepeatedly(Return(EOK)); // Mock successful strcpy_s

    Event_HandlerThread((void*)&Event_HandlerThreadID);
}

TEST_F(NotifyCompTestFixture, EventHandlerThread_mqreceive_fail) {
    FILE *original_stderr = stderr;
    stderr = fopen("/dev/null", "w");

    pthread_t Event_HandlerThreadID;
    numLoops = 1;

    EXPECT_CALL(*g_MQHandlerMock, mq_open(_,_, _, _))
        .WillOnce(Return((mqd_t)1)); // Mock a valid message queue descriptor

    EXPECT_CALL(*g_MQHandlerMock, mq_receive(_,_, _, _))
        .WillOnce(Return((mqd_t)-1)); // Mock an invalid message queue descriptor


    Event_HandlerThread((void*)&Event_HandlerThreadID);

    stderr = original_stderr;
}

TEST_F(NotifyCompTestFixture, EventHandlerThread_strcpy_fail) {
    pthread_t Event_HandlerThreadID;
    numLoops = 0;

    EXPECT_CALL(*g_MQHandlerMock, mq_open(_,_, _, _))
        .WillOnce(Return((mqd_t)1)); // Mock a valid message queue descriptor

    EXPECT_CALL(*g_MQHandlerMock, mq_receive(_,_, _, _))
        .WillOnce(Return((mqd_t)0)); // Mock a valid message queue descriptor

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
        .WillOnce(Return(ESNULLP)); // Mock successful strcpy_s

    Event_HandlerThread((void*)&Event_HandlerThreadID);
}

TEST_F(NotifyCompTestFixture, ReloadNotifyParam_chPtr_null1) {

    FILE* mockFile = tmpfile();
    ASSERT_NE(mockFile, nullptr) << "tmpfile() Failed : Couldn't create temporary file";

    EXPECT_CALL(*g_fopenMock, fopen_mock(_, _))
        .Times(1)
        .WillOnce(Return(mockFile));

    EXPECT_CALL(*g_fileIOMock, fgets(_, _, _))
        .Times(2)
        .WillOnce(Invoke([](char* buf, int size, FILE* file) {
            strncpy(buf, ":", size);
            return buf;
        }))
        .WillOnce(Invoke([](char* buf, int size, FILE* file) {
            strncpy(buf, ":", size);
            return nullptr;
        }));

    EXPECT_CALL(*g_fileIOMock, fclose(mockFile))
        .Times(1)
        .WillOnce(Return(0));

    ReloadNotifyParam();
}

TEST_F(NotifyCompTestFixture, ReloadNotifyParam_chPtr_null2) {

    FILE* mockFile = tmpfile();
    ASSERT_NE(mockFile, nullptr) << "tmpfile() Failed : Couldn't create temporary file";

    EXPECT_CALL(*g_fopenMock, fopen_mock(_, _))
        .Times(1)
        .WillOnce(Return(mockFile));

    EXPECT_CALL(*g_fileIOMock, fgets(_, _, _))
        .Times(2)
        .WillOnce(Invoke([](char* buf, int size, FILE* file) {
            strncpy(buf, "0x00000001:", size);
            return buf;
        }))
        .WillOnce(Invoke([](char* buf, int size, FILE* file) {
            strncpy(buf, "0x00000001:", size);
            return nullptr;
        }));

    EXPECT_CALL(*g_fileIOMock, fclose(mockFile))
        .Times(1)
        .WillOnce(Return(0));

    ReloadNotifyParam();
}

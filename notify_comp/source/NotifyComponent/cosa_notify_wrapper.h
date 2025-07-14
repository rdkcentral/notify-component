/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2019 RDK Management
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
/**********************************************************************
   Copyright [2014] [Cisco Systems, Inc.]

   Licensed under the Apache License, Version 2.0 (the \"License\");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an \"AS IS\" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
**********************************************************************/
/**********************************************************************

    module: cosa_notify_wrapper.h

        For Notify component 

    ---------------------------------------------------------------

    description:

        This wrapper file defines all the platform-independent
        functions on logging

    ---------------------------------------------------------------

    environment:

        platform independent

    ---------------------------------------------------------------

    author:

        

    ---------------------------------------------------------------

    revision:

        

**********************************************************************/


#include "ccsp_trace.h"

extern   char*                      g_NotifyCompName;


/*
 * Logging wrapper APIs
 */
#define  CcspNotifyCompTraceEmergency(msg)                         \
    CcspTraceEmergency2(g_NotifyCompName, msg)

#define  CcspNotifyCompTraceAlert(msg)                             \
    CcspTraceAlert2(g_NotifyCompName, msg)

#define  CcspNotifyCompTraceCritical(msg)                          \
    CcspTraceCritical2(g_NotifyCompName, msg)

#define  CcspNotifyCompTraceError(msg)                             \
    CcspTraceError2(g_NotifyCompName, msg)

#define  CcspNotifyCompTraceWarning(msg)                           \
    CcspTraceWarning2(g_NotifyCompName, msg)

#define  CcspNotifyCompTraceNotice(msg)                            \
    CcspTraceNotice2(g_NotifyCompName, msg)

#define  CcspNotifyCompTraceDebug(msg)                             \
    CcspTraceInfo2(g_NotifyCompName, msg)

#define  CcspNotifyCompTraceInfo(msg)                              \
    CcspTraceInfo2(g_NotifyCompName, msg)



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

/**************************************************************************

    module: cosa_apis_pluginsampleobj.h

        For COSA Data Model Library Development

    -------------------------------------------------------------------

    description:

        This file defines the apis for objects to support Data Model Library.

    -------------------------------------------------------------------

    author:

        COSA XML TOOL CODE GENERATOR 1.0

    -------------------------------------------------------------------

    revision:

        12/02/2010    initial revision.

**************************************************************************/


#ifndef  _COSA_APIS_PLUGINSAMPLEOBJ_H
#define  _COSA_APIS_PLUGINSAMPLEOBJ_H

#include "slap_definitions.h"

/***********************************************************************

 APIs for Object:

    InternetGatewayDevice.X_CISCO_COM_COSADataModel.PluginSampleObj.

    *  PluginSampleObj_GetBulkParamValues
    *  PluginSampleObj_SetBulkParamValues
    *  PluginSampleObj_Validate
    *  PluginSampleObj_Commit
    *  PluginSampleObj_Rollback

***********************************************************************/
BOOL
NotifyComponent_GetParamUlongValue
    (
 	ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    );

BOOL
NotifyComponent_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
       ULONG                      uValue
    );

ULONG
NotifyComponent_Commit
    (
        ANSC_HANDLE                 hInsContext
    );

BOOL
NotifyComponent_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    );

BOOL
NotifyComponent_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    );
BOOL
NotifyComponent_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    );
	BOOL
NotifyComponent_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    );

UINT PA_to_Mask(char* PA_Name);
/* CID 63076: Parse warning*/
void NotifyParam(char* PA_Name, char* param_name, char* add);
/*CID 67366: Parse warning */
void AddNotifyParam(char* PA_Name, char* param_name);
/*CID 72257: Parse warning */
void DelNotifyParam(char* PA_Name, char* param_name);
/*CID: 56437 Parse warning - No Definition*/
//Print_Notify_Arr();
/*CID 56554: Parse warning */
void Find_Param(char* param_name, char* MsgStr);
/* CID 61319: Parse warning*/
void Notify_To_PAs(UINT PA_Bits, char* MsgStr);
void ReloadNotifyParam();
void UpdateNotifyParamFile();
void CreateEventHandlerThread();
void *Event_HandlerThread(void *threadid);
#endif

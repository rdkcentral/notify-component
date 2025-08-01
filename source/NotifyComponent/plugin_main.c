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
/*********************************************************************** 
  
    module: plugin_main.c

        Implement COSA Data Model Library Init and Unload apis.
 
    ---------------------------------------------------------------

    author:

        COSA XML TOOL CODE GENERATOR 1.0

    ---------------------------------------------------------------

    revision:

        09/28/2011    initial revision.

**********************************************************************/

#include "ansc_platform.h"
#include "ansc_load_library.h"
#include "cosa_plugin_api.h"
#include "plugin_main.h"
#include "cosa_apis_NotifyComponent.h"

#define THIS_PLUGIN_VERSION                         1

int ANSC_EXPORT_API
COSA_Init
    (
        ULONG                       uMaxVersionSupported, 
        void*                       hCosaPlugInfo         /* PCOSA_PLUGIN_INFO passed in by the caller */
    )
{
    PCOSA_PLUGIN_INFO               pPlugInfo  = (PCOSA_PLUGIN_INFO)hCosaPlugInfo;

    if ( uMaxVersionSupported < THIS_PLUGIN_VERSION )
    {
      /* this version is not supported */
        return -1;
    }   
    
    pPlugInfo->uPluginVersion       = THIS_PLUGIN_VERSION;
    /* register the back-end apis for the data model */
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "NotifyComponent_GetParamUlongValue",  NotifyComponent_GetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "NotifyComponent_SetParamUlongValue",  NotifyComponent_SetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "NotifyComponent_GetParamStringValue",  NotifyComponent_GetParamStringValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "NotifyComponent_SetParamStringValue",  NotifyComponent_SetParamStringValue);
	pPlugInfo->RegisterFunction(pPlugInfo->hContext, "NotifyComponent_GetParamBoolValue",  NotifyComponent_GetParamBoolValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "NotifyComponent_SetParamBoolValue",  NotifyComponent_SetParamBoolValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "NotifyComponent_Commit",  NotifyComponent_Commit);
    return  0;
}

BOOL ANSC_EXPORT_API
COSA_IsObjectSupported
    (
        char*                        pObjName
    )
{
    
    return TRUE;
}

void ANSC_EXPORT_API
COSA_Unload
    (
        void
    )
{
    /* unload the memory here */
}

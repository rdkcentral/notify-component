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

#include "ansc_platform.h"
#include "ansc_string_util.h"
#include "cosa_apis_NotifyComponent.h"
#include "ccsp_trace.h"
#include "ccsp_syslog.h"
#include "ccsp_base_api.h"
#include "dslh_definitions_database.h"
#include "cosa_notify_wrapper.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <mqueue.h>
#include "safec_lib_common.h"
#include "secure_wrapper.h"
#define EVENT_QUEUE_NAME  "/Notify_queue"

#define MAX_SIZE    2048
#define MAX_SIZE_EVT    1024


#define CHECK(x) \
    do { \
        if (!(x)) { \
            fprintf(stderr, "%s:%d: ", __func__, __LINE__); \
            perror(#x); \
            return; \
        } \
    } while (0) \


#define DYNAMIC_Notify

#define NotifyMask_WEBPA 	0x00000001
#define NotifyMask_DMCLI 	0x00000002
#define NotifyMask_SNMP 	0x00000004
#define NotifyMask_TR069 	0x00000008
#define NotifyMask_WIFI 	0x00000010

#if defined(FEATURE_SUPPORT_MESH)
#define NotifyMask_MESH     0x00000020
#endif

#if defined(UNIT_TEST_DOCKER_SUPPORT)
    extern FILE* fopen_mock(const char* filename, const char* mode);
    #define fopen  fopen_mock    // Mock fopen for testing
    extern int numLoops;
    #define FOREVER()    numLoops--  // Loop control for testing

#else
    #define FOREVER()    1           // Infinite loop

#endif

#define CCSP_DBUS_PATH_MS            "/com/cisco/spvtg/ccsp/MS"
#define CCSP_DBUS_INTERFACE_TR069PA  "eRT.com.cisco.spvtg.ccsp.tr069pa"
#define TR069_CONNECTED_CLIENT_PARAM "Device.TR069Notify.X_RDKCENTRAL-COM_Connected-Client"
#define TR069_NOTIFICATION_PARAM     "Device.TR069Notify.X_RDKCENTRAL-COM_TR069_Notification"
#define CONNECTED_CLIENT_STR         "Connected-Client"

#ifndef DYNAMIC_Notify
	typedef  struct _Notify_param
	{ 
	    char	param_name[256];
		UINT	Notify_PA;
	} Notify_param;

	#define MAX_NOTIFY_PARAM 200
	Notify_param Notify_param_arr[MAX_NOTIFY_PARAM];
	static UINT Ncount = 0;
#else
#include "ccsp_memory.h"

	typedef  struct _Notify_param
	{ 
	  	char	param_name[256];
		UINT	Notify_PA;
		struct _Notify_param *next;
	} Notify_param, *PNotify_param;
	PNotify_param head=NULL;
#endif

extern ANSC_HANDLE bus_handle;

/*CID : 121784 Parse warning*/
static void MsgPosttoQueue(char *pMsgStr);

#define NOTIFY_PARAM_FILE "/tmp/.NotifyParamListCache"
#define BUFF_SIZE 1024

#define NUM_NOTIFYMASK_TYPES (sizeof(notifyMask_type_table)/sizeof(notifyMask_type_table[0]))

typedef struct {
  char     *name;
  UINT    type;
} NOTIFY_MASK_TYPE;

  NOTIFY_MASK_TYPE notifyMask_type_table[] = {
  { "eRT.com.cisco.spvtg.ccsp.webpaagent", NotifyMask_WEBPA   },
  { "ccsp.busclient",                      NotifyMask_DMCLI   },
  { "SNMP",                                NotifyMask_SNMP    },
  { "eRT.com.cisco.spvtg.ccsp.tr069pa",    NotifyMask_TR069   },
  { "eRT.com.cisco.spvtg.ccsp.wifi",       NotifyMask_WIFI    },
#if defined(FEATURE_SUPPORT_MESH)
  { "eRT.com.cisco.spvtg.ccsp.meshagent",   NotifyMask_MESH   }
#endif
};
  
int getNotifyMask_type_from_name(char *name,  UINT *type_ptr)
{
  int rc = -1;
  int ind = -1;
  int i = 0;
  int strlength = 0;

  if((name == NULL) || (type_ptr == NULL))
     return 0;

  strlength = strlen(name);

  for (i = 0 ; i < NUM_NOTIFYMASK_TYPES ; ++i)
  {
      rc = strcmp_s(name, strlength, notifyMask_type_table[i].name, &ind);
      ERR_CHK(rc);
      if( (!ind) && (rc == EOK))
      {
          *type_ptr = notifyMask_type_table[i].type;
          return 1;
      }
  }
  return 0;
}

BOOL
NotifyComponent_GetParamUlongValue
    (
         ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    return TRUE;
}

BOOL
NotifyComponent_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG                      uValue
    )
{
    return TRUE;
}


BOOL
NotifyComponent_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    )
{

	char* p_write_id;
	char* p_notification;
	char* p_notify_param_name;
	char* st = NULL;
        errno_t                         rc                  = -1;
        int                             ind                 = -1;
       /* check the parameter name and set the corresponding value */
        rc = strcmp_s("SetNotifi_ParamName", strlen("SetNotifi_ParamName"), ParamName , &ind);
        ERR_CHK(rc);
        if((!ind) && (rc == EOK))
        {
    	    if(!strstr(pString, "Passphrase"))
		CcspNotifyCompTraceInfo((" \n Notification : < %s : %d > ParamName = %s \n",__FUNCTION__,__LINE__, pString));

		MsgPosttoQueue(pString);
		CcspNotifyCompTraceInfo((" \n Notification : Msg Posted to queue\n"));

		return TRUE;
	}

        rc = strcmp_s("Notifi_ParamName", strlen("Notifi_ParamName"), ParamName , &ind);
        ERR_CHK(rc);
        if((!ind) && (rc == EOK))
        {
		//CcspNotifyCompTraceInfo((" \n Notification : < %s : %d > Notifi_ParamName received\n",__FUNCTION__,__LINE__));
		//printf(" \n Notification : < %s : %d > ParamName = %s \n",__FUNCTION__,__LINE__, pString);

		p_notify_param_name = strtok_r(pString, ",", &st);

		p_write_id = strtok_r(NULL, ",", &st);

		p_notification = strtok_r(NULL, ",", &st);

		NotifyParam(p_write_id,p_notify_param_name,p_notification);
		
        return TRUE;
    }
		
    AnscTraceWarning(("Unsupported parameter '%s'\n", ParamName));
    return FALSE;
}


BOOL
NotifyComponent_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
   return TRUE;
}

ULONG
NotifyComponent_Commit
    (
        ANSC_HANDLE                 hInsContext
    )
{
   
    return 0;
}
BOOL
NotifyComponent_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    )
{

    return TRUE;
}
BOOL
NotifyComponent_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    )
{

    return TRUE;
}

/* CID: 63931 & 59967 Missing return statement & type specifier*/
void
NotifyParam(char* PA_Name, char* param_name, char* add)
{
        errno_t                         rc                  = -1;
        int                             ind                 = -1;
        rc = strcmp_s("true", strlen("true"), add , &ind);
        ERR_CHK(rc);
        if((!ind) && (rc == EOK))
        {
		AddNotifyParam(PA_Name, param_name);
        }
	else
		DelNotifyParam(PA_Name, param_name);
}

/*CID: 65190 & 58080 Missing return statement & type specifier*/
void 
AddNotifyParam(char* PA_Name, char* param_name)
{
 
errno_t rc = -1;
int ind = -1;
#ifndef DYNAMIC_Notify

	UINT i;
        size_t len = 0;
        len = strlen(param_name);

	for(i=0;i<Ncount;i++)
	{
                rc = strcmp_s(param_name, len, Notify_param_arr[i].param_name, &ind);
                ERR_CHK(rc);
                if((!ind) && (rc == EOK))
		{
			Notify_param_arr[i].Notify_PA |= PA_to_Mask(PA_Name);	
			CcspNotifyCompTraceInfo((" \n Notification : Parameter %s is added in the list by %s \n", param_name, PA_Name));
			break;
		}
	}

	if(i == Ncount)
	{
                rc = strcpy_s( Notify_param_arr[i].param_name, sizeof(Notify_param_arr[i].param_name),param_name);
                if(rc != EOK)
                {
                    ERR_CHK(rc);
                    return;
                } 
		Notify_param_arr[i].param_name[sizeof(Notify_param_arr[i].param_name)-1]=0;

		Notify_param_arr[i].Notify_PA = PA_to_Mask(PA_Name);
		Ncount++;
		CcspNotifyCompTraceInfo((" \n Notification : Parameter %s is added in the list by %s \n", param_name, PA_Name));
	}
#else

	PNotify_param temp=head;
	PNotify_param prev=head;
	BOOL found = 0;
	size_t strsize = 0;
        strsize = strlen(param_name);

	while(temp!=NULL)
	{
		
                rc = strcmp_s(param_name, strsize, temp->param_name, &ind);
                ERR_CHK(rc);
                if((!ind) && (rc == EOK))
		{
			temp->Notify_PA |= PA_to_Mask(PA_Name);
			CcspNotifyCompTraceInfo((" \n Notification : Parameter %s is added in the list by %s \n", param_name, PA_Name));
			found = 1;
			break;	
		}
		prev = temp;
		temp = temp->next;
	}

	if(found == 0)
	{
		PNotify_param new_node = (PNotify_param) AnscAllocateMemory(sizeof(Notify_param));

		if(new_node)
		{
                        /* Coverity Fix CID: 139325,135494 BUFFERSIZE_WARNING,STRING_OVERFLOW */
                        rc = strcpy_s(new_node->param_name, sizeof(new_node->param_name), param_name);
                        if(rc != EOK)
                        {
                            ERR_CHK(rc);
		            return;
                        }
			new_node->param_name[sizeof(new_node->param_name)-1] = '\0';

			new_node->Notify_PA = PA_to_Mask(PA_Name);
			new_node->next = NULL;


			if(prev == NULL)
				head = new_node;
			else
				prev->next = new_node;
			
			CcspNotifyCompTraceInfo((" \n Notification : Parameter %s is added in the list by %s \n", param_name, PA_Name));
		}
		else
		{
			CcspNotifyCompTraceInfo((" \n Notification : < %s : %d > Failed to Allocate Memory \n", __FUNCTION__,__LINE__));
		}
	}
	UpdateNotifyParamFile(); //Write the notification parameter to file
#endif

}

/* CID:66977 & 69506 Missing return statement & type specifier*/
void 
DelNotifyParam(char* PA_Name, char* param_name)
{

errno_t                         rc                  = -1;
int                             ind                 = -1;

#ifndef DYNAMIC_Notify

	UINT i;
        size_t len = 0;
        len = strlen(param_name);

	for(i=0;i<Ncount;i++)
	{
                rc = strcmp_s(param_name, len, Notify_param_arr[i].param_name, &ind);
                ERR_CHK(rc);
                if((!ind) && (rc == EOK))
		{
			Notify_param_arr[i].Notify_PA &= ~(PA_to_Mask(PA_Name));	
			CcspNotifyCompTraceInfo((" \n Notification : Parameter %s is deleted from the list by %s \n", param_name, PA_Name));
			break;
		}
	}

	if(i == Ncount)
	{
		CcspNotifyCompTraceInfo((" \n Notification :  param_name %s not found \n", param_name));
	}

#else

	BOOL found = 0;
	PNotify_param temp=head;
	PNotify_param prev=head;
	size_t strsize = 0;
        strsize = strlen(param_name);

	while(temp!=NULL)
	{
                rc = strcmp_s(param_name, strsize, temp->param_name, &ind);
                ERR_CHK(rc);
                if((!ind) && (rc == EOK))
		{
			temp->Notify_PA &= ~(PA_to_Mask(PA_Name));
			CcspNotifyCompTraceInfo((" \n Notification : Parameter %s is deleted from the list by %s \n", param_name, PA_Name));

			if(temp->Notify_PA == 0)
			{
				if(temp == head)
				{
					head = temp->next;	
				}
				else
				{
					prev->next = temp->next;
				}
				AnscFreeMemory(temp);
				temp = NULL;
			}
			found = 1;
			break;	
		}
		prev = temp;
		temp = temp->next;
	}

	if(found == 0)
	{
		CcspNotifyCompTraceInfo((" \n Notification :  param_name %s not found \n", param_name));
	}
	else
	{
		UpdateNotifyParamFile(); //Write the notification parameter to file
	}

#endif

}

UINT PA_to_Mask(char* PA_Name)
{
        UINT return_val = NotifyMask_WEBPA;

        if(getNotifyMask_type_from_name(PA_Name, &return_val))
        {
             return return_val;
        }
               
	return return_val;
}

/* CID: 56982 & 61465 Missing return statement & type specifier*/
void 
Find_Param(char* param_name, char* MsgStr)
{
#ifndef DYNAMIC_Notify

	UINT i;
	

	for(i=0;i<Ncount;i++)
	{

		if(param_name && strstr(Notify_param_arr[i].param_name, param_name))
		{
			CcspNotifyCompTraceInfo((" \n Notification : Parameter %s found in the list \n", param_name));
			Notify_To_PAs(Notify_param_arr[i].Notify_PA,MsgStr);	
			break;
		}
	}

	if(i == Ncount)
	{
		CcspNotifyCompTraceInfo((" \n Notification : Parameter %s not found in the list \n", param_name));

	}
#else

	BOOL found = 0;
	PNotify_param temp=head;


	while(temp!=NULL)
	{
		
		if(param_name && strstr(temp->param_name,param_name))
		{
			CcspNotifyCompTraceInfo((" \n Notification : Parameter %s found in the list \n", param_name));
			Notify_To_PAs(temp->Notify_PA, MsgStr); 
			found = 1;
			break;  
		}
		temp = temp->next;
	}

	if(found == 0)
	{
		CcspNotifyCompTraceInfo((" \n Notification : Parameter %s not found in the list \n", param_name));
	}

#endif

}

/* CID: 72649 & 66310 Missing return statement & type specifier*/
void 
Notify_To_PAs(UINT PA_Bits, char* MsgStr)
{

	parameterValStruct_t notif_val[1];
	char compo[256]; 
	char bus[256];
	char param_name[256];
	char* faultParam = NULL;
	int ret = CCSP_FAILURE;
        errno_t                         rc                  = -1;

	notif_val[0].parameterName =  param_name ;
	notif_val[0].parameterValue = MsgStr;
	notif_val[0].type = ccsp_string;

	if(PA_Bits & NotifyMask_WEBPA)
	{
#if defined(FEATURE_SUPPORT_MESH)
        BOOLEAN clientMsg = FALSE;
        BOOLEAN connectMsg = FALSE;
#endif
        BOOLEAN presence_notify = FALSE;

                rc = strcpy_s(compo, sizeof(compo),"eRT.com.cisco.spvtg.ccsp.webpaagent");
                if (rc != EOK)
                {
                    ERR_CHK(rc);
                    return;
                }

                rc = strcpy_s(bus, sizeof(bus) ,"/com/cisco/spvtg/ccsp/webpaagent");
                if (rc != EOK)
                {
                    ERR_CHK(rc);
                    return;
                }

		if(strstr(MsgStr,"Connected-Client"))
		{

                        rc = strcpy_s(param_name, sizeof(param_name), "Device.Webpa.X_RDKCENTRAL-COM_Connected-Client");
                        if (rc != EOK)
                        {
                            ERR_CHK(rc);
                            return;
                        }
#if defined(FEATURE_SUPPORT_MESH)
			clientMsg = TRUE;
			if (strstr(MsgStr, ",Connected"))
			{
			    connectMsg = TRUE;
			}
#endif
		}
		else
		{
                        rc = strcpy_s(param_name, sizeof(param_name), "Device.Webpa.X_RDKCENTRAL-COM_WebPA_Notification");
                        if (rc != EOK)
                        {
                            ERR_CHK(rc);
                            return;
                        }
		}

        if(strstr(MsgStr,"PresenceNotification"))
        {
                        rc = strcpy_s(param_name, sizeof(param_name), "Device.Webpa.X_RDKCENTRAL-COM_Connected-Client");
                        if (rc != EOK)
                        {
                            ERR_CHK(rc);
                            return;
                        }
            presence_notify = TRUE;
        }
#if defined(FEATURE_SUPPORT_MESH)
        {
            FILE *fp;
            int ret = 0;
            char buffer[50] = {0};
            BOOLEAN meshOffline = TRUE;

            if((fp = v_secure_popen("r", "sysevent get mesh_status")))
            {
                while(fgets(buffer, sizeof(buffer)-1, fp)!=NULL)
                {
                    buffer[sizeof(buffer) - 1] = '\0';
                }

                // The response back from the mesh_status sysevent should be "MESH|Full"
                if (strstr(buffer, "Full") != NULL)
                {
                    meshOffline = FALSE;
                }
                ret = v_secure_pclose(fp);
                if (ret != 0) {
                    CcspNotifyCompTraceInfo(("Error in closing pipe! ret val %d\n", ret));
                }
	    }

            // Only send Connected-Client connect messages if mesh is offline (other than presence notification)
            // RDKB-19887 - Send connected client status always for only presence notification
		    if (!clientMsg || (clientMsg && meshOffline && connectMsg) || (presence_notify))
		    {
		        //  printf(" \n Notification : call WEBPA notification  \n");
		        CcspNotifyCompTraceInfo((" \n Notification : call WEBPA notification  \n"));

		        ret = CcspBaseIf_setParameterValues(
		                bus_handle,
		                compo,
		                bus,
		                0,
		                DSLH_MPA_ACCESS_CONTROL_NOTIFY_COMP,
		                notif_val,
		                1,
		                TRUE,
		                &faultParam
		        );

		        if(ret != CCSP_SUCCESS)
		        {
		            CcspNotifyCompTraceInfo(("NOTIFICATION: %s : CcspBaseIf_setParameterValues failed. ret value = %d \n", __FUNCTION__, ret));
		            CcspNotifyCompTraceInfo(("NOTIFICATION: %s : Parameter = %s \n", __FUNCTION__, notif_val[0].parameterValue));

		        }
		    } else {
		        if (connectMsg) {
		            CcspNotifyCompTraceInfo((" \n Notification : MESH online, skip WEBPA connect notification  \n"));
		        }
		    }
		}
#else
	    //  printf(" \n Notification : call WEBPA notification  \n");
	    CcspNotifyCompTraceInfo((" \n Notification : call WEBPA notification  \n"));

		ret = CcspBaseIf_setParameterValues(
		  bus_handle,
		  compo,
		  bus,
		  0,
		  DSLH_MPA_ACCESS_CONTROL_NOTIFY_COMP,
		  notif_val,
		  1,
		  TRUE,
		  &faultParam
		  );
		
		if(ret != CCSP_SUCCESS)
		{
			CcspNotifyCompTraceInfo(("NOTIFICATION: %s : CcspBaseIf_setParameterValues failed. ret value = %d \n", __FUNCTION__, ret));
			CcspNotifyCompTraceInfo(("NOTIFICATION: %s : Parameter = %s \n", __FUNCTION__, notif_val[0].parameterValue));

		}
        //unused variable.
        (void)(presence_notify);
#endif
	}

	if(PA_Bits & NotifyMask_DMCLI)
	{
		/*TODO : call DMCLI notification*/
		CcspNotifyCompTraceInfo((" \n Notification : call DMCLI notification  \n"));
	}

	if(PA_Bits & NotifyMask_SNMP)
	{
		/*TODO : call SNMP notification*/
		CcspNotifyCompTraceInfo((" \n Notification : call SNMP notification  \n"));
	}

	if(PA_Bits & NotifyMask_TR069)
	{
		CcspNotifyCompTraceInfo((" \n Notification : call TR069 notification  \n"));

                rc = strcpy_s(compo, sizeof(compo),CCSP_DBUS_INTERFACE_TR069PA);
                if (rc != EOK)
                {
                    ERR_CHK(rc);
                    return;
                }
                rc = strcpy_s(bus, sizeof(bus),CCSP_DBUS_PATH_MS);
                if (rc != EOK)
                {
                    ERR_CHK(rc);
                    return;
                }

		if( AnscStrStr(MsgStr, CONNECTED_CLIENT_STR) )
		{
                  rc = strcpy_s(param_name, sizeof(param_name),TR069_CONNECTED_CLIENT_PARAM);
                  if (rc != EOK)
                  {
                     ERR_CHK(rc);
                     return;
                  }
		}
		else
		{
                  rc = strcpy_s(param_name, sizeof(param_name),TR069_NOTIFICATION_PARAM);
                  if (rc != EOK)
                  {
                     ERR_CHK(rc);
                     return;
                  }
		}

		ret = CcspBaseIf_setParameterValues(
		  bus_handle,
		  compo,
		  bus,
		  0,
		  DSLH_MPA_ACCESS_CONTROL_NOTIFY_COMP,
		  notif_val,
		  1,
		  TRUE,
		  &faultParam
		  );

		if( ret != CCSP_SUCCESS )
		{
		  CcspNotifyCompTraceInfo(("NOTIFICATION: %s : CcspBaseIf_setParameterValues failed. ret value = %d \n", __FUNCTION__, ret));
		  CcspNotifyCompTraceInfo(("NOTIFICATION: %s : Parameter = %s \n", __FUNCTION__, notif_val[0].parameterValue));
		}
	}

	if(PA_Bits & NotifyMask_WIFI)
	{
		CcspNotifyCompTraceInfo((" \n Notification : call WIFI notification  \n"));

                rc = strcpy_s(compo, sizeof(compo),"eRT.com.cisco.spvtg.ccsp.wifi");
                if (rc != EOK)
                {
                    ERR_CHK(rc);
                    return;
                }
                rc = strcpy_s(bus, sizeof(bus) ,"/com/cisco/spvtg/ccsp/wifi");
                if (rc != EOK)
                {
                    ERR_CHK(rc);
                    return;
                }

		if(strstr(MsgStr,"Connected-Client"))
		{
                        rc = strcpy_s(param_name, sizeof(param_name), "Device.WiFi.X_RDKCENTRAL-COM_Connected-Client");
                        if (rc != EOK)
                        {
                            ERR_CHK(rc);
                            return;
                        }
		}
		else
		{
                        rc = strcpy_s(param_name, sizeof(param_name), "Device.WiFi.X_RDKCENTRAL-COM_WiFi_Notification");
                        if (rc != EOK)
                        {
                            ERR_CHK(rc);
                            return;
                        }
		}
					
		ret = CcspBaseIf_setParameterValues(
		  bus_handle,
		  compo,
		  bus,
		  0,
		  DSLH_MPA_ACCESS_CONTROL_NOTIFY_COMP,
		  notif_val,
		  1,
		  TRUE,
		  &faultParam
		  );

		if(ret != CCSP_SUCCESS)
		{
			CcspNotifyCompTraceInfo(("NOTIFICATION: %s : CcspBaseIf_setParameterValues failed. ret value = %d \n", __FUNCTION__, ret));
			CcspNotifyCompTraceInfo(("NOTIFICATION: %s : Parameter = %s \n", __FUNCTION__, notif_val[0].parameterValue));
		}
		
	}

#if defined(FEATURE_SUPPORT_MESH)
    if(PA_Bits & NotifyMask_MESH)
    {
        CcspNotifyCompTraceInfo((" \n Notification : call MESH notification  \n"));

        rc = strcpy_s(compo, sizeof(compo),"eRT.com.cisco.spvtg.ccsp.meshagent");
        if (rc != EOK)
        {
            ERR_CHK(rc);
            return;
        } 
        rc = strcpy_s(bus, sizeof(bus),"/com/cisco/spvtg/ccsp/meshagent");
        if (rc != EOK)
        {
            ERR_CHK(rc);
            return;
        }
        
        rc = strcpy_s(param_name, sizeof(param_name),"Device.DeviceInfo.X_RDKCENTRAL-COM_xOpsDeviceMgmt.Mesh.X_RDKCENTRAL-COM_Connected-Client");
        if (rc != EOK)
        {
            ERR_CHK(rc);
            return;
        }
        char *param;
        char delim[2] = ",";
        char* contextStr = NULL;
        int ind = -1;
        char data[MAX_SIZE_EVT];

        rc = strcpy_s(data, MAX_SIZE_EVT,MsgStr);
        if (rc != EOK)
        {
            ERR_CHK(rc);
            return;
        }
        param = strtok_r(data, delim,&contextStr);
        if(param != NULL)
        {
            //Expected format is Connected-Client,<WiFi or Ethernet>,<mac>,Offline,<hostname>
            param = strtok_r(NULL, delim,&contextStr);
            rc = strcmp_s("Ethernet", strlen("Ethernet"), param , &ind);
            ERR_CHK(rc);
            if(!((!ind) && (rc == EOK)))
            {
                CcspNotifyCompTraceInfo((" \n Connect notify is skipping since it is wifi client = param_name : %s MsgStr = %s \n", param_name,MsgStr));
                return;
            }
        }

        ret = CcspBaseIf_setParameterValues(
          bus_handle,
          compo,
          bus,
          0,
          DSLH_MPA_ACCESS_CONTROL_NOTIFY_COMP,
          notif_val,
          1,
          TRUE,
          &faultParam
          );

        if(ret != CCSP_SUCCESS)
        {
            CcspNotifyCompTraceInfo(("NOTIFICATION: %s : CcspBaseIf_setParameterValues failed. ret value = %d \n", __FUNCTION__, ret));
            CcspNotifyCompTraceInfo(("NOTIFICATION: %s : Parameter = %s \n", __FUNCTION__, notif_val[0].parameterValue));
        }

    }
#endif
}

static void MsgPosttoQueue(char *pMsgStr)
{
	mqd_t mq;
	char buffer[MAX_SIZE] ={0};
        errno_t rc = -1;
	mq = mq_open(EVENT_QUEUE_NAME, O_WRONLY);
	CHECK((mqd_t)-1 != mq);
        /* Coveriyt Fix CID : 135449 STRING_OVERFLOW */
        if(strlen(pMsgStr) < MAX_SIZE)
        {
	  rc = strcpy_s(buffer, MAX_SIZE,pMsgStr);
          if (rc != EOK)
          {
             ERR_CHK(rc);
             return;
          }

          buffer[sizeof(buffer)-1] = '\0';
	}

	CHECK(0 <= mq_send(mq, buffer, MAX_SIZE, 0));
	CHECK((mqd_t)-1 != mq_close(mq));
}

void* Event_HandlerThread(void *threadid)
{
    mqd_t mq;
    struct mq_attr attr;
    char buffer[MAX_SIZE + 1];
    errno_t rc = -1;
    size_t len = 0;

    /* initialize the queue attributes */
    attr.mq_flags = 0;
    attr.mq_maxmsg = 100;
    attr.mq_msgsize = MAX_SIZE;
    attr.mq_curmsgs = 0;

    /* create the message queue */
    mq = mq_open(EVENT_QUEUE_NAME, O_CREAT | O_RDONLY, 0644, &attr);

    /* CID: 60483 Missing return statement 
     * Event_HandlerThread() return type modified from void* to void */
    if ( !((mqd_t)-1 != mq)) {
         fprintf(stderr, "%s:%d: ", __func__, __LINE__);
         perror("((mqd_t)-1 != mq)");
         return NULL;
    }
    do
    {
        ssize_t bytes_read;

        /* receive the message */
        bytes_read = mq_receive(mq, buffer, MAX_SIZE, NULL);
        /* CID: 60483 Missing return statement*/
        if ( !(bytes_read >= 0)) {
            fprintf(stderr, "%s:%d: ", __func__, __LINE__);
            perror("(bytes_read >= 0)");
            return NULL;
        }
        /* CID: 63986 - Array name cant be NULL
         * remove the check since its always TRUE*/
		 

        buffer[bytes_read] = '\0';
		if(!strstr(buffer, "Passphrase"))
		CcspNotifyCompTraceInfo((" \n Notification : Msg recieved from queue = %s\n", buffer));

        {
		char* p_notify_param_name;
		char* st;
		char setnotify_param[512]={0};

                rc = strcpy_s( setnotify_param, sizeof(setnotify_param) ,buffer);
                if (rc != EOK)
                {
                    ERR_CHK(rc);
                    continue;
                }

                len = strlen(buffer);
                if(!len)
                    continue;
		p_notify_param_name = strtok_s(buffer, &len, ",", &st);
                if(!p_notify_param_name)
                    continue;
		Find_Param(p_notify_param_name, setnotify_param);
		CcspNotifyCompTraceInfo((" \n Notification : Msg processed\n"));
        }
    } while (FOREVER());
#ifndef UNIT_TEST_DOCKER_SUPPORT
    pthread_exit(NULL);
#endif
}

void CreateEventHandlerThread()
{
	pthread_t Event_HandlerThreadID;
	int res;
	res = pthread_create(&Event_HandlerThreadID, NULL, Event_HandlerThread, "Event_HandlerThread");
	if(res != 0) {
	CcspNotifyCompTraceInfo(("Create Event_HandlerThread error %d\n", res));
	}

}
/**
 * Write the contents of the linked list into /tmp/.NotifyParamListCache
 * file. This is called when there is a ADD/DEL call of notifyParam
 */
void UpdateNotifyParamFile()
{
	FILE *fptr;
	char str[BUFF_SIZE];
	if ((fptr = fopen(NOTIFY_PARAM_FILE, "w+")) == NULL) {
		CcspNotifyCompTraceInfo(
				("\n Could not open %s file\n", __FUNCTION__));
		return;
	}
	PNotify_param temp = head;
	while (temp != NULL) {
		snprintf(str, sizeof(str), "0x%08u:%s\n", temp->Notify_PA, temp->param_name);   //CID:190147
		fprintf(fptr, "%s", str);
		temp = temp->next;
	}

	//Close file pointer
	if (fptr != NULL) {
		fclose(fptr);
	}

}

/**
 * When the notify_comp crashes and comes back up online, the other
 * components do not register the PAs that need to be notified if the
 * interested TR-181 parameter changes state. To circumvent this, when
 * a PA registers with notify_comp, write it into a file under /tmp  which
 * can then be read when the notify_component restarts after a crash and
 * the PNotify_param link is updated. If file does not exist, it means that
 * the device was rebooted and the registration will happen.
 * The file content is mentioned below
 * 0x00000002:Device.WiFi.SSID.8.Enable
 */
void ReloadNotifyParam()
{
	FILE *fptr;
	char str[BUFF_SIZE];
	char chPA_Name_MASK[64] = {0};
	char chParam_Name[BUFF_SIZE] = {0};
	PNotify_param temp=head;
	PNotify_param prev=head;
	char chDelim[] = ":";
	char *chPtr;
	if ((fptr = fopen(NOTIFY_PARAM_FILE, "r")) == NULL) {
		CcspNotifyCompTraceInfo(
				("\n File %s not present. Nothing to load\n", __FUNCTION__));
		return;
	}
	int icount=0;
	while(fgets(str, BUFF_SIZE,fptr) != NULL)
	{
		memset(chPA_Name_MASK,0,sizeof(chPA_Name_MASK));
		memset(chParam_Name,0,sizeof(chParam_Name));
		chPtr = strtok(str,chDelim);
		if(chPtr != NULL)
		{
			chPtr += 2; //Ignore the "0x" in the PA_Name_MASK
			snprintf(chPA_Name_MASK, sizeof(chPA_Name_MASK), "%s", chPtr);    //CID:190161
		}
		else
		{
			continue; //failsafe
		}

		chPtr = strtok(NULL, chDelim);
		if(chPtr != NULL)
                        snprintf(chParam_Name, sizeof(chParam_Name), "%s", chPtr);      //CID:190161
		else
		{
			continue; //failsafe
		}
		PNotify_param new_node = (PNotify_param) AnscAllocateMemory(
				sizeof(Notify_param));

		if (new_node) {
			/* chParam_Name is upto 1024 bytes, new_node->param_name is 256 bytes, so we need to handle truncation */
			size_t len = strlen(chParam_Name);
			if (len >= sizeof(new_node->param_name))
				len = sizeof(new_node->param_name) - 1;
			memcpy(new_node->param_name, chParam_Name, len);
			new_node->param_name[len] = 0;

			new_node->Notify_PA = atoi(chPA_Name_MASK);
			new_node->next = NULL;

			if (prev == NULL)
			{
				head = prev = new_node;
			}
			else
			{
				temp = prev->next;
				if(temp != NULL)
					prev = temp;
				prev->next = new_node;
			}
			CcspNotifyCompTraceInfo(
					("Notification(R) : Parameter %s is added in the list by %s \n", chParam_Name, chPA_Name_MASK));
		} else {
			CcspNotifyCompTraceInfo(
					("Notification(R) : %s  Failed to Allocate Memory \n", __FUNCTION__));
		}
		icount++;

	}

	if(fptr != NULL)
		fclose(fptr);
}

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
/*********************************************************************************

    description:

        This is the template file of ssp_main.c for XxxxSsp.
        Please replace "XXXX" with your own ssp name with the same up/lower cases.

  ------------------------------------------------------------------------------

    revision:

        09/08/2011    initial revision.

**********************************************************************************/


#ifdef __GNUC__
#ifndef _BUILD_ANDROID
#include <execinfo.h>
#endif
#endif

#include "cosa_apis_NotifyComponent.h"
#include "ssp_global.h"
#include "stdlib.h"
#include "ccsp_dm_api.h"
#include "cosa_notify_wrapper.h" 
#include "safec_lib_common.h"
#include "secure_wrapper.h"

#ifdef INCLUDE_BREAKPAD
#include "breakpad_wrapper.h"
#endif

#include <stdbool.h>
#include "cap.h"
static cap_user appcaps;

extern char*                                pComponentName;
#define DEBUG_INI_NAME  "/etc/debug.ini"
char                                        g_Subsystem[32]         = {0};
static char                         g_NotifyName[256]      = {0};
char*                               g_NotifyCompName         = g_NotifyName;
extern ANSC_HANDLE bus_handle;
/* CID: 58178 Missing Type Specifier*/
extern void CreateEventHandlerThread();
int  cmd_dispatch(int  command)
{

    ANSC_STATUS  returnStatus    = ANSC_STATUS_SUCCESS;
    switch ( command )
    {
        case    'e' :

            CcspNotifyCompTraceNotice(("Connect to bus daemon...\n"));

            {
                char                            CName[256];
		errno_t                         rc               = -1;

                    rc = sprintf_s(CName,sizeof(CName), "%s%s", g_Subsystem, CCSP_COMPONENT_ID_NOTIFYCOMPONENT);
                    if(rc < EOK)
                    {
                        ERR_CHK(rc);
                        return -1;
                    }

                ssp_Mbi_MessageBusEngage
                    ( 
                        CName,
                        CCSP_MSG_BUS_CFG,
                        CCSP_COMPONENT_PATH_NOTIFYCOMPONENT
                    );
            }

          returnStatus = ssp_create();
          if(ANSC_STATUS_SUCCESS != returnStatus)
             return -1;

          returnStatus = ssp_engage();
          if(ANSC_STATUS_SUCCESS != returnStatus)
             return -1;
        
            break;

        case    'm':

                AnscPrintComponentMemoryTable(pComponentName);

                break;

        case    't':

                AnscTraceMemoryTable();

                break;

        case    'c':
                
                returnStatus = ssp_cancel();
                if(ANSC_STATUS_SUCCESS != returnStatus)
                   return -1;

                break;

        default:
            break;
    }

    return 0;
}

static void _print_stack_backtrace(void)
{
#ifdef __GNUC__
#ifndef _BUILD_ANDROID
	void* tracePtrs[100];
	char** funcNames = NULL;
	int i, count = 0;

	count = backtrace( tracePtrs, 100 );
	backtrace_symbols_fd( tracePtrs, count, 2 );

	funcNames = backtrace_symbols( tracePtrs, count );

	if ( funcNames ) {
            // Print the stack trace
	    for( i = 0; i < count; i++ )
		printf("%s\n", funcNames[i] );

            // Free the string pointers
            free( funcNames );
	}
#endif
#endif
}

static void daemonize(void) {
#ifndef  _DEBUG
	int fd;
#endif
	switch (fork()) {
	case 0:
		break;
	case -1:
		// Error
		CcspNotifyCompTraceError(("Error daemonizing (fork)! %d - %s\n", errno, strerror(
				errno)));
		exit(0);
		break;
	default:
		_exit(0);
	}

	if (setsid() < 	0) {
		CcspNotifyCompTraceError(("Error demonizing (setsid)! %d - %s\n", errno, strerror(errno)));
		exit(0);
	}

//	chdir("/");


#ifndef  _DEBUG

	fd = open("/dev/null", O_RDONLY);
	if (fd != 0) {
		dup2(fd, 0);
		close(fd);
	}
	fd = open("/dev/null", O_WRONLY);
	if (fd != 1) {
		dup2(fd, 1);
		close(fd);
	}
	fd = open("/dev/null", O_WRONLY);
	if (fd != 2) {
		dup2(fd, 2);
		close(fd);
	}
#endif
}

void sig_handler(int sig)
{
    if ( sig == SIGINT ) {
    	signal(SIGINT, sig_handler); /* reset it to this function */
    	CcspNotifyCompTraceInfo(("SIGINT received!\n"));
	exit(0);
    }
    else if ( sig == SIGUSR1 ) {
    	signal(SIGUSR1, sig_handler); /* reset it to this function */
    	CcspNotifyCompTraceInfo(("SIGUSR1 received!\n"));
    }
    else if ( sig == SIGUSR2 ) {
    	CcspNotifyCompTraceInfo(("SIGUSR2 received!\n"));
    }
    else if ( sig == SIGCHLD ) {
    	signal(SIGCHLD, sig_handler); /* reset it to this function */
    	CcspNotifyCompTraceInfo(("SIGCHLD received!\n"));
    }
    else if ( sig == SIGPIPE ) {
    	signal(SIGPIPE, sig_handler); /* reset it to this function */
    	CcspNotifyCompTraceInfo(("SIGPIPE received!\n"));
    }
    else if ( sig == SIGALRM ) 
    {

        signal(SIGALRM, sig_handler); /* reset it to this function */
        CcspNotifyCompTraceInfo(("SIGALRM received!\n"));
	}
    else {
    	/* get stack trace first */
    	_print_stack_backtrace();
    	CcspNotifyCompTraceInfo(("Signal %d received, exiting!\n", sig));
        #ifdef FEATURE_SUPPORT_RDKLOG
        rdk_logger_deinit();
        #endif
        ssp_Mbi_Finalize(NULL);
    	exit(0);
    }

}

int drop_root()
{
    int retval = 0;
    CcspNotifyCompTraceInfo(("NonRoot feature is enabled, dropping root privileges for notify_comp process\n"));
    appcaps.caps = NULL;
    appcaps.user_name = NULL;

    if(init_capability() != NULL) {
        if(drop_root_caps(&appcaps) != -1) {
            if(update_process_caps(&appcaps) != -1) {
                read_capability(&appcaps);
                retval = 1;
            }
        }
    }
    return retval;
}

int main(int argc, char* argv[])
{
    ANSC_STATUS                     returnStatus       = ANSC_STATUS_SUCCESS;
    BOOL                            bRunAsDaemon       = TRUE;
    int                             cmdChar            = 0;
    int                             idx = 0;
    errno_t                         rc                 = -1;
    int                             ind                = -1;
    int                             ret                = 0;

    extern ANSC_HANDLE bus_handle;
    char *subSys            = NULL;  
    DmErr_t    err;

    // Buffer characters till newline for stdout and stderr
    setlinebuf(stdout);
    setlinebuf(stderr);

    #ifdef FEATURE_SUPPORT_RDKLOG
    RDK_LOGGER_INIT();
    #endif
   

    for (idx = 1; idx < argc; idx++)
    {
        rc = strcmp_s("-subsys", strlen("-subsys"), argv[idx], &ind);
        ERR_CHK(rc);
	if((rc == EOK) && (ind == 0))
	{
	    if ((idx+1) < argc)
            {
                rc = strcpy_s(g_Subsystem, sizeof(g_Subsystem), argv[idx+1]);
	        if(rc != EOK)
	        {
		        ERR_CHK(rc);
                CcspNotifyCompTraceError(("exit ERROR %s:%d\n", __FUNCTION__, __LINE__));
                #ifdef FEATURE_SUPPORT_RDKLOG
                rdk_logger_deinit();
                #endif
		        exit(1);
	        }
            }
            else
            {
               CcspTraceInfo(("Argument missing after -subsys\n"));
            }
	}
	else
	{
	    rc = strcmp_s("-c", strlen("-c"), argv[idx], &ind);
            ERR_CHK(rc);
	    if((rc == EOK) && (ind == 0))
	    {
		 bRunAsDaemon = FALSE;
            }
	}
    }

    pComponentName          = CCSP_COMPONENT_NAME_NOTIFYCOMPONENT;

    if ( g_NotifyName[0] == 0 )
    {
        AnscCopyString(g_NotifyName, CCSP_COMPONENT_NAME_NOTIFYCOMPONENT);
    }

    if(!drop_root()) {
        CcspNotifyCompTraceError(("drop_root method failed!\n"));
    }

    if ( bRunAsDaemon ) 
        daemonize();

#ifdef INCLUDE_BREAKPAD
    breakpad_ExceptionHandler();
#else
    signal(SIGTERM, sig_handler);
    signal(SIGINT, sig_handler);
    /*signal(SIGCHLD, sig_handler);*/
    signal(SIGUSR1, sig_handler);
    signal(SIGUSR2, sig_handler);

    signal(SIGSEGV, sig_handler);
    signal(SIGBUS, sig_handler);
    signal(SIGKILL, sig_handler);
    signal(SIGFPE, sig_handler);
    signal(SIGILL, sig_handler);
    signal(SIGQUIT, sig_handler);
    signal(SIGHUP, sig_handler);
	signal(SIGALRM, sig_handler);
#endif
   /*Check if /tmp/.NotifyParamListCache file is present.
    * If its present, it means that the notify_comp crashed and is being restarted by SelfHeal.
    * During startup, read the file and load the contents[PA_MASK:param_name] so that they
    * can be notified appropriately.
    */
   ReloadNotifyParam();
   ret = cmd_dispatch('e');
   if(ret != 0)
   {
     CcspNotifyCompTraceError(("exit ERROR %s:%d\n", __FUNCTION__, __LINE__));
     #ifdef FEATURE_SUPPORT_RDKLOG
     rdk_logger_deinit();
     #endif
     ssp_Mbi_Finalize(NULL);
     exit(1);
   }
#ifdef _COSA_SIM_
    subSys = "";        /* PC simu use empty string as subsystem */
#else
    subSys = NULL;      /* use default sub-system */
#endif
    err = Cdm_Init(bus_handle, subSys, NULL, NULL, pComponentName);
    if (err != CCSP_SUCCESS)
    {
        fprintf(stderr, "Cdm_Init: %s\n", Cdm_StrError(err));
        #ifdef FEATURE_SUPPORT_RDKLOG
        rdk_logger_deinit();
        #endif
        ssp_Mbi_Finalize(NULL);
        exit(1);
    }
    //rdk_logger_init("/fss/gw/lib/debug.ini");
    v_secure_system("touch /tmp/Notify_initialized");
	CreateEventHandlerThread();

    if ( bRunAsDaemon )
    {
        while(1)
        {
            sleep(30);
        }
    }
    else
    {
        while ( cmdChar != 'q' )
        {
            cmdChar = getchar();

           ret = cmd_dispatch(cmdChar);
           if(ret != 0)
           {
             CcspNotifyCompTraceError(("exit ERROR %s:%d\n", __FUNCTION__, __LINE__));
             #ifdef FEATURE_SUPPORT_RDKLOG
             rdk_logger_deinit();
             #endif
             ssp_Mbi_Finalize(NULL);
             exit(1);
           }
        }
    }

	err = Cdm_Term();
	if (err != CCSP_SUCCESS)
	{
	fprintf(stderr, "Cdm_Term: %s\n", Cdm_StrError(err));
    #ifdef FEATURE_SUPPORT_RDKLOG
    rdk_logger_deinit();
    #endif
    ssp_Mbi_Finalize(NULL);
	exit(1);
	}

    returnStatus = ssp_cancel();
    if (ANSC_STATUS_SUCCESS != returnStatus)
    {
      CcspNotifyCompTraceError(("exit ERROR %s:%d\n", __FUNCTION__, __LINE__));
      #ifdef FEATURE_SUPPORT_RDKLOG
      rdk_logger_deinit();
      #endif
      exit(1);
    }
    #ifdef FEATURE_SUPPORT_RDKLOG
    rdk_logger_deinit();
    #endif
    return 0;
}


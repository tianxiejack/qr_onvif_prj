/*
 * main.cpp
 *
 *  Created on: 2019年7月22日
 *      Author: jet
 */


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "onvif_dump.h"
#include "onvif_comm.h"

#include "wsseapi.h"
#include "stdsoap2.h"

#if 0
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;
#endif

/************************************************************************
**函数：ONVIF_GetDeviceInformation
**功能：获取设备基本信息
**参数：
        [in] DeviceXAddr - 设备服务地址
**返回：
        0表明成功，非0表明失败
**备注：
************************************************************************/
int ONVIF_GetDeviceInformation(const char *DeviceXAddr)
{
    int result = 0;
    struct soap *soap = NULL;
    struct _tds__GetDeviceInformation           devinfo_req;
    struct _tds__GetDeviceInformationResponse   devinfo_resp;

    SOAP_ASSERT(NULL != DeviceXAddr);
    SOAP_ASSERT(NULL != (soap = ONVIF_soap_new(SOAP_SOCK_TIMEOUT)));

    ONVIF_SetAuthInfo(soap, USERNAME, PASSWORD);

    memset(&devinfo_req, 0x00, sizeof(devinfo_req));
    memset(&devinfo_resp, 0x00, sizeof(devinfo_resp));
    result = soap_call___tds__GetDeviceInformation(soap, DeviceXAddr, NULL, &devinfo_req, &devinfo_resp);
    SOAP_CHECK_ERROR(result, soap, "GetDeviceInformation");

    dump_tds__GetDeviceInformationResponse(&devinfo_resp);

EXIT:

    if (NULL != soap) {
        ONVIF_soap_delete(soap);
    }
    return result;
}


/************************************************************************
**函数：ONVIF_GetCapabilities
**功能：获取设备能力信息
**参数：
        [in] DeviceXAddr - 设备服务地址
**返回：
        0表明成功，非0表明失败
**备注：
    1). 其中最主要的参数之一是媒体服务地址
************************************************************************/
int ONVIF_GetCapabilities(const char *DeviceXAddr)
{
    int result = 0;
    struct soap *soap = NULL;
    struct _tds__GetCapabilities            req;
    struct _tds__GetCapabilitiesResponse    rep;

    SOAP_ASSERT(NULL != DeviceXAddr);
    SOAP_ASSERT(NULL != (soap = ONVIF_soap_new(SOAP_SOCK_TIMEOUT)));

    ONVIF_SetAuthInfo(soap, USERNAME, PASSWORD);

    memset(&req, 0x00, sizeof(req));
    memset(&rep, 0x00, sizeof(rep));
    result = soap_call___tds__GetCapabilities(soap, DeviceXAddr, NULL, &req, &rep);
	    dump_tds__GetCapabilitiesResponse(&rep);
    SOAP_CHECK_ERROR(result, soap, "GetCapabilities");

EXIT:

    if (NULL != soap) {
        ONVIF_soap_delete(soap);
    }
    return result;
}


int ONVIF_GetPtzStatus(const char *DeviceXAddr)
{
	int result = 0;
	struct soap *soap = NULL;


	SOAP_ASSERT(NULL != DeviceXAddr);
	SOAP_ASSERT(NULL != (soap = ONVIF_soap_new(SOAP_SOCK_TIMEOUT)));

	ONVIF_SetAuthInfo(soap, USERNAME, PASSWORD);


	struct _tptz__GetStatus req;
	struct _tptz__GetStatusResponse rep;
	
	memset(&req, 0x00, sizeof(req));
	memset(&rep, 0x00, sizeof(rep));
	result = soap_call___tptz__GetStatus(soap,DeviceXAddr,NULL,&req,&rep);

	SOAP_CHECK_ERROR(result, soap, "GetPTZstatus");
	dump_tds__GetPTZStatus(&rep);
	
EXIT:

    if (NULL != soap) {
        ONVIF_soap_delete(soap);
    }
    return result;
}


int ONVIF_GetNodes(const char *DeviceXAddr)
{
	int result = 0;
	struct soap *soap = NULL;

	SOAP_ASSERT(NULL != DeviceXAddr);
	SOAP_ASSERT(NULL != (soap = ONVIF_soap_new(SOAP_SOCK_TIMEOUT)));

	ONVIF_SetAuthInfo(soap, USERNAME, PASSWORD);

	struct _tptz__GetNodes req;
	struct _tptz__GetNodesResponse rep;

	soap_call___tptz__GetNodes(soap, DeviceXAddr, NULL, &req,&rep);
	
	SOAP_CHECK_ERROR(result, soap, "GetNodes");


EXIT:

    if (NULL != soap) {
        ONVIF_soap_delete(soap);
    }
    return result;

}


int ONVIF_GetConfigurationOptions(const char *DeviceXAddr)
{
	int result = 0;
	struct soap *soap = NULL;

	SOAP_ASSERT(NULL != DeviceXAddr);
	SOAP_ASSERT(NULL != (soap = ONVIF_soap_new(SOAP_SOCK_TIMEOUT)));

	ONVIF_SetAuthInfo(soap, USERNAME, PASSWORD);

	struct _tptz__GetConfigurationOptions req;
	struct _tptz__GetConfigurationOptionsResponse rep;
	
	soap_call___tptz__GetConfigurationOptions(soap,DeviceXAddr,NULL,&req,&rep);

	SOAP_CHECK_ERROR(result, soap, "GetConfigurationOptions");


EXIT:

    if (NULL != soap) {
        ONVIF_soap_delete(soap);
    }
    return result;

}


int ONVIF_checktest(const char *DeviceXAddr)
{
	struct soap *soap = NULL;
	
	//soap_init(soap);
	SOAP_ASSERT(NULL != DeviceXAddr);
	SOAP_ASSERT(NULL != (soap = ONVIF_soap_new(SOAP_SOCK_TIMEOUT)));

	char * ip;
	char Mediaddr[256]="";
	char profile[256]="";
	float pan = 1;
	float panSpeed = 1;
	float tilt = 1;
	float tiltSpeed = 0.5;
	float zoom = 0;
	float zoomSpeed = 0.5;
	struct _tds__GetCapabilities            	req;
	struct _tds__GetCapabilitiesResponse    	rep;
	struct _trt__GetProfiles 			getProfiles;
	struct _trt__GetProfilesResponse		response;	
	struct _tptz__AbsoluteMove           absoluteMove;
	struct _tptz__AbsoluteMoveResponse   absoluteMoveResponse;
	struct _tptz__GetStatus statusReq;
	struct _tptz__GetStatusResponse statusRep;


			
	req.Category = (enum tt__CapabilityCategory *)soap_malloc(soap, sizeof(int));
	req.__sizeCategory = 1;
	*(req.Category) = (enum tt__CapabilityCategory)0;
       
    char endpoint[255];
    memset(endpoint, '\0', 255);

    ip = "192.168.0.64"; 
    
    sprintf(endpoint, "http://%s/onvif/device_service", ip);    

    soap_call___tds__GetCapabilities(soap, endpoint, NULL, &req, &rep);
    if (soap->error)  
    {  
        printf("[%s][%d]--->>> soap result: %d, %s, %s\n", __func__, __LINE__, 
	                                        soap->error, *soap_faultcode(soap), 
	                                        *soap_faultstring(soap));  	 
    } 
    else
	{
        printf("get capability success\n");
        //printf("Dev_XAddr====%s\n",rep.Capabilities->Device->XAddr);
        printf("Med_XAddr====%s\n",rep.Capabilities->Media->XAddr);
        //printf("PTZ_XAddr====%s\n",rep.Capabilities->PTZ->XAddr);
        strcpy(Mediaddr,rep.Capabilities->Media->XAddr);
    }	
    printf("\n");
	
    soap_wsse_add_UsernameTokenDigest(soap, NULL, USERNAME, PASSWORD);
	

    if(soap_call___trt__GetProfiles(soap,Mediaddr,NULL,&getProfiles,&response)==SOAP_OK)
    {
        strcpy(profile, response.Profiles[0].token);
        printf("get profile succeed \n");		
	    printf("profile====%s\n",profile);	
    }
    else
    {
        printf("get profile failed \n");
	    printf("[%s][%d]--->>> soap result: %d, %s, %s\n", __func__, __LINE__, 
	                                        soap->error, *soap_faultcode(soap), 
	                                        *soap_faultstring(soap));  
    }
    printf("\n");	


	char PTZendpoint[255];
	memset(PTZendpoint, '\0', 255);
	sprintf(PTZendpoint, "http://%s/onvif/PTZ", ip);
	printf("PTZendpoint is %s \n", PTZendpoint);  
	statusReq.ProfileToken = profile;
	soap_wsse_add_UsernameTokenDigest(soap, NULL, USERNAME, PASSWORD);

	int result;
	result = soap_call___tptz__GetStatus(soap, PTZendpoint, NULL, &statusReq, 
	                                        &statusRep);			

	SOAP_CHECK_ERROR(result, soap, "GetPTZstatus");
	
	dump_tds__GetPTZStatus(&statusRep);


	
EXIT:
		 

    soap_destroy(soap); // clean up class instances
    soap_end(soap); // clean up everything and close socket, // userid and passwd were deallocated
    soap_done(soap); // close master socket and detach context
    printf("\n");	
		
    return 0;
	
}

void cb_discovery(char *DeviceXAddr)
{
	ONVIF_GetDeviceInformation(DeviceXAddr);

	//int64 t1,t2 ;
	
	//while(1)
	{
		ONVIF_checktest(DeviceXAddr);

	//	printf(" time end points   = %d \n" , getTickCount()/getTickFrequency());

	}
	return ;
}




int main(int argc, char **argv)
{
	//ONVIF_DetectDevice(NULL);

	ONVIF_DetectDevice(cb_discovery);

    return 0;
}

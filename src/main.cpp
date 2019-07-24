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
	    //GetStatus(soap, DeviceXAddr,NULL ,&req, &rep);
    printf("111111\n");

	dump_tds__GetPTZStatus(&rep);
		printf("222222\n");

	SOAP_CHECK_ERROR(result, soap, "GetPTZstatus");

	


EXIT:

    if (NULL != soap) {
        ONVIF_soap_delete(soap);
    }
    return result;
}

void cb_discovery(char *DeviceXAddr)
{
	ONVIF_GetPtzStatus(DeviceXAddr);
}

int main(int argc, char **argv)
{
	//ONVIF_DetectDevice(NULL);

	ONVIF_DetectDevice(cb_discovery);

    return 0;
}

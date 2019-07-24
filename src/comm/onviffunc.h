/*
 * onviffunc.h
 *
 *  Created on: 2019年7月24日
 *      Author: jet
 */

#ifndef ONVIFFUNC_H_
#define ONVIFFUNC_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SOAP_ASSERT     assert
#define SOAP_DBGLOG     printf
#define SOAP_DBGERR     printf

#define SOAP_TO         "urn:schemas-xmlsoap-org:ws:2005:04:discovery"
#define SOAP_ACTION     "http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe"

#define SOAP_MCAST_ADDR "soap.udp://239.255.255.250:3702"                       // onvif规定的组播地址

#define SOAP_ITEM       ""                                                      // 寻找的设备范围
#define SOAP_TYPES      "dn:NetworkVideoTransmitter"                            // 寻找的设备类型

#define SOAP_SOCK_TIMEOUT    (1)                                               // socket超时时间（单秒秒）



void soap_perror(struct soap *soap, const char *str);
void* ONVIF_soap_malloc(struct soap *soap, unsigned int n);
struct soap *ONVIF_soap_new(int timeout);
void ONVIF_soap_delete(struct soap *soap);
void ONVIF_init_header(struct soap *soap);
void ONVIF_init_ProbeType(struct soap *soap, struct wsdd__ProbeType *probe);
void ONVIF_DetectDevice(void (*cb)(char *DeviceXAddr));


#ifdef __cplusplus
}
#endif

#endif /* ONVIFFUNC_H_ */

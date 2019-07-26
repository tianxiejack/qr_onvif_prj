/*
 * main.cpp
 *
 *  Created on: 2019年7月22日
 *      Author: jet
 */

#include <thread>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>


#include "onvif_dump.h"
#include "onvif_comm.h"

#include "wsseapi.h"
#include "stdsoap2.h"


using namespace std;
using namespace cv;


typedef enum{
	PTZ_LEFT,
	PTZ_RIGHT,
	PTZ_UP,
	PTZ_DOWN,
	PTZ_LEFTUP,
	PTZ_LEFTDOWN,
	PTZ_RIGHTUP,
	PTZ_RIGHTDOWN,
	PTZ_ZOOMIN,
	PTZ_ZOOMOUT,
	PTZ_CONTRL_MAX
};

#ifdef __cplusplus
extern "C" {
#endif

#include "libavcodec/avcodec.h"
#include "libavdevice/avdevice.h"
#include "libavformat/avformat.h"
#include "libavfilter/avfilter.h"
#include "libavutil/avutil.h"
#include "libswscale/swscale.h"
#include "libavutil/pixdesc.h"

#ifdef __cplusplus
}
#endif

/************************************************************************
**函数：open_rtsp
**功能：从RTSP获取音视频流数据
**参数：
        [in]  rtsp - RTSP地址
**返回：无
************************************************************************/
void open_rtsp(char *rtsp)
{
    unsigned int    i;
    int             ret;
    int             video_st_index = -1;
    int             audio_st_index = -1;
    AVFormatContext *ifmt_ctx = NULL;
    AVPacket        pkt;
    AVStream        *st = NULL;
    char            errbuf[64];

    bool nRestart = false;
	int videoindex = -1;
	int audioindex = -1;
	AVStream *pVst;
	uint8_t* buffer_rgb = NULL;
	AVCodecContext *pVideoCodecCtx = NULL;
	AVFrame         *pFrame = av_frame_alloc();
	AVFrame         *pFrameRGB = av_frame_alloc();
	int got_picture;
	SwsContext      *img_convert_ctx = NULL;
	AVCodec *pVideoCodec = NULL;


    av_register_all();                                                          // Register all codecs and formats so that they can be used.
    avformat_network_init();                                                    // Initialization of network components

    if ((ret = avformat_open_input(&ifmt_ctx, rtsp, 0, NULL)) < 0) {            // Open the input file for reading.
        printf("Could not open input file '%s' (error '%s')\n", rtsp, av_make_error_string(errbuf, sizeof(errbuf), ret));
        goto EXIT;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, NULL)) < 0) {                // Get information on the input file (number of streams etc.).
        printf("Could not open find stream info (error '%s')\n", av_make_error_string(errbuf, sizeof(errbuf), ret));
        goto EXIT;
    }

    for (i = 0; i < ifmt_ctx->nb_streams; i++) {                                // dump information
        av_dump_format(ifmt_ctx, i, rtsp, 0);
    }

    for (i = 0; i < ifmt_ctx->nb_streams; i++) {                                // find video stream index
        st = ifmt_ctx->streams[i];
        switch(st->codec->codec_type) {
        case AVMEDIA_TYPE_AUDIO: audio_st_index = i; break;
        case AVMEDIA_TYPE_VIDEO: video_st_index = i; break;
        default: break;
        }
    }
    if (-1 == video_st_index) {
        printf("No H.264 video stream in the input file\n");
        goto EXIT;
    }

    av_init_packet(&pkt);                                                       // initialize packet.
    pkt.data = NULL;
    pkt.size = 0;



	while (1)
    {
        do {
            ret = av_read_frame(ifmt_ctx, &pkt);                                // read frames



#if 1
			cout << pkt.size << endl;
			//decode stream
			if (!nRestart)
			{
				for (int i = 0; i < ifmt_ctx->nb_streams; i++)
				{
					if ((ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) && (videoindex < 0))
					{
						videoindex = i;
					}
					if ((ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) && (audioindex < 0))
					{
						audioindex = i;
					}
				}
				pVst = ifmt_ctx->streams[videoindex];
				pVideoCodecCtx = pVst->codec;
				pVideoCodec = avcodec_find_decoder(pVideoCodecCtx->codec_id);
				if (pVideoCodec == NULL)
					return;
				//pVideoCodecCtx = avcodec_alloc_context3(pVideoCodec);
 
				if (avcodec_open2(pVideoCodecCtx, pVideoCodec, NULL) < 0)
					return;
			}
 
			if (pkt.stream_index == videoindex)
			{
				fprintf(stdout, "pkt.size=%d,pkt.pts=%lld, pkt.data=0x%x.", pkt.size, pkt.pts, (unsigned int)*pkt.data);
				int av_result = avcodec_decode_video2(pVideoCodecCtx, pFrame, &got_picture, &pkt);
 
				if (got_picture)
				{
					fprintf(stdout, "decode one video frame!\n");
				}
 
				if (av_result < 0)
				{
					fprintf(stderr, "decode failed: inputbuf = 0x%x , input_framesize = %d\n", pkt.data, pkt.size);
					return;
				}
#if 1
				if (got_picture)
				{
					int bytes = avpicture_get_size(AV_PIX_FMT_RGB24, pVideoCodecCtx->width, pVideoCodecCtx->height);
					buffer_rgb = (uint8_t *)av_malloc(bytes);
					avpicture_fill((AVPicture *)pFrameRGB, buffer_rgb, AV_PIX_FMT_RGB24, pVideoCodecCtx->width, pVideoCodecCtx->height);
 
					img_convert_ctx = sws_getContext(pVideoCodecCtx->width, pVideoCodecCtx->height, pVideoCodecCtx->pix_fmt,
						pVideoCodecCtx->width, pVideoCodecCtx->height, AV_PIX_FMT_BGR24, SWS_FAST_BILINEAR, NULL, NULL, NULL);
					if (img_convert_ctx == NULL)
					{
 
						printf("can't init convert context!\n");
						return;
					}
					sws_scale(img_convert_ctx, pFrame->data, pFrame->linesize, 0, pVideoCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);

					cv::Mat pRgbImg = cv::Mat(pVideoCodecCtx->height,pVideoCodecCtx->width,CV_8UC3);
					//IplImage *pRgbImg = cvCreateImage(cvSize(pVideoCodecCtx->width, pVideoCodecCtx->height), 8, 3);

					memcpy(pRgbImg.data, buffer_rgb, pVideoCodecCtx->width * 3 * pVideoCodecCtx->height);
 
					cv::imshow("GB28181Show", pRgbImg);
 
					//DetectFace(Img);
					cv::waitKey(1);

					sws_freeContext(img_convert_ctx);
					av_free(buffer_rgb);
				}
#endif
			}
 
#endif




	} while (ret == AVERROR(EAGAIN));


        if (ret < 0) {
            printf("Could not read frame (error '%s')\n", av_make_error_string(errbuf, sizeof(errbuf), ret));
            break;
        }

        if (pkt.stream_index == video_st_index) {                               // video frame
            printf("Video Packet size = %d\n", pkt.size);
        } else if(pkt.stream_index == audio_st_index) {                         // audio frame
            printf("Audio Packet size = %d\n", pkt.size);
        } else {
            printf("Unknow Packet size = %d\n", pkt.size);
        }

        av_packet_unref(&pkt);
    }

EXIT:

    if (NULL != ifmt_ctx) {
        avformat_close_input(&ifmt_ctx);
        ifmt_ctx = NULL;
    }
    return ;
}

/************************************************************************
**函数：ONVIF_GetStreamUri
**功能：获取设备码流地址(RTSP)
**参数：
        [in]  MediaXAddr    - 媒体服务地址
        [in]  ProfileToken  - the media profile token
        [out] uri           - 返回的地址
        [in]  sizeuri       - 地址缓存大小
**返回：
        0表明成功，非0表明失败
**备注：
************************************************************************/
int ONVIF_GetStreamUri(const char *MediaXAddr, char *ProfileToken, char *uri, unsigned int sizeuri)
{
    int result = 0;
    struct soap *soap = NULL;
    struct tt__StreamSetup              ttStreamSetup;
    struct tt__Transport                ttTransport;
    struct _trt__GetStreamUri           req;
    struct _trt__GetStreamUriResponse   rep;

    SOAP_ASSERT(NULL != MediaXAddr);
    SOAP_ASSERT(NULL != uri);
    memset(uri, 0x00, sizeuri);

    SOAP_ASSERT(NULL != (soap = ONVIF_soap_new(SOAP_SOCK_TIMEOUT)));

    memset(&req, 0x00, sizeof(req));
    memset(&rep, 0x00, sizeof(rep));
    memset(&ttStreamSetup, 0x00, sizeof(ttStreamSetup));
    memset(&ttTransport, 0x00, sizeof(ttTransport));
    ttStreamSetup.Stream                = tt__StreamType__RTP_Unicast;
    ttStreamSetup.Transport             = &ttTransport;
    ttStreamSetup.Transport->Protocol   = tt__TransportProtocol__RTSP;
    ttStreamSetup.Transport->Tunnel     = NULL;
    req.StreamSetup                     = &ttStreamSetup;
    req.ProfileToken                    = ProfileToken;

    ONVIF_SetAuthInfo(soap, USERNAME, PASSWORD);
    result = soap_call___trt__GetStreamUri(soap, MediaXAddr, NULL, &req, &rep);
    SOAP_CHECK_ERROR(result, soap, "GetServices");

    dump_trt__GetStreamUriResponse(&rep);

    result = -1;
    if (NULL != rep.MediaUri) {
        if (NULL != rep.MediaUri->Uri) {
            if (sizeuri > strlen(rep.MediaUri->Uri)) {
                strcpy(uri, rep.MediaUri->Uri);
                result = 0;
            } else {
                SOAP_DBGERR("Not enough cache!\n");
            }
        }
    }

EXIT:

    if (NULL != soap) {
        ONVIF_soap_delete(soap);
    }

    return result;
}


void callbackAboutGetStream(char *DeviceXAddr)
{
		int stmno = 0;                                                              // 码流序号，0为主码流，1为辅码流
		int profile_cnt = 0;                                                       // 设备配置文件个数
		struct tagProfile *profiles = NULL;                             // 设备配置文件列表
		struct tagCapabilities capa;                                         // 设备能力信息

		char uri[ONVIF_ADDRESS_SIZE] = {0};                          // 不带认证信息的URI地址
		char uri_auth[ONVIF_ADDRESS_SIZE + 50] = {0};          // 带有认证信息的URI地址

		ONVIF_GetCapabilities(DeviceXAddr, &capa);               // 获取设备能力信息（获取媒体服务地址）

		profile_cnt = ONVIF_GetProfiles(capa.MediaXAddr, &profiles);      // 获取媒体配置信息（主/辅码流配置信息）

	    if (profile_cnt > stmno) {
	        ONVIF_GetStreamUri(capa.MediaXAddr, profiles[stmno].token, uri, sizeof(uri)); // 获取RTSP地址

	        make_uri_withauth(uri, USERNAME, PASSWORD, uri_auth, sizeof(uri_auth)); // 生成带认证信息的URI（有的IPC要求认证）

	        open_rtsp(uri_auth);                                                    // 读取主码流的音视频数据
	    }

	    if (NULL != profiles) {
	        free(profiles);
	        profiles = NULL;
	    }


}






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


int ONVIF_checktest_GetPtzStatus(const char *DeviceXAddr)
{
	struct soap *soap = NULL;
	
	//soap_init(soap);
	//SOAP_ASSERT(NULL != DeviceXAddr);
	SOAP_ASSERT(NULL != (soap = ONVIF_soap_new(SOAP_SOCK_TIMEOUT)));

	char * ip;
	char Mediaddr[256]="";
	static char profile[256]="";
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
static int flag = 0;
#if 1
if(flag == 0)
{
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
	flag = 1;
}
#endif

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




int ONVIF_checktest_absoluteMove(const char *DeviceXAddr)
{
	struct soap *soap = NULL;
	
	//soap_init(soap);
	SOAP_ASSERT(NULL != DeviceXAddr);
	SOAP_ASSERT(NULL != (soap = ONVIF_soap_new(SOAP_SOCK_TIMEOUT)));

	char * ip;
	char Mediaddr[256]="";
	static char profile[256]="";
	float pan = 0.1;
	float panSpeed = 1;
	float tilt = 0.2;
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
static int flag = 0;
#if 1
if(flag == 0)
{
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
	flag = 1;
}
#endif


	char PTZendpoint[255];
	memset(PTZendpoint, '\0', 255);
	sprintf(PTZendpoint, "http://%s/onvif/PTZ", ip);
	printf("PTZendpoint is %s \n", PTZendpoint);        
    
	absoluteMove.ProfileToken = profile;
	//setting pan and tilt
	absoluteMove.Position = soap_new_tt__PTZVector(soap, -1);
	absoluteMove.Position->PanTilt = soap_new_tt__Vector2D(soap, -1);
	absoluteMove.Speed = soap_new_tt__PTZSpeed(soap, -1);
	absoluteMove.Speed->PanTilt = soap_new_tt__Vector2D(soap, -1);
	//pan
	absoluteMove.Position->PanTilt->x = pan;
	absoluteMove.Speed->PanTilt->x = panSpeed;
	//tilt
	absoluteMove.Position->PanTilt->y = tilt;
	absoluteMove.Speed->PanTilt->y = tiltSpeed;
	//setting zoom
	absoluteMove.Position->Zoom = soap_new_tt__Vector1D(soap, -1);
	absoluteMove.Speed->Zoom = soap_new_tt__Vector1D(soap, -1);
	absoluteMove.Position->Zoom->x = zoom;
	absoluteMove.Speed->Zoom->x = zoomSpeed;

	soap_wsse_add_UsernameTokenDigest(soap, NULL, USERNAME, PASSWORD);
	soap_call___tptz__AbsoluteMove(soap, PTZendpoint, NULL, &absoluteMove, 
	                                        &absoluteMoveResponse);		
	
EXIT:
		 

    soap_destroy(soap); // clean up class instances
    soap_end(soap); // clean up everything and close socket, // userid and passwd were deallocated
    soap_done(soap); // close master socket and detach context
    printf("\n");	
		
    return 0;
	
}



int ONVIF_checktest_relativeMove(const char *DeviceXAddr)
{
	struct soap *soap = NULL;
	
	//soap_init(soap);
	//SOAP_ASSERT(NULL != DeviceXAddr);
	SOAP_ASSERT(NULL != (soap = ONVIF_soap_new(SOAP_SOCK_TIMEOUT)));

	char * ip;
	char Mediaddr[256]="";
	static char profile[256]="";
	float pan = 0;
	float panSpeed = 0;
	float tilt = 0;
	float tiltSpeed = 0;
	float zoom = 0;
	float zoomSpeed = 0;
	struct _tds__GetCapabilities            	req;
	struct _tds__GetCapabilitiesResponse    	rep;
	struct _trt__GetProfiles 			getProfiles;
	struct _trt__GetProfilesResponse		response;	
	struct _tptz__AbsoluteMove           absoluteMove;
	struct _tptz__AbsoluteMoveResponse   absoluteMoveResponse;
	struct _tptz__GetStatus statusReq;
	struct _tptz__GetStatusResponse statusRep;

struct _tptz__RelativeMove ptz_req;
struct _tptz__RelativeMoveResponse ptz_resp;
memset(&ptz_req, 0x0, sizeof(ptz_req));
memset(&ptz_resp, 0x0, sizeof(ptz_resp));
			
	req.Category = (enum tt__CapabilityCategory *)soap_malloc(soap, sizeof(int));
	req.__sizeCategory = 1;
	*(req.Category) = (enum tt__CapabilityCategory)0;
       
    char endpoint[255];
    memset(endpoint, '\0', 255);

    ip = "192.168.0.64"; 
    
    sprintf(endpoint, "http://%s/onvif/device_service", ip);  
static int flag = 0;
#if 1
if(flag == 0)
{
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
	flag = 1;
}
#endif





	char PTZendpoint[255];
	memset(PTZendpoint, '\0', 255);
	sprintf(PTZendpoint, "http://%s/onvif/PTZ", ip);
	printf("PTZendpoint is %s \n", PTZendpoint);        

ptz_req.ProfileToken = profile;
ptz_req.Translation = soap_new_tt__PTZVector(soap, -1);
ptz_req.Translation->PanTilt =  soap_new_tt__Vector2D(soap, -1);
ptz_req.Translation->Zoom = soap_new_tt__Vector1D(soap,-1);

float move_step = 0.5;
if(move_step >= 1)
{
	move_step = 0.99999;
}
int commond = 1;

switch(commond)
{
	case PTZ_LEFT:
		ptz_req.Translation->PanTilt->x = move_step;
		break;
	case PTZ_RIGHT:
		ptz_req.Translation->PanTilt->x = -move_step;
		break;
	case PTZ_UP:
		ptz_req.Translation->PanTilt->y = -move_step;
		break;
	case PTZ_DOWN:
		ptz_req.Translation->PanTilt->y = move_step;
		break;
	case PTZ_LEFTUP:
		ptz_req.Translation->PanTilt->x = move_step;
		ptz_req.Translation->PanTilt->y = -move_step;
		break;
	case PTZ_LEFTDOWN:
		ptz_req.Translation->PanTilt->x = move_step;
		ptz_req.Translation->PanTilt->y = move_step;
		break;
	case PTZ_RIGHTUP:
		ptz_req.Translation->PanTilt->x = -move_step;
		ptz_req.Translation->PanTilt->y = -move_step;
		break;
	case PTZ_RIGHTDOWN:
		ptz_req.Translation->PanTilt->x = -move_step;
		ptz_req.Translation->PanTilt->y = move_step;
	break;
	case PTZ_ZOOMIN:
		ptz_req.Translation->Zoom->x = move_step;
		break;
	case PTZ_ZOOMOUT:
		ptz_req.Translation->Zoom->x = -move_step;
		break;
	default:
		//goto onvif_OptPTZ_end;
		break;
}


	soap_wsse_add_UsernameTokenDigest(soap, NULL, USERNAME, PASSWORD);
	soap_call___tptz__RelativeMove(soap, PTZendpoint, NULL, &ptz_req, &ptz_resp);

	

EXIT:
		 

    soap_destroy(soap); // clean up class instances
    soap_end(soap); // clean up everything and close socket, // userid and passwd were deallocated
    soap_done(soap); // close master socket and detach context
    printf("\n");	
		
    return 0;
	
}

static float globalVector = 0.5;
int ONVIF_checktest_continuesMove(const char *DeviceXAddr)
{
	struct soap *soap = NULL;
	
	//soap_init(soap);
	//SOAP_ASSERT(NULL != DeviceXAddr);
	SOAP_ASSERT(NULL != (soap = ONVIF_soap_new(SOAP_SOCK_TIMEOUT)));

	char * ip;
	char Mediaddr[256]="";
	static char profile[256]="";
	static float pan = 0.5;
	pan = globalVector;
	float panSpeed = 1;
	float tilt = 0.0;
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

struct _tptz__ContinuousMove ptz_req;
struct _tptz__ContinuousMoveResponse ptz_resp;
memset(&ptz_req, 0x0, sizeof(ptz_req));
memset(&ptz_resp, 0x0, sizeof(ptz_resp));
			
	req.Category = (enum tt__CapabilityCategory *)soap_malloc(soap, sizeof(int));
	req.__sizeCategory = 1;
	*(req.Category) = (enum tt__CapabilityCategory)0;
       
    char endpoint[255];
    memset(endpoint, '\0', 255);

    ip = "192.168.0.64"; 
    
    sprintf(endpoint, "http://%s/onvif/device_service", ip);  
static int flag = 0;
#if 1
if(flag == 0)
{
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
	flag = 1;
}
#endif





	char PTZendpoint[255];
	memset(PTZendpoint, '\0', 255);
	sprintf(PTZendpoint, "http://%s/onvif/PTZ", ip);
	printf("PTZendpoint is %s \n", PTZendpoint);        


ptz_req.ProfileToken = profile;

ptz_req.Velocity = soap_new_tt__PTZSpeed(soap,-1);
ptz_req.Velocity->PanTilt = soap_new_tt__Vector2D(soap, -1);
ptz_req.Velocity->Zoom = soap_new_tt__Vector1D(soap,-1);


float move_step = 0.5;
if(move_step >= 1)
{
	move_step = 0.99999;
}

	ptz_req.Velocity->PanTilt->x = move_step;
	


	soap_wsse_add_UsernameTokenDigest(soap, NULL, USERNAME, PASSWORD);
	soap_call___tptz__ContinuousMove(soap, PTZendpoint, NULL, &ptz_req, &ptz_resp);

	

EXIT:
		 

    soap_destroy(soap); // clean up class instances
    soap_end(soap); // clean up everything and close socket, // userid and passwd were deallocated
    soap_done(soap); // close master socket and detach context
    printf("\n");	
		
    return 0;
	
}


int ONVIF_checktest_stopMove(const char *DeviceXAddr)
{

	usleep(50*1000);
	
	struct soap *soap = NULL;
	
	//soap_init(soap);
	//SOAP_ASSERT(NULL != DeviceXAddr);
	SOAP_ASSERT(NULL != (soap = ONVIF_soap_new(SOAP_SOCK_TIMEOUT)));

	char * ip;
	char Mediaddr[256]="";
	static char profile[256]="";
	float pan = 0.1;
	float panSpeed = 1;
	float tilt = 0.2;
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

struct _tptz__Stop ptz_req;
struct _tptz__StopResponse ptz_resp;
memset(&ptz_req, 0x0, sizeof(ptz_req));
memset(&ptz_resp, 0x0, sizeof(ptz_resp));
			
	req.Category = (enum tt__CapabilityCategory *)soap_malloc(soap, sizeof(int));
	req.__sizeCategory = 1;
	*(req.Category) = (enum tt__CapabilityCategory)0;
       
    char endpoint[255];
    memset(endpoint, '\0', 255);

    ip = "192.168.0.64"; 
    
    sprintf(endpoint, "http://%s/onvif/device_service", ip);  
static int flag = 0;
#if 1
if(flag == 0)
{
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
	flag = 1;
}
#endif





	char PTZendpoint[255];
	memset(PTZendpoint, '\0', 255);
	sprintf(PTZendpoint, "http://%s/onvif/PTZ", ip);
	printf("PTZendpoint is %s \n", PTZendpoint);        


ptz_req.ProfileToken = profile;

ptz_req.PanTilt = soap_new_xsd__boolean(soap,-1);
ptz_req.Zoom = soap_new_xsd__boolean(soap,-1);


float move_step = 0.5;
if(move_step >= 1)
{
	move_step = 0.99999;
}

	*ptz_req.PanTilt = xsd__boolean__true_;
	*ptz_req.Zoom = xsd__boolean__true_;
	


	soap_wsse_add_UsernameTokenDigest(soap, NULL, USERNAME, PASSWORD);
	int result = soap_call___tptz__Stop(soap, PTZendpoint, NULL, &ptz_req, &ptz_resp);
	printf("*************send stop PTZ !!! \n");
	
	SOAP_CHECK_ERROR(result, soap, "ptzStop");
	
EXIT:
		 

    soap_destroy(soap); // clean up class instances
    soap_end(soap); // clean up everything and close socket, // userid and passwd were deallocated
    soap_done(soap); // close master socket and detach context
    printf("\n");	
		
    return 0;
	
}

static bool thFlag = true;
void th_function()  
{  	

	while(thFlag)
	{
		ONVIF_checktest_GetPtzStatus(NULL);
		globalVector += 0.01;
		usleep(30*1000);
		ONVIF_checktest_continuesMove(NULL);
		printf(" time end points   = %f  \n" , getTickCount()/getTickFrequency());

	}

		
} 

void cb_discovery(char *DeviceXAddr)
{

	callbackAboutGetStream(DeviceXAddr);


	return;


	ONVIF_GetDeviceInformation(DeviceXAddr);

	ONVIF_checktest_stopMove(NULL);

	 std::thread t(th_function); 
	 
	//ONVIF_checktest_continuesMove(DeviceXAddr);

	sleep(2);
	
	thFlag = false;

	t.join();

	//ONVIF_checktest_relativeMove(NULL);
	
	ONVIF_checktest_stopMove(NULL);

	return ;
}




int main(int argc, char **argv)
{
	//ONVIF_DetectDevice(NULL);

	ONVIF_DetectDevice(cb_discovery);

    return 0;
}

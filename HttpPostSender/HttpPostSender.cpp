#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include "HttpPostSender.h"
#include "PostMessages.h"

#include "logging.h"
#include "FileConfiguration.h"
#include <curl/curl.h>

#define SENSOR_MAKE "EYELOCK"
#define SENSOR_MODEL "HBOX"

const char logger[] = "HttpPostSender";

using namespace std;

class TimeFormatter
{
public:
	static std::string format(struct tm *input)
	{
		if (input == NULL)
		{
			return std::string("");
		}

		char buffer[21];
		//2011-03-08T05:25:00Z
		strftime(buffer, 21, "%FT%TZ", input);
		return std::string(buffer);
	}
};

HttpPostSender::HttpPostSender(Configuration& conf) : GenericProcessor(conf), curl(NULL), headers(NULL), m_debug(false)
{
	EyelockLog(logger, DEBUG, "curl global initialization");
	curl_global_init(CURL_GLOBAL_ALL);
};

HttpPostSender::~HttpPostSender()
{
	EyelockLog(logger, DEBUG, "curl global deinitialization");

	if (headers)
	{
		curl_slist_free_all(headers); /* free the header list */
		headers = NULL;
	}

	if (curl)
	{
		curl_easy_cleanup(curl);
	}

	curl_global_cleanup();
}

Copyable *HttpPostSender::createNewQueueItem()
{
	return new PostMsg("", PostMsg::UNDEFINED);
}

bool HttpPostSender::enqueMsg(Copyable& msg)
{
	EyelockLog(logger, TRACE, "Enqueing message");
	bool ret = GenericProcessor::enqueMsg(msg);
	EyelockLog(logger, TRACE, "Enqueing message result: %d", ret);

	return ret;
}

string HttpPostSender::mapEndpointName(PostMsg::Endpoint enumVal)
{
	string strVal;

	//UNDEFINED, POST_FACE, POST_IRIS, POST_FACE_IRIS, SIGNAL_ERROR, SIGNAL_MAINTENANCE, SIGNAL_HEARTBEAT, SIGNAL_TRIPWIRE
	switch (enumVal)
	{
		case PostMsg::POST_FACE:
		{
			strVal = string(getConf()->getValue("Eyelock.HttpPostSenderDestPostFace", "PostFace"));
			break;
		}
		case PostMsg::POST_IRIS:
		{
			strVal = string(getConf()->getValue("Eyelock.HttpPostSenderDestPostIris", "PostIris"));
			break;
		}
		case PostMsg::POST_FACE_IRIS:
		{
			strVal = string(getConf()->getValue("Eyelock.HttpPostSenderDestPostFaceIris", "PostFaceIris"));
			break;
		}
		case PostMsg::SIGNAL_ERROR:
		{
			strVal = string(getConf()->getValue("Eyelock.HttpPostSenderDestSignalError", "SignalError"));
			break;
		}
		case PostMsg::SIGNAL_MAINTENANCE:
		{
			strVal = string(getConf()->getValue("Eyelock.HttpPostSenderDestSignalMaintenance", "SignalMaintenance"));
			break;
		}
		case PostMsg::SIGNAL_HEARTBEAT:
		{
			strVal = string(getConf()->getValue("Eyelock.HttpPostSenderDestSignalHeartBeat", "SignalHeartBeat"));
			break;
		}
		case PostMsg::SIGNAL_TRIPWIRE:
		{
			strVal = string(getConf()->getValue("Eyelock.HttpPostSenderDestSignalTripwire", "SignalTripwire"));
			break;
		}
		default:
		{
			EyelockLog(logger, ERROR, "Invalid endpoint specified");
			break;
		}
	}

	return strVal;
}


#if 0
void HttpPostSender::init()
{
	char name[150];
	GenericProcessor::init();
	curl = (void*) curl_easy_init();
	if (curl)
	{
		m_debug = getConf()->getValue("Eyelock.HttpPostSenderDebug", false);
		if (m_debug)
		{
			/* ask libcurl to show us the verbose output */
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		}

		curl_easy_setopt(curl, CURLOPT_POST, 1L);
#if 0
		headers = curl_slist_append(headers, "Content-Type: application/soap+xml; charset=utf-8; action=\”http://gotmb.gov/PostIris”");
		// headers = curl_slist_append(headers, "Content-Type: application/soap+xml; charset=utf-8; action=\”PostIris\”");
		// headers = curl_slist_append(headers, "Content-Type: application/soap+xml; charset=utf-8; action=PostIris");
		// headers = curl_slist_append(headers, "SOAPAction: http://gotmb.gov/PostIris");
		headers = curl_slist_append(headers, "Connection: keep-alive");
		headers = curl_slist_append(headers, "Expect:  ");
#else
		//headers = curl_slist_append(headers, "Content-Type: text/xml; charset=utf-8");
		//headers = curl_slist_append(headers, "Connection: keep-alive");
		//headers = curl_slist_append(headers, "SOAPAction: \"http://gotmb.gov/PostIris\"");
		//headers = curl_slist_append(headers, "Expect:  ");
#if 0
		string destEndpoint = mapEndpointName(((PostMsg*)inputMsg)->getEndpoint());
		sprintf(name, "SOAPAction:\"http://gotmb.gov/%s\"", destEndpoint.c_str());
		printf("destEndpoint.......%s  link.......%s\n", destEndpoint.c_str(), name);
#endif
		headers = curl_slist_append(headers, "Content-Type: text/xml; charset=utf-8");
		headers = curl_slist_append(headers, "Connection: keep-alive");
		#if 1

		// headers = curl_slist_append(headers, "SOAPAction: \"http://gotmb.gov/SignalHeartbeat\"");
		headers = curl_slist_append(headers, "SOAPAction: \"http://gotmb.gov/PostIris\"");


		#else
		headers = curl_slist_append(headers, name);
		#endif
		headers = curl_slist_append(headers, "Accept: text/xml, multipart/related");
		headers = curl_slist_append(headers, "User-Agent: JAX-WS RI 2.2.4-b01");
		headers = curl_slist_append(headers, "Expect:  ");

#endif
		// headers = curl_slist_append(headers, "SOAPAction: http://gotmb.gov/PostIris");
		if (headers)
		{
			/* pass our list of custom made headers */
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		}
		else
		{
			EyelockLog(logger, ERROR, "curl_slist_append() failed");
		}
	}
	else
	{
		EyelockLog(logger, ERROR, "curl_easy_init() failed");
	}
}

void HttpPostSender::process(Copyable *inputMsg)
{
	EyelockLog(logger, TRACE, "Processing message");

	string destScheme = string(getConf()->getValue("Eyelock.HttpPostSenderDestScheme", "https"));
	string destUrl = string(getConf()->getValue("Eyelock.HttpPostSenderDestAddress", ""));
	string destEndpoint = mapEndpointName(((PostMsg*)inputMsg)->getEndpoint());
	EyelockLog(logger, DEBUG, "Endpoint specified: %s", destEndpoint.c_str());

	if (destUrl.empty() || destEndpoint.empty())
	{
		EyelockLog(logger, ERROR, "Invalid destination specified");
	}
	else
	{
		//string dest = destScheme + "://" + destUrl + "/" + destEndpoint;

		string dest = destScheme + "://" + destUrl;  //  + "/" + destEndpoint;

		EyelockLog(logger, DEBUG, "Sending message to %s", dest.c_str());
		sendMsg(*((PostMsg*)inputMsg), dest);
	}
}
#else
void HttpPostSender::init()
{
       GenericProcessor::init();

       msgformat = getConf()->getValue("Eyelock.HttpPostSenderMsgFormat", "FORMAT_SOAP");
       soapnamespace = getConf()->getValue("Eyelock.HttpPostSenderSOAPNamespace", "http://eyelock.com");

		m_xPixelResolutionPerCM = getConf()->getValue("Eyelock.xPixelResolution", 100);
		m_yPixelResolutionPerCM = getConf()->getValue("Eyelock.yPixelResolution", 100);


       curl = (void*) curl_easy_init();
       if (curl)
       {
    	   m_debug = getConf()->getValue("Eyelock.HttpPostSenderDebug", false);
           if (m_debug)
           {
                    /* ask libcurl to show us the verbose output */
        	   curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
           }
           curl_easy_setopt(curl, CURLOPT_POST, 1L);
           int requestTimeout = getConf()->getValue("Eyelock.HttpPostSenderRequestTimeout", 0);
           curl_easy_setopt(curl, CURLOPT_TIMEOUT, requestTimeout);
       }
       else
       {
              EyelockLog(logger, ERROR, "curl_easy_init() failed");
       }
}

void HttpPostSender::process(Copyable *inputMsg)
{
	   char name[150];
       EyelockLog(logger, TRACE, "Processing message");
       string destScheme = string(getConf()->getValue("Eyelock.HttpPostSenderDestScheme", "https"));
       string destUrl = string(getConf()->getValue("Eyelock.HttpPostSenderDestAddress", ""));
       string destEndpoint = mapEndpointName(((PostMsg*)inputMsg)->getEndpoint());
       EyelockLog(logger, DEBUG, "Endpoint specified: %s", destEndpoint.c_str());

       if (destUrl.empty() || destEndpoint.empty())
       {
              EyelockLog(logger, ERROR, "Invalid destination specified");
       }
       else
       {

    	   // http://gotmb.gov/%s
    	   // Build POST headers here...

           if (curl)
           {
        	    curl_slist_free_all(headers); /* free the header list */
        	    headers = NULL;


        	    if (msgformat == "FORMAT_SOAP")
				{
        	    	printf("###### building SOAP HEADERS ########### \n");
					headers = curl_slist_append(headers, "Content-Type: text/xml; charset=utf-8");
					headers = curl_slist_append(headers, "Connection: keep-alive");

					// headers = curl_slist_append(headers, "SOAPAction: \"http://gotmb.gov/PostIris\"");

//					sprintf(name, "SOAPAction: \"http://gotmb.gov/%s\"", destEndpoint.c_str());
					sprintf(name, "SOAPAction: \"%s/%s\"", soapnamespace.c_str(), destEndpoint.c_str());
					printf("destEndpoint.......%s  link.......%s\n", destEndpoint.c_str(), name);
					headers = curl_slist_append(headers, name);


					headers = curl_slist_append(headers, "Accept: text/xml, multipart/related");
					headers = curl_slist_append(headers, "User-Agent: JAX-WS RI 2.2.4-b01");
					headers = curl_slist_append(headers, "Expect:  ");
				}
        	    else if (msgformat == "FORMAT_JSON")
        	    {
        	    	printf("###### building JSON HEADERS ########### \n");

					headers = curl_slist_append(headers, "Content-Type: application/json");
					headers = curl_slist_append(headers, "Accept: application/json");
//					headers = curl_slist_append(headers, "Connection: keep-alive");

					// The endpoint is handled differently for mdtf
					destUrl += "/";
					destUrl += destEndpoint;
        	    }
        	    else
  				  EyelockLog(logger, ERROR, "HTTPPostSenderMsgFormat is invalid.  Check Eyelock.INI...");

				if (headers)
				{
				  /* pass our list of custom made headers */
				  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
				}
				else
				{
				  EyelockLog(logger, ERROR, "curl_slist_append() failed, message not sent");
				  return;
				}

		  }
		  else
		  {
				 EyelockLog(logger, ERROR, "process() failed, no curl object, message not sent");
				 return;
		  }

		  string dest = destScheme + "://" + destUrl;
  		// printf("Sending message to %s", dest.c_str());

		  EyelockLog(logger, DEBUG, "Sending message to %s\n", dest.c_str());
		  sendMsg(*((PostMsg*)inputMsg), dest);
       }
}

#endif

int HttpPostSender::getQueueSize(Configuration* conf)
{
	return getConf()->getValue("Eyelock.HttpPostSenderQSize", 5);
}

int HttpPostSender::sendMsg(const PostMsg& msg, const string& url)
{
	timeval t;
	uint64_t before, after;
	if (m_debug)
	{
		gettimeofday(&t,0);
		before = t.tv_sec * 1000000 + t.tv_usec;
	}

	CURLcode res;
	int ret = 0;

	if (curl)
	{
		EyelockLog(logger, TRACE, "sending message with curl to %s", url.c_str());
		/* First set the URL that is about to receive our POST. This URL can
		   just as well be a https:// URL if that is what should receive the
		   data. */

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

		/* Now specify the POST data */
		string serialized = msg.getXml();
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, serialized.length());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, serialized.c_str());

		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl);
		/* Check for errors */
		if(res != CURLE_OK)
		{
			EyelockLog(logger, ERROR, "curl_easy_perform() failed: %s", curl_easy_strerror(res));
			ret = 2;
		}

	}
	else
	{
		EyelockLog(logger, ERROR, "curl is not initialized");
		ret = 1;
	}

	if (m_debug)
	{
		gettimeofday(&t,0);
		after = t.tv_sec * 1000000 + t.tv_usec;
		EyelockLog(logger, DEBUG, "curl handle: %lu, before: %lu, after: %lu, diff: %lu", (unsigned long)curl, before, after, (after-before));
	}

	return ret;
}

bool HttpPostSender::getDeviceInfo(DeviceInfo& deviceInfo)
{
	EyelockLog(logger, TRACE, "Retrieving data related to the device");
	deviceInfo.egressId = getConf()->getValue("Eyelock.EgressId", 0);
	deviceInfo.ingressId = getConf()->getValue("Eyelock.IngressId", 0);

	deviceInfo.make = SENSOR_MAKE;
	if (deviceInfo.make.empty() || deviceInfo.make.length() > 50)
	{
		EyelockLog(logger, ERROR, "Invalid device make");
		return false;
	}

	deviceInfo.model = SENSOR_MODEL;
	if (deviceInfo.model.empty() || deviceInfo.model.length() > 50)
	{
		EyelockLog(logger, ERROR, "Invalid device model");
		return false;
	}

	deviceInfo.serialNum = getConf()->getValue("Eyelock.DeviceSerialNum", "000");
	if (deviceInfo.serialNum.empty() || deviceInfo.serialNum.length() > 50)
	{
		EyelockLog(logger, ERROR, "Invalid device serial number");
		return false;
	}

	// According to the document, it's either "M followed by the MAC ID" or "Configurable". Or two different fields?
	deviceInfo.uniqueId = getConf()->getValue("Eyelock.DeviceUniqueId", "DeviceUniqueId");

	deviceInfo.locationId = getConf()->getValue("Eyelock.UniqueDeviceLocationId", "UniqueDeviceLocationId");
	if (deviceInfo.locationId.empty() || deviceInfo.locationId.length() > 150)
	{
		EyelockLog(logger, ERROR, "Invalid unique device location ID");
		return false;
	}

	return true;
}

bool HttpPostSender::createFaceStructure(const HTTPPOSTImage& image, const DeviceInfo& deviceInfo, HTTPPostFace& face)
{
	if (image.type != HTTPPOSTImage::FACE)
	{
		EyelockLog(logger, ERROR, "Invalid image type for Face structure");
		return false;
	}
	face.imt = "FACE";

	face.oid = deviceInfo.ingressId;
	face.eid = deviceInfo.egressId;

	face.hll = image.horLineLength;
	face.vll = image.verLineLength;

	switch (image.scaleUnits)
	{
		case HTTPPOSTImage::PIXEL_PER_INCH:
		{
			face.slc = 1;
			break;
		}
		case HTTPPOSTImage::PIXEL_PER_CM:
		{
			face.slc = 2;
			break;
		}
		default:
		{
			EyelockLog(logger, ERROR, "Invalid scale units for Face structure");
			return false;
		}
	}

	face.thps = m_xPixelResolutionPerCM;
	face.tvps = m_yPixelResolutionPerCM;

	switch (image.compressAlg)
	{
		case HTTPPOSTImage::JPEGB:
		{
			face.cga = "JPEGB";
			break;
		}
		case HTTPPOSTImage::PNG:
		{
			face.cga = "PNG";
			break;
		}
		case HTTPPOSTImage::NONE:
		{
			face.cga = "NONE";
			break;
		}
		default:
		{
			EyelockLog(logger, ERROR, "Invalid image compression algorithm for Face structure");
			return false;
		}
	}

	switch (image.colorSpace)
	{
		case HTTPPOSTImage::GRAY:
		{
			face.csp = "GRAY";
			break;
		}
		case HTTPPOSTImage::YCC:
		{
			face.cga = "YCC";
			break;
		}
		case HTTPPOSTImage::RGB:
		{
			face.cga = "RGB";
			break;
		}
		default:
		{
			EyelockLog(logger, ERROR, "Invalid image color space for Face structure");
			return false;
		}
	}

	face.dui = deviceInfo.uniqueId;

	face.mms.mak = deviceInfo.make;
	face.mms.mod = deviceInfo.model;
	face.mms.ser = deviceInfo.serialNum;

	face.geo.grt = deviceInfo.locationId;

	if (image.acquisTime == NULL)
	{
		EyelockLog(logger, DEBUG, "Image acquisition time not specified for Face structure");
	}
	face.geo.ute = TimeFormatter::format(image.acquisTime);

	switch (image.uncompressImageType)
	{
		case HTTPPOSTImage::BMP:
		{
			face.imgt = "BMP";
			break;
		}
		case HTTPPOSTImage::RAW:
		{
			face.imgt = "RAW";
			break;
		}
		default:
		{
			EyelockLog(logger, ERROR, "Invalid uncompressed image type for Face structure");
			return false;
		}
	}

	if (image.acquisTime == NULL || image.size <=0)
	{
		EyelockLog(logger, DEBUG, "Invalid image data for Face structure");
		return false;
	}
	face.data = Serializable::base64(image.data, image.size);

	return true;
}

bool HttpPostSender::createIrisStructure(const HTTPPOSTImage& image, const DeviceInfo& deviceInfo, HTTPPostIris& iris)
{
	if (image.type != HTTPPOSTImage::IRIS)
	{
		EyelockLog(logger, ERROR, "Invalid image type for Iris structure");
		return false;
	}

	// elr field must be set by caller class

	iris.oid = deviceInfo.ingressId;
	iris.eid = deviceInfo.egressId;

	iris.hll = image.horLineLength;
	iris.vll = image.verLineLength;

	switch (image.scaleUnits)
	{
		case HTTPPOSTImage::PIXEL_PER_INCH:
		{
			iris.slc = 1;
			break;
		}
		case HTTPPOSTImage::PIXEL_PER_CM:
		{
			iris.slc = 2;
			break;
		}
		default:
		{
			EyelockLog(logger, ERROR, "Invalid scale units for Iris structure");
			return false;
		}
	}

	iris.thps = m_xPixelResolutionPerCM;
	iris.tvps = m_yPixelResolutionPerCM;

	if (image.compressAlg != HTTPPOSTImage::NONE)
	{
		EyelockLog(logger, ERROR, "Invalid compression algorithm for Iris structure");
		return false;
	}
	iris.cga = "NONE";

	if (image.bppx != 8)
	{
		EyelockLog(logger, ERROR, "Invalid bits per pixel ratio for Iris structure");
		return false;
	}
	iris.bpx = 8;

	if (image.colorSpace != HTTPPOSTImage::GRAY)
	{
		EyelockLog(logger, ERROR, "Invalid color space for Iris structure");
		return false;
	}
	iris.csp = "GRAY"; // why dsp not csp in the document?

	iris.dui = deviceInfo.uniqueId;

	iris.mms.mak = deviceInfo.make;
	iris.mms.mod = deviceInfo.model;
	iris.mms.ser = deviceInfo.serialNum;

	iris.geo.grt = deviceInfo.locationId;
	if (image.acquisTime == NULL)
	{
		EyelockLog(logger, DEBUG, "Image acquisition time not specified for Iris structure");
	}
	iris.geo.ute = TimeFormatter::format(image.acquisTime);

	switch (image.uncompressImageType)
	{
		case HTTPPOSTImage::BMP:
		{
			iris.imgt = "BMP";
			break;
		}
		case HTTPPOSTImage::RAW:
		{
			iris.imgt = "RAW";
			break;
		}
		default:
		{
			EyelockLog(logger, ERROR, "Invalid uncompressed image type for Iris structure");
			return false;
		}
	}

	if (image.data == NULL || image.size <= 0)
	{
		EyelockLog(logger, DEBUG, "Invalid image data for Iris structure");
		return false;
	}

	iris.data = Serializable::base64(image.data, image.size);

	return true;
}


bool HttpPostSender::createIrisStructure(BDBIris *ptheIris, const DeviceInfo& deviceInfo, HTTPPostIris& iris)
{
	// elr field must be set by caller class
	iris.elr = ptheIris->m_EyeLabel == BDBIris::SUBJECT_EYE_LABEL_RIGHT ? 2 : 1;
	iris.dui = deviceInfo.uniqueId;

	iris.oid = deviceInfo.ingressId;
	iris.eid = deviceInfo.egressId;

	iris.hll = ptheIris->m_nImageWidth;
	iris.vll = ptheIris->m_nImageHeight;

	iris.slc = 2; // units per CM hardcoded...

	iris.thps = m_xPixelResolutionPerCM; // from config file...
	iris.tvps = m_yPixelResolutionPerCM;

	iris.bpx = ptheIris->m_BitDepth; // Bits per pixel

	enum CompressionAlgorithm { JPEGB, J2K, PNG, NONE };
	CompressionAlgorithm compressAlg;
	switch (ptheIris->m_ImageFormat)
	{
		default:
		case BDBIris::IMAGEFORMAT_MONO_RAW:
		{
			iris.imgt = "RAW";
			break;
		}

		case BDBIris::IMAGEFORMAT_MONO_JPEG2000:
		{
			iris.imgt = "J2K";
			break;
		}

		case BDBIris::IMAGEFORMAT_MONO_PNG:
		{
			iris.imgt = "PNG";
			break;
		}
	}

	// DMOTODO... dunno what we really need to be setting this to.  Seems redundant, but used by SPARWAR... so just leave at default of NONE
	iris.cga = "NONE";
	iris.csp = "GRAY"; // why dsp not csp in the document?


	iris.mms.mak = deviceInfo.make;
	iris.mms.mod = deviceInfo.model;
	iris.mms.ser = deviceInfo.serialNum;

	iris.geo.grt = deviceInfo.locationId;

	time_t rawtime;

	time(&rawtime);
	struct tm timeinfo = *localtime(&rawtime);
	iris.geo.ute = TimeFormatter::format(&timeinfo);

	//iris.data = Serializable::base64(image.data, image.size);
	vector<unsigned char> theData = ptheIris->GetImageData();

	iris.data = Serializable::base64((char *)theData.data(), theData.size());
	return true;
}


bool HttpPostSender::enquePostFace(const HTTPPOSTImage& image)
{
	EyelockLog(logger, TRACE, "Trying to enque PostFace");

	DeviceInfo deviceInfo;
	if (!getDeviceInfo(deviceInfo))
	{
		EyelockLog(logger, ERROR, "Unable to get device-related info for PostFace message");
		return false;
	}

	HTTPPostFace face;
	if (!createFaceStructure(image, deviceInfo, face))
	{
		return false;
	}

	PostMsg postMsg = PostFace(face).generatePostMsg(msgformat, soapnamespace);
	if (!enqueMsg(postMsg))
	{
		EyelockLog(logger, ERROR, "Unable to enque PostFace");
		return false;
	}

	return true;
}

bool HttpPostSender::enquePostIris(const HTTPPOSTImage& leftEye, const HTTPPOSTImage& rightEye)
{
	EyelockLog(logger, TRACE, "Trying to enque PostIris with two irises");

	DeviceInfo deviceInfo;
	if (!getDeviceInfo(deviceInfo))
	{
		EyelockLog(logger, ERROR, "Unable to get device-related info for PostIris message");
		return false;
	}

	HTTPPostIris leftIris, rightIris;
	leftIris.elr = 1;
	rightIris.elr = 2;
	if (!createIrisStructure(leftEye, deviceInfo, leftIris) || !createIrisStructure(rightEye, deviceInfo, rightIris))
	{
		return false;
	}

	PostMsg postMsg = PostIris(leftIris, rightIris).generatePostMsg(msgformat, soapnamespace);
	if (!enqueMsg(postMsg))
	{
		EyelockLog(logger, ERROR, "Unable to enque PostIris with two irises");
		return false;
	}

	return true;
}

bool HttpPostSender::enquePostIris(const HTTPPOSTImage& eye, int eyePos)
{
	EyelockLog(logger, TRACE, "Trying to enque PostIris with one iris");

	DeviceInfo deviceInfo;
	if (!getDeviceInfo(deviceInfo))
	{
		EyelockLog(logger, ERROR, "Unable to get device-related info for PostIris message");
		return false;
	}

	HTTPPostIris iris;
	iris.elr = eyePos;
	if (!createIrisStructure(eye, deviceInfo, iris))
	{
		return false;
	}

	PostMsg postMsg = PostSingleIris(iris).generatePostMsg(msgformat, soapnamespace);
	if (!enqueMsg(postMsg))
	{
		EyelockLog(logger, ERROR, "Unable to enque PostIris with one iris");
		return false;
	}

	return true;
}

// Data is full ISOBiometric
bool HttpPostSender::enquePostIris(ISOBiometric& ISOBiometricData)
{
	EyelockLog(logger, TRACE, "Trying to enque PostIris with ISOBiometric");

	DeviceInfo deviceInfo;
	if (!getDeviceInfo(deviceInfo))
	{
		EyelockLog(logger, ERROR, "Unable to get device-related info for PostIris message");
		return false;
	}


	HTTPPostIris IrisFirst, IrisSecond;
	HTTPPostIris *pIris;


	bool bBothEyes = false;

	for (int nIris = 0; nIris < ISOBiometricData.GetIrisCount(); nIris++)
	{
		if (nIris > 0)
			bBothEyes = true;

		if (!bBothEyes)
			pIris = &IrisFirst;
		else
			pIris = &IrisSecond;

		BDBIris *ptheIris = ISOBiometricData.GetIrisAt(nIris);

		if (NULL == ptheIris)
			return false;

		if (ptheIris->m_EyeLabel == BDBIris::SUBJECT_EYE_LABEL_RIGHT)
			pIris->elr = 2;
		else
			pIris->elr = 1; // UNKNOWN will be listed as 'Left'.

		if (!createIrisStructure(ptheIris, deviceInfo, *pIris))
			return false;
	}


	PostMsg postMsg = bBothEyes ? PostIris(IrisFirst, IrisSecond).generatePostMsg(msgformat, soapnamespace) : PostSingleIris(IrisFirst).generatePostMsg(msgformat, soapnamespace);


	if (!enqueMsg(postMsg))
	{
		EyelockLog(logger, ERROR, "Unable to enque PostIris");
		return false;
	}
	return true;
}



bool HttpPostSender::enquePostFaceIris(const HTTPPOSTImage& faceImage, const HTTPPOSTImage& leftEyeImage, const HTTPPOSTImage& rightEyeImage)
{
	EyelockLog(logger, TRACE, "Trying to enque PostFaceIris");

	DeviceInfo deviceInfo;
	if (!getDeviceInfo(deviceInfo))
	{
		EyelockLog(logger, ERROR, "Unable to get device-related info for PostFaceIris message");
		return false;
	}

	HTTPPostFace face;
	if (!createFaceStructure(faceImage, deviceInfo, face))
	{
		return false;
	}

	HTTPPostIris leftIris, rightIris;
	leftIris.elr = 1;
	rightIris.elr = 2;
	if (!createIrisStructure(leftEyeImage, deviceInfo, leftIris) || !createIrisStructure(rightEyeImage, deviceInfo, rightIris))
	{
		return false;
	}

	PostMsg postMsg = PostFaceIris(face, leftIris, rightIris).generatePostMsg(msgformat, soapnamespace);
	if (!enqueMsg(postMsg))
	{
		EyelockLog(logger, ERROR, "Unable to enque PostFaceIris");
		return false;
	}

	return true;
}

bool HttpPostSender::enqueSignalError(std::string code, std::string message, ErrorSeverity severity)
{
	SignalError error;

	error.code = code;
	if (error.code.empty() || error.code.length() > 50)
	{
		EyelockLog(logger, ERROR, "Invalid error code");
		return false;
	}

	error.message = message;
	if (error.message.empty() || error.message.length() > 50)
	{
		EyelockLog(logger, ERROR, "Invalid error message");
		return false;
	}

	switch (severity)
	{
		case HttpPostSender::WARN:
		{
			error.severity = "BMP";
			break;
		}
		case HttpPostSender::CRITICAL_ERROR:
		{
			error.severity = "CRITICAL";
			break;
		}
		case HttpPostSender::FATAL_ERROR:
		{
			error.severity = "FATAL";
			break;
		}
		default:
		{
			EyelockLog(logger, ERROR, "Invalid severity for SignalError");
			return false;
		}
	}

	DeviceInfo deviceInfo;
	if (!getDeviceInfo(deviceInfo))
	{
		EyelockLog(logger, ERROR, "Unable to get device-related info for SignalError");
		return false;
	}
	error.dui = deviceInfo.uniqueId; // TODO: length check 4..18? refer to comments in getDeviceInfo()

	PostMsg postMsg = error.generatePostMsg(msgformat, soapnamespace);
	if (!enqueMsg(postMsg))
	{
		EyelockLog(logger, ERROR, "Unable to enque SignalError");
		return false;
	}

	return true;
}

bool HttpPostSender::enqueSignalMaintenance(std::string code, std::string message, MaintenanceSeverity severity)
{
	SignalMaintenance maintenance;

	maintenance.code = code;
	if (maintenance.code.empty() || maintenance.code.length() > 50)
	{
		EyelockLog(logger, ERROR, "Invalid maintenance code");
		return false;
	}

	maintenance.message = message;
	if (maintenance.message.empty() || maintenance.message.length() > 50)
	{
		EyelockLog(logger, ERROR, "Invalid maintenance message");
		return false;
	}

	switch (severity)
	{
		case HttpPostSender::PERIODIC_MAINTENANCE:
		{
			maintenance.severity = "PERIODIC";
			break;
		}
		case HttpPostSender::REQUIRED_MAINTENANCE:
		{
			maintenance.severity = "REQUIRED";
			break;
		}
		case HttpPostSender::CRITICAL_MAINTENANCE:
		{
			maintenance.severity = "CRITICAL";
			break;
		}
		default:
		{
			EyelockLog(logger, ERROR, "Invalid uncompressed image type for PostFace message type");
			return false;
		}
	}

	DeviceInfo deviceInfo;
	if (!getDeviceInfo(deviceInfo))
	{
		EyelockLog(logger, ERROR, "Unable to get device info");
		return false;
	}
	maintenance.dui = deviceInfo.uniqueId;

	PostMsg postMsg = maintenance.generatePostMsg(msgformat, soapnamespace);
	if (!enqueMsg(postMsg))
	{
		EyelockLog(logger, ERROR, "Unable to enque SignalMaintenance");
		return false;
	}

	return true;
}

bool HttpPostSender::enqueSignalHeartbeat()
{
	EyelockLog(logger, TRACE, "Trying to enque SignalHeartbeat");

	SignalHeartbeat hb;

	DeviceInfo deviceInfo;
	if (!getDeviceInfo(deviceInfo))
	{
		EyelockLog(logger, ERROR, "Unable to get device info");
		return false;
	}
	hb.dui = deviceInfo.uniqueId;

	PostMsg postMsg = hb.generatePostMsg(msgformat, soapnamespace);
	if (!enqueMsg(postMsg))
	{
		EyelockLog(logger, ERROR, "Unable to enque SignalHeartbeat");
		return false;
	}

	return true;
}

bool HttpPostSender::enqueSignalTripwire(bool ingress)
{
	SignalTripwire tw;
	tw.dir = (int) ingress;

	DeviceInfo deviceInfo;
	if (!getDeviceInfo(deviceInfo))
	{
		EyelockLog(logger, ERROR, "Unable to get device info");
		return false;
	}

	tw.dui = deviceInfo.uniqueId;
	tw.eid = deviceInfo.egressId;
	tw.oid = deviceInfo.ingressId;

	PostMsg postMsg = tw.generatePostMsg(msgformat, soapnamespace);
	if (!enqueMsg(postMsg))
	{
		EyelockLog(logger, ERROR, "Unable to enque SignalTripwire");
		return false;
	}

	return true;
}


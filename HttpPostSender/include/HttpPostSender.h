#ifndef HTTPPOSTSENDER_H_
#define HTTPPOSTSENDER_H_

#include "GenericProcessor.h"
#include "logging.h"
#include "ISOBiometric.h"

#include <string>
#include <ctime>

// in Image struct, data and time are referenced without copying. Caller must be responsible for memory allocation/deallocation
// in HttpPostSender::enqueXXX methods, data and time referenced in Image struct are copied to internal structures. The memory can be deallocated after method returns

struct HTTPPOSTImage
{
	enum ImageType { FACE, IRIS };
	ImageType type;

	int horLineLength;
	int verLineLength;

	enum ScaleUnits { PIXEL_PER_INCH, PIXEL_PER_CM };
	ScaleUnits scaleUnits;

	int trHorPxScale; // Transmitted horizontal pixel scale
	int trVerPxScale; // Transmitted vertical pixel scale
	int bppx; // Bits per pixel

	enum CompressionAlgorithm { JPEGB, J2K, PNG, NONE };
	CompressionAlgorithm compressAlg;

	enum ColorSpace { GRAY, YCC, RGB };
	ColorSpace colorSpace;

	enum UncompressedImageType { BMP, RAW };
	UncompressedImageType uncompressImageType;

	struct tm *acquisTime; // Universal time of sensor acquisition

	char *data;
	int size;

	HTTPPOSTImage() : type(FACE), horLineLength(0), verLineLength(0), scaleUnits(PIXEL_PER_CM), trHorPxScale(0), trVerPxScale(0), bppx(0), compressAlg(NONE), colorSpace(GRAY), uncompressImageType(RAW), acquisTime(NULL), data(NULL), size(0) {};
};

struct PostMsg : public Copyable
{
	virtual std::string getXml() const { return xml; };
	virtual ~PostMsg() {};

	enum Endpoint { UNDEFINED, POST_FACE, POST_IRIS, POST_FACE_IRIS, SIGNAL_ERROR, SIGNAL_MAINTENANCE, SIGNAL_HEARTBEAT, SIGNAL_TRIPWIRE };

	PostMsg(std::string _xml, Endpoint _endpoint): xml(_xml), endpoint(_endpoint) {};
	virtual void CopyFrom(const Copyable& other)
	{
		xml = ((const PostMsg&)other).xml;
		endpoint = ((const PostMsg&)other).endpoint;
	};

	virtual Endpoint getEndpoint() const { return endpoint; };

protected:
	std::string xml;
	Endpoint endpoint;
};

// forward declaration of internal structures which will be used in methods signatures
struct HTTPPostIris;
struct HTTPPostFace;

struct curl_slist;

class HttpPostSender : public GenericProcessor
{
public:
	HttpPostSender(Configuration& conf);
	virtual ~HttpPostSender();

	virtual void init();
	void process(Copyable *inputMsg);
	int getQueueSize(Configuration* conf);

	Copyable *createNewQueueItem();
	const char *getName() { return "HttpPostSender"; };

	bool enquePostFace(const HTTPPOSTImage& image);
	bool enquePostIris(const HTTPPOSTImage& leftEye, const HTTPPOSTImage& rightEye);
	bool enquePostIris(const HTTPPOSTImage& eye, int eyePos); // eyePos: 1 = left eye, 2 = right eye
	bool enquePostIris(ISOBiometric& ISOImageData); // For ISOBiometric eye side is embedded in ISO structure
	bool enquePostFaceIris(const HTTPPOSTImage& faceImage, const HTTPPOSTImage& leftEyeImage, const HTTPPOSTImage& rightEyeImage);

	enum ErrorSeverity { WARN, CRITICAL_ERROR, FATAL_ERROR };
	bool enqueSignalError(std::string code, std::string message, ErrorSeverity severity);

	enum MaintenanceSeverity { PERIODIC_MAINTENANCE, REQUIRED_MAINTENANCE, CRITICAL_MAINTENANCE };
	bool enqueSignalMaintenance(std::string code, std::string message, MaintenanceSeverity severity);
	bool enqueSignalHeartbeat();
	bool enqueSignalTripwire(bool ingress); // 1 = ingress, 0 = egress

protected:
	bool enqueMsg(Copyable& msg);
	int sendMsg(const PostMsg& msg, const std::string& url);

	struct DeviceInfo
	{
		int ingressId;
		int egressId;

		std::string uniqueId;
		std::string make;
		std::string model;
		std::string serialNum;

		std::string locationId;
	};

	std::string mapEndpointName(PostMsg::Endpoint enumVal);

	bool getDeviceInfo(DeviceInfo& deviceInfo);

	bool createFaceStructure(const HTTPPOSTImage& image, const DeviceInfo& deviceInfo, HTTPPostFace& face);
	bool createIrisStructure(const HTTPPOSTImage& image, const DeviceInfo& deviceInfo, HTTPPostIris& iris);
	bool createIrisStructure(BDBIris *ptheIris, const DeviceInfo& deviceInfo, HTTPPostIris& iris);


	std::string msgformat; //Name of protocol to use for formatting message from Eyelock.HTTPPostSenderMsgFormat... (FORMAT_JSON and FORMAT_SOAP)
	std::string soapnamespace; // Namespace for SOAP msg... defaults to http://eyelock.com

	int m_xPixelResolutionPerCM;
	int m_yPixelResolutionPerCM;


	void *curl;
	struct curl_slist *headers;
	bool m_debug;
};

#endif /* HTTPPOSTSENDER_H_ */


#include <iostream>
#include "logging.h"
#include <ctime>
#include "log4cxx/logger.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/propertyconfigurator.h"
#include "CommonDefs.h"

//#define MQTT_ENABLE

void xxlog(const char * msg)
{
	FILE* fp=fopen(LOGFILE,"a");
		fprintf(fp, msg);
		fclose(fp);

}
void xxlog(const char * msg, ...)
{
	std::time_t now = std::time(NULL);
	std::tm * ptm = std::localtime(&now);
	char timebuff[32];
	std::strftime(timebuff,32, "%a, %d.%m.%Y %H:%M:%S  %Z", ptm);

	va_list args;
	va_start(args, msg);
	FILE* fp=fopen(LOGFILE,"a");
		fprintf(fp,"%s >", timebuff);

		vfprintf(fp, msg,args);
		fclose(fp);

		va_end(args);

}

using namespace log4cxx;
using namespace log4cxx::helpers;
static LogType m_LogLevel;
pthread_mutex_t m_LogLock;
pthread_mutex_t m_EventLock;

#ifdef MQTT_ENABLE
class mqttClient : public mosqpp::mosquittopp
{
private:
 const char    *host;
 const char    *id;

 int           port;
 int           keepalive;

 void on_connect(int rc);
 void on_disconnect(int rc);
 void on_publish(int mid);

public:
 mqttClient(const char *id, const char *host, int port);
 ~mqttClient();
 bool send_message(const char * _message, const char *pTopic);

};

mqttClient *m_pmqtt;

void MQTT_Logging(const char* szTopic, const char *szMsg);
#endif

void EyelockEvent(const char * fmt, ...)
{
	pthread_mutex_lock(&m_LogLock);
	LoggerPtr logger(Logger::getLogger("eventlog"));

	va_list args;
	va_start(args, fmt);
	static char msg[256];
	vsnprintf(msg, 256, fmt, args);
	va_end(args);

	LOG4CXX_FATAL(logger, msg);
	pthread_mutex_unlock(&m_LogLock);
}

void EyelockLogLevel(int logLevel) {
	m_LogLevel = (LogType)logLevel;
	printf("log level %d\n", m_LogLevel);
}
void EyelockLogInit() {
	if (pthread_mutex_init(&m_LogLock, NULL) != 0 || pthread_mutex_init(&m_EventLock, NULL) != 0)
	{
	    printf("\n mutex init failed\n");
	}

#ifdef MQTT_ENABLE
	m_pmqtt = new mqttClient("Eyelock", "127.0.0.1", 1883);
#endif

	//load config...  ONLY ONCE!
	PropertyConfigurator::configure("nxtlog.cfg");
}

void EyelockLogClose() {
	pthread_mutex_destroy(&m_LogLock);
	pthread_mutex_destroy(&m_EventLock);

#ifdef MQTT_ENABLE
	if (NULL != m_pmqtt)
		delete m_pmqtt;
#endif
}
void EyelockLog(const char *filename, LogType level, const char *fmt, ...) {
	if (level < m_LogLevel)
		return;

	pthread_mutex_lock(&m_LogLock);
	LoggerPtr logger(Logger::getLogger("nxtlog"));

	va_list va;
	static char msg[1024];
	static char tmpmsg[256];
	va_start(va, fmt);
	vsnprintf(tmpmsg, 256, fmt, va);
	va_end(va);

	sprintf(msg, "[%s] - %s", filename, tmpmsg);

#ifdef  MQTT_ENABLE
	MQTT_Logging(filename, msg);
#endif

	switch (level) {
		case TRACE:
			LOG4CXX_TRACE(logger, msg);
			break;
		case DEBUG:
			LOG4CXX_DEBUG(logger, msg);
			break;
		case INFO:
			LOG4CXX_INFO(logger, msg);
			break;
		case WARN:
			LOG4CXX_WARN(logger, msg);
			break;
		case ERROR:
			LOG4CXX_ERROR(logger, msg);
			printf(msg);
			break;
		case FATAL:
			LOG4CXX_FATAL(logger, msg);
			break;
		default:
			LOG4CXX_TRACE(logger, msg);
			break;
	}

	pthread_mutex_unlock(&m_LogLock);

}

void PortComLog(const char *filename, LogType level, const char *fmt, ...) {
	if (level < m_LogLevel)
		return;

	pthread_mutex_lock(&m_LogLock);
	LoggerPtr logger(Logger::getLogger("facetrackerlog"));

	va_list va;
	static char msg[1024];
	static char tmpmsg[256];
	va_start(va, fmt);
	vsnprintf(tmpmsg, 256, fmt, va);
	va_end(va);

	sprintf(msg, "[%s] - %s", filename, tmpmsg);


	switch (level) {
		case TRACE:
			LOG4CXX_TRACE(logger, msg);
			break;
		case DEBUG:
			LOG4CXX_DEBUG(logger, msg);
			break;
		case INFO:
			LOG4CXX_INFO(logger, msg);
			break;
		case WARN:
			LOG4CXX_WARN(logger, msg);
			break;
		case ERROR:
			LOG4CXX_ERROR(logger, msg);
			printf(msg);
			break;
		case FATAL:
			LOG4CXX_FATAL(logger, msg);
			break;
		default:
			LOG4CXX_TRACE(logger, msg);
			break;
	}

	pthread_mutex_unlock(&m_LogLock);

}

void MotorLog(const char *filename, LogType level, const char *fmt, ...) {
	if (level < m_LogLevel)
		return;

	pthread_mutex_lock(&m_LogLock);
	LoggerPtr logger(Logger::getLogger("motorlog"));

	va_list va;
	static char msg[1024];
	static char tmpmsg[256];
	va_start(va, fmt);
	vsnprintf(tmpmsg, 256, fmt, va);
	va_end(va);

	sprintf(msg, "[%s] - %s", filename, tmpmsg);


	switch (level) {
		case TRACE:
			LOG4CXX_TRACE(logger, msg);
			break;
		case DEBUG:
			LOG4CXX_DEBUG(logger, msg);
			break;
		case INFO:
			LOG4CXX_INFO(logger, msg);
			break;
		case WARN:
			LOG4CXX_WARN(logger, msg);
			break;
		case ERROR:
			LOG4CXX_ERROR(logger, msg);
			printf(msg);
			break;
		case FATAL:
			LOG4CXX_FATAL(logger, msg);
			break;
		default:
			LOG4CXX_TRACE(logger, msg);
			break;
	}

	pthread_mutex_unlock(&m_LogLock);

}



#ifdef  MQTT_ENABLE
void MQTT_Logging(const char* szTopic, const char *szMsg)
{
	char szFinalTopic[256];
	char szSubTopic[256];
	char *pch;
	int nIndex = 0;

	pch = strchr(szMsg, '{');

	if (pch != NULL)
	{
		// Move to the first subtopic character...
		pch++;

		while (*pch != NULL)
		{
			// Are we done?
			if (*pch == '}')
				break;

			szSubTopic[nIndex++] = *pch++;
		}

		szSubTopic[nIndex] = NULL;
	}
	else
		szSubTopic[nIndex] = NULL; // No subtopic found...

	if (0 == strlen(szSubTopic))
		strcpy(szFinalTopic, "/");

	strcat(szFinalTopic, szTopic);

	if (0 != strlen(szSubTopic))
	{
		strcat(szFinalTopic, "/");
		strcat(szFinalTopic, szSubTopic);
	}

	if (NULL != m_pmqtt)
		m_pmqtt->send_message(szMsg, szFinalTopic);
	// szSubTopic now contains the 'subtopic' text
	// Use 'filename' as the MQTT Topic and szSubTopic as the MQTT subtopic.

	// ADD MQTT Library Calls Here!
	// Build MQTT Message with
	// szTopic
	// szSubTopic
	// and szMsg
}



#define MAX_PAYLOAD 50
#define DEFAULT_KEEP_ALIVE 60


//Mosquitto
mqttClient::mqttClient(const char * _id, const char * _host, int _port) : mosquittopp(_id)
 {
 mosqpp::lib_init();        // Mandatory initialization for mosquitto library
 this->keepalive = 60;    // Basic configuration setup for myMosq class
 this->id = _id;
 this->port = _port;
 this->host = _host;

 connect_async(host,     // non blocking connection to broker request
		 	 port,
		 	 keepalive);

 loop_start();            // Start thread managing connection / publish / subscribe
 };


mqttClient::~mqttClient()
{
	loop_stop();            // Kill the thread
	mosqpp::lib_cleanup();    // Mosquitto library cleanup
 }


bool mqttClient::send_message(const  char * _message, const char *pTopic)
 {
	 // Send message - depending on QoS, mosquitto lib managed re-submission this the thread
	 //
	 // * NULL : Message Id (int *) this allow to latter get status of each message
	 // * topic : topic to be used
	 // * length of the message
	 // * message
	 // * qos (0,1,2)
	 // * retain (boolean) - indicates if message is retained on broker or not
	 // Should return MOSQ_ERR_SUCCESS
	 int ret = publish(NULL,pTopic,strlen(_message),_message,1,false);
	 return ( ret == MOSQ_ERR_SUCCESS );
 }

//on_connect / on_disconnect are called by thread each time we exeperience a server connection / disconnection

void mqttClient::on_disconnect(int rc)
{
	std::cout << ">> myMosq - disconnection(" << rc << ")" << std::endl;
}

void mqttClient::on_connect(int rc)
 {
	 if ( rc == 0 )
	 {
		 std::cout << ">> myMosq - connected with server" << std::endl;
	 }
	 else
	 {
		 std::cout << ">> myMosq - Impossible to connect with server(" << rc << ")" << std::endl;
	 }
 }


void mqttClient::on_publish(int mid)
 {
	std::cout << ">> myMosq - Message (" << mid << ") succeed to be published " << std::endl;
 }
#endif

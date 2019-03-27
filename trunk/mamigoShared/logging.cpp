#include <iostream>
#include "logging.h"
#include <ctime>
#include "log4cxx/logger.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/propertyconfigurator.h"
#include "CommonDefs.h"

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

void EyelockEvent(const char * fmt, ...)
{
	pthread_mutex_lock(&m_LogLock);
	LoggerPtr logger(Logger::getLogger("EyelockEvent"));
//	PropertyConfigurator::configure("nxtevent.cfg");

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
	PropertyConfigurator::configure("nxtlog.cfg");
	PropertyConfigurator::configure("facetrackerlog.cfg");
	PropertyConfigurator::configure("motorlog.cfg");
	PropertyConfigurator::configure("nxtevent.cfg");

	if (pthread_mutex_init(&m_LogLock, NULL) != 0 || pthread_mutex_init(&m_EventLock, NULL) != 0)
	{
	    printf("\n mutex init failed\n");
	}
}
void EyelockLogClose() {
	pthread_mutex_destroy(&m_LogLock);
	pthread_mutex_destroy(&m_EventLock);
}
void EyelockLog(const char *filename, LogType level, const char *fmt, ...) {
	if (level < m_LogLevel)
		return;

	pthread_mutex_lock(&m_LogLock);
	LoggerPtr logger(Logger::getLogger(filename));
//LEAKS (moved to EyelockLogInit only call it once...)	PropertyConfigurator::configure("nxtlog.cfg");

	va_list va;
	static char msg[256];
	va_start(va, fmt);
	vsnprintf(msg, 256, fmt, va);
	va_end(va);

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
	LoggerPtr logger(Logger::getLogger(filename));

	va_list va;
	static char msg[256];
	va_start(va, fmt);
	vsnprintf(msg, 256, fmt, va);
	va_end(va);

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
	LoggerPtr logger(Logger::getLogger(filename));

	va_list va;
	static char msg[256];
	va_start(va, fmt);
	vsnprintf(msg, 256, fmt, va);
	va_end(va);

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



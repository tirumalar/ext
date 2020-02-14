// This header should be included where ever logging functionality is desired
// It provides a macro wrapper around the Log4cxx macros to enable variable
// length argument lists and formatting without incurring the additional overhead
// of unnecessary function calls...
//
// Use EYELOCK_XXXXX Macros to write text log file
// Use EYELOCK_XXXXXLOGIMAGE Macros to create, LogImage objects and write them to a separate binary log file
//

#pragma once

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <log4cxx/logger.h>
#include <LogImageJSON.h> // Interface for logging of images to binary file...


#define IMAGE_LOGGING 1

const char *log_format(const char *fmt, ...);

// Text logging macros
#define EYELOCK_TRACE(logger, fmt, ...) LOG4CXX_TRACE(logger, log_format(fmt, ## __VA_ARGS__))
#define EYELOCK_DEBUG(logger, fmt, ...) LOG4CXX_DEBUG(logger, log_format(fmt, ## __VA_ARGS__))
#define EYELOCK_INFO(logger, fmt, ...) LOG4CXX_INFO(logger, log_format(fmt, ## __VA_ARGS__))
#define EYELOCK_WARN(logger, fmt, ...) LOG4CXX_WARN(logger, log_format(fmt, ## __VA_ARGS__))
#define EYELOCK_ERROR(logger, fmt, ...) LOG4CXX_ERROR(logger, log_format(fmt, ## __VA_ARGS__))
#define EYELOCK_FATAL(logger, fmt, ...) LOG4CXX_FATAL(logger, log_format(fmt, ## __VA_ARGS__))

// We currently support 3 levels for imglog and remoteimglog.log
// never write directly to imglog remoteimglog.  It's in the chain and always gets called when using logger remoteimglog.imglog
// TRACE - if enabled, writes full frames to the log (with or without images depending on setting of remoteimglog.imglog
// DEBUG - if enabled, writes crops to the log (with or without images depending on setting of remoteimglog.imglog
// INFO	 - if enabled, writes metadata only to the log
// WARN, ERROR, FATAL - Set to any one of these to completely disable logging...
//        		LOG4CXX_DEBUG(log4cxx::Logger::getLogger("remoteimglog"), pLogImage->GetObjectAsJSON());\

// If INFO or higher logging is enabled, we log something, otherwise we log nothing...
#ifdef IMAGE_LOGGING
#define EYELOCK_CREATELOGIMAGE_DEBUG(logger, key, imageData, width, height) { \
        if (LOG4CXX_UNLIKELY(logger->isInfoEnabled())) {\
        		if (NULL != imageData)\
					LogImageRecordJSON::put(key, imageData, width, height);\
           }}
#else
	#define EYELOCK_CREATELOGIMAGE_DEBUG(logger, key, imageData, width, height) { }
#endif


#ifdef IMAGE_LOGGING
#define EYELOCK_MODIFYLOGIMAGE_DEBUG(logger, key, pLogImage) { \
        if (LOG4CXX_UNLIKELY(logger->isInfoEnabled())) {\
        	pLogImage = LogImageRecordJSON::get(key);\
           }\
		   else {\
		   	   pLogImage = NULL;\
		   }}
#else
#define EYELOCK_MODIFYLOGIMAGE_DEBUG(logger, key, pLogImage) { pLogImage = NULL; }
#endif

#ifdef IMAGE_LOGGING
#define EYELOCK_WRITELOGIMAGE_DEBUG(logger, key, pLogImage) { \
        if (LOG4CXX_UNLIKELY(logger->isInfoEnabled())) {\
        	pLogImage = LogImageRecordJSON::get(key);\
        	if (NULL != pLogImage) {\
        		LogImageRecordJSON::remove(key); \
        		LOG4CXX_INFO(logger, pLogImage->GetObjectAsJSON());\
        		delete pLogImage; \
        		pLogImage = NULL;\
        	}}\
			else {\
				pLogImage = NULL;\
			  }\
			}
#else
	#define EYELOCK_WRITELOGIMAGE_DEBUG(logger, key, pLogImage) { pLogImage = NULL; }
#endif


// Only log anything related to full frames if TRACE is enabled in remoteimglog.imglog
#define EYELOCK_CREATELOGIMAGE_TRACE(logger, key, imageData, width, height) { \
        if (LOG4CXX_UNLIKELY(logger->isTraceEnabled())) {\
        		EYELOCK_CREATELOGIMAGE_DEBUG(logger, key, imageData, width, height);\
           }}

#define EYELOCK_MODIFYLOGIMAGE_TRACE(logger, key, pLogImage) {\
        if (LOG4CXX_UNLIKELY(logger->isTraceEnabled())) {\
        	EYELOCK_MODIFYLOGIMAGE_DEBUG(logger, key, pLogImage);\
        }else {\
        	pLogImage = NULL;\
          }}

#define EYELOCK_WRITELOGIMAGE_TRACE(logger, key, pLogImage) { \
        if (LOG4CXX_UNLIKELY(logger->isTraceEnabled())) {\
        	EYELOCK_WRITELOGIMAGE_DEBUG(logger, key, pLogImage);\
        }else {\
			pLogImage = NULL;\
        }}\





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

const char *log_format(const char *fmt, ...);

// Text logging macros
#define EYELOCK_TRACE(logger, fmt, ...) LOG4CXX_TRACE(logger, log_format(fmt, ## __VA_ARGS__))
#define EYELOCK_DEBUG(logger, fmt, ...) LOG4CXX_DEBUG(logger, log_format(fmt, ## __VA_ARGS__))
#define EYELOCK_INFO(logger, fmt, ...) LOG4CXX_INFO(logger, log_format(fmt, ## __VA_ARGS__))
#define EYELOCK_WARN(logger, fmt, ...) LOG4CXX_WARN(logger, log_format(fmt, ## __VA_ARGS__))
#define EYELOCK_ERROR(logger, fmt, ...) LOG4CXX_ERROR(logger, log_format(fmt, ## __VA_ARGS__))
#define EYELOCK_FATAL(logger, fmt, ...) LOG4CXX_FATAL(logger, log_format(fmt, ## __VA_ARGS__))

//  These macros are for dealing with log images...
#define EYELOCK_CREATELOGIMAGE_TRACE(logger, key, imageData, width, height) { \
        if (LOG4CXX_UNLIKELY(logger->isTraceEnabled())) {\
        		if (NULL != imageData)\
					LogImageRecordJSON::put(key, imageData, width, height);\
           }}

#define EYELOCK_MODIFYLOGIMAGE_TRACE(logger, key, pLogImage) { \
        if (LOG4CXX_UNLIKELY(logger->isTraceEnabled())) {\
        	pLogImage = LogImageRecordJSON::get(key);\
           }\
		   else {\
		   	   pLogImage = NULL;\
		   }}

#define EYELOCK_WRITELOGIMAGE_TRACE(logger, key, pLogImage) { \
        if (LOG4CXX_UNLIKELY(logger->isTraceEnabled())) {\
        	pLogImage = LogImageRecordJSON::get(key);\
        	if (NULL != pLogImage) {\
        		LogImageRecordJSON::remove(key); \
        		LOG4CXX_TRACE(logger, pLogImage->GetObjectAsJSON());\
        		delete pLogImage; \
        		pLogImage = NULL;\
           }}\
			else {\
				pLogImage = NULL;\
			  }\
			}


#define EYELOCK_CREATELOGIMAGE_DEBUG(logger, key, imageData, width, height) { \
        if (LOG4CXX_UNLIKELY(logger->isDebugEnabled())) {\
        		EYELOCK_CREATELOGIMAGE_TRACE(logger, key, imageData, width, height);\
           }}

#define EYELOCK_MODIFYLOGIMAGE_DEBUG(logger, key, pLogImage) {\
        if (LOG4CXX_UNLIKELY(logger->isDebugEnabled())) {\
        	EYELOCK_MODIFYLOGIMAGE_TRACE(logger, key, pLogImage);\
        }else {\
        	pLogImage = NULL;\
          }}

#define EYELOCK_WRITELOGIMAGE_DEBUG(logger, key, pLogImage) { \
        if (LOG4CXX_UNLIKELY(logger->isDebugEnabled())) {\
        	EYELOCK_WRITELOGIMAGE_TRACE(logger, key, pLogImage);\
        }else {\
			pLogImage = NULL;\
        }}\





/*
 * logging.h
 *
 *  Created on: Apr 21, 2011
 *      Author: developer1
 */

#ifndef LOGGING_H_
#define LOGGING_H_
#include <stdio.h>
#include <stdarg.h>
#include "CommonDefs.h"

#define LOG_TAG "Eyelock"
#define LOGFILE "log.txt"
#ifdef ANDROID
#include <android/log.h>
#  define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#else
#  define QUOTEME_(x) #x
#  define QUOTEME(x) QUOTEME_(x)
#  define LOGI(...) {FILE* fp=fopen(LOGFILE,"a");\
	fprintf(fp, LOG_TAG " (" __FILE__ ":" QUOTEME(__LINE__) "): " __VA_ARGS__);\
	fclose(fp);}

#  define LOG(a, b) {FILE* fp=fopen(LOGFILE,"a");\
	fprintf(fp, a,  b);\
	fclose(fp);}
#endif

enum LogType{ TRACE, DEBUG, INFO, WARN, ERROR, FATAL, NONE };

void xxlog(const char * msg);
void xxlog(const char * msg, ...);
void EyelockLogLevel(int logLevel);
void EyelockLogClose();
void EyelockLogInit();
extern "C" void EyelockEvent(const char * msg, ...);
extern "C" void EyelockLog(const char *filename, LogType level, const char *fmt, ...);
extern "C" void PortComLog(const char *filename, LogType level, const char *fmt, ...);
extern "C" void MotorLog(const char *filename, LogType level, const char *fmt, ...);
#endif /* LOGGING_H_ */

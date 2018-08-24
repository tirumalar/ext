/*
 * eyelock_com.h
 *
 *  Created on: Sep 6, 2017
 *      Author: root
 */

#ifndef EYELOCK_COM_H_
#define EYELOCK_COM_H_



 #ifdef __cplusplus
     extern "C" {
 #endif
     void setRGBled(int R,int G,int B,int mtime,int VIPcall,int mask);
     void MoveTo(int v);
     char* GetTimeStamp();
     void SetFaceMode();
     void RecoverModeDrop();
 #ifdef __cplusplus
     }
 #endif
#define MATCHER_STAT_MATCH   1
#define MATCHER_STAT_DETECT  2
#define MATCHER_STAT_FAIL    3
#define MATCHER_STAT_UNKNOWN 4
extern int g_MatchState ;

#endif /* EYELOCK_COM_H_ */
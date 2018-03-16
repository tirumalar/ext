/*
 * EyelockClientAccessManager.h
 *
 *  Created on: Oct 23, 2013
 *      Author: developer
 */

#ifndef EYELOCKCLIENTACCESSMANAGER_H_
#define EYELOCKCLIENTACCESSMANAGER_H_
#include <thrift/transport/TSSLSocket.h>

#ifdef __APPLE__
    using namespace std;
#endif

namespace apache { namespace thrift { namespace transport {

//class DefaultClientAccessManager;

class EyelockClientAccessManager: public DefaultClientAccessManager {
 public:
	Decision verify(const sockaddr_storage& sa)
	  throw() {
	  (void) sa;
	  printf("EyelockClientAccessManager::SKIP\n");
	  return SKIP;
	}

	Decision verify(const string& host,const char* name,int size) throw() {
	  string eyelock("www.eyelock.com");
	  printf("EyelockClientAccessManager:: %d %s \n",size,name);
	  return DefaultClientAccessManager::verify(eyelock,name,size);
	}

	Decision verify(const sockaddr_storage& sa,const char* data,int size) throw() {
	  bool match = false;
	  if (sa.ss_family == AF_INET && size == sizeof(in_addr)) {
	    match = (memcmp(&((sockaddr_in*)&sa)->sin_addr, data, size) == 0);
	  } else if (sa.ss_family == AF_INET6 && size == sizeof(in6_addr)) {
	    match = (memcmp(&((sockaddr_in6*)&sa)->sin6_addr, data, size) == 0);
	  }
	  if(!match) printf("EyelockClientAccessManager::SKIPIT \n");
	  return (match ? ALLOW : SKIP);
	}
};
}}}


#endif /* EYELOCKCLIENTACCESSMANAGER_H_ */

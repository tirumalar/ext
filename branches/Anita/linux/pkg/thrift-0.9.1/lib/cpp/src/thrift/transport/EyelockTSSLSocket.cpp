/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <thrift/thrift-config.h>

#include <errno.h>
#include <string>
#include <vector>
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#include <sys/types.h>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#include <boost/lexical_cast.hpp>
#include <boost/shared_array.hpp>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>
#include <thrift/concurrency/Mutex.h>
#include <thrift/transport/TSSLSocket.h>
#include <thrift/transport/PlatformSocket.h>

/*Madhav*/
#if defined _WIN32 || defined _WIN64 || defined __APPLE__
#define uint8_t unsigned char
#define uint32_t unsigned int
#define int32_t int
#define uint64_t unsigned long long int
#endif
/*Madhav*/

#define OPENSSL_VERSION_NO_THREAD_ID 0x10000000L

using namespace std;
using namespace boost;
using namespace apache::thrift::concurrency;

struct CRYPTO_dynlock_value {
  Mutex mutex;
};

namespace apache { namespace thrift { namespace transport {
// TSSLSocket implementation
EyelockTSSLSocket::~EyelockTSSLSocket() {
}

EyelockTSSLSocket::EyelockTSSLSocket(boost::shared_ptr<SSLContext> ctx):TSSLSocket(ctx) {
}

EyelockTSSLSocket::EyelockTSSLSocket(boost::shared_ptr<SSLContext> ctx, int socket): TSSLSocket(ctx,socket){
}

EyelockTSSLSocket::EyelockTSSLSocket(boost::shared_ptr<SSLContext> ctx, string host, int port):TSSLSocket(ctx,host,port){
}
void EyelockTSSLSocket::authorize() {
  bool sha1match=false;
  static std::vector<std::string > m_sha1;
//  printf("EyelockTSSLSocket::authorize\n");
  X509* cert = SSL_get_peer_certificate(ssl_);
  if (cert == NULL) {
    // certificate is not present
    if (SSL_get_verify_mode(ssl_) & SSL_VERIFY_FAIL_IF_NO_PEER_CERT) {
      throw TSSLException("authorize: required certificate not present");
    }
    // certificate was optional: didn't intend to authorize remote
    if (server() && access_ != NULL) {
      throw TSSLException("authorize: certificate required for authorization");
    }
    throw TSSLException("authorize: No Certificate present");
  }else{
		if(m_sha1.size()>0){
			for(int i=0;i<m_sha1.size();i++){
				if(memcmp(m_sha1[i].c_str(),cert->sha1_hash,20) == 0){
					sha1match = true;
				}
			}
		}
		if(sha1match){
//			printf("Found SHA in the List\n");
			X509_free(cert);
			return;
		}

		X509_STORE *store = SSL_CTX_get_cert_store(ctx_->get());
		X509_STORE_CTX *csc;
		csc = X509_STORE_CTX_new();
		if (csc == NULL){
			printf("Unable to create store \n");
			X509_free(cert);
			throw TSSLException("authorize: Certificate Not Signed ");
			return;// -1;
		}
		if(!X509_STORE_CTX_init(csc,store,cert,0)){
			printf("Unable to create store init \n");
			X509_free(cert);
			throw TSSLException("authorize: Certificate Not Signed ");
			return; //-2;
		}
		int i=X509_verify_cert(csc);
		X509_STORE_CTX_free(csc);
		if(i == 1){
			string test;
			vector<string>::iterator it;
			it = m_sha1.begin();
			it = m_sha1.insert ( it ,test );
			m_sha1[0].resize(20);
			memcpy((void*)m_sha1[0].c_str(),cert->sha1_hash,20);
//			printf("Added SHA to the List\n");
		}
		if(i != 1){
			X509_free(cert);
			throw TSSLException("authorize: Certificate Not Signed ");
		}
		X509_free(cert);
  }
}
void TSSLSocketFactory::setEyelockSocket(bool es){
	m_eyelockSocket = es;
}

//void TSSLSocketFactory::setCertAuthenticationCallback(void *func,int mode){
//	typedef int (*verifycb)(int preverify_ok, X509_STORE_CTX *ctx);
//
//     SSL_CTX_set_verify(ctx_->get(), mode, verifycb(func));
//}



}}}

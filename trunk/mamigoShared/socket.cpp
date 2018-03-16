/* socket.cpp
 *
 * Copyright (c) 2000 Sean Walton and Macmillan Publishers.  Use may be in
 * whole or in part in accordance to the General Public License (GPL).
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
*/

/*****************************************************************************/
/*** socket.cpp                                                            ***/
/***                                                                       ***/
/*** Implements the C++ socket framework.                                  ***/
/*****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <assert.h>
#include <iostream>
#include "socket.h"

#include "SecureSocket.h"
#include "logging.h"

const char logger[30] = "socket";

///////////////////////////////////////////////////////////////////////////////
/// SimpleString
//-----------------------------------------------------------------------------
//--- Constructors & Destructors
SimpleString::SimpleString(const char* s)
{
	Length = strlen(s);
	Buffer = new char[Length+1];
	strcpy(Buffer, s);
}

SimpleString::SimpleString(const SimpleString& s)
{
	Length = s.GetLength();
	Buffer = new char[Length+1];
	strcpy(Buffer, s.GetString());
}

SimpleString::~SimpleString(void)
{
	delete Buffer;
}

//-----------------------------------------------------------------------------
//--- Operator overload support
void SimpleString::Append(const char* s)
{
	Length += strlen(s);
	char *tmps=new char[Length+1];
	strcpy(tmps, Buffer);
	strcat(tmps, s);
	delete Buffer;
	Buffer = tmps;
}

void SimpleString::Append(const SimpleString& s)
{
	Length += s.GetLength();
	char *tmps=new char[Length+1];
	strcpy(tmps, Buffer);
	strcat(tmps, s.GetString());
	delete Buffer;
	Buffer = tmps;
}

///////////////////////////////////////////////////////////////////////////////
/// Exception
//-----------------------------------------------------------------------------
//--- Specific Implementation
void Exception::PrintException(void) const
{
	fprintf(stderr, "%s", Msg.GetString());
	if ( ErrNo != 0 )
	{
		errno = ErrNo;
		EyelockLog(logger, ERROR, "%s -- Error", Msg.GetString());
		perror("-- Error");
	}
	else
		EyelockLog(logger, DEBUG, "%s -- OK", Msg.GetString());
}

///////////////////////////////////////////////////////////////////////////////
/// HostAddress
//-----------------------------------------------------------------------------
//--- Constructors & Destructors

//
// We have to be able to instantiate a HostAddress with a hostname that is not able to resolve
// at the present time, but is expected to resolve in the future.  The boolean resolve parameter
// allows us to disable the Resolve() call in the constructor that can throw an exception if
// the hostname doesn't resolve.
//
HostAddress* HostAddress::MakeHost(const char* Name, ENetwork network, bool resolve){
	HostAddress *hs = NULL;
	try{
		hs = new HostAddress(Name,network,resolve);
	}
	catch(Exception& nex)
	{
		std::cout<< "Exception during creating HostAddress for " << *Name << endl;
		nex.PrintException();
	}
	catch(const char *msg)
	{
		std::cout<< "Exception during creating HostAddress for " << *Name << endl;
		std::cout<< msg <<endl;
	}
	catch(...)
	{
		std::cout<< "Exception during creating HostAddress for " << *Name << endl;
	}
	return hs;
}

HostAddress::HostAddress(const char* Name, ENetwork network, bool resolve) : Addr(0), Network(network)
{
	// Store original address for case where host name resolution changes (DHCP lease expiration)
	Address[0] = 0;
	if(Name != 0)
	{
		strncpy(Address, Name, MaxHostName-1);
	}

	if(resolve)
	{
		Resolve(Name, network);
	}
}

bool HostAddress::Resolve()
{
	return Resolve(Address[0] ? Address : 0, Network);
}

bool HostAddress::Resolve(const char* Name, ENetwork network)
{
	if(Addr)
	{
		delete Addr;
	}

	switch ( network )
	{
		case eIPv4:
		{	long s_addr;
			int port = 0;

			if ( Name != 0 )
			{
				strncpy(HostName,Name,MaxHostName-1);
				char *portname = strrchr(HostName, ':');
				if ( portname != 0  &&  (port = atoi(portname+1)) > 0 )
					*portname = 0;
				struct hostent *host = gethostbyname(HostName);
				if ( host == NULL )
					throw NetDNSException(SimpleString("gethostbyname [") + HostName + "]");
				s_addr = *reinterpret_cast<long*>(host->h_addr);
			}
			else
				s_addr = INADDR_ANY;
			struct sockaddr_in *addr = new struct sockaddr_in();
			addr->sin_family = network;
			addr->sin_port = htons(port);
			addr->sin_addr.s_addr = s_addr;
			Addr = reinterpret_cast<struct sockaddr*>(addr);
			break;
		}
		case eIPv6:
		{
			struct sockaddr_in6 *addr = new struct sockaddr_in6();
			addr->sin6_family = network;
			addr->sin6_port = 0;
			if ( Name == 0 )
				Name = "0::0";
			if ( inet_pton(eIPv6, Name, &addr->sin6_addr) == 0 )
				throw NetConversionException(SimpleString("inet_pton failed [") + Name + "]");
			Addr = reinterpret_cast<struct sockaddr*>(addr);
			break;
		}
		default :
			throw Exception("Network type not supported", ENOTSUP);
	}
	Network = network;

	return true;
}

HostAddress::HostAddress(const HostAddress& Address) : Addr(0)
{
	Network = Address.Network;

	switch ( Network )
	{
		case eIPv4:
		{
			struct sockaddr_in *addr = new struct sockaddr_in();
			memcpy(addr, Address.Addr, sizeof(*addr));
			Addr = reinterpret_cast<struct sockaddr*>(addr);
			break;
		}
		case eIPv6:
		{
			struct sockaddr_in6 *addr = new struct sockaddr_in6();
			memcpy(addr, Address.Addr, sizeof(*addr));
			Addr = reinterpret_cast<struct sockaddr*>(addr);
			break;
		}
	}
}

HostAddress::~HostAddress(void)
{
	delete Addr;
}

//-----------------------------------------------------------------------------
//--- Specific Implementation
void HostAddress::SetPort(int Port)
{
	switch ( Network )
	{
		case eIPv4:
		{
			struct sockaddr_in *inet = reinterpret_cast<struct sockaddr_in*>(Addr);
			inet->sin_port = htons(Port);
			break;
		}
		case eIPv6:
		{
			struct sockaddr_in6 *inet6 = reinterpret_cast<struct sockaddr_in6*>(Addr);
			inet6->sin6_port = htons(Port);
			break;
		}
	}
}

int HostAddress::GetPort(void) const
{
	switch ( Network )
	{
		case eIPv4: return (reinterpret_cast<struct sockaddr_in*>(Addr))->sin_port;
		case eIPv6: return (reinterpret_cast<struct sockaddr_in6*>(Addr))->sin6_port;
	}
	return 0;
}

int HostAddress::GetSize(void) const
{
	switch ( Network )
	{
		case eIPv4: return sizeof(struct sockaddr_in);
		case eIPv6: return sizeof(struct sockaddr_in6);
	}
	return 0;
}

int HostAddress::operator ==(HostAddress& Address) const
{
	if ( Network != Address.Network )
		return 0;
	switch ( Network )
	{
		case eIPv4:
		case eIPv6: return (memcmp(Addr, Address.Addr, GetSize()) == 0);
		default: return 0;
	}
}

const char* HostAddress::GetHost(bool byName)
{
	switch ( Network )
	{
		case eIPv4:
		{
			char *name;
			struct sockaddr_in *inet = reinterpret_cast<struct sockaddr_in*>(Addr);

			if ( byName )
			{
				char *addr = reinterpret_cast<char*>(&(inet->sin_addr.s_addr));
				int size = sizeof(inet->sin_addr.s_addr);
				struct hostent *host = gethostbyaddr(addr, size, GetNetwork());
				if ( host == NULL )
					throw NetDNSException(SimpleString("gethostbyaddr failed for [") +
						inet_ntoa(inet->sin_addr) + "]");
				name = host->h_name;
			}
			else
				name = inet_ntoa(inet->sin_addr);
			sprintf(HostName, "%s:%d", name, (inet->sin_port));
			break;
		}
		case eIPv6:
		{
			struct sockaddr_in6 *inet6 = reinterpret_cast<struct sockaddr_in6*>(Addr);
			inet_ntop(GetNetwork(), Addr, HostName, MaxHostName);
			char TempS[10];
			sprintf(TempS, ":%d", inet6->sin6_port);
			strncat(HostName, TempS, MaxHostName);
			break;
		}
		default: strncpy(HostName, "<unsupported>", MaxHostName); break;
	}
	HostName[MaxHostName-1] = 0;
	return HostName;
}
///////////////////////////////////////////////////////////////////////////////
/// Socket
//-----------------------------------------------------------------------------
//--- Constructors & Destructors
Socket::Socket(ENetwork Network, EProtocol Protocol):m_isTimoutSet(false),rcvBuff(0), rcvBuffSize(0), m_shouldClose(true),mySecureTrait(0),m_network(Network),m_protocol(Protocol)
{
	SD = socket(Network, Protocol, 0);
	if ( SD < 0 )
		throw NetException("Could not create socket");
	makeBuff();
}

Socket::Socket(const Socket& sock):m_isTimoutSet(false),rcvBuff(0), rcvBuffSize(0),m_shouldClose(true),mySecureTrait(0)
{
	int err;
	socklen_t size=sizeof(err);
	if ( getsockopt(SD, SOL_SOCKET, SO_ERROR, &err, &size) != 0 )
		throw NetException("Socket error");
	if ( (SD = dup(sock.SD)) < 0 )
		throw FileException("Can't copy socket");

	makeBuff(sock.rcvBuffSize);
}

Socket::~Socket(void)
{
	if(m_shouldClose)
	{
		if(mySecureTrait)
			delete mySecureTrait;
		if ( close(SD) != 0 ){
			if(rcvBuff) free(rcvBuff);
			rcvBuff = 0;
			throw FileException("Can't close socket");
		}
	}
	if(rcvBuff)
		free(rcvBuff);
}

void Socket::makeBuff(int size){
	rcvBuffSize=size;
	rcvBuff=(char *)malloc(rcvBuffSize);
}

void Socket::SecureIt()
{
	if(mySecureTrait!=NULL) throw NetConfigException("Socket all ready secured!");
}

void Socket::SecureIt(SecureTrait* secureTrait)
{
	SecureIt();
	mySecureTrait = secureTrait;
}
//-----------------------------------------------------------------------------
//--- Specific Implementation

void Socket::IgnoreSigPipe(){
	signal(SIGPIPE, SIG_IGN);
}
void Socket::Bind(HostAddress& Addr, bool bReuse)
{
	if(bReuse)
		ShareAddress(1);
	if ( bind(SD, Addr.GetAddress(), Addr.GetSize()) != 0 )
		throw NetConnectException("Could not bind socket");
}

int Socket::mySend(char *buf, int bytes, int Options) const
{
	if(mySecureTrait)
	{
		return mySecureTrait->mySend(buf,bytes);
	}
	else
	{
		return send(SD, buf, bytes, Options);
	}
}

int Socket::myRecv(char *buf, int bytes, int Options) const {
	if (mySecureTrait) {
		return mySecureTrait->myRecv(buf, bytes);
	} else {
		return recv(SD, buf, bytes, Options);
	}
}

int Socket::mySendTo(char *buf, int bytes, int Options, struct sockaddr *addr, socklen_t addrSize) const
{
	return sendto(SD, buf, bytes, Options,addr,addrSize);
}
int Socket::myRecvFrom(char *buf, int bytes, int Options, struct sockaddr *addr, socklen_t* addrSize) const
{
	return recvfrom(SD, buf, bytes, Options,addr,addrSize);
}

int  Socket::Send(Message& Msg, int Options) const
{
	int bytes;
	char *buf = Msg.Wrap(bytes);
	bytes = mySend(buf, bytes, Options);
	delete buf;
	if ( bytes < 0 )
		throw NetIOException("Could not send message");
	return bytes;
}
int  Socket::Send(HostAddress& Addr, Message& Msg, int Options) const
{
	int bytes=0;
	bytes = mySendTo(Msg.GetBuffer(), Msg.GetSize(), Options, Addr.GetAddress(), Addr.GetSize());
//	EyelockLog(logger, DEBUG, "sent %d",bytes);
	if ( bytes < 0 )
		throw NetIOException("Could not send directed message");
	return bytes;
}
// send all the bytes, by dividing into smaller packets
#define PACKSIZE 0xC000
void  Socket::SendAll(Message& Msg, int Options) const
{
	int sent=0,trySending=0;
	char* buff=Msg.GetBuffer();
	int bytesLeft=Msg.GetSize();

	while(bytesLeft>0)
	{
		trySending=bytesLeft;
		if(trySending>PACKSIZE) trySending=PACKSIZE;
		sent= mySend(buff, trySending, Options);
		if ( sent < 0 ) {
			EyelockLog(logger, ERROR, "failed to send %d bytes",trySending);
			throw NetIOException("Could not send directed message");
		}
		bytesLeft-=sent;
		buff=buff+sent;
	}
}

void  Socket::SendChunk(Message& Msg, int Options) const
{	bool done = false;
	int bytes=0;
	char *buff;
	do
	{	bytes=PACKSIZE;
		buff = Msg.Wrap(bytes);
//		EyelockLog(logger, DEBUG, "SendFileMsg :: bytes %d",bytes);
		if(bytes==0){
			done = true;
			EyelockLog(logger, DEBUG, "SendChunk :: done %d",done?1:0);
			return;
		}
		int sent = mySend(buff, bytes, Options);
		if ( sent < 0 ) {
			EyelockLog(logger, ERROR, "failed to send %d bytes",bytes);
			throw NetIOException("Could not send directed message");
		}
	}
	while (!done);
	return;
}
// send all the bytes, by dividing into smaller packets

void  Socket::SendAll(HostAddress& Addr, Message& Msg, int Options) const
{
	int sent=0,trySending=0;
	char* buff=Msg.GetBuffer();
	int bytesLeft=Msg.GetSize();

	while(bytesLeft>0)
	{
		trySending=bytesLeft;
		if(trySending>PACKSIZE) trySending=PACKSIZE;
		sent= mySendTo(buff, trySending, Options, Addr.GetAddress(), Addr.GetSize());
		if ( sent < 0 ) {
			EyelockLog(logger, ERROR, "failed to send %d bytes",trySending);
			throw NetIOException("Could not send directed message");
		}
		bytesLeft-=sent;
		buff=buff+sent;
	}
}
/*
 * Does not use socket internal buffer
 */
int  Socket::ReceiveOn(Message& Msg, int Options) const
{
	assert(Msg.isFixed());// do not allow Mutable messages
	bool done;
	int bytes=0;
	int cnt=0;
	do
	{
		bytes = myRecv(Msg.GetBuffer(), Msg.GetSize(), Options);
		if ( bytes <= 0 )
		{
			throw NetIOException("Could not get message");
		}
		cnt++;
		done = Msg.Unwrap(0, bytes, cnt);
	}
	while ( !done );
	return bytes;
}

int  Socket::Receive(Message& Msg, int Options) const
{	bool done;
	int bytes=0;
	int cnt=0;
	do
	{
		bytes = myRecv(rcvBuff, rcvBuffSize, Options);
		if ( bytes <= 0 )
		{
			throw NetIOException("Could not get message");
		}
		cnt++;
		done = Msg.Unwrap(rcvBuff, bytes, cnt);
	}
	while ( !done );
	return bytes;
}
/*
 * Read count bytes in nonblocking fashion
 * assumes that Msg already has enough space
 * return true on success
 */
bool  Socket::Read_nonblocking(Message& Msg, int count) const
{
	char *buf=Msg.GetBuffer();
	int bytes=myRecv(buf, count, MSG_DONTWAIT);
	return bytes==count;
}

/*
 * Reads in nonblocking fashion as much as Msg allows
 * return count of bytes read
 */
int  Socket::Read_nonblocking(Message& Msg) const
{
	return myRecv(Msg.GetBuffer(), Msg.GetSize(), MSG_DONTWAIT);
}
int  Socket::Receive(HostAddress& Addr, Message& Msg, int Options) const
{	socklen_t len = Addr.GetSize();
	bool done;
	int bytes, cnt=0;
	do
	{
		bytes = myRecvFrom(rcvBuff, rcvBuffSize, Options, Addr.GetAddress(), &len);
		if ( bytes < 0 )
		{
			throw NetIOException("Could not get message");
		}
		cnt++;
		done = Msg.Unwrap(rcvBuff, bytes, cnt);
	}
	while ( !done );
	return Msg.GetSize();
}

int Socket::GetSockName(struct sockaddr *address, socklen_t *address_len)
{
	if ( getsockname(SD, address, address_len) != 0 )
		throw NetException("Socket error");
   return 0;
}

int Socket::GetPeerName(struct sockaddr *address, socklen_t *address_len)
{
	if ( getpeername(SD, address, address_len) != 0 )
		throw NetException("Socket error");
   return 0;
}

//-----------------------------------------------------------------------------
//--- Class configuration (CRUD)
void Socket::PermitRoute(bool Setting)
{
	int val = (Setting == 0);
	if ( setsockopt(SD, SOL_SOCKET, SO_DONTROUTE, &val, sizeof(val)) != 0 )
		throw NetConfigException("Socket Option: set DONTROUTE");
}

void Socket::KeepAlive(bool Setting)
{
	int val = (Setting != 0);
	if ( setsockopt(SD, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val)) != 0 )
		throw NetConfigException("Socket Option: set KEEPALIVE");
}

void Socket::ShareAddress(bool Setting)
{
	int val = (Setting != 0);
	if ( setsockopt(SD, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) != 0 )
		throw NetConfigException("Socket Option: set REUSEADDR");
}

int  Socket::GetReceiveSize(void)
{
	int val;
	socklen_t size=sizeof(val);
	if ( getsockopt(SD, SOL_SOCKET, SO_RCVBUF, &val, &size) != 0 )
		throw NetConfigException("Socket Option: get RCVBUF");
	return val;
}

void Socket::SetReceiveSize(int Bytes)
{
	if ( setsockopt(SD, SOL_SOCKET, SO_RCVBUF, &Bytes, sizeof(Bytes)) != 0 )
		throw NetConfigException("Socket Option: set RCVBUF");
}

int  Socket::GetSendSize(void)
{
	int val;
	socklen_t size=sizeof(val);
	if ( getsockopt(SD, SOL_SOCKET, SO_SNDBUF, &val, &size) != 0 )
		throw NetConfigException("Socket Option: get SNDBUF");
	return val;
}

void Socket::SetSendSize(int Bytes)
{
	if ( setsockopt(SD, SOL_SOCKET, SO_SNDBUF, &Bytes, sizeof(Bytes)) != 0 )
		throw NetConfigException("Socket Option: set SNDBUF");
}

int  Socket::GetMinReceive(void)
{
	int val;
	socklen_t size=sizeof(val);
	if ( getsockopt(SD, SOL_SOCKET, SO_RCVLOWAT, &val, &size) != 0 )
		throw NetConfigException("Socket Option: get RCVLOWAT");
	return val;
}

void Socket::SetMinReceive(int Bytes) //---Not yet implemented in Linux
{
	if ( setsockopt(SD, SOL_SOCKET, SO_RCVLOWAT, &Bytes, sizeof(Bytes)) != 0 )
		throw NetConfigException("Socket Option: set RCVLOWAT");
}

int  Socket::GetMinSend(void)
{
	int val;
	socklen_t size=sizeof(val);
	if ( getsockopt(SD, SOL_SOCKET, SO_SNDLOWAT, &val, &size) != 0 )
		throw NetConfigException("Socket Option: get SNDLOWAT");
	return val;
}

void Socket::SetMinSend(int Bytes) //---Not yet implemented in Linux
{
	if ( setsockopt(SD, SOL_SOCKET, SO_SNDLOWAT, &Bytes, sizeof(Bytes)) != 0 )
		throw NetConfigException("Socket Option: set SNDLOWAT");
}

struct timeval Socket::GetReceiveTimeout(void)
{
	struct timeval val;
	socklen_t size=sizeof(val);
	if ( getsockopt(SD, SOL_SOCKET, SO_RCVTIMEO, &val, &size) != 0 )
		throw NetConfigException("Socket Option: get RCVTIMEO");
	return val;
}

void Socket::SetReceiveTimeout(struct timeval& val) //---Not yet implemented in Linux
{
	if ( setsockopt(SD, SOL_SOCKET, SO_RCVTIMEO, &val, sizeof(val)) != 0 )
		throw NetConfigException("Socket Option: set RCVTIMEO");
}

struct timeval Socket::GetSendTimeout(void)
{
	struct timeval val;
	socklen_t size=sizeof(val);
	if ( getsockopt(SD, SOL_SOCKET, SO_SNDTIMEO, &val, &size) != 0 )
		throw NetConfigException("Socket Option: get SNDTIMEO");
	return val;
}

void Socket::SetSendTimeout(struct timeval& val)
{
	if ( setsockopt(SD, SOL_SOCKET, SO_SNDTIMEO, &val, sizeof(val)) != 0 )
		throw NetConfigException("Socket Option: set SNDTIMEO");
}

void Socket::SetTimeouts(struct timeval& val){
	m_timeval = val;
	m_isTimoutSet = true;
	SetSendTimeout(val);
	SetReceiveTimeout(val);
}
ENetwork Socket::GetType(void)
{
	int val;
	socklen_t size=sizeof(val);
	if ( getsockopt(SD, SOL_SOCKET, SO_TYPE, &val, &size) != 0 )
		throw NetConfigException("Socket Option: get TYPE");
	return (ENetwork)val;
}

int  Socket::GetTTL(void)
{
	int val;
	socklen_t size=sizeof(val);
	if ( GetType() == eIPv4 )
	{
		if ( getsockopt(SD, SOL_IP, IP_TTL, &val, &size) != 0 )
			throw NetConfigException("IP Option: get TTL");
	}
	else if ( GetType() == eIPv6 )
	{
		if ( getsockopt(SD, SOL_IPV6, IPV6_UNICAST_HOPS, &val, &size) != 0 )
			throw NetConfigException("IP Option: get MULTICAST_TTL");
	}
	return val;
}

void Socket::SetTTL(int Hops)
{
	if ( GetType() == eIPv4 )
	{
		if ( setsockopt(SD, SOL_IP, IP_TTL, &Hops, sizeof(Hops)) != 0 )
			throw NetConfigException("IP Option: set TTL");
	}
	else if ( GetType() == eIPv6 )
	{
		if ( setsockopt(SD, SOL_IPV6, IPV6_UNICAST_HOPS, &Hops, sizeof(Hops)) != 0 )
			throw NetConfigException("IP Option: set MULTICAST_TTL");
	}
}

int  Socket::GetError(void)
{
	int val;
	socklen_t size=sizeof(val);
	if ( getsockopt(SD, SOL_SOCKET, SO_ERROR, &val, &size) != 0 )
		throw NetConfigException("Socket Option: get ERROR");
	return val;
}

///////////////////////////////////////////////////////////////////////////////
/// SocketStream
//-----------------------------------------------------------------------------
//--- Class configuration (CRUD)

int  SocketStream::GetMaxSegmentSize(void)
{
	int val;
	socklen_t size=sizeof(val);
	if ( getsockopt(SD, SOL_TCP, TCP_MAXSEG, &val, &size) != 0 )
		throw NetConfigException("Socket Option: get TCP_MAXSEG");
	return val;
}

void SocketStream::SetMaxSegmentSize(short Bytes)
{
	if ( setsockopt(SD, SOL_TCP, TCP_MAXSEG, &Bytes, sizeof(Bytes)) != 0 )
		throw NetConfigException("Socket Option: set TCP_MAXSEG");
}

void SocketStream::DontDelay(bool Setting)
{
	if ( setsockopt(SD, SOL_TCP, TCP_NODELAY, &Setting, sizeof(Setting)) != 0 )
		throw NetConfigException("Socket Option: DONTDELAY ");
}

///////////////////////////////////////////////////////////////////////////////
/// SocketServer
//-----------------------------------------------------------------------------
//--- Constructors & Destructors
SocketServer::SocketServer(int Port, ENetwork Network, int QLen): SocketStream(Network)
{
	type = eNone;
	task_fn = 0;
	HostAddress Addr(0);
	Addr.SetPort(Port);
	Bind(Addr);
	if ( listen(SD, QLen) != 0 )
		throw NetConnectException("Listen");
	struct sigaction act;
	memset(&act, 0 , sizeof(act));
	act.sa_handler = SigChild;
	act.sa_flags = SA_NOCLDSTOP;
	if ( sigaction(SIGCHLD, &act, 0) != 0 )
		throw Exception("Sigaction -- processes");
}

SocketServer::SocketServer(HostAddress& Me, int QLen): SocketStream(Me.GetNetwork())
{
	type = eNone;
	task_fn = 0;
	Bind(Me);
	if ( listen(SD, QLen) != 0 )
		throw NetConnectException("Could not convert to listening socket");
	struct sigaction act;
	memset(&act, 0 , sizeof(act));
	act.sa_handler = SigChild;
	act.sa_flags = SA_NOCLDSTOP;
	if ( sigaction(SIGCHLD, &act, 0) != 0 )
		throw Exception("Sigaction -- processes");
}

SocketServer::~SocketServer(void)
{
}

void SocketServer::SecureIt()
{
	Socket::SecureIt();
	mySecureTrait = new SecureTrait(false,0);
	mySecureTrait->initCtx();

}
void SocketServer::AddCipher(std::string cipher)
{
	if(mySecureTrait){
		mySecureTrait->SetCipher(cipher);
	}
}
//-----------------------------------------------------------------------------
//--- Specific Implementation

void SocketServer::OnClientAccept(int clientSD,void (*Server)(Socket& Client))
{
	if(mySecureTrait)
	{
		mySecureTrait->OnClientAccept(clientSD,Server);
	}
	else
	{
		Socket s(clientSD);
		(*Server)(s);
	}
}
void SocketServer::OnClientAccept(int clientSD, void (*Server)(Socket& Client, void *arg), void *arg)
{
	if (mySecureTrait) {
		mySecureTrait->OnClientAccept(clientSD, Server, arg);
	} else {
		Socket s(clientSD);
		(*Server)(s, arg);
	}
}

void SocketServer::Accept(void (*Server)(Socket&))
{
	int client = accept(SD, 0, 0);
	if ( client < 0 )
		throw NetConnectException("Problems with accepting a connection");
	switch ( type )
	{
		case eCallback:
		case eProcess:
		case eThread: break;
	}
	OnClientAccept(client, Server);
}

void SocketServer::Accept(void (*Server)(Socket& Client, void *arg), void *arg){

	int client = accept(SD, 0, 0);
	if ( client < 0 )
		throw NetConnectException("Problems with accepting a connection");
	switch ( type )
	{
		case eCallback:
		case eProcess:
		case eThread: break;
	}
	OnClientAccept(client, Server,arg);

}
void SocketServer::Accept(HostAddress& Addr, void (*Server)(Socket&))
{
	socklen_t size=Addr.GetSize();
	int client = accept(SD, Addr.GetAddress(), &size);
	if ( client < 0 )
		throw NetConnectException("Problems with accepting a connection");
	switch ( type )
	{
		case eCallback:
		case eProcess:
		case eThread: break;
	}
	OnClientAccept(client, Server);
}

//-----------------------------------------------------------------------------
//--- Class configuration (CRUD)
void SocketServer::RegTask(void (*FN)(Socket& socket, HostAddress& addr))
{
}


///////////////////////////////////////////////////////////////////////////////
/// SocketClient
//-----------------------------------------------------------------------------
//--- Constructors & Destructors
SocketClient::SocketClient(HostAddress& Host, ENetwork Network): SocketStream(Network)
{
	Connect(Host);
}

void SocketClient::SecureIt()
{
	Socket::SecureIt();
	mySecureTrait = new SecureTrait(true,0);
	mySecureTrait->initCtx();
	mySecureTrait->initSSL(SD);

}
//-----------------------------------------------------------------------------
//--- Specific Implementation


void SocketClient::Connect(HostAddress& Address)
{
	if(!Address.GetAddress())
	{
		Address.Resolve(); // throws on failure
	}

	if ( connect(SD, Address.GetAddress(), Address.GetSize()) != 0 )
	{
		throw NetConnectException("Connect");
	}
	else
	{
		if(mySecureTrait)
		{
			mySecureTrait->Connect(Address);
		}
	}
}

/*
 * Since HostAddress name resolution is cached, connect() may fail
 * due to a modified IP address, such as in the case of DHCP lease expiration
 * on the host machine. In case of failure this method will perform name resolution
 * on the fly followed by a second connection attempt to the host.  This provides
 * a centralized approach to handling DHCP configured systems.  We may also need
 * to perform name resolution prior to the initial connect() call in case the input
 * HostAddress has not yet resolved.  This may be the case when the host joins the
 * network intermittently.
 */

void SocketClient::ConnectByHostname(HostAddress& Address)
{
	if(!Address.GetAddress())  // Address hasn't yet been resolved.
	{
		Address.Resolve(); // throws on failure
	}

	int status = connect(SD, Address.GetAddress(), Address.GetSize());
	if (status != 0)
	{
		close(SD);
		SD = socket(m_network, m_protocol, 0);
		SetTimeout();
		Address.Resolve();
		status = connect(SD, Address.GetAddress(), Address.GetSize());
	}

	if(status == 0)
	{
		if(mySecureTrait)
		{
			mySecureTrait->Connect(Address);
		}
	}
	else
	{
		throw NetConnectException("Connect");
	}
}


///////////////////////////////////////////////////////////////////////////////
/// Datagram
//-----------------------------------------------------------------------------
//--- Constructors & Destructors

Datagram::Datagram(HostAddress& Me, ENetwork Network, EProtocol Protocol): Socket(Network, Protocol)
{
	Bind(Me);
}

Datagram::Datagram(ENetwork Network, EProtocol Protocol): Socket(Network, Protocol)
{}

//-----------------------------------------------------------------------------
//--- Private class configuration (CRUD)

void Datagram::SetTOS(bool Setting, int Val)
{
	int tos;
	socklen_t size=sizeof(tos);
	if ( getsockopt(SD, SOL_IP, IP_TOS, &tos, &size) != 0 )
		throw NetConfigException("Socket Option: get IP_TOS");
	if ( Setting != 0 )
		tos |= Val;
	else
		tos &= ~Val;
	if ( setsockopt(SD, SOL_IP, IP_TOS, &tos, sizeof(tos)) != 0 )
		throw NetConfigException("Socket Option: set IP_TOS");
}

//-----------------------------------------------------------------------------
//--- Class configuration (CRUD)

void Datagram::MinimizeDelay(bool Setting)
{
	SetTOS(Setting, IPTOS_LOWDELAY);
}

void Datagram::MaximizeThroughput(bool Setting)
{
	SetTOS(Setting, IPTOS_THROUGHPUT);
}

void Datagram::MaximizeReliability(bool Setting)
{
	SetTOS(Setting, IPTOS_RELIABILITY);
}

void Datagram::MinimizeCost(bool Setting)
{
#ifndef __ANDRIOD__
	SetTOS(Setting, IPTOS_LOWCOST);
#else
	EyelockLog(logger, DEBUG, "SetTOS(Setting, IPTOS_LOWCOST) Commented in android");
#endif
}

void Datagram::PermitFragNegotiation(EFrag Setting)
{
	if ( setsockopt(SD, SOL_IP, IP_MTU_DISCOVER, &Setting, sizeof(Setting)) != 0 )
		throw NetConfigException("Socket Option: set MTU_DISCOVER");
}

///////////////////////////////////////////////////////////////////////////////
/// Broadcast
//-----------------------------------------------------------------------------
//--- Constructors & Destructors
Broadcast::Broadcast(HostAddress& Me): Datagram(Me)
{
	const int on=1;
	if ( setsockopt(SD, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) != 0 )
		throw NetConfigException("Socket Option: set BROADCAST");
}

///////////////////////////////////////////////////////////////////////////////
/// MessageGroup
//-----------------------------------------------------------------------------
//--- Constructors & Destructors
MessageGroup::~MessageGroup(void)
{
	HostAddress NoAddr(0, GetType());
	for ( int i = 0; i < MAXGROUPS; i++ )
		if ( Addr[i] != NoAddr )
			Drop(Addr[i]);
}

//-----------------------------------------------------------------------------
//--- Specific Implementation
void MessageGroup::Connect(HostAddress& Address)
{
	if ( connect(SD, Address.GetAddress(), Address.GetSize()) != 0 )
		throw NetConnectException("Connect");
}

int MessageGroup::FindSlot(HostAddress& Host)
{	int slot=-1;

	for ( int i = 0; i < MAXGROUPS  &&  slot < 0; i++ )
		if ( Addr[i] == Host )
			slot = i;
	return slot;
}

void MessageGroup::Join(HostAddress& Address, int IFIndex)
{	int slot=-1;
	HostAddress NoAddr(0, Address.GetNetwork());

	slot = FindSlot(NoAddr);
	if ( slot < 0 )
		throw RangeException("Multicast table full");
	if ( GetType() == eIPv4 )
	{	struct ip_mreqn mreq;

		bzero(&mreq, sizeof(mreq));
		void* src = &(reinterpret_cast<struct sockaddr_in *>(Address.GetAddress())->sin_addr);
		memcpy(&mreq.imr_multiaddr, src, sizeof(mreq.imr_multiaddr));
		mreq.imr_ifindex = IFIndex;
		if ( setsockopt(SD, SOL_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) != 0 )
			throw NetConfigException("Socket Option: Join Multicast");
	}
	else if ( GetType() == eIPv6 )
	{	struct ipv6_mreq mreq;

		bzero(&mreq, sizeof(mreq));
		void* src = &(reinterpret_cast<struct sockaddr_in *>(Address.GetAddress())->sin_addr);
		memcpy(&mreq.ipv6mr_multiaddr, src, sizeof(mreq.ipv6mr_multiaddr));
		mreq.ipv6mr_interface = IFIndex;
		if ( setsockopt(SD, SOL_IPV6, IPV6_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) != 0 )
			throw NetConfigException("Socket Option: Join Multicast");
	}
	Addr[slot] = Address;
}

void MessageGroup::Drop(HostAddress& Address)
{	int slot=-1;

	for ( int i = 0; i < MAXGROUPS  &&  slot < 0; i++ )
		if ( Addr[i] == Address )
			slot = i;
	if ( slot == -1 )
		throw RangeException("Group not joined");
	if ( GetType() == eIPv4 )
	{	struct ip_mreq mreq;
		struct sockaddr_in *inet = reinterpret_cast<struct sockaddr_in *>(Addr[slot].GetAddress());

		mreq.imr_multiaddr = inet->sin_addr;
		mreq.imr_interface.s_addr = INADDR_ANY;
		if ( setsockopt(SD, SOL_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq)) != 0 )
			throw NetConfigException("Socket Option: Drop Multicast");
	}
	else if ( GetType() == eIPv6 )
	{	struct ipv6_mreq mreq;
		struct sockaddr_in6 *inet6 = reinterpret_cast<struct sockaddr_in6 *>(Addr[slot].GetAddress());
		mreq.ipv6mr_multiaddr = inet6->sin6_addr;
		mreq.ipv6mr_interface = INADDR_ANY;
		if ( setsockopt(SD, SOL_IPV6, IPV6_DROP_MEMBERSHIP, &mreq, sizeof(mreq)) != 0 )
			throw NetConfigException("Socket Option: Drop Multicast");
	}
	Addr[slot] = HostAddress();
}

//-----------------------------------------------------------------------------
//--- Class configuration (CRUD)

void MessageGroup::Loopback(bool Setting)
{
	int val = (Setting != 0);
	if ( GetType() == eIPv4 )
	{
		if ( setsockopt(SD, SOL_IP, IP_MULTICAST_LOOP, &val, sizeof(val)) != 0 )
			throw NetConfigException("IP Option: set MULTICAST_TTL");
	}
	else if ( GetType() == eIPv6 )
	{
		if ( setsockopt(SD, SOL_IPV6, IPV6_MULTICAST_LOOP, &val, sizeof(val)) != 0 )
			throw NetConfigException("IP Option: set MULTICAST_TTL");
	}
}

int  MessageGroup::GetTTL(void)
{
	int val;
	socklen_t size=sizeof(val);
	if ( GetType() == eIPv4 )
	{
		if ( getsockopt(SD, SOL_IP, IP_MULTICAST_TTL, &val, &size) != 0 )
			throw NetConfigException("IP Option: get MULTICAST_TTL");
	}
	else if ( GetType() == eIPv6 )
	{
		if ( getsockopt(SD, SOL_IPV6, IPV6_MULTICAST_HOPS, &val, &size) != 0 )
			throw NetConfigException("IP Option: get MULTICAST_TTL");
	}
	return val;
}

void MessageGroup::SetTTL(int Hops)
{
	if ( GetType() == eIPv4 )
	{
		if ( setsockopt(SD, SOL_IP, IP_MULTICAST_TTL, &Hops, sizeof(Hops)) != 0 )
			throw NetConfigException("IP Option: set MULTICAST_TTL");
	}
	else if ( GetType() == eIPv6 )
	{
		if ( setsockopt(SD, SOL_IPV6, IPV6_MULTICAST_HOPS, &Hops, sizeof(Hops)) != 0 )
			throw NetConfigException("IP Option: set MULTICAST_TTL");
	}
}


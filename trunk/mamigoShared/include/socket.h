#pragma once
/*****************************************************************************/
/*** socket.h                                                              ***/
/***                                                                       ***/
/*** Defines the C++ socket framework.                                     ***/
/*****************************************************************************/

#include <sys/socket.h>
#include <resolv.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <errno.h>
#include <string>
#include <string.h>
#include "Message.h"

typedef enum { eIPv4=PF_INET, eIPv6=PF_INET6 } ENetwork;
typedef enum { eStream=SOCK_STREAM, eDatagram=SOCK_DGRAM } EProtocol;

///////////////////////////////////////////////////////////////////////////////
/// SimpleString
//
//----------------------------------------------------------------------------
//---This class is separated and different from TextMessage, because exception
//---management must have the minimal number points of failure.  It would be
//---*very*bad* to have an exception within an exception.
//----------------------------------------------------------------------------
class SimpleString
{
	private:
		char *Buffer;
		int  Length;

	public:
		SimpleString(const char* s);
		SimpleString(const SimpleString& s);
		virtual ~SimpleString(void);

	protected:
		void Append(const char* s);
		void Append(const SimpleString& s);

	public:
		SimpleString& operator +(const char* s)		{ Append(s); return *this; }
		SimpleString& operator +(SimpleString s)	{ Append(s); return *this; }

	public:
		const char* GetString(void) const			{ return Buffer; }
		int GetLength(void) const					{ return Length; }
};

///////////////////////////////////////////////////////////////////////////////
/// Exception
class Exception
{
	private:
		int ErrNo;
		SimpleString Msg;

	public:
		Exception(const SimpleString M, int Err=0): Msg(M), ErrNo(Err) {}
		virtual ~Exception(void) {}

	public:
		int GetError(void) const						{ return ErrNo; }
		virtual void PrintException(void) const;
};

///////////////////////////////////////////////////////////////////////////////
/// RangeException
class RangeException: public Exception
{
	public:
		RangeException(const SimpleString M): Exception(M, ERANGE) {}
		virtual ~RangeException(void){}
};

///////////////////////////////////////////////////////////////////////////////
/// FileException
class FileException: public Exception
{
	public:
		FileException(const SimpleString M): Exception(M, errno) {}
		virtual ~FileException(void){}
};

///////////////////////////////////////////////////////////////////////////////
/// NetException
class NetException: public Exception
{
	public:
		NetException(const SimpleString M): Exception(M, errno) {}
		virtual ~NetException(void){}
};

///////////////////////////////////////////////////////////////////////////////
/// NetConversionException
class NetConversionException: public NetException
{
	public:
		NetConversionException(const SimpleString M): NetException(M) {}
		virtual ~NetConversionException(void){}
};

///////////////////////////////////////////////////////////////////////////////
/// NetDNSException
class NetDNSException: public NetException
{
	public:
		NetDNSException(const SimpleString M): NetException(M) {}
		virtual ~NetDNSException(void){}
};

///////////////////////////////////////////////////////////////////////////////
/// NetIOException
class NetIOException: public NetException
{
	public:
		NetIOException(const SimpleString M): NetException(M) {}
		virtual ~NetIOException(void){}
};

///////////////////////////////////////////////////////////////////////////////
/// NetConnectException
class NetConnectException: public NetException
{
	public:
		NetConnectException(const SimpleString M): NetException(M) {}
		virtual ~NetConnectException(void){}
};

///////////////////////////////////////////////////////////////////////////////
/// NetConfigException
class NetConfigException: public NetException
{
	public:
		NetConfigException(const SimpleString M): NetException(M) {}
		virtual ~NetConfigException(void){}
};

///////////////////////////////////////////////////////////////////////////////
/// HostAddress
class HostAddress
{
	private:
		static const int MaxHostName=50;
		ENetwork Network;
		char HostName[MaxHostName], Address[MaxHostName];
		struct sockaddr *Addr;

	public:
		static HostAddress* MakeHost(const char* Name=0, ENetwork Network=eIPv4, bool resolve=true);
		HostAddress(const char* Name=0, ENetwork Network=eIPv4, bool resolve=true);
		HostAddress(const HostAddress& Address);
		virtual ~HostAddress(void);
        bool Resolve();
        bool Resolve(const char* Name, ENetwork Network=eIPv4);
        const char * GetHostName() { return HostName; }
        const char * GetOrigHostName() { return Address; }
	public:
		void SetPort(int Port);
		int GetPort(void) const;
		ENetwork GetNetwork(void) const			{ return Network; }
		struct sockaddr* GetAddress(void) const	{ return Addr; }
		int GetSize(void) const;
		int operator ==(HostAddress& Address) const;
		int operator !=(HostAddress& Address) const	{ return !(operator ==(Address)); }
		const char* GetHost(bool byName=1);
};
///////////////////////////////////////////////////////////////////////////////
/// Socket

#define SOCKET_RECV_BUFF_SIZE 65536
class SecureTrait;
class Socket
{
	protected:
		int SD;
		char *rcvBuff;	// a single buffer to be used every time
		int rcvBuffSize;
		bool m_shouldClose;
		ENetwork m_network;
		EProtocol m_protocol;
		struct timeval m_timeval;
		bool m_isTimoutSet;
		void makeBuff(int size=SOCKET_RECV_BUFF_SIZE);
	public:
		Socket(void):m_shouldClose(true),mySecureTrait(0),m_isTimoutSet(false) { SD = -1; makeBuff();}
		Socket(int sd): SD(sd),m_shouldClose(true),mySecureTrait(0),m_isTimoutSet(false) {makeBuff();}
		Socket(ENetwork Network, EProtocol Protocol);
		Socket(const Socket& sock);
		virtual void SecureIt(); // add a secure trait to this socket
		virtual void SecureIt(SecureTrait* secureTrait); // add a secure trait to this socket
	public:
		virtual ~Socket(void);

	public:
		int GetSD(void){ return SD;}
		SecureTrait* GetST(void){ return mySecureTrait;}
		void Bind(HostAddress& Addr, bool bReuse=true);

		/**
		 * AK: Deprecated: should not be used
		 */
		int  Send(Message& Msg, int Options=0) const;
		void SendChunk(Message& Msg, int Options=0) const;
		int  Send(HostAddress& Addr, Message& Msg, int Options=0) const;
		void SendAll(HostAddress& Addr, Message& Msg, int Options=0) const;
		void SendAll(Message& Msg, int Options=0) const;
		int  Receive(Message& Msg, int Options=0) const;
		int  Receive(HostAddress& Addr, Message& Msg, int Options=0) const;
		int  ReceiveOn(Message& Msg, int Options=0) const;
		bool Read_nonblocking(Message& Msg, int count) const;
		int  Read_nonblocking(Message& Msg) const;
		void CloseInput(void) const						{ shutdown(SD, 0); }
		void CloseOutput(void) const					{ shutdown(SD, 1); }
		void SetshouldClose(bool bClose) { m_shouldClose=bClose;}
	public:
		void PermitRoute(bool Setting);
		void KeepAlive(bool Setting);
		void ShareAddress(bool Setting);
		int  GetReceiveSize(void);
		void SetReceiveSize(int Bytes);
		int  GetSendSize(void);
		void SetSendSize(int Bytes);
		int  GetMinReceive(void);
		void SetMinReceive(int Bytes);		//---Not yet implemented in Linux
		int  GetMinSend(void);
		void SetMinSend(int Bytes);			//---Not yet implemented in Linux
		struct timeval GetReceiveTimeout(void);
		void SetReceiveTimeout(struct timeval& val); //---Not yet implemented in Linux
		struct timeval GetSendTimeout(void);
		void SetSendTimeout(struct timeval& val); //---Not yet implemented in Linux
		void SetTimeouts(struct timeval& val); // same timeout for both send and rcv
		void SetTimeout(){if(m_isTimoutSet) SetTimeouts(m_timeval);}
		ENetwork GetType(void);
		virtual int  GetTTL(void);
		virtual void SetTTL(int Hops);
		int  GetError(void);
		void IgnoreSigPipe();
		int GetSockName(struct sockaddr *address, socklen_t *address_len);
        int GetPeerName(struct sockaddr *address, socklen_t *address_len);
	protected:

		virtual int mySend(char *buf, int bytes, int Options) const;
		virtual int myRecv(char *buf, int bytes, int Options) const;
		virtual int mySendTo(char *buf, int bytes, int Options, struct sockaddr *addr, socklen_t addrSize) const;
		virtual int myRecvFrom(char *buf, int bytes, int Options, struct sockaddr *addr, socklen_t* addrSize) const;
		SecureTrait *mySecureTrait;
};

///////////////////////////////////////////////////////////////////////////////
/// SocketStream
class SocketStream: public Socket
{
	private:
		void (*Urg_FN)(char *Msg);

	public:
		SocketStream(void);
		SocketStream(int sd): Urg_FN(0), Socket(sd){}
		SocketStream(ENetwork Network): Socket(Network, eStream){}
		SocketStream(const SocketStream& sock): Urg_FN(sock.Urg_FN), Socket(sock) {}
		virtual ~SocketStream(void){}

	public:
		int  GetMaxSegmentSize(void);
		void SetMaxSegmentSize(short Bytes);
		void DontDelay(bool Setting);
};

///////////////////////////////////////////////////////////////////////////////
/// SocketServer
class SocketServer: public SocketStream
{
	public:
		typedef enum { eNone, eCallback, eProcess, eThread } ETask;

	private:
		ETask type;
		void (*task_fn)(Socket& socket, HostAddress& addr);

	public:
		SocketServer(int port, ENetwork Network=eIPv4, int QLen=15);
		SocketServer(HostAddress& Addr, int QLen=15);
		SocketServer(const SocketServer& sock):type(sock.type),task_fn(sock.task_fn),SocketStream(sock){}
		virtual ~SocketServer(void);
		virtual void SecureIt(); // add a secure trait to this socket
		virtual void AddCipher(std::string cipher);
	private:
		static void SigChild(int sig)	{ wait(0); }

	public:
		void Accept(void (*Server)(Socket& Client));
		void Accept(void (*Server)(Socket& Client, void *arg), void *arg);
		void Accept(HostAddress& Addr, void (*Server)(Socket& Client));
		void RegTask(void (*FN)(Socket& socket, HostAddress& addr));
	protected:
		virtual void OnClientAccept(int clientSD, void (*Server)(Socket& Client));
		virtual void OnClientAccept(int clientSD, void (*Server)(Socket& Client, void *arg), void *arg);
};

///////////////////////////////////////////////////////////////////////////////
/// SocketClient
class SocketClient: public SocketStream
{
	public:
		SocketClient(ENetwork Network=eIPv4): SocketStream(Network){}
		SocketClient(HostAddress& Host, ENetwork Network=eIPv4);
		SocketClient(const SocketClient& sock):SocketStream(sock){}
		virtual ~SocketClient(void){}
		virtual void SecureIt();
		void Connect(HostAddress& Addr);
		void ConnectByHostname(HostAddress& Addr);
};

///////////////////////////////////////////////////////////////////////////////
/// Datagram
class Datagram: public Socket
{
	public:
		typedef enum { eJustSend, eHints, eNegotiate } EFrag;

	public:
		Datagram(HostAddress& Me, ENetwork Network=eIPv4, EProtocol Protocol=eDatagram);
		Datagram(ENetwork Network=eIPv4, EProtocol Protocol=eDatagram);
		virtual ~Datagram(void){}

	private:
		void SetTOS(bool Setting, int Val);

	public:
		void MinimizeDelay(bool Setting);
		void MaximizeThroughput(bool Setting);
		void MaximizeReliability(bool Setting);
		void MinimizeCost(bool Setting);
		void PermitFragNegotiation(EFrag Setting);
};

///////////////////////////////////////////////////////////////////////////////
/// Broadcast
class Broadcast: public Datagram
{
	private:
		HostAddress Address;

	public:
		Broadcast(HostAddress& Me);
		virtual ~Broadcast(void){}
};

///////////////////////////////////////////////////////////////////////////////
/// MessageGroup
class MessageGroup: public Datagram
{
	private:
		static const int MAXGROUPS=10;
		HostAddress Addr[MAXGROUPS];

	public:
		MessageGroup(HostAddress& Me, ENetwork Network=eIPv4): Datagram(Me, Network) {}
		virtual ~MessageGroup(void);

	private:
		int  FindSlot(HostAddress& Host);

	public:
		void Connect(HostAddress& Address);
		void Join(HostAddress& Address, int IFIndex=0);
		void Drop(HostAddress& Address);

	public:
		void Loopback(bool Setting);
		int  GetTTL(void);
		void SetTTL(int Hops);
};


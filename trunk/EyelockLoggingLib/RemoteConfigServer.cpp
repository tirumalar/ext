/*
 * RemoteConfigServer.cpp
 *
 *  Created on: 18 Aug, 2009
 *      Author: akhil
 */

#include "RemoteConfigServer.h"
#include "FileConfiguration.h"
#include "Configuration.h"
#include "socket.h"
#include <sys/time.h>
#include <time.h>
#include <iostream>
#include <sstream>
#include <string>
#include "MessageExt.h"
#include "Synchronization.h"
#include "SocketFactory.h"
#include "logging.h"
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/exception.h>

using namespace std;
extern "C" {
#include "file_manip.h"
}
#include "NetworkUtilities.h"

const char logger[30] = "RemoteConfigServer";

uint64_t  RemoteConfigServer::GetFuteristicTime(int futSec)
{
    struct timeval curTime;
    gettimeofday(&curTime, 0);
    uint64_t temp = (curTime.tv_sec + futSec);
    temp = temp * 1000000u;
    temp += curTime.tv_usec;
    temp = temp/1000;
    return temp;
}

RemoteConfigServer::RemoteConfigServer(Configuration& conf) :
m_debug(0)
,m_logging(false)
,m_socketFactory(0)
{
	// The port we listen on for incoming config requests...
	m_port = conf.getValue("RemoteConfigServer.port", 2221);

	m_debug = conf.getValue("RemoteConfigServer.Debug",false);

	int MsgSize = LogConfigMsg::calcMessageSize();
	m_LogConfigMsg = new LogConfigMsg(MsgSize);

  	m_pSockSrv = 0;
  	m_logging = conf.getValue("Eyelock.Logging", false);

  	m_socketFactory = new SocketFactory(conf);
}

RemoteConfigServer::~RemoteConfigServer()
{
	if (m_LogConfigMsg)
		delete m_LogConfigMsg;
	if (m_socketFactory)
		delete m_socketFactory;
}

bool RemoteConfigServer::IsHealthy(uint64_t curTime)
{
	return true;
} //commitcomment



void RemoteConfigServer::HandleMessage(LogConfigMsg& msg) {

	LOGCONFIGMESSAGETYPE msgType=msg.getMsgType();
	switch(msgType)
	{
		case REMOTEIP_MSG:
		{
			// HandleMsg
			const std::string WHITESPACE = " \t";
			char IPbuffer[32];

			memset(IPbuffer, '\0', 32);

			if (msg.getRemoteIPData(IPbuffer))
			{
				// Write the config file/update internal properties...
				// Run through the config file and update all RemoteHost entries to new IP

				// Create a new cfg file with proper settings... then force reload of log4cxx...
				// make the backup
				remove("nxtlog.remote"); // Delete remote logging config if it's there...

				// open nxtlog.cfg as input
				// open create nxtlog.remote as output
				std::ifstream input( "nxtlog.cfg");
				std::ofstream output( "nxtlog.remote");

				if (output.good()) // If file created and opened OK
				{
					// write our .remote config file replacing the original settings with what the user wants...
					std::string line;
					while (std::getline(input, line))
					{
						if (line.length())
						{
							// ltrim
							size_t start = line.find_first_not_of(WHITESPACE);
							line = (start == std::string::npos) ? "" : line.substr(start);

							if (line.length() == 0)
							{
								output << line << "\n";
								continue;
							}

							// Skip comments...
							if (line[0] == '#')
								output << line << "\n";
							else
							{
								// do we have REMOTEHOST in this line?
								std::stringstream ss(line);

								std::string s;
								if (std::getline(ss, s, '='))
								{
									if (s.find("RemoteHost") != string::npos)
										output << s << "=" << IPbuffer  << "\n";
									else if (line.find("log4j.appender.nxtlog=") != string::npos)
										output << s << "=" << "org.apache.log4j.RollingFileAppender, org.apache.log4j.XMLSocketAppender"  << "\n";
									else if (line.find("log4j.appender.eventlog=") != string::npos)
										output << s << "=" << "org.apache.log4j.RollingFileAppender, org.apache.log4j.XMLSocketAppender"  << "\n";
									else if (line.find("log4j.appender.motorlog=") != string::npos)
										output << s << "=" << "org.apache.log4j.RollingFileAppender, org.apache.log4j.XMLSocketAppender"  << "\n";
									else if (line.find("log4j.appender.facetrackinglog=") != string::npos)
										output << s << "=" << "org.apache.log4j.RollingFileAppender, org.apache.log4j.XMLSocketAppender"  << "\n";
									else
										output << line << "\n";
								}
								else // just write the line..
									output << line << "\n";
							}
						}
						else
							output << "\n";
					}

					input.close();
					output.close();

					// Reload the logging subsystem
					log4cxx::PropertyConfigurator::configure("/home/root/nxtlog.remote");
				}
				else // Failed
				{
					input.close();
					output.close();
				}

				break;

#if 0
				// Create a backup of our current file...
				// remove any old file...
				// If .logbak exists, then it is the original cfg file...(user could change settings more than once before reverting...)
				// we need to rename it to .cfg before continuing...
				// this just fails if it isn't there...
				rename("nxtlog.logbak", "nxtlog.cfg"); // Just in case

				// make the backup
				std::ifstream  src("nxtlog.cfg", std::ios::binary);
				std::ofstream  dst("nxtlog.logbak",   std::ios::binary);
				dst << src.rdbuf();
				src.close();
				dst.close();

				// remove any old file...
				remove("nxtlog.update");

				// rename nxtlog.cfg to nxtlog.update
				rename("nxtlog.cfg", "nxtlog.update");

				// open nxtlog.cfg
				// open create nxtlog.update
				std::ifstream input( "nxtlog.update");
				std::ofstream output( "nxtlog.cfg");

				// read/write lines and process...
				std::string line;
				while (std::getline(input, line))
				{
				//	printf("TheLine = %s\n", line.c_str());
					if (line.length())
					{
						// ltrim
						size_t start = line.find_first_not_of(WHITESPACE);
						line = (start == std::string::npos) ? "" : line.substr(start);

						if (line.length() == 0)
						{
							output << line << "\n";
							continue;
						}

						// Skip comments...
						if (line[0] == '#')
						    output << line << "\n";
						else
						{
							// do we have REMOTEHOST in this line?
							std::stringstream ss(line);

							std::string s;
							if (std::getline(ss, s, '='))
							{
								if (s.find("RemoteHost") != string::npos)
									output << s << "=" << IPbuffer  << "\n";
								else if (line.find("log4j.appender.nxtlog=") != string::npos)
									output << s << "=" << "org.apache.log4j.RollingFileAppender, org.apache.log4j.XMLSocketAppender"  << "\n";
								else if (line.find("log4j.appender.eventlog=") != string::npos)
									output << s << "=" << "org.apache.log4j.RollingFileAppender, org.apache.log4j.XMLSocketAppender"  << "\n";
								else if (line.find("log4j.appender.motorlog=") != string::npos)
									output << s << "=" << "org.apache.log4j.RollingFileAppender, org.apache.log4j.XMLSocketAppender"  << "\n";
								else if (line.find("log4j.appender.facetrackinglog=") != string::npos)
									output << s << "=" << "org.apache.log4j.RollingFileAppender, org.apache.log4j.XMLSocketAppender"  << "\n";
								else
								    output << line << "\n";
							}
							else // just write the line..
							    output << line << "\n";
						}
					}
					else
					    output << "\n";
				}

				input.close();
				output.close();

				// Reload the logging subsystem
				log4cxx::PropertyConfigurator::configure("/home/root/nxtlog.cfg");

				// Now that we have reloaded... put the original nxtlog.cfg back...
				ifstream f("nxtlog.logbak");
				if (f.good())
				{
					f.close();
					remove("nxtlog.cfg");

					// replace the original file..
					rename("nxtlog.logbak", "nxtlog.cfg");
				}
				else // hmmm... shouldn't be the case... rename the .update file.. this is our last chance to fix it.
				{
					remove("nxtlog.cfg");
					rename("nxtlog.update", "nxtlog.cfg");
				}
#endif
			}
		}

		case IMGLOGLEVEL_MSG:
		{
			// HandleMsg
			char szRequestedLogLevel[64];

			memset(szRequestedLogLevel, '\0', 64);

			bool bSaveToDisk;

			if (msg.getImgLogLevelData(szRequestedLogLevel, bSaveToDisk))
			{
				log4cxx::LoggerPtr theLog = log4cxx::Logger::getLogger("imglog");
				theLog->setLevel(log4cxx::Level::toLevel(szRequestedLogLevel));

				theLog = log4cxx::Logger::getLogger("imglog.remoteimglog");
				theLog->setLevel(log4cxx::Level::toLevel(szRequestedLogLevel));

				theLog->setAdditivity(bSaveToDisk);
			}

			break;
		}


		case LOGGERLOGLEVEL_MSG:
		{
			// HandleMsg
			char szLogger[64];
			char szRequestedLogLevel[64];

			memset(szLogger, '\0', 64);
			memset(szRequestedLogLevel, '\0', 64);

			if (msg.getLoggerLogLevelData(szLogger, szRequestedLogLevel))
			{
				log4cxx::LoggerPtr theLog = log4cxx::Logger::getLogger(szLogger);
				theLog->setLevel(log4cxx::Level::toLevel(szRequestedLogLevel));
			}

			break;
		}


		case REBOOTDEVICE_MSG:
		{
			// HandleMsg
			int status = system("reboot");
			// Reboot...
			break;
		}


		case REVERTCONFIG_MSG:
		{
			// Reload the 'default' logging subsystem
			log4cxx::PropertyConfigurator::configure("/home/root/nxtlog.cfg");
			break;
		}


		default:
		{
			cerr<<"Unknown MessageType "<<endl;
			if(m_logging)
			{
				struct timeval m_timer;
				gettimeofday(&m_timer, 0);
				TV_AS_USEC(m_timer,a);
				FILE *fp = fopen("dump.txt","a");
				int le =0;
				fprintf(fp,"%.*s;%llu;\n",50,msg.GetBuffer(),a);
				fclose(fp);
			}

			break;
		}
	}
}


void RemoteConfigServer::onConnect(Socket& client, void *arg) {
	RemoteConfigServer *me = (RemoteConfigServer *) arg;
	if(me->do_serv_task(client))
	{
		client.CloseInput();
	}
	else
	{
		// since another thread is using this socket lets avoid closing it
		client.SetshouldClose(false);
	}
}


// called when there is a connection
bool RemoteConfigServer::do_serv_task(Socket& client) {
	try {
		if(ShouldIQuit()) return true;

		if(m_debug)
			EyelockLog(logger, DEBUG, "RemoteConfigServer::do_serv_task - msgType %d", m_LogConfigMsg->getMsgType()); fflush(stdout);

		client.Receive(*m_LogConfigMsg);

		HandleMessage(*m_LogConfigMsg);

		return true;

	} catch (NetIOException nex) {
		EyelockLog(logger, ERROR, "do_serv_task() exception: %d", nex.GetError());
		nex.PrintException();
	}
	catch (const char *msg) {
		EyelockLog(logger, ERROR, "do_serv_task() exception: %s", msg);
		cerr <<msg <<endl;
	}
	return true;
}

void RemoteConfigServer::DeleteSocketStream(SocketServer *&s)
{
	if(s != NULL)
	{
		s->CloseInput();
		s->CloseOutput();
		delete s;
		s = 0;
	}
}

void RemoteConfigServer::CloseServer()
{
	// NOOP
}

int RemoteConfigServer::End()
{
	EyelockLog(logger, DEBUG, "RemoteConfigServer::End()"); fflush(stdout);
	m_QuitStatus.lock(); m_QuitStatus.set(true); m_QuitStatus.unlock();
    {
#ifndef HBOX_PG
    	SafeLock<SocketServer *> lock(m_pSockSrv);
    	if(m_pSockSrv.get())
    	{
    		m_pSockSrv.get()->CloseInput();
    		m_pSockSrv.get()->CloseOutput();
    	}
#else
    	if(m_pSockSrv)
    	{
    		m_pSockSrv->CloseInput();
    	    m_pSockSrv->CloseOutput();
    	}
#endif
    }

   // if (phpThread)
    	//pthread_join (phpThread, NULL);

	EyelockLog(logger, DEBUG, "RemoteConfigServer::End() => HThread::End()"); fflush(stdout);
	HThread::End();
	EyelockLog(logger, DEBUG, "RemoteConfigServer::End() joined!!!"); fflush(stdout);
}


unsigned int RemoteConfigServer::MainLoop() {

	std::string name = "RemoteConfigServer::";

	bool reset = false;

	while (!ShouldIQuit()) {
		if(m_debug)
			EyelockLog(logger, DEBUG, "%s Looping again!", name.c_str()); fflush(stdout);
#ifndef HBOX_PG
		try{
			{
				SafeLock<SocketServer *> lock(m_pSockSrv);
				if(reset)
				{
					EyelockLog(logger, INFO, "%s SocketServer Delete!", name.c_str()); fflush(stdout);
					DeleteSocketStream(m_pSockSrv.get());
					reset = false;
				}
				if(!m_pSockSrv.get())
				{
					EyelockLog(logger, INFO, "%s SocketServer Create! Port = %d", name.c_str(), m_port); fflush(stdout);
					m_pSockSrv.get()= new SocketServer(m_socketFactory->createSocketServer("GRI.RemoteConfigServerSecure",m_port));
					//m_pSockSrv.get()= new SocketServer(m_socketFactory->createSocketServer("GRI.NwDispatcherSecure",m_port));
				}

			}

			m_pSockSrv.get()->ShareAddress(true);

			m_pSockSrv.get()->Accept(onConnect, this);

			if(m_debug)
				EyelockLog(logger, DEBUG, "%s: processed connection", name.c_str()); fflush(stdout);
		}
#else
		try{
			// printf("Inside try of RemoteConfigServer\n");
					{
						// SafeLock<SocketServer *> lock(m_pSockSrv);
						if(reset)
						{
							EyelockLog(logger, INFO, "%s SocketServer Delete!", name.c_str()); fflush(stdout);
							DeleteSocketStream(m_pSockSrv);
							reset = false;
						}
						if(!m_pSockSrv)
						{
							// printf("m_port...%d\n", m_port);
							// printf("Inside m_pSockSrv of RemoteConfigServer\n");
							EyelockLog(logger, INFO, "%s SocketServer Create!", name.c_str()); fflush(stdout);
							m_pSockSrv = m_socketFactory->createSocketServer2("GRI.RemoteConfigServerSecure", m_port, eIPv4);
							// perror("Error in Socket creation in RemoteConfigServer\n");
						}


					}
					m_pSockSrv->ShareAddress(true);
					m_pSockSrv->Accept(onConnect, this);

					if(m_debug)
						EyelockLog(logger, DEBUG, "%s: processed connection", name.c_str()); fflush(stdout);
				}

#endif
		catch(Exception& ncex){
			EyelockLog(logger, ERROR, "MainLoop() exception: %d", ncex.GetError());
			cout <<name; cout.flush();
			ncex.PrintException();
			reset = true;
			//sleep(1);
		}
		catch(const char* msg){
			EyelockLog(logger, ERROR, "MainLoop() exception: %s", msg);
			cout <<name<<msg <<endl;
			reset = true;
			//sleep(1);
		}
	}

	EyelockLog(logger, DEBUG, "RemoteConfigServer::MainLoop; CloserServer()"); fflush(stdout);

	CloseServer();

	return 0;
}


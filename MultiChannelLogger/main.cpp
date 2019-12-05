#include "Poco/AutoPtr.h"
#include "Poco/File.h"
#include "Poco/Exception.h"

#include "Poco/Util/Application.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Util/IntValidator.h"
#include "Poco/Util/RegExpValidator.h"
#include "Poco/Util/AbstractConfiguration.h"
#include "Poco/Util/LayeredConfiguration.h"
#include "Poco/Util/XMLConfiguration.h"

#include "Poco/Net/NetException.h"

#include "Poco/Net/Socket.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/Net/SecureStreamSocket.h"
#include "Poco/Net/NetSSL.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/SSLManager.h"
#include "Poco/Net/SSLException.h"
#include "Poco/Net/NetException.h"

#include "Poco/Logger.h"
#include "Poco/Channel.h"
#include "Poco/SplitterChannel.h"
#include "Poco/ConsoleChannel.h"
#include "Poco/SimpleFileChannel.h"

#include "Poco/FormattingChannel.h"
#include "Poco/PatternFormatter.h"

#include <iostream>
#include <string>
#include <vector>

#include "Validators.h"

using Poco::AutoPtr;
using Poco::File;

using Poco::Logger;
using Poco::Channel;
using Poco::SplitterChannel;
using Poco::ConsoleChannel;
using Poco::SimpleFileChannel;
using Poco::FormattingChannel;
using Poco::PatternFormatter;

using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;
using Poco::Util::RegExpValidator;
using Poco::Util::IntValidator;
using Poco::Util::AbstractConfiguration;
using Poco::Util::LayeredConfiguration;
using Poco::Util::XMLConfiguration;
using Poco::Util::OptionCallback;



using namespace std;

class TcpMessenger
{
public:
	TcpMessenger(const std::string& _hostName, Poco::UInt16 _port, bool _isSecure = false, Poco::Net::Context::Ptr _pContext = 0): endpoint(Poco::Net::SocketAddress(_hostName, _port)), isSecure(_isSecure), pContext(_pContext), pSocket(0)
	{
	};

	virtual ~TcpMessenger()
	{
		close();
	};

	void open()
	{
		if (!pSocket)
		{
			if (isSecure)
			{
				pSocket = new Poco::Net::SecureStreamSocket(endpoint, pContext);
				Poco::Net::SSLManager::instance().initializeClient(0, 0, pContext);
			}
			else
			{
				pSocket = new Poco::Net::StreamSocket(endpoint);
			}
			pSocket->setReceiveTimeout(Poco::Timespan((long) 5, (long) 0)); // 5 seconds
		}
	};

	void close()
	{
		if (pSocket)
		{
			delete pSocket;
			pSocket = 0;
		}
	};

	void send(const std::string& message)
	{
		pSocket->sendBytes(message.c_str(), message.length());
	};

private:
	TcpMessenger(const TcpMessenger&);
	TcpMessenger& operator = (const TcpMessenger&);

	Poco::Net::SocketAddress endpoint;
	bool isSecure;
	Poco::Net::Context::Ptr pContext;
	Poco::Net::StreamSocket* pSocket;

};

class TcpChannel : public Poco::Channel
{
public:
	TcpChannel(TcpMessenger* _pTcpMessenger): pTcpMessenger(_pTcpMessenger)
	{
	};

	void log(const Poco::Message &msg)
	{
		open();
		pTcpMessenger->send(msg.getText());
	}

	void open()
	{
		pTcpMessenger->open();
	};

	void close()
	{
		pTcpMessenger->close();
	};

protected:
	~TcpChannel() {};

private:
	TcpChannel(const TcpChannel&);
	TcpChannel& operator = (const TcpChannel&);
	TcpMessenger* pTcpMessenger;
};


class MultiChannelLogger: public Application
	/// Tool for logging to file, console and TCP receiver.
	///
	/// MultiChannelLogger --help (on Unix platforms) or MultiChannelLogger /help (elsewhere) for
	/// usage information.
{
public:
	MultiChannelLogger(): _helpRequested(false), _saveSettingsRequested (false), _msgVec(), _pUserConfig(new Poco::Util::XMLConfiguration)
	{
		File userConfig(USER_CONFIG);
		if (!userConfig.exists())
		{
			throw Poco::ApplicationException("Initialization error", "Configuration file " + USER_CONFIG + " not found");
		}
		_pUserConfig->load(USER_CONFIG);
		//config().add(_pUserConfig, -100);
		config().addWriteable(_pUserConfig, 100);
	}

protected:
	void initialize(Application& self)
	{
		//loadConfiguration(); // loading configuration from default location is disabled
		Application::initialize(self);
		Poco::Net::initializeSSL();
	}

	XMLConfiguration& userConfig()
	{
		return *_pUserConfig;
	}

	void saveUserConfig()
	{
		_pUserConfig->save(USER_CONFIG);
	}

	void uninitialize()
	{
		// uninitialization
		Application::uninitialize();
		Poco::Net::uninitializeSSL();
	}

	void reinitialize(Application& self)
	{
		Application::reinitialize(self);
		// reinitialization
	}

	void defineOptions(OptionSet& options)
	{
		Application::defineOptions(options);

		options.addOption(
			Option("help", "h", "display usage information")
				.required(false)
				.repeatable(false)
				.callback(OptionCallback<MultiChannelLogger>(this, &MultiChannelLogger::handleHelp)));

		options.addOption(
			Option("logfile-enable", "l", "enable logging to local file (\"true\\false\\TRUE\\FALSE\\1\\0\")")
				.required(false)
				.repeatable(false)
				.validator(new BoolValidator())
				.argument("\"BOOL\"", true)
				.binding("enablelog", _pUserConfig));

		options.addOption(
			Option("logfile", "f", "set the log file")
				.required(false)
				.repeatable(false)
				//.validator(new PathValidator()) // TODO: create suitable validator
				.argument("\"/path/to/log/file\"", true)
				.binding("logfile", _pUserConfig));

		options.addOption(
			Option("tcp-enable", "t", "enable sending TCP messages (\"true\\false\\TRUE\\FALSE\\1\\0\")")
				.required(false)
				.repeatable(false)
				.validator(new BoolValidator())
				.argument("\"BOOL\"", true)
				.binding("enabletcp", _pUserConfig));

		options.addOption(
			Option("dest-ip", "d", "set the TCP messages destination IP")
				.required(false)
				.repeatable(false)
				.validator(new RegExpValidator("^(?:(?:25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])\\.){3}(?:25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])$"))
				.argument("\"IP\"", true)
				.binding("msgdestip", _pUserConfig));

		options.addOption(
			Option("dest-port", "p", "set the TCP messages destination port")
				.required(false)
				.repeatable(false)
				.validator(new IntValidator(1, 65535))
				.argument("\"PORT\"", true)
				.binding("msgdestport", _pUserConfig));

		options.addOption(
			Option("secure", "s", "set the secure communication (\"true\\false\\TRUE\\FALSE\\1\\0\")")
				.required(false)
				.repeatable(false)
				.validator(new BoolValidator())
				.argument("\"BOOL\"", true)
				.binding("enablesecure", _pUserConfig));

		options.addOption(
			Option("cert", "c", "set the certificate")
				.required(false)
				.repeatable(false)
				.validator(new FileReadableValidator())
				.argument("\"/path/to/cert/file\"", true)
				.binding("cert", _pUserConfig));

		options.addOption(
			Option("key", "k", "set the private key")
				.required(false)
				.repeatable(false)
				.validator(new FileReadableValidator())
				.argument("\"/path/to/key/file\"", true)
				.binding("key", _pUserConfig));

		options.addOption(
			Option("rootca", "r", "set the trusted certificates")
				.required(false)
				.repeatable(false)
				.validator(new FileReadableValidator())
				.argument("\"/path/to/rootCA/file\"", true)
				.binding("rootca", _pUserConfig));

		// TODO: separate options for message priority (info, error etc.)
		options.addOption(
			Option("log-message", "L", "log the specified message")
				.required(false)
				.repeatable(true)
				.validator(new MessageLengthValidator())
				.argument("message", true)
				.callback(OptionCallback<MultiChannelLogger>(this, &MultiChannelLogger::handleLogMessage)));

		options.addOption(
			Option("save-config", "S", "save current settings to configuration file")
				.required(false)
				.repeatable(false)
				.callback(OptionCallback<MultiChannelLogger>(this, &MultiChannelLogger::handleSaveSettings)));
	}

	void handleHelp(const std::string& name, const std::string& value)
	{
		_helpRequested = true;
		displayHelp();
		stopOptionsProcessing();
	}

	void handleConfig(const std::string& name, const std::string& value)
	{
		loadConfiguration(value);
	}

	void handleSaveSettings(const std::string& name, const std::string& value)
	{
		_saveSettingsRequested = true;
	}

	void handleLogMessage(const std::string& name, const std::string& value)
	{
		_msgVec.push_back(value);
	}

	void displayHelp()
	{
		HelpFormatter helpFormatter(options());
		helpFormatter.setCommand(commandName());
		helpFormatter.setUsage("OPTIONS");
		helpFormatter.setHeader("Tool for logging to file, console and TCP receiver.");
		helpFormatter.format(std::cout);
	}

	void printProperties(const std::string& base)
	{
		AbstractConfiguration::Keys keys;
		config().keys(base, keys);
		if (keys.empty())
		{
			if (config().hasProperty(base))
			{
				std::string msg;
				msg.append(base);
				msg.append(" = ");
				msg.append(config().getString(base));
				logger().information(msg);
			}
		}
		else
		{
			for (AbstractConfiguration::Keys::const_iterator it = keys.begin(); it != keys.end(); ++it)
			{
				std::string fullKey = base;
				if (!fullKey.empty()) fullKey += '.';
				fullKey.append(*it);
				printProperties(fullKey);
			}
		}
	}

	Poco::Net::Context::Ptr createContext()
	{
		Poco::Net::Context::Ptr pContext;

		std::string rootcert;
		bool loadDefaultCA = true;
		// TODO:: server certificate validation

		pContext = new Poco::Net::Context(Poco::Net::Context::CLIENT_USE, rootcert, Poco::Net::Context::VERIFY_NONE, 9, loadDefaultCA);
		pContext->enableExtendedCertificateVerification(false); // disabling Poco's extended certificate verification

		Poco::Crypto::X509Certificate x509certificate(config().getString("cert", "./rootCert/certs/nanoNXTDefault.crt"));
		Poco::Crypto::RSAKey rsakey("", config().getString("key", "./rootCert/certs/nanoNXTDefault.key"));

		pContext->useCertificate(x509certificate);
		pContext->usePrivateKey(rsakey);

		return pContext;
	}

	string getDeviceTypeAndId()
	{
		char myID[10];

	    strcpy(myID, "0000");
	    FILE *fp = fopen("/home/root/id.txt", "r");
	    if (fp != NULL)
	    {
	 	   if (fgets(myID, 10, fp) != NULL)
	 	   {
	 		   myID[strlen(myID)-1] = '\0';
	 	   }
	 	   fclose(fp);
	    }
	    else
	    {
	 	   printf("Unable to open device ID file\n");
	 	   return "";
	    }

	    string deviceTypeStr = "DEVICETYPE:2;";
	    string deviceIdStr = "DEVICEID:" + string(myID) + ";";
	    return deviceTypeStr + deviceIdStr;
	}

	void logMessages(const string& logFile, TcpMessenger* pTcpMessenger = NULL)
	{
		AutoPtr<SplitterChannel> pSplitter(new SplitterChannel);

		AutoPtr<ConsoleChannel> pCons(new ConsoleChannel);
		pSplitter->addChannel(pCons);

		if (!logFile.empty())
		{
			AutoPtr<SimpleFileChannel> pFile(new SimpleFileChannel(logFile));
			pSplitter->addChannel(pFile);
		}

		if (pTcpMessenger != NULL)
		{
			AutoPtr<TcpChannel> pTcp(new TcpChannel(pTcpMessenger));
			pSplitter->addChannel(pTcp);
		}

		AutoPtr<PatternFormatter> pPF(new PatternFormatter);

		string deviceTypeAndId = getDeviceTypeAndId();

		if (deviceTypeAndId.empty())
		{
			pPF->setProperty("pattern", "FIRMWARE;HOST:%N;%Y-%m-%d;%H:%M:%S%s;%t");
		}
		else
		{
			pPF->setProperty("pattern", "FIRMWARE;" + deviceTypeAndId + "%Y-%m-%d;%H:%M:%S%s;%t");
		}

		AutoPtr<FormattingChannel> pFC(new FormattingChannel(pPF, pSplitter));
		Logger::root().setChannel(pFC);

		vector<string>::iterator it;
		for (it = _msgVec.begin(); it != _msgVec.end(); it++)
		{
			string msgText(*it);
			Logger::root().information(msgText);
		}
	}

	int main(const std::vector<std::string>& args)
	{
		if (_saveSettingsRequested)
		{
			saveUserConfig();
		}

		if (!_helpRequested)
		{
			try
			{
				bool enableLogFile = config().getBool("enablelog", false);
				string logFile;
				if (enableLogFile)
				{
					logFile = config().getString("logfile", "");
				}

				bool enableTcp = config().getBool("enabletcp", false);

				if (enableTcp)
				{
					string messageDestinationIp = config().getString("msgdestip", "");
					Poco::UInt16 messageDestinationPort = config().getUInt("msgdestport", 0);
					bool enableSecure = config().getBool("enablesecure", false);
					Poco::Net::Context::Ptr pContext = 0;
					if (enableSecure)
					{
						pContext = createContext(); // AutoPtr, then no explicit deallocation
					}
					TcpMessenger tcpMessenger(messageDestinationIp, messageDestinationPort, enableSecure, pContext);
					logMessages(logFile, &tcpMessenger);
				}
				else
				{
					logMessages(logFile);
				}
			}
			catch (Poco::Net::NetException& exc)
			{
				cerr << exc.displayText() << endl;
				return Application::EXIT_PROTOCOL;
			}
			catch (Poco::IOException& exc)
			{
				cerr << exc.displayText() << endl;
				return Application::EXIT_IOERR;
			}
			catch (Poco::Exception& exc)
			{
				cerr << exc.displayText() << endl;
				return Application::EXIT_SOFTWARE;
			}
			catch (...)
			{
				cerr << "Unknown error" << endl;
				return Application::EXIT_SOFTWARE;
			}

			//logger().information("Application properties:");
			//printProperties("");
		}

		return Application::EXIT_OK;
	}

private:
	bool _helpRequested;
	bool _saveSettingsRequested;
	std::vector<std::string> _msgVec;

	static const std::string USER_CONFIG;
	Poco::AutoPtr<Poco::Util::XMLConfiguration> _pUserConfig;
};

const std::string MultiChannelLogger::USER_CONFIG("MultiChannelLoggerSettings.xml"); // default settings file is "MultiChannelLogger.xml"

POCO_APP_MAIN(MultiChannelLogger)

/*
 * EyelockBinaryAppender.h
 *
 *  Created on: Nov 20, 2019
 *      Author: eyelock
 */

//#ifndef INCLUDE_EYELOCKBINARYAPPENDER_H_
//#define INCLUDE_EYELOCKBINARYAPPENDER_H_

#include <log4cxx/appenderskeleton.h>
#include <log4cxx/spi/loggingevent.h>
#include <fstream>
#include <sstream>
#include <ostream>
#include <streambuf>
#include <string>


using namespace log4cxx::helpers;
using namespace log4cxx::spi;
using namespace std;
using namespace log4cxx;

namespace log4cxx
{
	/**
		An appender that appends logging events to a binary file...
	*/
	class LOG4CXX_EXPORT EyelockBinaryAppender : public AppenderSkeleton
	{
	public:
		DECLARE_LOG4CXX_OBJECT(EyelockBinaryAppender)
		BEGIN_LOG4CXX_CAST_MAP()
				LOG4CXX_CAST_ENTRY(EyelockBinaryAppender)
				LOG4CXX_CAST_ENTRY_CHAIN(AppenderSkeleton)
		END_LOG4CXX_CAST_MAP()


		EyelockBinaryAppender() : m_bImmediateFlush(true), m_bAppend(true), m_MaxFileSize(1024*1024*1024), m_MaxBackupIndex(2) { };
		virtual ~EyelockBinaryAppender() {};

	private:
			bool m_bImmediateFlush;
			ofstream m_os;

			//fields
			string m_sFilename;
			bool m_bAppend;
			long m_MaxFileSize;
			int	m_MaxBackupIndex;

		    //getter & setter methods
		    string getFile() { return m_sFilename; }
		    void setFile(string file) { m_sFilename = file; }
		    bool getAppend() { return m_bAppend; }
		    void setAppend(bool append) {m_bAppend = append; }
		    LogString stripDuplicateBackslashes(const LogString& src);


//			std::vector<spi::LoggingEventPtr> vector;

			/**
			This method is called by the AppenderSkeleton#doAppend
			method.
			*/
	public:
		    static void ForceLinking() { return; }
		    // Overrides
			virtual void append(const LoggingEventPtr& event, Pool& p);
		    virtual void activateOptions(Pool& p);
		    virtual void close();
		    virtual bool requiresLayout() const {return false; }
		    virtual void setOption(const LogString& option, const LogString& value);

	private:
		    // Members
		    void reset();
		    void closeFile();
		    void setFile(string sFilename, bool append);
		    bool checkEntryConditions();
			bool isClosed() const { return closed; }
			bool rollover(Pool& p);
			long GetFileSize(std::string filename);


			//const std::vector<spi::LoggingEventPtr>& getVector() const
			//		{ return vector; }
	};

	LOG4CXX_PTR_DEF(EyelockBinaryAppender);
}

//#endif /* INCLUDE_EYELOCKBINARYAPPENDER_H_ */

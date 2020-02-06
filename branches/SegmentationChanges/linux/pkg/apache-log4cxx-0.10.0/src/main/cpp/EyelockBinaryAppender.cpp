// EyelockBinaryAppender.cpp

#include <stdio.h>
#include <log4cxx/appenderskeleton.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/logstring.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/synchronized.h>
#include <log4cxx/helpers/optionconverter.h>



#include <log4cxx/eyelock/EyelockBinaryAppender.h>


using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::spi;

using namespace std;

	// Register this class with log4cxx
	IMPLEMENT_LOG4CXX_OBJECT(EyelockBinaryAppender)

	// Overrides
    void EyelockBinaryAppender::append(const LoggingEventPtr& event, Pool& p)
    {
        if (!checkEntryConditions())
        	return;

        try
        {
            string theData = event->getMessage();

            if (theData.length() == 0)
                return;

            m_os.write(theData.data(), theData.size());

            if (m_bImmediateFlush)
            {
                m_os.flush();
            }
        }
        catch (IOException ex)
        {
            LogLog::error((LogString) LOG4CXX_STR("[EyelockBinaryAppender:  ]Not allowed to write to a closed appender."));
        }
    }


    void EyelockBinaryAppender::activateOptions(Pool& pool)
    {
		if (m_sFilename.length())
		{
		  try
		  {
			setFile(m_sFilename, m_bAppend);
		  }
		  catch(IOException e)
		  {
		//	errorHandler.error("setFile("+file+","+append+") call failed.",
		//		   e, ErrorCode.FILE_OPEN_FAILURE);
		  }
		}
		else
		{
		  LogLog::error((LogString) LOG4CXX_STR("File option not set for appender [") +name+ + LOG4CXX_STR("] ."));
		}
	}


    void EyelockBinaryAppender::setOption(const LogString& option, const LogString& value)
    {
            if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("FILE"), LOG4CXX_STR("file"))
                    || StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("FILENAME"), LOG4CXX_STR("filename")))
            {
                    synchronized sync(mutex);
                    m_sFilename = stripDuplicateBackslashes(value);
            }
            else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("APPEND"), LOG4CXX_STR("append")))
            {
                    synchronized sync(mutex);
                    m_bAppend = OptionConverter::toBoolean(value, true);
            }
    }


    LogString EyelockBinaryAppender::stripDuplicateBackslashes(const LogString& src)
    {
        logchar backslash = 0x5C; // '\\'
        LogString::size_type i = src.find_last_of(backslash);
        if (i != LogString::npos) {
            LogString tmp(src);
            for(;
                i != LogString::npos && i > 0;
                i = tmp.find_last_of(backslash, i - 1)) {
                //
                //   if the preceding character is a slash then
                //      remove the preceding character
                //      and continue processing
                if (tmp[i - 1] == backslash) {
                    tmp.erase(i, 1);
                    i--;
                    if (i == 0) break;
                } else {
                    //
                    //  if there an odd number of slashes
                    //     the string wasn't trying to work around
                    //     OptionConverter::convertSpecialChars
                    return src;
                }
           }
           return tmp;
        }
        return src;
    }


	void EyelockBinaryAppender::close()
	{
        if (this->closed)
			return;

        this->closed = true;
        reset();
	}


	// Member functions

	void EyelockBinaryAppender::reset()
	{
       closeFile();
    }

	void EyelockBinaryAppender::closeFile()
	{
	    if (m_os.is_open())
	    {
			try
			{
				m_os.flush();
				m_os.close();
			}
	      	catch(IOException ex)
	      	{
				// Exceptionally, it does not make sense to delegate to an
				//// ErrorHandler. Since a closed appender is basically dead.
		//		LogLog.error("Could not close " + os, e);
	      	}
	    }
	}


    void EyelockBinaryAppender::setFile(string sFilename, bool bAppend)
    {
        reset();

        try
        {
            if (getAppend())
            	m_os.open(sFilename.c_str(), ios::out | ios::binary | ios::app);
            else
            	m_os.open(sFilename.c_str(), ios::out | ios::binary | ios::trunc);
        }
        catch (IOException ex)
        {
            LogLog::error((LogString) LOG4CXX_STR("[EyelockBinaryAppender:  Failed to open file Stream [") + sFilename + LOG4CXX_STR("["));

#if 0 //DMOTODO
            //   if parent directory does not exist then attempt to create it and try to create file
            String parentName = new File(file).getParent();
            if (parentName != null) {
                File parentDir = new File(parentName);
                if (!parentDir.exists() && parentDir.mkdirs()) {
                    os = new FileOutputStream(file, append);
                } else {
                    throw ex;
                }
            } else {
                throw ex;
            }
#endif
        }


//        LogLog.debug("setFile ended");
    }


    bool EyelockBinaryAppender::checkEntryConditions()
    {
        if (closed)
        {
       //     LogLog.warn("Not allowed to write to a closed appender.");
            return false;
        }
#if 0
        if (m_os == NULL)
        {
        //    errorHandler.error("No output stream or file set for the appender named ["+ name + "].");
            return false;
        }
#endif
        return true;
    }












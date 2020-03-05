// EyelockBinaryAppender.cpp

#include <stdio.h>
#include <sys/stat.h>
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
                m_os.flush();

            // Now that we have written the file...
            // Check for rollover, and roll it over if necessary...
            rollover(p);
        }
        catch (IOException ex)
        {
            LogLog::error((LogString) LOG4CXX_STR("[EyelockBinaryAppender:  ]Not allowed to write to a closed appender."));
        }
    }


	bool EyelockBinaryAppender::rollover(Pool& p)
	{
	    synchronized sync(mutex);

	    try
	    {
	    	// Check fileLength to see if it's time to Rollover...
			long lSize = GetFileSize(m_sFilename);

			if (-1 == lSize)
			{
				return false; // Didn't rollover...
			}
			else
			{
				if (lSize > m_MaxFileSize) // Need to rollover...
				{
					char szTemp[16];
					int nCurrentIndex = m_MaxBackupIndex;

					closeFile();

					// Now, delete the highest file...
					string theFile = m_sFilename;
					sprintf(szTemp, ".%d", m_MaxBackupIndex);
					theFile += szTemp;

					// If successfully removed...
					remove(theFile.c_str()); // It may or may not exist, so this call could fail (which we ignore)

					// Rename the other files...
					do
					{
						string theFromFile = m_sFilename;

						if (nCurrentIndex > 1)
						{
							sprintf(szTemp, ".%d", nCurrentIndex-1);
							theFromFile += szTemp;
						}

						string theToFile = m_sFilename;
						sprintf(szTemp, ".%d", nCurrentIndex);
						theToFile += szTemp;

						rename(theFromFile.c_str(), theToFile.c_str());
						printf("Renaming: From %s, To %s\n", theFromFile.c_str(), theToFile.c_str());

						nCurrentIndex--;
					}while (nCurrentIndex > 0);

					// Now, create/open the new file...
					setFile(m_sFilename, m_bAppend);

					return true;
				}
			}
	    }
	    catch (std::exception& ex)
	    {
	    	LogLog::warn(LOG4CXX_STR("Exception during rollover"));
	    }

	  return false;
	}

	long EyelockBinaryAppender::GetFileSize(std::string filename)
	{
	    struct stat stat_buf;
	    int rc = stat(filename.c_str(), &stat_buf);

	    return rc == 0 ? stat_buf.st_size : -1;
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
            else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("MAXFILESIZE"), LOG4CXX_STR("maxfilesize")))
            {
					synchronized sync(mutex);
					// Default... 1 gig...
					m_MaxFileSize = OptionConverter::toFileSize(value, 1024*1024*1024);
					printf("MaxFileSize = %ld\n", m_MaxFileSize);
            }
            else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("MAXBACKUPINDEX"), LOG4CXX_STR("maxbackupindex")))
            {
                    synchronized sync(mutex);
                    m_MaxBackupIndex = OptionConverter::toInt(value, 2);
                	printf("m_MaxBackupIndex = %d\n", m_MaxBackupIndex);
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












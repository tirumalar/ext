/*
 * FileConfiguration.cpp
 *
 *  Created on: Apr 18, 2017
 *      Author: builder
 */

#include <fstream>
#include <sstream>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

#include <iostream>

#include "FileConfiguration.h"
#include "Synchronization.h"

#include "logging.h"

const char logger[30] = "FileConfiguration";

using namespace std;


namespace EyelockConfigurationNS
{
	static inline void ltrim(std::string &s)
	{
		s.erase(s.begin(), find_if(s.begin(), s.end(), not1(ptr_fun<int, int>(isspace))));
	}


	static inline void rtrim(std::string &s)
	{
		s.erase(find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(isspace))).base(), s.end());
	}


	static inline void trim(std::string &s)
	{
		ltrim(s);
		rtrim(s);
	}


	FileConfiguration::FileConfiguration(istream& istr)
	{
		load(istr);
	}


	FileConfiguration::FileConfiguration(const string& path)
	{
		load(path);
	}


	void FileConfiguration::load(istream& istr)
	{
		EyelockLog(logger, TRACE, "Loading from istream");
		ScopeLock lock(m_lock);
		_map.clear();
		while (!istr.eof())
		{
			parseLine(istr);
		}
	}


	bool FileConfiguration::load(const string& path)
	{
		EyelockLog(logger, DEBUG, "Loading from file: %s", path.c_str());
		ifstream istr(path.c_str());
		if (istr.good())
		{
			load(istr);
			return true;
		}

		return false;
	}


	bool FileConfiguration::checkFile(const string& path)
	{
		EyelockLog(logger, DEBUG, "Checking file: %s", path.c_str());
		ifstream fs(path.c_str());
		return fs.good();
	}


	bool FileConfiguration::save(const string& path)
	{
		EyelockLog(logger, DEBUG, "Saving to file: %s", path.c_str());
		ofstream ofs(path.c_str(), ifstream::out);
		if (!ofs.good())
		{
			return false;
		}

		IStringMap::const_iterator it = _map.begin();

		while (it != _map.end())
		{
			ofs << it->first << "=" << it->second << endl;
			it++;
		}

		return true;
	}


	bool FileConfiguration::update(const string& path, const string& tempPath)
	{
		EyelockLog(logger, DEBUG, "Updating file: %s, temporary file: %s", path.c_str(), tempPath.c_str());
		ifstream ifs(path.c_str(), ifstream::in);
		if (!ifs.good())
		{
			return save(path);
		}

		string tempPathLocal = tempPath;
		if (tempPathLocal.empty())
		{
			//tempPathLocal = path + ".temp";
			tempPathLocal = path;
		}

		EyelockLog(logger, DEBUG, "Processing updating %s, temporary file: %s", path.c_str(), tempPathLocal.c_str());
		IStringMap *pTempMap = new IStringMap(_map);
		stringstream ss;
		while (!ifs.eof())
		{
			updateLine(ifs, ss, pTempMap);
		}

		IStringMap::const_iterator it = pTempMap->begin();
		while (it != pTempMap->end())
		{
			ss << it->first << "=" << it->second << endl;
			it++;
		}

		delete pTempMap;

		ofstream ofs(tempPathLocal.c_str(), ofstream::out);
		if (!ofs.good())
		{
			return false;
		}

		ofs << ss.rdbuf();

		return true;
	}


	bool FileConfiguration::getRaw(const string& key, string& value)
	{
		EyelockLog(logger, TRACE, "Retrieving raw value for %s", key.c_str());
		IStringMap::const_iterator it = _map.find(key);
		if (it != _map.end())
		{
			value = it->second;
			return true;
		}
		else return false;
	}


	void FileConfiguration::setRaw(const std::string& key, const std::string& value)
	{
		EyelockLog(logger, TRACE, "Setting raw value for %s", key.c_str());
		_map[key] = value;
	}


	void FileConfiguration::keys(Keys& range)
	{
		EyelockLog(logger, DEBUG, "Retrieving keys");
		ScopeLock lock(m_lock);
		range.clear();
		for (IStringMap::const_iterator it = _map.begin(); it != _map.end(); ++it)
		{
			range.push_back(it->first);
		}
	}


	void FileConfiguration::removeRaw(const std::string& key)
	{
		EyelockLog(logger, TRACE, "Removing key: %s", key.c_str());
		IStringMap::iterator it = _map.begin();
		IStringMap::iterator itCur;
		while (it != _map.end())
		{
			itCur = it++;
			if (itCur->first.compare(key) == 0)
			{
				_map.erase(itCur);
			}
		}
	}


	void FileConfiguration::parseLine(istream& istr)
	{
		static const int eof = char_traits<char>::eof();

		int c = istr.get();
		while (c != eof && isspace(c))
		{
			c = istr.get();
		}
		if (c != eof)
		{
			if (c == ';' || c == '#' || c == '/') // number signed is used in default Eyelock.ini: # PIXCLK = ... (FW 5.01.1657)
												  // slash is also used: //Require the below 2 key only in master. Processing single slash for simplicity, assuming valid key doesn't start with it
			{
				while (c != eof && c != '\n')
				{
					c = istr.get();
				}
			}
			else
			{
				string key;
				while (c != eof && c != '=' && c != '\n')
				{
					key += (char) c; c = istr.get();
				}
				string value;
				if (c == '=')
				{
					c = istr.get();
					while (c != eof && c != '\n')
					{
						value += (char) c;
						c = istr.get();
					}
				}
				trim(value);
				_map[key] = value;
			}
		}
	}


	void FileConfiguration::updateLine(istream& istr, stringstream& ostr, IStringMap* pTempMap)
	{
		static const int eof = char_traits<char>::eof();

		IStringMap::iterator it;

		int c = istr.get();
		while (c != eof && isspace(c))
		{
			ostr.put(c);
			c = istr.get();
		}
		if (c != eof)
		{
			if (c == ';' || c == '#' || c == '/') // number signed is used in default Eyelock.ini: # PIXCLK = ... (FW 5.01.1657)
												  // slash is also used: //Require the below 2 key only in master. Processing single slash for simplicity, assuming valid key doesn't start with it
			{
				while (c != eof && c != '\n')
				{
					ostr.put(c);
					c = istr.get();
				}
				if (c != eof)
				{
					ostr.put(c);
				}
			}
			else
			{
				string key;
				while (c != eof && c != '=' && c != '\n')
				{
					key += (char) c;
					c = istr.get();
				}

				it = pTempMap->find(key);
				if (it != pTempMap->end())
				{
					ostr << key << "=" << it->second << endl;
					pTempMap->erase(it);
				}

				while (c != eof && c != '\n')
				{
					c = istr.get();
				}
			}
		}
	}

} // namespace EyelockConfigurationNS




/*
 * FileConfiguration.h
 *
 *  Created on: Apr 18, 2017
 *      Author: builder
 */

#ifndef EYELOCKCONFIGURATIONLIB_FILECONFIGURATION_H_
#define EYELOCKCONFIGURATIONLIB_FILECONFIGURATION_H_

#include "AbstractConfiguration.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

#define FILECONFIGURATION_LINE_LENGTH_MAX 256

namespace EyelockConfigurationNS
{
	class FileConfiguration : public AbstractConfiguration
	{
	public:
		FileConfiguration() {};
		virtual ~FileConfiguration() {};

		FileConfiguration(std::istream& istr);
			/// Creates an FileConfiguration and loads the configuration data
			/// from the given stream, which must be in initialization file format.

		FileConfiguration(const std::string& path);
			/// Creates an FileConfiguration and loads the configuration data
			/// from the given file, which must be in initialization file format.

		void load(std::istream& istr);
			/// Loads the configuration data from the given stream, which
			/// must be in initialization file format.

		bool load(const std::string& path);
			/// Loads the configuration data from the given file, which
			/// must be in initialization file format.

		static bool checkFile(const std::string& path);

		bool save(const std::string& path);
			/// Saves the configuration data to the given file
			/// in initialization file format.

		bool update(const std::string& path, const std::string& tempPath = "");
			/// Updates the file in initialization format
			/// with the stored configuration data.

		void keys(Keys& range);
			/// Returns the names of all keys.

	protected:
		bool getRaw(const std::string& key, std::string& value);
		void setRaw(const std::string& key, const std::string& value);
		void enumerate(const std::string& key, Keys& range);
		void removeRaw(const std::string& key);

	private:
		typedef std::map<std::string, std::string> IStringMap;

		IStringMap _map;

		void parseLine(std::istream& istr);
		void updateLine(std::istream& istr, std::stringstream& ostr, IStringMap* pTempMap);
	};

} // namespace EyelockConfigurationNS

#endif /* EYELOCKCONFIGURATIONLIB_FILECONFIGURATION_H_ */

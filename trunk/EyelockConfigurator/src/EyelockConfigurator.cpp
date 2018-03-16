//============================================================================
// Name        : EyelockConfigurator.cpp
// Author      : 
// Version     :
// Copyright   : 
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <fcntl.h>
#include "logging.h"

#include "EyelockConfiguration.h"
#include "FileConfiguration.h"

using namespace std;
using namespace EyelockConfigurationNS;

const char logger[30] = "EyelockConfigurator";

int main(int argc, char *argv[])
{
	EyelockLog(logger, DEBUG, "EyelockConfigurator Start");

	if (argc < 3)
	{
		cerr << "Eyelock configuration tool" << endl;
		cerr << "USAGE: " << argv[0] << "UPDATE <INPUT_FILE>" << endl;
		cerr << "\tUpdates Eyelock app configuration file with parameters specified in input file" << endl;
		cerr << "USAGE: " << argv[0] << "PREPARE <OUTPUT_FILE>" << endl;
		cerr << "\tSaves current Eyelock app configuration to file" << endl;
		return 1;
	}

	// DEBUG
	// TODO:: remove DEBUG!!!!
	const string debugEyelockIniFile = "/home/EyelockConfiguratorTest/Eyelock.ini";

	EyelockLog(logger, TRACE, "Loading current configuration from %s", debugEyelockIniFile.c_str());
	EyelockConfiguration conf(debugEyelockIniFile);

	// for both operation types (PREPARE and UPDATE)
	string operation(argv[1]);
	string filepath(argv[2]);
	if (operation == "UPDATE")
	{
		EyelockLog(logger, TRACE, "Updating configuration from %s", filepath.c_str());

		if (!EyelockConfigurationNS::FileConfiguration::checkFile(filepath))
		{
			EyelockLog(logger, ERROR, "Error opening input file: %s", filepath.c_str());
			return 3;
		}
		if (!conf.updateFromFile(filepath))
		{
			EyelockLog(logger, ERROR, "Error updating configuration from file %s", filepath.c_str());
			return 4;
		}

		if (!conf.save())
		{
			EyelockLog(logger, ERROR, "Error saving configuration");
			return 5;
		}
	}
	else if (operation == "PREPARE")
	{
		EyelockLog(logger, TRACE, "Preparing configuration file %s", filepath.c_str());
		conf.save(filepath);
	}
	else
	{
		EyelockLog(logger, ERROR, "Invalid operation requested: %s", operation.c_str());
		return 3;
	}

	return 0;
}

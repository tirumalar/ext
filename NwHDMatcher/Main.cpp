/*
 * Main.cpp
 *
 *  Created on: 10-Mar-2010
 *      Author: akhil
 */
#include "NWHDMatcher.h"
#include "FileConfiguration.h"

int main(void)
{
	FileConfiguration conf("NwHDMatcher.ini");
	NWHDMatcher n(conf);
	n.run();
	return 0;
}

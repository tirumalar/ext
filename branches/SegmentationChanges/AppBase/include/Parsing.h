/*
 * Parsing.h
 *
 *  Created on: Feb 3, 2012
 *      Author: dhirvonen
 */

#ifndef PARSING_H_
#define PARSING_H_

#include <string>
#include <vector>

inline std::vector<std::string> tokenize(const std::string& str, const std::string& delimiters)
{
        std::vector<std::string> tokens;

        // skip delimiters at beginning.
        std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);

        // find first "non-delimiter".
        std::string::size_type pos = str.find_first_of(delimiters, lastPos);

        while (std::string::npos != pos || std::string::npos != lastPos)
        {
                // found a token, add it to the vector.
                tokens.push_back(str.substr(lastPos, pos - lastPos));

                // skip delimiters.  Note the "not_of"
                lastPos = str.find_first_not_of(delimiters, pos);

                // find next "non-delimiter"
                pos = str.find_first_of(delimiters, lastPos);
        }

        return tokens;
}

#endif /* PARSING_H_ */

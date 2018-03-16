/*
 * Configuration.h
 *
 *  Created on: 9 Dec, 2008
 *      Author: akhil
 *      Class responsible for supplying the configuration information
 *      Subclasses may use any mechanism to acquire the configuration
 *      The parent class is used only to query the information
 */

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <string>
#include <map>

typedef std::map <std::string, std::pair<std::string, std::string> > TStrStrMap;
typedef std::pair<std::string, std::string> TStrStrPair;

class Configuration {
public:
	Configuration();
	virtual ~Configuration();
	bool getValue(const char *key, bool defValue=false);
	int getValue(const char *key, int defValue=0);
	short getValue(const char *key, short defValue=0);
	float getValue(const char *key, float defValue=0.0f);
	char getValue(const char *key, char defValue=0);
	const char *getValue(const char *key, const char *defValue=0);
	std::string getValueStr(const char *key, const std::string& value);
	short getValueIndex(const char *key, short min, short max, short defValue, ...);
	unsigned int getValueAsBinary(const char *key, const char *def);
	void setValue(const char* key, const char *value);
	TStrStrMap getMap(){return confMap;}
	TStrStrMap& getRefMap(){return confMap;}
	inline std::string trimStr(const std::string& Src,const std::string& c = " \r\n\t") {
			int p2 = Src.find_last_not_of(c);
			if (p2 == std::string::npos)
				return std::string();
			int p1 = Src.find_first_not_of(c);
			if (p1 == std::string::npos)
				p1 = 0;
			return Src.substr(p1, (p2 - p1) + 1);
		}
	void setValueIfKeyExists(const char* key, const char *value);
	bool hasKey(const char *key);

protected:
	const char* _getValue(const char *key);
	int _getValueStr(const char *key, std::string& value);
	void  _setValue(std::string& key, std::string& value);
	bool  _removeKey(std::string& skey);

private:
	TStrStrMap confMap;


};

#endif /* CONFIGURATION_H_ */

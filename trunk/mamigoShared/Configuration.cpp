/*
 * Configuration.cpp
 *
 *  Created on: 9 Dec, 2008
 *      Author: akhil
 */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "Configuration.h"
#include <algorithm>
#include <cctype>

#if (defined(__WIN32__) || defined(_WIN32))
#define strcasecmp strcmpi
#endif

struct to_upper {
  int operator() ( int ch )
  {
    return std::toupper ( ch );
  }
};


Configuration::Configuration() {
	// TODO Auto-generated constructor stub

}

Configuration::~Configuration() {
	confMap.clear();
}

int Configuration::_getValueStr(const char *key, std::string& value){
	std::string sKey(key);
	std::transform(sKey.begin(), sKey.end(), sKey.begin(), to_upper());
	if(confMap.find(sKey)!=confMap.end()){
		TStrStrPair pair = confMap[sKey];
		value = pair.second;
		return 0;
	}
	else return 1;
}
const char* Configuration::_getValue(const char *key){
	std::string sKey(key);
	std::transform(sKey.begin(), sKey.end(), sKey.begin(), to_upper());
	if(confMap.find(sKey)!=confMap.end()){
		TStrStrPair pair = confMap[sKey];
		return pair.second.c_str();
	}
	else return 0;
}
void  Configuration:: _setValue(std::string& key, std::string& value){
	key=trimStr(key);
	std::string origKey = key;
	value=trimStr(value);
	std::map<std::string, std::pair<std::string, std::string> >::iterator iter2;
	iter2 = confMap.find(key);
	TStrStrPair orig,pair;
	if (iter2 != confMap.end())
	{
		orig = iter2->second;
	}
	std::transform(key.begin(), key.end(), key.begin(), to_upper());
	_removeKey(key);	// we dont want duplicates
	if (iter2 != confMap.end())
	{
		pair = TStrStrPair(orig.first, value);
	}
	else
	{
		pair = TStrStrPair(origKey, value);
	}
	confMap[key]= pair;
}
bool  Configuration::_removeKey(std::string& sKey){
	std::string sKeyVal(sKey);
	std::transform(sKeyVal.begin(), sKeyVal.end(), sKeyVal.begin(), to_upper());
	return confMap.erase(sKeyVal);
}
bool  Configuration::hasKey(const char *key){
	std::string sKey(key);
	std::transform(sKey.begin(), sKey.end(), sKey.begin(), to_upper());
	return confMap.find(sKey)!=confMap.end();
}

void Configuration::setValue(const char* key, const char *value){
	std::string sKey(key);
	std::string sValue(value);
	_setValue(sKey,sValue);
}
bool Configuration::getValue(const char *key, bool defValue){
	if(!hasKey(key)) return defValue;
	std::string temp;
	if (_getValueStr(key, temp)) return defValue;
	const char* v=temp.c_str();

	if(0==strcasecmp(v,"1")) return true;
	if(0==strcasecmp(v,"Y")) return true;
	if(0==strcasecmp(v,"true")) return true;
	if(0==strcasecmp(v,"yes")) return true;
	return false;
}
int Configuration::getValue(const char *key, int defValue){
	if(!hasKey(key)) return defValue;
	std::string temp;
	if (_getValueStr(key, temp)) return defValue;
	const char* v=temp.c_str();
	return atoi(v);
}

short Configuration::getValue(const char *key, short defValue){
	if(!hasKey(key)) return defValue;
	std::string temp;
	if (_getValueStr(key, temp)) return defValue;
	const char* v=temp.c_str();
	return (short)(atoi(v));
}

float Configuration::getValue(const char *key, float defValue){
	if(!hasKey(key)) return defValue;
	std::string temp;
	if (_getValueStr(key, temp)) return defValue;
	const char* v=temp.c_str();
	return atof(v);
}
char Configuration::getValue(const char *key, char defValue){
	if(!hasKey(key)) return defValue;
	std::string temp;
	if (_getValueStr(key, temp)) return defValue;
	const char* v=temp.c_str();
	return v[0];
}
const char *Configuration::getValue(const char *key, const char *defValue){
	if(!hasKey(key)) return defValue;
	const char* v=_getValue(key);
	return v;
}

std::string Configuration::getValueStr(const char *key, const std::string& defValue){
	std::string temp;
	return (_getValueStr(key, temp)) ? defValue : temp;
}
/*
 * Select from a set of choices  min is the startIndex, max is the last index both inclusive
 */
short Configuration::getValueIndex(const char *key, short min, short max, short defValue, ...) {
	const char *candidate, *value = getValue(key, "");
	if (strlen(value) == 0)
		return defValue;

	va_list vl;
	va_start(vl,defValue);
	for (int i = min; i <= max; i++) {
		candidate = va_arg(vl,const char*);
		if (0 == strcasecmp(candidate, value)) {
			va_end(vl);
			return i;
		}
	}
	va_end(vl);
	return defValue;
}


unsigned int Configuration::getValueAsBinary(const char *key, const char *def){
	const char *inp=getValue(key,def);

	int sz=strlen(inp);
	unsigned int ret=0;
	for(int i=0;i<sz;i++){
		unsigned char bit=(inp[i]=='1');
		ret <<= 1;
		ret |=bit;
	}
	return ret;
}

void Configuration::setValueIfKeyExists(const char* key, const char *value){
	std::string sKey(key);
	std::string sValue(value);
	if (hasKey(key)) _setValue(sKey,sValue);
}


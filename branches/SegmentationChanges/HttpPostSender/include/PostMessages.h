#ifndef POSTMESSAGES_H_
#define POSTMESSAGES_H_

#include <string>
#include <boost/property_tree/ptree.hpp> // TODO: use forward declaration instead of including to eliminate the dependency on boost in user classes

#include "MessageExt.h"
#include "HttpPostSender.h"

struct Serializable
{
	virtual boost::property_tree::ptree serialize() const = 0;
	virtual ~Serializable() {};

	static std::string base64(const char* data, int size);
	static std::string Base64encode(const unsigned char* data, unsigned int size);
};

struct Mms : Serializable
{
	std::string mak;
	std::string mod;
	std::string ser;
	boost::property_tree::ptree serialize() const;
};

struct Geo : Serializable
{
	std::string grt;
	std::string ute;
	boost::property_tree::ptree serialize() const;
};

struct HTTPPostFace : Serializable
{
	std::string imt;
	int oid;
	int eid;
	int hll;
	int vll;
	int slc;
	int thps;
	int tvps;
	std::string cga;
	std::string csp;
	std::string dui;
	Mms mms;
	Geo geo;
	std::string imgt;
	std::string data; // base64

	HTTPPostFace(): imt(), oid(), eid(), hll(), vll(), slc(), thps(), tvps(), mms(), geo() {}

	HTTPPostFace(const char* _data, int _size);

	boost::property_tree::ptree serialize() const;
};

struct HTTPPostIris : Serializable
{
	int elr;
	int oid;
	int eid;
	int hll;
	int vll;
	int slc;
	int thps;
	int tvps;
	std::string cga;
	int bpx;
	std::string csp; // typo? in Face, it's csp = color space
	std::string dui;
	Mms mms;
	Geo geo;
	std::string imgt;
	std::string data; // base64

	HTTPPostIris(): elr(), oid(), eid(), hll(), vll(), slc(), thps(), tvps(), bpx(), mms(), geo() {}

	HTTPPostIris(const char* _data, int _size);

	boost::property_tree::ptree serialize() const;
};

struct Sendable
{
	virtual PostMsg generatePostMsg(std::string msgformat, std::string soapnamespace) const = 0;
	virtual ~Sendable() {};

};

struct SoapWrappable
{
	SoapWrappable() {};
	virtual ~SoapWrappable() {};

protected:
	boost::property_tree::ptree pt;
	virtual void fillSoap(const boost::property_tree::ptree& inner);

	std::string UnescapeJSON(const std::string& s)
	{
		std::string res;
		std::string::const_iterator it = s.begin();
	  while (it != s.end())
	  {
	    char c = *it++;
	    if (c == '\\' && it != s.end())
	    {
	      switch (*it++) {
	      case '\\': c = '\\'; break;
	      case '"': c = '\"'; break;
	      case 'n': c = '\n'; break;
	      case 't': c = '\t'; break;
	      // all other escapes
	      default:
	        break; // Copy it as is...
	      }
	    }
	    res += c;
	  }

	  return res;
	}
};

struct PostFace : Sendable
{
	HTTPPostFace face;
	PostFace(const HTTPPostFace& _face) : face(_face) {}

	PostMsg generatePostMsg(std::string msgformat, std::string soapnamespace) const;
};

struct PostIris : SoapWrappable
{
	HTTPPostIris iris1;
	HTTPPostIris iris2;

	PostIris(const HTTPPostIris& _iris1, const HTTPPostIris& _iris2) : iris1(_iris1), iris2(_iris2) {}

	PostMsg generatePostMsg(std::string msgformat, std::string soapnamespace);
};

struct PostSingleIris : SoapWrappable
{
	HTTPPostIris iris;

	PostSingleIris(const HTTPPostIris& _iris) : iris(_iris) {}

	PostMsg generatePostMsg(std::string msgformat, std::string soapnamespace);
};

struct PostFaceIris
{
	HTTPPostIris iris1;
	HTTPPostIris iris2;
	HTTPPostFace face;

	PostFaceIris(const HTTPPostFace& _face, const HTTPPostIris& _iris1, const HTTPPostIris& _iris2) : iris1(_iris1), iris2(_iris2), face(_face) {}

	PostMsg generatePostMsg(std::string msgformat, std::string soapnamespace) const;
};

struct SignalError
{
	std::string code;
	std::string message;
	std::string severity;
	std::string dui;

	PostMsg generatePostMsg(std::string msgformat, std::string soapnamespace) const;
};

struct SignalMaintenance
{
	std::string code;
	std::string message;
	std::string severity;
	std::string dui;

	PostMsg generatePostMsg(std::string msgformat, std::string soapnamespace) const;
};

struct SignalHeartbeat : SoapWrappable
{
	std::string dui;

	PostMsg generatePostMsg(std::string msgformat, std::string soapnamespace);
};

struct SignalTripwire
{
	std::string dui;
	int dir;
	int oid;
	int eid;

	PostMsg generatePostMsg(std::string msgformat, std::string soapnamespace) const;
};


#endif /* POSTMESSAGES_H_ */

#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>

#include "PostMessages.h"

std::string Serializable::base64(const char* data, int size)
{
	if (data == NULL || size <= 0)
	{
		return std::string("");
	}

	return Base64encode((const unsigned char *)data, (unsigned int)size);
}


static const std::string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";


std::string Serializable::Base64encode(unsigned char const* bytes_to_encode, unsigned int in_len)
{
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--)
  {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3)
    {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while((i++ < 3))
      ret += '=';

  }

  return ret;
}


void SoapWrappable::fillSoap(const boost::property_tree::ptree& inner)
{
	// <soap12:Envelope xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
	// xmlns:xsd="http://www.w3.org/2001/XMLSchema"
	// xmlns:soap12="https://www.w3.org/2003/05/soap-envelope">
	// </soap12:Envelope>

	pt.put("S:Envelope", "");
	//pt.add("S:Envelope.<xmlattr>.xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	//pt.add("S:Envelope.<xmlattr>.xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
	pt.add("S:Envelope.<xmlattr>.xmlns:S", "http://schemas.xmlsoap.org/soap/envelope/");

	pt.add_child("S:Envelope.S:Body", inner);
}

boost::property_tree::ptree Mms::serialize() const
{
	boost::property_tree::ptree pt;
	pt.put("mak", mak);
	pt.put("mod", mod);
	pt.put("ser", ser);
	return pt;
}

boost::property_tree::ptree Geo::serialize() const
{
	boost::property_tree::ptree pt;
	pt.put("grt", grt);
	pt.put("ute", ute);
	return pt;
}

HTTPPostFace::HTTPPostFace(const char* _data, int _size): imt(), oid(), eid(), hll(), vll(), slc(), thps(), tvps(), mms(), geo()
{
	data = base64(_data, _size);
}

boost::property_tree::ptree HTTPPostFace::serialize() const
{
	boost::property_tree::ptree pt;
	pt.put("imt", imt);
	pt.put("oid", oid);
	pt.put("eid", eid);
	pt.put("hll", hll);
	pt.put("vll", vll);
	pt.put("slc", slc);
	pt.put("thps", thps);
	pt.put("tvps", tvps);
	pt.put("cga", cga);
	pt.put("csp", csp);
	pt.put("dui", dui);
	pt.add_child("mms", mms.serialize());
	pt.add_child("geo", geo.serialize());
	pt.put("imgt", imgt);
	pt.put("data", data);
	return pt;
}

HTTPPostIris::HTTPPostIris(const char* _data, int _size): elr(), oid(), eid(), hll(), vll(), slc(), thps(), tvps(), bpx(), mms(), geo()
{
	data = base64(_data, _size);
}

boost::property_tree::ptree HTTPPostIris::serialize() const
{
	boost::property_tree::ptree pt;
	pt.put("elr", elr);
	pt.put("oid", oid);
	pt.put("eid", eid);
	pt.put("hll", hll);
	pt.put("vll", vll);
	pt.put("slc", slc);
	pt.put("thps", thps);
	pt.put("tvps", tvps);
	pt.put("cga", cga);
	pt.put("bpx", bpx);
	pt.put("csp", csp);
	pt.put("dui", dui);
	pt.add_child("mms", mms.serialize());
	pt.add_child("geo", geo.serialize());
	pt.put("imgt", imgt);
	pt.put("data", data);
	return pt;
}

PostMsg PostFace::generatePostMsg(std::string msgformat, std::string soapnamespace) const
{
	boost::property_tree::ptree pt;

	pt.add_child("face", face.serialize());

	std::stringstream ss;
	boost::property_tree::xml_parser::write_xml(ss, pt);

	return PostMsg(ss.str(), PostMsg::POST_FACE);
}

PostMsg PostIris::generatePostMsg(std::string msgformat, std::string soapnamespace)
{

	if (msgformat == "FORMAT_SOAP")
	{
		std::stringstream ss;
		std::string soapns = soapnamespace + "/";

		boost::property_tree::ptree innerPt;

		innerPt.put("PostIris", "");
		innerPt.add("PostIris.<xmlattr>.xmlns", soapns.c_str());
		innerPt.add_child("PostIris.irises.iris", iris1.serialize());
		innerPt.add_child("PostIris.irises.iris", iris2.serialize());

		fillSoap(innerPt);

		boost::property_tree::xml_parser::write_xml(ss, pt);

		return PostMsg(ss.str(), PostMsg::POST_IRIS);
	}
	else if (msgformat == "FORMAT_JSON")// JSON
	{
		std::string jsonstring;

		// For JSON we only support 3 items... StationID, left and right images only...  No other info...
		// Add one in, just to get our deviceID serialized, that's the only thing we need besides the actual image data

		// So let's just build up the message...
		jsonstring = "{\n";

		// Do left first...
		jsonstring += "\"LeftIrisImageData\":\"";
		jsonstring += ((iris1.elr == 1) ? iris1.data : iris2.data);
		jsonstring += "\",\n";

		// Do the right
		jsonstring += "\"RightIrisImageData\":\"";
		jsonstring += ((iris1.elr == 1) ? iris2.data : iris1.data);	// No matter what, can't send same eye twice so... it is what it is...
		jsonstring += "\",\n";

		// Do the station
		jsonstring += "\"StationID\":\"";
		jsonstring += iris1.dui;
		jsonstring += "\"\n}";

		return PostMsg(jsonstring, PostMsg::POST_IRIS);
	}
	else
	{
		printf("[PostIris] Unknown Message Format!\n");
	}
}



PostMsg PostSingleIris::generatePostMsg(std::string msgformat, std::string soapnamespace)
{
	if (msgformat == "FORMAT_SOAP")
	{
		std::stringstream ss;
		std::string soapns = soapnamespace + "/";

		boost::property_tree::ptree innerPt;

		innerPt.put("PostIris", "");
		innerPt.add("PostIris.<xmlattr>.xmlns", soapns.c_str());
		innerPt.add_child("PostIris.irises.iris", iris.serialize());

		fillSoap(innerPt);

		boost::property_tree::xml_parser::write_xml(ss, pt);

		return PostMsg(ss.str(), PostMsg::POST_IRIS);
	}
	else if (msgformat == "FORMAT_JSON")// JSON
	{
		std::string jsonstring;

		// For JSON we only support 3 items... StationID, left and right images only...  No other info...
		// Add one in, just to get our deviceID serialized, that's the only thing we need besides the actual image data

		// So let's just build up the message...
		jsonstring += "{\n";

		// Do left first...
		jsonstring += "\"LeftIrisImageData\":\"";
		jsonstring += ((iris.elr == 1) ? iris.data : "");
		jsonstring += "\",\n";

		// Do the right
		jsonstring += "\"RightIrisImageData\":\"";
		jsonstring += ((iris.elr == 2) ? iris.data : "");
		jsonstring += "\",\n";

		jsonstring += "\"StationID\":\"";
		jsonstring += iris.dui;
		jsonstring += "\"\n}";

		return PostMsg(jsonstring, PostMsg::POST_IRIS);
	}
	else
	{
		printf("[PostSingleIris] Unknown Message Format!\n");
	}
}

PostMsg PostFaceIris::generatePostMsg(std::string msgformat, std::string soapnamespace) const
{
	boost::property_tree::ptree pt;

	pt.put("faceiris", "");
	pt.add_child("faceiris.face", face.serialize());
	pt.put("faceiris.irises", "");
	pt.add_child("faceiris.irises.iris", iris1.serialize());
	pt.add_child("faceiris.irises.iris", iris2.serialize());

	std::stringstream ss;
	boost::property_tree::xml_parser::write_xml(ss, pt);
	return PostMsg(ss.str(), PostMsg::POST_FACE_IRIS);
}

PostMsg SignalError::generatePostMsg(std::string msgformat, std::string soapnamespace) const
{
	boost::property_tree::ptree pt;
	pt.put("error.code", code);
	pt.put("error.message", message);
	pt.put("error.severity", severity);
	pt.put("error.dui", dui);

	std::stringstream ss;
	boost::property_tree::xml_parser::write_xml(ss, pt);
	return PostMsg(ss.str(), PostMsg::SIGNAL_ERROR);
}

PostMsg SignalMaintenance::generatePostMsg(std::string msgformat, std::string soapnamespace) const
{
	boost::property_tree::ptree pt;
	pt.put("maintenance.code", code);
	pt.put("maintenance.message", message);
	pt.put("maintenance.severity", severity);
	pt.put("maintenance.dui", dui);

	std::stringstream ss;
	boost::property_tree::xml_parser::write_xml(ss, pt);
	return PostMsg(ss.str(), PostMsg::SIGNAL_MAINTENANCE);
}

PostMsg SignalHeartbeat::generatePostMsg(std::string msgformat, std::string soapnamespace)
{
	boost::property_tree::ptree innerPt;
	std::string soapns = soapnamespace + "/";

	innerPt.put("SignalHeartbeat", "");
	innerPt.add("SignalHeartbeat.<xmlattr>.xmlns", soapns.c_str());

	innerPt.put("SignalHeartbeat.err.dui", dui);

	fillSoap(innerPt);

	std::stringstream ss;
	boost::property_tree::xml_parser::write_xml(ss, pt);
	return PostMsg(ss.str(), PostMsg::SIGNAL_HEARTBEAT);
}

PostMsg SignalTripwire::generatePostMsg(std::string msgformat, std::string soapnamespace) const
{
	boost::property_tree::ptree pt;
	pt.put("tripwire.dui", dui);
	pt.put("tripwire.dir", dir);
	pt.put("tripwire.oid", oid);
	pt.put("tripwire.eid", eid);

	std::stringstream ss;
	boost::property_tree::xml_parser::write_xml(ss, pt);
	return PostMsg(ss.str(), PostMsg::SIGNAL_TRIPWIRE);
}

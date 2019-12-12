#include "Poco/Exception.h"
#include "Poco/Path.h"
#include "Poco/File.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/Validator.h"
#include "Poco/Format.h"
#include "Poco/RegularExpression.h"

#define MAXMESSAGELENGTH 255

class PathValidator : public Poco::Util::Validator
{
public:
	void validate(const Poco::Util::Option& option, const std::string& value)
	{
		Poco::Path filepath("");
		if (!filepath.tryParse(value))
		{
			throw Poco::InvalidArgumentException(Poco::format("invalid path", option.fullName()));
		}
	}
};

class FileExistenceValidator : public Poco::Util::Validator
{
public:
	void validate(const Poco::Util::Option& option, const std::string& value)
	{
		Poco::Path filepath("");
		if (filepath.tryParse((value)))
		{
			Poco::File file(filepath);
			if (!file.exists())
			{
				throw Poco::InvalidArgumentException(Poco::format("file doesn't exist", option.fullName()));
			}
			if (!file.isFile())
			{
				throw Poco::InvalidArgumentException(Poco::format("invalid file path", option.fullName()));
			}
		}
		else
		{
			throw Poco::InvalidArgumentException(Poco::format("invalid path", option.fullName()));
		}
	}
};

class FileReadableValidator : public Poco::Util::Validator
{
public:
	void validate(const Poco::Util::Option& option, const std::string& value)
	{
		Poco::AutoPtr<FileExistenceValidator> fileExistenceValidator = new FileExistenceValidator();

		fileExistenceValidator->validate(option, value);
		Poco::File file(value);
		if (!file.canRead())
		{
			throw Poco::InvalidArgumentException(Poco::format("file cannot be read", option.fullName()));
		}
	}
};

class FileWritableValidator : public Poco::Util::Validator
{
public:
	void validate(const Poco::Util::Option& option, const std::string& value)
	{
		Poco::AutoPtr<FileExistenceValidator> fileExistenceValidator = new FileExistenceValidator();

		fileExistenceValidator->validate(option, value);
		Poco::File file(value);
		if (!file.canWrite())
		{
			throw Poco::InvalidArgumentException(Poco::format("file cannot be written", option.fullName()));
		}
	}
};

class BoolValidator : public Poco::Util::Validator
{
public:
	void validate(const Poco::Util::Option& option, const std::string& value)
	{
		if (!(value == "TRUE" || value == "true" || value == "FALSE" || value == "false" || value == "1" || value == "0"))
		{
			throw Poco::InvalidArgumentException(Poco::format("invalid bool", option.fullName()));
		}
	}
};

class MessageLengthValidator : public Poco::Util::Validator
{
public:
	void validate(const Poco::Util::Option& option, const std::string& value)
	{
		if (value.length() > MAXMESSAGELENGTH)
		{
			throw Poco::InvalidArgumentException(Poco::format("message is too long", option.fullName()));
		}
	}
};


class IpValidator : public Poco::Util::Validator
{
public:
	void validate(const Poco::Util::Option& option, const std::string& value)
	{
		Poco::RegularExpression epRegex("^(?:(?:25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])\\.){3}(?:25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])$");
		Poco::RegularExpression epRegex6(
			"^("
			"([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|"          // 1:2:3:4:5:6:7:8
			"([0-9a-fA-F]{1,4}:){1,7}:|"                         // 1::                              1:2:3:4:5:6:7::
			"([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|"         // 1::8             1:2:3:4:5:6::8  1:2:3:4:5:6::8
			"([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|"  // 1::7:8           1:2:3:4:5::7:8  1:2:3:4:5::8
			"([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|"  // 1::6:7:8         1:2:3:4::6:7:8  1:2:3:4::8
			"([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|"  // 1::5:6:7:8       1:2:3::5:6:7:8  1:2:3::8
			"([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|"  // 1::4:5:6:7:8     1:2::4:5:6:7:8  1:2::8
			"[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|"       // 1::3:4:5:6:7:8   1::3:4:5:6:7:8  1::8
			":((:[0-9a-fA-F]{1,4}){1,7}|:)|"                     // ::2:3:4:5:6:7:8  ::2:3:4:5:6:7:8 ::8       ::
			"fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|"     // fe80::7:8%eth0   fe80::7:8%1     (link-local IPv6 addresses with zone index)
			"::(ffff(:0{1,4}){0,1}:){0,1}"
			"((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.){3,3}"
			"(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|"          // ::255.255.255.255   ::ffff:255.255.255.255  ::ffff:0:255.255.255.255  (IPv4-mapped IPv6 addresses and IPv4-translated addresses)
			"([0-9a-fA-F]{1,4}:){1,4}:"
			"((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.){3,3}"
			"(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])"           // 2001:db8:3:4::192.0.2.33  64:ff9b::192.0.2.33 (IPv4-Embedded IPv6 Address)
			")"
				);


		Poco::RegularExpression::Match mtch;
		if (!epRegex.match(value, mtch))
		{
			if (!epRegex6.match(value, mtch))
			{
				throw Poco::InvalidArgumentException(Poco::format("invalid IP", option.fullName()));
			}
		}
	}
};

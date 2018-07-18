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
		Poco::RegularExpression::Match mtch;
		if (!epRegex.match(value, mtch))
		{
			throw Poco::InvalidArgumentException(Poco::format("invalid IP", option.fullName()));
		}
	}
};

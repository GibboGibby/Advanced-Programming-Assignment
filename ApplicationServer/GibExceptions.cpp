#include "GibExceptions.h"


std::ostream& operator<<(std::ostream& os, const GibException* e)
{
	// Friend function to output the appropriate error
	os << e->OutputError();
	return os;
}

std::string InvalidConvertException::OutputError() const
{
	return std::string("Error converting this value - " + _errorString + " - This must be an " + _neededType);
}

std::string InvalidArgumentException::OutputError() const
{
	return std::string("Invalid arguments have been passed to the server - Invalid Argument: " + _errorString);
}

#include "GibExceptions.h"


std::ostream& operator<<(std::ostream& os, GibException* e)
{
	os << e->OutputError();
	return os;
}

std::string InvalidConvertException::OutputError()
{
	return std::string("Error converting this value - " + _errorString + " - This needs to be a string");
}

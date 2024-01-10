#pragma once
#include <exception>
#include <iostream>
#include <string>

class GibException : public std::exception
{
protected:
	std::string _errorString;

public:
	virtual std::string OutputError() = 0;

	friend std::ostream& operator<<(std::ostream& os, const GibException* e);
};

class InvalidConvertException : public GibException
{
	InvalidConvertException(const char* error)
	{
		_errorString = error;
	}

	std::string OutputError() override;
};
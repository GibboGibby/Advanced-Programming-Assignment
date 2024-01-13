#pragma once
#include <exception>
#include <iostream>
#include <string>

class GibException : public std::exception
{
protected:
	std::string _errorString;

public:
	virtual std::string OutputError() const = 0;

	friend std::ostream& operator<<(std::ostream& os, const GibException* e);
};

class InvalidConvertException : public GibException
{
private:
	std::string _neededType;
public:
	InvalidConvertException(const char* error, std::string neededType)
	{
		_errorString = error;
		_neededType = neededType;
	}

	std::string OutputError() const override;
};

class InvalidArgumentException : public GibException
{
public:
	InvalidArgumentException(const char* error)
	{
		_errorString = error;
	}

	std::string OutputError() const override;
};

class InvalidRangeException : public GibException
{
private:
	const char* _min;
	const char* _max;
public:
	InvalidRangeException(const char* error, const char* min, const char* max)
	{
		_errorString = error;
		_min = min;
		_max = max;
	}

	std::string OutputError() const override;
};
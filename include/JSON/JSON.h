#pragma once
#include "Vector.h"

// should provide util for a custom json implementation used for storing node data, canvas position, settings, etc.
/*
"sectionName": {
	"data": "string",
	"num": 1234,
	"float": 1.04,
	"array": [
		42,
		"name", // comment
		"obj",
		{
			"nestedObject": 42
			"okay": true
		}
	]
}
*/

#include <vector>
#include <unordered_map>
#include <map>
#include <fstream>

struct JSONType
{
	enum Type { Num, Float, String, Bool, Array, Object };
	union Data {
		long i;
		double f;
		std::string s;
		bool b;
		std::vector<JSONType> arr;
		std::unordered_map<std::string, JSONType> obj;
	} data;
	Type t;

	std::string ToString(int indents = 0) const;
};

class JSONDecoder
{
public:
	std::vector<std::pair<std::string, JSONType>> Decode(const std::string& str);

private:
	std::vector<std::string> Tokenise(const std::string& inp) const;
};
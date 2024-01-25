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
	enum Type { None, Num, Float, String, Bool, Array, Object };
	long i = 0l;
	double f = 0.0;
	std::string s;
	bool b = false;
	std::vector<JSONType> arr;
	std::unordered_map<std::string, JSONType> obj;
	Type t;

	inline JSONType() :t(Type::None) {}
	inline JSONType(long i) : i(i), t(Type::Num) {}
	inline JSONType(double f) : f(f), t(Type::Float) {}
	inline JSONType(const std::string& s) : s(s), t(Type::String) {}
	inline JSONType(const char* s) : s(s), t(Type::String) {}
	inline JSONType(bool b) : b(b), t(Type::Bool) {}
	inline JSONType(const std::vector<JSONType>& arr) : arr(arr), t(Type::Array) {}
	inline JSONType(const std::unordered_map<std::string, JSONType> obj) : obj(obj), t(Type::Object) {}

	std::string ToString(int indents = 0) const;
	static JSONType FromTokens(const std::vector<std::string>& tokens, Type type);
};

class JSONDecoder
{
public:
	std::vector<std::pair<std::string, JSONType>> Decode(const std::string& str);

private:
	std::vector<std::string> Tokenise(const std::string& inp) const;
};

extern void RegisterJSONCommands();
extern void TestJSONCommand(std::vector<std::string> args);
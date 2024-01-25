#include "JSON/JSON.h"
#include <sstream>
#include "Engine/Console.h"

std::string JSONType::ToString(int indents) const
{
    switch (t)
    {
    case Num:
        return std::to_string(i);
    case Float:
        return std::to_string(f);
    case String:
        return "\"" + s + "\"";
    case Bool:
        return b ? "true" : "false";

    case Array:
    {
        std::string ind = std::string(indents, '\t');
        std::string r = "[\n";
        for (auto& v : arr)
        {
            r += ind + "\t" + v.ToString(indents + 1) + ",\n";
        }
        return r + ind + "]";
    }

    case Object:
    {
        std::string ind = std::string(indents, '\t');
        std::string r = "{\n";
        for (auto& v : obj)
        {
            r += ind + "\t\"" + v.first + "\": " + v.second.ToString(indents + 1) + ",\n";
        }
        return r + ind + "}";
    }
    }
    return "ERRTYPE";
}

JSONType JSONType::FromTokens(const std::vector<std::string>& tokens, Type type)
{
    switch (type)
    {
    case Type::Num:
        return JSONType();
    case Type::Float:
        return JSONType();
    case Type::String:
        return JSONType();
    case Type::Bool:
        return JSONType();
    case Type::Array:
        return JSONType();
    case Type::Object:
        return JSONType();
    }
    return JSONType();
}

std::vector<std::pair<std::string, JSONType>> JSONDecoder::Decode(const std::string& str)
{
    std::vector<std::string> tokens = Tokenise(str);
    
    std::vector<JSONType> res;
    for (auto& t : tokens)
        res.push_back(JSONType(t));

    return {
        {"tokens", JSONType(res)}
    };
}

#include <regex>
std::vector<std::string> JSONDecoder::Tokenise(const std::string& inp) const
{
    std::istringstream ss{inp};
    std::string line;
    std::ostringstream sum;
    while (std::getline(ss, line))
    {
        // locate and remove any possible comments
        bool inQuotes = false;
        for (size_t i = 0; i < line.size(); i++)
        {
            if (line[i] == '\"' && (i == 0 || !(inQuotes && line[i - 1] == '\\')))
                inQuotes = !inQuotes;
            if (i != 0 && line[i] == '/' && line[i - 1] == '/')
            {
                line = line.substr(0, i - 1);
                break;
            }
        }
        sum << line;
    }
    std::string tot = sum.str();

    // tokenise with regex
    // made this bad boy up all by myself (pain and suffering was endured at heroic length)
    std::regex tokenise("(?:\"(?:(?:\\\\\")|[^\"])*\")|(?:\\d+(?:.\\d+)?)|[{}[\\]:,]|(?:true)|(?:false)");
    std::vector<std::string> result;
    auto begin = std::sregex_iterator(tot.begin(), tot.end(), tokenise);
    auto end = std::sregex_iterator();
    for (std::sregex_iterator i = begin; i != end; i++)
        result.push_back((*i).str());

    return result;
}

void RegisterJSONCommands()
{
    Console::AddCommand(&TestJSONCommand, "json");
    Console::Log("<JSON>: registered json command");

    JSONType jsonobj = JSONType({
        { "dumb", JSONType((long)24)},
        { "another", JSONType(12.04)},
        { "arr", JSONType({ JSONType((long)24), JSONType("string"), JSONType(true)})}
    });
    Console::Log("Test json object: " + jsonobj.ToString());
}

void TestJSONCommand(std::vector<std::string> args)
{
    std::string s;
    for (auto& v : args)
        s += v;
    JSONDecoder d;
    std::vector<std::pair<std::string, JSONType>> r = d.Decode(s);
    Console::Log("Decode result:");
    for (auto& p : r)
        Console::Log("\"" + p.first + "\": " + p.second.ToString());
}

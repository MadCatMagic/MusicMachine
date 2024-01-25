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
        for (size_t i = 0; i < arr.size(); i++)
        {
            r += ind + "\t" + arr[i].ToString(indents + 1) + (i < arr.size() - 1 ? ",\n" : "\n");
        }
        return r + ind + "]";
    }

    case Object:
    {
        std::string ind = std::string(indents, '\t');
        std::string r = "{\n";
        size_t i = 0;
        for (auto& v : obj)
        {
            i++;
            r += ind + "\t\"" + v.first + "\": " + v.second.ToString(indents + 1) + (i < obj.size() ? ",\n" : "\n");
        }
        return r + ind + "}";
    }
    }
    return "ERRTYPE";
}

#include <exception>

JSONType JSONType::FromTokens(const std::vector<std::string>& tokens)
{
    // will be either bool, long, double, or text
    if (tokens.size() == 1)
    {
        std::string s = tokens[0];
        if (s[0] == '\"')
            return JSONType(s.substr(1, s.size() - 2));
        if (s == "true")
            return JSONType(true);
        if (s == "false")
            return JSONType(false);
        if (std::find(s.begin(), s.end(), '.') != s.end())
            return JSONType(std::stod(s));
        return JSONType(std::stol(s));
    }

    // array type
    if (tokens[0] == "[")
    {
        JSONType t{ Type::Array };
        std::vector<std::string> subSection;
        int currentLevel = 0;
        for (size_t i = 1; i < tokens.size(); i++)
        {
            if (tokens[i] == "{" || tokens[i] == "[")
                currentLevel++;
            if (currentLevel == 0 && (tokens[i] == "," || tokens[i] == "]"))
            {
                t.arr.push_back(FromTokens(subSection));
                subSection.clear();
            }
            else
                subSection.push_back(tokens[i]);
            if (tokens[i] == "}" || tokens[i] == "]")
                currentLevel--;
        }
        return t;
    }

    // object type
    if (tokens[0] == "{")
    {
        JSONType t{ Type::Object };
        std::string first;
        std::vector<std::string> subSection;
        int currentLevel = 0;
        for (size_t i = 1; i < tokens.size(); i++)
        {
            if (first != "")
            {
                if (tokens[i] == "{" || tokens[i] == "[")
                    currentLevel++;
                if (currentLevel == 0 && (tokens[i] == "," || tokens[i] == "}"))
                {
                    t.obj[first.substr(1, first.size() - 2)] = FromTokens(subSection);
                    subSection.clear();
                    first = "";
                }
                else
                    subSection.push_back(tokens[i]);
                if (tokens[i] == "}" || tokens[i] == "]")
                    currentLevel--;
            }
            else if (tokens[i] == ":")
                first = tokens[i - 1];
        }
        return t;
    }

    // error!
    std::string s;
    for (const std::string& v : tokens)
        s += "\"" + v + "\", ";
    Console::LogErr("Malformed sequence of tokens passed to FromTokens: [" + s + "]");

    return JSONType();
}

std::vector<std::pair<std::string, JSONType>> JSONDecoder::Decode(const std::string& str)
{
    std::vector<std::string> tokens = Tokenise(str);
    JSONType t = JSONType::FromTokens(tokens);
    std::vector<JSONType> res;
    for (auto& t : tokens)
        res.push_back(JSONType(t));

    return {
        {"tokens", JSONType(res)},
        {"obj", t}
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
    TestJSONCommand({ jsonobj.ToString() });
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

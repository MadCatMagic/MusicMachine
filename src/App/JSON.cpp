#include "App/JSON.h"
#include <sstream>
#include "Engine/Console.h"

#include <fstream>
#include <iomanip>
#include <regex>

std::string JSONType::ToString(bool compress, int indents) const
{
    switch (t)
    {
    case Num:
        return std::to_string(i);
    case Float:
    {
        std::stringstream ss;
        ss << std::setprecision(16) << f;
        return ss.str();
    }
    case String:
        return "\"" + s + "\"";
    case Bool:
        return b ? "true" : "false";

    case Array:
    {
        if (arr.size() == 0)
            return "[]";
        if (compress)
        {
            std::string r = "[";
            for (size_t i = 0; i < arr.size(); i++)
                r += arr[i].ToString(true) + (i < arr.size() - 1 ? "," : "");
            return r + "]";
        }
        else
        {
            std::string ind = std::string(indents, '\t');
            std::string r = "[\n";
            for (size_t i = 0; i < arr.size(); i++)
                r += ind + "\t" + arr[i].ToString(false, indents + 1) + (i < arr.size() - 1 ? ",\n" : "\n");
            return r + ind + "]";
        }
    }

    case Object:
    {
        if (obj.size() == 0)
            return "{}";
        if (compress)
        {
            std::string r = "{";
            size_t i = 0;
            for (auto& v : obj)
            {
                i++;
                r += "\"" + v.first + "\":" + v.second.ToString(true) + (i < obj.size() ? "," : "");
            }
            return r + "}";
        }
        else
        {
            std::string ind = std::string(indents, '\t');
            std::string r = "{\n";
            size_t i = 0;
            for (auto& v : obj)
            {
                i++;
                r += ind + "\t\"" + v.first + "\": " + v.second.ToString(false, indents + 1) + (i < obj.size() ? ",\n" : "\n");
            }
            return r + ind + "}";
        }
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
        if (s == "ERRTYPE")
        {
            Console::LogWarn("Token of type ERRTYPE passed to FromTokens, returning an empty JSONType.");
            return JSONType();
        }
        return JSONType(std::stol(s));
    }

    // array type
    if (tokens[0] == "[")
    {
        JSONType t{ Type::Array };
        if (tokens[1] == "]")
            return t;
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
        if (tokens[1] == "}")
            return t;
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

    throw std::runtime_error("JSON decoding error");
}

// expects a file pointing to an object.
std::pair<std::unordered_map<std::string, JSONType>, bool> JSONConverter::DecodeFile(const std::string& filename)
{
    std::ifstream stream(filename);
    std::string f;
    if (stream) {
        std::ostringstream ss;
        ss << stream.rdbuf(); // reading data
        f = ss.str();
    }
    stream.close();
    return Decode(f);
}

// if this process fails, result.second == false.
// if it is an object, returns the map of object keys to items in result.first.
// otherwise, returns a map containing { "obj": JSONType }
std::pair<std::unordered_map<std::string, JSONType>, bool> JSONConverter::Decode(const std::string& str)
{
    std::vector<std::string> tokens = Tokenise(str);
    JSONType t;
    try
    {
        t = JSONType::FromTokens(tokens);
    }
    catch (...)
    {
        Console::LogErr("Failed to decode tokens in JSONDecoder::Decode");
        //std::vector<JSONType> res;
        //for (auto& t : tokens)
        //    res.push_back(JSONType(t));
        //Console::LogErr(JSONType(res).ToString());
        return { {}, false };
    }

    if (t.t == JSONType::Object)
        return { t.obj, true };
    else
        return { {{ "obj", t }}, true };
}

void JSONConverter::WriteFile(const std::string& filename, const JSONType& type)
{
    std::ofstream stream(filename, std::ios::out | std::ios::trunc);
    if (type.t != JSONType::Object)
        throw "AAAA";
    
    std::string dat = type.ToString(true);
    stream.write(dat.c_str(), dat.size());
    stream.close();
}

std::vector<std::string> JSONConverter::Tokenise(const std::string& inp) const
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
    std::regex tokenise("(?:\"(?:(?:\\\\\")|[^\"])*\")|(?:[-]?\\d+(?:[.]\\d+)?)|[{}[\\]:,]|(?:true)|(?:false)|(?:ERRTYPE)");
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
}

void TestJSONCommand(std::vector<std::string> args)
{
    std::string s;
    for (auto& v : args)
        s += v;
    JSONConverter d;
    auto r = d.Decode(s);
    if (!r.second)
        return;
    Console::Log("Decode result:");
    for (auto& p : r.first)
        Console::Log("\"" + p.first + "\": " + p.second.ToString(false));
}

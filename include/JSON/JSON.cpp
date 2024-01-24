#include "JSON/JSON.h"
#include <sstream>

std::string JSONType::ToString(int indents) const
{
    switch (t)
    {
    case Num:
        return std::to_string(data.i);
    case Float:
        return std::to_string(data.f);
    case String:
        return data.s;
    case Bool:
        return data.b ? "true" : "false";

    case Array:
    {
        std::string ind = std::string(indents, '\t');
        std::string r = "[\n";
        for (auto& v : data.arr)
        {
            r += ind + "\t" + v.ToString(indents + 1) + ",\n";
        }
        return r + ind + "]";
    }

    case Object:
    {
        std::string ind = std::string(indents, '\t');
        std::string r = "{\n";
        for (auto& v : data.obj)
        {
            r += ind + "\t\"" + v.first + "\": " + v.second.ToString(indents + 1) + ",\n";
        }
        return r + ind + "}";
    }
    }
    return "ERRTYPE";
}

std::vector<std::pair<std::string, JSONType>> JSONDecoder::Decode(const std::string& str)
{
    std::vector<std::string> tokens = Tokenise(str);

    return std::vector<std::pair<std::string, JSONType>>();
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

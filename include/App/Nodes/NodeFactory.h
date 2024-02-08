#pragma once
#include <functional>
#include "App/Nodes/Node.h"

// from https://stackoverflow.com/questions/5450742/is-it-possible-in-c-to-loop-over-all-subclasses-of-an-abstract-class
// lol
class NodeFactory
{
public:
    typedef std::function<Node*()> Builder;

    /// returns true if the registration succeeded, false otherwise
    inline bool Register(const std::string& key, const std::string& menuName, Builder const& builder) 
    {
        return map.insert(std::make_pair(key, std::make_pair(menuName, builder))).second;
    }

    /// returns a pointer to a new instance of Foo (or a derived class)
    /// if the key was found, 0 otherwise
    inline Node* Build(const std::string& key) const 
    {
        auto it = map.find(key);
        if (it == map.end()) { return nullptr; } // no such key
        return (it->second.second)();
    }

    // returns <'internal name', 'menu name'>
    inline std::vector<std::pair<std::string, std::string>> Names() const
    {
        std::vector<std::pair<std::string, std::string>> arr;
        for (const auto& pair : map)
            arr.push_back(std::make_pair(pair.first, pair.second.first));
        return arr;
    }

private:
    std::map<std::string, std::pair<std::string, Builder>> map;
};

template <typename Derived>
extern inline Node* NodeBuilder() { return new Derived(); }

extern inline NodeFactory& GetNodeFactory() { static NodeFactory F; return F; }
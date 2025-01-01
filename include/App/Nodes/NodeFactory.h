#pragma once
#include <functional>
#include "App/Nodes/Node.h"

class NodeFactory
{
public:
    typedef std::function<Node*()> Builder;

    /// returns true if the registration succeeded, false otherwise
    inline bool Register(const std::string& key, const std::string& folder, const std::string& menuName, Builder const& builder) 
    {
        BuilderData obj;
        obj.builder = builder;
        obj.folder = folder;
        obj.menuName = menuName;
        return map.insert(std::make_pair(key, obj)).second;
    }

    // returns a pointer to a new instance of Node (or a derived class)
    // if the key was found, 0 otherwise
    inline Node* Build(const std::string& key) const 
    {
        auto it = map.find(key);
        if (it == map.end()) { return nullptr; } // no such key
        return (it->second.builder)();
    }

    // returns <'internal name', 'menu name'>
    inline std::vector<std::pair<std::string, std::string>> Names() const
    {
        std::vector<std::pair<std::string, std::string>> arr;
        for (const auto& pair : map)
            arr.push_back(std::make_pair(pair.first, pair.second.menuName));
        return arr;
    }

    typedef std::vector<std::pair<std::string, std::vector<std::pair<std::string, std::string>>>> FolderData;
    // returns ['folder': ['internal name', 'menu name']]
    inline FolderData Folders() const
    {
        FolderData dat;
        dat.push_back({ "", {} });
        for (const auto& pair : map)
        {
            for (size_t i = 0; i < dat.size(); i++)
                if (dat[i].first == pair.second.folder)
                {
                    dat[i].second.push_back(std::make_pair(pair.first, pair.second.menuName));
                    goto escape;
                }
            dat.insert(dat.begin(), {pair.second.folder, {}});
            dat[0].second.push_back(std::make_pair(pair.first, pair.second.menuName));
        escape:
            ;
        }
        return dat;
    }

private:
    struct BuilderData
    {
        std::string menuName;
        std::string folder;
        Builder builder;
    };

    std::map<std::string, BuilderData> map;
};

template <typename Derived>
extern inline Node* NodeBuilder() { return new Derived(); }

// singleton object
extern inline NodeFactory& GetNodeFactory() { static NodeFactory F; return F; }
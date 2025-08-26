#pragma once

#include <string>
#include <unordered_map>
#include <vector>

class GraphicsPoints
{
private:
    std::unordered_map<std::string, std::vector<double>> graphics_value{
        {"wins", {}},
        {"damage", {}},
        {"xp", {}},
        {"rating", {}}};

    std::unordered_map<std::string, std::vector<int>> graphics_x_value{
        {"wins", {}},
        {"damage", {}},
        {"xp", {}},
        {"rating", {}}};

public:
    void setXYPoint(const std::string &key, int x, double y)
    {
        this->graphics_value[key].push_back(y);
        this->graphics_x_value[key].push_back(x);
    }

    double getLastXPoint(const std::string &key) const
    {
        auto it = graphics_x_value.find(key);
        if (it != graphics_x_value.end() && !it->second.empty())
        {
            return it->second.back();
        }
        return 0.0;
    }

    double getLastYPoint(const std::string &key) const
    {
        auto it = graphics_value.find(key);
        if (it != graphics_value.end() && !it->second.empty())
        {
            return it->second.back();
        }
        return 0.0;
    }

    const std::vector<double> &getYValues(const std::string &key) const
    {
        static const std::vector<double> empty;
        auto it = graphics_value.find(key);
        return (it != graphics_value.end()) ? it->second : empty;
    }

    const std::vector<int> &getXValues(const std::string &key) const
    {
        static const std::vector<int> empty;
        auto it = graphics_x_value.find(key);
        return (it != graphics_x_value.end()) ? it->second : empty;
    }
};
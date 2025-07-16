#pragma once

#include <cstdint>
#include <unordered_map>
#include <string>
#include <classes/json.hpp>
#include <fstream>

using json = nlohmann::json;

struct VehicleData
{
    int64_t id = 0;
    int64_t battles = 0;
    int64_t totalDamage = 0;
    int64_t wins = 0;

    VehicleData operator-(const VehicleData &other) const
    {
        return {
            battles - other.battles,
            totalDamage - other.totalDamage,
            wins - other.wins};
    }
};

class VehicleStats
{
private:
    VehicleData firstData;
    VehicleData newData;
    VehicleData currentData;

    std::unordered_map<int64_t, VehicleData> vehicles;
    std::unordered_map<int64_t, std::string> names_vehicles;

public:
    VehicleStats(const std::string &path)
    {
        std::ifstream file(path);
        if (!file.is_open())
        {
            throw std::runtime_error("Can't open file for load vehicle names...");
        }
        json j;
        file >> j;

        if (j.contains("tanks"))
        {
            j = j["tanks"];
        }
        for (auto i : j)
        {
            if (i.contains("id") && i.contains("name"))
            {
                int64_t id = i["id"].get<int64_t>();
                std::string name = i["name"];
                names_vehicles[id] = name;
            }
        }
    }

    void initialStats(const VehicleData &stats)
    {
        firstData = stats;
    }

    void updateStats(const VehicleData &newStats)
    {
        newData = newStats;
        currentData = newData - firstData;
    }

    const VehicleData &getFirstData() const
    {
        return firstData;
    }

    const VehicleData &getCurrentData() const
    {
        return currentData;
    }
};
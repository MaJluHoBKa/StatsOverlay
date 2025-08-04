#pragma once

#include <cstdint>
#include <unordered_map>
#include <string>
#include "json.hpp"
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
            id,
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
    std::unordered_map<int64_t, VehicleData> first_vehicles;
    std::unordered_map<int64_t, VehicleData> prev_vehicles;
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

    void initialStats(const json &stats)
    {
        for (auto i : stats)
        {
            VehicleData vehicleData;
            vehicleData.id = i["tank_id"].get<int64_t>();
            vehicleData.battles = i["all"]["battles"].get<int64_t>();
            vehicleData.wins = i["all"]["wins"].get<int64_t>();
            vehicleData.totalDamage = i["all"]["damage_dealt"].get<int64_t>();
            first_vehicles[vehicleData.id] = vehicleData;
            vehicles[vehicleData.id] = vehicleData - first_vehicles[vehicleData.id];
            prev_vehicles = vehicles;
        }
    }

    void updateStats(const json &newStats)
    {
        prev_vehicles = vehicles;
        for (auto i : newStats)
        {
            VehicleData vehicleData;
            vehicleData.id = i["tank_id"].get<int64_t>();
            vehicleData.battles = i["all"]["battles"].get<int64_t>();
            vehicleData.wins = i["all"]["wins"].get<int64_t>();
            vehicleData.totalDamage = i["all"]["damage_dealt"].get<int64_t>();
            if (vehicles.count(vehicleData.id))
            {
                vehicles[vehicleData.id] = vehicleData - first_vehicles[vehicleData.id];
            }
            else
            {
                first_vehicles[vehicleData.id] = vehicleData;
                vehicles[vehicleData.id] = vehicleData - first_vehicles[vehicleData.id];
                prev_vehicles[vehicleData.id] = vehicleData - first_vehicles[vehicleData.id];
            }
        }
    }

    const VehicleData *getUpdatedVehicle() const
    {
        for (const auto &v : vehicles)
        {
            auto it = prev_vehicles.find(v.first);
            if (it != prev_vehicles.end() && v.second.battles > it->second.battles)
            {
                auto nameIt = names_vehicles.find(v.first);
                std::string tankName = (nameIt != names_vehicles.end()) ? nameIt->second : "Unknown";
                double winrate = (static_cast<double>(v.second.wins) / v.second.battles) * 100.0;
                // std::cout << "Tank: " << tankName
                //           << " Battle: " << v.second.battles
                //           << " Damage: " << (v.second.totalDamage / v.second.battles)
                //           << " Wins: " << std::round(winrate * 100.0) / 100.0 << "%" << std::endl;
                return &v.second;
            }
        }
        return nullptr;
    }

    std::string getName(int64_t id)
    {
        if (names_vehicles.count(id))
        {
            return names_vehicles[id];
        }
        return "Unknown";
    }

    void setNames(const json &names)
    {
        for (auto &[key, value] : names.items())
        {
            if (value.contains("tank_id") && value.contains("name"))
            {
                int64_t id = value["tank_id"].get<int64_t>();
                std::string name = value["name"];
                names_vehicles[id] = name;
            }
        }
    }
};
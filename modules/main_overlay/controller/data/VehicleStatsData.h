#pragma once

#include <cstdint>
#include <unordered_map>
#include <string>
#include <../external/json.hpp>
#include <fstream>
#include <QFile>

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

class VehicleStatsData
{
private:
    VehicleData firstData;
    VehicleData newData;
    VehicleData currentData;

    std::unordered_map<int64_t, VehicleData> vehicles;
    std::unordered_map<int64_t, VehicleData> first_vehicles;
    std::unordered_map<int64_t, VehicleData> prev_vehicles;
    std::unordered_map<int64_t, std::string> names_vehicles;
    std::unordered_map<int64_t, int64_t> tier_vehicles;

public:
    VehicleStatsData()
    {
        QFile file(":/vehicles_data_stats/resources/vehicles/tanks.json");
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            throw std::runtime_error("Can't open file for load vehicle names...");
        }

        QByteArray data = file.readAll();
        file.close();

        json j = json::parse(data.toStdString());

        if (j.contains("tanks"))
        {
            j = j["tanks"];
        }

        for (auto &i : j)
        {
            if (i.contains("id") && i.contains("name") && i.contains("tier"))
            {
                int64_t id = i["id"].get<int64_t>();
                std::string name = i["name"];
                int64_t tier = i["tier"].get<int64_t>();

                names_vehicles[id] = name;
                tier_vehicles[id] = tier;
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
        std::cout << "=== updateStats called ===" << std::endl;
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
                std::cout << "Tank: " << tankName
                          << " Battle: " << v.second.battles
                          << " Damage: " << (v.second.totalDamage / v.second.battles)
                          << " Wins: " << std::round(winrate * 100.0) / 100.0 << "%" << std::endl;
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
        return std::to_string(id);
    }

    int64_t getTier(int64_t id)
    {
        if (tier_vehicles.count(id))
        {
            return tier_vehicles[id];
        }
        return 0;
    }

    void setNames(const json &names)
    {
        for (auto &[key, value] : names.items())
        {
            if (value.contains("tank_id") && value.contains("name") && value.contains("tier"))
            {
                int64_t id = value["tank_id"].get<int64_t>();
                std::string name = value["name"];
                int64_t tier = value["tier"].get<int64_t>();
                names_vehicles[id] = name;
                tier_vehicles[id] = tier;
            }
        }
    }
};
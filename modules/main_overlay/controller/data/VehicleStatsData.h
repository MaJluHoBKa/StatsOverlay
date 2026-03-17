#pragma once

#include <cstdint>
#include <unordered_map>
#include <string>
#include <../external/json.hpp>
#include <fstream>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

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

struct VehicleInfo
{
    int64_t tank_id;
    std::string name;
    int64_t tier;
    std::string type;
    std::string nation;
    std::string status;
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
    std::unordered_map<int64_t, VehicleInfo> vehicles_info;

public:
    VehicleStatsData()
    {
        QByteArray jsonData;

        // 1️⃣ пробуем GitHub
        jsonData = downloadFromGitHub();

        if (!jsonData.isEmpty() && loadVehiclesFromJson(jsonData))
        {
            qDebug() << "[VehicleStats] Loaded vehicles from GitHub";
            return;
        }

        // // 2️⃣ fallback — ресурсы
        // QFile file(":/vehicles_data_stats/resources/vehicles/tanks.json");
        // if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        // {
        //     jsonData = file.readAll();
        //     file.close();

        //     if (loadVehiclesFromJson(jsonData))
        //     {
        //         qDebug() << "[VehicleStats] Loaded vehicles from resources";
        //         return;
        //     }
        // }

        // 3️⃣ всё плохо
        throw std::runtime_error("Failed to load vehicle info from GitHub and resources");
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
                auto nameIt = vehicles_info.find(v.first);
                std::string tankName = (nameIt != vehicles_info.end()) ? nameIt->second.name : "Unknown";
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

    bool loadVehiclesFromJson(const QByteArray &raw)
    {
        json j = json::parse(raw.toStdString(), nullptr, false);
        if (j.is_discarded())
            return false;

        if (j.contains("tanks"))
            j = j["tanks"];

        for (auto &i : j)
        {
            if (!i.contains("id") || !i.contains("name") || !i.contains("tier"))
                continue;

            int64_t id = i["id"].get<int64_t>();

            vehicles_info[id] = {
                id,
                i["name"].get<std::string>(),
                i["tier"].get<int64_t>(),
                i.value("type", ""),
                i.value("nation", ""),
                i.value("status", "default")};
        }

        return !vehicles_info.empty();
    }

    QByteArray downloadFromGitHub()
    {
        QNetworkAccessManager manager;
        QNetworkRequest request(QUrl(
            "https://raw.githubusercontent.com/MaJluHoBKa/vehicles_db_lesta/main/tanks.json"));

        QEventLoop loop;
        QNetworkReply *reply = manager.get(request);
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        QByteArray data;
        if (reply->error() == QNetworkReply::NoError)
            data = reply->readAll();

        reply->deleteLater();
        return data;
    }

    std::string getName(int64_t id)
    {
        if (vehicles_info.count(id))
        {
            return vehicles_info[id].name;
        }
        return std::to_string(id);
    }

    int64_t getTier(int64_t id)
    {
        if (vehicles_info.count(id))
        {
            return vehicles_info[id].tier;
        }
        return 1;
    }

    std::string getType(int64_t id)
    {
        if (vehicles_info.count(id))
        {
            return vehicles_info[id].type;
        }
        return "";
    }

    std::string getNation(int64_t id)
    {
        if (vehicles_info.count(id))
        {
            return vehicles_info[id].nation;
        }
        return "";
    }

    std::string getStatus(int64_t id)
    {
        if (vehicles_info.count(id))
        {
            return vehicles_info[id].status;
        }
        return "default";
    }
};
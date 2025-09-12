#pragma once

#include <string>
#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <../external/httplib.h>
#include <../external/json.hpp>
#include <curl/curl.h>
#include <Windows.h>
#include <QFile>
#include <QStandardPaths>
#include <main_overlay/controller/data/MainStatsData.h>
#include <main_overlay/controller/data/RatingStatsData.h>
// #include "MasteryStats.h"
// #include "OtherStats.h"
#include <main_overlay/controller/data/VehicleStatsData.h>

using json = nlohmann::json;

struct Player
{
    std::string nickname;
    int64_t id;
    int64_t battles;
    int64_t damage;
    int64_t wins;
    int64_t tier;
    std::string tank_name;
    int64_t team;
};

class ApiController
{
private:
    std::string base_url = "https://api.tanki.su/wot";
    std::string application_id = "4d9feb2c53c712fc6a87e026c990dd12";
    std::string token;
    std::string account_id;
    std::string nickname;

    bool isAuth = false;
    bool isFirstMainStats = true;
    bool isFirstRatingStats = true;
    bool isFirstOtherStats = true;
    bool isFirstMasteryStats = true;
    bool isFirstVehiclesStats = true;

    MainStatsData mainStats;
    RatingStatsData ratingStats;
    // MasteryStats masteryStats;
    // OtherStats otherStats;
    VehicleStatsData vehicleStats;

    std::vector<Player> allies;
    std::vector<Player> enemies;
    std::unordered_map<std::string, Player &> db_players;

    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
    {
        ((std::string *)userp)->append((char *)contents, size * nmemb);
        return size * nmemb;
    }

public:
    bool is_auth()
    {
        return this->isAuth;
    }
    bool login()
    {
        std::string redirect_url = "http://localhost:5000/";
        std::string url = this->base_url + "/auth/login/";

        CURL *curl;
        CURLcode res;
        std::string readBuffer;

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
        if (curl)
        {
            std::string full_url = url + "?application_id=" + this->application_id +
                                   "&redirect_uri=" + redirect_url + "&nofollow=1";

            curl_easy_setopt(curl, CURLOPT_URL, full_url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            QFile certFile(":/certs/external/curl/cacert.pem");
            if (certFile.open(QIODevice::ReadOnly))
            {
                QByteArray certData = certFile.readAll();

                QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/cacert.pem";
                QFile tempFile(tempPath);
                if (tempFile.open(QIODevice::WriteOnly))
                {
                    tempFile.write(certData);
                    tempFile.close();
                    curl_easy_setopt(curl, CURLOPT_CAINFO, tempPath.toStdString().c_str());
                }
                else
                {
                    std::cerr << "Cannot create temp cert file" << std::endl;
                }
            }
            else
            {
                std::cerr << "Cannot open cert from resources" << std::endl;
            }

            res = curl_easy_perform(curl);
            if (res != CURLE_OK)
            {
                std::cerr << "CURL failed: " << curl_easy_strerror(res) << " (code " << res << ")" << std::endl;
                curl_easy_cleanup(curl);
                curl_global_cleanup();
                return false;
            }
            curl_easy_cleanup(curl);
        }
        curl_global_cleanup();

        json j;
        try
        {
            j = json::parse(readBuffer);
        }
        catch (...)
        {
            return false;
        }

        std::string location = j["data"]["location"];
        if (location.empty())
        {
            return false;
        }
        ShellExecuteA(NULL, "open", location.c_str(), NULL, NULL, SW_SHOWNORMAL);

        httplib::Server svr;

        svr.Get("/", [&](const httplib::Request &req, httplib::Response &res)
                {
        this->token = req.get_param_value("access_token");
        this->account_id = req.get_param_value("account_id");
        this->nickname = req.get_param_value("nickname");
        auto status = req.get_param_value("status");

        if (status == "ok" && !token.empty()) {
            res.set_redirect("https://lesta.ru/ru/");
            this->isAuth = true;
            svr.stop();
        } else {
            res.set_redirect("https://lesta.ru/ru/");
            svr.stop();
        } });

        svr.listen("0.0.0.0", 5000);
        return true;
    }

    bool logout()
    {
        CURL *curl;
        CURLcode res;
        std::string readBuffer;
        bool success = false;

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();

        if (curl)
        {
            std::string base = "https://api.tanki.su/wot/auth/logout/";
            std::string postFields = "application_id=" + this->application_id +
                                     "&access_token=" + this->token;
            curl_easy_setopt(curl, CURLOPT_URL, base.c_str());
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            QFile certFile(":/certs/external/curl/cacert.pem");
            if (certFile.open(QIODevice::ReadOnly))
            {
                QByteArray certData = certFile.readAll();

                QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/cacert.pem";
                QFile tempFile(tempPath);
                if (tempFile.open(QIODevice::WriteOnly))
                {
                    tempFile.write(certData);
                    tempFile.close();
                    curl_easy_setopt(curl, CURLOPT_CAINFO, tempPath.toStdString().c_str());
                }
                else
                {
                    std::cerr << "Cannot create temp cert file" << std::endl;
                }
            }
            else
            {
                std::cerr << "Cannot open cert from resources" << std::endl;
            }

            res = curl_easy_perform(curl);

            if (res == CURLE_OK)
            {
                json j = json::parse(readBuffer, nullptr, false);
                if (j["status"] == "ok")
                {
                    this->token.clear();
                    this->account_id.clear();
                    this->nickname.clear();

                    this->isFirstMainStats = true;
                    this->isFirstRatingStats = true;
                    this->isFirstMasteryStats = true;
                    this->isFirstOtherStats = true;
                    this->isAuth = false;
                    success = true;
                }
            }
        }
        if (curl)
            curl_easy_cleanup(curl);
        curl_global_cleanup();

        return success;
    }

    bool prolongate()
    {
        CURL *curl;
        CURLcode res;
        std::string readBuffer;
        bool success = false;

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();

        if (curl)
        {
            std::string base = "https://api.tanki.su/wot/auth/prolongate/";
            std::string postFields = "application_id=" + this->application_id +
                                     "&access_token=" + this->token;
            curl_easy_setopt(curl, CURLOPT_URL, base.c_str());
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            QFile certFile(":/certs/external/curl/cacert.pem");
            if (certFile.open(QIODevice::ReadOnly))
            {
                QByteArray certData = certFile.readAll();

                QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/cacert.pem";
                QFile tempFile(tempPath);
                if (tempFile.open(QIODevice::WriteOnly))
                {
                    tempFile.write(certData);
                    tempFile.close();
                    curl_easy_setopt(curl, CURLOPT_CAINFO, tempPath.toStdString().c_str());
                }
                else
                {
                    std::cerr << "Cannot create temp cert file" << std::endl;
                }
            }
            else
            {
                std::cerr << "Cannot open cert from resources" << std::endl;
            }

            res = curl_easy_perform(curl);

            if (res == CURLE_OK)
            {
                json j = json::parse(readBuffer, nullptr, false);
                if (j["status"] == "ok")
                {
                    this->token = j["data"]["access_token"];
                    success = true;
                }
            }
        }
        if (curl)
            curl_easy_cleanup(curl);
        curl_global_cleanup();

        return success;
    }

    bool update_main_stats()
    {
        CURL *curl;
        CURLcode res;
        std::string readBuffer;
        bool success = false;

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();

        if (curl)
        {
            std::string base = "https://papi.tanksblitz.ru/wotb/account/info/";
            std::string url = base +
                              "?application_id=" + this->application_id +
                              "&access_token=" + this->token +
                              "&account_id=" + this->account_id;
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            QFile certFile(":/certs/external/curl/cacert.pem");
            if (certFile.open(QIODevice::ReadOnly))
            {
                QByteArray certData = certFile.readAll();

                QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/cacert.pem";
                QFile tempFile(tempPath);
                if (tempFile.open(QIODevice::WriteOnly))
                {
                    tempFile.write(certData);
                    tempFile.close();
                    curl_easy_setopt(curl, CURLOPT_CAINFO, tempPath.toStdString().c_str());
                }
                else
                {
                    std::cerr << "Cannot create temp cert file" << std::endl;
                }
            }
            else
            {
                std::cerr << "Cannot open cert from resources" << std::endl;
            }

            res = curl_easy_perform(curl);

            if (res == CURLE_OK)
            {
                json j = json::parse(readBuffer, nullptr, false);
                if (j["status"] == "ok")
                {
                    auto &data = j["data"][this->account_id];
                    StatsData statsData;
                    statsData.credits = data["private"]["credits"].get<int64_t>();
                    statsData.gold = data["private"]["gold"].get<int64_t>();
                    statsData.exp_battle = data["statistics"]["all"]["xp"].get<int64_t>();
                    statsData.exp_free = data["private"]["free_xp"].get<int64_t>();
                    statsData.battles = data["statistics"]["all"]["battles"].get<int64_t>();
                    statsData.wins = data["statistics"]["all"]["wins"].get<int64_t>();
                    statsData.losses = data["statistics"]["all"]["losses"].get<int64_t>();
                    statsData.totalDamage = data["statistics"]["all"]["damage_dealt"].get<int64_t>();

                    if (this->isFirstMainStats)
                    {
                        this->mainStats.initialStats(statsData);
                        this->isFirstMainStats = false;
                    }
                    else
                    {
                        this->mainStats.updateStats(statsData);
                    }
                    // std::cout << "StatsData:" << std::endl;
                    // std::cout << "  Credits: " << mainStats.getCredits() << std::endl;
                    // std::cout << "  Gold: " << mainStats.getGold() << std::endl;
                    // std::cout << "  Exp battle: " << mainStats.getAvgExp() << std::endl;
                    // std::cout << "  Free exp: " << mainStats.getExpFree() << std::endl;
                    // std::cout << "  Battles: " << mainStats.getBattles() << std::endl;
                    // std::cout << "  Percent wins: " << mainStats.getPercentWins() << std::endl;
                    // std::cout << "  Losses: " << mainStats.getLosses() << std::endl;
                    // std::cout << "  Avg damage: " << mainStats.getAvgDamage() << std::endl;
                    success = true;
                }
            }
        }
        if (curl)
            curl_easy_cleanup(curl);
        curl_global_cleanup();

        return success;
    }

    bool update_rating_stats()
    {
        CURL *curl;
        CURLcode res;
        std::string readBuffer;
        bool success = false;

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();

        if (curl)
        {
            std::string base = "https://papi.tanksblitz.ru/wotb/account/info/";
            std::string url = base +
                              "?application_id=" + this->application_id +
                              "&access_token=" + this->token +
                              "&account_id=" + this->account_id +
                              "&extra=statistics.rating";
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            QFile certFile(":/certs/external/curl/cacert.pem");
            if (certFile.open(QIODevice::ReadOnly))
            {
                QByteArray certData = certFile.readAll();

                QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/cacert.pem";
                QFile tempFile(tempPath);
                if (tempFile.open(QIODevice::WriteOnly))
                {
                    tempFile.write(certData);
                    tempFile.close();
                    curl_easy_setopt(curl, CURLOPT_CAINFO, tempPath.toStdString().c_str());
                }
                else
                {
                    std::cerr << "Cannot create temp cert file" << std::endl;
                }
            }
            else
            {
                std::cerr << "Cannot open cert from resources" << std::endl;
            }

            res = curl_easy_perform(curl);

            if (res == CURLE_OK)
            {
                json j = json::parse(readBuffer, nullptr, false);
                if (j["status"] == "ok")
                {
                    auto &data = j["data"][this->account_id];
                    RatingData ratingData;
                    ratingData.mm_rating = data["statistics"]["rating"]["mm_rating"].get<double>();
                    ratingData.calib_battles = data["statistics"]["rating"]["calibration_battles_left"].get<int64_t>();
                    ratingData.battles = data["statistics"]["rating"]["battles"].get<int64_t>();
                    ratingData.exp_battle = data["statistics"]["rating"]["xp"].get<int64_t>();
                    ratingData.wins = data["statistics"]["rating"]["wins"].get<int64_t>();
                    ratingData.losses = data["statistics"]["rating"]["losses"].get<int64_t>();
                    ratingData.totalDamage = data["statistics"]["rating"]["damage_dealt"].get<int64_t>();

                    if (this->isFirstRatingStats)
                    {
                        this->ratingStats.initialStats(ratingData);
                        this->isFirstRatingStats = false;
                    }
                    else
                    {
                        this->ratingStats.updateStats(ratingData);
                    }
                    // std::cout << "ratingData:" << std::endl;
                    // std::cout << "  mm_rating: " << ratingStats.getRating() << std::endl;
                    // std::cout << "  progress: " << ratingStats.getDiffRating() << std::endl;
                    // std::cout << "  Calib battles: " << ratingStats.getCalibBattles() << std::endl;
                    // std::cout << "  Battles: " << ratingStats.getBattles() << std::endl;
                    // std::cout << "  Exp battle: " << ratingStats.getAvgExp() << std::endl;
                    // std::cout << "  Wins: " << ratingStats.getPercentWins() << std::endl;
                    // std::cout << "  Total damage: " << ratingStats.getAvgDamage() << std::endl;
                    success = true;
                }
            }
        }
        if (curl)
            curl_easy_cleanup(curl);
        curl_global_cleanup();

        return success;
    }

    // bool update_mastery_stats()
    // {
    //     CURL *curl;
    //     CURLcode res;
    //     std::string readBuffer;
    //     bool success = false;

    //     curl_global_init(CURL_GLOBAL_DEFAULT);
    //     curl = curl_easy_init();

    //     if (curl)
    //     {
    //         std::string base = "https://papi.tanksblitz.ru/wotb/account/achievements/";
    //         std::string url = base +
    //                           "?application_id=" + this->application_id +
    //                           "&account_id=" + this->account_id +
    //                           "&fields=achievements";
    //         curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    //         curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    //         curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    //         res = curl_easy_perform(curl);

    //         if (res == CURLE_OK)
    //         {
    //             json j = json::parse(readBuffer, nullptr, false);
    //             if (j["status"] == "ok")
    //             {
    //                 auto &data = j["data"][this->account_id];
    //                 MasteryData masteryData;
    //                 masteryData.mastery = data["achievements"]["markOfMastery"].get<int64_t>();
    //                 masteryData.mastery_1 = data["achievements"]["markOfMasteryI"].get<int64_t>();
    //                 masteryData.mastery_2 = data["achievements"]["markOfMasteryII"].get<int64_t>();
    //                 masteryData.mastery_3 = data["achievements"]["markOfMasteryIII"].get<int64_t>();

    //                 if (this->isFirstMasteryStats)
    //                 {
    //                     this->masteryStats.initialStats(masteryData);
    //                     this->isFirstMasteryStats = false;
    //                 }
    //                 else
    //                 {
    //                     this->masteryStats.updateStats(masteryData);
    //                 }
    //                 masteryData = masteryStats.getCurrentData();
    //                 // std::cout << "masteryData:" << std::endl;
    //                 // std::cout << "  markOfMastery: " << masteryData.mastery << std::endl;
    //                 // std::cout << "  markOfMasteryI: " << masteryData.mastery_1 << std::endl;
    //                 // std::cout << "  markOfMasteryII: " << masteryData.mastery_2 << std::endl;
    //                 // std::cout << "  markOfMasteryIII: " << masteryData.mastery_3 << std::endl;
    //                 success = true;
    //             }
    //         }
    //     }
    //     if (curl)
    //         curl_easy_cleanup(curl);
    //     curl_global_cleanup();

    //     return success;
    // }

    // bool update_other_stats()
    // {
    //     CURL *curl;
    //     CURLcode res;
    //     std::string readBuffer;
    //     bool success = false;

    //     curl_global_init(CURL_GLOBAL_DEFAULT);
    //     curl = curl_easy_init();

    //     if (curl)
    //     {
    //         std::string base = "https://papi.tanksblitz.ru/wotb/account/info/";
    //         std::string url = base +
    //                           "?application_id=" + this->application_id +
    //                           "&access_token=" + this->token +
    //                           "&account_id=" + this->account_id +
    //                           "&extra=statistics.rating";
    //         curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    //         curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    //         curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    //         res = curl_easy_perform(curl);

    //         if (res == CURLE_OK)
    //         {
    //             json j = json::parse(readBuffer, nullptr, false);
    //             if (j["status"] == "ok")
    //             {
    //                 auto &data = j["data"][this->account_id];
    //                 OtherData otherData;
    //                 otherData.battles = data["statistics"]["all"]["battles"].get<int64_t>() + data["statistics"]["rating"]["battles"].get<int64_t>();
    //                 otherData.hits = data["statistics"]["all"]["hits"].get<int64_t>() + data["statistics"]["rating"]["hits"].get<int64_t>();
    //                 otherData.shots = data["statistics"]["all"]["shots"].get<int64_t>() + data["statistics"]["rating"]["shots"].get<int64_t>();
    //                 otherData.survived = data["statistics"]["all"]["survived_battles"].get<int64_t>() + data["statistics"]["rating"]["survived_battles"].get<int64_t>();
    //                 otherData.frags = data["statistics"]["all"]["frags"].get<int64_t>() + data["statistics"]["rating"]["frags"].get<int64_t>();
    //                 otherData.receiverDamage = data["statistics"]["all"]["damage_received"].get<int64_t>() + data["statistics"]["rating"]["damage_received"].get<int64_t>();
    //                 otherData.totalDamage = data["statistics"]["all"]["damage_dealt"].get<int64_t>() + data["statistics"]["rating"]["damage_dealt"].get<int64_t>();
    //                 otherData.lifeTime = data["private"]["battle_life_time"].get<int64_t>();

    //                 if (this->isFirstOtherStats)
    //                 {
    //                     this->otherStats.initialStats(otherData);
    //                     this->isFirstOtherStats = false;
    //                 }
    //                 else
    //                 {
    //                     this->otherStats.updateStats(otherData);
    //                 }
    //                 // std::cout << "otherData:" << std::endl;
    //                 // std::cout << "  Percent Hits: " << otherStats.getPercentHits() << std::endl;
    //                 // std::cout << "  Percent Survived: " << otherStats.getPercentSurvived() << std::endl;
    //                 // std::cout << "  Life Time: " << otherStats.getLifeTime() << std::endl;
    //                 // std::cout << "  K Damage: " << otherStats.getDamageK() << std::endl;
    //                 // std::cout << "  K Frags: " << otherStats.getFragsK() << std::endl;
    //                 success = true;
    //             }
    //         }
    //     }
    //     if (curl)
    //         curl_easy_cleanup(curl);
    //     curl_global_cleanup();
    //     return success;
    // }

    bool get_players_stats()
    {
        CURL *curl;
        CURLcode res;
        std::string readBuffer;
        bool success = false;

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();

        if (curl)
        {

            std::string base = "https://papi.tanksblitz.ru/wotb/account/info/";
            std::string url = base +
                              "?application_id=" + this->application_id +
                              "&account_id=" + join() +
                              "&fields=statistics.all.battles,statistics.all.wins,statistics.all.damage_dealt";
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            QFile certFile(":/certs/external/curl/cacert.pem");
            if (certFile.open(QIODevice::ReadOnly))
            {
                QByteArray certData = certFile.readAll();

                QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/cacert.pem";
                QFile tempFile(tempPath);
                if (tempFile.open(QIODevice::WriteOnly))
                {
                    tempFile.write(certData);
                    tempFile.close();
                    curl_easy_setopt(curl, CURLOPT_CAINFO, tempPath.toStdString().c_str());
                }
                else
                {
                    std::cerr << "Cannot create temp cert file" << std::endl;
                }
            }
            else
            {
                std::cerr << "Cannot open cert from resources" << std::endl;
            }

            res = curl_easy_perform(curl);

            if (res == CURLE_OK)
            {
                json j = json::parse(readBuffer, nullptr, false);
                if (j["status"] == "ok")
                {
                    auto &data = j["data"];
                    for (auto it = data.begin(); it != data.end(); ++it)
                    {
                        std::string player_id = it.key();
                        auto stats = it.value()["statistics"]["all"];

                        auto db_it = db_players.find(player_id);
                        if (db_it != db_players.end())
                        {
                            Player &p = db_it->second;
                            p.battles = stats.value("battles", 0);
                            p.damage = stats.value("damage_dealt", 0);
                            p.wins = stats.value("wins", 0);
                        }
                    }
                    success = true;
                }
            }
        }
        if (curl)
            curl_easy_cleanup(curl);
        curl_global_cleanup();

        return success;
    }

    bool update_vehicles_stats()
    {
        if (this->isFirstVehiclesStats)
        {
            if (!get_vehicles_names())
            {
                return false;
            }
        }
        CURL *curl;
        CURLcode res;
        std::string readBuffer;
        bool success = false;

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();

        if (curl)
        {
            std::string base = "https://papi.tanksblitz.ru/wotb/tanks/stats/";
            std::string url = base +
                              "?application_id=" + this->application_id +
                              "&access_token=" + this->token +
                              "&account_id=" + this->account_id;
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            QFile certFile(":/certs/external/curl/cacert.pem");
            if (certFile.open(QIODevice::ReadOnly))
            {
                QByteArray certData = certFile.readAll();

                QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/cacert.pem";
                QFile tempFile(tempPath);
                if (tempFile.open(QIODevice::WriteOnly))
                {
                    tempFile.write(certData);
                    tempFile.close();
                    curl_easy_setopt(curl, CURLOPT_CAINFO, tempPath.toStdString().c_str());
                }
                else
                {
                    std::cerr << "Cannot create temp cert file" << std::endl;
                }
            }
            else
            {
                std::cerr << "Cannot open cert from resources" << std::endl;
            }

            res = curl_easy_perform(curl);

            if (res == CURLE_OK)
            {
                json j = json::parse(readBuffer, nullptr, false);
                if (j["status"] == "ok")
                {
                    auto &data = j["data"][this->account_id];
                    if (this->isFirstVehiclesStats)
                    {
                        this->vehicleStats.initialStats(data);
                        this->isFirstVehiclesStats = false;
                    }
                    else
                    {
                        this->vehicleStats.updateStats(data);
                    }
                    success = true;
                }
            }
        }
        if (curl)
            curl_easy_cleanup(curl);
        curl_global_cleanup();

        return success;
    }

    bool get_vehicles_names()
    {
        CURL *curl;
        CURLcode res;
        std::string readBuffer;
        bool success = false;

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();

        if (curl)
        {
            std::string base = "https://papi.tanksblitz.ru/wotb/encyclopedia/vehicles/";
            std::string url = base +
                              "?application_id=" + this->application_id +
                              "&fields=name,tank_id,tier";
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            QFile certFile(":/certs/external/curl/cacert.pem");
            if (certFile.open(QIODevice::ReadOnly))
            {
                QByteArray certData = certFile.readAll();

                QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/cacert.pem";
                QFile tempFile(tempPath);
                if (tempFile.open(QIODevice::WriteOnly))
                {
                    tempFile.write(certData);
                    tempFile.close();
                    curl_easy_setopt(curl, CURLOPT_CAINFO, tempPath.toStdString().c_str());
                }
                else
                {
                    std::cerr << "Cannot create temp cert file" << std::endl;
                }
            }
            else
            {
                std::cerr << "Cannot open cert from resources" << std::endl;
            }

            res = curl_easy_perform(curl);

            if (res == CURLE_OK)
            {
                json j = json::parse(readBuffer, nullptr, false);
                if (j["status"] == "ok")
                {
                    auto &data = j["data"];
                    vehicleStats.setNames(data);
                    success = true;
                }
            }
        }
        if (curl)
            curl_easy_cleanup(curl);
        curl_global_cleanup();

        return success;
    }

    const VehicleData *get_updated_vehicles() const
    {
        return vehicleStats.getUpdatedVehicle();
    }

    std::string getVehicleName(int64_t id)
    {
        return vehicleStats.getName(id);
    }

    MainStatsData getMainStats() const
    {
        return this->mainStats;
    }

    RatingStatsData getRatingStats() const
    {
        return this->ratingStats;
    }

    // MasteryStats getMasteryStats() const
    // {
    //     return this->masteryStats;
    // }

    // OtherStats getOtherStats() const
    // {
    //     return this->otherStats;
    // }

    // VehicleStats getVehicleStats() const
    // {
    //     return this->vehicleStats;
    // }

    std::string join()
    {
        std::string str = "";
        if (this->allies.size() + this->enemies.size() == 14)
        {
            str += std::to_string(this->allies[0].id);
            for (int i = 1; i < this->allies.size(); i++)
            {
                str += ",";
                str += std::to_string(this->allies[i].id);
            }
            for (int i = 0; i < this->enemies.size(); i++)
            {
                str += ",";
                str += std::to_string(this->enemies[i].id);
            }
        }
        return str;
    }

    void setPlayers(const std::vector<Player> allies, const std::vector<Player> enemies)
    {
        this->allies = allies;
        this->enemies = enemies;

        this->db_players.clear();

        for (auto &p : this->allies)
        {
            db_players.emplace(std::to_string(p.id), p);
        }

        for (auto &p : this->enemies)
        {
            db_players.emplace(std::to_string(p.id), p);
        }
    }

    static void toLowerInplace(std::string &s)
    {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c)
                       { return std::tolower(c); });
    }

    void sortPlayers(std::vector<Player> &players)
    {
        std::sort(players.begin(), players.end(), [](const Player &a, const Player &b)
                  {
                      if (a.tier != b.tier)
                          return a.tier > b.tier;

                      std::string an = a.tank_name, bn = b.tank_name;
                      toLowerInplace(an);
                      toLowerInplace(bn);
                      if (an != bn)
                          return an < bn;

                      return false; });
    }

    std::vector<Player> getSortedAllies() const
    {
        return this->allies;
    }

    std::vector<Player> getSortedEnemies() const
    {
        return this->enemies;
    }

    std::string getNickname() const
    {
        return this->nickname;
    }
};
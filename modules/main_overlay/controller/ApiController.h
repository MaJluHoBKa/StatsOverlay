#pragma once

#include <string>
#include <algorithm>
#include <filesystem>
#include <cctype>
#include <unordered_map>
#include <../external/httplib.h>
#include <../external/json.hpp>
#include <curl/curl.h>
#include <Windows.h>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDate>
#include <QDateTime>
#include <QTextStream>
#include <QSysInfo>
#include <main_overlay/controller/data/MainStatsData.h>
#include <main_overlay/controller/data/RatingStatsData.h>
#include <main_overlay/controller/data/MasteryStatsData.h>
#include <main_overlay/controller/data/OtherStatsData.h>
#include <main_overlay/controller/data/VehicleStatsData.h>
#include <main_overlay/controller/ParseReplay.h>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

using json = nlohmann::json;

struct Player
{
    std::string nickname;
    int64_t id;
    int64_t tank_id;
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

    QFile m_logFile;

    bool isAuth = false;
    bool isFirstMainStats = true;
    bool isFirstRatingStats = true;
    bool isFirstOtherStats = true;
    bool isFirstMasteryStats = true;
    bool isFirstVehiclesStats = true;
    bool isGunMark = false;
    bool isTankLock = false;

    MainStatsData mainStats;
    RatingStatsData ratingStats;
    MasteryStatsData masteryStats;
    OtherStatsData otherStats;
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
        log("Запуск процедуры авторизации", 4, QDateTime::currentDateTime());
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

            QString exeDir = QCoreApplication::applicationDirPath();
            QString certPath = exeDir + "/certificate/cacert.pem";
            std::string certPathUtf8 = certPath.toUtf8().constData();

            if (QFile::exists(certPath))
            {
                curl_easy_setopt(curl, CURLOPT_CAINFO, certPathUtf8.c_str());
            }
            else
            {
                log("Файл сертификата не найден: " + certPath, 0, QDateTime::currentDateTime());
            }

            res = curl_easy_perform(curl);
            if (res != CURLE_OK)
            {
                QString err = "Ошибка CURL: " + QString::fromStdString(curl_easy_strerror(res)) +
                              " (код " + QString::number(res) + ")";
                log(err, 4, QDateTime::currentDateTime());
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
            log("Ошибка парсинга JSON ответа при авторизации", 4, QDateTime::currentDateTime());
            return false;
        }

        std::string location = j["data"]["location"];
        if (location.empty())
        {
            log("Не получен URL для авторизации", 4, QDateTime::currentDateTime());
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
            log("Авторизация успешна: " + QString::fromStdString(this->nickname), 4, QDateTime::currentDateTime());
            res.set_redirect("https://lesta.ru/ru/");
            this->isAuth = true;
            svr.stop();
        } else {
            log("Ошибка авторизации", 4, QDateTime::currentDateTime());
            res.set_redirect("https://lesta.ru/ru/");
            svr.stop();
        } });

        svr.listen("0.0.0.0", 5000);

        log("Авторизация завершена успешно", 4, QDateTime::currentDateTime());
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

            QString exeDir = QCoreApplication::applicationDirPath();
            QString certPath = exeDir + "/certificate/cacert.pem";
            std::string certPathUtf8 = certPath.toUtf8().constData();

            if (QFile::exists(certPath))
            {
                curl_easy_setopt(curl, CURLOPT_CAINFO, certPathUtf8.c_str());
            }
            else
            {
                log("Файл сертификата не найден: " + certPath, 0, QDateTime::currentDateTime());
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

            QString exeDir = QCoreApplication::applicationDirPath();
            QString certPath = exeDir + "/certificate/cacert.pem";
            std::string certPathUtf8 = certPath.toUtf8().constData();

            if (QFile::exists(certPath))
            {
                curl_easy_setopt(curl, CURLOPT_CAINFO, certPathUtf8.c_str());
            }
            else
            {
                log("Файл сертификата не найден: " + certPath, 0, QDateTime::currentDateTime());
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

            QString exeDir = QCoreApplication::applicationDirPath();
            QString certPath = exeDir + "/certificate/cacert.pem";
            std::string certPathUtf8 = certPath.toUtf8().constData();

            if (QFile::exists(certPath))
            {
                curl_easy_setopt(curl, CURLOPT_CAINFO, certPathUtf8.c_str());
            }
            else
            {
                log("Файл сертификата не найден: " + certPath, 0, QDateTime::currentDateTime());
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

            QString exeDir = QCoreApplication::applicationDirPath();
            QString certPath = exeDir + "/certificate/cacert.pem";
            std::string certPathUtf8 = certPath.toUtf8().constData();

            if (QFile::exists(certPath))
            {
                curl_easy_setopt(curl, CURLOPT_CAINFO, certPathUtf8.c_str());
            }
            else
            {
                log("Файл сертификата не найден: " + certPath, 0, QDateTime::currentDateTime());
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

    bool update_mastery_stats()
    {
        CURL *curl;
        CURLcode res;
        std::string readBuffer;
        bool success = false;

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();

        if (curl)
        {
            std::string base = "https://papi.tanksblitz.ru/wotb/account/achievements/";
            std::string url = base +
                              "?application_id=" + this->application_id +
                              "&account_id=" + this->account_id +
                              "&fields=achievements";
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            QString exeDir = QCoreApplication::applicationDirPath();
            QString certPath = exeDir + "/certificate/cacert.pem";
            std::string certPathUtf8 = certPath.toUtf8().constData();

            if (QFile::exists(certPath))
            {
                curl_easy_setopt(curl, CURLOPT_CAINFO, certPathUtf8.c_str());
            }
            else
            {
                log("Файл сертификата не найден: " + certPath, 0, QDateTime::currentDateTime());
            }

            res = curl_easy_perform(curl);

            if (res == CURLE_OK)
            {
                json j = json::parse(readBuffer, nullptr, false);
                if (j["status"] == "ok")
                {
                    auto &data = j["data"][this->account_id];
                    MasteryData masteryData;
                    masteryData.mastery = data["achievements"]["markOfMastery"].get<int64_t>();
                    masteryData.mastery_1 = data["achievements"]["markOfMasteryI"].get<int64_t>();
                    masteryData.mastery_2 = data["achievements"]["markOfMasteryII"].get<int64_t>();
                    masteryData.mastery_3 = data["achievements"]["markOfMasteryIII"].get<int64_t>();

                    if (this->isFirstMasteryStats)
                    {
                        this->masteryStats.initialStats(masteryData);
                        this->isFirstMasteryStats = false;
                    }
                    else
                    {
                        this->masteryStats.updateStats(masteryData);
                    }
                    masteryData = masteryStats.getCurrentData();
                    // std::cout << "masteryData:" << std::endl;
                    // std::cout << "  markOfMastery: " << masteryData.mastery << std::endl;
                    // std::cout << "  markOfMasteryI: " << masteryData.mastery_1 << std::endl;
                    // std::cout << "  markOfMasteryII: " << masteryData.mastery_2 << std::endl;
                    // std::cout << "  markOfMasteryIII: " << masteryData.mastery_3 << std::endl;
                    success = true;
                }
            }
        }
        if (curl)
            curl_easy_cleanup(curl);
        curl_global_cleanup();

        return success;
    }

    bool update_other_stats()
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

            QString exeDir = QCoreApplication::applicationDirPath();
            QString certPath = exeDir + "/certificate/cacert.pem";
            std::string certPathUtf8 = certPath.toUtf8().constData();

            if (QFile::exists(certPath))
            {
                curl_easy_setopt(curl, CURLOPT_CAINFO, certPathUtf8.c_str());
            }
            else
            {
                log("Файл сертификата не найден: " + certPath, 0, QDateTime::currentDateTime());
            }

            res = curl_easy_perform(curl);

            if (res == CURLE_OK)
            {
                json j = json::parse(readBuffer, nullptr, false);
                if (j["status"] == "ok")
                {
                    auto &data = j["data"][this->account_id];
                    OtherData otherData;
                    otherData.battles = data["statistics"]["all"]["battles"].get<int64_t>() + data["statistics"]["rating"]["battles"].get<int64_t>();
                    otherData.hits = data["statistics"]["all"]["hits"].get<int64_t>() + data["statistics"]["rating"]["hits"].get<int64_t>();
                    otherData.shots = data["statistics"]["all"]["shots"].get<int64_t>() + data["statistics"]["rating"]["shots"].get<int64_t>();
                    otherData.survived = data["statistics"]["all"]["survived_battles"].get<int64_t>() + data["statistics"]["rating"]["survived_battles"].get<int64_t>();
                    otherData.frags = data["statistics"]["all"]["frags"].get<int64_t>() + data["statistics"]["rating"]["frags"].get<int64_t>();
                    otherData.receiverDamage = data["statistics"]["all"]["damage_received"].get<int64_t>() + data["statistics"]["rating"]["damage_received"].get<int64_t>();
                    otherData.totalDamage = data["statistics"]["all"]["damage_dealt"].get<int64_t>() + data["statistics"]["rating"]["damage_dealt"].get<int64_t>();
                    otherData.lifeTime = data["private"]["battle_life_time"].get<int64_t>();

                    if (this->isFirstOtherStats)
                    {
                        this->otherStats.initialStats(otherData);
                        this->isFirstOtherStats = false;
                    }
                    else
                    {
                        this->otherStats.updateStats(otherData);
                    }
                    // std::cout << "otherData:" << std::endl;
                    // std::cout << "  Percent Hits: " << otherStats.getPercentHits() << std::endl;
                    // std::cout << "  Percent Survived: " << otherStats.getPercentSurvived() << std::endl;
                    // std::cout << "  Life Time: " << otherStats.getLifeTime() << std::endl;
                    // std::cout << "  K Damage: " << otherStats.getDamageK() << std::endl;
                    // std::cout << "  K Frags: " << otherStats.getFragsK() << std::endl;
                    success = true;
                }
            }
        }
        if (curl)
            curl_easy_cleanup(curl);
        curl_global_cleanup();
        return success;
    }

    bool get_players_stats()
    {
        setPlayers(this->allies, this->enemies);
        CURL *curl;
        CURLcode res;
        std::string readBuffer;
        bool success = false;

        log("Запрос статистики игроков запущен", 3, QDateTime::currentDateTime());

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

            QString exeDir = QCoreApplication::applicationDirPath();
            QString certPath = exeDir + "/certificate/cacert.pem";
            std::string certPathUtf8 = certPath.toUtf8().constData();

            if (QFile::exists(certPath))
            {
                curl_easy_setopt(curl, CURLOPT_CAINFO, certPathUtf8.c_str());
            }
            else
            {
                log("Файл сертификата не найден: " + certPath, 0, QDateTime::currentDateTime());
            }

            res = curl_easy_perform(curl);

            if (res == CURLE_OK)
            {
                json j = json::parse(readBuffer, nullptr, false);
                if (j["status"] == "ok")
                {
                    log("Статистика игроков успешно получена", 3, QDateTime::currentDateTime());

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
                            if (is_auth())
                            {
                                p.tank_name = vehicleStats.getName(p.tank_id);
                            }
                            else
                            {
                                p.tank_name = std::to_string(p.tank_id);
                            }
                        }
                    }
                    sortPlayers(this->allies);
                    sortPlayers(this->enemies);
                    success = true;
                }
                else
                {
                    log("API вернуло ошибку: " + QString::fromStdString(readBuffer),
                        3, QDateTime::currentDateTime());
                }
            }
            else
            {
                log("Ошибка CURL: " + QString::fromStdString(curl_easy_strerror(res)),
                    3, QDateTime::currentDateTime());
            }
        }
        if (curl)
            curl_easy_cleanup(curl);
        curl_global_cleanup();

        return success;
    }

    bool loadReplayPlayers()
    {
        this->allies.clear();
        this->enemies.clear();
        this->db_players.clear();

        log("Поиск реплея начат", 3, QDateTime::currentDateTime());

        // Базовый путь к Documents/TanksBlitz/replays
        QString documents = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        QDir replaysDir(documents + "/TanksBlitz/replays/");

        if (!replaysDir.exists())
        {
            log("Папка с реплеями не найдена", 3, QDateTime::currentDateTime());
            return false;
        }

        // Ищем папку, начинающуюся с recording
        QStringList recordings = replaysDir.entryList(QStringList() << "recording*", QDir::Dirs | QDir::NoDotAndDotDot);

        if (recordings.isEmpty())
        {
            log("Нет подходящих папок recording*", 3, QDateTime::currentDateTime());
            return false;
        }

        // Берём первую подходящую
        QString recordingPath = replaysDir.absoluteFilePath(recordings.first());
        QString replayFile = recordingPath + "/data.replay";

        QFile file(replayFile);
        if (!file.exists())
        {
            log("Файл data.replay не найден: " + replayFile, 3, QDateTime::currentDateTime());
            return false;
        }

        log("Найден реплей: " + replayFile, 3, QDateTime::currentDateTime());

        // Вызываем наш парсер
        Sleep(10000);
        ReplayParser parser(&this->m_logFile);
        std::vector<PlayerRaw> raw = parser.parseReplayFile(std::filesystem::path(replayFile.toStdWString()));

        if (raw.size() == 14)
        {
            // std::cout << "here 0";
            int allies = 0;
            for (int i = 0; i < raw.size(); i++)
            {
                // std::cout << "here 1";
                if (raw[i].nickname == this->nickname)
                {
                    allies = raw[i].team_number;
                }
            }
            if (allies != 0)
            {
                // std::cout << "here 2";
                for (int i = 0; i < raw.size(); i++)
                {
                    // std::cout << "here 3";
                    Player player;
                    player.nickname = raw[i].nickname;
                    player.id = raw[i].account_id;
                    player.tank_id = raw[i].tank_id;
                    player.team = raw[i].team_number;
                    player.tier = vehicleStats.getTier(raw[i].tank_id);
                    if (raw[i].team_number == allies)
                    {
                        this->allies.push_back(player);
                    }
                    else
                    {
                        this->enemies.push_back(player);
                    }
                }
                log("Игроки успешно загружены из реплея", 3, QDateTime::currentDateTime());
                return true;
            }
            else
            {
                log("Не удалось определить команду игрока", 3, QDateTime::currentDateTime());
                return false;
            }
        }
        else
        {
            log("Некорректное количество игроков в реплее: " + QString::number(raw.size()),
                3, QDateTime::currentDateTime());
            return false;
        }
    }

    bool update_vehicles_stats()
    {
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

            QString exeDir = QCoreApplication::applicationDirPath();
            QString certPath = exeDir + "/certificate/cacert.pem";
            std::string certPathUtf8 = certPath.toUtf8().constData();

            if (QFile::exists(certPath))
            {
                curl_easy_setopt(curl, CURLOPT_CAINFO, certPathUtf8.c_str());
            }
            else
            {
                log("Файл сертификата не найден: " + certPath, 0, QDateTime::currentDateTime());
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

    const VehicleData *get_updated_vehicles() const
    {
        return vehicleStats.getUpdatedVehicle();
    }

    std::string getVehicleName(int64_t id)
    {
        return vehicleStats.getName(id);
    }

    int64_t getVehicleTier(int64_t id)
    {
        return vehicleStats.getTier(id);
    }

    std::string getVehicleType(int64_t id)
    {
        return vehicleStats.getType(id);
    }

    std::string getVehicleNation(int64_t id)
    {
        return vehicleStats.getNation(id);
    }

    std::string getVehicleStatus(int64_t id)
    {
        return vehicleStats.getStatus(id);
    }

    MainStatsData getMainStats() const
    {
        return this->mainStats;
    }

    RatingStatsData getRatingStats() const
    {
        return this->ratingStats;
    }

    MasteryStatsData getMasteryStats() const
    {
        return this->masteryStats;
    }

    OtherStatsData getOtherStats() const
    {
        return this->otherStats;
    }

    bool isMark() const
    {
        return this->isGunMark;
    }

    void setMark(bool mark)
    {
        this->isGunMark = mark;
    }

    bool isLock() const
    {
        return this->isTankLock;
    }

    void setTankLock(bool mark)
    {
        this->isTankLock = mark;
    }

    // VehicleStats getVehicleStats() const
    // {
    //     return this->vehicleStats;
    // }

    void reset()
    {
        this->isFirstMainStats = true;
        this->isFirstRatingStats = true;
        this->isFirstMasteryStats = true;
        this->isFirstOtherStats = true;
        // this->isFirstVehiclesStats = true;
    }

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

    static bool tankNameCompare(const std::string &a, const std::string &b)
    {
        size_t n = std::min(a.size(), b.size());
        for (size_t i = 0; i < n; ++i)
        {
            unsigned char ca = static_cast<unsigned char>(a[i]);
            unsigned char cb = static_cast<unsigned char>(b[i]);

            bool aDigit = (ca >= '0' && ca <= '9');
            bool bDigit = (cb >= '0' && cb <= '9');

            if (aDigit != bDigit)
                return aDigit; // цифра "меньше" всех

            bool aLatin = (ca >= 'a' && ca <= 'z') || (ca >= 'A' && ca <= 'Z');
            bool bLatin = (cb >= 'a' && cb <= 'z') || (cb >= 'A' && cb <= 'Z');

            if (aLatin != bLatin)
                return aLatin; // латиница меньше остальных

            if (ca != cb)
                return ca < cb;
        }
        return a.size() < b.size();
    }

    void sortPlayers(std::vector<Player> &players)
    {
        for (auto &p : players)
        {
            std::cout << p.tier << " : " << p.tank_name << "\n";
        }
        // сначала сортируем по имени танка
        std::stable_sort(players.begin(), players.end(), [](const Player &a, const Player &b)
                         {
        std::string an = a.tank_name, bn = b.tank_name;
        ApiController::toLowerInplace(an);
        ApiController::toLowerInplace(bn);
        return ApiController::tankNameCompare(an, bn); });

        // потом по tier сверху вниз, стабильная сортировка сохранит имя внутри одного tier
        std::stable_sort(players.begin(), players.end(), [](const Player &a, const Player &b)
                         { return a.tier > b.tier; });
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

    std::string getToken() const
    {
        return this->token;
    }

    std::string getAccountId() const
    {
        return this->account_id;
    }

    void setNickname(std::string nickname)
    {
        this->nickname = nickname;
    }

    void setToken(std::string token)
    {
        this->token = token;
    }

    void setAccountId(std::string account_id)
    {
        this->account_id = account_id;
    }

    void setAuth(bool auth)
    {
        this->isAuth = auth;
    }

    void initLog()
    {
        QString documents = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        QDir dir(documents + "/StatsOverlay/logs");

        if (!dir.exists())
            dir.mkpath(".");

        QString fileName = QDate::currentDate().toString("yyyy-MM-dd") + ".log";
        m_logFile.setFileName(dir.filePath(fileName));

        if (m_logFile.open(QIODevice::Append | QIODevice::Text))
        {
            QTextStream out(&m_logFile);
            out << "\n==== Log started ====\n";
            out << "OS: " << QSysInfo::prettyProductName()
                << " (kernel: " << QSysInfo::kernelVersion() << ")\n";
            out << "=====================\n\n";
        }
    }

    void log(const QString &message, int page, const QDateTime &timestamp)
    {
        if (!m_logFile.isOpen())
            return;

        static const QString prefixes[] = {
            "[MAIN PAGE]",
            "[RATING PAGE]",
            "[VEHICLE PAGE]",
            "[PLAYER PAGE]",
            "[AUTH]"};

        QString prefix;
        if (page >= 0 && page < 5)
            prefix = prefixes[page];
        else
            prefix = "[UNKNOWN]";

        QTextStream out(&m_logFile);
        out << timestamp.toString("yyyy-MM-dd hh:mm:ss") << " "
            << prefix << " " << message << "\n";
    }
};
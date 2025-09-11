#pragma once

#include <cstdint>
#include <cmath>

struct StatsData
{
    int64_t credits = 0;
    int64_t gold = 0;
    int64_t exp_battle = 0;
    int64_t exp_free = 0;
    int64_t battles = 0;
    int64_t wins = 0;
    int64_t losses = 0;
    int64_t totalDamage = 0;

    StatsData operator-(const StatsData &other) const
    {
        return {
            credits - other.credits,
            gold - other.gold,
            exp_battle - other.exp_battle,
            exp_free - other.exp_free,
            battles - other.battles,
            wins - other.wins,
            losses - other.losses,
            totalDamage - other.totalDamage};
    }
};

class MainStatsData
{
private:
    StatsData firstData;
    StatsData newData;
    StatsData currentData;

public:
    void initialStats(const StatsData &stats)
    {
        firstData = stats;
    }

    void updateStats(const StatsData &newStats)
    {
        newData = newStats;
        currentData = newData - firstData;
    }

    int64_t getGold() const
    {
        return currentData.gold;
    }

    int64_t getCredits() const
    {
        return currentData.credits;
    }

    int64_t getExpBattle() const
    {
        return currentData.exp_battle;
    }

    int64_t getExpFree() const
    {
        return currentData.exp_free;
    }

    int64_t getBattles() const
    {
        return currentData.battles;
    }

    int64_t getWins() const
    {
        return currentData.wins;
    }

    int64_t getLosses() const
    {
        return currentData.losses;
    }

    int64_t getTotalDamage() const
    {
        return currentData.totalDamage;
    }

    int64_t getAvgDamage() const
    {
        if (currentData.battles <= 0)
        {
            return -1;
        }
        return currentData.totalDamage / currentData.battles;
    }

    int64_t getAvgExp() const
    {
        if (currentData.battles <= 0)
        {
            return -1;
        }
        return currentData.exp_battle / currentData.battles;
    }

    float getPercentWins() const
    {
        if (currentData.battles <= 0)
        {
            return -1.0f;
        }
        float percent = static_cast<float>(currentData.wins) / currentData.battles * 100.0f;
        return std::round(percent * 100.0f) / 100.0f;
    }
};
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
            exp_battle - other.battles,
            exp_free - other.exp_free,
            battles - other.battles,
            wins - other.wins,
            losses - other.losses,
            totalDamage - other.totalDamage};
    }
};

class MainStats
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

    const StatsData &getFirstData() const
    {
        return firstData;
    }

    const StatsData &getCurrentData() const
    {
        return currentData;
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
        return round(static_cast<float>(currentData.wins) / static_cast<float>(currentData.battles) * 100.0) / 100.0;
    }
};
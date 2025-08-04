#pragma once

#include <cstdint>
#include <cmath>

struct RatingData
{
    int64_t mm_rating = 0;
    int64_t calib_battles = 0;
    int64_t exp_battle = 0;
    int64_t battles = 0;
    int64_t wins = 0;
    int64_t totalDamage = 0;

    RatingData operator-(const RatingData &other) const
    {
        return {
            mm_rating - other.mm_rating,
            calib_battles - other.calib_battles,
            exp_battle - other.exp_battle,
            battles - other.battles,
            wins - other.wins,
            totalDamage - other.totalDamage};
    }
};

class RatingStats
{
private:
    RatingData firstData;
    RatingData newData;
    RatingData currentData;

public:
    void initialStats(const RatingData &stats)
    {
        firstData = stats;
        firstData.mm_rating = 3000 + stats.mm_rating * 10;
    }

    void updateStats(const RatingData &newStats)
    {
        newData = newStats;
        currentData = newData - firstData;
        currentData.calib_battles = 10 - newStats.calib_battles;
        currentData.mm_rating = 3000 + newStats.mm_rating * 10;
    }

    int64_t getRating() const
    {
        return currentData.mm_rating;
    }

    int64_t getDiffRating() const
    {
        return currentData.mm_rating - firstData.mm_rating;
    }

    int64_t getCalibBattles() const
    {
        return currentData.calib_battles;
    }

    int64_t getExpBattle() const
    {
        return currentData.exp_battle;
    }

    int64_t getBattles() const
    {
        return currentData.battles;
    }

    int64_t getWins() const
    {
        return currentData.wins;
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
        return round(static_cast<float>(currentData.wins) / static_cast<float>(currentData.battles) * 100.0) / 100.0;
    }
};
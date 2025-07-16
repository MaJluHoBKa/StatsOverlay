#pragma once

#include <cstdint>

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

class MainStats
{
private:
    RatingData firstData;
    RatingData newData;
    RatingData currentData;

public:
    void initialStats(const RatingData &stats)
    {
        firstData = stats;
    }

    void updateStats(const RatingData &newStats)
    {
        newData = newStats;
        currentData = newData - firstData;
    }

    const RatingData &getFirstData() const
    {
        return firstData;
    }

    const RatingData &getCurrentData() const
    {
        return currentData;
    }
};
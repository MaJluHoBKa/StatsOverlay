#pragma once

#include <cstdint>

struct OtherData
{
    int64_t battles = 0;
    int64_t hits = 0;
    int64_t shots = 0;
    int64_t survived = 0;
    int64_t frags = 0;
    int64_t receiverDamage = 0;
    int64_t totalDamage = 0;
    int64_t lifeTime = 0;

    OtherData operator-(const OtherData &other) const
    {
        return {
            battles - other.battles,
            hits - other.hits,
            shots - other.shots,
            survived - other.survived,
            frags - other.frags,
            receiverDamage - other.receiverDamage,
            totalDamage - other.totalDamage,
            lifeTime - other.lifeTime};
    }
};

class OtherStats
{
private:
    OtherData firstData;
    OtherData newData;
    OtherData currentData;

public:
    void initialStats(const OtherData &stats)
    {
        firstData = stats;
    }

    void updateStats(const OtherData &newStats)
    {
        newData = newStats;
        currentData = newData - firstData;
    }

    const OtherData &getFirstData() const
    {
        return firstData;
    }

    const OtherData &getCurrentData() const
    {
        return currentData;
    }
};
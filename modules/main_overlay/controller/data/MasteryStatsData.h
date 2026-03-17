#pragma once

#include <cstdint>

struct MasteryData
{
    int64_t mastery = 0;
    int64_t mastery_1 = 0;
    int64_t mastery_2 = 0;
    int64_t mastery_3 = 0;

    MasteryData operator-(const MasteryData &other) const
    {
        return {
            mastery - other.mastery,
            mastery_1 - other.mastery_1,
            mastery_2 - other.mastery_2,
            mastery_3 - other.mastery_3};
    }
};

class MasteryStatsData
{
private:
    MasteryData firstData;
    MasteryData newData;
    MasteryData currentData;

public:
    void initialStats(const MasteryData &stats)
    {
        firstData = stats;
        currentData = stats - firstData;
    }

    void updateStats(const MasteryData &newStats)
    {
        newData = newStats;
        currentData = newData - firstData;
    }

    const MasteryData &getFirstData() const
    {
        return firstData;
    }

    const MasteryData &getCurrentData() const
    {
        return currentData;
    }
};
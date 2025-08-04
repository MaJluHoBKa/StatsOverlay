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

    float getPercentHits() const
    {
        if (currentData.shots <= 0)
        {
            return -1.0f;
        }
        float percent = static_cast<float>(currentData.hits) / currentData.shots * 100.0f;
        return std::round(percent * 100.0f) / 100.0f;
    }

    float getPercentSurvived() const
    {
        if (currentData.battles <= 0)
        {
            return -1.0f;
        }
        float percent = static_cast<float>(currentData.survived) / currentData.battles * 100.0f;
        return std::round(percent * 100.0f) / 100.0f;
    }

    int64_t getLifeTime() const
    {
        if (currentData.battles <= 0)
        {
            return -1;
        }
        return currentData.lifeTime / currentData.battles;
    }

    float getDamageK() const
    {
        if (currentData.receiverDamage <= 0)
        {
            return -1.0f;
        }
        return round(static_cast<float>(currentData.totalDamage) / static_cast<float>(currentData.receiverDamage) * 100.0) / 100.0;
    }

    float getFragsK() const
    {
        if (currentData.battles <= 0)
        {
            return -1.0f;
        }
        int deaths = currentData.battles - currentData.survived;
        if (deaths > 0)
        {
            return round(static_cast<float>(currentData.frags) / static_cast<float>(deaths) * 100.0) / 100.0;
        }
        return static_cast<float>(currentData.frags);
    }
};
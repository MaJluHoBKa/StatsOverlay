#include <iostream>
#include <chrono>
#include <random>
#include "classes/MainStats.h" // Твой хедер с StatsData и MainStats

int main()
{
    constexpr int N = 1'000'000;
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist_credits(0, 1'000'000);
    std::uniform_int_distribution<int> dist_gold(0, 1'000);
    std::uniform_int_distribution<int> dist_exp_battle(0, 50'000);
    std::uniform_int_distribution<int> dist_exp_free(0, 10'000);
    std::uniform_int_distribution<int> dist_battles(1, 10'000);
    std::uniform_int_distribution<int> dist_totalDamage(0, 2'000'000);

    auto start = std::chrono::steady_clock::now();

    for (int i = 0; i < N; ++i)
    {
        StatsData tmpData;
        tmpData.credits = dist_credits(rng);
        tmpData.gold = dist_gold(rng);
        tmpData.exp_battle = dist_exp_battle(rng);
        tmpData.exp_free = dist_exp_free(rng);
        tmpData.battles = dist_battles(rng);
        tmpData.totalDamage = dist_totalDamage(rng);
        tmpData.losses = rng() % (tmpData.battles + 1);
        tmpData.wins = tmpData.battles - tmpData.losses;

        MainStats main;
        main.initialStats(tmpData);
        main.updateStats(tmpData);
        volatile auto avgDamage = main.getAvgDamage();
        volatile auto avgExp = main.getAvgExp();
        volatile auto percentWins = main.getPercentWins();
    }

    auto end = std::chrono::steady_clock::now();
    double elapsed_sec = std::chrono::duration<double>(end - start).count();

    std::cout << "C++ биндинг StatsData: " << elapsed_sec << " сек\n";
    return 0;
}

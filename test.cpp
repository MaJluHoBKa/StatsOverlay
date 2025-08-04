#include <iostream>
#include <chrono>
#include "classes/MainStats.h"
#include "classes/ApiController.h"

int main()
{
    ApiController apiController;
    apiController.login();
    auto start = std::chrono::high_resolution_clock::now();
    std::cout << apiController.get_vehicles_names();
    std::cout << apiController.update_vehicles_stats();
    while (true)
    {
        apiController.update_main_stats();
        apiController.update_rating_stats();
        apiController.update_mastery_stats();
        apiController.update_other_stats();
        std::cout << apiController.update_vehicles_stats() << std::endl;
        apiController.get_updated_vehicles();
        Sleep(15000);
    }
    std::cout << apiController.prolongate() << std::endl;
    std::cout << apiController.logout() << std::endl;
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Runtime: " << duration.count() << " ms\n";
    return 0;
}

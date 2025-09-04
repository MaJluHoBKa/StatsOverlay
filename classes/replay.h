#pragma once
#include <string>

#ifdef _WIN32
#define DLL_EXPORT extern "C" __declspec(dllexport)
#else
#define DLL_EXPORT extern "C"
#endif

struct Player
{
    std::string nickname;
    int team_number;
    uint64_t account_id;
    std::string clan_tag;
    int platoon_number;
    int tank_id;
};

DLL_EXPORT const char *ParseReplay(const char *path);
DLL_EXPORT void FreeResult(const char *ptr);
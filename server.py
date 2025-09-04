import json
from math import floor
import time
import requests
import webbrowser
from flask import Flask, redirect, request
import threading

def resource_path(relative_path):
        import sys, os
        if hasattr(sys, '_MEIPASS'):
            return os.path.join(sys._MEIPASS, relative_path)
        return os.path.join(os.path.abspath("."), relative_path)

class APIClient:
    def __init__(self, base_url="https://api.tanki.su/wot", application_id="4d9feb2c53c712fc6a87e026c990dd12"):
        self.base_url = base_url
        self.application_id = application_id
        self.token = None
        self.time_pointer = None
        self.account_id = None
        self.nickname = None
        self.is_auth = False

        self.is_first_main_stats = True
        self.is_first_rating_stats = True
        self.is_first_tech_stats = True
        self.is_first_other_stats = True
        self.is_first_master_stats = True

        self.main_stats_structure = {
            "credits": 0,
            "gold": 0,
            "exp_battle": 0,
            "exp_free": 0,
            "battles": 0,
            "wins": 0,
            "losses": 0,
            "totalDamage": 0,
            "avgAssist": 0,
            "avgBlock": 0,
        }
        self.first_main_stats_economic_structure = {
            "credits": 0,
            "gold": 0,
            "exp_battle": 0,
            "exp_free": 0,
        }
        self.first_main_stats_structure = {
            "battles": 0,
            "wins": 0,
            "losses": 0,
            "totalDamage": 0,
        }

        self.first_other_stats_structure = {
            "battles": 0,
            "hits": 0,
            "shots": 0,
            "survived": 0,
            "frags": 0,
            "receiverDamage": 0,
            "totalDamage": 0,
            "lifeTime": 0,
        }        
        self.other_stats_structure = {
            "battles": 0,
            "hits": 0,
            "shots": 0,
            "survived": 0,
            "frags": 0,
            "receiverDamage": 0,
            "totalDamage": 0,
            "lifeTime": 0,
        }
        
        self.first_master_structure = {
            "mastery": 0,
            "mastery_1": 0,
            "mastery_2": 0,
            "mastery_3": 0,
        }
        self.master_structure = {
            "mastery": 0,
            "mastery_1": 0,
            "mastery_2": 0,
            "mastery_3": 0,
        }

        self.rating_stats_structure = {
            "mm_rating": 0,
            "calib_battle": 0,
            "exp_battle": 0,
            "battles": 0,
            "wins": 0,
            "totalDamage": 0,
        }
        self.first_rating_stats_structure = {
            "mm_rating": 0,
            "exp_battle": 0,
            "battles": 0,
            "wins": 0,
            "totalDamage": 0,
        }
        
        self.tech_stats_array = {}
        self.prev_tech_stats_array = {}
        tech_stats_structure = {
            "id": 0,
            "battles": 0,
            "totalDamage": 0,
            "wins": 0,
        }
        self.first_tech_stats_array = {}
        self.tech_info_dataset = {}

        self.graphics_value = {
            "wins": [],
            "damage": [],
            "xp": [],
            "rating": [],
        }
        self.graphics_x_value = {
            "wins": [],
            "damage": [],
            "xp": [],
            "rating": [],
        }

        self.players_stats = {}
        self.player_tanks = {}

    def reset_stats(self):
        self.main_stats_structure = {
            "credits": 0,
            "exp_battle": 0,
            "exp_free": 0,
            "battles": 0,
            "winRate": 0.0,
            "avgDamage": 0,
            "avgAssist": 0,
            "avgBlock": 0,
            "avgExp": 0,
        }

    def set_main_stats(self):
        if self.is_auth:
            url = "https://papi.tanksblitz.ru/wotb/account/info/"
            params = {
                "application_id": self.application_id,
                "access_token": self.token,
                "account_id": self.account_id,
            }
            try:
                response = requests.get(url, params=params)
                response.raise_for_status()
                data = response.json()
                if self.is_first_main_stats:
                    if "data" in data and str(self.account_id) in data["data"]:
                        self.first_main_stats_economic_structure["credits"] = data.get("data", {}).get(self.account_id, {}).get("private", {}).get("credits")
                        self.first_main_stats_economic_structure["gold"] = data.get("data", {}).get(self.account_id, {}).get("private", {}).get("gold")
                        self.first_main_stats_economic_structure["exp_battle"] = data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("all", {}).get("xp")
                        self.first_main_stats_economic_structure["exp_free"] = data.get("data", {}).get(self.account_id, {}).get("private", {}).get("free_xp")
                        
                        self.first_main_stats_structure["battles"] = data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("all", {}).get("battles")
                        self.first_main_stats_structure["totalDamage"] = data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("all", {}).get("damage_dealt")
                        self.first_main_stats_structure["losses"] = data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("all", {}).get("losses")
                        self.first_main_stats_structure["wins"] = data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("all", {}).get("wins")
                        self.is_first_main_stats = False

                if "data" in data and str(self.account_id) in data["data"]:
                    self.main_stats_structure["credits"] = data.get("data", {}).get(self.account_id, {}).get("private", {}).get("credits") - self.first_main_stats_economic_structure["credits"]
                    self.main_stats_structure["gold"] = data.get("data", {}).get(self.account_id, {}).get("private", {}).get("gold") - self.first_main_stats_economic_structure["gold"]
                    self.main_stats_structure["exp_battle"] = data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("all", {}).get("xp") - self.first_main_stats_economic_structure["exp_battle"]
                    self.main_stats_structure["exp_free"] = data.get("data", {}).get(self.account_id, {}).get("private", {}).get("free_xp") - self.first_main_stats_economic_structure["exp_free"]
                    self.main_stats_structure["battles"] = data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("all", {}).get("battles") - self.first_main_stats_structure["battles"]
                    self.main_stats_structure["totalDamage"] = data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("all", {}).get("damage_dealt") - self.first_main_stats_structure["totalDamage"]
                    self.main_stats_structure["losses"] = data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("all", {}).get("losses") - self.first_main_stats_structure["losses"]
                    self.main_stats_structure["wins"] = data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("all", {}).get("wins") - self.first_main_stats_structure["wins"]
                    print("Данные MAIN получены и записаны.")
                else:
                    print("Ошибка: данные не найдены в ответе API.")
                    return False
            except requests.exceptions.RequestException as e:
                print(f"Ошибка при получении данных: {e}")
                return False
            return True
        else:
            print("Ошибка: Необходимо авторизоваться.")
            return False
    
    def set_rating_stats(self):
        if self.is_auth:
            url = "https://papi.tanksblitz.ru/wotb/account/info/"
            params = {
                "application_id": self.application_id,
                "access_token": self.token,
                "account_id": self.account_id,
                "extra": "statistics.rating"
            }
            try:
                response = requests.get(url, params=params)
                response.raise_for_status()
                data = response.json()
                if self.is_first_rating_stats:
                    if "data" in data and str(self.account_id) in data["data"]:
                        self.first_rating_stats_structure["mm_rating"] = 3000 + (floor(data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("rating", {}).get("mm_rating") * 10.0))
                        self.first_rating_stats_structure["battles"] = data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("rating", {}).get("battles")
                        self.first_rating_stats_structure["wins"] = data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("rating", {}).get("wins")
                        self.first_rating_stats_structure["totalDamage"] = data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("rating", {}).get("damage_dealt")
                        self.first_rating_stats_structure["exp_battle"] = data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("rating", {}).get("xp")
                        self.is_first_rating_stats = False

                if "data" in data and str(self.account_id) in data["data"]:
                    self.rating_stats_structure["mm_rating"] = 3000 + (floor(data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("rating", {}).get("mm_rating") * 10.0))
                    self.rating_stats_structure["calib_battle"] = 10 - data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("rating", {}).get("calibration_battles_left")
                    self.rating_stats_structure["battles"] = data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("rating", {}).get("battles") - self.first_rating_stats_structure["battles"]
                    self.rating_stats_structure["wins"] = data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("rating", {}).get("wins") - self.first_rating_stats_structure["wins"]
                    self.rating_stats_structure["totalDamage"] = data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("rating", {}).get("damage_dealt") - self.first_rating_stats_structure["totalDamage"]
                    self.rating_stats_structure["exp_battle"] = data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("rating", {}).get("xp") - self.first_rating_stats_structure["exp_battle"]

                    print("Данные RATING получены и записаны.")
                
                else:
                    print("Ошибка: данные не найдены в ответе API.")
                    return False
            except requests.exceptions.RequestException as e:
                print(f"Ошибка при получении данных: {e}")
                return False
            return True
        else:
            print("Ошибка: Необходимо авторизоваться.")
            return False

    def set_tech_stats(self):
        if self.is_auth:
            url = "https://papi.tanksblitz.ru/wotb/tanks/stats/"
            params = {
                "application_id": self.application_id,
                "access_token": self.token,
                "account_id": self.account_id,
            }
            try:
                response = requests.get(url, params=params)
                response.raise_for_status()
                data = response.json()

                if "data" in data and str(self.account_id) in data["data"]:
                    tanks_data = data["data"][str(self.account_id)]
                    if isinstance(tanks_data, list):
                        if self.is_first_tech_stats:
                            self.set_unknow_tanks()
                        for tank in tanks_data:
                            tank_id = tank.get("tank_id")
                            if tank_id is not None:
                                if self.is_first_tech_stats:
                                    self.first_tech_stats_array[tank_id] = {
                                        "battles": tank["all"].get("battles", 0),
                                        "totalDamage": tank["all"].get("damage_dealt", 0),
                                        "wins": tank["all"].get("wins", 0),
                                    }
            
                                self.tech_stats_array[tank_id] = {
                                    "id": tank_id,
                                    "battles": tank["all"].get("battles", 0) - self.first_tech_stats_array.get(tank_id, {}).get("battles", 0),
                                    "totalDamage": tank["all"].get("damage_dealt", 0) - self.first_tech_stats_array.get(tank_id, {}).get("totalDamage", 0),
                                    "wins": tank["all"].get("wins", 0) - self.first_tech_stats_array.get(tank_id, {}).get("wins", 0),
                                }

                        if self.is_first_tech_stats:
                            self.is_first_tech_stats = False
                else:
                    print("Ошибка: данные не найдены в ответе API.")
                    return False
            except requests.exceptions.RequestException as e:
                print(f"Ошибка при получении данных: {e}")
                return False
            return True
        else:
            print("Ошибка: Необходимо авторизоваться.")
            return False

    def set_other_stats(self):
        if self.is_auth:
            url = "https://papi.tanksblitz.ru/wotb/account/info/"
            params = {
                "application_id": self.application_id,
                "access_token": self.token,
                "account_id": self.account_id,
                "extra": "statistics.rating"
            }
            try:
                response = requests.get(url, params=params)
                response.raise_for_status()
                data = response.json()
                if self.is_first_other_stats:
                    if "data" in data and str(self.account_id) in data["data"]:
                        self.first_other_stats_structure["battles"] = data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("all", {}).get("battles") + data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("rating", {}).get("battles")
                        self.first_other_stats_structure["hits"] = data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("all", {}).get("hits") + data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("rating", {}).get("hits")
                        self.first_other_stats_structure["shots"] = data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("all", {}).get("shots") + data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("rating", {}).get("shots")
                        self.first_other_stats_structure["survived"] = data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("all", {}).get("survived_battles") + data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("rating", {}).get("survived_battles")
                        self.first_other_stats_structure["frags"] = data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("all", {}).get("frags") + data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("rating", {}).get("frags")
                        self.first_other_stats_structure["receiverDamage"] = data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("all", {}).get("damage_received") + data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("rating", {}).get("damage_received")
                        self.first_other_stats_structure["totalDamage"] = data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("all", {}).get("damage_dealt") + data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("rating", {}).get("damage_dealt")
                        self.first_other_stats_structure["lifeTime"] = data.get("data", {}).get(self.account_id, {}).get("private", {}).get("battle_life_time")                        
                        self.is_first_other_stats = False

                if "data" in data and str(self.account_id) in data["data"]:
                    self.other_stats_structure["battles"] = data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("all", {}).get("battles") + data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("rating", {}).get("battles") - self.first_other_stats_structure["battles"]
                    self.other_stats_structure["hits"] = data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("all", {}).get("hits") + data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("rating", {}).get("hits") - self.first_other_stats_structure["hits"]
                    self.other_stats_structure["shots"] = data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("all", {}).get("shots") + data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("rating", {}).get("shots") - self.first_other_stats_structure["shots"]
                    self.other_stats_structure["survived"] = data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("all", {}).get("survived_battles") + data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("rating", {}).get("survived_battles") - self.first_other_stats_structure["survived"]
                    self.other_stats_structure["frags"] = data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("all", {}).get("frags") + data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("rating", {}).get("frags") - self.first_other_stats_structure["frags"]
                    self.other_stats_structure["receiverDamage"] = data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("all", {}).get("damage_received") + data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("rating", {}).get("damage_received") - self.first_other_stats_structure["receiverDamage"]
                    self.other_stats_structure["totalDamage"] = data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("all", {}).get("damage_dealt") + data.get("data", {}).get(self.account_id, {}).get("statistics", {}).get("rating", {}).get("damage_dealt") - self.first_other_stats_structure["totalDamage"]
                    self.other_stats_structure["lifeTime"] = data.get("data", {}).get(self.account_id, {}).get("private", {}).get("battle_life_time") - self.first_other_stats_structure["lifeTime"]
                else:
                    print("Ошибка: данные не найдены в ответе API.")
                    return False
            except requests.exceptions.RequestException as e:
                print(f"Ошибка при получении данных: {e}")
                return False
            return True
        else:
            print("Ошибка: Необходимо авторизоваться.")
            return False

    def set_master_stats(self):
        if self.is_auth:
            url = "https://papi.tanksblitz.ru/wotb/account/achievements/"
            params = {
                "application_id": self.application_id,
                "account_id": self.account_id,
                "fields": "achievements",
            }
            try:
                response = requests.get(url, params=params)
                response.raise_for_status()
                data = response.json()

                if self.is_first_master_stats:
                    if "data" in data and str(self.account_id) in data["data"]:
                        self.first_master_structure["mastery"] = data.get("data", {}).get(self.account_id, {}).get("achievements", {}).get("markOfMastery")
                        self.first_master_structure["mastery_1"] = data.get("data", {}).get(self.account_id, {}).get("achievements", {}).get("markOfMasteryI")
                        self.first_master_structure["mastery_2"] = data.get("data", {}).get(self.account_id, {}).get("achievements", {}).get("markOfMasteryII")
                        self.first_master_structure["mastery_3"] = data.get("data", {}).get(self.account_id, {}).get("achievements", {}).get("markOfMasteryIII")
                        self.is_first_master_stats = False

                if "data" in data and str(self.account_id) in data["data"]:
                    self.master_structure["mastery"] = data.get("data", {}).get(self.account_id, {}).get("achievements", {}).get("markOfMastery") - self.first_master_structure["mastery"]
                    self.master_structure["mastery_1"] = data.get("data", {}).get(self.account_id, {}).get("achievements", {}).get("markOfMasteryI") - self.first_master_structure["mastery_1"]
                    self.master_structure["mastery_2"] = data.get("data", {}).get(self.account_id, {}).get("achievements", {}).get("markOfMasteryII") - self.first_master_structure["mastery_2"]
                    self.master_structure["mastery_3"] = data.get("data", {}).get(self.account_id, {}).get("achievements", {}).get("markOfMasteryIII") - self.first_master_structure["mastery_3"]
                else:
                    print("Ошибка: данные не найдены в ответе API.")
                    return False
            except requests.exceptions.RequestException as e:
                print(f"Ошибка при получении данных: {e}")
                return False
            return True
        else:
            print("Ошибка: Необходимо авторизоваться.")
            return False

    def get_players(self, unique_nicks, tank_ids, team):
        if self.is_auth:
            url = "https://papi.tanksblitz.ru/wotb/account/list/"
            params = {
                "application_id": self.application_id,
                "search": ",".join(unique_nicks),
                "type" : "exact",
                "limit": "100"
            }
            try:
                response = requests.get(url, params=params)
                response.raise_for_status()
                data = response.json()
                players_id = []

                if "data" in data and isinstance(data["data"], list):
                    for player in data["data"]:
                        if "account_id" in player:
                            players_id.append(str(player["account_id"]))                          
                else:
                    print("Ошибка: данные не найдены в ответе API.")
                    return False
                
                url = "https://papi.tanksblitz.ru/wotb/account/info/"
                params = {
                    "application_id": self.application_id,
                    "account_id": ",".join(players_id),
                    "fields" : "statistics.all.battles,statistics.all.wins,statistics.all.damage_dealt,nickname",
                }   
                
                try:
                    response = requests.get(url, params=params)
                    response.raise_for_status()
                    data = response.json()

                    if "data" in data and isinstance(data["data"], dict):
                        ids = 0
                        # создаем соответствие nickname -> tank_id
                        nick_to_tank = {nick: tid for nick, tid in zip(unique_nicks, tank_ids)}
                        for account_id, info in data["data"].items():
                            stats = info.get("statistics", {}).get("all", {})
                            battles = stats.get("battles", 0)
                            wins = stats.get("wins", 0)
                            damage = stats.get("damage_dealt", 0)
                            nickname = info.get("nickname")
                            tank_id = nick_to_tank.get(nickname)


                            avg_damage = damage // battles if battles > 0 else 0
                            winrate = round((wins / battles * 100), 2) if battles > 0 else 0.0

                            url = "https://papi.tanksblitz.ru/wotb/encyclopedia/vehicles/"
                            params = {
                                "application_id": self.application_id,
                                "fields" : "tier,name",
                                "tank_id": tank_id
                            } 

                            try:
                                response = requests.get(url, params=params)
                                response.raise_for_status()
                                data = response.json()
                                
                                tank_tier = "0"
                                tank_name = "Unknown"

                                if "data" in data and isinstance(data["data"], dict):
                                    for tid, tank_info in data["data"].items():
                                        if tank_info:
                                            tank_tier = str(tank_info.get("tier", "0"))
                                            tank_name = tank_info.get("name", "Unknown")
                                        elif tank_id in self.tech_info_dataset:
                                            tank_tier = str(self.tech_info_dataset[tank_id].get("tier", "0"))
                                            tank_name = self.tech_info_dataset[tank_id].get("name", "Unknown")

                                        # fallback из self.tech_info_dataset, если имя не найдено
                                        if not tank_name and tank_id in self.tech_info_dataset:
                                            tank_name = self.tech_info_dataset[tank_id].get("name", "Unknown")
                                            tank_tier = str(self.tech_info_dataset[tank_id].get("tier", "0"))

                                        team_index = None
                                        if nickname in unique_nicks:
                                            team_index = unique_nicks.index(nickname)

                                        self.players_stats[account_id] = {
                                            "nickname": nickname,
                                            "battles": battles,
                                            "avg_damage": avg_damage,
                                            "winrate": winrate,
                                            "tank_tier": tank_tier,
                                            "tank_name": tank_name,
                                            "team": team[team_index] if team_index is not None else "0"  # используем массив team
                                        }
                                    ids += 1

                            except requests.exceptions.RequestException as e:
                                print(f"Ошибка при получении данных: {e}")
                                return False  
                        
                        allies = []
                        enemies = []

                        player_team = None
                        for acc_id, stats in self.players_stats.items():
                            if stats.get("nickname") == self.nickname:
                                player_team = stats.get("team")
                                break
                            
                        # Если ник не найден, считаем союзников командой 2
                        if player_team is None:
                            player_team = 2

                        # Разбиваем на союзников и противников
                        for acc_id, stats in self.players_stats.items():
                            if stats.get("team") == player_team:
                                allies.append((acc_id, stats))
                            else:
                                enemies.append((acc_id, stats))

                        allies_indexed = [(i, acc_id, stats) for i, (acc_id, stats) in enumerate(allies)]
                        enemies_indexed = [(i, acc_id, stats) for i, (acc_id, stats) in enumerate(enemies)]
                        
                        allies_sorted = sorted(
                            allies_indexed,
                            key=lambda x: (
                                int(x[2].get("tank_tier", 0)),            # тир
                                x[2].get("tank_name", "").strip().lower(), # имя танка
                                x[0]                                      # исходный порядок
                            )
                        )
                        
                        enemies_sorted = sorted(
                            enemies_indexed,
                            key=lambda x: (
                                int(x[2].get("tank_tier", 0)),
                                x[2].get("tank_name", "").strip().lower(),
                                x[0]
                            )
                        )
                        
                        # возвращаем к обычной форме
                        allies_sorted = [(acc_id, stats) for _, acc_id, stats in allies_sorted]
                        enemies_sorted = [(acc_id, stats) for _, acc_id, stats in enemies_sorted]

                        # собираем обратно в словарь в правильном порядке
                        from collections import OrderedDict
                        self.players_stats = OrderedDict(allies_sorted + enemies_sorted)

                        return True  
                                    
                    else:
                        print("Ошибка: данные не найдены в ответе API.")
                        return False  


                except requests.exceptions.RequestException as e:
                    print(f"Ошибка при получении данных: {e}")
                    return False

            except requests.exceptions.RequestException as e:
                print(f"Ошибка при получении данных: {e}")
                return False
        else:
            print("Ошибка: Необходимо авторизоваться.")
            return False


    def set_tech_info(self, id):
        if self.is_auth:
            url = "https://papi.tanksblitz.ru/wotb/encyclopedia/vehicles/"
            params = {
                "application_id": self.application_id,
                "fields": "name,tank_id,tier",
                "tank_id": id,
            }
            try:
                response = requests.get(url, params=params)
                response.raise_for_status()
                data = response.json()

                if "data" in data:
                    tanks_data = data["data"]
                    if isinstance(tanks_data, dict):
                        for tank_id, tank_info in tanks_data.items():
                            if tank_info is None:
                                self.tech_info_dataset[int(tank_id)] = {
                                    "name": tank_id,
                                    "tank_id": tank_id,
                                    "tier": 0,
                                }
                            else:
                                self.tech_info_dataset[int(tank_id)] = {
                                    "name": tank_info.get("name"),
                                    "tank_id": tank_info.get("tank_id"),
                                    "tier": tank_info.get("tier")
                                }
                            # if tank_info is None:
                            #     print(f"Танк ID {tank_id}: данные отсутствуют, пропускаем...")
                            #     continue
                            # self.tech_info_dataset[int(tank_id)] = {
                            #     "name": tank_info.get("name"),
                            #     "tank_id": tank_info.get("tank_id")
                            # }
                            print(f"Танк ID: {tank_id}, Имя: {self.tech_info_dataset[tank_id]['name']}")
                    else:
                        print("Ошибка: Ожидался словарь в поле 'data'.")
                        return False
                else:
                    print("Ошибка: данные не найдены в ответе API.")
                    return False
            except requests.exceptions.RequestException as e:
                print(f"Ошибка при получении данных: {e}")
                return False
            return True
        else:
            print("Ошибка: Необходимо авторизоваться.")
            return False

    def set_unknow_tanks(self):
        path = resource_path("src/tank-list/tanks.json")

        with open(path, "r", encoding="utf-8") as f:
            tanks_json = json.load(f)

        for tank_info in tanks_json.get("tanks", []):
            tank_id = tank_info.get("id")
            tank_name = tank_info.get("name")
            tier = tank_info.get("tier")

            if tank_id is not None and tank_name:
                self.tech_info_dataset[tank_id] = {
                    "name": tank_name,
                    "tank_id": tank_id,
                    "tier": tier
                }
            print(f"Танк ID: {tank_id}, Имя: {self.tech_info_dataset[tank_id]['name']}")

    def save_current_to_first(self):
        self.is_first_main_stats = True
        self.is_first_rating_stats = True
        self.is_first_tech_stats = True
        self.is_first_other_stats = True
        self.is_first_master_stats = True
        # self.first_main_stats_economic_structure["credits"] = self.first_main_stats_economic_structure["credits"] + self.main_stats_structure["credits"]
        # self.first_main_stats_economic_structure["gold"] = self.first_main_stats_economic_structure["gold"] + self.main_stats_structure["gold"]
        # self.first_main_stats_economic_structure["exp_battle"] = self.first_main_stats_economic_structure["exp_battle"] + self.main_stats_structure["exp_battle"]
        # self.first_main_stats_economic_structure["exp_free"] = self.first_main_stats_economic_structure["exp_free"] + self.main_stats_structure["exp_free"]
        # self.first_main_stats_structure["battles"] = self.first_main_stats_structure["battles"] + self.main_stats_structure["battles"]
        # self.first_main_stats_structure["wins"] = self.first_main_stats_structure["wins"] + self.main_stats_structure["wins"]
        # self.first_main_stats_structure["losses"] = self.first_main_stats_structure["losses"] + self.main_stats_structure["losses"]
        # self.first_main_stats_structure["totalDamage"] = self.first_main_stats_structure["totalDamage"] + self.main_stats_structure["totalDamage"]

        # self.first_rating_stats_structure["mm_rating"] = self.rating_stats_structure["mm_rating"]
        # self.first_rating_stats_structure["exp_battle"] = self.first_rating_stats_structure["exp_battle"] + self.rating_stats_structure["exp_battle"]
        # self.first_rating_stats_structure["battles"] = self.first_rating_stats_structure["battles"] + self.rating_stats_structure["battles"]
        # self.first_rating_stats_structure["wins"] = self.first_rating_stats_structure["wins"] + self.rating_stats_structure["wins"]
        # self.first_rating_stats_structure["totalDamage"] = self.first_rating_stats_structure["totalDamage"] + self.rating_stats_structure["totalDamage"]

        # for tank_id, current_stats in self.tech_stats_array.items():
        #     first_stats = self.first_tech_stats_array.get(tank_id, {"battles": 0, "totalDamage": 0, "wins": 0})
        
        #     self.first_tech_stats_array[tank_id] = {
        #         "battles": first_stats["battles"] + current_stats.get("battles", 0),
        #         "totalDamage": first_stats["totalDamage"] + current_stats.get("totalDamage", 0),
        #         "wins": first_stats["wins"] + current_stats.get("wins", 0),
        #     }
        # self.tech_info_dataset.clear()

    def authenticate(self):
        """
        Выполняет авторизацию через OpenID.
        """
        url = f"{self.base_url}/auth/login/"
        redirect_uri = "http://localhost:5000/"  # Локальный сервер для обработки ответа
        params = {
            "application_id": self.application_id,
            "redirect_uri": redirect_uri,
            "nofollow": 1
        }
        try:
            # Отправляем запрос на авторизацию
            response = requests.get(url, params=params)
            response.raise_for_status()

            # Получаем URL для авторизации
            data = response.json()
            location = data.get("data", {}).get("location")
            if not location:
                print("Ошибка: URL для авторизации не получен.")
                return False

            # Открываем URL авторизации в браузере
            print("Открываем страницу авторизации в браузере...")
            webbrowser.open(location)

            # Запускаем локальный сервер для обработки redirect_uri
            self.start_local_server()
            return True

        except requests.exceptions.RequestException as e:
            print(f"Ошибка авторизации: {e}")
            return False

    def exit_auth(self):
        url = f"{self.base_url}/auth/logout/"
        params = {
            "application_id": self.application_id,
            "access_token": self.token,
        }
        try:
            response = requests.post(url, params=params)
            response.raise_for_status()
            
            if response.status_code == 200:
                print("Выход из аккаунта выполнен успешно.")
                self.token = None
                self.account_id = None
                self.nickname = None
                return True
            else:
                print(f"Ошибка выхода: неожиданный статус ответа {response.status_code}")
                return False

        except requests.exceptions.RequestException as e:
            print(f"Ошибка авторизации: {e}")
            return False

    # метод, который будет сохранять все флаги и rirst stats в файл json в папку Документы/StatsOverlay
    def save_stats_to_file(self, filename="stats.json"):
        import os
        import json

        # Путь к папке Документы
        documents_path = os.path.expanduser("~/Documents/StatsOverlay")
        if not os.path.exists(documents_path):
            os.makedirs(documents_path)

        # Полный путь к файлу
        file_path = os.path.join(documents_path, filename)

        # Сбор данных для сохранения
        stats_data = {
            "nickname": self.nickname,
            "pointer": self.token,
            "time": self.time_pointer,
            "is_auth": self.is_auth,
            "account_id": self.account_id,
            "is_first_main_stats": self.is_first_main_stats,
            "is_first_rating_stats": self.is_first_rating_stats,
            "is_first_tech_stats": self.is_first_tech_stats,
            "is_first_other_stats": self.is_first_other_stats,
            "is_first_master_stats": self.is_first_master_stats,
            "first_main_stats_economic_structure": self.first_main_stats_economic_structure,
            "first_main_stats_structure": self.first_main_stats_structure,
            "first_rating_stats_structure": self.first_rating_stats_structure,
            "first_tech_stats_array": self.first_tech_stats_array,
            "tech_stats_array": self.tech_stats_array,
            "prev_tech_stats_array": self.prev_tech_stats_array,
            "first_other_stats_structure": self.first_other_stats_structure,
            "first_master_structure": self.first_master_structure,
            "tech_info_dataset": self.tech_info_dataset,
            # "graphics_value": self.graphics_value,
            # "graphics_x_value": self.graphics_x_value,
        }

        # Сохранение данных в файл
        with open(file_path, "w", encoding="utf-8") as f:
            json.dump(stats_data, f, ensure_ascii=False, indent=4)
        
        print(f"Статистика успешно сохранена в {file_path}")

    def load_stats_from_file(self, filename="stats.json"):
        import os
        import json

        # Путь к файлу
        documents_path = os.path.expanduser("~/Documents/StatsOverlay")
        file_path = os.path.join(documents_path, filename)

        # Проверка наличия файла
        if not os.path.exists(file_path):
            print(f"Файл {file_path} не найден. Загрузка пропущена.")
            return

        # Загрузка и установка данных
        with open(file_path, "r", encoding="utf-8") as f:
            try:
                stats_data = json.load(f)
                
                self.token = stats_data.get("pointer")
                self.account_id = stats_data.get("account_id")
                self.is_auth = stats_data.get("is_auth")
                self.time_pointer = stats_data.get("time")
                self.nickname = stats_data.get("nickname")

                self.is_first_main_stats = stats_data.get("is_first_main_stats", True)
                self.is_first_rating_stats = stats_data.get("is_first_rating_stats", True)
                self.is_first_tech_stats = stats_data.get("is_first_tech_stats", True)
                self.is_first_other_stats = stats_data.get("is_first_other_stats", True)
                self.is_first_master_stats = stats_data.get("is_first_master_stats", True)

                self.first_main_stats_economic_structure = stats_data.get("first_main_stats_economic_structure", {})
                self.first_main_stats_structure = stats_data.get("first_main_stats_structure", {})
                self.first_rating_stats_structure = stats_data.get("first_rating_stats_structure", {})
                self.first_other_stats_structure = stats_data.get("first_other_stats_structure", {})
                self.first_master_structure = stats_data.get("first_master_structure", {})
                # self.graphics_value = stats_data.get("graphics_value", {
                #     "wins": [],
                #     "damage": [],
                #     "xp": [],
                #     "rating": [],
                # })
                # self.graphics_x_value = stats_data.get("graphics_x_value", {
                #     "wins": [],
                #     "damage": [],
                #     "xp": [],
                #     "rating": [],
                # })

                # Кастим ключи обратно в int
                self.first_tech_stats_array = {
                    int(k): v for k, v in stats_data.get("first_tech_stats_array", {}).items()
                }

                print(f"Статистика успешно загружена из {file_path}")
            except json.JSONDecodeError:
                print(f"Ошибка: файл {file_path} повреждён или имеет неверный формат.")

    def start_local_server(self):
        """
        Запускает локальный сервер для обработки redirect_uri.
        """
        app = Flask(__name__)

        @app.route("/")
        def handle_response():
            # Получаем параметры из redirect_uri
            self.token = request.args.get("access_token")
            self.account_id = request.args.get("account_id")
            self.nickname = request.args.get("nickname")
            self.time_pointer = request.args.get("expires_at")
            status = request.args.get("status")

            print("HANDLE RESPONSE: self =", id(self), "nickname =", self.nickname)

            if status == "ok" and self.token:
                print(f"Авторизация успешна! Ник: {self.nickname}")
                return redirect("https://lesta.ru/ru/")
            else:
                error_message = request.args.get("message", "Неизвестная ошибка")
                print(f"Ошибка авторизации: {error_message}")
                return redirect("https://lesta.ru/ru/")

        # Запускаем сервер в отдельном потоке
        threading.Thread(target=lambda: app.run(port=5000, debug=False, use_reloader=False)).start()

    def authenticate_in_thread(self):
        """
        Запускает процесс авторизации в отдельном потоке.
        """
        auth_thread = threading.Thread(target=self.authenticate)
        auth_thread.daemon = True  # Поток завершится, если основной поток завершится
        auth_thread.start()

    def prolong_token(self):
        url = "https://api.tanki.su/wot/auth/prolongate/"

        data = {
            "application_id": self.application_id,
            "access_token": self.token
        }

        try:
            response = requests.post(url, data=data)
            response.raise_for_status()
            data = response.json()

            if data.get("status") == "ok":
                self.token = data["data"]["access_token"]
                self.token_expiration = data["data"]["expires_at"]
                print(f"✅ Новый токен сохранён.")
                return True
            else:
                print(f"⚠️ Ошибка от API: {data}")
                return False
        except requests.exceptions.RequestException as e:
            print(f"❌ Ошибка запроса: {e}")
            return False

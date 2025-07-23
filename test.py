import time
from classes import stats as stats_cpp_module
import random

start = time.time()

for _ in range(1_000_000):
    battles = random.randint(1, 10_000)
    losses = random.randint(0, battles)
    wins = battles - losses

    first = {
        "credits": random.randint(0, 1_000_000),
        "gold": random.randint(0, 1_000),
        "exp_battle": random.randint(0, 50_000),
        "exp_free": random.randint(0, 10_000),
        "battles": battles,
        "wins": wins,
        "losses": losses,
        "totalDamage": random.randint(0, 2_000_000)
    }

    new = dict(first)  # типа update без изменений
    diff = {k: new[k] - first[k] for k in new}

    avg_dmg = diff["totalDamage"] // diff["battles"] if diff["battles"] > 0 else -1
    avg_exp = diff["exp_battle"] // diff["battles"] if diff["battles"] > 0 else -1
    winrate = round((diff["wins"] / diff["battles"]) * 100.0, 2) if diff["battles"] > 0 else -1.0

end = time.time()
print(f"Python словарь: {end - start:.4f} сек")



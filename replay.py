import ctypes
from parse_replay import ReplayParser

if __name__ == "__main__":
    replay_path = "data (1).replay"
    parser = ReplayParser(replay_path)
    unique_players = parser.parse_replay()

    print(unique_players)
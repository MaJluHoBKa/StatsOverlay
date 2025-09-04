import struct
from google.protobuf.internal import decoder

class ReplayParser:
    def __init__(self, path):
        self.path = path
        self.client_hash = None
        self.client_version = None
        self.all_players = []

    @staticmethod
    def read_quirky_length(data, pos):
        first = data[pos]
        pos += 1
        if first < 0x80:
            return first, pos
        val = first & 0x7F
        for shift in (7, 14, 21):
            val |= data[pos] << shift
            pos += 1
        return val, pos

    @staticmethod
    def parse_player_manual(data):
        pos = 0
        player = {"nickname": None, "team_number": None, "account_id": None,
                  "clan_tag": None, "platoon_number": None, "tank_id": None}

        while pos < len(data):
            try:
                tag, pos = decoder._DecodeVarint(data, pos)
            except Exception:
                break
            field_number = tag >> 3
            wire_type = tag & 0x7

            if field_number == 2 and wire_type == 2:  # field_2 -> tank info
                length, pos = decoder._DecodeVarint(data, pos)
                value = data[pos:pos+length]
                pos += length
                if len(value) >= 2:
                    # первое 16-битное число little-endian считается Tank ID
                    player["tank_id"] = int.from_bytes(value[0:2], 'little')

            elif field_number == 3 and wire_type == 2:  # nickname
                length, pos = decoder._DecodeVarint(data, pos)
                player["nickname"] = data[pos:pos+length].decode('utf-8', errors='ignore')
                pos += length

            elif field_number == 4 and wire_type == 0:  # team_number
                val, pos = decoder._DecodeVarint(data, pos)
                player["team_number"] = val

            elif field_number == 7 and wire_type == 0:  # account_id
                val, pos = decoder._DecodeVarint(data, pos)
                player["account_id"] = val

            elif field_number == 8 and wire_type == 2:  # clan_tag
                length, pos = decoder._DecodeVarint(data, pos)
                player["clan_tag"] = data[pos:pos+length].decode('utf-8', errors='ignore')
                pos += length

            elif field_number == 11 and wire_type == 0:  # platoon_number
                val, pos = decoder._DecodeVarint(data, pos)
                player["platoon_number"] = val

            else:
                # Пропуск неизвестного поля
                if wire_type == 0:
                    _, pos = decoder._DecodeVarint(data, pos)
                elif wire_type == 2:
                    length, pos = decoder._DecodeVarint(data, pos)
                    pos += length
                else:
                    pos += 1

        return player

    @classmethod
    def parse_update_arena_manual(cls, payload):
        pos = 0
        all_players = []

        while pos < len(payload):
            try:
                tag, pos_new = decoder._DecodeVarint(payload, pos)
            except Exception:
                break
            pos = pos_new
            field_number = tag >> 3
            wire_type = tag & 0x7

            if field_number == 1 and wire_type == 2:  # UpdateArena.players block
                length, pos_new = decoder._DecodeVarint(payload, pos)
                pos = pos_new
                block = payload[pos:pos+length]
                pos += length

                # Первый уровень вложенности: Players
                inner_pos = 0
                while inner_pos < len(block):
                    try:
                        inner_tag, inner_pos_new = decoder._DecodeVarint(block, inner_pos)
                    except Exception:
                        break
                    inner_pos = inner_pos_new
                    inner_field_number = inner_tag >> 3
                    inner_wire_type = inner_tag & 0x7

                    if inner_field_number == 1 and inner_wire_type == 2:  # Players.players repeated
                        plength, inner_pos_new = decoder._DecodeVarint(block, inner_pos)
                        inner_pos = inner_pos_new
                        player_bytes = block[inner_pos:inner_pos+plength]
                        inner_pos += plength
                        player = cls.parse_player_manual(player_bytes)
                        all_players.append(player)
                    else:
                        if inner_wire_type == 0:
                            _, inner_pos = decoder._DecodeVarint(block, inner_pos)
                        elif inner_wire_type == 2:
                            l2, inner_pos = decoder._DecodeVarint(block, inner_pos)
                            inner_pos += l2
                        else:
                            inner_pos += 1
            else:
                # Пропуск неизвестного поля
                if wire_type == 0:
                    _, pos = decoder._DecodeVarint(payload, pos)
                elif wire_type == 2:
                    length, pos_new = decoder._DecodeVarint(payload, pos)
                    pos = pos_new + length
                else:
                    pos += 1
        return all_players

    @classmethod
    def parse_entity_method(cls, payload):
        players = []
        view = payload
        if len(view) < 12:
            return players

        # header1
        view = view[4:]
        # sub_type
        sub_type = struct.unpack_from("<I", view, 0)[0]
        view = view[4:]
        if sub_type != 50:
            return players

        inner_length = struct.unpack_from("<I", view, 0)[0]
        view = view[4:]

        try:
            field_number, pos = decoder._DecodeVarint(view, 0)
            view = view[pos:]
        except Exception:
            return players

        message_length, pos = cls.read_quirky_length(view, 0)
        proto_bytes = view[pos:pos+message_length]

        players = cls.parse_update_arena_manual(proto_bytes)
        return players

    def parse_replay(self):
        with open(self.path, "rb") as f:
            data = f.read()

        pos = 0

        # Заголовок
        magic = struct.unpack_from("<I", data, pos)[0]
        if magic != 0x12345678:
            raise ValueError(f"Invalid magic: {magic}")
        pos += 4
        pos += 8  # мусор

        hash_len = data[pos]
        pos += 1
        self.client_hash = data[pos:pos+hash_len].hex()
        pos += hash_len

        ver_len = data[pos]
        pos += 1
        self.client_version = data[pos:pos+ver_len].decode("utf-8")
        pos += ver_len
        pos += 1  # неизвестный байт

        entity_method_count = 0
        self.all_players = []

        while pos + 12 <= len(data):
            length, type_, clock = struct.unpack_from("<IIf", data, pos)
            pos += 12
            payload = data[pos:pos+length]
            pos += length

            if type_ == 8:
                entity_method_count += 1
                try:
                    players = self.parse_entity_method(payload)
                    self.all_players.extend(players)
                except Exception as e:
                    print(f"[WARN] Failed to parse EntityMethod packet #{entity_method_count}: {e}")

        return self._get_unique_nicks()

    def _get_unique_nicks(self):
        # Сортировка по team_number: союзники (2) первыми, противники (1) после
        allies = [p for p in self.all_players if p.get("team_number") == 2]
        enemies = [p for p in self.all_players if p.get("team_number") == 1]
    
        seen_nicks = set()
        unique_players = []
    
        for p in allies + enemies:
            nick = p.get("nickname")
            tank_id = p.get("tank_id")
            team = p.get("team_number")
            if nick and nick not in seen_nicks:
                unique_players.append({"nickname": nick, "tank_id": tank_id, "team": team})
                seen_nicks.add(nick)
        return unique_players

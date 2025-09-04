#include <fstream>
#include <vector>
#include <string>
#include <set>
#include <iostream>
#include <cstdint>
#include <json.hpp>

using json = nlohmann::json;

struct Player
{
    std::string nickname;
    int team_number = 0;
    int tank_id = 0;
    uint64_t account_id = 0;
    std::string clan_tag;
    int platoon_number = 0;
};

// --- Varint decoder ---
uint64_t decodeVarint(const uint8_t *data, size_t &pos, size_t size)
{
    uint64_t result = 0;
    int shift = 0;
    while (pos < size)
    {
        uint8_t byte = data[pos++];
        result |= uint64_t(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0)
            break;
        shift += 7;
    }
    return result;
}

// --- Quirky length ---
uint64_t read_quirky_length(const std::vector<uint8_t> &data, size_t &pos)
{
    if (pos >= data.size())
        return 0;
    uint64_t val = 0;
    int shift = 0;
    while (pos < data.size())
    {
        uint8_t byte = data[pos++];
        val |= uint64_t(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0)
            break;
        shift += 7;
        if (shift > 28) // quirky length максимум 4 байта
            break;
    }
    return val;
}

// --- Парсинг одного игрока ---
Player parse_player_manual(const std::vector<uint8_t> &data)
{
    Player p{};
    size_t pos = 0;
    while (pos < data.size())
    {
        uint64_t tag = decodeVarint(data.data(), pos, data.size());
        int field_number = tag >> 3;
        int wire_type = tag & 0x7;

        if (field_number == 2 && wire_type == 2)
        { // tank_id
            uint64_t len = decodeVarint(data.data(), pos, data.size());
            if (len >= 2 && pos + 1 < data.size())
            {
                p.tank_id = data[pos] | (data[pos + 1] << 8);
            }
            pos += len;
        }
        else if (field_number == 3 && wire_type == 2)
        { // nickname
            uint64_t len = decodeVarint(data.data(), pos, data.size());
            if (pos + len <= data.size())
                p.nickname = std::string((char *)&data[pos], len);
            pos += len;
        }
        else if (field_number == 4 && wire_type == 0)
        { // team_number
            p.team_number = int(decodeVarint(data.data(), pos, data.size()));
        }
        else if (field_number == 7 && wire_type == 0)
        { // account_id
            p.account_id = decodeVarint(data.data(), pos, data.size());
        }
        else if (field_number == 8 && wire_type == 2)
        { // clan_tag
            uint64_t len = decodeVarint(data.data(), pos, data.size());
            if (pos + len <= data.size())
                p.clan_tag = std::string((char *)&data[pos], len);
            pos += len;
        }
        else if (field_number == 11 && wire_type == 0)
        { // platoon_number
            p.platoon_number = int(decodeVarint(data.data(), pos, data.size()));
        }
        else
        {
            if (wire_type == 0)
                decodeVarint(data.data(), pos, data.size());
            else if (wire_type == 2)
                pos += decodeVarint(data.data(), pos, data.size());
            else
                pos++;
        }
    }
    return p;
}

// --- parse UpdateArena.players.players ---
std::vector<Player> parse_update_arena_manual(const std::vector<uint8_t> &payload)
{
    std::vector<Player> all_players;
    size_t pos = 0;

    while (pos < payload.size())
    {
        uint64_t tag = decodeVarint(payload.data(), pos, payload.size());
        int field_number = tag >> 3;
        int wire_type = tag & 0x7;

        if (field_number == 1 && wire_type == 2) // один игрок
        {
            uint64_t len2 = decodeVarint(payload.data(), pos, payload.size());
            if (pos + len2 <= payload.size())
            {
                std::vector<uint8_t> player_block(payload.begin() + pos, payload.begin() + pos + len2);
                Player p = parse_player_manual(player_block);
                all_players.push_back(p);
                pos += len2; // <-- сдвигаем ровно один раз
            }
            else
            {
                pos = payload.size(); // защита от выхода
            }
        }
        else
        {
            // пропуск неизвестного поля
            if (wire_type == 0)
                decodeVarint(payload.data(), pos, payload.size());
            else if (wire_type == 2)
            {
                uint64_t l = decodeVarint(payload.data(), pos, payload.size());
                pos += l;
            }
            else
                pos++;
        }
    }

    return all_players;
}

// --- parse EntityMethod sub_type=50 с отладкой ---
std::vector<Player> parse_entity_method(const std::vector<uint8_t> &payload)
{
    std::vector<Player> players;
    if (payload.size() < 12)
        return players;

    size_t pos = 4; // skip header
    uint32_t sub_type = *(uint32_t *)&payload[pos];
    pos += 4;

    if (sub_type != 50)
        return players;

    uint32_t inner_length = *(uint32_t *)&payload[pos];
    pos += 4;

    std::vector<uint8_t> view(payload.begin() + pos, payload.end());

    std::cout << "[DEBUG] Sub_type=" << sub_type
              << ", payload length=" << payload.size()
              << ", inner_length=" << inner_length
              << ", data offset=" << pos << "\n";

    // Пропускаем первый varint (обычно 1)
    size_t varint_pos = 0;
    // decodeVarint(view.data(), varint_pos, view.size());
    std::cout << "[DEBUG] First varint skipped, next offset=" << varint_pos << "\n";

    // Всё остальное — это proto_bytes
    std::vector<uint8_t> proto_bytes(view.begin(), view.end());

    std::cout << "[DEBUG] proto_bytes size=" << proto_bytes.size() << "\n";

    players = parse_update_arena_manual(proto_bytes);
    if (!players.empty())
        std::cout << "[DEBUG] Found " << players.size() << " players in sub_type 50\n";
    else
        std::cout << "[DEBUG] No players found in sub_type 50\n";

    return players;
}

// --- Parse replay file ---
int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: replay_parser.exe <replay_file>\n";
        return 1;
    }

    std::ifstream f(argv[1], std::ios::binary);
    if (!f.is_open())
    {
        std::cout << "Failed to open file\n";
        return 1;
    }
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(f)), {});
    f.close();

    size_t pos = 0;
    uint32_t magic = *(uint32_t *)&data[pos];
    pos += 4;
    if (magic != 0x12345678)
    {
        std::cout << "Invalid magic\n";
        return 1;
    }
    pos += 8; // мусор
    uint8_t hash_len = data[pos++];
    pos += hash_len;
    uint8_t ver_len = data[pos++];
    pos += ver_len + 1;

    int entity_method_count = 0;

    while (pos + 12 <= data.size())
    {
        uint32_t length = *(uint32_t *)&data[pos];
        pos += 4;
        uint32_t type = *(uint32_t *)&data[pos];
        pos += 4;
        float clock = *(float *)&data[pos];
        pos += 4;
        if (pos + length > data.size())
            break;

        std::vector<uint8_t> payload(data.begin() + pos, data.begin() + pos + length);
        pos += length;

        if (type == 8 && length > 1000)
        {
            entity_method_count++;
            std::cout << "\nEntityMethod packet #" << entity_method_count
                      << ", type=" << type << ", length=" << length << "\n";

            auto players = parse_entity_method(payload);
            if (!players.empty())
            {
                std::cout << "  -> Found " << players.size() << " players\n";
                for (auto &p : players)
                {
                    std::cout << "     " << p.nickname
                              << ", team=" << p.team_number
                              << ", tank_id=" << p.tank_id << "\n";
                }
            }
            else
            {
                std::cout << "  -> No players found\n";
            }
        }
    }

    return 0;
}

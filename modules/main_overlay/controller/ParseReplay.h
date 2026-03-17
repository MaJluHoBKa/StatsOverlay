#pragma once
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <iostream>
#include <cstdint>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

struct PlayerRaw
{
    std::string nickname;
    int team_number = 0;
    int tank_id = 0;
    uint64_t account_id = 0;
    std::string clan_tag;
    int platoon_number = 0;
};

class ReplayParser
{
public:
    explicit ReplayParser(QFile *logFile)
        : m_logFile(logFile)
    {
        if (m_logFile && !m_logFile->isOpen())
            m_logFile->open(QIODevice::Append | QIODevice::Text);
    }

    std::vector<PlayerRaw> parseReplayFile(const std::filesystem::path &path)
    {
        std::vector<PlayerRaw> result;
        log(QString("Opening replay file: %1").arg(QString::fromStdWString(path.wstring())));

        std::ifstream f(path, std::ios::binary);
        if (!f.is_open())
        {
            log("Failed to open replay file");
            return result;
        }

        std::vector<uint8_t> data((std::istreambuf_iterator<char>(f)), {});
        f.close();

        size_t pos = 0;
        if (data.size() < 12)
            return result;

        uint32_t magic = *(uint32_t *)&data[pos];
        pos += 4;
        if (magic != 0x12345678)
        {
            return result;
        }

        pos += 8; // мусор
        if (pos >= data.size())
            return result;
        uint8_t hash_len = data[pos++];
        pos += hash_len;
        if (pos >= data.size())
            return result;
        uint8_t ver_len = data[pos++];
        pos += ver_len + 1;

        int packet_count = 0;
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
                packet_count++;

                auto players = parseEntityMethod(payload);

                for (auto &p : players)
                {
                    if (!p.nickname.empty() && p.team_number != 0)
                        result.push_back(p);
                }
            }
        }

        log(QString("Total players parsed: %1").arg(result.size()));
        return result;
    }

private:
    QFile *m_logFile = nullptr;

    void log(const QString &message)
    {
        if (!m_logFile || !m_logFile->isOpen())
            return;

        QTextStream out(m_logFile);
        out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << " [ReplayParser] " << message << "\n";
    }

    static uint64_t decodeVarint(const uint8_t *data, size_t &pos, size_t size)
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

    PlayerRaw parsePlayerManual(const std::vector<uint8_t> &data)
    {
        PlayerRaw p{};
        size_t pos = 0;

        while (pos < data.size())
        {
            uint64_t tag = decodeVarint(data.data(), pos, data.size());
            int field_number = tag >> 3;
            int wire_type = tag & 0x7;

            switch (field_number)
            {
            case 2: // tank_id
                if (wire_type == 2)
                {
                    uint64_t len = decodeVarint(data.data(), pos, data.size());
                    if (len >= 2 && pos + 1 < data.size())
                        p.tank_id = data[pos] | (data[pos + 1] << 8);
                    pos += len;
                }
                else
                    skipField(wire_type, data, pos);
                break;

            case 3: // nickname
                if (wire_type == 2)
                {
                    uint64_t len = decodeVarint(data.data(), pos, data.size());
                    if (pos + len <= data.size())
                        p.nickname = std::string((char *)&data[pos], len);
                    pos += len;
                }
                else
                    skipField(wire_type, data, pos);
                break;

            case 4: // team_number
                if (wire_type == 0)
                    p.team_number = int(decodeVarint(data.data(), pos, data.size()));
                else
                    skipField(wire_type, data, pos);
                break;

            case 7: // account_id
                if (wire_type == 0)
                    p.account_id = decodeVarint(data.data(), pos, data.size());
                else
                    skipField(wire_type, data, pos);
                break;

            case 8: // clan_tag
                if (wire_type == 2)
                {
                    uint64_t len = decodeVarint(data.data(), pos, data.size());
                    if (pos + len <= data.size())
                        p.clan_tag = std::string((char *)&data[pos], len);
                    pos += len;
                }
                else
                    skipField(wire_type, data, pos);
                break;

            case 11: // platoon_number
                if (wire_type == 0)
                    p.platoon_number = int(decodeVarint(data.data(), pos, data.size()));
                else
                    skipField(wire_type, data, pos);
                break;

            default:
                skipField(wire_type, data, pos);
                break;
            }
        }

        return p;
    }

    void skipField(int wire_type, const std::vector<uint8_t> &data, size_t &pos)
    {
        if (wire_type == 0)
            decodeVarint(data.data(), pos, data.size());
        else if (wire_type == 2)
        {
            uint64_t len = decodeVarint(data.data(), pos, data.size());
            pos += len <= data.size() - pos ? len : data.size() - pos;
        }
        else if (wire_type == 1)
            pos += 8 <= data.size() - pos ? 8 : data.size() - pos;
        else if (wire_type == 5)
            pos += 4 <= data.size() - pos ? 4 : data.size() - pos;
        else
            pos = data.size();
    }

    std::vector<PlayerRaw> parseUpdateArenaManual(const std::vector<uint8_t> &payload)
    {
        std::vector<PlayerRaw> all_players;
        size_t pos = 0;

        while (pos < payload.size())
        {
            uint64_t tag = decodeVarint(payload.data(), pos, payload.size());
            int field_number = tag >> 3;
            int wire_type = tag & 0x7;

            if (field_number == 1 && wire_type == 2)
            {
                uint64_t length = decodeVarint(payload.data(), pos, payload.size());
                if (pos + length > payload.size())
                    break;

                std::vector<uint8_t> block(payload.begin() + pos, payload.begin() + pos + length);
                pos += length;

                size_t inner_pos = 0;
                while (inner_pos < block.size())
                {
                    uint64_t inner_tag = decodeVarint(block.data(), inner_pos, block.size());
                    int inner_field_number = inner_tag >> 3;
                    int inner_wire_type = inner_tag & 0x7;

                    if (inner_field_number == 1 && inner_wire_type == 2)
                    {
                        uint64_t plength = decodeVarint(block.data(), inner_pos, block.size());
                        if (inner_pos + plength > block.size())
                            break;

                        std::vector<uint8_t> player_bytes(block.begin() + inner_pos, block.begin() + inner_pos + plength);
                        inner_pos += plength;

                        all_players.push_back(parsePlayerManual(player_bytes));
                    }
                    else
                    {
                        skipField(inner_wire_type, block, inner_pos);
                    }
                }
            }
            else
            {
                skipField(wire_type, payload, pos);
            }
        }

        return all_players;
    }

    std::vector<PlayerRaw> parseEntityMethod(const std::vector<uint8_t> &payload)
    {
        std::vector<PlayerRaw> players;
        if (payload.size() < 12)
            return players;

        size_t pos = 4;
        uint32_t sub_type = *(uint32_t *)&payload[pos];
        pos += 4;

        if (sub_type != 55)
            return players;

        pos += 8; // inner_length + unknown
        if (pos >= payload.size())
            return players;

        std::vector<uint8_t> view(payload.begin() + pos, payload.end());
        size_t varint_pos = 0;
        decodeVarint(view.data(), varint_pos, view.size());

        std::vector<uint8_t> proto_bytes(view.begin() + varint_pos, view.end());
        players = parseUpdateArenaManual(proto_bytes);
        return players;
    }
};

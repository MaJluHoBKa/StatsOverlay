#include "ReplayTracker.h"
#include <QFile>
#include <QDebug>
#include <QtEndian>
#include <zlib.h>
#include <cstring>

namespace
{
    constexpr quint32 PKT_ENTITY_PROPERTY = 8;
    constexpr quint32 PKT_ENTITY_APPEAR = 5;
    constexpr quint32 PKT_ENTITY_ENTER = 10;
    constexpr quint32 PKT_ENTITY_CREATE = 32;
    constexpr quint32 PROP_HP_UPDATE = 2;
    constexpr quint32 PROP_HIT_EVENT = 9;
    constexpr quint32 PROP_PLAYER_INFO = 54;
    constexpr int T5_HP_OFFSET = 51;
    constexpr int HIT_PEN = 3, HIT_BLOCKED = 4, HIT_RICOCHET = 2, HIT_NO_PEN = 1;
    static constexpr bool isHitBlocked(int r)
    {
        return r == HIT_NO_PEN || r == HIT_RICOCHET || r == HIT_BLOCKED;
    }
    const char *HIT_NAMES[] = {"blocked", "pen", "crit", "pen+fire", "ricochet"};
}

ReplayTracker::ReplayTracker(QObject *parent) : QObject(parent) {}

void ReplayTracker::reset()
{
    m_lastFileSize = 0;
    m_selfEid = -1;
    m_worldEid = 0;
    m_entityToNick.clear();
    m_entityToTeam.clear();
    m_entityToVehicle.clear();
    m_nickToVehicle.clear();
    m_aidToVehicle.clear();
    m_nickToAid.clear();
    m_maxHp.clear();
    m_dmgRcvd.clear();
    m_hitResults.clear();
    m_events.clear();
    m_totalDamage = 0;
    m_incomingEvents.clear();
    m_totalReceived = 0;
    m_blockedEvents.clear();
    m_totalBlocked = 0;
    m_assistEvents.clear();
    m_totalAssist = 0;
    m_battleHistory.clear();
    m_battleStats = BattleStats{};
}

bool ReplayTracker::update(const QString &replayPath, const QString &selfNick)
{
    m_replayPath = replayPath;
    QFile file(replayPath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly))
        return false;
    qint64 size = file.size();
    if (size == m_lastFileSize)
    {
        file.close();
        return false;
    }
    m_lastFileSize = size;
    QByteArray data = file.readAll();
    file.close();
    if (data.size() < 200)
        return false;
    parseFile(data, selfNick);
    return true;
}

// ══════════════════════════════════════════════════════════════════════════
//  parseFile
// ══════════════════════════════════════════════════════════════════════════
void ReplayTracker::parseFile(const QByteArray &data, const QString &selfNick)
{
    int evEnd = data.size();

    // ── ШАГ A: early region (0x40+) для большого prop=55 (updateArena) ───────
    // Большой prop=55 появляется в самом начале файла ДО первого zlib-блока.
    // Содержит eid + vcd + nick для ВСЕХ 14 игроков.
    {
        int evStartEarly = -1;
        for (int i = 0x40; i < qMin(evEnd - 12, 0x2000); ++i)
        {
            quint32 sz = u32(data, i), typ = u32(data, i + 4);
            float gt = 0;
            std::memcpy(&gt, data.constData() + i + 8, 4);
            if (sz > 0 && sz < 50000 && typ > 0 && typ < 100 && gt >= 0.f && gt < 3600.f)
            {
                int nxt = i + 12 + (int)sz;
                if (nxt + 12 <= evEnd)
                {
                    quint32 sz2 = u32(data, nxt), typ2 = u32(data, nxt + 4);
                    float gt2 = 0;
                    std::memcpy(&gt2, data.constData() + nxt + 8, 4);
                    if (sz2 > 0 && sz2 < 50000 && typ2 > 0 && typ2 < 100 && gt2 >= 0.f && gt2 < 3600.f)
                    {
                        evStartEarly = i;
                        break;
                    }
                }
            }
        }
        if (evStartEarly >= 0)
        {
            QByteArray earlyRegion = data.mid(evStartEarly, evEnd - evStartEarly);
            buildEntityMapFromArena(earlyRegion);
        }
    }

    // ── ШАГ B: оригинальная логика — zlib + основной поток ───────────────────
    QVector<int> zlibOff = findZlibOffsets(data);
    if (zlibOff.isEmpty())
        return;

    if (zlibOff.size() >= 2 && m_entityToNick.isEmpty())
        extractEntityMapFromResult(data, zlibOff[1]);

    int evStart = zlibOff[0];
    {
        bool found = false;
        for (int i = zlibOff[0]; i < evEnd - 12 && !found; ++i)
        {
            quint32 sz = u32(data, i), typ = u32(data, i + 4);
            float gt = 0;
            std::memcpy(&gt, data.constData() + i + 8, 4);
            if (sz > 0 && sz < 50000 && typ > 0 && typ < 100 && gt >= 0.f && gt < 3600.f)
            {
                int nxt = i + 12 + (int)sz;
                if (nxt + 12 <= evEnd)
                {
                    quint32 sz2 = u32(data, nxt), typ2 = u32(data, nxt + 4);
                    float gt2 = 0;
                    std::memcpy(&gt2, data.constData() + nxt + 8, 4);
                    if (sz2 > 0 && sz2 < 50000 && typ2 > 0 && typ2 < 100 && gt2 >= 0.f && gt2 < 3600.f)
                    {
                        evStart = i;
                        found = true;
                    }
                }
            }
        }
    }

    QByteArray region = data.mid(evStart, evEnd - evStart);
    if (region.isEmpty())
        return;

    m_events.clear();
    m_hitResults.clear();
    m_totalDamage = 0;
    m_incomingEvents.clear();
    m_totalReceived = 0;
    m_blockedEvents.clear();
    m_totalBlocked = 0;
    m_assistEvents.clear();
    m_totalAssist = 0;
    m_battleHistory.clear();

    parseEvents(region, selfNick);
}

// ══════════════════════════════════════════════════════════════════════════
//  buildEntityMapFromArena — парсит большой prop=55, заполняет eid+vcd+nick
// ══════════════════════════════════════════════════════════════════════════
void ReplayTracker::buildEntityMapFromArena(const QByteArray &region)
{
    QVector<Packet> pkts = iterPackets(region);
    for (const Packet &p : pkts)
    {
        if (p.type != PKT_ENTITY_PROPERTY || p.payload.size() < 1012)
            continue;
        quint32 prop = u32(p.payload, 4);
        quint32 isz = u32(p.payload, 8);
        if (prop != 55 || isz < 1000 || p.payload.size() < 12 + (int)isz)
            continue;

        const QByteArray inner = p.payload.mid(12, isz);
        int pos = 0, ilen = inner.size();

        while (pos < ilen - 5)
        {
            if ((quint8)inner[pos] != 0x0a)
            {
                ++pos;
                continue;
            }
            ++pos;
            int vp = pos;
            quint64 smLen = varint(inner, vp);
            if (smLen < 10 || smLen > 500 || vp + (int)smLen > ilen)
            {
                ++pos;
                continue;
            }
            const QByteArray sm = inner.mid(vp, smLen);
            pos = vp + smLen;

            if ((quint8)sm[0] != 0x08)
                continue;
            int sp = 1;
            quint64 eid = varint(sm, sp);
            if (eid <= 0xFFFF || eid > 0x7FFFFFFF)
                continue;

            QString nick;
            int vcd = 0;
            while (sp < sm.size())
            {
                quint8 tb = (quint8)sm[sp++];
                int fn = tb >> 3, wt = tb & 7;
                if (wt == 0)
                {
                    varint(sm, sp);
                }
                else if (wt == 2)
                {
                    quint64 ln = varint(sm, sp);
                    if (sp + (int)ln > sm.size())
                        break;
                    if (fn == 2 && ln >= 2)
                        vcd = (quint8)sm[sp] | ((quint8)sm[sp + 1] << 8);
                    else if (fn == 3 && ln >= 3 && ln <= 50)
                        nick = QString::fromUtf8(sm.constData() + sp, (int)ln);
                    sp += (int)ln;
                }
                else if (wt == 1)
                {
                    sp += 8;
                }
                else if (wt == 5)
                {
                    sp += 4;
                }
                else
                    break;
            }
            if (nick.isEmpty())
                continue;
            int ieid = (int)eid;
            if (!m_entityToNick.contains(ieid))
                m_entityToNick[ieid] = nick;
            if (vcd > 0 && !m_entityToVehicle.contains(ieid))
                m_entityToVehicle[ieid] = vcd;
            qDebug() << "[RT55big] eid=" << Qt::hex << ieid
                     << "nick=" << nick << "vcd=" << vcd;
        }
        qDebug() << "[RT55big] parsed: entityToNick=" << m_entityToNick.size()
                 << "entityToVehicle=" << m_entityToVehicle.size();
        break; // только первый большой prop=55
    }
}

// ══════════════════════════════════════════════════════════════════════════
//  parseEvents — оригинальный код без изменений
// ══════════════════════════════════════════════════════════════════════════
void ReplayTracker::parseEvents(const QByteArray &region, const QString &selfNick)
{
    QVector<Packet> pkts = iterPackets(region);

    // Проход -1: ReplayParser
    if (m_nickToVehicle.isEmpty() && !m_replayPath.isEmpty())
    {
        ReplayParser rp(nullptr);
        auto players = rp.parseReplayFile(m_replayPath.toStdWString());
        for (const auto &pl : players)
        {
            if (pl.nickname.empty() || pl.tank_id == 0)
                continue;
            QString nick = QString::fromStdString(pl.nickname);
            m_nickToVehicle[nick] = pl.tank_id;
        }
    }

    // Проход 0: type=13
    for (const Packet &p : pkts)
    {
        if (p.type != 13 || p.payload.size() < 20)
            continue;
        parseBattleResultPacket(p.payload);
        qDebug() << "[ReplayTracker] type=13 parsed:"
                 << "entityToNick=" << m_entityToNick.size()
                 << "aidToVehicle=" << m_aidToVehicle.size();
        break;
    }

    // Проход 1: entity map + selfEid
    if (m_selfEid < 0)
    {
        if (m_worldEid == 0)
        {
            for (const Packet &p : pkts)
            {
                if (p.type != PKT_ENTITY_PROPERTY || p.payload.size() < 52)
                    continue;
                quint32 prop = u32(p.payload, 4), isz = u32(p.payload, 8);
                if (prop != PROP_PLAYER_INFO || (int)isz < 40)
                    continue;
                if (p.payload.size() < 12 + (int)isz)
                    continue;
                QByteArray inn = p.payload.mid(12, isz);
                int nl = (quint8)inn[38];
                if (nl < 3 || 39 + nl > inn.size())
                    continue;
                QString nick = QString::fromUtf8(inn.constData() + 39, nl);
                bool ok = nick.length() >= 3;
                for (QChar c : nick)
                    if (c.unicode() < 32 || c.unicode() > 126)
                    {
                        ok = false;
                        break;
                    }
                if (ok)
                {
                    m_worldEid = (int)u32(p.payload, 0);
                    break;
                }
            }
            if (m_worldEid == 0)
                return;
        }
        const int worldEid = m_worldEid;

        // prop=55/54 chain
        {
            int pendingEid = -1;
            float pendingT = -1.f;
            for (const Packet &p : pkts)
            {
                if (p.type != PKT_ENTITY_PROPERTY || p.payload.size() < 12)
                    continue;
                if ((int)u32(p.payload, 0) != worldEid)
                    continue;
                quint32 prop = u32(p.payload, 4), isz = u32(p.payload, 8);
                if (prop == 55 && isz >= 4 && isz <= 14 && p.payload.size() >= (int)isz + 12)
                {
                    QByteArray inn = p.payload.mid(12, isz);
                    const int vstarts[] = {5, 4, 1, 0};
                    for (int vs : vstarts)
                    {
                        if (vs >= inn.size())
                            continue;
                        int vp = vs;
                        quint64 eid = varint(inn, vp);
                        if (eid > 0xFFFF)
                        {
                            pendingEid = (int)eid;
                            pendingT = p.gameTime;
                            break;
                        }
                    }
                }
                else if (prop == PROP_PLAYER_INFO && (int)isz >= 40 && p.payload.size() >= 12 + (int)isz)
                {
                    QByteArray inn = p.payload.mid(12, isz);
                    int nl = (quint8)inn[38];
                    if (nl > 0 && 39 + nl <= inn.size())
                    {
                        QString nick = QString::fromUtf8(inn.constData() + 39, nl);
                        if (nick.length() >= 3 && nick.length() <= 50)
                        {
                            if (inn.size() >= 34 && !m_nickToAid.contains(nick))
                            {
                                quint32 aid = qFromLittleEndian<quint32>((uchar *)inn.constData() + 30);
                                if (aid > 0)
                                    m_nickToAid[nick] = aid;
                            }
                            if (pendingEid > 0 && (p.gameTime - pendingT) < 1.0f && !m_entityToNick.contains(pendingEid))
                                m_entityToNick[pendingEid] = nick;
                        }
                    }
                    pendingEid = -1;
                }
            }
        }

        // type=5
        for (const Packet &p : pkts)
        {
            if (p.type != PKT_ENTITY_APPEAR || p.payload.size() < 64)
                continue;
            int eid = (int)u32(p.payload, 0), nl = (quint8)p.payload[62];
            if (nl > 0 && 63 + nl <= p.payload.size())
            {
                QString nick = QString::fromUtf8(p.payload.constData() + 63, nl);
                if (nick.length() >= 3 && nick.length() <= 50)
                {
                    if (!m_entityToNick.contains(eid))
                        m_entityToNick[eid] = nick;
                    int tidOff = 63 + nl + 1;
                    if (!m_entityToVehicle.contains(eid) && tidOff + 1 < p.payload.size())
                    {
                        int tid = (quint8)p.payload[tidOff] | ((quint8)p.payload[tidOff + 1] << 8);
                        if (tid > 0)
                            m_entityToVehicle[eid] = tid;
                    }
                }
            }
        }

        // selfEid
        for (auto it = m_entityToNick.begin(); it != m_entityToNick.end(); ++it)
            if (it.value() == selfNick)
            {
                m_selfEid = it.key();
                break;
            }

        // zip prop54+type10
        {
            QVector<QString> nicks;
            QVector<int> eids;
            for (const Packet &p : pkts)
            {
                if (p.payload.size() < 4)
                    continue;
                if ((int)u32(p.payload, 0) != worldEid)
                    continue;
                if (p.type == PKT_ENTITY_PROPERTY && p.payload.size() >= 12)
                {
                    quint32 prop = u32(p.payload, 4), isz = u32(p.payload, 8);
                    if (prop == PROP_PLAYER_INFO && (int)isz >= 40 && p.payload.size() >= 12 + (int)isz)
                    {
                        QByteArray inn = p.payload.mid(12, isz);
                        int nl = (quint8)inn[38];
                        if (nl > 0 && 39 + nl <= inn.size())
                        {
                            QString nick = QString::fromUtf8(inn.constData() + 39, nl);
                            if (nick.length() >= 3 && nick.length() <= 50)
                                nicks.append(nick);
                        }
                    }
                }
                else if (p.type == PKT_ENTITY_ENTER && p.payload.size() >= 12)
                {
                    int eid = (int)u32(p.payload, 8);
                    if (eid > 0)
                        eids.append(eid);
                }
            }
            int n = qMin(nicks.size(), eids.size());
            for (int i = 0; i < n; ++i)
            {
                if (!m_entityToNick.contains(eids[i]))
                    m_entityToNick[eids[i]] = nicks[i];
                if (nicks[i] == selfNick && m_selfEid < 0)
                    m_selfEid = eids[i];
            }
        }
        if (m_selfEid < 0)
            qDebug() << "[ReplayTracker] selfEid not found for" << selfNick;
    }

    // Линкуем eid→vcd через nick
    for (auto it = m_nickToAid.cbegin(); it != m_nickToAid.cend(); ++it)
    {
        if (!m_nickToVehicle.contains(it.key()))
        {
            int vcd = m_aidToVehicle.value(it.value(), 0);
            if (vcd > 0)
                m_nickToVehicle[it.key()] = vcd;
        }
    }
    if (!m_nickToVehicle.isEmpty())
    {
        for (auto it = m_entityToNick.cbegin(); it != m_entityToNick.cend(); ++it)
        {
            int eid = it.key();
            if (!m_entityToVehicle.contains(eid))
            {
                int tid = m_nickToVehicle.value(it.value(), 0);
                if (tid > 0)
                    m_entityToVehicle[eid] = tid;
            }
        }
    }
    qDebug() << "[ReplayTracker] entity map: nick=" << m_entityToNick.size()
             << "vcd=" << m_entityToVehicle.size() << "selfEid=" << Qt::hex << m_selfEid;

    // maxHp
    for (const Packet &p : pkts)
    {
        if (p.type == PKT_ENTITY_APPEAR && p.payload.size() > T5_HP_OFFSET + 1)
        {
            int eid = (int)u32(p.payload, 0), hp = (int)u16(p.payload, T5_HP_OFFSET);
            if (hp > 0 && !m_maxHp.contains(eid))
                m_maxHp[eid] = hp;
        }
    }

    // Проход 3: prop=9 hit results
    for (const Packet &p : pkts)
    {
        if (p.type != PKT_ENTITY_PROPERTY || p.payload.size() < 12)
            continue;
        int teid = (int)u32(p.payload, 0);
        quint32 prop = u32(p.payload, 4), isz = u32(p.payload, 8);
        if (prop != PROP_HIT_EVENT || (int)isz < 10 || p.payload.size() < 12 + (int)isz)
            continue;
        QByteArray inn = p.payload.mid(12, isz);
        int seid = (int)u32(inn, 0);
        if (seid == m_selfEid)
            m_hitResults[HitKey{(int)(p.gameTime * 1000.f), seid, teid}] = (quint8)inn[9];
    }

    // Проход 3.5: prop=9 blocked + assist
    if (m_selfEid >= 0)
    {
        const int selfTeam = m_entityToTeam.value(m_selfEid, -1);
        for (const Packet &p : pkts)
        {
            if (p.type != PKT_ENTITY_PROPERTY || p.payload.size() < 12)
                continue;
            quint32 prop = u32(p.payload, 4), isz = u32(p.payload, 8);
            if (prop != PROP_HIT_EVENT || (int)isz < 10 || p.payload.size() < 12 + (int)isz)
                continue;
            QByteArray inn = p.payload.mid(12, isz);
            int shooter = (int)u32(inn, 0), target = (int)u32(inn, 4), result = (quint8)inn[9];

            if (target == m_selfEid && isHitBlocked(result) && (int)isz >= 20)
            {
                int dmgBlocked = (int)u16(inn, 18);
                if (dmgBlocked > 0)
                {
                    m_totalBlocked += dmgBlocked;
                    BlockedEvent bev;
                    bev.time_s = p.gameTime;
                    bev.attacker = m_entityToNick.value(shooter, QString("eid_%1").arg(shooter, 0, 16));
                    bev.damage_blocked = dmgBlocked;
                    bev.total_blocked = m_totalBlocked;
                    m_blockedEvents.append(bev);
                    qDebug() << "[ReplayTracker] blocked" << dmgBlocked << "from" << bev.attacker;
                }
            }
            if (selfTeam >= 0 && shooter != m_selfEid && (int)isz >= 21)
            {
                int sTeam = m_entityToTeam.value(shooter, -1), tTeam = m_entityToTeam.value(target, -1);
                int spotFlag = (quint8)inn[20], dmg = (int)u16(inn, 18);
                if (sTeam == selfTeam && tTeam != selfTeam && spotFlag == 1 && dmg > 0)
                {
                    m_totalAssist += dmg;
                    AssistEvent aev;
                    aev.time_s = p.gameTime;
                    aev.ally = m_entityToNick.value(shooter, QString("eid_%1").arg(shooter, 0, 16));
                    aev.target = m_entityToNick.value(target, QString("eid_%1").arg(target, 0, 16));
                    aev.damage = dmg;
                    aev.total_assist = m_totalAssist;
                    m_assistEvents.append(aev);
                }
            }
        }
    }

    // Проход 4: prop=2 → damage / incoming
    QMap<int, int> lastHp = m_maxHp;
    for (const Packet &p : pkts)
    {
        if (p.type == PKT_ENTITY_APPEAR && p.payload.size() > T5_HP_OFFSET + 1)
        {
            int eid = (int)u32(p.payload, 0), hp = (int)u16(p.payload, T5_HP_OFFSET);
            if (hp > 0)
                lastHp[eid] = hp;
            continue;
        }
        if (p.type != PKT_ENTITY_PROPERTY || p.payload.size() < 12)
            continue;
        int teid = (int)u32(p.payload, 0);
        quint32 prop = u32(p.payload, 4), isz = u32(p.payload, 8);
        if (prop != PROP_HP_UPDATE || (int)isz < 6 || p.payload.size() < 12 + (int)isz)
            continue;
        QByteArray inn = p.payload.mid(12, isz);
        int curHp = (int)u16(inn, 0), seid = (int)u32(inn, 2);
        int flag = (inn.size() > 6) ? (quint8)inn[6] : 0;
        int prevHp = lastHp.value(teid, -1);
        lastHp[teid] = curHp;
        if (flag == 1 || prevHp < 0)
            continue;
        int dmg = prevHp - curHp;
        if (dmg <= 0)
            continue;
        QString evType = (flag == 2) ? "fire" : "shot";

        if (seid == m_selfEid)
        {
            QString targetName = m_entityToNick.value(teid, QString("eid_%1").arg(teid, 0, 16));
            int tMs = (int)(p.gameTime * 1000.f);
            quint8 hc = m_hitResults.value(HitKey{tMs, m_selfEid, teid}, 0xFF);
            QString hname = (hc <= 4) ? QString(HIT_NAMES[hc]) : QString();
            m_totalDamage += dmg;
            DamageEvent ev;
            ev.time_s = p.gameTime;
            ev.shooter = selfNick;
            ev.target = targetName;
            ev.damage = dmg;
            ev.hp_after = curHp;
            ev.hit_result = hname;
            ev.event_type = evType;
            ev.total_damage = m_totalDamage;
            ev.vehicle_cd = m_entityToVehicle.value(teid, 0);
            m_events.append(ev);
        }
        if (teid == m_selfEid && m_selfEid >= 0)
        {
            QString attackerName = m_entityToNick.value(seid, QString("eid_%1").arg(seid, 0, 16));
            m_totalReceived += dmg;
            IncomingEvent iev;
            iev.time_s = p.gameTime;
            iev.attacker = attackerName;
            iev.damage = dmg;
            iev.hp_after = curHp;
            iev.event_type = evType;
            iev.total_received = m_totalReceived;
            iev.vehicle_cd = m_entityToVehicle.value(seid, 0);
            m_incomingEvents.append(iev);
        }
    }

    // k-way merge → battleHistory
    {
        m_battleHistory.clear();
        m_battleHistory.reserve(m_events.size() + m_incomingEvents.size() + m_blockedEvents.size() + m_assistEvents.size());
        int di = 0, ii = 0, bi = 0, ai = 0;
        const int dn = m_events.size(), in_ = m_incomingEvents.size(), bn = m_blockedEvents.size(), an = m_assistEvents.size();
        int cumDmg = 0, cumRcv = 0, cumBlk = 0, cumAst = 0;
        while (di < dn || ii < in_ || bi < bn || ai < an)
        {
            float td = (di < dn) ? m_events[di].time_s : 1e9f;
            float ti = (ii < in_) ? m_incomingEvents[ii].time_s : 1e9f;
            float tb = (bi < bn) ? m_blockedEvents[bi].time_s : 1e9f;
            float ta = (ai < an) ? m_assistEvents[ai].time_s : 1e9f;
            float tmin = qMin(qMin(td, ti), qMin(tb, ta));
            if (td == tmin && di < dn)
            {
                const DamageEvent &e = m_events[di++];
                cumDmg = e.total_damage;
                m_battleHistory.append(BattleEvent::fromDamage(e, cumRcv, cumBlk, cumAst));
            }
            else if (ti == tmin && ii < in_)
            {
                const IncomingEvent &e = m_incomingEvents[ii++];
                cumRcv = e.total_received;
                m_battleHistory.append(BattleEvent::fromIncoming(e, cumDmg, cumBlk, cumAst));
            }
            else if (tb == tmin && bi < bn)
            {
                const BlockedEvent &e = m_blockedEvents[bi++];
                cumBlk = e.total_blocked;
                m_battleHistory.append(BattleEvent::fromBlocked(e, cumDmg, cumRcv, cumAst));
            }
            else if (ai < an)
            {
                const AssistEvent &e = m_assistEvents[ai++];
                cumAst = e.total_assist;
                m_battleHistory.append(BattleEvent::fromAssist(e, cumDmg, cumRcv, cumBlk));
            }
        }
    }
}

// ══════════════════════════════════════════════════════════════════════════
void ReplayTracker::parseBattleResultPacket(const QByteArray &payload)
{
    if (payload.size() < 20)
        return;
    int pklOff = -1;
    for (int i = 4; i < qMin(payload.size() - 2, 20); ++i)
        if ((quint8)payload[i] == 0x80 && (quint8)payload[i + 1] == 0x02)
        {
            pklOff = i;
            break;
        }
    if (pklOff < 0)
        return;
    int pklBodyOff = pklOff + 2, compressedSize = 0, pklEndOff = pklBodyOff;
    quint8 op = (quint8)payload[pklBodyOff];
    if (op == 0x4b && pklBodyOff + 1 < payload.size())
    {
        compressedSize = (quint8)payload[pklBodyOff + 1];
        pklEndOff = pklBodyOff + 2;
    }
    else if (op == 0x4d && pklBodyOff + 2 < payload.size())
    {
        compressedSize = (quint8)payload[pklBodyOff + 1] | ((quint8)payload[pklBodyOff + 2] << 8);
        pklEndOff = pklBodyOff + 3;
    }
    else if (op == 0x4a && pklBodyOff + 4 < payload.size())
    {
        compressedSize = qFromLittleEndian<qint32>((uchar *)payload.constData() + pklBodyOff + 1);
        pklEndOff = pklBodyOff + 5;
    }
    else
    {
        for (int i = pklOff + 3; i < qMin(pklOff + 30, payload.size() - 1); ++i)
        {
            quint8 b0 = (quint8)payload[i], b1 = (quint8)payload[i + 1];
            if (b0 == 0xd4 || (b0 == 0x78 && (b1 == 0x9c || b1 == 0x01 || b1 == 0xda)))
            {
                compressedSize = payload.size() - i - 4;
                pklEndOff = i;
                break;
            }
        }
        if (compressedSize <= 0)
            return;
    }
    int zlibDataOff = -1;
    for (int i = pklEndOff; i < qMin(pklEndOff + 30, payload.size() - 1); ++i)
    {
        if (i + 5 < payload.size())
        {
            quint8 b4 = (quint8)payload[i + 4], b5 = (quint8)payload[i + 5];
            if (b4 == 0x78 && (b5 == 0x01 || b5 == 0x9c || b5 == 0xda))
            {
                zlibDataOff = i;
                break;
            }
        }
    }
    if (zlibDataOff < 0)
        zlibDataOff = pklEndOff + 2;
    QByteArray compressed = payload.mid(zlibDataOff + 4);
    if (compressed.isEmpty())
        return;
    QByteArray dec = zlibDecompress2(compressed, 0);
    if (dec.isEmpty())
        return;
    parseBattleResultProto(dec);
}

QByteArray ReplayTracker::zlibDecompress2(const QByteArray &data, int offset)
{
    if (offset >= data.size())
        return {};
    const int MAX = 4 * 1024 * 1024;
    QByteArray out(MAX, 0);
    z_stream s{};
    s.next_in = (Bytef *)data.constData() + offset;
    s.avail_in = (uInt)(data.size() - offset);
    s.next_out = (Bytef *)out.data();
    s.avail_out = (uInt)MAX;
    if (inflateInit(&s) != Z_OK)
        return {};
    inflate(&s, Z_FINISH);
    inflateEnd(&s);
    return out.left((int)s.total_out);
}

void ReplayTracker::parseBattleResultProto(const QByteArray &dec)
{
    QByteArray pb;
    for (int i = 0; i < dec.size(); ++i)
    {
        quint8 op = (quint8)dec[i];
        int len = 0, hdr = 0;
        if (op == 0x54 && i + 5 <= dec.size())
        {
            len = qFromLittleEndian<qint32>((uchar *)dec.constData() + i + 1);
            hdr = 5;
        }
        else if (op == 0x43 && i + 5 <= dec.size())
        {
            len = qFromLittleEndian<qint32>((uchar *)dec.constData() + i + 1);
            hdr = 5;
        }
        else if (op == 0x8c && i + 1 < dec.size())
        {
            len = (quint8)dec[i + 1];
            hdr = 2;
        }
        else if (op == 0x58 && i + 5 <= dec.size())
        {
            len = qFromLittleEndian<qint32>((uchar *)dec.constData() + i + 1);
            hdr = 5;
        }
        if (len > 200 && i + hdr + len <= dec.size())
        {
            QByteArray c = dec.mid(i + hdr, len);
            if (c.size() > pb.size())
                pb = c;
        }
    }
    if (pb.isEmpty())
        return;
    auto outer = parsePb(pb);
    QMap<quint64, QString> acctToNick;
    for (auto &raw : pbAll(outer, 201))
    {
        auto f = parsePb(raw);
        quint64 acct = pbInt(f, 1);
        QByteArray i2 = pbBytes(f, 2);
        if (!i2.isEmpty())
        {
            auto f2 = parsePb(i2);
            QString nick = pbStr(f2, 1);
            if (!nick.isEmpty() && acct)
                acctToNick[acct] = nick;
        }
    }
    for (auto &raw : pbAll(outer, 301))
    {
        auto f = parsePb(raw);
        int eid = (int)pbInt(f, 1);
        QByteArray sr = pbBytes(f, 2);
        if (sr.isEmpty() || eid <= 0)
            continue;
        auto s = parsePb(sr);
        quint64 acct = pbInt(s, 101);
        int team = (int)pbInt(s, 102);
        int dmgRcvd = (int)pbInt(s, 11);
        QString nick = acctToNick.value(acct);
        if (nick.isEmpty())
            continue;
        int vehicleCD = (int)pbInt(s, 103);
        m_entityToNick[eid] = nick;
        m_entityToTeam[eid] = team;
        if (!m_entityToVehicle.contains(eid))
            m_entityToVehicle[eid] = vehicleCD;
        if (vehicleCD > 0 && acct > 0)
            m_aidToVehicle[(quint32)acct] = vehicleCD;
        m_dmgRcvd[eid] = dmgRcvd;
        if (eid == m_selfEid)
        {
            m_battleStats.dmg_dealt = (int)pbInt(s, 2);
            m_battleStats.dmg_blocked = (int)pbInt(s, 3);
            m_battleStats.shots = (int)pbInt(s, 4);
            m_battleStats.hits = (int)pbInt(s, 5);
            m_battleStats.pens = (int)pbInt(s, 7);
            m_battleStats.dmg_received = (int)pbInt(s, 11);
            m_battleStats.spot_assist = (int)pbInt(s, 9);
            m_battleStats.track_assist = (int)pbInt(s, 10);
            m_battleStats.valid = true;
        }
    }
    if (m_aidToVehicle.isEmpty())
    {
        const char tag101_raw[2] = {(char)0xa8, (char)0x06};
        const QByteArray tag101(tag101_raw, 2);
        int pos = 0;
        while (pos < dec.size() - 12)
        {
            int idx = dec.indexOf(tag101, pos);
            if (idx < 0)
                break;
            int vpos = idx + 2;
            quint64 aid = varint(dec, vpos);
            if (aid == 0 || aid > 0xFFFFFFFFULL)
            {
                pos = idx + 1;
                continue;
            }
            quint64 t2 = varint(dec, vpos);
            if ((t2 >> 3) != 102 || vpos >= dec.size())
            {
                pos = idx + 1;
                continue;
            }
            varint(dec, vpos);
            if (vpos >= dec.size())
            {
                pos = idx + 1;
                continue;
            }
            quint64 t3 = varint(dec, vpos);
            if ((t3 >> 3) != 103 || vpos >= dec.size())
            {
                pos = idx + 1;
                continue;
            }
            quint64 vcd = varint(dec, vpos);
            if (vcd > 0 && vcd < 200000)
                m_aidToVehicle[(quint32)aid] = (int)vcd;
            pos = vpos;
        }
    }
}

bool ReplayTracker::extractEntityMapFromResult(const QByteArray &data, int zlibOffset)
{
    QByteArray dec = zlibDecompress(data, zlibOffset);
    if (dec.size() < 50)
        return false;
    parseBattleResultProto(dec);
    return !m_entityToNick.isEmpty();
}

QVector<int> ReplayTracker::findZlibOffsets(const QByteArray &data)
{
    QVector<int> r;
    for (int i = 0; i < data.size() - 1 && r.size() < 2; ++i)
    {
        quint8 b0 = (quint8)data[i], b1 = (quint8)data[i + 1];
        if (b0 == 0x78 && (b1 == 0x9c || b1 == 0xda || b1 == 0x5e || b1 == 0x01))
        {
            QByteArray d = zlibDecompress(data, i);
            if (d.size() > 100)
                r.append(i);
        }
    }
    return r;
}

QVector<ReplayTracker::Packet> ReplayTracker::iterPackets(const QByteArray &region)
{
    QVector<Packet> r;
    int pos = 0;
    const uchar *raw = (const uchar *)region.constData();
    const int len = region.size();
    while (pos + 12 <= len)
    {
        quint32 sz = qFromLittleEndian<quint32>(raw + pos);
        quint32 typ = qFromLittleEndian<quint32>(raw + pos + 4);
        float gt = 0;
        std::memcpy(&gt, raw + pos + 8, 4);
        if (sz == 0 || sz > 200000 || typ == 0 || typ > 10000)
            break;
        if (pos + 12 + (int)sz > len)
            break;
        Packet p;
        p.type = typ;
        p.gameTime = gt;
        p.payload = region.mid(pos + 12, sz);
        r.append(p);
        pos += 12 + (int)sz;
    }
    return r;
}

QByteArray ReplayTracker::zlibDecompress(const QByteArray &data, int offset)
{
    if (offset < 0 || offset >= data.size())
        return {};
    const int MAX = 4 * 1024 * 1024;
    QByteArray out(MAX, 0);
    z_stream s{};
    s.next_in = (Bytef *)data.constData() + offset;
    s.avail_in = (uInt)(data.size() - offset);
    s.next_out = (Bytef *)out.data();
    s.avail_out = (uInt)MAX;
    if (inflateInit(&s) != Z_OK)
        return {};
    inflate(&s, Z_FINISH);
    inflateEnd(&s);
    return out.left((int)s.total_out);
}

quint16 ReplayTracker::u16(const QByteArray &b, int off) { return (off + 2 <= b.size()) ? qFromLittleEndian<quint16>((uchar *)b.constData() + off) : 0; }
quint32 ReplayTracker::u32(const QByteArray &b, int off) { return (off + 4 <= b.size()) ? qFromLittleEndian<quint32>((uchar *)b.constData() + off) : 0; }
quint64 ReplayTracker::varint(const QByteArray &b, int &pos)
{
    quint64 r = 0;
    int sh = 0;
    while (pos < b.size())
    {
        quint8 bt = (quint8)b[pos++];
        r |= (quint64)(bt & 0x7F) << sh;
        if (!(bt & 0x80))
            break;
        sh += 7;
    }
    return r;
}
QVector<ReplayTracker::PbField> ReplayTracker::parsePb(const QByteArray &buf)
{
    QVector<PbField> fs;
    int pos = 0;
    while (pos < buf.size())
    {
        int s = pos;
        quint64 tag = varint(buf, pos);
        if (pos == s)
            break;
        int fn = (int)(tag >> 3), wt = (int)(tag & 7);
        if (fn <= 0 || fn > 100000)
            break;
        PbField f;
        f.fn = fn;
        f.wt = wt;
        f.iv = 0;
        if (wt == 0)
        {
            f.iv = varint(buf, pos);
        }
        else if (wt == 2)
        {
            quint64 l = varint(buf, pos);
            if (pos + (int)l > buf.size())
                break;
            f.bv = buf.mid(pos, (int)l);
            pos += (int)l;
        }
        else if (wt == 1)
        {
            if (pos + 8 > buf.size())
                break;
            pos += 8;
        }
        else if (wt == 5)
        {
            if (pos + 4 > buf.size())
                break;
            pos += 4;
        }
        else
            break;
        fs.append(f);
    }
    return fs;
}
quint64 ReplayTracker::pbInt(const QVector<PbField> &fs, int fn, quint64 def)
{
    for (auto &f : fs)
        if (f.fn == fn && f.wt == 0)
            return f.iv;
    return def;
}
QByteArray ReplayTracker::pbBytes(const QVector<PbField> &fs, int fn)
{
    for (auto &f : fs)
        if (f.fn == fn && f.wt == 2)
            return f.bv;
    return {};
}
QString ReplayTracker::pbStr(const QVector<PbField> &fs, int fn)
{
    QByteArray b = pbBytes(fs, fn);
    return b.isEmpty() ? QString() : QString::fromUtf8(b);
}
QVector<QByteArray> ReplayTracker::pbAll(const QVector<PbField> &fs, int fn)
{
    QVector<QByteArray> r;
    for (auto &f : fs)
        if (f.fn == fn && f.wt == 2)
            r.append(f.bv);
    return r;
}
#pragma once
#include <QObject>
#include <main_overlay/controller/ParseReplay.h>
#include <QVector>
#include <QMap>
#include <QSet>
#include <QString>
#include <QByteArray>

struct DamageEvent
{
    float time_s = 0.f;
    QString shooter;
    QString target;
    int damage = 0;
    int hp_after = 0;
    QString hit_result;
    QString event_type;
    int total_damage = 0;
    int vehicle_cd = 0;
};
struct IncomingEvent
{
    float time_s = 0.f;
    QString attacker;
    int damage = 0;
    int hp_after = 0;
    QString event_type;
    int total_received = 0;
    int vehicle_cd = 0;
};
struct BlockedEvent
{
    float time_s = 0.f;
    QString attacker;
    int damage_blocked = 0;
    int total_blocked = 0;
};
struct AssistEvent
{
    float time_s = 0.f;
    QString ally;
    QString target;
    int damage = 0;
    int total_assist = 0;
};
struct BattleEvent
{
    enum Type
    {
        Damage,
        Incoming,
        Blocked,
        Assist
    };
    Type type;
    float time_s = 0.f;
    QString target;
    int damage = 0;
    int hp_after = 0;
    QString hit_result;
    QString event_type;
    int vehicle_cd = 0;
    QString attacker;
    QString ally;
    int total_damage = 0;
    int total_received = 0;
    int total_blocked = 0;
    int total_assist = 0;

    static BattleEvent fromDamage(const DamageEvent &e, int rcv, int blk, int ast)
    {
        BattleEvent b;
        b.type = Damage;
        b.time_s = e.time_s;
        b.target = e.target;
        b.damage = e.damage;
        b.hp_after = e.hp_after;
        b.hit_result = e.hit_result;
        b.event_type = e.event_type;
        b.total_damage = e.total_damage;
        b.total_received = rcv;
        b.total_blocked = blk;
        b.total_assist = ast;
        b.vehicle_cd = e.vehicle_cd;
        return b;
    }
    static BattleEvent fromIncoming(const IncomingEvent &e, int dmg, int blk, int ast)
    {
        BattleEvent b;
        b.type = Incoming;
        b.time_s = e.time_s;
        b.attacker = e.attacker;
        b.damage = e.damage;
        b.hp_after = e.hp_after;
        b.event_type = e.event_type;
        b.total_damage = dmg;
        b.total_received = e.total_received;
        b.total_blocked = blk;
        b.total_assist = ast;
        b.vehicle_cd = e.vehicle_cd;
        return b;
    }
    static BattleEvent fromBlocked(const BlockedEvent &e, int dmg, int rcv, int ast)
    {
        BattleEvent b;
        b.type = Blocked;
        b.time_s = e.time_s;
        b.attacker = e.attacker;
        b.damage = e.damage_blocked;
        b.total_damage = dmg;
        b.total_received = rcv;
        b.total_blocked = e.total_blocked;
        b.total_assist = ast;
        return b;
    }
    static BattleEvent fromAssist(const AssistEvent &e, int dmg, int rcv, int blk)
    {
        BattleEvent b;
        b.type = Assist;
        b.time_s = e.time_s;
        b.ally = e.ally;
        b.target = e.target;
        b.damage = e.damage;
        b.total_damage = dmg;
        b.total_received = rcv;
        b.total_blocked = blk;
        b.total_assist = e.total_assist;
        return b;
    }
};
struct BattleStats
{
    int dmg_dealt = 0;
    int shots = 0;
    int hits = 0;
    int pens = 0;
    int dmg_blocked = 0;
    int dmg_received = 0;
    int spot_assist = 0;
    int track_assist = 0;
    bool valid = false;
};

class ReplayTracker : public QObject
{
    Q_OBJECT
public:
    explicit ReplayTracker(QObject *parent = nullptr);
    bool update(const QString &replayPath, const QString &selfNick);
    void reset();

    const QVector<DamageEvent> &events() const { return m_events; }
    int totalDamage() const { return m_totalDamage; }
    const QVector<IncomingEvent> &incomingEvents() const { return m_incomingEvents; }
    int totalReceived() const { return m_totalReceived; }
    const QVector<BlockedEvent> &blockedEvents() const { return m_blockedEvents; }
    int totalBlocked() const { return m_totalBlocked; }
    const QVector<AssistEvent> &assistEvents() const { return m_assistEvents; }
    int totalAssist() const { return m_totalAssist; }
    const QVector<BattleEvent> &battleHistory() const { return m_battleHistory; }
    const BattleStats &battleStats() const { return m_battleStats; }
    int selfEid() const { return m_selfEid; }
    int entityMapSize() const { return m_entityToNick.size(); }

private:
    void parseFile(const QByteArray &data, const QString &selfNick);
    void parseEvents(const QByteArray &region, const QString &selfNick);
    void buildEntityMapFromArena(const QByteArray &region);
    bool extractEntityMapFromResult(const QByteArray &data, int zlibOffset);

    void parseBattleResultPacket(const QByteArray &payload);
    void parseBattleResultProto(const QByteArray &dec);
    static QByteArray zlibDecompress2(const QByteArray &data, int offset);

    static QVector<int> findZlibOffsets(const QByteArray &data);
    static QByteArray zlibDecompress(const QByteArray &data, int offset);

    struct Packet
    {
        quint32 type;
        float gameTime;
        QByteArray payload;
    };
    static QVector<Packet> iterPackets(const QByteArray &region);

    static quint16 u16(const QByteArray &b, int off);
    static quint32 u32(const QByteArray &b, int off);
    static quint64 varint(const QByteArray &b, int &pos);

    struct PbField
    {
        int fn, wt;
        quint64 iv;
        QByteArray bv;
    };
    static QVector<PbField> parsePb(const QByteArray &buf);
    static quint64 pbInt(const QVector<PbField> &fs, int fn, quint64 def = 0);
    static QByteArray pbBytes(const QVector<PbField> &fs, int fn);
    static QString pbStr(const QVector<PbField> &fs, int fn);
    static QVector<QByteArray> pbAll(const QVector<PbField> &fs, int fn);

    struct HitKey
    {
        int tMs, shooterEid, targetEid;
        bool operator<(const HitKey &o) const
        {
            if (tMs != o.tMs)
                return tMs < o.tMs;
            if (shooterEid != o.shooterEid)
                return shooterEid < o.shooterEid;
            return targetEid < o.targetEid;
        }
    };

    QString m_replayPath;
    qint64 m_lastFileSize = 0;
    int m_selfEid = -1;
    int m_worldEid = 0;

    QMap<int, QString> m_entityToNick;
    QMap<int, int> m_entityToTeam;
    QMap<int, int> m_entityToVehicle;
    QMap<QString, int> m_nickToVehicle;
    QMap<quint32, int> m_aidToVehicle;
    QMap<QString, quint32> m_nickToAid;
    QMap<int, int> m_maxHp;
    QMap<int, int> m_dmgRcvd;
    QMap<HitKey, quint8> m_hitResults;

    QVector<DamageEvent> m_events;
    int m_totalDamage = 0;
    QVector<IncomingEvent> m_incomingEvents;
    int m_totalReceived = 0;
    QVector<BlockedEvent> m_blockedEvents;
    int m_totalBlocked = 0;
    QVector<AssistEvent> m_assistEvents;
    int m_totalAssist = 0;
    QVector<BattleEvent> m_battleHistory;
    BattleStats m_battleStats;
};
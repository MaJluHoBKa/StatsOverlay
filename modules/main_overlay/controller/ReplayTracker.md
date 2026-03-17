# ReplayTracker — документация

## Содержание
1. [Контекст и ограничения](#контекст-и-ограничения)
2. [Структура файла реплея](#структура-файла-реплея)
3. [Формат пакетов BigWorld](#формат-пакетов-bigworld)
4. [Справочник типов пакетов](#справочник-типов-пакетов)
5. [Алгоритм парсинга](#алгоритм-парсинга)
6. [Описание классов](#описание-классов)

---

## Контекст и ограничения

WoT Blitz записывает реплей **на клиенте** — то есть фиксирует только те пакеты, которые сервер прислал именно этому игроку. Из этого следует несколько важных ограничений:

- **HP-обновления (prop=2) приходят только для видимых целей.** Если враг за укрытием или вне зоны обнаружения — попадание по нему не породит prop=2, даже если оно было реальным пробитием. Поэтому итоговый урон из реплея может быть меньше серверного (battle result).
- **Непробития и рикошеты не генерируют prop=2** — они вообще не оставляют следа в HP-потоке. Только `prop=9` (hit event) фиксирует факт выстрела с указанием результата.
- **Recorder реплея не имеет prop=55** — специальный пакет анонса entity_id, который сервер шлёт для других 13 игроков, для самого записывающего не приходит. Его entity_id определяется отдельным методом.

---

## Структура файла реплея

```
data.replay
├── [0 .. zlibOff1-1]     Заголовок (JSON-метаданные, zlib-сжатый)
├── zlibOff1              zlib-блок №1 — метаданные боя (первый блок)
├── [evStart .. evEnd]    Поток пакетов BigWorld (основной контент)
│     ├── Packet[0]       type=8, type=5, type=10, type=32, type=39 ...
│     ├── ...
│     └── Packet[N]       type=13 — battle result (появляется в конце боя)
└── zlibOff2              zlib-блок №2 — battle result (дублируется отдельно)
```

**evStart** определяется эвристически: первый байт после `zlibOff1 + 10`, где начинается корректный пакет (размер 1–50000, тип 1–100, время 0–3600 секунд).

---

## Формат пакетов BigWorld

Каждый пакет в потоке имеет заголовок из 12 байт:

```
Offset  Size  Type    Description
0       4     u32 LE  payload_size
4       4     u32 LE  packet_type
8       4     f32 LE  game_time (секунды от начала боя)
12      N     bytes   payload (N = payload_size)
```

Пакеты типа `8` (PKT_ENTITY_PROPERTY) имеют дополнительный заголовок внутри payload:

```
payload:
  0       4     u32 LE  entity_id (weid — entity которому принадлежит свойство)
  4       4     u32 LE  prop_id   (номер свойства)
  8       4     u32 LE  inner_size
  12      N     bytes   inner     (N = inner_size)
```

---

## Справочник типов пакетов

| Тип | Константа | Описание |
|-----|-----------|----------|
| 5  | `PKT_ENTITY_APPEAR`   | Спавн/респавн entity. `payload[0:4]` = entity_id, `payload[51:53]` = max HP (u16 LE), `payload[62]` = nick_len, `payload[63..]` = nick (ASCII) |
| 8  | `PKT_ENTITY_PROPERTY` | Обновление свойства entity. Содержит weid + prop_id + inner |
| 10 | `PKT_ENTITY_ENTER`    | Entity входит в мир. `payload[0:4]` = weid, `payload[8:12]` = entity_id вошедшего |
| 13 | *(battle result)*     | Итог боя. Содержит pickle + zlib + protobuf со статистикой всех 14 игроков |
| 32 | `PKT_ENTITY_CREATE`   | Создание entity (инициализация свойств) |
| 39 | *(position)*          | Позиция/ротация entity |

### Свойства entity (prop_id в пакете type=8)

| prop_id | Константа | inner-структура |
|---------|-----------|-----------------|
| 2  | `PROP_HP_UPDATE`   | `[0:2]` = cur_hp (u16 LE), `[2:6]` = shooter_eid (u32 LE), `[6]` = flag (0=выстрел, 1=ремонт, 2=огонь) |
| 9  | `PROP_HIT_EVENT`   | `[0:4]` = shooter_eid (u32 LE), `[4:8]` = target_eid (u32 LE), `[9]` = hit_result (0=pen, 1=no_pen, 2=ricochet, 3=no_dmg, 4=miss) |
| 54 | `PROP_PLAYER_INFO` | `[30:34]` = account_id (u32 LE), `[38]` = nick_len, `[39..]` = nick (ASCII). Приходит от WORLD_EID |
| 55 | *(entity_id ann.)*  | size=9, `[5:9]` = varint → entity_id игрока. Приходит от WORLD_EID перед prop=54 |

### WORLD_EID

Специальная служебная entity, существующая в каждом бою. Это единственная entity, которая шлёт `prop=54` с никами игроков. WORLD_EID **динамический** — разный в каждом бою. Определяется как `weid` первого пакета `type=8, prop=54` с валидным ASCII-ником.

### flag в prop=2

| Значение | Смысл |
|----------|-------|
| 0 | Обычный выстрел (пробитие с уроном) |
| 1 | Ремонт модуля (HP увеличивается — не урон) |
| 2 | Урон от огня (fire damage, несколько тиков) |

Непробития, рикошеты и промахи **не порождают `prop=2`** — они фиксируются только через `prop=9`.

---

## Алгоритм парсинга

### Точка входа: `update(replayPath, selfNick)`

Вызывается по таймеру. Читает файл только если размер изменился (`m_lastFileSize`). При изменении запускает полный перепарсинг через `parseFile()`.

### `parseFile(data, selfNick)`

1. Находит zlib-блоки через `findZlibOffsets()`.
2. Если второй zlib-блок найден и `m_entityToNick` пустой — читает entity map из него как запасной источник (`extractEntityMapFromResult()`).
3. Определяет `evStart` — начало пакетного потока.
4. Очищает `m_events`, `m_hitResults`, `m_totalDamage` (но **не** `m_selfEid`, `m_worldEid`, `m_entityToNick` — они накапливаются).
5. Вызывает `parseEvents(region, selfNick)`.

### `parseEvents(region, selfNick)` — пять проходов

#### Проход 0 — Battle Result (type=13)

Ищет пакет `type=13` в потоке. При нахождении разбирает его через `parseBattleResultPacket()`, который извлекает полную карту `entity_id → nick → team → dmg_rcvd` для всех 14 игроков. Это наиболее полный источник entity map — появляется в конце боя.

#### Проход 1 — Entity Map + selfEid

Выполняется только если `m_selfEid < 0` (не найден ещё).

**Шаг 1 — определить WORLD_EID:**  
Ищет первый `type=8, prop=54` с валидным ASCII-ником (≥3 символов, только печатные). `weid` этого пакета = WORLD_EID. Кешируется в `m_worldEid` на весь бой.

**Шаг 2 — prop=55/54 → entity map (13 из 14 игроков):**  
```
prop=55 (size=9, WORLD_EID)  →  inner[5:] = varint = entity_id
prop=54 (WORLD_EID, ±1s)     →  inner[38]=nl, inner[39..]=nick
```
Пары формируют карту `entity_id → nick`. Фильтр: entity_id принимается только если `eid >> 16 == worldEid >> 16` — это отсеивает мусорные маленькие eid (0x1bd, 0x22b...) которые prop=55 шлёт для других объектов.

**Шаг 3 — type=5 → entity map (умершие игроки):**  
Пакет respawn содержит nick прямо в payload. Дополняет карту для игроков, погибших хотя бы раз.

**Шаг 4 — selfEid из карты:**  
Если `selfNick` уже есть в `m_entityToNick` (попал через type=5 — игрок умирал), берём оттуда.

**Шаг 5 — selfEid через prop=54 + type=10 (основной метод):**  
Recorder реплея не получает собственный prop=55, поэтому в карте после шагов 2–3 его может не быть. Надёжный паттерн:

```
prop=54(selfNick) на WORLD_EID  →  sawSelfNick = true
...возможны другие prop=54...
type=10(weid=WORLD_EID)         →  payload[8:12] = selfEid  ✓
```

Ключевой нюанс: `sawSelfNick` **не сбрасывается** при виде других ников — только при `type=10` для чужого entity. Это важно, потому что реальная последовательность пакетов выглядит так:
```
prop=54("ParryLikeMe")
prop=54("ParryLikeMe")
prop=54("X0JlOD")       ← другой ник, НО не сбрасываем
type=10  →  payload[8:12] = 0x10172c12  ✓
```

#### Проход 2 — max HP из type=5

Для каждого пакета respawn (`type=5`) читает `payload[51:53]` = начальный HP entity. Сохраняется в `m_maxHp[eid]` (первый respawn = максимальный HP в бою).

#### Проход 3 — hit results из prop=9

Собирает результаты наших выстрелов в кеш `m_hitResults[{time_ms, shooterEid, targetEid}] = hit_result`. Используется в проходе 4 для аннотации события типом попадания.

#### Проход 4 — HP-дельты → урон

Основной цикл. Инициализирует `lastHp` из `m_maxHp`. Для каждого `prop=2`:

1. Обновляет `lastHp[teid] = curHp` всегда (чтобы не терять baseline для следующих ударов).
2. Если `shooter != selfEid` — пропускает.
3. Если нет baseline (`prevHp < 0`) — пропускает первый удар по этой цели.
4. Если `flag == 1` (ремонт) или `dmg <= 0` — пропускает.
5. Создаёт `DamageEvent` с `dmg = prevHp - curHp`, ищет hit_result из кеша prop=9, определяет `event_type` по flag (0=shot, 2=fire).

### Парсинг battle result (type=13)

Структура payload:
```
[0:2]    u16_le = 100 (magic)
[2:6]    u32_le = 12
[6..]    pickle protocol 2: (compressed_size, checksum)
[6+pkl_size+4 ..]  zlib → 4 bytes skip → protobuf в latin-1 pickle
```

Protobuf outer message:
- **field 201** — массив `{account_id, {nick, ...}}` → карта account→nick
- **field 301** — массив `{entity_id, {account_id(101), team(102), dmg_rcvd(11), ...}}`

---

## Описание классов

### `DamageEvent`

Структура данных одного события урона.

```cpp
struct DamageEvent {
    float   time_s;       // время от начала боя (секунды)
    QString shooter;      // ник стрелявшего (всегда selfNick)
    QString target;       // ник цели или "eid_XXXXXXXX" если не известен
    int     damage;       // нанесённый урон (HP delta)
    int     hp_after;     // HP цели после удара
    QString hit_result;   // "pen", "pen+fire", "ricochet", "crit", "blocked" или ""
    QString event_type;   // "shot" (flag=0), "fire" (flag=2), "repair" (flag=1)
    int     total_damage; // накопленный урон к этому моменту
};
```

**Важно:** `hit_result` берётся из `prop=9` по ключу `{time_ms, shooterEid, targetEid}`. Если prop=9 не совпал по времени — поле пустое. `event_type = "repair"` на практике не появляется в событиях урона (такие события фильтруются).

---

### `ReplayTracker`

Qt-класс (`QObject`) для live-мониторинга реплея. Предназначен для вызова из `QTimer`.

#### Публичные методы

| Метод | Описание |
|-------|----------|
| `bool update(replayPath, selfNick)` | Читает файл если изменился, запускает парсинг. Возвращает `true` если были изменения |
| `void reset()` | Полный сброс состояния. Вызывать когда файл реплея исчезает (конец боя/новый бой) |
| `const QVector<DamageEvent>& events()` | Список событий урона в хронологическом порядке |
| `int totalDamage()` | Суммарный урон на текущий момент |
| `int selfEid()` | Entity ID recorder'а реплея (-1 если ещё не найден) |
| `int entityMapSize()` | Количество известных entity в карте |

#### Состояние (приватные поля)

| Поле | Тип | Описание |
|------|-----|----------|
| `m_lastFileSize` | `qint64` | Размер файла при последнем чтении. Изменение = триггер перепарсинга |
| `m_selfEid` | `int` | Entity ID recorder'а. -1 до обнаружения. После нахождения не меняется до `reset()` |
| `m_worldEid` | `int` | WORLD_EID текущего боя. 0 до обнаружения |
| `m_entityToNick` | `QMap<int,QString>` | Карта entity_id → ник игрока |
| `m_entityToTeam` | `QMap<int,int>` | entity_id → номер команды (из battle result) |
| `m_maxHp` | `QMap<int,int>` | entity_id → начальный HP (из type=5) |
| `m_dmgRcvd` | `QMap<int,int>` | entity_id → полученный урон (из battle result) |
| `m_hitResults` | `QMap<HitKey,quint8>` | Кеш результатов выстрелов для аннотации событий |
| `m_events` | `QVector<DamageEvent>` | Список событий урона (очищается при каждом перепарсинге) |
| `m_totalDamage` | `int` | Суммарный урон (очищается при каждом перепарсинге) |

#### Внутренние структуры

**`Packet`** — распакованный пакет из потока:
```cpp
struct Packet { quint32 type; float gameTime; QByteArray payload; };
```

**`HitKey`** — ключ для кеша prop=9:
```cpp
struct HitKey { int tMs, shooterEid, targetEid; };
```
Используется для сопоставления события `prop=9` (результат выстрела) с событием `prop=2` (HP update) по времени и участникам.

**`PbField`** — поле protobuf:
```cpp
struct PbField { int fn, wt; quint64 iv; QByteArray bv; };
```

#### Ограничения и известные поведения

- **Первый удар по цели без type=5 теряется.** Если игрок не умирал и не получал respawn, его начальный HP неизвестен. Первый prop=2 от нас по такой цели становится новым baseline, урон не фиксируется.
- **entity map заполняется постепенно.** Во время активного боя карта может содержать не всех игроков — только тех, кто умер (type=5) или чей battle result уже пришёл (type=13). Цели могут отображаться как `eid_XXXXXXXX` до конца боя.
- **m_events и m_totalDamage пересчитываются каждый раз** при изменении файла. `m_selfEid`, `m_worldEid`, `m_entityToNick` накапливаются и не сбрасываются между итерациями — только через `reset()`.
- **Зависимость от zlib.** Требует линковки с `-lz`. Декомпрессия ограничена 4 МБ буфером.
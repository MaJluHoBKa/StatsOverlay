# StatsOverlay

> Десктопный оверлей для отслеживания сессионной статистики и хода боёв в **Tanks Blitz** (Lesta Games).  
> Работает поверх игры, не требует её перезапуска и обновляется в реальном времени.

---

## Содержание

- [Возможности](#возможности)
- [Требования](#требования)
- [Сборка из исходников](#сборка-из-исходников)
- [Структура проекта](#структура-проекта)
- [Архитектура](#архитектура)
- [Модуль controller](#модуль-controller)
- [Модуль widgets](#модуль-widgets)
- [Боевые виджеты](#боевые-виджеты)
- [Парсер реплеёв](#парсер-реплеёв)
- [Активация](#активация)
- [Авторизация через Lesta API](#авторизация-через-lesta-api)
- [Сессионная статистика](#сессионная-статистика)
- [Стримерский режим](#стримерский-режим)
- [Внешние зависимости](#внешние-зависимости)
- [Конфигурация и файлы данных](#конфигурация-и-файлы-данных)
- [Лицензия](#лицензия)

### Справочник классов
- [Вспомогательные структуры](#1-вспомогательные-структуры)
- [ApiController — детально](#21-apicontroller)
- [ReplayTracker — детально](#22-replaytracker)
- [ReplayParser — детально](#23-replayparser)
- [HWIDActivator — детально](#24-hwidactivator)
- [Классы данных — детально](#25-mainstatsdata)
- [MainStats — детально](#31-mainstats)
- [RatingStats — детально](#32-ratingstats)
- [OtherStats — детально](#33-otherstats)
- [VehicleStats — детально](#34-vehiclestats)
- [PlayerStats — детально](#35-playerstats)
- [GunMarks — детально](#36-gunmarks)
- [InfoPage — детально](#37-infopage)
- [Widgets — детально](#38-widgets)
- [Подокна — детально](#4-подокна)
- [MainOverlay — детально](#5-mainoverlay)
- [SubOverlay — детально](#6-suboverlay)
- [ActivationWindow — детально](#7-activationwindow)

---

## Возможности

| Категория | Что умеет |
|-----------|-----------|
| **Сессионная статистика** | Кредиты, золото, опыт, бои, победы, средний урон — всё считается от момента запуска |
| **Рейтинговая статистика** | Текущий рейтинг MM, изменение за сессию, калибровочные бои |
| **Статистика по технике** | Последний сыгранный танк: уровень, нация, тип, результаты боя |
| **Статистика игроков** | Список союзников и противников с данными API, сортировка по уровню и названию танка |
| **Отметки на стволе** | Прогресс получения отметок (1–3), требуемый средний урон для следующей |
| **Боевые виджеты** | Нанесённый урон, заблокированный урон, ассисты, входящий урон — в реальном времени |
| **История боя** | Хронологический журнал всех событий текущего боя |
| **Стримерский режим** | Компактный отдельный оверлей для трансляции |
| **Перетаскивание** | Окно можно переместить в любое место экрана |
| **Настройка прозрачности** | Регулировка прозрачности фона через контекстное меню |
| **Горячие клавиши** | Переключение страниц без мыши |

---

## Требования

### Для запуска готового бинарника
- Windows 10 / 11 (x64)
- Tanks Blitz от Lesta Games (российский клиент, `api.tanki.su`)
- Интернет-соединение (для Lesta API)

### Для сборки из исходников
- Qt 6.9.x (MinGW 64-bit) — рекомендуется установить через [Qt Online Installer](https://www.qt.io/download)
- CMake ≥ 3.16
- MinGW 13.x (поставляется вместе с Qt)
- Git

---

## Сборка из исходников

### 1. Клонировать репозиторий

```bash
git clone https://github.com/MaJluHoBKa/StatsOverlay.git
cd StatsOverlay
git checkout pre-2-3
```

### 2. Открыть в Qt Creator

Открыть файл `CMakeLists.txt` через **File → Open File or Project**.  
Qt Creator автоматически сконфигурирует проект.

### 3. Или собрать через командную строку

```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles" \
  -DCMAKE_BUILD_TYPE=Release \
  -DQt6_DIR="C:/Qt/6.9.2/mingw_64/lib/cmake/Qt6"
cmake --build . --parallel
```

### 4. Скопировать DLL рядом с exe

После сборки рядом с `StatsOverlay.exe` должна находиться `libcurl-x64.dll`.  
CMake копирует её автоматически через `add_custom_command`.

Для развёртывания Qt-зависимостей:

```bash
windeployqt StatsOverlay.exe
```

### 5. Готовый бинарник

Находится в `dist/StatsOverlay-Beta-PC-v0_7.exe`.

---

## Структура проекта

```
StatsOverlay/
│
├── main.cpp                          # Точка входа: инициализация, активация, запуск
├── CMakeLists.txt                    # Система сборки
│
├── modules/
│   ├── main_overlay/                 # Основное оверлей-окно
│   │   ├── main_overlay.h/.cpp       # Главный виджет: навигация, drag, анимации
│   │   ├── main_resources.qrc
│   │   │
│   │   ├── activate/
│   │   │   └── activation_window.h/.cpp   # Диалог активации по HWID-ключу
│   │   │
│   │   ├── controller/               # Бизнес-логика (без UI)
│   │   │   ├── ApiController.h       # Центральный контроллер: API, авторизация
│   │   │   ├── ReplayTracker.h/.cpp  # Трекер реплея в реальном времени
│   │   │   ├── ParseReplay.h         # Низкоуровневый парсер .wotreplay
│   │   │   ├── HWID.h                # Генерация и верификация ключа активации
│   │   │   └── data/
│   │   │       ├── MainStatsData.h       # Основная статистика сессии
│   │   │       ├── RatingStatsData.h     # Рейтинговая статистика
│   │   │       ├── MasteryStatsData.h    # Мастерство экипажа
│   │   │       ├── OtherStatsData.h      # Расширенная статистика
│   │   │       └── VehicleStatsData.h    # Статистика по технике
│   │   │
│   │   └── widgets/                  # UI-страницы оверлея
│   │       ├── main_stats/           # Страница основной статистики
│   │       ├── rating_stats/         # Страница рейтинговой статистики
│   │       ├── other_stats/          # Страница расширенной статистики
│   │       ├── vehicles_stats/       # Страница статистики по технике
│   │       ├── player_stats/         # Страница игроков текущего боя
│   │       │   └── split_players/    # Окно разбивки по командам
│   │       ├── gun_marks/            # Страница отметок на стволе
│   │       │   └── gunmarks_window/  # Детализация отметок
│   │       ├── info_page/            # Информационная страница
│   │       └── widgets_page/         # Боевые виджеты
│   │           ├── dmg_window/       # Виджет урона в реальном времени
│   │           ├── history_window/   # Виджет истории событий боя
│   │           └── widgets_window/   # Управление активными виджетами
│   │               └── FlowLayout    # Адаптивный FlowLayout
│   │
│   └── sub_overlay/                  # Стримерский оверлей
│       └── sub_overlay.h/.cpp
│
├── external/
│   ├── httplib.h                     # cpp-httplib: встроенный HTTP-сервер
│   ├── json.hpp                      # nlohmann/json
│   └── curl/                         # libcurl (x64, Windows) + OpenSSL
│
├── resources/
│   ├── fonts/JetBrainsMono-Bold.ttf
│   ├── icons/                        # Иконки наций, типов техники, UI
│   └── vehicles/
│       ├── tanks.json                # База техники (резервная копия)
│       └── vehicles_db.json
│
├── dist/
│   └── StatsOverlay-Beta-PC-v0_7.exe
│
├── genkey.py                         # Утилита генерации ключа активации
└── stats.py                          # Вспомогательный скрипт
```

---

## Архитектура

```
main.cpp
  │
  ├─► HWIDActivator::checkActivation()   — проверка активации
  │       если не активирован → ActivationWindow
  │
  ├─► ApiController (singleton-like)     — создаётся один раз, передаётся везде
  │       │
  │       ├─ login() / prolongate()      — OAuth2 через libcurl + httplib сервер
  │       ├─ get_main_stats()            — запрос к api.tanki.su
  │       ├─ get_rating_stats()
  │       ├─ get_other_stats()
  │       ├─ get_vehicles_stats()
  │       ├─ get_mastery_stats()
  │       └─ данные → MainStatsData / RatingStatsData / ...
  │
  ├─► MainOverlay (QWidget, поверх всех окон)
  │       │
  │       ├─ QStackedWidget (8 страниц)
  │       │     ├─ [0] InfoPage
  │       │     ├─ [1] GunMarks
  │       │     ├─ [2] PlayerStats
  │       │     ├─ [3] Widgets (боевые)
  │       │     ├─ [4] OtherStats
  │       │     ├─ [5] VehicleStats
  │       │     ├─ [6] RatingStats
  │       │     └─ [7] MainStats
  │       │
  │       └─ ConfigPanel (анимируемая боковая панель настроек)
  │
  ├─► SubOverlay                         — стримерский компактный оверлей
  │
  └─► ReplayTracker (фоновый поток)
          │
          ├─ следит за файлом .wotreplay
          ├─ парсит Protobuf-пакеты типа EntityMethod
          ├─ эмитирует события: DamageEvent, IncomingEvent,
          │                     BlockedEvent, AssistEvent
          └─ обновляет BattleStats
```

Все виджеты получают указатель на `ApiController` через конструктор и опрашивают его по таймеру (`QTimer`). Никаких глобальных переменных — единственный источник данных.

---

## Справочник классов

> Детальное описание всех классов, структур, методов и алгоритмов.


## 1. Вспомогательные структуры

### `InfoLabel` / `InfoRatingLabel` / `InfoOtherLabel`

Одинаковые по смыслу структуры для хранения тройки UI-элементов одной строки статистики. Определены в `main_stats.h`, `rating_stats.h`, `other_stats.h` соответственно.

```cpp
struct InfoLabel {
    QLabel *value = nullptr;   // числовое значение справа
    QLabel *arrow = nullptr;   // стрелка ▲▼ с цветовой индикацией
    QFrame *line  = nullptr;   // горизонтальная линия-разделитель (меняет цвет)
};
```

Хранятся в `std::unordered_map<std::string, InfoLabel*>` где ключ — строковое название метрики (например `"Золото"`, `"Урон"`). Позволяет обновлять конкретную строку по имени без перебора всех виджетов.

---

### `PlayerRows`

Структура одной строки в таблице игроков (`players_window.h`).

```cpp
struct PlayerRows {
    QHBoxLayout *row   = nullptr;   // горизонтальный контейнер строки
    QLabel *name       = nullptr;   // никнейм игрока
    QLabel *tank_name  = nullptr;   // название танка
    QLabel *battles    = nullptr;   // количество боёв
    QLabel *wins       = nullptr;   // процент побед
    QLabel *damage     = nullptr;   // средний урон
};
```

---

### `BattleData`

Агрегированные данные текущего боя для `DamageWidget` (`dmg.h`).

```cpp
struct BattleData {
    int64_t damage;   // суммарный нанесённый урон
    int64_t block;    // суммарный заблокированный урон
    int64_t assist;   // суммарный урон ассистов
};
```

---

### `Player`

Данные об игроке текущего боя (`ApiController.h`).

```cpp
struct Player {
    std::string nickname;
    int64_t id;          // account_id
    int64_t tank_id;     // идентификатор танка
    int64_t battles;     // всего боёв на аккаунте
    int64_t damage;      // суммарный урон на аккаунте (для расчёта среднего)
    int64_t wins;        // победы
    int64_t tier;        // уровень танка
    std::string tank_name;
    int64_t team;        // 1 = союзник, 2 = противник
};
```

---

### `PlayerRaw`

Сырые данные игрока из парсера реплея (`ParseReplay.h`), до обогащения данными API.

```cpp
struct PlayerRaw {
    std::string nickname;
    int team_number;       // 1 или 2
    int tank_id;
    uint64_t account_id;
    std::string clan_tag;
    int platoon_number;    // номер взвода (0 если нет)
};
```

---

### `DamageEvent` / `IncomingEvent` / `BlockedEvent` / `AssistEvent`

Структуры событий боя, формируемые `ReplayTracker` (`ReplayTracker.h`).

```cpp
struct DamageEvent {
    float time_s;         // игровое время боя в секундах
    QString shooter;      // никнейм стрелявшего (self)
    QString target;       // никнейм цели
    int damage;           // нанесённый урон
    int hp_after;         // HP цели после попадания
    QString hit_result;   // "penetration", "ricochet", "no_damage", "critical_hit" и др.
    QString event_type;   // тип события
    int total_damage;     // накопленный урон за весь бой
    int vehicle_cd;       // идентификатор танка цели (для поиска имени)
};

struct IncomingEvent {
    float time_s;
    QString attacker;      // кто нанёс урон по self
    int damage;
    int hp_after;
    QString event_type;
    int total_received;    // накопленный полученный урон
    int vehicle_cd;        // танк атакующего
};

struct BlockedEvent {
    float time_s;
    QString attacker;
    int damage_blocked;    // сколько заблокировано в этот раз
    int total_blocked;     // накопленный заблокированный урон
};

struct AssistEvent {
    float time_s;
    QString ally;          // союзник-подсветчик или союзник с гусенью
    QString target;        // цель
    int damage;            // урон ассиста
    int total_assist;      // накопленный урон ассистов
};
```

---

### `BattleEvent`

Унифицированная структура для истории боя, объединяющая все типы событий (`ReplayTracker.h`).

```cpp
struct BattleEvent {
    enum Type { Damage, Incoming, Blocked, Assist };
    Type type;
    float time_s;
    // Поля из всех типов (не все используются в каждом типе):
    QString target, attacker, ally, hit_result, event_type;
    int damage, hp_after, vehicle_cd;
    int total_damage, total_received, total_blocked, total_assist;

    // Фабричные методы:
    static BattleEvent fromDamage(const DamageEvent &e, int rcv, int blk, int ast);
    static BattleEvent fromIncoming(const IncomingEvent &e, int dmg, int blk, int ast);
    static BattleEvent fromBlocked(const BlockedEvent &e, int dmg, int rcv, int ast);
    static BattleEvent fromAssist(const AssistEvent &e, int dmg, int rcv, int blk);
};
```

Фабричные методы заполняют `BattleEvent` из соответствующей структуры события и **текущих накопленных счётчиков** остальных типов — чтобы в каждой записи истории был полный срез всех показателей на момент события.

---

### `BattleStats`

Итоговая статистика боя, формируемая `ReplayTracker` из `battle_results` пакета реплея.

```cpp
struct BattleStats {
    int dmg_dealt   = 0;   // нанесённый урон
    int shots       = 0;   // выстрелов
    int hits        = 0;   // попаданий
    int pens        = 0;   // пробитий
    int dmg_blocked = 0;   // заблокировано
    int dmg_received= 0;   // получено
    int spot_assist = 0;   // урон с подсвета
    int track_assist= 0;   // урон с гусени
    bool valid      = false;
};
```

---

## 2. Модуль controller

### 2.1 ApiController

**Файл:** `controller/ApiController.h`  
**Наследование:** не наследует Qt-классов, plain C++ класс

Центральный объект приложения. Создаётся в `main.cpp`, передаётся указателем во все виджеты через конструктор. Является единственным источником данных для всей UI-части.

#### Поля

```cpp
// Сетевые
std::string base_url = "https://api.tanki.su/wot";
std::string application_id = "4d9feb2c53c712fc6a87e026c990dd12";
std::string token;        // OAuth2 access_token
std::string account_id;   // строковый id аккаунта Lesta
std::string nickname;

// Состояние
bool isAuth = false;
bool isFirstMainStats    = true;   // true = следующий запрос будет initialStats()
bool isFirstRatingStats  = true;
bool isFirstOtherStats   = true;
bool isFirstMasteryStats = true;
bool isFirstVehiclesStats= true;
bool isGunMark  = false;   // активна ли страница отметок
bool isTankLock = false;   // заблокирован ли выбор танка

// Объекты данных
MainStatsData    mainStats;
RatingStatsData  ratingStats;
MasteryStatsData masteryStats;
OtherStatsData   otherStats;
VehicleStatsData vehicleStats;

// Игроки боя
std::vector<Player> allies;
std::vector<Player> enemies;
std::unordered_map<std::string, Player&> db_players;   // id → Player

// Логирование
QFile m_logFile;
```

#### Методы авторизации

**`bool login()`**
1. Формирует URL: `GET /wot/auth/login/?application_id=...&redirect_uri=http://localhost:5000/&nofollow=1`
2. Выполняет запрос через libcurl, получает JSON с полем `data.location`
3. Открывает браузер через `ShellExecuteA(NULL, "open", location, ...)`
4. Поднимает синхронный HTTP-сервер (cpp-httplib) на `0.0.0.0:5000`
5. Принимает GET `/` с параметрами `access_token`, `account_id`, `nickname`, `status`
6. При `status == "ok"` сохраняет токен, выставляет `isAuth = true`, останавливает сервер

**`bool logout()`**
- POST на `/wot/auth/logout/` с `access_token`
- При `status == "ok"` очищает токен, nickname, account_id, сбрасывает флаги `isFirst*`

**`bool prolongate()`**
- POST на `/wot/auth/prolongate/` с `access_token`
- Обновляет `token` новым значением из ответа
- Вызывается при старте если `auth.json` содержит действующие данные

#### Методы получения статистики

Все методы используют один паттерн:
1. Формируют URL с нужными параметрами
2. Выполняют GET-запрос через libcurl
3. Парсят JSON через nlohmann/json
4. Если `isFirst* == true` — вызывают `initialStats()`, иначе `updateStats()`
5. Сбрасывают флаг `isFirst* = false`
6. Возвращают `true` при успехе

**`bool update_main_stats()`**  
Эндпоинт: `/wot/account/info/?account_id=...&access_token=...&fields=statistics.all`  
Заполняет `StatsData`: `credits`, `gold`, `exp_battle`, `exp_free`, `battles`, `wins`, `losses`, `totalDamage`

**`bool update_rating_stats()`**  
Эндпоинт: `/wot/account/info/?extra=statistics.rating`  
Заполняет `RatingData`: `mm_rating` (raw), `calib_battles`, `exp_battle`, `battles`, `wins`, `totalDamage`

**`bool update_other_stats()`**  
Эндпоинт: `/wot/account/info/?fields=statistics.all`  
Заполняет `OtherData`: `battles`, `hits`, `shots`, `survived`, `frags`, `receiverDamage`, `totalDamage`, `lifeTime`

**`bool update_mastery_stats()`**  
Эндпоинт: `/wot/account/achievements/?account_id=...`  
Заполняет `MasteryData`: `mastery` (Ace Tanker/Туз), `mastery_1`, `mastery_2`, `mastery_3`

**`bool update_vehicles_stats()`**  
Эндпоинты:  
- `/wot/account/tanks/?account_id=...` — список tank_id в гараже  
- `/wot/tanks/stats/?tank_id=...` — детальная статистика по каждому  
Заполняет карту `VehicleData` по tank_id: `battles`, `wins`, `totalDamage`

#### Методы работы с игроками боя

**`bool loadReplayPlayers()`**  
Находит активный файл реплея в `Documents/TanksBlitz/replays/recording*/data.replay`.  
Использует `ReplayParser` для извлечения `PlayerRaw` из пакета `UpdateArena`.  
Запрашивает статистику игроков через API:
- `/wot/account/list/?search=...` — поиск account_id по никнейму
- `/wot/account/info/?account_id=...` — статистика каждого игрока  
Вызывает `setPlayers(allies, enemies)`.

**`bool get_players_stats()`**  
Батч-запрос статистики для всех игроков текущего боя.  
Формирует строку ID через `join()` (формат: `id1,id2,...,id14`).  
Заполняет поля `battles`, `damage`, `wins` в каждом `Player`.  
Обогащает `tank_name` и `tier` из `VehicleStatsData`.

**`void setPlayers(vector<Player> allies, vector<Player> enemies)`**  
Сохраняет списки, заполняет `db_players` (map id → Player).

**`void sortPlayers(vector<Player> &players)`**  
Двухуровневая сортировка:
1. `stable_sort` по `tank_name` через `tankNameCompare` — кастомный компаратор: цифры `<` латиница `<` кириллица
2. `stable_sort` по `tier` по убыванию (сохраняет порядок по имени внутри уровня)

**`static bool tankNameCompare(const string &a, const string &b)`**  
Сравнивает посимвольно: если один символ — цифра, другой нет, цифра «меньше».  
Если один — латиница, другой нет, латиница «меньше».  
Иначе обычное лексикографическое сравнение.

#### Методы управления данными

**`void reset()`**  
Сбрасывает флаги `isFirst* = true`. При следующем опросе API текущие данные станут новой базовой точкой сессии.

**`void setMark(bool)` / `bool isMark()`**  
Флаг активности страницы отметок. Используется в `VehicleStats` для блокировки смены отслеживаемого танка.

**`void setTankLock(bool)` / `bool isLock()`**  
Флаг блокировки смены танка. При `true` `VehicleStats` не обновляет выбранный танк даже если сыгран другой.

**`string join()`**  
Возвращает строку `id1,id2,...` из объединённых allies + enemies. Только если размер равен 14 (полный состав боя 7x7).

#### Логирование

**`void initLog()`**  
Создаёт `%Documents%/StatsOverlay/logs/YYYY-MM-DD.log`.  
Записывает заголовок с версией ОС и ядра.

**`void log(const QString &message, int page, const QDateTime &timestamp)`**  
Записывает строку с префиксом страницы:
- 0 → `[MAIN PAGE]`
- 1 → `[RATING PAGE]`
- 2 → `[VEHICLE PAGE]`
- 3 → `[PLAYER PAGE]`
- 4 → `[AUTH]`

#### Сохранение сессии

При `MainOverlay::logout()`:
1. Считывает токен, никнейм, account_id из `ApiController`
2. Шифрует каждый XOR с ключом `"speaker"` + Base64
3. Записывает JSON в `%AppData%/StatsOverlay/auth.json`

При следующем запуске `main.cpp` читает файл, расшифровывает, восстанавливает состояние через `setToken`, `setNickname`, `setAccountId`, `setAuth(true)`, затем вызывает `prolongate()`.

---

### 2.2 ReplayTracker

**Файл:** `controller/ReplayTracker.h/.cpp`  
**Наследование:** `QObject`

Трекер боя в реальном времени. Периодически читает `.wotreplay` файл и извлекает события боя.

#### Поля состояния

```cpp
QString m_replayPath;       // путь к текущему реплею
qint64 m_lastFileSize = 0;  // последний известный размер файла
int m_selfEid  = -1;        // entity ID игрока
int m_worldEid = 0;         // entity ID мирового объекта
```

#### Карты сущностей

```cpp
QMap<int, QString> m_entityToNick;    // entity_id → nickname
QMap<int, int>     m_entityToTeam;    // entity_id → team (1/2)
QMap<int, int>     m_entityToVehicle; // entity_id → vehicle_cd (tank_id)
QMap<QString, int> m_nickToVehicle;   // nickname → vehicle_cd
QMap<quint32, int> m_aidToVehicle;    // account_id → vehicle_cd
QMap<QString, quint32> m_nickToAid;   // nickname → account_id
QMap<int, int>     m_maxHp;           // entity_id → макс. HP
QMap<int, int>     m_dmgRcvd;         // entity_id → накопленный полученный урон
```

#### Структура `HitKey`

```cpp
struct HitKey {
    int tMs;          // время в миллисекундах
    int shooterEid;   // entity ID стрелявшего
    int targetEid;    // entity ID цели
    bool operator<(const HitKey &o) const { ... }
};
QMap<HitKey, quint8> m_hitResults;  // результат попадания для дедупликации
```

#### Накопленные данные

```cpp
QVector<DamageEvent>  m_events;          int m_totalDamage   = 0;
QVector<IncomingEvent>m_incomingEvents;  int m_totalReceived = 0;
QVector<BlockedEvent> m_blockedEvents;   int m_totalBlocked  = 0;
QVector<AssistEvent>  m_assistEvents;    int m_totalAssist   = 0;
QVector<BattleEvent>  m_battleHistory;
BattleStats           m_battleStats;
```

#### Метод `bool update(const QString &replayPath, const QString &selfNick)`

Основной публичный метод. Алгоритм:
1. Если `replayPath` изменился — сбрасывает всё через `reset()`
2. Сравнивает текущий размер файла с `m_lastFileSize`
3. Если размер не изменился — возвращает `false` (нет новых данных)
4. Читает весь файл в `QByteArray`
5. Вызывает `parseFile(data, selfNick)`
6. Обновляет `m_lastFileSize`
7. Возвращает `true`

#### Метод `void parseFile(const QByteArray &data, const QString &selfNick)`

Последовательно вызывает:
1. `buildEntityMapFromArena(region)` — строит карты игроков из пакета `UpdateArena`
2. `extractEntityMapFromResult(data, zlibOffset)` — более надёжный путь через `battle_results`
3. `parseEvents(region, selfNick)` — разбирает события боя

#### Метод `void buildEntityMapFromArena(const QByteArray &region)`

Ищет пакеты типа `EntityMethod` с subtype `55` (UpdateArena).  
Разбирает Protobuf через `parsePb()`.  
Заполняет `m_entityToNick`, `m_entityToTeam`, `m_entityToVehicle`.  
Находит `m_selfEid` по совпадению nickname с `selfNick`.

#### Метод `void parseEvents(const QByteArray &region, const QString &selfNick)`

Итерирует все пакеты через `iterPackets()`. Для каждого пакета типа `EntityMethod`:
- Определяет тип события по полям Protobuf
- **Damage**: `shooter == self` → создаёт `DamageEvent`, добавляет в `m_events`
- **Incoming**: `target == self` → создаёт `IncomingEvent`
- **Blocked**: вычисляет разницу фактического и нанесённого урона
- **Assist**: `ally != self, target != self` → создаёт `AssistEvent`
- Для каждого события создаёт `BattleEvent` через фабричные методы с текущими счётчиками

#### Вспомогательные методы

**`static QVector<Packet> iterPackets(const QByteArray &region)`**  
Парсит бинарный поток пакетов. Каждый пакет: `[4б length][4б type][4б gameTime][length б payload]`.  
Возвращает вектор `Packet { type, gameTime, payload }`.

**`static QByteArray zlibDecompress(const QByteArray &data, int offset)`**  
Декомпрессия zlib-блока начиная с `offset`.

**`static QVector<int> findZlibOffsets(const QByteArray &data)`**  
Поиск всех вхождений zlib magic bytes `0x78 0x9C` в данных реплея.

**`static QVector<PbField> parsePb(const QByteArray &buf)`**  
Ручной парсер Protobuf без внешних библиотек. Возвращает вектор полей `{fn, wt, iv, bv}` (field_number, wire_type, integer_value, bytes_value).

**`static quint64 varint(const QByteArray &b, int &pos)`**  
Декодирует Protobuf varint начиная с позиции `pos`, увеличивает `pos`.

#### Метод `void reset()`

Полная очистка: все векторы событий, счётчики, карты сущностей, `m_selfEid = -1`, `m_lastFileSize = 0`.

---

### 2.3 ReplayParser

**Файл:** `controller/ParseReplay.h`

Упрощённый парсер для быстрого извлечения **только списка игроков** из реплея. Не зависит от Qt (использует `std::filesystem`, `std::ifstream`). Создаётся и используется в `ApiController::loadReplayPlayers()`.

#### Конструктор

```cpp
explicit ReplayParser(QFile *logFile)
```
Принимает открытый лог-файл для записи диагностики. Если файл не открыт — пытается открыть на дозапись.

#### Метод `vector<PlayerRaw> parseReplayFile(const filesystem::path &path)`

1. Читает весь файл в `vector<uint8_t>`
2. Проверяет magic bytes `0x12345678` (первые 4 байта)
3. Пропускает заголовок: `+8 байт` мусор → `+1` длина хэша → `+hash_len` → `+1` длина версии → `+ver_len+1`
4. Итерирует пакеты: `[4б length][4б type][4б clock][length б payload]`
5. Ищет пакеты `type == 8 && length > 1000`
6. Для каждого такого пакета вызывает `parseEntityMethod(payload)`
7. Возвращает все `PlayerRaw` с непустым никнеймом и ненулевой командой

#### Метод `vector<PlayerRaw> parseEntityMethod(const vector<uint8_t> &payload)`

1. Читает `uint32_t sub_type` по смещению `+4`
2. Если `sub_type != 55` — возвращает пустой вектор (не UpdateArena)
3. Пропускает `+8` байт (inner_length + unknown)
4. Декодирует один varint (длина proto-блока)
5. Передаёт оставшиеся байты в `parseUpdateArenaManual()`

#### Метод `vector<PlayerRaw> parseUpdateArenaManual(const vector<uint8_t> &payload)`

Разбирает Protobuf вручную:
- Поле `1` (repeated LEN) → блок с игроками
  - Внутри блока: поле `1` (LEN) → данные одного игрока → `parsePlayerManual()`
- Все остальные поля → `skipField()`

#### Метод `PlayerRaw parsePlayerManual(const vector<uint8_t> &data)`

| Protobuf field | wire_type | Поле `PlayerRaw` |
|----------------|-----------|------------------|
| 2 | LEN | `tank_id` (первые 2 байта little-endian) |
| 3 | LEN | `nickname` (UTF-8 строка) |
| 4 | VARINT | `team_number` |
| 7 | VARINT | `account_id` (uint64) |
| 8 | LEN | `clan_tag` |
| 11 | VARINT | `platoon_number` |

#### Метод `static uint64_t decodeVarint(const uint8_t *data, size_t &pos, size_t size)`

Стандартный декодер Protobuf varint: читает байты пока установлен старший бит, собирает в `uint64_t` по 7 бит.

#### Метод `void skipField(int wire_type, ...)`

Пропускает поле согласно wire_type:
- `0` (VARINT) → вызывает `decodeVarint`
- `2` (LEN) → читает длину, пропускает
- `1` (64-bit) → `pos += 8`
- `5` (32-bit) → `pos += 4`
- иначе → `pos = data.size()` (аварийный выход)

---

### 2.4 HWIDActivator

**Файл:** `controller/HWID.h`  
Утилитный класс с только статическими методами. Объект не создаётся.

#### Константа

```cpp
static inline const char *SECRET_SEED = "bambam";
```

#### Статические методы

**`static QString getHWID()`**

Формирует JSON-строку из трёх системных значений:
```json
{"machine_id":"...","kernel":"...","cpu_arch":"..."}
```
Где:
- `machine_id` = `QSysInfo::machineUniqueId()` — из реестра `SOFTWARE\Microsoft\Cryptography\MachineGuid`
- `kernel` = `QSysInfo::kernelVersion()` — версия ядра Windows
- `cpu_arch` = `QSysInfo::currentCpuArchitecture()` — `"x86_64"` и т.д.

Вычисляет `SHA-256(json_string)` через `QCryptographicHash`, возвращает hex-строку (64 символа).

**`static QString generateKey()`**

```
combined = "bambam:" + HWID
key = SHA-256(combined).hex.left(16).toUpper()
```
Возвращает 16-символьный ключ активации в верхнем регистре, например `"A3F2B9C1D4E5F678"`.

**`static bool activate(const QString &key)`**

1. Проверяет `key == generateKey()`
2. При совпадении: создаёт директорию `%AppData%/StatsOverlay` если не существует
3. Записывает JSON:
   ```json
   { "hwid": "...", "key": "...", "activated": true }
   ```
4. Возвращает `true`

**`static bool checkActivation()`**

1. Читает `activation.json`
2. Извлекает `hwid`, `key`, `activated`
3. Возвращает `true` только если:
   - `hwid == getHWID()` (файл с этой машины)
   - `key == generateKey()` (ключ верный для текущего железа)
   - `activated == true`

**`static QString getActivationFilePath()`**

Возвращает путь `%AppData%/StatsOverlay/StatsOverlay/activation.json`.

#### Приватные низкоуровневые методы (WinAPI, запасные)

**`static string sha256_hex(const string &input)`** — SHA-256 через `BCrypt API`. Не используется в основном пути (там `QCryptographicHash`), оставлен для совместимости.

**`static string _get_machine_id()`** — читает `MachineGuid` из реестра напрямую через `RegQueryValueExA`.

**`static string _get_processor_id()`** — читает CPUID через `__cpuid`, возвращает 16-символьную hex-строку.

**`static string _get_disk_serial()`** — читает серийный номер диска `C:\` через `GetVolumeInformationA`.

---

### 2.5 MainStatsData

**Файл:** `controller/data/MainStatsData.h`

#### Структура `StatsData`

```cpp
struct StatsData {
    int64_t credits, gold, exp_battle, exp_free;
    int64_t battles, wins, losses, totalDamage;
    StatsData operator-(const StatsData &other) const { ... }  // поэлементная разность
};
```

#### Поля класса

```cpp
StatsData firstData;    // снимок на старте сессии
StatsData newData;      // последнее полученное значение
StatsData currentData;  // currentData = newData - firstData
```

#### Методы

**`void initialStats(const StatsData &stats)`**  
Сохраняет `firstData = stats`. Вычисляет `currentData = stats - firstData` (будет нулевым).

**`void updateStats(const StatsData &newStats)`**  
Обновляет `newData = newStats`. Пересчитывает `currentData = newData - firstData`.

**Геттеры** (все возвращают значение из `currentData`):
- `getGold()`, `getCredits()`, `getExpBattle()`, `getExpFree()`
- `getBattles()`, `getWins()`, `getLosses()`, `getTotalDamage()`

**Вычисляемые геттеры:**
- `getAvgDamage()` = `totalDamage / battles`, `-1` если `battles <= 0`
- `getAvgExp()` = `exp_battle / battles`, `-1` если `battles <= 0`
- `getPercentWins()` = `wins / battles * 100`, округление `*100/100`, `-1.0f` если `battles <= 0`

---

### 2.6 RatingStatsData

**Файл:** `controller/data/RatingStatsData.h`

Аналогична `MainStatsData` но с особой логикой пересчёта рейтинга MM.

#### Структура `RatingData`

```cpp
struct RatingData {
    double mm_rating;       // абсолютный рейтинг (уже в формате 3000+)
    double diff;            // изменение рейтинга от старта сессии
    int64_t calib_battles;  // сыграно калибровочных боёв (0–10)
    int64_t exp_battle, battles, wins, losses, totalDamage;
};
```

#### Особенности `initialStats()`

```cpp
// raw_rating из API умножается на 10 и смещается на 3000
firstData.mm_rating = 3000.0 + stats.mm_rating * 10.0;
firstData.diff = 3000.0 + stats.mm_rating * 10.0;  // начальная точка для diff
currentData.calib_battles = 10 - stats.calib_battles;  // осталось боёв (из 10)
currentData.mm_rating = firstData.mm_rating;
currentData.diff = 0;  // изменение = 0 в начале сессии
```

#### Особенности `updateStats()`

```cpp
currentData.diff = (3000.0 + newStats.mm_rating * 10.0) - firstData.mm_rating;
currentData.mm_rating = 3000.0 + newStats.mm_rating * 10.0;
currentData.calib_battles = 10 - newStats.calib_battles;
```

#### Геттеры

- `getRating()` → `static_cast<int64_t>(currentData.mm_rating)` — отображаемый рейтинг
- `getDiffRating()` → `static_cast<int64_t>(currentData.diff)` — изменение за сессию (может быть отрицательным)
- `getCalibBattles()` → `currentData.calib_battles` — осталось калибровочных боёв
- `getAvgDamage()`, `getAvgExp()`, `getPercentWins()` — аналогично MainStatsData

---

### 2.7 MasteryStatsData

**Файл:** `controller/data/MasteryStatsData.h`

#### Структура `MasteryData`

```cpp
struct MasteryData {
    int64_t mastery;    // «Туз» — высший знак мастерства
    int64_t mastery_1;  // 1-й знак (самый нижний)
    int64_t mastery_2;  // 2-й знак
    int64_t mastery_3;  // 3-й знак
    MasteryData operator-(const MasteryData &other) const { ... }
};
```

Порядок: `mastery` = «Туз» (4-й), `mastery_1..3` = 1–3-й знаки. В UI отображаются слева направо: Туз, 1, 2, 3.

#### Геттеры

- `getFirstData()` — абсолютные значения на начало сессии (для отображения общего числа)
- `getCurrentData()` — разница за сессию

`OtherStats` использует `getCurrentData()` для отображения знаков полученных за сессию.

---

### 2.8 OtherStatsData

**Файл:** `controller/data/OtherStatsData.h`

#### Структура `OtherData`

```cpp
struct OtherData {
    int64_t battles, hits, shots, survived;
    int64_t frags, receiverDamage, totalDamage, lifeTime;
};
```

#### Вычисляемые геттеры

| Метод | Формула | Edge cases |
|-------|---------|------------|
| `getPercentHits()` | `hits / shots * 100` | `-1.0f` если `shots <= 0` |
| `getPercentSurvived()` | `survived / battles * 100` | `-1.0f` если `battles <= 0` |
| `getLifeTime()` | `lifeTime / battles` (сек) | `-1` если `battles <= 0` |
| `getDamageK()` | `totalDamage / receiverDamage` | `-1.0f` если `receiverDamage <= 0` |
| `getFragsK()` | `frags / (battles - survived)` | `-1.0f` если `battles <= 0`; если `deaths == 0` → возвращает `frags` |
| `getBattles()` | `currentData.battles` | — |
| `getShots()` | `currentData.shots` | — |
| `getReceiverDamage()` | `currentData.receiverDamage` | — |
| `getDeaths()` | `battles - survived` | — |

---

### 2.9 VehicleStatsData

**Файл:** `controller/data/VehicleStatsData.h`

Наиболее сложный класс данных. Работает с картами вместо одиночных структур.

#### Структуры

```cpp
struct VehicleData {
    int64_t id, battles, totalDamage, wins;
    VehicleData operator-(const VehicleData &other) const { ... }
};

struct VehicleInfo {
    int64_t tank_id;
    std::string name, type, nation, status;
    int64_t tier;
};
```

#### Поля

```cpp
std::unordered_map<int64_t, VehicleData> vehicles;         // сессионная разница
std::unordered_map<int64_t, VehicleData> first_vehicles;   // снимок на старте
std::unordered_map<int64_t, VehicleData> prev_vehicles;    // предыдущее состояние
std::unordered_map<int64_t, VehicleInfo> vehicles_info;    // база данных техники
```

#### Конструктор

Пытается загрузить базу данных техники:
1. `downloadFromGitHub()` — синхронный GET через QNetworkAccessManager + QEventLoop к `raw.githubusercontent.com/MaJluHoBKa/vehicles_db_lesta/main/tanks.json`
2. Если успешно → `loadVehiclesFromJson(data)` → заполняет `vehicles_info`
3. Если не удалось → бросает `std::runtime_error` (fallback на Qt ресурсы закомментирован)

#### Метод `void initialStats(const json &stats)`

Итерирует массив танков из API-ответа.  
Для каждого: создаёт `VehicleData`, сохраняет в `first_vehicles[id]`.  
Вычисляет `vehicles[id] = data - first_vehicles[id]` (будет нулевым).  
Копирует в `prev_vehicles`.

#### Метод `void updateStats(const json &newStats)`

1. `prev_vehicles = vehicles` (сохраняем предыдущее состояние)
2. Для каждого танка из ответа:
   - Если `id` уже есть в `first_vehicles` → `vehicles[id] = newData - first_vehicles[id]`
   - Если новый танк (не был в гараже при старте) → `first_vehicles[id] = newData`, `vehicles[id] = 0`, `prev_vehicles[id] = 0`

#### Метод `const VehicleData *getUpdatedVehicle() const`

Ищет танк у которого `vehicles[id].battles > prev_vehicles[id].battles`.  
Это единственный танк, на котором только что завершился бой.  
Выводит в stdout: имя, бои, средний урон, процент побед.  
Возвращает указатель на `VehicleData` или `nullptr`.

#### Геттеры информации о технике

- `getName(id)` → имя танка из `vehicles_info`, `to_string(id)` если нет
- `getTier(id)` → уровень (1–10), `1` если нет
- `getType(id)` → тип (`"lt"`, `"mt"`, `"ht"`, `"at-spg"`, `"spg"`), `""` если нет
- `getNation(id)` → нация (`"ussr"`, `"germany"` и т.д.), `""` если нет
- `getStatus(id)` → статус (`"default"`, `"premium"`, `"collector"`), `"default"` если нет

#### Метод `bool loadVehiclesFromJson(const QByteArray &raw)`

Парсит JSON через nlohmann/json.  
Ищет ключ `"tanks"` — если есть, работает с вложенным массивом.  
Для каждого объекта: обязательные поля `id`, `name`, `tier`; опциональные `type`, `nation`, `status`.  
Возвращает `false` если `vehicles_info` пуста после парсинга.

---

## 3. Модуль widgets — страницы

### 3.1 MainStats

**Файл:** `widgets/main_stats/main_stats.h/.cpp`  
**Наследование:** `QWidget`  
**Ширина:** фиксированная 260px

Страница основной экономической и боевой статистики сессии. Индекс в `QStackedWidget`: **7**.

#### Поля

```cpp
std::unordered_map<std::string, InfoLabel*> info;  // карта метрик по имени
MainStatsData prev_stats;                           // предыдущее значение для стрелок
ApiController *m_apiController = nullptr;
```

#### Конструктор

Строит UI через многократные вызовы `addContent()`:
1. Экономика: Золото, Кредиты, Боевой опыт, Свободный опыт
2. Горизонтальный разделитель + заголовок «Боевая эффективность»
3. Бои: Проведено боев, Победы, Урон, Опыт

Запускает `QTimer` с интервалом **30 секунд** → `updatingMainStats()`.

#### Метод `void addContent(QVBoxLayout*, QPixmap, QString title, bool is_top, bool is_bottom)`

Создаёт строку статистики: `[иконка] [название] [──────────] [значение] [стрелка]`.  
Шрифт значения — `JetBrainsMono-Bold`. Значение в тёмном прямоугольнике (`background: #383838`).  
Регистрирует виджеты в `info[title]`.

#### Метод `void updatingMainStats()` (слот)

Выполняется через `QtConcurrent::run()` (фоновый поток):
1. Вызывает `m_apiController->update_main_stats()`
2. При успехе передаёт данные в GUI-поток через `QMetaObject::invokeMethod(..., Qt::QueuedConnection)`
3. Вызывает `setValue()` для каждой метрики
4. Вызывает `updateColor(mainStatsData)` для сравнения с `prev_stats`
5. Сохраняет `prev_stats = mainStatsData`

#### Метод `void setValue(string key, int64_t value)`

Форматирует через `formatInt()` (разбивает на группы по 3 цифры через пробел), устанавливает в `info[key]->value`.

#### Метод `void setValue(string key, int64_t value, int64_t wins, int64_t losses)`

Специальный для «Проведено боев»: форматирует как `"42 [28 - 14]"` (бои [победы - поражения]).

#### Метод `void setValue(string key, float value)`

Для процентов: `formatFloat()` + `"%"`. Например `"58.72%"`.

#### Метод `void updateColor(const MainStatsData &mainStatsData)`

Сравнивает каждое значение с `prev_stats`:
- Выросло → стрелка `▲`, цвет `#66ff66` (зелёный)
- Упало → стрелка `▼`, цвет `#ff6666` (красный)

Применяет цвет и к стрелке, и к горизонтальной линии (`info[key]->line`).

#### Метод `void resetValue()`

Сохраняет текущие данные как новую базу (`prev_stats`).  
Устанавливает все значения в `"-"`, стрелки в `""`, цвета в `#e2ded3`.

#### Вспомогательные методы форматирования

**`formatFloat(float value)`** — разделяет целую и дробную части, форматирует с пробелами тысяч. Например `1 234.56`.

**`formatInt(int64_t value)`** — разбивает число пробелами каждые 3 цифры: `1 234 567`.

---

### 3.2 RatingStats

**Файл:** `widgets/rating_stats/rating_stats.h/.cpp`  
**Наследование:** `QWidget`  
**Ширина:** фиксированная 260px  
**Индекс:** **6**

Архитектурно идентична `MainStats`. Ключевые отличия:

**Метрики:**
- Текущий рейтинг, Прогресс рейтинга, Калибровочные бои
- Разделитель + «Боевая эффективность»
- Проведено боев, Победы, Урон, Опыт

**Слот `updatingRatingStats()`:**  
Интервал таймера **30 секунд**.  
Вызывает `update_rating_stats()`.  
Для «Проведено боев» использует `setValue(key, battles, wins, losses)`.

**Особенность отображения рейтинга:**  
`getRating()` возвращает уже преобразованное значение `3000 + raw*10`.  
`getDiffRating()` может быть отрицательным (потеря рейтинга).

---

### 3.3 OtherStats

**Файл:** `widgets/other_stats/other_stats.h/.cpp`  
**Наследование:** `QWidget`  
**Ширина:** фиксированная 260px  
**Индекс:** **4**

#### Особенность UI

Верхняя часть — блок медалей мастерства из 4 иконок + 4 числовых значений в контейнере с рамкой.

Иконки медалей (слева направо): Туз, 1-й знак, 2-й знак, 3-й знак.  
Значения хранятся в `QHash<QString, QLabel*> data_master_values`:
- `"Mastery_1"` → значение Туза
- `"Mastery_2"` → значение 1-го знака
- `"Mastery_3"` → значение 2-го знака
- `"Mastery_4"` → значение 3-го знака

Нижняя часть — метрики через `addContent()`:
- Процент попаданий, Процент выживания
- Коэфф. урона, Коэфф. уничтожения
- Ср. время выживания

#### Слот `updatingOtherStats()`

Интервал **30 секунд**. Выполняет **два** параллельных запроса в одном `QtConcurrent::run()`:
1. `update_mastery_stats()` → обновляет `data_master_values`
2. `update_other_stats()` → обновляет `info[]`

#### Метод `setValue(string key, int64_t value)`

Специальный для «Ср. время выживания»: преобразует секунды в формат `M:SS`.  
Например 185 секунд → `"3:05"`.

#### Метод `setValueK(string key, float value)`

Для коэффициентов: `formatFloat()` без знака процента. Например `"1.87"`.

---

### 3.4 VehicleStats

**Файл:** `widgets/vehicles_stats/vehicles_stats.h/.cpp`  
**Наследование:** `QWidget`  
**Ширина:** максимальная 260px (может расширяться)  
**Индекс:** **5**

#### Конструктор

Создаёт заголовок, разделитель, шапку таблицы с иконками (тип, название, бои, победы, урон).  
Создаёт `QScrollArea` с `QVBoxLayout` для строк танков.  
Добавляет fade-overlay снизу скроллируемой зоны (градиент `rgba(30,30,30,0) → rgba(30,30,30,255)`).  
Запускает `QTimer` с интервалом **30 секунд**.

#### Конструктор принимает

```cpp
VehicleStats(ApiController *apiController, GunMarks *gunMark, QWidget *parent)
```

Ссылка на `GunMarks` нужна для вызова `updateInfo(tankName, totalDamage)` при смене танка.

#### Слот `updatingVehicleStats()`

1. Вызывает `update_vehicles_stats()` через `QtConcurrent::run()`
2. Получает последний танк через `get_updated_vehicles()`
3. Если танк найден и `!isLock()`:
   - Получает `tank_id`, имя, тип, нацию, уровень, статус
   - Обновляет UI строку танка
   - Вызывает `gunMark->updateInfo(tankName, totalDamage)` если `isMark()`
4. Строки танков добавляются/обновляются через `addTankRow()` / `updateExistingRow()`

#### Метод `addTankRow(...)`

Создаёт строку: иконка уровня (I–X как текст) + иконка типа/нации/статуса + название + бои + % побед + средний урон.  
Иконки типа техники загружаются по пути `":vehicle_stats/resources/icons/lt_default.png"` с учётом статуса (default/premium/collector).  
Иконки наций — флаги по коду нации (`ussr`, `germany`, `usa` и т.д.).  
Строка добавляется в `QVBoxLayout` прокручиваемой зоны.  
Устанавливает `tag` на `QFrame` для поиска по `findViewWithTag(tankId)`.

---

### 3.5 PlayerStats

**Файл:** `widgets/player_stats/player_stats.h/.cpp`  
**Наследование:** `QWidget`  
**Ширина:** максимальная 260px  
**Индекс:** **2**

#### Поля

```cpp
PlayerStatsWindow *alliesWindow  = new PlayerStatsWindow("Союзники");
PlayerStatsWindow *enemiesWindow = new PlayerStatsWindow("Противники");
std::vector<Player> allies, enemies;
bool isPlayers = false;   // загружены ли игроки для текущего боя
bool isOn      = false;   // включена ли функция
bool busy      = false;   // блокировка параллельных запросов

// Кнопки-чекбоксы видимости столбцов
QPushButton *playersBox, *tanksBox, *battlesBox, *winsBox, *damageBox;
QLabel *playersCheck, *tanksCheck, *battlesCheck, *winsCheck, *damageCheck;
bool is_players, is_tanks, is_battles, is_wins, is_damage;  // все = true
```

#### `GlobalHotkeyFilterPlayers` (локальный класс в .cpp)

Реализует `QAbstractNativeEventFilter`.  
Перехватывает `WM_HOTKEY` с `wParam == 4` (зарегистрирован как `Ctrl + -` = `MOD_CONTROL + VK_OEM_MINUS`).  
Вызывает `PlayerStats::hotHide()`.

#### Конструктор

- Создаёт `alliesWindow` и `enemiesWindow` (скрыты)
- Кнопка «Вкл/Выкл» — вызывает `setIsOn()`
- 5 кнопок-чекбоксов (Игроки, Танки, Бои, Победы, Урон) → `toggleXxx()` слоты
- Слайдер прозрачности (10–100%) → синхронизирует оба окна через `setOpacity()`
- Регистрирует горячую клавишу `Ctrl + -` через `RegisterHotKey(NULL, 4, MOD_CONTROL, VK_OEM_MINUS)`
- `QTimer` 10 секунд → `updatingPlayerStats()`

#### Метод `void setIsOn()`

При включении: меняет стиль кнопки на «активный» (зелёная рамка), показывает оба окна.  
При выключении: скрывает окна, сбрасывает `isOn = false`.

#### Слот `void updatingPlayerStats()`

1. Если `!isOn` → очищает оба окна, `isPlayers = false`
2. Если `busy == true` → пропускает итерацию (предотвращает параллельные запросы)
3. Устанавливает `busy = true`
4. `QtConcurrent::run()`:
   - Вызывает `m_apiController->loadReplayPlayers()`
   - Если успех и `!isPlayers`:
     - Вызывает `get_players_stats()`
     - Передаёт в GUI: `alliesWindow->setData(allies)`, `enemiesWindow->setData(enemies)`
     - `isPlayers = true`
   - Если нет активного реплея: очищает оба окна, `isPlayers = false`
   - В любом случае: `busy = false`

#### Слоты `toggleXxx()`

5 одинаковых по структуре слотов (togglePlayers, toggleTanks, toggleBattles, toggleWins, toggleDamage).  
При вызове инвертируют флаг `is_xxx`, обновляют галочку в кнопке, вызывают `alliesWindow->toggleXxx()` и `enemiesWindow->toggleXxx()`.

#### Метод `void hotHide()`

Переключает видимость `alliesWindow` и `enemiesWindow` по горячей клавише.

---

### 3.6 GunMarks

**Файл:** `widgets/gun_marks/gun_marks.h/.cpp`  
**Наследование:** `QWidget`  
**Ширина:** максимальная 260px  
**Индекс:** **1**

#### Поля

```cpp
GunMarksWindow *gunMarksWindow = new GunMarksWindow();  // плавающее окно с данными
bool isMarks = false;    // есть ли данные для отображения
bool isOn    = false;    // активно ли отслеживание
bool isFirst = true;     // первое обновление (для блокировки танка)

// Поля ввода
QLineEdit *tf;   // планка отметки (требуемый средний урон)
QLineEdit *btf;  // текущий средний урон

// Данные отслеживания
std::string tankName;
int64_t mark;              // планка (целевой средний урон)
std::vector<int64_t> damages;  // скользящее окно урона (100 боёв)
int64_t currentDamage;
int64_t battles = 100;
int64_t totalDamage;       // суммарный урон за все бои
int64_t firstDamage;       // урон в нулевом бою (база)
int64_t clearedDamage;     // урон вышедшего из окна боя

bool is_next  = true;   // показывать прогнозы +0.5/1/2%
bool is_shots = true;   // показывать требуемый урон
```

#### Конструктор

- Создаёт кнопки: «Урон для +0.5/1/2%», «Требуемый урон» — с галочками через `createButtonCheckbox` lambda
- Поля ввода `tf` (планка) и `btf` (текущий средний), с валидатором 0–9999
- Кнопка «Начать/Закончить» → `setIsOn()`
- Слайдер прозрачности → сохраняет в `QSettings("MyCompany", "StatsOverlay")`
- `QTimer` 10 секунд → `updatingMarkStats()`

#### Метод `void setIsOn()`

**При включении** (если поля заполнены и `mark > 0`):
1. Инициализирует `damages` как 100 элементов равных `btf.text().toInt()`
2. Устанавливает `mark = tf.text().toInt()`
3. Показывает `gunMarksWindow`, вызывает `gunMarksWindow->init(mark)`
4. Флаги: `isOn = true`, `isMarks = true`, `hotVisible = true`
5. Сообщает `m_apiController->setMark(true)` — блокирует смену танка в VehicleStats

**При выключении**:
1. Очищает все данные и флаги
2. Скрывает и сбрасывает `gunMarksWindow`
3. Разблокирует: `m_apiController->setMark(false)`, `setTankLock(false)`

#### Метод `void updateInfo(std::string tankName, int64_t totalDamage)` (вызывается из VehicleStats)

При первом вызове (`isFirst == true`):
- Сохраняет `tankName`
- Меняет иконку с unlock на lock
- Отображает название танка
- `isFirst = false`

При последующих вызовах:
- `damageForBattle = totalDamage - this->totalDamage` (урон за последний бой)
- Добавляет `damageForBattle` в конец `damages`, удаляет первый элемент (скользящее окно 100 боёв)

#### Слот `void updatingMarkStats()`

Если `isOn && isMarks && isAuth()`:  
Суммирует `damages` → `damage`.  
Вызывает `gunMarksWindow->setData(damage, mark, damages[0])`.

#### Слоты `toggleNext()` / `toggleShots()`

Инвертируют флаги `is_next` / `is_shots`.  
Обновляют галочку в кнопке.  
Вызывают `gunMarksWindow->toggleNext(bool)` для скрытия/показа строк прогноза.

---

### 3.7 InfoPage

**Файл:** `widgets/info_page/info_page.h/.cpp`  
**Наследование:** `QWidget`  
**Ширина:** максимальная 280px  
**Индекс:** **0** (высота окна 400px)

Принимает в конструктор указатели на все страницы статистики (`MainStats*`, `RatingStats*`, `VehicleStats*`, `OtherStats*`) для вызова их метода `resetValue()` при сбросе.

#### UI-элементы

- Заголовок «STATS OVERLAY v2.3»
- `QScrollArea` с текстом лицензионного соглашения (высота 185px, кастомный скроллбар 4px)
- Кнопка «Центр поддержки» → `QDesktopServices::openUrl("https://lesta.ru/support/ru/")`
- Кнопка «Сбросить статистику» → `onResetClicked()`
- Кнопка «Авторизоваться» / «Выход» → `onAuthClicked()`
- `QLabel *message` — статусная строка (никнейм или сообщение об ошибке)
- `QLabel` «© Леста Игры. Все права защищены»

#### Слот `void onResetClicked()`

```cpp
m_apiController->reset();
m_mainStats->resetValue();
m_ratingStats->resetValue();
m_otherStats->resetValue();
// m_vehicleStats->resetValue() — закомментировано
```

#### Слот `void onAuthClicked()`

**Если авторизован** → вызывает `m_apiController->logout()`:
- При успехе: `message->setText("Выполнен выход")`, кнопка → «Авторизоваться»
- При ошибке: `message->setText("Ошибка выхода...")`

**Если не авторизован** → запускает `std::thread` (detach):
- Вызывает `m_apiController->login()` (блокирующий — ждёт callback)
- При успехе: `QMetaObject::invokeMethod(...)` → обновляет UI в GUI-потоке: `message->setText(nickname)`, кнопка → «Выход»
- При ошибке: устанавливает сообщение об ошибке

---

### 3.8 Widgets

**Файл:** `widgets/widgets_page/widgets.h/.cpp`  
**Наследование:** `QWidget`  
**Ширина:** максимальная 260px  
**Индекс:** **3**

Контейнерная страница управления боевыми виджетами и трекером реплеев.

#### Поля

```cpp
WidgetsWindow *widgetsWindow = nullptr;   // плавающее окно с тайлами статистики
DamageWidget  *dmgWidget     = nullptr;   // виджет урона в бою
HistoryWidget *hWidget       = nullptr;   // виджет истории событий

ReplayTracker *m_replayTracker = nullptr;
bool m_replayBusy = false;   // мьютекс для ReplayTracker

bool isOn = false;
bool is_rating, is_master, is_battles, is_xp, is_alive, is_shots, is_wins, is_damage;
bool is_dmg_widget, is_history_widget;  // все = true
```

#### Конструктор

- Создаёт `widgetsWindow`, `dmgWidget`, `hWidget` (все скрыты)
- Сетка 4×2 кнопок-чекбоксов: Бои/Рейтинг, Победы/Мастер, Урон/Выжил, Опыт/Точность
- Кнопка «Включить/Выключить» → `setIsOn()`
- Два чекбокса боевых виджетов: «Эффек-ть» (DmgWidget), «Cобытия» (HistoryWidget)
- Слайдер прозрачности → `widgetsWindow->setOpacity()`
- `new ReplayTracker(this)`
- `QTimer` 5 секунд → `updateBattleDamage()`
- `QTimer` 30 секунд → `updatingWidgets()`

#### Слот `void updatingWidgets()`

Читает данные из `ApiController` (без HTTP-запросов — берёт уже загруженные):
- `getMainStats()`, `getRatingStats()`, `getOtherStats()`, `getMasteryStats()`
- Объединяет обычные бои и рейтинговые: `battles = main.getBattles() + rating.getBattles()`
- Вычисляет агрегированные показатели:
  - `wins` = суммарные победы / суммарные бои × 100
  - `damage` = суммарный урон / суммарные бои
  - `xp` = суммарный опыт / суммарные бои
- Обновляет тайлы: `widgetsWindow->updateTile(key, value)`
- Для побед: `widgetsWindow->updateWins(wins)` — с цветовой индикацией
- `Мастер` = `masteryData.getCurrentData().mastery`
- `Рейтинг` = `ratingStats.getRating()`

#### Слот `void updateBattleDamage()`

Интервал **5 секунд**. Логика с `m_replayBusy` мьютексом:

1. Ищет директорию `Documents/TanksBlitz/replays/recording*/data.replay`
2. Если файл не найден → сбрасывает `ReplayTracker`, скрывает и очищает `dmgWidget` и `hWidget`
3. Если найден → `m_replayTracker->update(replayPath, selfNick)`
4. При получении новых данных:
   - Создаёт `BattleData { totalDamage, totalBlocked, totalAssist }`
   - `dmgWidget->setData(data)` — показывает если `is_dmg_widget`
   - `hWidget->setHistory(history)` — показывает если `is_history_widget`
5. Выводит подробный дебаг-лог в консоль (событие по событию)

#### Слоты `toggleXxx()` (10 слотов)

Все 10 слотов работают через `QMetaObject::invokeMethod(..., Qt::QueuedConnection)`.  
Инвертируют флаг, обновляют галочку, вызывают соответствующий метод `widgetsWindow->toggleXxx()`.

**`toggleBattleStats()`** — управляет `is_dmg_widget` (не показывает/скрывает сам виджет, флаг проверяется в `updateBattleDamage()`).

**`toggleHistory()`** — управляет `is_history_widget` аналогично.

---

## 4. Подокна

### 4.1 GunMarksWindow

**Файл:** `widgets/gun_marks/gunmarks_window/gunmarks_window.h/.cpp`  
**Наследование:** `QWidget`  
**Тип окна:** `WindowStaysOnTopHint | FramelessWindowHint | Tool`  
**Ширина:** фиксированная 180px  
**Позиция:** `x=30`, `y = (screenH - h)/2 + 150`

Плавающее полупрозрачное окно отображения прогресса отметки. Создаётся и хранится в `GunMarks`.

#### UI-элементы

- `QLabel *percent` — текущий % (`"94.37%"`)
- «В бою:» + `QLabel *inBattle` — изменение за последний бой (`"+0.12%"`)
- `QProgressBar *progress` — 0–100%, меняет цвет при 100%
- «Сум. урон» + `QLabel *currentSummaryDamage` — формат `"1 234/1 500"`
- «Треб. урон» + `QLabel *reqAvgDamage`
- «+0.5%» + `QLabel *next05`
- «+1%» + `QLabel *next1`
- «+2%» + `QLabel *next2`

#### Метод `void init(int64_t mark)`

Инициализирует начальное состояние: `currentSummaryDamage = "0/mark"`, `reqAvgDamage = mark`.

#### Метод `void setData(int64_t damage, int64_t mark, int64_t firstDamage)`

Основной метод обновления. Вычисляет:

```
totalPlannedDamage = mark * 100
sumDamage = damage / 100.0              // текущий средний урон (float)
curPercent = (damage / totalPlannedDamage) * 100 (округление до 2 знаков)

postSumDamage = (damage - firstDamage) / 100
postPercent = postSumDamage * 100 / mark
lostPercent = postPercent - curPercent   // изменение за последний бой

reqDamage = (mark*100 - (damage - firstDamage)) / 100 + sumDamage

next05 = mark*100/200 + firstDamage     // урон нужный для +0.5%
next1  = mark*100/100 + firstDamage     // для +1%
next2  = mark*100/50  + firstDamage     // для +2%
```

При `curPercent >= 100`: отображает «100.0%», прогресс зелёный (`#4cd964`).

#### Метод `void clearData()`

Сбрасывает все значения в `"0.0%"`, `"0"`, `0`.

#### Метод `void toggleNext(bool visible)`

Скрывает/показывает блоки `next05_layout`, `next1_layout`, `next2_layout`.  
После изменения: `updateGeometry()` + `QTimer::singleShot(0, adjustSize)` для пересчёта высоты.

#### Метод `void setOpacity(double opacity)`

Сохраняет `m_backgroundOpacity`. Реальное применение — в `paintEvent()`.

#### `void paintEvent(QPaintEvent*)`

Рисует скруглённый прямоугольник `drawRoundedRect(rect(), 5, 5)` с цветом `rgba(30,30,30, opacity*255)` и белой рамкой.

#### Drag-and-drop

`mousePressEvent` + `mouseMoveEvent` + `mouseReleaseEvent` — стандартный drag по `event->globalPos() - frameGeometry().topLeft()`.

#### Утилиты форматирования

**`formatFloat(float)`** — 2 знака после точки, убирает незначащие нули. `"1.20"` → `"1.2"`, `"1.00"` → `"1"`.

**`formatInt(int64_t)`** — пробелы каждые 3 цифры. `1234567` → `"1 234 567"`.

---

### 4.2 PlayerStatsWindow

**Файл:** `widgets/player_stats/split_players/players_window.h/.cpp`  
**Наследование:** `QWidget`  
**Тип окна:** `WindowStaysOnTopHint | FramelessWindowHint | Tool`  
**Позиция:** Союзники: `x=30`, Противники: `x = screenW - 400`

Плавающее окно со списком игроков одной команды (7 игроков + строка AVG).

#### Поля

```cpp
QVBoxLayout *data_players;            // контейнер строк
std::vector<PlayerRows*> rows_players;  // 7 строк игроков
PlayerRows *avg;                      // строка среднего (AVG)
int defaultWidth = 386;               // текущая ширина (изменяется при toggle)
double m_backgroundOpacity;
// иконки заголовков
QLabel *playerIcon, *tankIcon, *battlesIcon, *winsIcon, *damageIcon;
```

#### Конструктор

- Заголовок с названием («Союзники» / «Противники»)
- Строка иконок-заголовков (5 колонок)
- `addRows()` — создаёт 8 строк (7 + AVG)

#### Метод `void addRows()`

Создаёт 8 `PlayerRows`. Каждая строка: 5 `QLabel` шириной `[105, 105, 50, 50, 50]` px.  
Строка 8 (индекс 7) — это `avg` с горизонтальным разделителем перед ней.

#### Метод `void setData(vector<Player> players)`

Заполняет строки данными. Для каждого из 7 игроков:
- Никнейм обрезается до 12 символов + `"..."` если длиннее
- Название танка — аналогично
- **Цветовая индикация боёв:**
  - `< 5 000` → `#d13b49` (красный)
  - `5 000–15 000` → `#80d41d` (зелёный)
  - `15 000–30 000` → `#5cded6` (бирюзовый)
  - `>= 30 000` → `#a08bea` (фиолетовый)
- **Цветовая индикация побед:**
  - `>= 70%` → `#a08bea`
  - `60–70%` → `#5cded6`
  - `50–60%` → `#80d41d`
  - `< 50%` → `#d13b49`
- **Цветовая индикация среднего урона:**
  - `< 1 000` → красный
  - `1 000–1 500` → зелёный
  - `1 500–2 500` → бирюзовый
  - `>= 2 500` → фиолетовый

Строка AVG (`avg`): суммирует battles, wins, damage по всем 7 игрокам → вычисляет средние. Цвет `#ffd166` (золотой).

#### Метод `void clearData()`

Очищает текст всех строк.

#### Toggle-методы (5 штук)

`togglePlayers`, `toggleTanks`, `toggleBattles`, `toggleWins`, `toggleDamage` — скрывают/показывают соответствующие `QLabel` во всех строках, изменяют `defaultWidth`, вызывают `updateWindowSize()`.

`updateWindowSize()` — устанавливает `setFixedWidth(max(150, defaultWidth))`.

#### `void paintEvent`

Рисует скруглённый прямоугольник `rgba(30,30,30, opacity*255)`, без пера.

---

### 4.3 DamageWidget

**Файл:** `widgets/widgets_page/dmg_window/dmg.h/.cpp`  
**Наследование:** `QWidget`  
**Тип окна:** `WindowStaysOnTopHint | FramelessWindowHint | WindowTransparentForInput | Tool`  
**Ширина:** максимальная 75px  
**Позиция:** `x=0`, `y = screenH - h - 200`

Прозрачный виджет без захвата мыши (`WA_TransparentForMouseEvents`). Отображается поверх игры, не мешает управлению.

#### UI-элементы

3 строки: иконка (30px) + `QLabel`:
- `damage` — нанесённый урон (иконка total_damage)
- `block` — заблокированный урон (иконка block)
- `assist` — урон ассистов (иконка assist)

#### Метод `void setData(const BattleData &data)`

Устанавливает текст через `formatInt()`:
```cpp
damage->setText(formatInt(data.damage));
block ->setText(formatInt(data.block));
assist->setText(formatInt(data.assist));
```

#### `void paintEvent`

Рисует горизонтальный градиент:
- Левый край: `rgba(35,35,35, 204)` — 80% непрозрачность
- 60% ширины: `rgba(35,35,35, 120)` — 47%
- Правый край: `rgba(35,35,35, 0)` — полностью прозрачный

Создаёт эффект «тающего» фона справа.

---

### 4.4 HistoryWidget

**Файл:** `widgets/widgets_page/history_window/hdmg.h`  
**Наследование:** `QWidget`  
**Тип окна:** аналогично `DamageWidget` (прозрачный, без мыши)  
**Ширина:** фиксированная 175px

#### Структура `Row`

```cpp
struct Row {
    QWidget *widget = nullptr;  // контейнер строки
    QLabel *dmg     = nullptr;  // текст события
};
static constexpr int MAX_ROWS = 10;
static constexpr int ROW_H    = 25;
```

#### Конструктор

Создаёт `MAX_ROWS` строк высотой `ROW_H`. Все скрыты изначально.

#### Метод `void setHistory(const QVector<BattleEvent> &history)`

1. Фильтрует события: только `Damage`, `Incoming`, `Blocked`
2. Берёт последние `MAX_ROWS` событий (с конца вектора)
3. Для каждой строки (от новейшего к старейшему):
   - Получает имя танка через `m_apiController->getVehicleName(ev.vehicle_cd)`
   - Если имя `"0"` → `"???"`
   - Форматирует: `"1 234 - ИмяТанка"`
   - **Damage** → зелёный текст (`#a3f09b`), фон `damage_green.png`
   - **Incoming** → красный текст (`#f09e9e`), фон `damage_red.png`
   - **Blocked** → белый текст (`#dbdbdb`), фон `damage_block.png`
4. Фон устанавливается через `setStyleSheet` с `background-image` на контейнере строки
5. `setFixedHeight(count * ROW_H)` — виджет автоматически сжимается

Иконки (`damage_green/red/block.png`) отображаются слева через `background-position: left center`.

---

### 4.5 WidgetsWindow

**Файл:** `widgets/widgets_page/widgets_window/widgets_window.h/.cpp`  
**Наследование:** `QWidget`  
**Тип окна:** `WindowStaysOnTopHint | FramelessWindowHint | Tool`  
**Ширина:** фиксированная 185px

Плавающее окно с тайлами сессионной статистики.

#### Поля

```cpp
FlowLayout *flowLayout = nullptr;
QMap<QString, QWidget*> tiles;        // label → tile-виджет
QMap<QString, QLabel*>  valueLabels;  // label → label значения
double m_backgroundOpacity = 0.85;
```

#### Метод `QWidget* createTile(const QString &label, const QString &value)`

Создаёт тайл 80×50px:
- `QLabel *val` — значение (16px, bold, центр)
- `QLabel *lbl` — подпись (10px, тёмно-серый `#555555`)
- Хранит `val` в `valueLabels[label]`

#### Конструктор

Создаёт 8 тайлов через `addTile()`: Бои, Победы, Урон, Опыт, Рейтинг, Мастер, Выжил, Точность.  
Раскладывает через `FlowLayout`. Вычисляет размер через `adjustSize()`.  
Позиция: `x=30`, `y = (screenH - h)/2 + 100`.

#### Метод `void updateTile(const QString &label, const QString &value)`

Устанавливает текст в `valueLabels[label]`.

#### Метод `void updateWins(const float &value)`

Специальный метод для тайла «Победы» с цветовой индикацией:
- `>= 70%` → `#9989e6` (фиолетовый)
- `60–70%` → `#72d1ff` (синий)
- `50–60%` → `#a8e689` (зелёный)
- `< 50%` → `#ffffff` (белый)

#### Toggle-методы (8 штук)

Тонкие обёртки над `setTileVisible(label, visible)`:
`toggleBattles`, `toggleWins`, `toggleDamage`, `toggleXp`, `toggleAlive`, `toggleShots`, `toggleRating`, `toggleMaster`.

#### Метод `void setTileVisible(const QString &label, bool visible)`

Скрывает/показывает тайл. После: `flowLayout->invalidate()` + `adjustSize()` для пересчёта раскладки.

#### Метод `void setOpacity(double opacity)`

Применяет прозрачность ко всем тайлам через `setStyleSheet`:
```css
background-color: rgba(35,35,35,alpha); border-radius: 5px;
```

---

### 4.6 FlowLayout

**Файл:** `widgets/widgets_page/widgets_window/FlowLayout.h/.cpp`  
**Наследование:** `QLayout`

Реализация адаптивной раскладки «слева направо с переносом строки». Стандартный пример из Qt Documentation, адаптированный для проекта.

#### Поля

```cpp
QList<QLayoutItem*> itemList;
int m_hSpace;   // горизонтальный отступ между элементами
int m_vSpace;   // вертикальный отступ между строками
```

#### Ключевые методы

**`int doLayout(const QRect &rect, bool testOnly) const`**

Основной алгоритм раскладки:
1. Вычитает margins из `rect` → `effectiveRect`
2. Итерирует элементы слева направо
3. Если следующий элемент не помещается в строку → переходит на новую строку
4. Если `testOnly == false` → применяет `item->setGeometry(pos, size)`
5. Возвращает итоговую высоту

**`bool hasHeightForWidth() const`** → `true`  
**`int heightForWidth(int width) const`** → `doLayout(QRect(0,0,width,0), true)` — только вычисление высоты без рендера

**`int smartSpacing(QStyle::PixelMetric pm) const`**

Если `m_hSpace/m_vSpace == -1` — использует системный интервал из `QStyle`. Иначе — значение поля.

**`QSize minimumSize() const`**

Максимальный `minimumSize()` среди всех элементов + margins.

---

## 5. MainOverlay

**Файл:** `modules/main_overlay/main_overlay.h/.cpp`  
**Наследование:** `QWidget`

Главное оверлей-окно приложения. Создаётся в `main.cpp` после успешной активации.

#### Флаги окна

```
Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool
WA_TranslucentBackground
```

#### Поля

```cpp
QStackedWidget *stacked_widget = nullptr;
GunMarks    *gunMark   = nullptr;
Widgets     *widgets   = nullptr;
PlayerStats *players   = nullptr;
VehicleStats*vehicle   = nullptr;
InfoPage    *info      = nullptr;
ApiController *m_apiController = nullptr;

QWidget *configPanel;
double m_backgroundOpacity = 1.0;
bool m_dragActive = false;
QPoint m_dragStartPos;
int m_currentIndexPage = 5;    // начальная страница — VehicleStats
int configPanelNormalWidth = 30;
int defaultWidth = 280;
std::vector<QPushButton*> m_navButtons;
```

#### Конструктор (принимает `ApiController*`)

- Устанавливает флаги, атрибуты прозрачности
- Создаёт вертикальный layout: `[configPanel] | [stacked_widget]`
- `configPanel` — анимируемая панель с навигационными кнопками (ширина 0→30px при hover)
- Начальный размер: `defaultWidth × 200`

#### Методы управления страницами

**`void switchPage(int index)`**  
Устанавливает `m_currentIndexPage = index`, вызывает `switchHotPage()`, обновляет стили навигации.

**`void switchHotPage()`**  
Центральный метод переключения. Логика:
- Страница 5 (VehicleStats) → ширина `defaultWidth + 110` (шире для иконок)
- Страница 0 (InfoPage) → высота 400px
- Остальные → `defaultWidth × 200`
- Устанавливает `setMaximumWidth()` для каждой страницы отдельно
- Применяет `stacked_widget->setCurrentIndex()` + `resize(w, h)`

**`void nextHotPage()` / `void prevHotPage()`**  
Декремент/инкремент `m_currentIndexPage` с граничными проверками → `switchHotPage()`.

#### Метод `void showConfigPanel(bool show)`

Анимирует появление/скрытие боковой панели через два параллельных `QPropertyAnimation`:
1. `configPanel::maximumWidth`: `0 → 30` (показать) или `30 → 0` (скрыть)
2. `this::geometry`: расширение/сжатие окна вправо на 30px

Длительность 200мс, `QEasingCurve::InOutQuad`.

#### Методы drag-and-drop

`mousePressEvent` — сохраняет `m_dragStartPos`.  
`mouseMoveEvent` — `move(event->globalPos() - m_dragStartPos)`.  
`mouseReleaseEvent` — сбрасывает `m_dragActive`.

#### Контекстное меню (`contextMenuEvent`)

Меню с пунктами:
- «Прозрачность» — `QSlider` в `QWidgetAction` (10–100%)
- «Выйти» → `MainOverlay::logout()`

#### Метод `void logout()`

1. Если авторизован: шифрует учётные данные XOR+Base64, сохраняет в `auth.json`
2. Вызывает `QApplication::quit()`

#### `void drawFrostPatterns(QPainter &p)`

Декоративный метод рисования фростовых узоров (используется в `paintEvent`).

#### `void updateNavButtonStyles(int activePage)`

Обновляет стиль навигационных кнопок: активная кнопка — непрозрачная, остальные — полупрозрачные.

---

## 6. SubOverlay

**Файл:** `modules/sub_overlay/sub_overlay.h/.cpp`  
**Наследование:** `QWidget`

Компактная кнопка-триггер для скрытия/показа главного оверлея.

#### Флаги окна

```
Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool
WA_TranslucentBackground
```

#### Поля

```cpp
MainOverlay *mainOverlay = nullptr;
bool drag_active = false;
bool is_visible  = true;    // видим ли MainOverlay
QPoint global_pos;          // позиция при начале перетаскивания
```

#### `GlobalHotkeyFilterSub` (локальный класс в .cpp)

Реализует `QAbstractNativeEventFilter`.  
Перехватывает `WM_HOTKEY` с `wParam == 1` (зарегистрирован как `Ctrl + =`).  
Вызывает `SubOverlay::mainHide()`.

#### Конструктор

- Размер 45×45px
- Позиция: `x=0`, `y = (screenH - 40) / 2` (вертикальный центр)
- Иконка `mini_stats_icon.png` в `QLabel` с рамкой и тёмным фоном
- `RegisterHotKey(NULL, 1, MOD_CONTROL, 0xBB)` — регистрирует `Ctrl + =` (0xBB = VK_OEM_PLUS)

#### Метод `void setMainWidget(MainOverlay *mainOverlay)`

Сохраняет указатель на главный оверлей.

#### Метод `void mainHide()`

Переключает видимость `MainOverlay`:
- Если `isMainVisible()` → `mainOverlay->hide()`, `setMainVisible(false)`
- Иначе → `mainOverlay->show()`, `setMainVisible(true)`

#### Mouse Events

**`mousePressEvent`:**
- ПКМ → начинает перетаскивание (`setDragActive(true)`)
- ЛКМ → вызывает `mainHide()`

**`mouseMoveEvent`:**
- При `drag_active`: вычисляет delta от `global_pos`, перемещает окно

**`mouseReleaseEvent`:**
- ПКМ → `setDragActive(false)`, вызывает `snapToClosestScreenEdge()`

#### Метод `void snapToClosestScreenEdge()`

Вычисляет расстояния до всех 4 краёв экрана. Прилипает к ближайшему:
- Левый: `new_x = screen.left()`
- Правый: `new_x = screen.right() - width()`
- Верхний: `new_y = screen.top()`
- Нижний: `new_y = screen.bottom() - height()`

---

## 7. ActivationWindow

**Файл:** `modules/main_overlay/activate/activation_window.h`  
**Наследование:** `QDialog`

Диалоговое окно активации программы. Создаётся в `main.cpp` если `HWIDActivator::checkActivation()` возвращает `false`.

#### Поля

```cpp
QLabel *hwidLabel;    // отображает HWID пользователя
QLineEdit *keyInput;  // поле ввода ключа активации
```

#### Конструктор

- Размер 600×250px, `WindowStaysOnTopHint | WindowCloseButtonHint`
- Отображает HWID через `HWIDActivator::getHWID()`
- Кнопка «Копировать ID» → `copyHWID()`
- Поле ввода ключа
- Кнопка «Активировать» → `tryActivate()`

#### Слот `void copyHWID()`

`QApplication::clipboard()->setText(HWIDActivator::getHWID())`.  
Временно меняет текст кнопки на «Скопировано!» через `QTimer::singleShot(2000, ...)`.

#### Слот `void tryActivate()`

1. Читает `keyInput->text().trimmed()`
2. Если пусто — подсвечивает поле красным
3. Если `HWIDActivator::activate(key)` вернул `true`:
   - Проверяет дату: если `QDate::currentDate() > QDate(2026, 3, 31)` → выводит «Версия устарела»
   - Иначе → `accept()` (диалог закрывается с результатом `Accepted`)
4. Если `false` → подсвечивает красным, «Неверный ключ активации»

#### `void closeEvent(QCloseEvent*)`

Вызывает `reject()` — при закрытии крестиком программа завершается (обрабатывается в `main.cpp`).

---

*Документ сгенерирован на основе исходного кода ветки `pre-2-3`*  
*Проект: StatsOverlay | Автор: А. С. Коротков | НИУ ВШЭ, 2026*

## Парсер реплеёв

`ReplayTracker` + `ReplayParser` — двухуровневый парсер файлов `.wotreplay`.

### Формат файла

```
.wotreplay (бинарный файл)
  │
  ├─ Заголовок: magic bytes (0x12345678), хэш, версия
  │
  └─ Пакеты:
       ├─ type=8, length>1000 → EntityMethod пакет
       │     └─ subtype=55 → UpdateArena (список игроков)
       │           └─ Protobuf → [PlayerRaw: nickname, team, tank_id, account_id, clan, platoon]
       │
       └─ Другие пакеты (урон, движение и т.д.)
             └─ Парсинг через zlib-декомпрессию и Protobuf varint
```

`ReplayTracker` отслеживает файл по размеру: при каждом тике таймера проверяет, вырос ли файл, и парсит только новую часть.

---

## Активация

Программа использует **HWID-активацию** — привязку к железу машины.

### Алгоритм

```
HWID = SHA-256( machine_unique_id + kernel_version + cpu_arch )
Key  = SHA-256( "bambam" + ":" + HWID ).hex[:16].upper()
```

### Файлы

| Файл | Содержимое |
|------|------------|
| `%AppData%\StatsOverlay\activation.json` | `{ hwid, key, activated: true }` |
| `%AppData%\StatsOverlay\auth.json` | Зашифрованные XOR+Base64 токен, никнейм, account_id |

### Генерация ключа (для администратора)

```bash
python genkey.py
# Введи HWID пользователя → получишь ключ активации
```

---

## Авторизация через Lesta API

```
1. ApiController::login()
   └─ GET api.tanki.su/wot/auth/login/?application_id=...&redirect_uri=http://localhost:5000/&nofollow=1
   └─ Получает location URL → открывает в браузере (ShellExecuteA)

2. Встроенный HTTP-сервер (cpp-httplib) слушает localhost:5000
   └─ Браузер редиректит с параметрами: access_token, account_id, nickname, status

3. Токен сохраняется в памяти ApiController
   └─ При выходе → шифруется XOR и сохраняется в auth.json
   └─ При следующем запуске → восстанавливается и продлевается через /auth/prolongate/
```

### Используемые эндпоинты

| Метод | URL | Назначение |
|-------|-----|------------|
| GET | `/wot/auth/login/` | Получение URL авторизации |
| POST | `/wot/auth/logout/` | Выход |
| POST | `/wot/auth/prolongate/` | Продление токена |
| GET | `/wot/account/info/` | Основная статистика |
| GET | `/wot/account/tanks/` | Статистика по технике |
| GET | `/wot/tanks/stats/` | Детальная статистика танков |
| GET | `/wot/account/achievements/` | Мастерство |

---

## Сессионная статистика

Все классы `*StatsData` работают по единому принципу:

```
initialStats(data)   → сохраняет firstData (снимок на старте сессии)
updateStats(data)    → currentData = newData - firstData
getXxx()             → возвращает currentData.xxx (разница с началом сессии)
```

Сброс сессии — вызов `reset()` в `ApiController`, который выставляет флаги `isFirst*Stats = true`: при следующем опросе API новые данные станут базовой точкой.

---

## Стримерский режим

`SubOverlay` — отдельное прозрачное окно (`Qt::FramelessWindowHint + Qt::WindowStaysOnTopHint`), запускается параллельно с основным. Предназначено для вывода компактной статистики в кадр стрима. Поддерживает перетаскивание.

---

## Внешние зависимости

| Библиотека | Версия | Назначение |
|------------|--------|------------|
| [Qt](https://www.qt.io/) | 6.9.2 | GUI, сеть, ресурсы |
| [libcurl](https://curl.se/libcurl/) | x64 Windows | HTTPS-запросы к Lesta API |
| [cpp-httplib](https://github.com/yhirose/cpp-httplib) | header-only | OAuth2 callback сервер |
| [nlohmann/json](https://github.com/nlohmann/json) | header-only | Парсинг JSON-ответов API |
| [OpenSSL](https://www.openssl.org/) | bundled с curl | TLS для HTTPS |
| zlib | bundled | Декомпрессия Protobuf в реплеях |

Все зависимости поставляются в репозитории в `external/` — дополнительная установка не требуется.

---

## Конфигурация и файлы данных

### Автоматически создаваемые файлы

| Путь | Описание |
|------|----------|
| `%AppData%\StatsOverlay\activation.json` | Статус активации и HWID |
| `%AppData%\StatsOverlay\auth.json` | Сессионные данные Lesta (XOR+Base64) |
| `%Documents%\StatsOverlay\logs\YYYY-MM-DD.log` | Лог работы приложения |

### База данных техники

При запуске загружается с GitHub:
```
https://raw.githubusercontent.com/MaJluHoBKa/vehicles_db_lesta/main/tanks.json
```
При недоступности используется резервная копия из `resources/vehicles/tanks.json`.

### Формат tanks.json

```json
{
  "tanks": [
    { "id": 1, "name": "T-34", "tier": 5, "type": "mt", "nation": "ussr", "status": "default" }
  ]
}
```

Типы: `lt`, `mt`, `ht`, `at-spg`, `spg`. Статусы: `default`, `premium`, `collector`.

---

## Известные ограничения

- Работает только с российским клиентом Tanks Blitz (Lesta Games, `api.tanki.su`). Wargaming EU/NA не поддерживается.
- Парсер реплеёв привязан к текущему формату клиента. После крупных обновлений может потребоваться обновление парсера.
- Активация привязана к железу — при смене материнской платы потребуется новый ключ.

---

## Лицензия

Проект является учебной выпускной квалификационной работой.  
Используя приложение, вы соглашаетесь с лицензионным соглашением.

Используется API Леста Игры:
- [Условия использования API](https://developers.lesta.ru/documentation/rules/agreement/)
- [Политика конфиденциальности](https://legal.lesta.ru/privacy-policy/)

**Благодарности:** участникам группы Blitz Hata VЫP, Леста Игры за API, сообществу Tanks Blitz за тестирование.

**Телеграмм-канал автора:** [Telegram](https://t.me/tanksblitz_pmods)

**Запрещено:** коммерческое использование, модификация и декомпиляция кода, распространение без указания автора, использование в нарушение правил Леста Игры.

---

*Автор: А. С. Коротков | НИУ ВШЭ, Факультет компьютерных наук, 2026*
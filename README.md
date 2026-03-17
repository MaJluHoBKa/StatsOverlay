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

## Модуль controller

Директория: `modules/main_overlay/controller/`

Содержит всю бизнес-логику приложения — без единого UI-элемента. Здесь происходит взаимодействие с Lesta API, парсинг реплеёв, хранение статистики и управление активацией.

---

### ApiController

Файл: `controller/ApiController.h`

Центральный класс всего приложения. Создаётся единожды в `main.cpp` и передаётся указателем во все виджеты. Отвечает за:

**Авторизацию:**
- `login()` — OAuth2 через браузер. Открывает страницу входа Lesta, поднимает встроенный HTTP-сервер на `localhost:5000` (cpp-httplib), принимает токен из редиректа.
- `logout()` — POST-запрос на `/wot/auth/logout/`, очищает токен.
- `prolongate()` — продление токена через `/wot/auth/prolongate/`, вызывается при старте если токен уже сохранён.

**Получение статистики** (все методы используют libcurl + nlohmann/json):
- `get_main_stats()` — `/wot/account/info/`: кредиты, золото, опыт, бои, победы, урон.
- `get_rating_stats()` — `/wot/account/info/` с параметром `extra=statistics.rating`: рейтинг MM, калибровочные бои.
- `get_other_stats()` — та же ручка, другие поля: попадания, выстрелы, выживания, фраги, время жизни.
- `get_mastery_stats()` — `/wot/account/achievements/`: значки мастерства 1–4 уровня.
- `get_vehicles_stats()` — `/wot/account/tanks/` + `/wot/tanks/stats/`: статистика по каждому танку в гараже.

**Работе с игроками боя:**
- `setPlayers(allies, enemies)` — сохраняет списки союзников и противников после парсинга реплея.
- `sortPlayers()` — двухуровневая сортировка: сначала по уровню танка (убывание), затем по названию (кастомный компаратор с поддержкой кириллицы и цифр).
- `join()` — формирует строку ID игроков через запятую для батч-запроса к API.

**Логированию:**
- `initLog()` / `log(message, page, timestamp)` — пишет в `%Documents%/StatsOverlay/logs/YYYY-MM-DD.log` с префиксом страницы (`[AUTH]`, `[MAIN PAGE]`, `[RATING PAGE]` и т.д.).

**Сохранению сессии:**
- Токен, никнейм и account_id шифруются XOR с ключом и кодируются в Base64 перед записью в `auth.json`.

---

### ReplayTracker

Файлы: `controller/ReplayTracker.h/.cpp`

Главный трекер боя в реальном времени. Наследует `QObject`, работает в связке с `QTimer` — периодически перечитывает файл реплея по мере его роста.

**Принцип работы:**

```
QTimer (каждые ~500мс)
  └─► ReplayTracker::update(replayPath, selfNick)
        ├─ если fileSize не изменился → пропустить
        ├─ читает только новые байты (от lastFileSize до конца)
        └─ parseFile() → parseEvents() + buildEntityMapFromArena()
```

**Что отслеживает:**

| Событие | Структура | Поля |
|---------|-----------|------|
| Нанесённый урон | `DamageEvent` | `time_s`, `shooter`, `target`, `damage`, `hp_after`, `hit_result`, `event_type`, `total_damage`, `vehicle_cd` |
| Входящий урон | `IncomingEvent` | `time_s`, `attacker`, `damage`, `hp_after`, `event_type`, `total_received`, `vehicle_cd` |
| Заблокированный урон | `BlockedEvent` | `time_s`, `attacker`, `damage_blocked`, `total_blocked` |
| Урон ассиста | `AssistEvent` | `time_s`, `ally`, `target`, `damage`, `total_assist` |

Все события агрегируются в `BattleEvent` — универсальную структуру с типом (`Damage` / `Incoming` / `Blocked` / `Assist`) для передачи в виджет истории.

**Итоговая статистика боя** (`BattleStats`):

```cpp
struct BattleStats {
    int dmg_dealt;    // нанесённый урон
    int shots;        // выстрелов
    int hits;         // попаданий
    int pens;         // пробитий
    int dmg_blocked;  // заблокировано
    int dmg_received; // получено
    int spot_assist;  // урон с подсвета
    int track_assist; // урон с гусени
    bool valid;
};
```

**Построение карты игроков:**
- `buildEntityMapFromArena()` — разбирает Protobuf-пакет `UpdateArena`, строит `QMap<int, QString> entityToNick` (entity ID → никнейм), `entityToTeam`, `entityToVehicle`.
- `extractEntityMapFromResult()` — альтернативный путь через `battle_results` в конце реплея, более надёжный.

---

### ParseReplay

Файл: `controller/ParseReplay.h`

Низкоуровневый парсер формата `.wotreplay`. Используется для быстрого извлечения **списка игроков** из реплея (без полного разбора событий боя).

**Формат файла:**

```
[4 байта]  magic = 0x12345678
[8 байт]   мусор
[1 байт]   длина хэша
[N байт]   хэш
[1 байт]   длина версии
[M байт]   версия + 1 байт
[пакеты...]
```

**Алгоритм поиска игроков:**

```
for каждый пакет:
    if type == 8 and length > 1000:
        payload → parseEntityMethod()
            if subtype == 55:  // UpdateArena
                payload → parseUpdateArenaManual()
                    поле 1 (repeated) → блоки игроков
                        поле 1 → parsePlayerManual()
                            field 2 → tank_id
                            field 3 → nickname
                            field 4 → team_number
                            field 7 → account_id
                            field 8 → clan_tag
                            field 11 → platoon_number
```

Protobuf парсится вручную через `decodeVarint()` — без сторонних библиотек.

**Структура игрока:**

```cpp
struct PlayerRaw {
    std::string nickname;
    int team_number;   // 1 = союзники, 2 = противники
    int tank_id;
    uint64_t account_id;
    std::string clan_tag;
    int platoon_number;
};
```

---

### HWID

Файл: `controller/HWID.h`

Утилита аппаратной активации. Вся логика — статические методы, файл хранения — `%AppData%/StatsOverlay/activation.json`.

| Метод | Описание |
|-------|----------|
| `getHWID()` | Собирает строку из `machineUniqueId + kernelVersion + cpuArch`, хэширует SHA-256 через Qt |
| `generateKey()` | `SHA-256("bambam:" + HWID).hex.left(16).toUpper()` |
| `activate(key)` | Проверяет ключ, записывает `activation.json` при совпадении |
| `checkActivation()` | Читает `activation.json`, сверяет HWID и ключ с текущей машиной |

---

### Классы данных (`data/`)

Все пять классов данных следуют одному паттерну — **разностная статистика**:

```
initialStats(snapshot)  →  firstData = snapshot
updateStats(snapshot)   →  currentData = snapshot - firstData
getXxx()                →  currentData.xxx
```

#### MainStatsData — основная статистика

```cpp
struct StatsData {
    int64_t credits, gold, exp_battle, exp_free;
    int64_t battles, wins, losses, totalDamage;
};
```

Вычисляемые методы: `getAvgDamage()`, `getAvgExp()`, `getPercentWins()` — возвращают `-1` если `battles == 0`.

#### RatingStatsData — рейтинговая статистика

Особенность: формула пересчёта рейтинга MM:
```
mm_rating = 3000.0 + raw_rating * 10.0
```
`getDiffRating()` — изменение рейтинга от точки старта сессии.  
`getCalibBattles()` = `10 - calib_battles` (оставшиеся калибровочные бои).

#### MasteryStatsData — мастерство экипажа

```cpp
struct MasteryData {
    int64_t mastery;    // «Туз» (4-й знак)
    int64_t mastery_1;  // 1-й знак
    int64_t mastery_2;  // 2-й знак
    int64_t mastery_3;  // 3-й знак
};
```

Предоставляет `getFirstData()` и `getCurrentData()` — виджет отображает абсолютные и сессионные значения одновременно.

#### OtherStatsData — расширенная статистика

| Метод | Формула |
|-------|---------|
| `getPercentHits()` | `hits / shots * 100` |
| `getPercentSurvived()` | `survived / battles * 100` |
| `getLifeTime()` | `lifeTime / battles` (среднее в секундах) |
| `getDamageK()` | `totalDamage / receiverDamage` |
| `getFragsK()` | `frags / (battles - survived)` |
| `getDeaths()` | `battles - survived` |

#### VehicleStatsData — статистика по технике

Хранит три словаря: `first_vehicles` (снимок на старте), `vehicles` (сессионная разница), `prev_vehicles` (предыдущее состояние).

**Определение последнего сыгранного танка:**
```cpp
// getUpdatedVehicle() — танк у которого увеличилось число боёв
for (v : vehicles):
    if v.battles > prev_vehicles[v.id].battles:
        return &v
```

База данных техники загружается с GitHub при старте, при недоступности — из резервной копии в Qt ресурсах.

---

## Модуль widgets

Директория: `modules/main_overlay/widgets/`

Каждый виджет — самостоятельный `QWidget`, добавляемый в `QStackedWidget` главного окна. Переключение через навигационную боковую панель (`MainOverlay::switchPage(int index)`). Все виджеты получают `ApiController*` через конструктор и обновляются по `QTimer`.

---

### main_stats — Основная статистика

**Индекс страницы:** 7

Отображает разность между текущей статистикой и статистикой на момент первого запроса:

| Метрика | Метод |
|---------|-------|
| Кредиты | `MainStatsData::getCredits()` |
| Золото | `MainStatsData::getGold()` |
| Опыт за бои | `MainStatsData::getExpBattle()` |
| Свободный опыт | `MainStatsData::getExpFree()` |
| Бои | `MainStatsData::getBattles()` |
| Победы / Поражения | `getWins()` / `getLosses()` |
| Средний урон | `MainStatsData::getAvgDamage()` |
| Средний опыт | `MainStatsData::getAvgExp()` |
| % побед | `MainStatsData::getPercentWins()` |

---

### rating_stats — Рейтинговая статистика

**Индекс страницы:** 6

| Метрика | Метод |
|---------|-------|
| Рейтинг MM | `RatingStatsData::getRating()` |
| Изменение рейтинга (Δ) | `RatingStatsData::getDiffRating()` |
| Осталось калибровочных | `RatingStatsData::getCalibBattles()` |
| Бои / победы / урон | соответствующие методы |

---

### other_stats — Расширенная статистика

**Индекс страницы:** 4

| Метрика | Метод |
|---------|-------|
| Точность | `OtherStatsData::getPercentHits()` |
| Выживаемость | `OtherStatsData::getPercentSurvived()` |
| Среднее время жизни | `OtherStatsData::getLifeTime()` |
| Коэффициент урон/получен | `OtherStatsData::getDamageK()` |
| Коэффициент фраги/смерти | `OtherStatsData::getFragsK()` |

---

### vehicles_stats — Статистика по технике

**Индекс страницы:** 5 | **Ширина окна:** 390px

Автоматически определяет последний сыгранный танк через `VehicleStatsData::getUpdatedVehicle()` и отображает:
- Иконка нации (флаги 12 наций)
- Иконка типа техники (ЛТ/СТ/ТТ/ПТ в вариантах `default`/`premium`/`collector`)
- Название танка, уровень (I–X)
- Бои / победы / средний урон на этом танке за сессию

---

### player_stats — Статистика игроков

**Индекс страницы:** 2

Получает список игроков из `ApiController::getSortedAllies()` / `getSortedEnemies()` — данные туда попадают после того как `ParseReplay` разобрал реплей текущего боя.

Для каждого игрока: никнейм + клан, название и уровень танка, бои/% побед/средний урон из API.

**Сортировка (`ApiController::sortPlayers()`):**
1. `stable_sort` по имени танка (кастомный компаратор: цифры < латиница < кириллица)
2. `stable_sort` по уровню (убывание), сохраняя порядок по имени внутри уровня

**Подокно разбивки** (`split_players/players_window`): союзники и противники в двух колонках.

---

### gun_marks — Отметки на стволе

**Индекс страницы:** 1

Отслеживание прогресса получения отметок на стволе выбранного танка:
- Текущий процент (например `94.37%`) и прогресс-бар
- Изменение за последний бой (`+0.12%` / `-0.05%`)
- Суммарный урон по текущей сессии отметок
- Требуемый средний урон для 1, 2, 3-й отметки

**Подокно** (`gunmarks_window/`): история боёв, настройки (выбор танка, количество боёв для расчёта).

---

### info_page — Информационная страница

**Индекс страницы:** 0 | **Высота:** 400px

- Кнопка авторизации / выхода (`ApiController::login()` / `MainOverlay::logout()`)
- Кнопка сброса сессионной статистики (`ApiController::reset()`)
- Информация об аккаунте после авторизации
- Ссылки и информация о версии

---

### widgets_page — Боевые виджеты

**Индекс страницы:** 3

Контейнерная страница. Управляет `ReplayTracker` и распределяет события по дочерним виджетам.

#### dmg_window — Виджет урона

Отображает нанесённый урон в реальном времени с анимацией каждого события. Показывает суммарный урон и количество пробитий за бой.

#### history_window — История событий

Прокручиваемый список всех событий боя из `ReplayTracker::battleHistory()`:

| Тип | Цвет | Содержимое |
|-----|------|------------|
| `Damage` | зелёный | Цель, урон, HP после, тип попадания |
| `Incoming` | красный | Атакующий, урон, HP после |
| `Blocked` | синий | Атакующий, заблокированный урон |
| `Assist` | жёлтый | Союзник, цель, урон ассиста |

Каждая строка содержит игровое время боя (`time_s`).

#### widgets_window — Управление виджетами

Диалог выбора активных виджетов с `FlowLayout` — кастомной реализацией `QLayout`, которая расставляет дочерние виджеты слева направо с автоматическим переносом строки при нехватке места.

---

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
#include <main_overlay/widgets/player_stats/player_stats.h>

class GlobalHotkeyFilterPlayers : public QAbstractNativeEventFilter
{
public:
    QWidget *overlay;

    GlobalHotkeyFilterPlayers(QWidget *overlay) : overlay(overlay) {}

    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override
    {
        MSG *msg = static_cast<MSG *>(message);
        if (msg->message == WM_HOTKEY)
        {
            PlayerStats *sub = qobject_cast<PlayerStats *>(overlay);
            if (!sub)
                return false;

            if (msg->wParam == 4) // Ctrl + UP
            {
                sub->hotHide();
            }
        }
        return false;
    }
};

PlayerStats::PlayerStats(ApiController *apiController, QWidget *parent)
    : m_apiController(apiController), QWidget(parent)
{
    setMaximumWidth(260);
    setContentsMargins(0, 2, 0, 2);
    setStyleSheet(
        "border-top-right-radius: 10px;"
        "border-bottom-right-radius: 10px;");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Главный слой для виджета
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(8, 6, 8, 6);

    // Слой заголовка статистики по игрокам
    QHBoxLayout *listsStats = new QHBoxLayout();
    listsStats->setAlignment(Qt::AlignTop);
    listsStats->setSpacing(5);

    QFrame *accent = new QFrame;
    accent->setFixedWidth(3);
    accent->setStyleSheet("background-color: #c8a84b; border-radius: 1px;");
    listsStats->addWidget(accent);

    QLabel *label = new QLabel;
    label->setText("Статистика игроков");
    label->setTextFormat(Qt::RichText);
    label->setStyleSheet(
        "font-family: Segoe UI;"
        "font-size: 14px;"
        "font-weight: bold;"
        "color: #e2ded3;"
        "white-space: nowrap;");
    label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    listsStats->addWidget(label);

    QPushButton *switchXVM = new QPushButton;
    switchXVM->setText("Выкл");
    switchXVM->setFixedSize(50, 25);
    switchXVM->setStyleSheet(
        "QPushButton {"
        "        background-color: rgb(50, 50, 50);"
        "        color: #e2ded3;"
        "        font-size: 13px;"
        "        font-family: Consolas;"
        "        font-weight: bold;"
        "        padding: 5px;"
        "        border-radius: 5px;"
        "    }"
        "    QPushButton:hover {"
        "        background-color: rgb(60, 60, 60);"
        "        color: #ffffff;"
        "    }"
        "    QPushButton:pressed {"
        "        background-color: rgb(90, 90, 90);"
        "        color: #cccccc;"
        "    }");
    connect(switchXVM, &QPushButton::clicked, this, [this]()
            { setIsOn(); });
    setButtonActive(switchXVM);
    listsStats->addWidget(switchXVM);

    mainLayout->addLayout(listsStats);

    QFrame *separator = new QFrame;
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Plain);
    separator->setStyleSheet("background-color: rgba(226, 222, 211, 0.15);");
    separator->setFixedHeight(1);
    mainLayout->addWidget(separator);

    auto createButtonCheckbox = [](const QString &text, QLabel *&checkPtr)
    {
        QPushButton *btn = new QPushButton;
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        btn->setStyleSheet(R"(
        QPushButton {
            border: none;
            border-radius: 5px;
            padding: 5px 10px;
            color: #e2ded3;
            font-family: Segoe UI;
            font-weight: bold;
            font-size: 13px;
            text-align: right;
        }
        QPushButton:hover {
            background-color: rgb(50, 50, 50);
        }
    )");

        // Контейнер внутри кнопки для галочки и текста
        QWidget *container = new QWidget(btn);
        container->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        QHBoxLayout *layout = new QHBoxLayout(container);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(5);

        // Галочка справа
        QLabel *check = new QLabel("✔", container);
        check->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        check->setStyleSheet("color: #e2ded3; font-weight: bold; font-size: 13px; padding-right: 5px");
        checkPtr = check;

        // Текст слева
        QLabel *label = new QLabel(text, container);
        label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        label->setStyleSheet("color: #e2ded3; font-weight: bold; font-size: 13px; padding-left: 5px");

        layout->addWidget(label);
        layout->addWidget(check);
        container->setLayout(layout);

        QVBoxLayout *btnLayout = new QVBoxLayout(btn);
        btnLayout->setContentsMargins(0, 0, 0, 0);
        btnLayout->addWidget(container);

        return btn;
    };

    playersBox = createButtonCheckbox("Игроки", playersCheck);
    tanksBox = createButtonCheckbox("Танки", tanksCheck);
    battlesBox = createButtonCheckbox("Бои", battlesCheck);
    winsBox = createButtonCheckbox("Победы", winsCheck);
    damageBox = createButtonCheckbox("Урон", damageCheck);

    connect(playersBox, &QPushButton::clicked, this, &PlayerStats::togglePlayers);
    connect(tanksBox, &QPushButton::clicked, this, &PlayerStats::toggleTanks);
    connect(battlesBox, &QPushButton::clicked, this, &PlayerStats::toggleBattles);
    connect(winsBox, &QPushButton::clicked, this, &PlayerStats::toggleWins);
    connect(damageBox, &QPushButton::clicked, this, &PlayerStats::toggleDamage);

    mainLayout->addWidget(playersBox);
    mainLayout->addWidget(tanksBox);
    mainLayout->addWidget(battlesBox);
    mainLayout->addWidget(winsBox);
    mainLayout->addWidget(damageBox);

    auto createOpacitySlider = [this]() -> QWidget *
    {
        QWidget *container = new QWidget;
        container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        QHBoxLayout *layout = new QHBoxLayout(container);
        layout->setContentsMargins(5, 0, 5, 0);
        layout->setSpacing(5);

        // Метка слева
        QLabel *label = new QLabel("Прозрачность", container);
        label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        label->setStyleSheet("color: #e2ded3; font-weight: bold; font-size: 13px; padding-right: 5px;");
        layout->addWidget(label);

        // Слайдер справа
        QSlider *slider = new QSlider(Qt::Horizontal, container);
        slider->setRange(10, 100);
        slider->setValue(int(m_backgroundOpacity * 100));
        slider->setStyleSheet(R"(
            QSlider::groove:horizontal {
                border: 1px solid #444;
                height: 4px;
                background: #333;
                border-radius: 2px;
            }
            QSlider::handle:horizontal {
                background: #ddd;
                border: 1px solid #777;
                width: 10px;
                height: 10px;
                margin: -3px 0;
                border-radius: 5px;
            }
        )");
        layout->addWidget(slider);

        // Событие изменения прозрачности
        connect(slider, &QSlider::valueChanged, this, [this](int value)
                {
        m_backgroundOpacity = value / 100.0;

        this->alliesWindow->setOpacity(m_backgroundOpacity);
        this->enemiesWindow->setOpacity(m_backgroundOpacity);
        this->alliesWindow->update();
        this->enemiesWindow->update();

        QSettings settings("MyCompany", "StatsOverlay");
        settings.setValue("backgroundOpacityPlayers", m_backgroundOpacity); });

        return container;
    };
    mainLayout->addWidget(createOpacitySlider());

    setLayout(mainLayout);

    RegisterHotKey(NULL, 4, MOD_CONTROL, VK_OEM_MINUS);
    auto *hotkeyFilter = new GlobalHotkeyFilterPlayers(this);
    qApp->installNativeEventFilter(hotkeyFilter);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &PlayerStats::updatingPlayerStats);
    timer->start(10000);
}

void PlayerStats::togglePlayers()
{
    QMetaObject::invokeMethod(
        this,
        [this]()
        {
            if (is_players)
            {
                playersCheck->setText("");
                this->alliesWindow->togglePlayers(false);
                this->enemiesWindow->togglePlayers(false);
            }
            else
            {
                playersCheck->setText("✔");
                this->alliesWindow->togglePlayers(true);
                this->enemiesWindow->togglePlayers(true);
            }
            is_players = !is_players;
        },
        Qt::QueuedConnection);
}

void PlayerStats::toggleTanks()
{
    QMetaObject::invokeMethod(
        this,
        [this]()
        {
            if (is_tanks)
            {
                tanksCheck->setText("");
                this->alliesWindow->toggleTanks(false);
                this->enemiesWindow->toggleTanks(false);
            }
            else
            {
                tanksCheck->setText("✔");
                this->alliesWindow->toggleTanks(true);
                this->enemiesWindow->toggleTanks(true);
            }
            is_tanks = !is_tanks;
        },
        Qt::QueuedConnection);
}

void PlayerStats::toggleBattles()
{
    QMetaObject::invokeMethod(
        this,
        [this]()
        {
            if (is_battles)
            {
                battlesCheck->setText("");
                this->alliesWindow->toggleBattles(false);
                this->enemiesWindow->toggleBattles(false);
            }
            else
            {
                battlesCheck->setText("✔");
                this->alliesWindow->toggleBattles(true);
                this->enemiesWindow->toggleBattles(true);
            }
            is_battles = !is_battles;
        },
        Qt::QueuedConnection);
}

void PlayerStats::toggleWins()
{
    QMetaObject::invokeMethod(
        this,
        [this]()
        {
            if (is_wins)
            {
                winsCheck->setText("");
                this->alliesWindow->toggleWins(false);
                this->enemiesWindow->toggleWins(false);
            }
            else
            {
                winsCheck->setText("✔");
                this->alliesWindow->toggleWins(true);
                this->enemiesWindow->toggleWins(true);
            }
            is_wins = !is_wins;
        },
        Qt::QueuedConnection);
}

void PlayerStats::toggleDamage()
{
    QMetaObject::invokeMethod(
        this,
        [this]()
        {
            if (is_damage)
            {
                damageCheck->setText("");
                this->alliesWindow->toggleDamage(false);
                this->enemiesWindow->toggleDamage(false);
            }
            else
            {
                damageCheck->setText("✔");
                this->alliesWindow->toggleDamage(true);
                this->enemiesWindow->toggleDamage(true);
            }
            is_damage = !is_damage;
        },
        Qt::QueuedConnection);
}

void PlayerStats::updatingPlayerStats()
{
    QMetaObject::invokeMethod(
        this,
        [this]()
        {
            if (!this->isOn)
            {
                alliesWindow->clearData();
                enemiesWindow->clearData();
                isPlayers = false;
                return;
            }

            if (!isAuth())
                return;

            if (busy)
                return;

            busy = true;

            QtConcurrent::run([this]()
                              {
                    if (this->m_apiController->loadReplayPlayers())
                    {
                        if(this->isPlayers)
                        {
                            busy = false;
                            return;
                        }
                        if (this->m_apiController->get_players_stats())
                        {
                            // результат надо передать в GUI-поток
                            QMetaObject::invokeMethod(this, [this]()
                            {
                                this->allies = this->m_apiController->getSortedAllies();
                                this->enemies = this->m_apiController->getSortedEnemies();
                                alliesWindow->setData(this->allies);
                                enemiesWindow->setData(this->enemies);
                                this->isPlayers = true;
                                busy = false;
                            }, Qt::QueuedConnection);
                        }
                    }
                    else
                    {
                        QMetaObject::invokeMethod(this, [this]()
                        {
                            alliesWindow->clearData();
                            enemiesWindow->clearData();
                            isPlayers = false;
                            busy = false;
                        }, Qt::QueuedConnection);
                    } });
        },
        Qt::QueuedConnection);
}
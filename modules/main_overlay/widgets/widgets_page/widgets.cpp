#include <main_overlay/widgets/widgets_page/widgets.h>

Widgets::Widgets(ApiController *apiController, QWidget *parent)
    : m_apiController(apiController), QWidget(parent)
{
    setMaximumWidth(260);
    setContentsMargins(0, 2, 0, 2);
    setStyleSheet(
        "border-top-right-radius: 10px;"
        "border-bottom-right-radius: 10px;");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    widgetsWindow = new WidgetsWindow();
    widgetsWindow->hide();

    dmgWidget = new DamageWidget();
    dmgWidget->hide();

    hWidget = new HistoryWidget(m_apiController);
    hWidget->hide();

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
    label->setText("Виджеты статистики");
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

    QGridLayout *checkGrid = new QGridLayout;
    checkGrid->setSpacing(2);
    checkGrid->setContentsMargins(0, 0, 0, 0);

    battlesBox = createButtonCheckbox("Бои", battlesCheck);
    winsBox = createButtonCheckbox("Победы", winsCheck);
    damageBox = createButtonCheckbox("Урон", damageCheck);
    xpBox = createButtonCheckbox("Опыт", xpCheck);
    aliveBox = createButtonCheckbox("Выжил", aliveCheck);
    shotsBox = createButtonCheckbox("Точность", shotsCheck);
    ratingBox = createButtonCheckbox("Рейтинг", ratingCheck);
    masterBox = createButtonCheckbox("Мастер", masterCheck);

    connect(battlesBox, &QPushButton::clicked, this, &Widgets::toggleBattles);
    connect(winsBox, &QPushButton::clicked, this, &Widgets::toggleWins);
    connect(damageBox, &QPushButton::clicked, this, &Widgets::toggleDamage);
    connect(xpBox, &QPushButton::clicked, this, &Widgets::toggleXp);
    connect(aliveBox, &QPushButton::clicked, this, &Widgets::toggleAlive);
    connect(shotsBox, &QPushButton::clicked, this, &Widgets::toggleShots);
    connect(ratingBox, &QPushButton::clicked, this, &Widgets::toggleRating);
    connect(masterBox, &QPushButton::clicked, this, &Widgets::toggleMaster);

    checkGrid->addWidget(battlesBox, 0, 0);
    checkGrid->addWidget(winsBox, 1, 0);
    checkGrid->addWidget(damageBox, 2, 0);
    checkGrid->addWidget(xpBox, 3, 0);
    checkGrid->addWidget(ratingBox, 0, 1);
    checkGrid->addWidget(masterBox, 1, 1);
    checkGrid->addWidget(aliveBox, 2, 1);
    checkGrid->addWidget(shotsBox, 3, 1);

    for (int i = 0; i < checkGrid->rowCount(); ++i)
        checkGrid->setRowStretch(i, 1);
    for (int i = 0; i < checkGrid->columnCount(); ++i)
        checkGrid->setColumnStretch(i, 1);

    mainLayout->addLayout(checkGrid, 1);

    QPushButton *switchWidgets = new QPushButton;
    switchWidgets->setText("Включить");
    switchWidgets->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    switchWidgets->setStyleSheet(
        "QPushButton {"
        "        background-color: rgb(50, 50, 50);"
        "        color: #e2ded3;"
        "        font-size: 14px;"
        "        font-family: Segoe UI;"
        "        font-weight: bold;"
        "        padding: 5px;"
        "        border-radius: 5px;"
        "    }"
        "    QPushButton:hover {"
        "        background-color: #4cd964;"
        "        color: #383838;"
        "    }"
        "    QPushButton:pressed {"
        "        background-color: #3cbf5f;"
        "        color: #383838;"
        "    }");
    connect(switchWidgets, &QPushButton::clicked, this, [this]()
            { setIsOn(); });
    setButtonActive(switchWidgets);
    mainLayout->addWidget(switchWidgets);

    QFrame *separator2 = new QFrame;
    separator2->setFrameShape(QFrame::HLine);
    separator2->setFrameShadow(QFrame::Plain);
    separator2->setStyleSheet("background-color: rgba(226, 222, 211, 0.15);");
    separator2->setFixedHeight(1);
    mainLayout->addWidget(separator2);

    QHBoxLayout *events_layout = new QHBoxLayout();
    events_layout->setSpacing(2);
    events_layout->setContentsMargins(0, 0, 0, 0);

    dmgInBattleBox = createButtonCheckbox("Эффек-ть", dmgCheck);
    historyInBattleBox = createButtonCheckbox("Cобытия", historyCheck);

    connect(dmgInBattleBox, &QPushButton::clicked, this, &Widgets::toggleBattleStats);
    connect(historyInBattleBox, &QPushButton::clicked, this, &Widgets::toggleHistory);

    events_layout->addWidget(dmgInBattleBox);
    events_layout->addWidget(historyInBattleBox);

    mainLayout->addLayout(events_layout);

    QFrame *separator3 = new QFrame;
    separator3->setFrameShape(QFrame::HLine);
    separator3->setFrameShadow(QFrame::Plain);
    separator3->setStyleSheet("background-color: rgba(226, 222, 211, 0.15);");
    separator3->setFixedHeight(1);
    mainLayout->addWidget(separator3);

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

        this->widgetsWindow->setOpacity(m_backgroundOpacity);
        this->widgetsWindow->update();

        QSettings settings("MyCompany", "StatsOverlay");
        settings.setValue("backgroundOpacityWidgets", m_backgroundOpacity); });

        return container;
    };
    mainLayout->addWidget(createOpacitySlider());

    setLayout(mainLayout);

    m_replayTracker = new ReplayTracker(this);

    QTimer *replayTimer = new QTimer(this);
    connect(replayTimer, &QTimer::timeout, this, &Widgets::updateBattleDamage);
    replayTimer->start(5000);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Widgets::updatingWidgets);
    timer->start(30000);
}

void Widgets::updateBattleDamage()
{
    QMetaObject::invokeMethod(
        this,
        [this]()
        {
            if (!this->is_dmg_widget && !this->is_history_widget)
                return;

            if (m_replayBusy)
                return;

            m_replayBusy = true;

            QtConcurrent::run([this]()
                              {
                // Ищем реплей — папка recording* в Documents/TanksBlitz/replays/
                QString documents = QStandardPaths::writableLocation(
                    QStandardPaths::DocumentsLocation);
                QDir replaysDir(documents + "/TanksBlitz/replays/");

                QString replayPath;

                if (replaysDir.exists()) {
                    QStringList recordings = replaysDir.entryList(
                        QStringList() << "recording*",
                        QDir::Dirs | QDir::NoDotAndDotDot);

                    if (!recordings.isEmpty()) {
                        QString candidate = replaysDir.absoluteFilePath(recordings.first())
                                            + "/data.replay";
                        if (QFile::exists(candidate))
                            replayPath = candidate;
                    }
                }

                if (replayPath.isEmpty()) {
                    // Нет активного боя — сбрасываем
                    QMetaObject::invokeMethod(this, [this]()
                    {
                        m_replayTracker->reset();
                        m_replayBusy = false;
                        
                        if(dmgWidget->isVisible())
                        {
                            dmgWidget->hide();
                        }
                        dmgWidget->clearData();

                        if(hWidget->isVisible())
                        {
                            hWidget->hide();
                        }
                        hWidget->clearData();

                    }, Qt::QueuedConnection);
                    return;
                }

                QString selfNick = QString::fromStdString(m_apiController->getNickname()); // ← адаптировать

                bool updated = m_replayTracker->update(replayPath, selfNick);

                if (updated) {
                    QVector<DamageEvent> events      = m_replayTracker->events();
                    QVector<BattleEvent> history     = m_replayTracker->battleHistory();
                    int totalDamage                  = m_replayTracker->totalDamage();
                    int totalBlocked                 = m_replayTracker->totalBlocked();
                    int totalAssist                  = m_replayTracker->totalAssist();

                    QMetaObject::invokeMethod(this, [this, events, history, totalDamage, totalBlocked, totalAssist]()
                    {
                        qDebug() << "════════════════════════════════════════";
                        qDebug() << "[BattleDmg] Игрок:" << m_apiController->getNickname()
                                 << " Урон:" << totalDamage
                                 << " Блок:" << totalBlocked
                                 << " Ассист:" << totalAssist
                                 << " Событий:" << events.size();
                        qDebug() << "────────────────────────────────────────";
                        for (const DamageEvent &ev : events) {
                            qDebug().noquote()
                                << QString("t=%1s  →%2  dmg=%3  hp=%4  [%5]  total=%6")
                                   .arg(ev.time_s,       6, 'f', 2)
                                   .arg(ev.target,      -22)
                                   .arg(ev.damage,        5)
                                   .arg(ev.hp_after,      5)
                                   .arg(ev.hit_result,  -10)
                                   .arg(ev.total_damage,  6);
                        }
                        qDebug() << "════════════════════════════════════════";

                        if (!dmgWidget->isVisible() && is_dmg_widget)
                            dmgWidget->show();

                        if (!hWidget->isVisible() && is_history_widget)
                            hWidget->show();

                        if (!events.isEmpty()) {
                            BattleData data;
                            data.damage = totalDamage;
                            data.block  = totalBlocked;
                            data.assist = totalAssist;
                            dmgWidget->setData(data);
                        }

                        hWidget->setHistory(history);

                        m_replayBusy = false;
                    }, Qt::QueuedConnection);
                } else {
                    QMetaObject::invokeMethod(this, [this]()
                    {
                        m_replayBusy = false;
                    }, Qt::QueuedConnection);
                } });
        },
        Qt::QueuedConnection);
}

void Widgets::toggleRating()
{
    QMetaObject::invokeMethod(
        this,
        [this]()
        {
            if (is_rating)
            {
                ratingCheck->setText("");
            }
            else
            {
                ratingCheck->setText("✔");
            }
            widgetsWindow->toggleRating(!is_rating);
            is_rating = !is_rating;
        },
        Qt::QueuedConnection);
}

void Widgets::toggleMaster()
{
    QMetaObject::invokeMethod(
        this,
        [this]()
        {
            if (is_master)
            {
                masterCheck->setText("");
            }
            else
            {
                masterCheck->setText("✔");
            }
            widgetsWindow->toggleMaster(!is_master);
            is_master = !is_master;
        },
        Qt::QueuedConnection);
}

void Widgets::toggleBattles()
{
    QMetaObject::invokeMethod(
        this,
        [this]()
        {
            if (is_battles)
            {
                battlesCheck->setText("");
            }
            else
            {
                battlesCheck->setText("✔");
            }
            widgetsWindow->toggleBattles(!is_battles);
            is_battles = !is_battles;
        },
        Qt::QueuedConnection);
}

void Widgets::toggleWins()
{
    QMetaObject::invokeMethod(
        this,
        [this]()
        {
            if (is_wins)
            {
                winsCheck->setText("");
            }
            else
            {
                winsCheck->setText("✔");
            }
            widgetsWindow->toggleWins(!is_wins);
            is_wins = !is_wins;
        },
        Qt::QueuedConnection);
}

void Widgets::toggleDamage()
{
    QMetaObject::invokeMethod(
        this,
        [this]()
        {
            if (is_damage)
            {
                damageCheck->setText("");
            }
            else
            {
                damageCheck->setText("✔");
            }
            widgetsWindow->toggleDamage(!is_damage);
            is_damage = !is_damage;
        },
        Qt::QueuedConnection);
}

void Widgets::toggleXp()
{
    QMetaObject::invokeMethod(
        this,
        [this]()
        {
            if (is_xp)
            {
                xpCheck->setText("");
            }
            else
            {
                xpCheck->setText("✔");
            }
            widgetsWindow->toggleXp(!is_xp);
            is_xp = !is_xp;
        },
        Qt::QueuedConnection);
}

void Widgets::toggleAlive()
{
    QMetaObject::invokeMethod(
        this,
        [this]()
        {
            if (is_alive)
            {
                aliveCheck->setText("");
            }
            else
            {
                aliveCheck->setText("✔");
            }
            widgetsWindow->toggleAlive(!is_alive);
            is_alive = !is_alive;
        },
        Qt::QueuedConnection);
}

void Widgets::toggleShots()
{
    QMetaObject::invokeMethod(
        this,
        [this]()
        {
            if (is_shots)
            {
                shotsCheck->setText("");
            }
            else
            {
                shotsCheck->setText("✔");
            }
            widgetsWindow->toggleShots(!is_shots);
            is_shots = !is_shots;
        },
        Qt::QueuedConnection);
}

void Widgets::toggleBattleStats()
{
    QMetaObject::invokeMethod(
        this,
        [this]()
        {
            if (is_dmg_widget)
            {
                dmgCheck->setText("");
            }
            else
            {
                dmgCheck->setText("✔");
            }
            is_dmg_widget = !is_dmg_widget;
        },
        Qt::QueuedConnection);
}

void Widgets::toggleHistory()
{
    QMetaObject::invokeMethod(
        this,
        [this]()
        {
            if (is_history_widget)
            {
                historyCheck->setText("");
            }
            else
            {
                historyCheck->setText("✔");
            }
            is_history_widget = !is_history_widget;
        },
        Qt::QueuedConnection);
}

void Widgets::updatingWidgets()
{
    QMetaObject::invokeMethod(
        this,
        [this]()
        {
            qDebug() << "updatingWidgets called, isAuth:" << isAuth();
            if (!isAuth())
                return;

            QtConcurrent::run([this]()
                              {
            MainStatsData main = m_apiController->getMainStats();
            RatingStatsData rating = m_apiController->getRatingStats();
            OtherStatsData other = m_apiController->getOtherStats();
            MasteryStatsData masteryData = m_apiController->getMasteryStats();

            int64_t battles = main.getBattles() + rating.getBattles();

            if (battles > 0)
            {
                float wins = std::round(((static_cast<float>(main.getWins()) +
                                          static_cast<float>(rating.getWins())) *
                                         100.0f / battles) * 100.0f) / 100.0f;
                int64_t damage = (main.getTotalDamage() + rating.getTotalDamage()) / battles;
                int64_t xp = (main.getExpBattle() + rating.getExpBattle()) / battles;
                float shots = other.getPercentHits();
                float alive = other.getPercentSurvived();

                QMetaObject::invokeMethod(this, [this, battles, damage, wins, xp, shots, alive]()
                {
                    widgetsWindow->updateTile("Бои", QString::fromStdString(widgetsWindow->formatInt(battles)));
                    widgetsWindow->updateWins(wins);
                    widgetsWindow->updateTile("Урон", QString::fromStdString(widgetsWindow->formatInt(damage)));
                    widgetsWindow->updateTile("Опыт", QString::fromStdString(widgetsWindow->formatInt(xp)));
                    if (shots >= 0.0f)
                        widgetsWindow->updateTile("Точность", QString::fromStdString(widgetsWindow->formatFloat(shots)) + "%");
                    if (alive >= 0.0f)
                        widgetsWindow->updateTile("Выжил", QString::fromStdString(widgetsWindow->formatFloat(alive)) + "%");
                }, Qt::QueuedConnection);
            }

            int64_t master = masteryData.getCurrentData().mastery;
            int64_t rank = rating.getRating();

            QMetaObject::invokeMethod(this, [this, master, rank]()
            {
                if (master >= 0)
                    widgetsWindow->updateTile("Мастер", QString::fromStdString(widgetsWindow->formatInt(master)));
                if (rank >= 0)
                    widgetsWindow->updateTile("Рейтинг", QString::fromStdString(widgetsWindow->formatInt(rank)));
            }, Qt::QueuedConnection); });
        },
        Qt::QueuedConnection);
}
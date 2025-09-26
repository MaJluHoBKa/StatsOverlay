#include <main_overlay/widgets/player_stats/split_players/players_window.h>

PlayerStatsWindow::PlayerStatsWindow(const QString &title, QWidget *parent = nullptr)
    : QWidget(parent)
{
    setMaximumWidth(260);
    setContentsMargins(0, 2, 0, 2);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet(
        "border-top-right-radius: 10px;"
        "border-bottom-right-radius: 10px;");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Главный слой для виджета
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(5);

    // Слой заголовка статистики по игрокам
    QHBoxLayout *listsStats = new QHBoxLayout();
    listsStats->setAlignment(Qt::AlignTop);
    listsStats->setSpacing(5);

    QLabel *icon = new QLabel;
    icon->setPixmap(QPixmap(":player_stats/resources/icons/arrow_icon.png"));
    icon->setMaximumWidth(30);
    icon->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    listsStats->addWidget(icon);

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

    // Выставление кнопок переключения
    QHBoxLayout *config_layout = new QHBoxLayout;
    config_layout->setSpacing(1);
    config_layout->setContentsMargins(0, 0, 0, 0);

    const QString baseStyle = R"(
                QPushButton {
                    border: 2px solid rgb(57, 57, 57);
                    font-family: Segoe UI;
                    font-weight: bold;
                    font-size: 14px;
                    color: #e2ded3;
                    border-radius: 3px;
                }
                QPushButton:hover {
                    background-color: rgb(60, 60, 60);
                }
                QPushButton:pressed {
                    background-color: rgb(90, 90, 90);
                }
                )";

    QPushButton *buttonAllies = new QPushButton;
    buttonAllies->setText("Союзники");
    buttonAllies->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    buttonAllies->setStyleSheet(baseStyle);
    connect(buttonAllies, &QPushButton::clicked, this, [this]()
            { switchPlayers(0); });
    config_layout->addWidget(buttonAllies);
    setButtonAllies(buttonAllies);

    QPushButton *buttonEnemies = new QPushButton;
    buttonEnemies->setText("Противники");
    buttonEnemies->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    buttonEnemies->setStyleSheet(baseStyle);
    connect(buttonEnemies, &QPushButton::clicked, this, [this]()
            { switchPlayers(1); });
    config_layout->addWidget(buttonEnemies);
    setButtonEnemies(buttonEnemies);

    mainLayout->addLayout(config_layout);

    // Выставление обозначающих иконок
    QWidget *header_widget = new QWidget;
    header_widget->setFixedHeight(25);

    QHBoxLayout *header = new QHBoxLayout(header_widget);
    header->setAlignment(Qt::AlignTop);
    header->setSpacing(1);
    header->setContentsMargins(0, 0, 0, 0);

    std::vector<int> sizes = {105, 105, 50, 50, 50};
    QStringList iconPaths = {
        ":player_stats/resources/icons/player_icon.png",
        ":player_stats/resources/icons/tanks_icon.png",
        ":player_stats/resources/icons/battles_icon.png",
        ":player_stats/resources/icons/win_icon.png",
        ":player_stats/resources/icons/damage_icon.png"};

    for (int i = 0; i < iconPaths.size(); ++i)
    {
        QLabel *icon = new QLabel();
        QPixmap pixmap(iconPaths[i]);
        icon->setPixmap(pixmap);
        icon->setFixedWidth(sizes[i]);
        icon->setStyleSheet(
            "border: 1px solid rgb(57, 57, 57);"
            "border-radius: 3px;");
        icon->setAlignment(Qt::AlignCenter);
        header->addWidget(icon);
    }
    mainLayout->addWidget(header_widget);

    // Создание слоев игроков
    QStackedWidget *pages = new QStackedWidget;
    pages->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout->addWidget(pages);
    setStackedWidgets(pages);

    // Создание списка игроков
    QVBoxLayout *data_allies_l = new QVBoxLayout;
    data_allies_l->setSpacing(1);
    data_allies_l->setContentsMargins(0, 0, 0, 0);
    data_allies_l->setAlignment(Qt::AlignTop);
    setDataAlliesLayout(data_allies_l);

    QWidget *data_allies_widget = new QWidget;
    data_allies_widget->setMaximumHeight(180);
    data_allies_widget->setLayout(data_allies_l);
    pages->addWidget(data_allies_widget);

    QVBoxLayout *data_enemies_l = new QVBoxLayout;
    data_enemies_l->setSpacing(1);
    data_enemies_l->setContentsMargins(0, 0, 0, 0);
    data_enemies_l->setAlignment(Qt::AlignTop);
    setDataEnemiesLayout(data_enemies_l);

    QWidget *data_enemies_widget = new QWidget;
    data_enemies_widget->setMaximumHeight(180);
    data_enemies_widget->setLayout(data_enemies_l);
    pages->addWidget(data_enemies_widget);

    addAlliesRows();
    addEnemiesRows();

    setLayout(mainLayout);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &PlayerStats::updatingPlayerStats);
    timer->start(10000);
}
#include <main_overlay/widgets/vehicles_stats/vehicles_stats.h>

VehicleStats::VehicleStats(ApiController *apiController, QWidget *parent)
    : m_apiController(apiController), QWidget(parent)
{
    setFixedWidth(280);
    setContentsMargins(0, 2, 0, 2);
    setStyleSheet(
        "border-top-right-radius: 10px;"
        "border-bottom-right-radius: 10px;");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Главный слой для виджета
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(5);

    // Слой заголовка статистики по технике
    QHBoxLayout *listsStats = new QHBoxLayout();
    listsStats->setAlignment(Qt::AlignTop);
    listsStats->setSpacing(5);

    QLabel *icon = new QLabel;
    icon->setPixmap(QPixmap(":vehicle_stats/resources/icons/arrow_icon.png"));
    icon->setMaximumWidth(30);
    icon->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    listsStats->addWidget(icon);

    QLabel *label = new QLabel;
    label->setText("Статистика по технике");
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

    // Выставление обозначающих иконок
    QWidget *header_widget = new QWidget;
    header_widget->setFixedHeight(25);

    QHBoxLayout *header = new QHBoxLayout(header_widget);
    header->setAlignment(Qt::AlignTop);
    header->setSpacing(1);
    header->setContentsMargins(0, 0, 0, 0);

    std::vector<int> sizes = {105, 50, 50, 50};
    QStringList iconPaths = {
        ":vehicle_stats/resources/icons/tanks_icon.png",
        ":vehicle_stats/resources/icons/battles_icon.png",
        ":vehicle_stats/resources/icons/win_icon.png",
        ":vehicle_stats/resources/icons/damage_icon.png"};

    for (int i = 0; i < iconPaths.size(); ++i)
    {
        QLabel *icon = new QLabel();
        QPixmap pixmap(iconPaths[i]);
        icon->setPixmap(pixmap);
        icon->setFixedWidth(sizes[i]);
        icon->setStyleSheet(
            "background-color: #282828;"
            "border-radius: 3px;");
        icon->setAlignment(Qt::AlignCenter);
        header->addWidget(icon);
    }
    mainLayout->addWidget(header_widget);

    // Создание списка танков
    QVBoxLayout *data = new QVBoxLayout;
    data->setSpacing(1);
    data->setContentsMargins(0, 0, 0, 0);
    data->setAlignment(Qt::AlignTop);

    QWidget *data_widget = new QWidget;
    setDataLayout(data);
    data_widget->setLayout(data);

    QScrollArea *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setWidget(data_widget);
    scroll->setStyleSheet(
        "QScrollArea {"
        "    background-color: rgba(0, 0, 0, 0);"
        "    border: none;"
        "    border-radius: 0px;"
        "}"
        "QScrollBar:vertical {"
        "    background: rgba(0, 0, 0, 0);"
        "    width: 4px;"
        "    margin: 2px 0;"
        "    border-radius: 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: #555555;"
        "    border-radius: 2px;"
        "}"
        "QScrollBar::groove:vertical {"
        "    border-radius: 0px;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    height: 0;"
        "}");
    scroll->setMaximumHeight(220);
    mainLayout->addWidget(scroll);

    setLayout(mainLayout);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &VehicleStats::updatingVehicleStats);
    timer->start(30000);
}
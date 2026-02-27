#include <main_overlay/widgets/vehicles_stats/vehicles_stats.h>

VehicleStats::VehicleStats(ApiController *apiController, GunMarks *gunMark, QWidget *parent)
    : m_apiController(apiController), m_gunMark(gunMark), QWidget(parent)
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

    // Слой заголовка статистики по технике
    QHBoxLayout *listsStats = new QHBoxLayout();
    listsStats->setAlignment(Qt::AlignTop);
    listsStats->setSpacing(5);

    QFrame *accent = new QFrame;
    accent->setFixedWidth(3);
    accent->setStyleSheet("background-color: #c8a84b; border-radius: 1px;");
    listsStats->addWidget(accent);

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

    QFrame *separator = new QFrame;
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Plain);
    separator->setStyleSheet("background-color: rgba(226, 222, 211, 0.15);");
    separator->setFixedHeight(1);
    mainLayout->addWidget(separator);

    // Выставление обозначающих иконок
    QWidget *header_widget = new QWidget;
    header_widget->setFixedHeight(25);
    header_widget->setStyleSheet(
        "background-color: #282828;"
        "border-radius: 3px;");

    QHBoxLayout *header = new QHBoxLayout(header_widget);
    header->setAlignment(Qt::AlignTop);
    header->setSpacing(0);
    header->setContentsMargins(0, 0, 0, 0);

    std::vector<int> sizes = {30, 105, 60, 60, 60};
    QStringList iconPaths = {
        ":vehicle_stats/resources/icons/type_icon.png",
        ":vehicle_stats/resources/icons/tanks_icon.png",
        ":vehicle_stats/resources/icons/battles_icon.png",
        ":vehicle_stats/resources/icons/win_icon.png",
        ":vehicle_stats/resources/icons/damage_icon.png"};

    QLabel *iconTier = new QLabel();
    iconTier->setText("I-X");
    iconTier->setFixedWidth(30);
    iconTier->setStyleSheet(
        "font-family: Segoe UI;"
        "font-size: 11px;"
        "font-weight: bold;"
        "color: #e2ded3;"
        "white-space: nowrap;"
        "background-color: transparent;"
        "border-radius: 3px;");
    iconTier->setAlignment(Qt::AlignCenter);
    header->addWidget(iconTier);

    for (int i = 0; i < iconPaths.size(); ++i)
    {
        QFrame *sep = new QFrame;
        sep->setFrameShape(QFrame::VLine);
        sep->setFixedWidth(1);
        sep->setFixedHeight(15);
        sep->setStyleSheet("background-color: rgba(255,255,255,0.08);");
        header->addWidget(sep, 0, Qt::AlignVCenter);

        QLabel *icon = new QLabel();
        QPixmap pixmap(iconPaths[i]);
        icon->setPixmap(pixmap);
        icon->setFixedWidth(sizes[i]);
        icon->setStyleSheet(
            "background-color: transparent;"
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
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
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

    // Обёртка
    QWidget *scrollContainer = new QWidget;
    scrollContainer->setFixedHeight(220);
    scrollContainer->setStyleSheet("background: transparent;");

    QVBoxLayout *scrollContainerLayout = new QVBoxLayout(scrollContainer);
    scrollContainerLayout->setContentsMargins(0, 0, 0, 0);
    scrollContainerLayout->addWidget(scroll);

    // Оверлей с fade снизу
    QWidget *fadeOverlay = new QWidget(scrollContainer);
    fadeOverlay->setAttribute(Qt::WA_TransparentForMouseEvents);
    fadeOverlay->setGeometry(0, 0, scrollContainer->width(), 220);
    fadeOverlay->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "stop:0.0 rgba(0,0,0,0),"
        "stop:0.9 rgba(0,0,0,0),"
        "stop:1.0 rgba(30,30,30,255));");
    fadeOverlay->raise();

    mainLayout->addWidget(scrollContainer);

    setLayout(mainLayout);

    addTankRow(23232, "ЛВ-5000", 10, "lt", "japan", "default", 9, 45.52, 150);
    addTankRow(23232, "Chieftain MK. 6", 10, "tt", "uk", "premium", 10, 68.52, 3420);
    addTankRow(23232, "Project Murat", 10, "st", "france", "collect", 9, 72.52, 3420);
    addTankRow(23232, "ИС-7", 10, "tt", "ussr", "default", 21, 59.52, 2220);
    addTankRow(23232, "Grille 15", 10, "pt", "germany", "default", 9, 72.52, 3420);
    addTankRow(23232, "ЛВ-1300 Уран", 10, "tt", "lesta", "default", 9, 45.52, 150);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &VehicleStats::updatingVehicleStats);
    timer->start(30000);
}
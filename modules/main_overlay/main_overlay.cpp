#include <main_overlay/main_overlay.h>

class GlobalHotkeyFilter : public QAbstractNativeEventFilter
{
public:
    QWidget *overlay;

    GlobalHotkeyFilter(QWidget *overlay) : overlay(overlay) {}

    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override
    {
        MSG *msg = static_cast<MSG *>(message);
        if (msg->message == WM_HOTKEY)
        {
            MainOverlay *sub = qobject_cast<MainOverlay *>(overlay);
            if (!sub)
                return false;

            if (msg->wParam == 2) // Ctrl + UP
            {
                sub->prevHotPage();
            }
            else if (msg->wParam == 3) // Ctrl + DOWN
            {
                sub->nextHotPage();
            }
        }
        return false;
    }
};

MainOverlay::MainOverlay(ApiController *apiController, QWidget *parent)
    : QWidget(parent), m_apiController(apiController)
{
    // Настройка основного окна
    QApplication::setStyle("Fusion");
    QApplication::setWindowIcon(QPixmap(":main/resources/icons/icon.ico"));
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    resize(280, 200);
    setAttribute(Qt::WA_TranslucentBackground);
    move(100, (QApplication::primaryScreen()->availableGeometry().height() - height()) / 2);

    // Главный слой для окна
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    PlayerStats *playerStats = new PlayerStats(m_apiController);
    OtherStats *otherStats = new OtherStats(m_apiController);
    VehicleStats *vehicleStats = new VehicleStats(m_apiController);
    RatingStats *ratingStats = new RatingStats(m_apiController);
    MainStats *mainStats = new MainStats(m_apiController);
    InfoPage *infoPage = new InfoPage(m_apiController, mainStats, ratingStats, vehicleStats, otherStats);

    setPlayerPage(playerStats);
    setVehiclePage(vehicleStats);
    setInfoPage(infoPage);

    // Панель управления
    configPanel = new QWidget;
    configPanel->setStyleSheet(
        "border: 0px solid #404040;"
        "border-top-left-radius: 10px;"
        "border-bottom-left-radius: 10px;");

    // Вертикальный контейнер для кнопок в панели управления
    QVBoxLayout *buttonsLayout = new QVBoxLayout;
    buttonsLayout->setSpacing(0);
    buttonsLayout->setContentsMargins(0, 0, 0, 0);

    // Кнопка основной статистики
    QPushButton *buttonMain = new QPushButton;
    buttonMain->setIcon(QIcon(QPixmap(":main/resources/icons/stats_icon.png")));
    buttonMain->setIconSize(QSize(24, 24));
    buttonMain->setStyleSheet(
        "QPushButton {"
        "    border: none;"
        "    padding: 2px;"
        "    border-top-left-radius: 10px;"
        "    border-bottom-left-radius: 0px;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgb(50, 50, 50);"
        "}"
        "QPushButton:pressed {"
        "    background-color: rgb(70, 70, 70);"
        "}");
    buttonMain->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    connect(buttonMain, &QPushButton::clicked, this, [this]()
            { switchPage(5); });
    buttonsLayout->addWidget(buttonMain);

    // Кнопка рейтинговой статистики
    QPushButton *buttonRating = new QPushButton;
    buttonRating->setIcon(QIcon(QPixmap(":main/resources/icons/rating_icon.png")));
    buttonRating->setIconSize(QSize(24, 24));
    buttonRating->setStyleSheet(
        "QPushButton {"
        "    border: none;"
        "    padding: 2px;"
        "    border-top-left-radius: 0px;"
        "    border-bottom-left-radius: 0px;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgb(50, 50, 50);"
        "}"
        "QPushButton:pressed {"
        "    background-color: rgb(70, 70, 70);"
        "}");
    buttonRating->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    connect(buttonRating, &QPushButton::clicked, this, [this]()
            { switchPage(4); });
    buttonsLayout->addWidget(buttonRating);

    // Кнопка танковой статистики
    QPushButton *buttonTanks = new QPushButton;
    buttonTanks->setIcon(QIcon(QPixmap(":main/resources/icons/tanks_icon.png")));
    buttonTanks->setIconSize(QSize(24, 24));
    buttonTanks->setStyleSheet(
        "QPushButton {"
        "    border: none;"
        "    padding: 2px;"
        "    border-top-left-radius: 0px;"
        "    border-bottom-left-radius: 0px;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgb(50, 50, 50);"
        "}"
        "QPushButton:pressed {"
        "    background-color: rgb(70, 70, 70);"
        "}");
    buttonTanks->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    connect(buttonTanks, &QPushButton::clicked, this, [this]()
            { switchPage(3); });
    buttonsLayout->addWidget(buttonTanks);

    // Кнопка коэффициентной статистики
    QPushButton *buttonOther = new QPushButton;
    buttonOther->setIcon(QIcon(QPixmap(":main/resources/icons/other_icon.png")));
    buttonOther->setIconSize(QSize(24, 24));
    buttonOther->setStyleSheet(
        "QPushButton {"
        "    border: none;"
        "    padding: 2px;"
        "    border-top-left-radius: 0px;"
        "    border-bottom-left-radius: 0px;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgb(50, 50, 50);"
        "}"
        "QPushButton:pressed {"
        "    background-color: rgb(70, 70, 70);"
        "}");
    buttonOther->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    connect(buttonOther, &QPushButton::clicked, this, [this]()
            { switchPage(2); });
    buttonsLayout->addWidget(buttonOther);

    // // Кнопка графиковой статистики
    // QPushButton *buttonGraphics = new QPushButton;
    // buttonGraphics->setIcon(QIcon(QPixmap(":main/resources/icons/graphics_icon.png")));
    // buttonGraphics->setIconSize(QSize(24, 24));
    // buttonGraphics->setStyleSheet(
    //     "QPushButton {"
    //     "    border: none;"
    //     "    padding: 2px;"
    //     "    border-top-left-radius: 0px;"
    //     "    border-bottom-left-radius: 0px;"
    //     "}"
    //     "QPushButton:hover {"
    //     "    background-color: rgb(50, 50, 50);"
    //     "}"
    //     "QPushButton:pressed {"
    //     "    background-color: rgb(70, 70, 70);"
    //     "}");
    // buttonGraphics->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    // buttonsLayout->addWidget(buttonGraphics);

    // // Кнопка плиточной статистики
    // QPushButton *buttonStream = new QPushButton;
    // buttonStream->setIcon(QIcon(QPixmap(":main/resources/icons/stream_icon.png")));
    // buttonStream->setIconSize(QSize(24, 24));
    // buttonStream->setStyleSheet(
    //     "QPushButton {"
    //     "    border: none;"
    //     "    padding: 2px;"
    //     "    border-top-left-radius: 0px;"
    //     "    border-bottom-left-radius: 0px;"
    //     "}"
    //     "QPushButton:hover {"
    //     "    background-color: rgb(50, 50, 50);"
    //     "}"
    //     "QPushButton:pressed {"
    //     "    background-color: rgb(70, 70, 70);"
    //     "}");
    // buttonStream->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    // buttonsLayout->addWidget(buttonStream);

    // Кнопка оленемера
    QPushButton *buttonPlayers = new QPushButton;
    buttonPlayers->setIcon(QIcon(QPixmap(":main/resources/icons/player_icon.png")));
    buttonPlayers->setIconSize(QSize(24, 24));
    buttonPlayers->setStyleSheet(
        "QPushButton {"
        "    border: none;"
        "    padding: 2px;"
        "    border-top-left-radius: 0px;"
        "    border-bottom-left-radius: 0px;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgb(50, 50, 50);"
        "}"
        "QPushButton:pressed {"
        "    background-color: rgb(70, 70, 70);"
        "}");
    buttonPlayers->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    connect(buttonPlayers, &QPushButton::clicked, this, [this]()
            { switchPage(1); });
    buttonsLayout->addWidget(buttonPlayers);

    // Кнопка информационной панели
    QPushButton *buttonInfo = new QPushButton;
    buttonInfo->setIcon(QIcon(QPixmap(":main/resources/icons/info_icon.png")));
    buttonInfo->setIconSize(QSize(24, 24));
    buttonInfo->setStyleSheet(
        "QPushButton {"
        "    border: none;"
        "    padding: 2px;"
        "    border-top-left-radius: 0px;"
        "    border-bottom-left-radius: 0px;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgb(50, 50, 50);"
        "}"
        "QPushButton:pressed {"
        "    background-color: rgb(70, 70, 70);"
        "}");
    buttonInfo->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    connect(buttonInfo, &QPushButton::clicked, this, [this]()
            { switchPage(0); });
    buttonsLayout->addWidget(buttonInfo);

    // Кнопка выхода
    QPushButton *buttonExit = new QPushButton;
    buttonExit->setIcon(QIcon(QPixmap(":main/resources/icons/exit_icon.png")));
    buttonExit->setIconSize(QSize(24, 24));
    buttonExit->setStyleSheet(
        "QPushButton {"
        "    border: none;"
        "    padding: 2px;"
        "    border-top-left-radius: 0px;"
        "    border-bottom-left-radius: 10px;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgb(50, 50, 50);"
        "}"
        "QPushButton:pressed {"
        "    background-color: rgb(70, 70, 70);"
        "}");
    buttonExit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    connect(buttonExit, &QPushButton::clicked, this, [this]()
            { logout(); });
    buttonsLayout->addWidget(buttonExit);

    configPanel->setLayout(buttonsLayout);
    configPanel->setMaximumWidth(0);
    showConfigPanel(false);
    mainLayout->addWidget(configPanel);
    defaultWidth = 250;
    resize(defaultWidth, height());

    // Контейнер с разными вкладками
    QStackedWidget *stacked_widget = new QStackedWidget;
    stacked_widget->setContentsMargins(0, 0, 0, 0);

    stacked_widget->addWidget(infoPage);
    stacked_widget->setCurrentIndex(0);

    stacked_widget->addWidget(playerStats);
    stacked_widget->setCurrentIndex(1);

    stacked_widget->addWidget(otherStats);
    stacked_widget->setCurrentIndex(2);

    stacked_widget->addWidget(vehicleStats);
    stacked_widget->setCurrentIndex(3);

    stacked_widget->addWidget(ratingStats);
    stacked_widget->setCurrentIndex(4);

    stacked_widget->addWidget(mainStats);
    stacked_widget->setCurrentIndex(5);

    setStackedWidget(stacked_widget);
    mainLayout->addWidget(stacked_widget);

    setLayout(mainLayout);

    QSettings settings("MyCompany", "StatsOverlay");
    m_backgroundOpacity = settings.value("backgroundOpacity", 1.0).toDouble();
    bool showPanel = settings.value("showPanel", true).toBool();
    configPanel->setVisible(showPanel);

    RegisterHotKey(NULL, 2, MOD_CONTROL, VK_UP);
    RegisterHotKey(NULL, 3, MOD_CONTROL, VK_DOWN);
    auto *hotkeyFilter = new GlobalHotkeyFilter(this);
    qApp->installNativeEventFilter(hotkeyFilter);
}

void MainOverlay::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(QColor(30, 30, 30, int(m_backgroundOpacity * 255)));
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(rect(), 10, 10);
}

void MainOverlay::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_dragActive = true;
        m_dragStartPos = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void MainOverlay::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragActive && (event->buttons() & Qt::LeftButton))
    {
        move(event->globalPos() - m_dragStartPos);
        event->accept();
    }
}

void MainOverlay::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_dragActive = false;
        event->accept();
    }
}

void MainOverlay::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.setMinimumWidth(180);
    menu.setStyleSheet(R"(
        QMenu {
            background-color: rgb(40,40,40);
            border: 1px solid #555;
            padding: 4px; /* внутренние отступы меню */
        }
        QMenu::item {
            padding: 6px 7px; /* отступы у пунктов */
        }
        QMenu::item:selected {
            background-color: rgb(60,60,60);
        }
    )");

    QWidgetAction *labelAction = new QWidgetAction(&menu);
    QLabel *label = new QLabel("Прозрачность:");
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    label->setStyleSheet("padding: 4px 0 8px 8px; color: #e2ded3;"); // фон прозрачный
    labelAction->setDefaultWidget(label);
    menu.addAction(labelAction);

    QWidgetAction *sliderAction = new QWidgetAction(&menu);
    QSlider *slider = new QSlider(Qt::Horizontal);
    slider->setRange(10, 100);
    slider->setValue(int(m_backgroundOpacity * 100));
    slider->setMinimumWidth(160);
    slider->setStyleSheet(R"(
        QSlider::groove:horizontal {
            border: 1px solid #444;
            height: 4px;
            background: #333;
            border-radius: 2px;
            margin: 0 8px;
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
    connect(slider, &QSlider::valueChanged, this, [this](int value)
            {
        m_backgroundOpacity = value / 100.0;
        QSettings settings("MyCompany", "StatsOverlay");
        settings.setValue("backgroundOpacity", m_backgroundOpacity);
        update(); });
    sliderAction->setDefaultWidget(slider);
    menu.addAction(sliderAction);

    menu.exec(event->globalPos());
}
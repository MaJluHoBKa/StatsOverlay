#include <main_overlay/main_overlay.h>

MainOverlay::MainOverlay(QWidget *parent) : QWidget(parent)
{
    // Настройка основного окна
    QApplication::setStyle("Fusion");
    QApplication::setWindowIcon(QPixmap(":resources/icons/icon.png"));
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    resize(400, 200);
    move(100, (QApplication::primaryScreen()->availableGeometry().height() - height()) / 2);

    // Главный слой для окна
    QHBoxLayout *mainLayout = new QHBoxLayout(this);

    // Панель управления
    QWidget *config_panel = new QWidget;
    config_panel->setStyleSheet(
        "background-color: rgba(30, 30, 30, 255);"
        "border: 0px solid #404040;"
        "border-top-left-radius: 10px;"
        "border-bottom-left-radius: 10px;");

    // Вертикальный контейнер для кнопок в панели управления
    QVBoxLayout *buttonsLayout = new QVBoxLayout;
    buttonsLayout->setSpacing(0);
    buttonsLayout->setContentsMargins(0, 0, 0, 0);

    // Кнопка основной статистики
    QPushButton *buttonMain = new QPushButton;
    buttonMain->setIcon(QIcon(QPixmap(":resources/icons/stats_icon.png")));
    buttonMain->setIconSize(QSize(24, 24));
    buttonMain->setStyleSheet(
        "QPushButton {"
        "    background-color: rgb(30, 30, 30);"
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
    buttonsLayout->addWidget(buttonMain);

    // Кнопка рейтинговой статистики
    QPushButton *buttonRating = new QPushButton;
    buttonRating->setIcon(QIcon(QPixmap(":resources/icons/rating_icon.png")));
    buttonRating->setIconSize(QSize(24, 24));
    buttonRating->setStyleSheet(
        "QPushButton {"
        "    background-color: rgb(30, 30, 30);"
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
    buttonsLayout->addWidget(buttonRating);

    // Кнопка танковой статистики
    QPushButton *buttonTanks = new QPushButton;
    buttonTanks->setIcon(QIcon(QPixmap(":resources/icons/tanks_icon.png")));
    buttonTanks->setIconSize(QSize(24, 24));
    buttonTanks->setStyleSheet(
        "QPushButton {"
        "    background-color: rgb(30, 30, 30);"
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
    buttonsLayout->addWidget(buttonTanks);

    // Кнопка коэффициентной статистики
    QPushButton *buttonOther = new QPushButton;
    buttonOther->setIcon(QIcon(QPixmap(":resources/icons/other_icon.png")));
    buttonOther->setIconSize(QSize(24, 24));
    buttonOther->setStyleSheet(
        "QPushButton {"
        "    background-color: rgb(30, 30, 30);"
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
    buttonsLayout->addWidget(buttonOther);

    // Кнопка графиковой статистики
    QPushButton *buttonGraphics = new QPushButton;
    buttonGraphics->setIcon(QIcon(QPixmap(":resources/icons/graphics_icon.png")));
    buttonGraphics->setIconSize(QSize(24, 24));
    buttonGraphics->setStyleSheet(
        "QPushButton {"
        "    background-color: rgb(30, 30, 30);"
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
    buttonGraphics->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    buttonsLayout->addWidget(buttonGraphics);

    // Кнопка плиточной статистики
    QPushButton *buttonStream = new QPushButton;
    buttonStream->setIcon(QIcon(QPixmap(":resources/icons/stream_icon.png")));
    buttonStream->setIconSize(QSize(24, 24));
    buttonStream->setStyleSheet(
        "QPushButton {"
        "    background-color: rgb(30, 30, 30);"
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
    buttonStream->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    buttonsLayout->addWidget(buttonStream);

    // Кнопка оленемера
    QPushButton *buttonPlayers = new QPushButton;
    buttonPlayers->setIcon(QIcon(QPixmap(":resources/icons/player_icon.png")));
    buttonPlayers->setIconSize(QSize(24, 24));
    buttonPlayers->setStyleSheet(
        "QPushButton {"
        "    background-color: rgb(30, 30, 30);"
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
    buttonsLayout->addWidget(buttonPlayers);

    // Кнопка информационной панели
    QPushButton *buttonInfo = new QPushButton;
    buttonInfo->setIcon(QIcon(QPixmap(":resources/icons/info_icon.png")));
    buttonInfo->setIconSize(QSize(24, 24));
    buttonInfo->setStyleSheet(
        "QPushButton {"
        "    background-color: rgb(30, 30, 30);"
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
    buttonsLayout->addWidget(buttonInfo);

    // Кнопка выхода
    QPushButton *buttonExit = new QPushButton;
    buttonExit->setIcon(QIcon(QPixmap(":resources/icons/exit_icon.png")));
    buttonExit->setIconSize(QSize(24, 24));
    buttonExit->setStyleSheet(
        "QPushButton {"
        "    background-color: rgb(30, 30, 30);"
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
    buttonsLayout->addWidget(buttonExit);

    config_panel->setLayout(buttonsLayout);
    mainLayout->addWidget(config_panel);
    setLayout(mainLayout);
}
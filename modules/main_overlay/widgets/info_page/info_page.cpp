#include <main_overlay/widgets/info_page/info_page.h>

InfoPage::InfoPage(ApiController *apiController, QWidget *parent)
    : QWidget(parent), m_apiController(apiController)
{
    setMaximumWidth(325);
    setContentsMargins(10, 2, 10, 2);
    setStyleSheet(
        "border-top-right-radius: 10px;"
        "border-bottom-right-radius: 10px;");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Основной контейнер
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(0, 2, 0, 2);
    mainLayout->setAlignment(Qt::AlignTop);

    // Заголовок
    QHBoxLayout *titleLayout = new QHBoxLayout;
    titleLayout->setSpacing(10);
    titleLayout->setContentsMargins(0, 2, 0, 2);

    QLabel *icon = new QLabel;
    icon->setPixmap(QPixmap(":info_page/resources/icons/arrow_icon.png"));
    titleLayout->addWidget(icon);

    QLabel *title = new QLabel;
    title->setText("STATS OVERLAY v2.0 TEST");
    title->setStyleSheet(
        "font-family: Segoe UI;"
        "font-weight: bold;"
        "font-size: 14px;"
        "color: #e2ded3;");
    title->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    titleLayout->addWidget(title);
    mainLayout->addLayout(titleLayout);

    // Лицензионное соглашение
    QScrollArea *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);

    scroll->setStyleSheet(
        "QScrollArea {"
        "background-color: rgba(50, 50, 50, 80);"
        "border: none;"
        "border-radius: 5px;"
        "}"
        "QScrollBar:vertical {"
        "background: rgba(0, 0, 0, 0);"
        "width: 4px;"
        "margin: 2px 0;"
        "border-radius: 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "background: #555555;"
        "border-radius: 2px;"
        "}"
        "QScrollBar::groove:vertical {"
        "border-radius: 0px;"
        "}"
        "/* Горизонтальный скроллбар */"
        "QScrollBar:horizontal {"
        "background: rgba(0, 0, 0, 0);"
        "height: 4px;"
        "margin: 0 2px;"
        "border-radius: 0px;"
        "}"
        "QScrollBar::handle:horizontal {"
        "background: #555555;"
        "border-radius: 2px;"
        "}"
        "QScrollBar::groove:horizontal {"
        "border-radius: 0px;"
        "}"
        "/* Общие настройки */"
        "QScrollBar::add-line:vertical, "
        "QScrollBar::sub-line:vertical,"
        "QScrollBar::add-line:horizontal, "
        "QScrollBar::sub-line:horizontal {"
        "height: 0;"
        "width: 0;"
        "}");
    scroll->setMaximumHeight(185);

    // Текст лицензионного соглашения
    QLabel *info = new QLabel;
    info->setOpenExternalLinks(true);
    info->setTextFormat(Qt::RichText);
    info->setWordWrap(true);
    info->setText(R"(
<p><b>Лицензионное соглашение</b></p>

<p>Используя приложение, вы соглашаетесь с лицензионным соглашением.</p>

<p>Используется API Леста Игры:<br>
- <a href="https://developers.lesta.ru/documentation/rules/agreement/" style="color: #72d1ff;">Условия использования API</a><br>
- <a href="https://legal.lesta.ru/privacy-policy/" style="color: #72d1ff;">Политика конфиденциальности</a></p>

<p>Благодарности:<br>
- Участникам группы Blitz Hata VЫP за поддержку и идеи<br>
- Леста Игры за предоставленный API<br>
- Сообществу Tanks Blitz за тестирование</p>

<p>Телеграмм-канал автора:<br>
- <a href="https://t.me/tanksblitz_pmods" style="color: #72d1ff;">Telegram</a></p>

<p>Запрещено:<br>
- Коммерческое использование и распространение <br>
- Модификация и декомпиляция кода <br>
- Использование в нарушение правил Леста Игры <br>
- Распространение приложения или его производных без указания автора</p>

<p>Автор не гарантирует 100% точность данных и не несет ответственности за:<br>
- Блокировки аккаунтов (используйте на свой риск)</p>
)");
    info->setStyleSheet(
        "font-family: Times New Roman;"
        "font-size: 14px;"
        "color: #e2ded3;");
    info->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    info->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QWidget *container = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(container);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->addWidget(info);

    scroll->setWidget(container);
    mainLayout->addWidget(scroll);

    int id = QFontDatabase::addApplicationFont(":/info_page/resources/fonts/JetBrainsMono-Bold.ttf");
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);

    QPushButton *donate_button = new QPushButton;
    donate_button->setText("Поддержать автора");
    donate_button->setStyleSheet(QString(
                                     "QPushButton {"
                                     "background-color: rgb(49, 17, 85);"
                                     "color: #e2ded3;"
                                     "border: 1px solid #220033;"
                                     "font-size: 11px;"
                                     "font-family: \"%1\";"
                                     "font-weight: bold;"
                                     "padding: 5px;"
                                     "border-radius: 5px;"
                                     "}"
                                     "QPushButton:hover {"
                                     "background-color: rgb(90, 90, 90);"
                                     "color: #ffffff;"
                                     "}"
                                     "QPushButton:pressed {"
                                     "background-color: rgb(50, 50, 50);"
                                     "color: #cccccc;"
                                     "}")
                                     .arg(family));
    donate_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(donate_button, &QPushButton::clicked, this, &InfoPage::onDonateClicked);
    mainLayout->addWidget(donate_button);

    // Кнопки доната, поддержки, сброса статистики и авторизации
    QPushButton *support_button = new QPushButton;
    support_button->setText("Центр поддержки");
    support_button->setStyleSheet(QString(
                                      "QPushButton {"
                                      "background-color: rgb(70, 70, 70);"
                                      "color: #e2ded3;"
                                      "border: 1px solid #333333;"
                                      "font-size: 11px;"
                                      "font-family: \"%1\";"
                                      "font-weight: bold;"
                                      "padding: 5px;"
                                      "border-radius: 5px;"
                                      "}"
                                      "QPushButton:hover {"
                                      "background-color: rgb(90, 90, 90);"
                                      "color: #ffffff;"
                                      "}"
                                      "QPushButton:pressed {"
                                      "background-color: rgb(50, 50, 50);"
                                      "color: #cccccc;"
                                      "}")
                                      .arg(family));
    support_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(support_button, &QPushButton::clicked, this, &InfoPage::onSupportClicked);
    mainLayout->addWidget(support_button);

    QPushButton *reset_button = new QPushButton;
    reset_button->setText("Сбросить статистику");
    reset_button->setStyleSheet(QString(
                                    "QPushButton {"
                                    "background-color: rgb(70, 70, 70);"
                                    "color: #e2ded3;"
                                    "border: 1px solid #333333;"
                                    "font-size: 11px;"
                                    "font-family: \"%1\";"
                                    "font-weight: bold;"
                                    "padding: 5px;"
                                    "border-radius: 5px;"
                                    "}"
                                    "QPushButton:hover {"
                                    "background-color: rgb(90, 90, 90);"
                                    "color: #ffffff;"
                                    "}"
                                    "QPushButton:pressed {"
                                    "background-color: rgb(50, 50, 50);"
                                    "color: #cccccc;"
                                    "}")
                                    .arg(family));
    reset_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    // connect(reset_button, &QPushButton::clicked, this, &InfoPage::onSupportClicked);
    mainLayout->addWidget(reset_button);

    QPushButton *auth_button = new QPushButton;
    if (isAuth())
    {
        auth_button->setText("Выход");
    }
    else
    {
        auth_button->setText("Авторизоваться");
    }
    auth_button->setStyleSheet(QString(
                                   "QPushButton {"
                                   "background-color: rgb(70, 70, 70);"
                                   "color: #e2ded3;"
                                   "border: 1px solid #333333;"
                                   "font-size: 11px;"
                                   "font-family: \"%1\";"
                                   "font-weight: bold;"
                                   "padding: 5px;"
                                   "border-radius: 5px;"
                                   "}"
                                   "QPushButton:hover {"
                                   "background-color: rgb(90, 90, 90);"
                                   "color: #ffffff;"
                                   "}"
                                   "QPushButton:pressed {"
                                   "background-color: rgb(50, 50, 50);"
                                   "color: #cccccc;"
                                   "}")
                                   .arg(family));
    auth_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(auth_button, &QPushButton::clicked, this, &InfoPage::onAuthClicked);
    setAuthButton(auth_button);
    mainLayout->addWidget(auth_button);

    // Логирование действий
    QLabel *message = new QLabel;
    if (isAuth())
    {
        message->setText(QString::fromStdString(getNickname()));
    }
    else
    {
        message->setText("Пожалуйста, авторизуйтесь.");
    }
    message->setStyleSheet(
        "font-family: Segoe UI;"
        "font-weight: bold;"
        "font-size: 14px;"
        "color: #99ff99;");
    message->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    message->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMessageWidget(message);
    mainLayout->addWidget(message);

    // Сообщение об авторстве
    QLabel *lesta = new QLabel;
    lesta->setText("© Леста Игры. Все права защищены");
    lesta->setStyleSheet(
        "font-family: Segoe UI;"
        "font-weight: bold;"
        "font-size: 10px;"
        "color: #e2ded3;"
        "padding: 5px;");
    lesta->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    lesta->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mainLayout->addWidget(lesta);

    setLayout(mainLayout);
}
#include <main_overlay/widgets/rating_stats/rating_stats.h>

RatingStats::RatingStats(ApiController *apiController, QWidget *parent)
    : m_apiController(apiController), QWidget(parent)
{
    setFixedWidth(260);
    setContentsMargins(0, 2, 0, 2);
    setStyleSheet(
        "border-top-right-radius: 10px;"
        "border-bottom-right-radius: 10px;");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Главный слой для виджета
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(8, 6, 8, 6);

    // Выставление экономики
    addContent(mainLayout, QPixmap(":rating_stats/resources/icons/rating_icon.png"), "Текущий рейтинг", true, false);
    addContent(mainLayout, QPixmap(":rating_stats/resources/icons/progress_icon.png"), "Прогресс рейтинга", false, false);
    addContent(mainLayout, QPixmap(":rating_stats/resources/icons/calib_icon.png"), "Калибровочные бои", false, false);

    QFrame *separator = new QFrame;
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Plain);
    separator->setStyleSheet("background-color: rgba(226, 222, 211, 0.15);");
    separator->setFixedHeight(1);
    mainLayout->addWidget(separator);

    // Слой заголовка боевой статистики
    QHBoxLayout *listsStats = new QHBoxLayout();
    listsStats->setSpacing(5);

    QFrame *accent = new QFrame;
    accent->setFixedWidth(3);
    accent->setStyleSheet("background-color: #c8a84b; border-radius: 1px;");
    listsStats->addWidget(accent);

    QLabel *label = new QLabel;
    label->setText("Боевая эффективность");
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

    // Выставление боевых показателей
    addContent(mainLayout, QPixmap(":main_stats/resources/icons/battles_icon.png"), "Проведено боев", false, false);
    addContent(mainLayout, QPixmap(":main_stats/resources/icons/win_icon.png"), "Победы", false, false);
    addContent(mainLayout, QPixmap(":main_stats/resources/icons/damage_icon.png"), "Урон", false, false);
    addContent(mainLayout, QPixmap(":main_stats/resources/icons/xp_icon.png"), "Опыт", false, true);

    setLayout(mainLayout);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &RatingStats::updatingRatingStats);
    timer->start(30000);
}

void RatingStats::addContent(QVBoxLayout *mainLayout, QPixmap icon_source, QString title, bool is_top, bool is_bottom)
{
    // Контейнер для установки содержимого
    QHBoxLayout *listsStats = new QHBoxLayout();
    listsStats->setSpacing(5);

    // Установка иконки
    QLabel *icon = new QLabel;
    icon->setPixmap(icon_source);
    icon->setMaximumWidth(30);
    icon->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    listsStats->addWidget(icon);

    // Шрифт для содержимого
    int id = QFontDatabase::addApplicationFont(":/main_stats/resources/fonts/JetBrainsMono-Bold.ttf");
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);

    // Установка названия статистики
    QLabel *label = new QLabel;
    label->setText(title);
    label->setMinimumWidth(0);
    label->setTextFormat(Qt::RichText);
    label->setStyleSheet(QString(
                             "font-family: \"%1\";"
                             "font-size: 11px;"
                             "font-weight: bold;"
                             "color: #e2ded3;"
                             "white-space: nowrap;")
                             .arg(family));
    label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    listsStats->addWidget(label);

    // Установка линий
    QFrame *line = new QFrame;
    line->setMinimumWidth(0);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Plain); // вместо Sunken/Styled
    line->setStyleSheet("background-color: #e2ded3;");
    line->setFixedHeight(1); // толщина линии
    line->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    listsStats->addWidget(line);

    // Установка показателей
    QLabel *value = new QLabel;
    value->setText("-");
    value->setMinimumWidth(15);
    value->setTextFormat(Qt::RichText);
    value->setContentsMargins(5, 3, 5, 3);
    value->setStyleSheet(QString(
                             "font-family: \"%1\";"
                             "font-size: 11px;"
                             "font-weight: bold;"
                             "color: #e2ded3;"
                             "white-space: nowrap;"
                             "background-color: #383838;"
                             "border-radius: 5px;")
                             .arg(family));
    value->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    value->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    listsStats->addWidget(value);

    // Установка стрелок
    QLabel *arrow = new QLabel;
    arrow->setText("");
    arrow->setMinimumWidth(0);
    arrow->setTextFormat(Qt::RichText);
    arrow->setStyleSheet(QString(
                             "font-family: \"%1\";"
                             "font-size: 5px;"
                             "font-weight: bold;"
                             "color: #e2ded3;"
                             "white-space: nowrap;")
                             .arg(family));
    arrow->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    arrow->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    listsStats->addWidget(arrow);

    appendInfoLabel(title.toStdString(), value, arrow, line);
    mainLayout->addLayout(listsStats);
}
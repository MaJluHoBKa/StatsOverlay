#include <main_overlay/widgets/other_stats/other_stats.h>

OtherStats::OtherStats(ApiController *apiController, QWidget *parent)
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

    // Выставление медалей
    QWidget *mainMasteryContainer = new QWidget();
    mainMasteryContainer->setStyleSheet(
        "border-radius: 10px;"
        "border: 2px solid rgb(57, 57, 57);");

    QVBoxLayout *masteryLayout = new QVBoxLayout();
    masteryLayout->setSpacing(0);

    QWidget *masteryContainer = new QWidget();
    masteryContainer->setStyleSheet("border: none;");
    QHBoxLayout *medalLayout = new QHBoxLayout();
    medalLayout->setContentsMargins(0, 0, 0, 0);
    medalLayout->setSpacing(0);
    medalLayout->setAlignment(Qt::AlignCenter);

    // Выставление медалей
    addContentMedal(medalLayout, QPixmap(":other_stats/resources/icons/master_icon.png"));
    addContentMedal(medalLayout, QPixmap(":other_stats/resources/icons/master1_icon.png"));
    addContentMedal(medalLayout, QPixmap(":other_stats/resources/icons/master2_icon.png"));
    addContentMedal(medalLayout, QPixmap(":other_stats/resources/icons/master3_icon.png"));
    masteryContainer->setLayout(medalLayout);
    masteryLayout->addWidget(masteryContainer);

    QWidget *masteryValueContainer = new QWidget();
    masteryValueContainer->setStyleSheet("border: none;");
    QHBoxLayout *valueLayout = new QHBoxLayout();
    valueLayout->setContentsMargins(0, 0, 0, 0);
    valueLayout->setSpacing(0);
    valueLayout->setAlignment(Qt::AlignCenter);

    // Высталение значений медалей
    addContentMedalValue(valueLayout, "Mastery_1");
    addContentMedalValue(valueLayout, "Mastery_2");
    addContentMedalValue(valueLayout, "Mastery_3");
    addContentMedalValue(valueLayout, "Mastery_4");
    masteryValueContainer->setLayout(valueLayout);
    masteryLayout->addWidget(masteryValueContainer);

    mainMasteryContainer->setLayout(masteryLayout);
    mainLayout->addWidget(mainMasteryContainer);

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
    label->setText("Боевые показатели");
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
    addContent(mainLayout, QPixmap(":other_stats/resources/icons/percent_damage_icon.png"), "Процент попаданий");
    addContent(mainLayout, QPixmap(":other_stats/resources/icons/percent_leave_icon.png"), "Процент выживания");
    addContent(mainLayout, QPixmap(":other_stats/resources/icons/k_shot_icon.png"), "Коэфф. урона");
    addContent(mainLayout, QPixmap(":other_stats/resources/icons/k_tank_icon.png"), "Коэфф. уничтожения");
    addContent(mainLayout, QPixmap(":other_stats/resources/icons/time_icon.png"), "Ср. время выживания");

    setLayout(mainLayout);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &OtherStats::updatingOtherStats);
    timer->start(30000);
}

void OtherStats::addContentMedal(QHBoxLayout *medalLayout, QPixmap icon_source)
{
    QLabel *iconMedal = new QLabel();
    iconMedal->setPixmap(icon_source);
    iconMedal->setStyleSheet("border: none;");
    medalLayout->addWidget(iconMedal);
}

void OtherStats::addContentMedalValue(QHBoxLayout *valueLayout, QString key)
{
    int id = QFontDatabase::addApplicationFont(":other_stats/resources/fonts/JetBrainsMono-Bold.ttf");
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);

    QLabel *value = new QLabel();
    value->setFixedSize(50, 20);
    value->setText("-");
    value->setAlignment(Qt::AlignCenter);
    value->setStyleSheet(QString(
                             "font-family: \"%1\";"
                             "font-size: 14px;"
                             "font-weight: bold;"
                             "color: #e2ded3;")
                             .arg(family));
    valueLayout->addWidget(value);
    data_master_values[key] = value;
}

void OtherStats::addContent(QVBoxLayout *mainLayout, QPixmap icon_source, QString title)
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
    int id = QFontDatabase::addApplicationFont(":other_stats/resources/fonts/JetBrainsMono-Bold.ttf");
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
                             "font-size: 9px;"
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

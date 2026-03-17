#include <main_overlay/widgets/player_stats/split_players/players_window.h>

PlayerStatsWindow::PlayerStatsWindow(const QString &title, QWidget *parent)
    : QWidget(parent)
{
    setContentsMargins(0, 2, 0, 2);
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet(
        "border-top-right-radius: 10px;"
        "border-bottom-right-radius: 10px;");
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    QRect screenGeometry = QApplication::primaryScreen()->availableGeometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();

    int y = (screenHeight - height()) / 2 - 150;
    int x = (title == "Союзники")
                ? 30
                : screenWidth - 400;
    move(x, y);

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
    label->setText(title);
    label->setTextFormat(Qt::RichText);
    label->setStyleSheet(
        "font-family: Segoe UI;"
        "font-size: 14px;"
        "font-weight: bold;"
        "color: #e2ded3;"
        "white-space: nowrap;");
    label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
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

    QHBoxLayout *header = new QHBoxLayout(header_widget);
    header->setAlignment(Qt::AlignTop);
    header->setSpacing(1);
    header->setContentsMargins(0, 0, 0, 0);

    std::vector<int> sizes = {105, 105, 50, 50, 50};
    QStringList iconPaths = {
        ":players_window/resources/icons/player_icon.png",
        ":players_window/resources/icons/tanks_icon.png",
        ":players_window/resources/icons/battles_icon.png",
        ":players_window/resources/icons/win_icon.png",
        ":players_window/resources/icons/damage_icon.png"};

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
        switch (i)
        {
        case 0:
            this->playerIcon = icon;
            break;
        case 1:
            this->tankIcon = icon;
            break;
        case 2:
            this->battlesIcon = icon;
            break;
        case 3:
            this->winsIcon = icon;
            break;
        case 4:
            this->damageIcon = icon;
            break;
        default:
            break;
        }
    }
    mainLayout->addWidget(header_widget);

    // Создание списка игроков
    data_players = new QVBoxLayout;
    data_players->setSpacing(1);
    data_players->setContentsMargins(0, 0, 0, 0);
    data_players->setAlignment(Qt::AlignTop);

    QWidget *data_widget = new QWidget;
    data_widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    data_widget->setMaximumHeight(180);
    data_widget->setLayout(data_players);
    addRows();

    mainLayout->addWidget(data_widget);

    setLayout(mainLayout);
}

void PlayerStatsWindow::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(QColor(30, 30, 30, int(m_backgroundOpacity * 255)));
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(rect(), 10, 10);
}

void PlayerStatsWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_dragActive = true;
        m_dragStartPos = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void PlayerStatsWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragActive && (event->buttons() & Qt::LeftButton))
    {
        move(event->globalPos() - m_dragStartPos);
        event->accept();
    }
}

void PlayerStatsWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_dragActive = false;
        event->accept();
    }
}
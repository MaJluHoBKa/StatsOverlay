#include <main_overlay/widgets/gun_marks/gunmarks_window/gunmarks_window.h>

GunMarksWindow::GunMarksWindow(QWidget *parent)
    : QWidget(parent)
{
    setFixedWidth(180);
    setContentsMargins(0, 0, 0, 0);
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet(
        "border-top-right-radius: 5px;"
        "border-bottom-right-radius: 5px;");
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    QRect screenGeometry = QApplication::primaryScreen()->availableGeometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();

    int y = (screenHeight - height()) / 2 + 150;
    int x = 30;
    move(x, y);

    // Главный слой для виджета
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(5);

    // Слой заголовка статистики по игрокам
    QHBoxLayout *listsStats = new QHBoxLayout();
    listsStats->setAlignment(Qt::AlignTop);
    listsStats->setSpacing(0);

    percent = new QLabel;
    percent->setText("0.0%");
    percent->setTextFormat(Qt::RichText);
    percent->setStyleSheet(
        "font-family: Segoe UI;"
        "font-size: 18px;"
        "font-weight: bold;"
        "color: #e2ded3;"
        "white-space: nowrap;");
    percent->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    percent->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    percent->setAttribute(Qt::WA_TransparentForMouseEvents);
    listsStats->addWidget(percent);

    QVBoxLayout *inBattleLayout = new QVBoxLayout();
    inBattleLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    QLabel *inBattleText = new QLabel("В бою:");
    inBattleText->setTextFormat(Qt::RichText);
    inBattleText->setStyleSheet(
        "font-family: Segoe UI;"
        "font-size: 12px;"
        "font-weight: normal;"
        "color: #a0a0a0;"
        "white-space: nowrap;");
    inBattleText->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    inBattleText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    inBattleText->setAttribute(Qt::WA_TransparentForMouseEvents);
    inBattleLayout->addWidget(inBattleText);

    inBattle = new QLabel;
    inBattle->setText("0.0%");
    inBattle->setTextFormat(Qt::RichText);
    inBattle->setStyleSheet(
        "font-family: Segoe UI;"
        "font-size: 12px;"
        "font-weight: normal;"
        "color: #a0a0a0;"
        "white-space: nowrap;");
    inBattle->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    inBattle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    inBattle->setAttribute(Qt::WA_TransparentForMouseEvents);
    inBattleLayout->addWidget(inBattle);

    listsStats->addLayout(inBattleLayout);
    mainLayout->addLayout(listsStats);

    progress = new QProgressBar();
    progress->setFixedHeight(5);
    progress->setTextVisible(false);
    progress->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    progress->setRange(0, 100);
    progress->setStyleSheet(R"(
        QProgressBar {
            background-color: rgba(50, 50, 50, 0);
            border: 1px solid #646464;
            border-radius: 1px;
            height: 6px;
        }

        QProgressBar::chunk {
            background-color: #ffffff;
            border-radius: 1px;
        }
    )");

    progress->setValue(0);
    mainLayout->addWidget(progress);

    // Сум урон
    QHBoxLayout *currentSummaryDamage_layout = new QHBoxLayout();
    currentSummaryDamage_layout->setAlignment(Qt::AlignTop);
    currentSummaryDamage_layout->setSpacing(0);

    QLabel *currentSummaryDamage_text = new QLabel("Сум. урон");
    currentSummaryDamage_text->setTextFormat(Qt::RichText);
    currentSummaryDamage_text->setStyleSheet(
        "font-family: Segoe UI;"
        "font-size: 14px;"
        "font-weight: normal;"
        "color: #e2ded3;"
        "white-space: nowrap;");
    currentSummaryDamage_text->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    currentSummaryDamage_text->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    currentSummaryDamage_text->setAttribute(Qt::WA_TransparentForMouseEvents);
    currentSummaryDamage_layout->addWidget(currentSummaryDamage_text);

    currentSummaryDamage = new QLabel("0/0");
    currentSummaryDamage->setTextFormat(Qt::RichText);
    currentSummaryDamage->setStyleSheet(
        "font-family: Segoe UI;"
        "font-size: 14px;"
        "font-weight: bold;"
        "color: #e2ded3;"
        "white-space: nowrap;");
    currentSummaryDamage->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    currentSummaryDamage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    currentSummaryDamage->setAttribute(Qt::WA_TransparentForMouseEvents);
    currentSummaryDamage_layout->addWidget(currentSummaryDamage);
    mainLayout->addLayout(currentSummaryDamage_layout);

    // Ср урон
    QHBoxLayout *reqAvgDamage_layout = new QHBoxLayout();
    reqAvgDamage_layout->setAlignment(Qt::AlignTop);
    reqAvgDamage_layout->setSpacing(0);

    QLabel *reqAvgDamage_text = new QLabel("Треб. урон");
    reqAvgDamage_text->setTextFormat(Qt::RichText);
    reqAvgDamage_text->setStyleSheet(
        "font-family: Segoe UI;"
        "font-size: 14px;"
        "font-weight: normal;"
        "color: #e2ded3;"
        "white-space: nowrap;");
    reqAvgDamage_text->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    reqAvgDamage_text->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    reqAvgDamage_text->setAttribute(Qt::WA_TransparentForMouseEvents);
    reqAvgDamage_layout->addWidget(reqAvgDamage_text);

    reqAvgDamage = new QLabel("0");
    reqAvgDamage->setTextFormat(Qt::RichText);
    reqAvgDamage->setStyleSheet(
        "font-family: Segoe UI;"
        "font-size: 14px;"
        "font-weight: bold;"
        "color: #e2ded3;"
        "white-space: nowrap;");
    reqAvgDamage->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    reqAvgDamage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    reqAvgDamage->setAttribute(Qt::WA_TransparentForMouseEvents);
    reqAvgDamage_layout->addWidget(reqAvgDamage);
    mainLayout->addLayout(reqAvgDamage_layout);

    // +0.5%
    QHBoxLayout *next05_layout = new QHBoxLayout();
    next05_layout->setAlignment(Qt::AlignTop);
    next05_layout->setSpacing(0);

    next05_text = new QLabel("+0.5%");
    next05_text->setTextFormat(Qt::RichText);
    next05_text->setStyleSheet(
        "font-family: Segoe UI;"
        "font-size: 12px;"
        "font-weight: normal;"
        "color: #e2ded3;"
        "white-space: nowrap;");
    next05_text->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    next05_text->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    next05_text->setAttribute(Qt::WA_TransparentForMouseEvents);
    next05_layout->addWidget(next05_text);

    next05 = new QLabel("0");
    next05->setTextFormat(Qt::RichText);
    next05->setStyleSheet(
        "font-family: Segoe UI;"
        "font-size: 12px;"
        "font-weight: bold;"
        "color: #e2ded3;"
        "white-space: nowrap;");
    next05->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    next05->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    next05->setAttribute(Qt::WA_TransparentForMouseEvents);
    next05_layout->addWidget(next05);
    mainLayout->addLayout(next05_layout);

    // +1%
    QHBoxLayout *next1_layout = new QHBoxLayout();
    next1_layout->setAlignment(Qt::AlignTop);
    next1_layout->setSpacing(0);

    next1_text = new QLabel("+1%");
    next1_text->setTextFormat(Qt::RichText);
    next1_text->setStyleSheet(
        "font-family: Segoe UI;"
        "font-size: 12px;"
        "font-weight: normal;"
        "color: #e2ded3;"
        "white-space: nowrap;");
    next1_text->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    next1_text->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    next1_text->setAttribute(Qt::WA_TransparentForMouseEvents);
    next1_layout->addWidget(next1_text);

    next1 = new QLabel("0");
    next1->setTextFormat(Qt::RichText);
    next1->setStyleSheet(
        "font-family: Segoe UI;"
        "font-size: 12px;"
        "font-weight: bold;"
        "color: #e2ded3;"
        "white-space: nowrap;");
    next1->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    next1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    next1->setAttribute(Qt::WA_TransparentForMouseEvents);
    next1_layout->addWidget(next1);
    mainLayout->addLayout(next1_layout);

    // +2%
    QHBoxLayout *next2_layout = new QHBoxLayout();
    next2_layout->setAlignment(Qt::AlignTop);
    next2_layout->setSpacing(0);

    next2_text = new QLabel("+2%");
    next2_text->setTextFormat(Qt::RichText);
    next2_text->setStyleSheet(
        "font-family: Segoe UI;"
        "font-size: 12px;"
        "font-weight: normal;"
        "color: #e2ded3;"
        "white-space: nowrap;");
    next2_text->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    next2_text->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    next2_text->setAttribute(Qt::WA_TransparentForMouseEvents);
    next2_layout->addWidget(next2_text);

    next2 = new QLabel("0");
    next2->setTextFormat(Qt::RichText);
    next2->setStyleSheet(
        "font-family: Segoe UI;"
        "font-size: 12px;"
        "font-weight: bold;"
        "color: #e2ded3;"
        "white-space: nowrap;");
    next2->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    next2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    next2->setAttribute(Qt::WA_TransparentForMouseEvents);
    next2_layout->addWidget(next2);
    mainLayout->addLayout(next2_layout);

    setLayout(mainLayout);
    adjustSize();
}

void GunMarksWindow::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(QColor(30, 30, 30, int(m_backgroundOpacity * 255)));
    QPen pen(Qt::white);
    pen.setWidth(1);
    p.setPen(pen);
    p.drawRoundedRect(rect(), 5, 5);
}

void GunMarksWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_dragActive = true;
        m_dragStartPos = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void GunMarksWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragActive && (event->buttons() & Qt::LeftButton))
    {
        move(event->globalPos() - m_dragStartPos);
        event->accept();
    }
}

void GunMarksWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_dragActive = false;
        event->accept();
    }
}
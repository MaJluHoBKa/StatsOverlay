#include <main_overlay/widgets/widgets_page/dmg_window/dmg.h>

DamageWidget::DamageWidget(QWidget *parent)
    : QWidget(parent)
{
    setMaximumWidth(75);
    setContentsMargins(0, 0, 0, 0);
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::WindowTransparentForInput | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    QRect screenGeometry = QApplication::primaryScreen()->availableGeometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();

    int x = 0;
    int y = screenHeight - height() - 200;
    move(x, y);

    // Главный слой для виджета
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(5, 5, 5, 5);

    // Урон
    QHBoxLayout *damage_layout = new QHBoxLayout();
    damage_layout->setAlignment(Qt::AlignTop);
    damage_layout->setSpacing(0);

    QLabel *damage_icon = new QLabel;
    damage_icon->setPixmap(QPixmap(":dmg/resources/icons/total_damage_icon.png"));
    damage_icon->setMaximumWidth(30);
    damage_icon->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    damage_layout->addWidget(damage_icon);

    damage = new QLabel("0");
    damage->setTextFormat(Qt::RichText);
    damage->setStyleSheet(
        "font-family: Segoe UI;"
        "font-size: 14px;"
        "font-weight: bold;"
        "color: #e2ded3;"
        "white-space: nowrap;");
    damage->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    damage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    damage->setAttribute(Qt::WA_TransparentForMouseEvents);
    damage_layout->addWidget(damage);
    mainLayout->addLayout(damage_layout);

    // Блок
    QHBoxLayout *block_layout = new QHBoxLayout();
    block_layout->setAlignment(Qt::AlignTop);
    block_layout->setSpacing(0);

    QLabel *block_icon = new QLabel;
    block_icon->setPixmap(QPixmap(":dmg/resources/icons/block_icon.png"));
    block_icon->setMaximumWidth(30);
    block_icon->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    block_layout->addWidget(block_icon);

    block = new QLabel("0");
    block->setTextFormat(Qt::RichText);
    block->setStyleSheet(
        "font-family: Segoe UI;"
        "font-size: 14px;"
        "font-weight: bold;"
        "color: #e2ded3;"
        "white-space: nowrap;");
    block->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    block->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    block->setAttribute(Qt::WA_TransparentForMouseEvents);
    block_layout->addWidget(block);
    mainLayout->addLayout(block_layout);

    // Ассист
    QHBoxLayout *assist_layout = new QHBoxLayout();
    assist_layout->setAlignment(Qt::AlignTop);
    assist_layout->setSpacing(0);

    QLabel *assist_icon = new QLabel;
    assist_icon->setPixmap(QPixmap(":dmg/resources/icons/assist_icon.png"));
    assist_icon->setMaximumWidth(30);
    assist_icon->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    assist_layout->addWidget(assist_icon);

    assist = new QLabel("0");
    assist->setTextFormat(Qt::RichText);
    assist->setStyleSheet(
        "font-family: Segoe UI;"
        "font-size: 14px;"
        "font-weight: bold;"
        "color: #e2ded3;"
        "white-space: nowrap;");
    assist->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    assist->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    assist->setAttribute(Qt::WA_TransparentForMouseEvents);
    assist_layout->addWidget(assist);
    mainLayout->addLayout(assist_layout);

    setLayout(mainLayout);
}

void DamageWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Основной градиент: слева непрозрачный → справа прозрачный
    QLinearGradient grad(0, 0, width(), 0);
    grad.setColorAt(0.0, QColor(35, 35, 35, 204)); // 80% непрозрачность
    grad.setColorAt(0.6, QColor(35, 35, 35, 120));
    grad.setColorAt(1.0, QColor(35, 35, 35, 0));
    p.fillRect(rect(), grad);
}
#include <main_overlay/widgets/gun_marks/gun_marks.h>

// class GlobalHotkeyFilterPlayers : public QAbstractNativeEventFilter
// {
// public:
//     QWidget *overlay;

//     GlobalHotkeyFilterPlayers(QWidget *overlay) : overlay(overlay) {}

//     bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override
//     {
//         MSG *msg = static_cast<MSG *>(message);
//         if (msg->message == WM_HOTKEY)
//         {
//             PlayerStats *sub = qobject_cast<PlayerStats *>(overlay);
//             if (!sub)
//                 return false;

//             if (msg->wParam == 4) // Ctrl + UP
//             {
//                 sub->hotHide();
//             }
//         }
//         return false;
//     }
// };

GunMarks::GunMarks(ApiController *apiController, QWidget *parent)
    : m_apiController(apiController), QWidget(parent)
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

    // Слой заголовка статистики по игрокам
    QHBoxLayout *listsStats = new QHBoxLayout();
    listsStats->setAlignment(Qt::AlignTop);
    listsStats->setSpacing(5);

    QFrame *accent = new QFrame;
    accent->setFixedWidth(3);
    accent->setStyleSheet("background-color: #c8a84b; border-radius: 1px;");
    listsStats->addWidget(accent);

    QLabel *label = new QLabel;
    label->setText("Отслеживание отметки");
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

    // Танк
    QWidget *tankLock_widget = new QWidget();
    tankLock_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QHBoxLayout *tankLock_layout = new QHBoxLayout(tankLock_widget);
    tankLock_layout->setContentsMargins(0, 0, 0, 0);
    tankLock_layout->setSpacing(5);

    // Tank Name
    tl = new QLabel(tankLock_widget);
    tl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    tl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    tl->setStyleSheet(R"(
        color: #e2ded3;
        font-weight: bold;
        font-size: 14px;
        font-family: Segoe UI;
        padding-right: 5px;
        border: none;
    )");

    // Текст слева
    tl_label = new QLabel(tankLock_widget);
    QIcon lockIcon(":gun_marks/resources/icons/unlock_icon.svg");
    tl_label->setPixmap(lockIcon.pixmap(16, 16));
    tl_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    tl_label->setStyleSheet("color: #e2ded3; font-weight: bold; font-size: 13px; padding-left: 5px");

    tankLock_layout->addWidget(tl_label);
    tankLock_layout->addWidget(tl);
    tankLock_widget->setLayout(tankLock_layout);
    mainLayout->addWidget(tankLock_widget);

    auto createButtonCheckbox = [](const QString &text, QLabel *&checkPtr)
    {
        QPushButton *btn = new QPushButton;
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        btn->setStyleSheet(R"(
        QPushButton {
            border: none;
            border-radius: 5px;
            padding: 5px 10px;
            color: #e2ded3;
            font-family: Segoe UI;
            font-weight: bold;
            font-size: 13px;
            text-align: right;
        }
        QPushButton:hover {
            background-color: rgb(50, 50, 50);
        }
    )");

        // Контейнер внутри кнопки для галочки и текста
        QWidget *container = new QWidget(btn);
        container->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        QHBoxLayout *layout = new QHBoxLayout(container);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(5);

        // Галочка справа
        QLabel *check = new QLabel("✔", container);
        check->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        check->setStyleSheet("color: #e2ded3; font-weight: bold; font-size: 13px; padding-right: 5px");
        checkPtr = check;

        // Текст слева
        QLabel *label = new QLabel(text, container);
        label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        label->setStyleSheet("color: #e2ded3; font-weight: bold; font-size: 13px; padding-left: 5px");

        layout->addWidget(label);
        layout->addWidget(check);
        container->setLayout(layout);

        QVBoxLayout *btnLayout = new QVBoxLayout(btn);
        btnLayout->setContentsMargins(0, 0, 0, 0);
        btnLayout->addWidget(container);

        return btn;
    };

    nextPercent = createButtonCheckbox("Урон для +0.5/1/2%", nextCheck);
    countShots = createButtonCheckbox("Требуемый урон", shotsCheck);

    connect(nextPercent, &QPushButton::clicked, this, &GunMarks::toggleNext);
    connect(countShots, &QPushButton::clicked, this, &GunMarks::toggleShots);

    mainLayout->addWidget(nextPercent);
    mainLayout->addWidget(countShots);

    // Планка
    QWidget *tfreq = new QWidget();
    tfreq->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QHBoxLayout *tfreq_layout = new QHBoxLayout(tfreq);
    tfreq_layout->setContentsMargins(0, 0, 0, 0);
    tfreq_layout->setSpacing(5);

    // Текстовое поле ввода
    tf = new QLineEdit(tfreq);
    tf->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    tf->setPlaceholderText("0000");
    tf->setValidator(new QIntValidator(0, 9999, tf));
    tf->setMaxLength(4);
    tf->setStyleSheet(R"(
        QLineEdit {
            color: #e2ded3;
            font-weight: bold;
            font-size: 14px;
            font-family: Segoe UI;
            padding-right: 5px;
            background-color: rgba(50, 50, 50, 80);
            border-radius: 5px;
            border: none;
        }
        QLineEdit::placeholder {    
            color: rgba(226, 222, 211, 120);
            font-weight: bold;
        }
    )");

    // Текст слева
    QLabel *tfreq_label = new QLabel("Планка отметки", tfreq);
    tfreq_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    tfreq_label->setStyleSheet("color: #e2ded3; font-weight: bold; font-size: 13px; padding-left: 5px");

    tfreq_layout->addWidget(tfreq_label);
    tfreq_layout->addWidget(tf);
    tfreq->setLayout(tfreq_layout);
    mainLayout->addWidget(tfreq);

    // Бои
    QWidget *breq = new QWidget();
    breq->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QHBoxLayout *breq_layout = new QHBoxLayout(breq);
    breq_layout->setContentsMargins(0, 0, 0, 0);
    breq_layout->setSpacing(5);

    // Текстовое поле ввода
    btf = new QLineEdit(breq);
    btf->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    btf->setPlaceholderText("0000");
    btf->setValidator(new QIntValidator(0, 9999, btf));
    btf->setMaxLength(4);
    btf->setStyleSheet(R"(
        QLineEdit {
            color: #e2ded3;
            font-weight: bold;
            font-size: 14px;
            font-family: Segoe UI;
            padding-right: 5px;
            background-color: rgba(50, 50, 50, 80);
            border-radius: 5px;
            border: none;
        }
        QLineEdit::placeholder {    
            color: rgba(226, 222, 211, 120);
            font-weight: bold;
        }
    )");

    // Текст слева
    QLabel *breq_label = new QLabel("Текущий ср. урон", breq);
    breq_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    breq_label->setStyleSheet("color: #e2ded3; font-weight: bold; font-size: 13px; padding-left: 5px");

    breq_layout->addWidget(breq_label);
    breq_layout->addWidget(btf);
    breq->setLayout(breq_layout);
    mainLayout->addWidget(breq);

    QPushButton *switchMarks = new QPushButton;
    switchMarks->setText("Начать");
    switchMarks->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    switchMarks->setStyleSheet(
        "QPushButton {"
        "        background-color: rgb(50, 50, 50);"
        "        color: #e2ded3;"
        "        font-size: 14px;"
        "        font-family: Segoe UI;"
        "        font-weight: bold;"
        "        padding: 5px;"
        "        border-radius: 5px;"
        "    }"
        "    QPushButton:hover {"
        "        background-color: #4cd964;"
        "        color: #383838;"
        "    }"
        "    QPushButton:pressed {"
        "        background-color: #3cbf5f;"
        "        color: #383838;"
        "    }");
    connect(switchMarks, &QPushButton::clicked, this, [this]()
            { setIsOn(); });
    setButtonActive(switchMarks);
    mainLayout->addWidget(switchMarks);

    auto createOpacitySlider = [this]() -> QWidget *
    {
        QWidget *container = new QWidget;
        container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        QHBoxLayout *layout = new QHBoxLayout(container);
        layout->setContentsMargins(5, 0, 5, 0);
        layout->setSpacing(5);

        // Метка слева
        QLabel *label = new QLabel("Прозрачность", container);
        label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        label->setStyleSheet("color: #e2ded3; font-weight: bold; font-size: 13px; padding-right: 5px;");
        layout->addWidget(label);

        // Слайдер справа
        QSlider *slider = new QSlider(Qt::Horizontal, container);
        slider->setRange(10, 100);
        slider->setValue(int(m_backgroundOpacity * 100));
        slider->setStyleSheet(R"(
            QSlider::groove:horizontal {
                border: 1px solid #444;
                height: 4px;
                background: #333;
                border-radius: 2px;
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
        layout->addWidget(slider);

        // Событие изменения прозрачности
        connect(slider, &QSlider::valueChanged, this, [this](int value)
                {
        m_backgroundOpacity = value / 100.0;

        this->gunMarksWindow->setOpacity(m_backgroundOpacity);
        this->gunMarksWindow->update();

        QSettings settings("MyCompany", "StatsOverlay");
        settings.setValue("backgroundOpacityGunMarks", m_backgroundOpacity); });

        return container;
    };
    mainLayout->addWidget(createOpacitySlider());

    setLayout(mainLayout);

    // RegisterHotKey(NULL, 4, MOD_CONTROL, VK_OEM_MINUS);
    // auto *hotkeyFilter = new GlobalHotkeyFilterPlayers(this);
    // qApp->installNativeEventFilter(hotkeyFilter);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &GunMarks::updatingMarkStats);
    timer->start(10000);
}

void GunMarks::toggleNext()
{
    QMetaObject::invokeMethod(
        this,
        [this]()
        {
            if (is_next)
            {
                nextCheck->setText("");
                this->gunMarksWindow->toggleNext(false);
                // this->enemiesWindow->togglePlayers(false);
            }
            else
            {
                nextCheck->setText("✔");
                this->gunMarksWindow->toggleNext(true);
                // this->enemiesWindow->togglePlayers(true);
            }
            is_next = !is_next;
        },
        Qt::QueuedConnection);
}

void GunMarks::toggleShots()
{
    QMetaObject::invokeMethod(
        this,
        [this]()
        {
            if (is_shots)
            {
                shotsCheck->setText("");
                // this->alliesWindow->toggleTanks(false);
                // this->enemiesWindow->toggleTanks(false);
            }
            else
            {
                shotsCheck->setText("✔");
                // this->alliesWindow->toggleTanks(true);
                // this->enemiesWindow->toggleTanks(true);
            }
            is_shots = !is_shots;
        },
        Qt::QueuedConnection);
}

void GunMarks::updatingMarkStats()
{
    QMetaObject::invokeMethod(
        this,
        [this]()
        {
            if (!this->isOn)
            {
                return;
            }

            if (!isAuth())
                return;

            QtConcurrent::run([this]()
                              {
                    if(this->isMarks)
                    {
                        // результат надо передать в GUI-поток
                        QMetaObject::invokeMethod(this, [this]()
                        {
                            int64_t damage = 0;
                            for(int64_t i : this->damages)
                            {
                                damage += i;
                            }
                            this->gunMarksWindow->setData(damage, this->mark, this->damages[0]);
                        }, Qt::QueuedConnection);
                    }
                    else
                    {
                        QMetaObject::invokeMethod(this, [this]()
                        {
                        }, Qt::QueuedConnection);
                    } });
        },
        Qt::QueuedConnection);
}
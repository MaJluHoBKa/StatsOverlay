#include <sub_overlay/sub_overlay.h>
#include <main_overlay/main_overlay.h>
#include <QAbstractNativeEventFilter>
#include <windows.h>

class GlobalHotkeyFilterSub : public QAbstractNativeEventFilter
{
public:
    QWidget *overlay;

    GlobalHotkeyFilterSub(QWidget *overlay) : overlay(overlay) {}

    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override
    {
        MSG *msg = static_cast<MSG *>(message);
        if (msg->message == WM_HOTKEY)
        {
            if (msg->wParam == 1)
            {
                SubOverlay *sub = qobject_cast<SubOverlay *>(overlay);
                if (sub)
                    sub->mainHide();
            }
        }
        return false;
    }
};

SubOverlay::SubOverlay(QWidget *parent) : QWidget(parent)
{
    QApplication::setStyle("Fusion");
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    resize(45, 45);
    move(0, (QApplication::primaryScreen()->availableGeometry().height() - 40) / 2);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(5, 5, 5, 5);

    QLabel *icon = new QLabel;

    icon->setPixmap(QPixmap(":sub/resources/icons/mini_stats_icon.png"));
    icon->setAlignment(Qt::AlignCenter);
    icon->setStyleSheet(
        "border: 2px solid #808080;"
        "border-radius: 10px;"
        "background-color: rgb(30, 30, 30);");
    layout->addWidget(icon);
    setLayout(layout);
    setMouseTracking(true);

    RegisterHotKey(NULL, 1, MOD_CONTROL, 0xBB);
    auto *hotkeyFilter = new GlobalHotkeyFilterSub(this);
    qApp->installNativeEventFilter(hotkeyFilter);
}

void SubOverlay::setMainWidget(MainOverlay *mainOverlay)
{
    this->mainOverlay = mainOverlay;
}

void SubOverlay::mainHide()
{
    if (mainOverlay != nullptr)
    {
        if (isMainVisible())
        {
            mainOverlay->hide();
            setMainVisible(false);
        }
        else
        {
            mainOverlay->show();
            setMainVisible(true);
        }
    }
}

void SubOverlay::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        setDragActive(true);
        setGlobalPos(event->globalPos());
    }

    if (event->button() == Qt::LeftButton)
    {
        mainHide();
    }
}

void SubOverlay::mouseMoveEvent(QMouseEvent *event)
{
    if (isDragActive())
    {
        QPoint delta = event->globalPos() - getGlobalPos();
        int new_x = x() + delta.x();
        int new_y = y() + delta.y();
        move(new_x, new_y);
        setGlobalPos(event->globalPos());
    }
}

void SubOverlay::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        setDragActive(false);
        snapToClosestScreenEdge();
    }
}

void SubOverlay::snapToClosestScreenEdge()
{
    QRect screen = QApplication::primaryScreen()->availableGeometry();
    int margin = 10;

    int new_x = x();
    int new_y = y();

    int distance_left = abs(new_x - screen.left());
    int distance_right = abs(new_x + width() - screen.right());
    int distance_top = abs(new_y + screen.top());
    int distance_bottom = abs(new_y + height() + screen.bottom());

    if (distance_left < distance_right && distance_left < distance_top && distance_left < distance_bottom)
    {
        new_x = screen.left();
    }
    else if (distance_right < distance_left && distance_right < distance_top && distance_right < distance_bottom)
    {
        new_x = screen.right() - width();
    }
    else if (distance_top < distance_left && distance_top < distance_right && distance_top < distance_bottom)
    {
        new_y = screen.top();
    }
    else if (distance_bottom < distance_left && distance_bottom < distance_right && distance_bottom < distance_top)
    {
        new_y = screen.bottom() - height();
    }

    move(new_x, new_y);
}

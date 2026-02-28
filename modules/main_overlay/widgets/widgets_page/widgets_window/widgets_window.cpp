#include <main_overlay/widgets/widgets_page/widgets_window/widgets_window.h>
#include <main_overlay/widgets/widgets_page/widgets_window/FlowLayout.h>

WidgetsWindow::WidgetsWindow(QWidget *parent)
    : QWidget(parent)
{
    setFixedWidth(190);
    setContentsMargins(0, 0, 0, 0);
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet(
        "border-top-right-radius: 5px;"
        "border-bottom-right-radius: 5px;");

    QRect screenGeometry = QApplication::primaryScreen()->availableGeometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();
    int x = 30;

    // Главный слой для виджета
    flowLayout = new FlowLayout(this, 8, 5, 5);

    addTile("Бои");
    addTile("Победы");
    addTile("Урон");
    addTile("Опыт");
    addTile("Рейтинг");
    addTile("Мастер");
    addTile("Выжил");
    addTile("Точность");

    setLayout(flowLayout);
    adjustSize();
    int y = (screenHeight - height()) / 2 + 100;
    move(x, y);
}

void WidgetsWindow::paintEvent(QPaintEvent *)
{
}

void WidgetsWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_dragActive = true;
        m_dragStartPos = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void WidgetsWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragActive && (event->buttons() & Qt::LeftButton))
    {
        move(event->globalPos() - m_dragStartPos);
        event->accept();
    }
}

void WidgetsWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_dragActive = false;
        event->accept();
    }
}
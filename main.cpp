#include <QApplication>
#include <QLabel>

#include <main_overlay/main_overlay.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainOverlay mainOverlay;
    mainOverlay.show();
    return app.exec();
}

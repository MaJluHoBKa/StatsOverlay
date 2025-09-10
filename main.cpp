#include <QApplication>

#include <main_overlay/main_overlay.h>
#include <sub_overlay/sub_overlay.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainOverlay mainOverlay;
    mainOverlay.show();

    SubOverlay subOverlay;
    subOverlay.setMainWidget(&mainOverlay);
    subOverlay.show();

    return app.exec();
}

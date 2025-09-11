#include <QApplication>

#include <main_overlay/main_overlay.h>
#include <sub_overlay/sub_overlay.h>

#include <main_overlay/controller/ApiController.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    ApiController *apiController = new ApiController;

    MainOverlay mainOverlay(apiController);
    mainOverlay.show();

    SubOverlay subOverlay;
    subOverlay.setMainWidget(&mainOverlay);
    subOverlay.show();

    return app.exec();
}

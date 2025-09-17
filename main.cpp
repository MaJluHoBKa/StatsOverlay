#include <QApplication>
#include <QMessageBox>
#include <QDate>

#include <main_overlay/main_overlay.h>
#include <sub_overlay/sub_overlay.h>

#include <main_overlay/controller/ApiController.h>

int main(int argc, char *argv[])
{
    // Проверка даты
    QDate currentDate = QDate::currentDate();
    QDate expiryDate(2025, 9, 21); // например, версия истекает 30 сентября 2025

    if (currentDate > expiryDate)
    {
        QMessageBox::critical(nullptr, "Обновление требуется",
                              "Срок действия этой версии истёк. Пожалуйста, обновите приложение.");
        return 0; // выход из приложения
    }

    QApplication app(argc, argv);

    ApiController *apiController = new ApiController;
    apiController->initLog();

    MainOverlay mainOverlay(apiController);
    mainOverlay.show();

    SubOverlay subOverlay;
    subOverlay.setMainWidget(&mainOverlay);
    subOverlay.show();

    return app.exec();
}

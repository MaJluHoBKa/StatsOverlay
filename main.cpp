#include <QApplication>
#include <QMessageBox>
#include <QDate>
#include <QCryptographicHash>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QClipboard>
#include <QTimer>

#include <main_overlay/activate/activation_window.h>
#include <main_overlay/main_overlay.h>
#include <sub_overlay/sub_overlay.h>

#include <main_overlay/controller/ApiController.h>

QByteArray xorEncryptDecrypt(const QByteArray &data, const QByteArray &key)
{
    QByteArray result;
    result.resize(data.size());

    for (int i = 0; i < data.size(); ++i)
    {
        result[i] = data[i] ^ key[i % key.size()];
    }
    return result;
}

bool loadAuth(ApiController *apiController)
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/auth.json";
    QFile file(path);
    if (!file.exists())
    {
        qDebug() << "Файл авторизации не найден:" << path;
        return false;
    }
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Не удалось открыть файл:" << file.errorString();
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (doc.isNull())
    {
        qWarning() << "Ошибка парсинга JSON:" << parseError.errorString();
        return false;
    }

    QJsonObject obj = doc.object();
    QByteArray key = "speaker";

    QString token = QString::fromUtf8(xorEncryptDecrypt(QByteArray::fromBase64(obj["token"].toString().toUtf8()), key));
    QString nickname = QString::fromUtf8(xorEncryptDecrypt(QByteArray::fromBase64(obj["nickname"].toString().toUtf8()), key));
    QString account_id = QString::fromUtf8(xorEncryptDecrypt(QByteArray::fromBase64(obj["account_id"].toString().toUtf8()), key));

    if (token.isEmpty() || account_id.isEmpty())
    {
        qWarning() << "Файл auth.json повреждён или пуст";
        return false;
    }

    apiController->setToken(token.toStdString());
    apiController->setNickname(nickname.toStdString());
    apiController->setAccountId(account_id.toStdString());
    apiController->setAuth(true);

    return true;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    bool activated = HWIDActivator::checkActivation();
    if (!activated)
    {
        ActivationWindow win;
        if (win.exec() != QDialog::Accepted) // если крестик или неверный ключ
            return 0;
    }

    QDate currentDate = QDate::currentDate();
    QDate expiryDate(2026, 3, 31);
    if (currentDate > expiryDate)
    {
        QMessageBox::critical(nullptr, "Обновление требуется",
                              "Срок действия этой версии истёк. Пожалуйста, обновите приложение.");
        return 0;
    }

    qApp->setStyleSheet(R"(
        QToolTip {
            background-color: rgb(40, 40, 40);
            color: #e2ded3;
            border: 1px solid #555;
            border-radius: 5px;
            padding: 2px 4px;
            font-family: Segoe UI;
            font-size: 9px;
        }
    )");

    ApiController *apiController = new ApiController;
    apiController->initLog();
    loadAuth(apiController);
    if (apiController->is_auth())
    {
        if (!apiController->prolongate())
        {
            apiController->logout();
            // 📂 Путь: %AppData%/StatsOverlay
            QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            QDir dir(appData);
            if (dir.exists())
            {
                QString authPath = dir.filePath("auth.json");
                if (QFile::exists(authPath))
                {
                    QFile::remove(authPath);
                }
            }
        }
    }

    MainOverlay mainOverlay(apiController);
    mainOverlay.show();

    SubOverlay subOverlay;
    subOverlay.setMainWidget(&mainOverlay);
    subOverlay.show();

    return app.exec();
}

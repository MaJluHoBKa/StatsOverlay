#pragma once

#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QScreen>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSizePolicy>
#include <QPainter>
#include <QStackedWidget>
#include <QMouseEvent>
#include <QMenu>
#include <QSlider>
#include <QAction>
#include <QCheckBox>
#include <QWidgetAction>
#include <QContextMenuEvent>
#include <QPropertyAnimation>
#include <QCryptographicHash>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QPainterPath>
#include <QIntValidator>
#include <QGraphicsOpacityEffect>
#include <main_overlay/widgets/main_stats/main_stats.h>
#include <main_overlay/widgets/rating_stats/rating_stats.h>
#include <main_overlay/widgets/other_stats/other_stats.h>
#include <main_overlay/widgets/vehicles_stats/vehicles_stats.h>
#include <main_overlay/widgets/player_stats/player_stats.h>
#include <main_overlay/widgets/gun_marks/gun_marks.h>
#include <main_overlay/widgets/info_page/info_page.h>
#include <main_overlay/controller/ApiController.h>
#include <../modules/sub_overlay/sub_overlay.h>
#include <windows.h>

class MainOverlay : public QWidget
{
    Q_OBJECT

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void enterEvent(QEnterEvent *event) override
    {
        QWidget::enterEvent(event);
        showConfigPanel(true);
    }

    void leaveEvent(QEvent *event) override
    {
        QWidget::leaveEvent(event);
        if (!this->underMouse())
        {
            showConfigPanel(false);
        }
    }

private:
    QStackedWidget *stacked_widget = nullptr;
    GunMarks *gunMark = nullptr;
    PlayerStats *players = nullptr;
    VehicleStats *vehicle = nullptr;
    InfoPage *info = nullptr;
    ApiController *m_apiController = nullptr;

    QWidget *configPanel;
    double m_backgroundOpacity = 1.0;

    bool m_dragActive = false;
    QPoint m_dragStartPos;
    int m_currentIndexPage = 5;
    int configPanelNormalWidth = 30;

    std::vector<QPushButton *> m_navButtons;
    void updateNavButtonStyles(int activePage);

    int defaultWidth = 280;

public:
    explicit MainOverlay(ApiController *apiController, QWidget *parent = nullptr);

    void setStackedWidget(QStackedWidget *stacked_widget)
    {
        if (this->stacked_widget == nullptr)
        {
            this->stacked_widget = stacked_widget;
        }
    }

    QStackedWidget *getStackedWidget() const
    {
        return this->stacked_widget;
    }

    void setPlayerPage(PlayerStats *players)
    {
        if (this->players == nullptr)
        {
            this->players = players;
        }
    }

    void setVehiclePage(VehicleStats *vehicle)
    {
        if (this->vehicle == nullptr)
        {
            this->vehicle = vehicle;
        }
    }

    void setInfoPage(InfoPage *info)
    {
        if (this->info == nullptr)
        {
            this->info = info;
        }
    }

    void logout()
    {
        if (this->m_apiController->is_auth())
        {
            QString token = QString::fromStdString(this->m_apiController->getToken());
            QString nickname = QString::fromStdString(this->m_apiController->getNickname());
            QString account_id = QString::fromStdString(this->m_apiController->getAccountId());

            QByteArray key = "speaker"; // 🔑 можно вынести в конфиг

            // Шифруем XOR + кодируем в Base64
            QByteArray tokenEnc = xorEncryptDecrypt(token.toUtf8(), key).toBase64();
            QByteArray nickEnc = xorEncryptDecrypt(nickname.toUtf8(), key).toBase64();
            QByteArray accIdEnc = xorEncryptDecrypt(account_id.toUtf8(), key).toBase64();

            // JSON
            QJsonObject obj;
            obj["token"] = QString(tokenEnc);
            obj["nickname"] = QString(nickEnc);
            obj["account_id"] = QString(accIdEnc);

            QJsonDocument doc(obj);

            // 📂 Путь: %AppData%/StatsOverlay
            QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            QDir dir(appData);
            if (!dir.exists())
            {
                if (!dir.mkpath("."))
                {
                    qWarning() << "Не удалось создать папку StatsOverlay в AppData!";
                    return;
                }
            }

            QFile file(dir.filePath("auth.json"));
            if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
            {
                qWarning() << "Не удалось открыть auth.json для записи!";
                return;
            }

            file.write(doc.toJson(QJsonDocument::Indented));
            file.close();
        }
        QApplication::quit();
    }

    void nextHotPage()
    {
        if (m_currentIndexPage > 0)
            m_currentIndexPage--;
        switchHotPage();
    }

    void prevHotPage()
    {
        if (m_currentIndexPage < stacked_widget->count() - 1)
            m_currentIndexPage++;
        switchHotPage();
    }

    void switchHotPage()
    {
        if (m_currentIndexPage < 0 || m_currentIndexPage >= stacked_widget->count())
            return;

        this->players->setMaximumWidth(260);
        this->vehicle->setMaximumWidth(260);
        this->info->setMaximumWidth(260);

        if (m_currentIndexPage == 0)
            this->info->setMaximumWidth(280);
        else if (m_currentIndexPage == 2)
            this->players->setMaximumWidth(280);
        else if (m_currentIndexPage == 4)
            this->vehicle->setMaximumWidth(370);

        int w = defaultWidth, h = 200;
        switch (m_currentIndexPage)
        {
        case 0:
            w = defaultWidth;
            h = 400;
            break; // информационная панель
        case 1:
            w = defaultWidth;
            h = 200;
            break; // отметка
        case 2:
            w = defaultWidth;
            h = 200;
            break; // игроки
        case 3:
            w = defaultWidth;
            h = 200;
            break; // коэффициенты
        case 4:
            w = defaultWidth + 110;
            h = 200;
            break; // танки
        case 5:
            w = defaultWidth;
            h = 200;
            break; // рейтинговая статистика
        case 6:
            w = defaultWidth;
            h = 200;
            break; // основная статистика
        }

        // сначала выставляем текущую страницу, потом resize
        stacked_widget->setCurrentIndex(m_currentIndexPage);
        updateNavButtonStyles(m_currentIndexPage);
        this->resize(w, h);
    }

    void switchPage(int index)
    {
        m_currentIndexPage = index;
        switchHotPage();
        updateNavButtonStyles(index);
    }

    void showConfigPanel(bool show)
    {
        // Анимация панели
        QPropertyAnimation *animPanel = new QPropertyAnimation(configPanel, "maximumWidth");
        animPanel->setDuration(200);
        animPanel->setEasingCurve(QEasingCurve::InOutQuad);

        // Анимация окна
        QPropertyAnimation *animWindow = new QPropertyAnimation(this, "geometry");
        animWindow->setDuration(200);
        animWindow->setEasingCurve(QEasingCurve::InOutQuad);

        QRect startGeom = geometry();
        QRect endGeom = startGeom;

        if (show)
        {
            configPanel->setVisible(true);

            animPanel->setStartValue(0);
            animPanel->setEndValue(configPanelNormalWidth);
            defaultWidth = 280;
            endGeom.setWidth(defaultWidth); // увеличиваем на 30 справа
        }
        else
        {
            animPanel->setStartValue(configPanel->maximumWidth());
            animPanel->setEndValue(0);
            defaultWidth = 250;
            endGeom.setWidth(defaultWidth); // возвращаем дефолтную ширину

            connect(animPanel, &QPropertyAnimation::finished, configPanel, [this]()
                    { configPanel->setVisible(false); });
        }

        animWindow->setStartValue(startGeom);
        animWindow->setEndValue(endGeom);

        animPanel->start(QAbstractAnimation::DeleteWhenStopped);
        animWindow->start(QAbstractAnimation::DeleteWhenStopped);
    }

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

    void drawFrostPatterns(QPainter &p);
};

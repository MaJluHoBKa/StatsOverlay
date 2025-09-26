#pragma once

#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QScreen>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSizePolicy>
#include <QFontDatabase>
#include <QScrollArea>
#include <QAbstractScrollArea>
#include <QDesktopServices>
#include <main_overlay/controller/ApiController.h>
#include <main_overlay/widgets/main_stats/main_stats.h>
#include <main_overlay/widgets/rating_stats/rating_stats.h>
#include <main_overlay/widgets/vehicles_stats/vehicles_stats.h>
#include <main_overlay/widgets/other_stats/other_stats.h>

#include <thread>
#include <chrono>

class InfoPage : public QWidget
{
    Q_OBJECT

private:
    ApiController *m_apiController = nullptr;
    MainStats *m_mainStats;
    RatingStats *m_ratingStats;
    VehicleStats *m_vehicleStats;
    OtherStats *m_otherStats;
    QLabel *message = nullptr;
    QPushButton *auth_button = nullptr;

public:
    explicit InfoPage(ApiController *apiController, MainStats *mainStats, RatingStats *ratingStats, VehicleStats *vehicleStats, OtherStats *otherStats, QWidget *parent = nullptr);

    void setMessageWidget(QLabel *message)
    {
        if (this->message == nullptr)
        {
            this->message = message;
        }
    }

    void setAuthButton(QPushButton *auth_button)
    {
        if (this->auth_button == nullptr)
        {
            this->auth_button = auth_button;
        }
    }

    bool isAuth() const
    {
        return this->m_apiController->is_auth();
    }

    std::string getNickname() const
    {
        return this->m_apiController->getNickname();
    }

private slots:
    void onDonateClicked()
    {
        QDesktopServices::openUrl(QUrl("https://www.donationalerts.com/r/parrylikeme"));
    }

    void onSupportClicked()
    {
        QDesktopServices::openUrl(QUrl("https://lesta.ru/support/ru/"));
    }

    void onResetClicked()
    {
        this->m_apiController->reset();
        this->m_mainStats->resetValue();
        this->m_ratingStats->resetValue();
        // this->m_vehicleStats->resetValue();
        this->m_otherStats->resetValue();
    }

    void onAuthClicked()
    {
        if (isAuth())
        {
            if (this->m_apiController->logout())
            {
                this->message->setText("Выполнен выход из аккаунта.");
                this->auth_button->setText("Авторизоваться");
            }
            else
            {
                this->message->setText("Ошибка выхода. Попробуйте снова.");
            }
        }
        else
        {
            auto auth_thread = [this]()
            {
                if (this->m_apiController->login())
                {
                    QString nickname = QString::fromStdString(getNickname());
                    QMetaObject::invokeMethod(
                        this,
                        [this, nickname]()
                        {
                            this->message->setText(nickname);
                            this->auth_button->setText("Выход");
                        },
                        Qt::QueuedConnection);
                }
                else
                {
                    QMetaObject::invokeMethod(
                        this,
                        [this]()
                        {
                            this->message->setText("Ошибка авторизации. Попробуйте снова.");
                        },
                        Qt::QueuedConnection);
                }
            };

            std::thread t(auth_thread);
            t.detach();
        }
    }
};
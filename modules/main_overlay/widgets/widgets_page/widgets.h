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
#include <QStackedWidget>
#include <QScrollArea>
#include <QTimer>
#include <QCheckBox>
#include <QSlider>
#include <QtConcurrent/QtConcurrent>
#include <main_overlay/controller/ApiController.h>
#include <main_overlay/widgets/widgets_page/widgets_window/widgets_window.h>

#include <unordered_map>
#include <sstream>
#include <cstdint>
#include <iomanip>
#include <string>
#include <iostream>
#include <thread>

class Widgets : public QWidget
{
    Q_OBJECT

private slots:
    void updatingWidgets();
    void toggleRating();
    void toggleMaster();
    void toggleBattles();
    void toggleXp();
    void toggleAlive();
    void toggleShots();
    void toggleWins();
    void toggleDamage();

private:
    ApiController *m_apiController = nullptr;

    WidgetsWindow *widgetsWindow;

    QPushButton *active = nullptr;
    bool isOn = false;

    QPushButton *battlesBox = nullptr;
    QPushButton *winsBox = nullptr;
    QPushButton *damageBox = nullptr;
    QPushButton *xpBox = nullptr;
    QPushButton *aliveBox = nullptr;
    QPushButton *shotsBox = nullptr;
    QPushButton *ratingBox = nullptr;
    QPushButton *masterBox = nullptr;

    QLabel *battlesCheck = nullptr;
    QLabel *winsCheck = nullptr;
    QLabel *damageCheck = nullptr;
    QLabel *xpCheck = nullptr;
    QLabel *aliveCheck = nullptr;
    QLabel *shotsCheck = nullptr;
    QLabel *ratingCheck = nullptr;
    QLabel *masterCheck = nullptr;

    bool is_rating = true;
    bool is_master = true;
    bool is_battles = true;
    bool is_xp = true;
    bool is_alive = true;
    bool is_shots = true;
    bool is_wins = true;
    bool is_damage = true;

    double m_backgroundOpacity = 1.0;

    bool hotVisible = false;

public:
    explicit Widgets(ApiController *apiController, QWidget *parent = nullptr);

    void setIsOn()
    {
        const QString baseStyle = R"(
                QPushButton {
                    background-color: rgb(50, 50, 50);
                    color: #e2ded3;
                    font-size: 14px;
                    font-family: Segoe UI;
                    font-weight: bold;
                    padding: 5px;
                    border-radius: 5px;
                }
                QPushButton:hover {
                    background-color: #4cd964;
                    color: #383838;
                }
                QPushButton:pressed {
                    background-color: #3cbf5f;
                    color: #383838;
                }
                )";
        const QString activeStyle = R"(
                QPushButton {
                    background-color: rgb(50, 50, 50);
                    color: #e2ded3;
                    font-size: 14px;
                    font-family: Segoe UI;
                    font-weight: bold;
                    padding: 5px;
                    border-radius: 5px;
                }
                QPushButton:hover {
                    background-color: #ff5756;
                    color: #383838;
                }

                QPushButton:pressed {
                    background-color: #ff8a8a;
                    color: #383838;
                }
                )";
        if (!this->isOn)
        {
            this->active->setStyleSheet(activeStyle);
            this->active->setText("Выключить");
            this->isOn = true;
            hotVisible = true;
            widgetsWindow->show();
            widgetsWindow->adjustSize();
        }
        else
        {
            this->active->setStyleSheet(baseStyle);
            this->active->setText("Включить");
            this->isOn = false;
            hotVisible = false;
            widgetsWindow->hide();
        }
    }

    void setButtonActive(QPushButton *button)
    {
        if (this->active == nullptr)
        {
            this->active = button;
        }
    }

    bool isAuth() const
    {
        return this->m_apiController->is_auth();
    }
};
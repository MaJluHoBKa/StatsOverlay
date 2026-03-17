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
#include <main_overlay/widgets/player_stats/split_players/players_window.h>

#include <unordered_map>
#include <sstream>
#include <cstdint>
#include <iomanip>
#include <string>
#include <iostream>
#include <thread>

class PlayerStats : public QWidget
{
    Q_OBJECT

private slots:
    void updatingPlayerStats();
    void togglePlayers();
    void toggleTanks();
    void toggleBattles();
    void toggleWins();
    void toggleDamage();

private:
    ApiController *m_apiController = nullptr;

    PlayerStatsWindow *alliesWindow = new PlayerStatsWindow("Союзники");
    PlayerStatsWindow *enemiesWindow = new PlayerStatsWindow("Противники");

    std::vector<Player> allies;
    std::vector<Player> enemies;
    bool isPlayers = false;

    QPushButton *active = nullptr;
    bool isOn = false;
    bool busy = false;

    QPushButton *playersBox = nullptr;
    QPushButton *tanksBox = nullptr;
    QPushButton *battlesBox = nullptr;
    QPushButton *winsBox = nullptr;
    QPushButton *damageBox = nullptr;

    QLabel *playersCheck = nullptr;
    QLabel *tanksCheck = nullptr;
    QLabel *battlesCheck = nullptr;
    QLabel *winsCheck = nullptr;
    QLabel *damageCheck = nullptr;

    bool is_players = true;
    bool is_tanks = true;
    bool is_battles = true;
    bool is_wins = true;
    bool is_damage = true;

    double m_backgroundOpacity;

    double hotVisible = false;

public:
    explicit PlayerStats(ApiController *apiController, QWidget *parent = nullptr);

    void setIsOn()
    {
        const QString baseStyle = R"(
                QPushButton {
                    border: 2px solid rgb(57, 57, 57);;
                    color: #e2ded3;
                    font-size: 13px;
                    font-family: Consolas;
                    font-weight: bold;
                    padding: 5px;
                    border-radius: 5px;
                }
                QPushButton:hover {
                    background-color: rgb(60, 60, 60);
                    color: #ffffff;
                }
                QPushButton:pressed {
                    background-color: rgb(90, 90, 90);
                    color: #cccccc;
                }
                )";
        const QString activeStyle = R"(
                QPushButton {
                    border: 2px solid #4cd964;
                    color: #4cd964;
                    font-size: 13px;
                    font-family: Consolas;
                    font-weight: bold;
                    padding: 5px;
                    border-radius: 5px;
                }
                QPushButton:hover {
                    background-color: #7be68f;
                    color: #383838;
                }

                QPushButton:pressed {
                    background-color: #3cbf5f;
                    color: #383838;
                }
                )";
        if (!this->isOn)
        {
            this->active->setStyleSheet(activeStyle);
            this->active->setText("Вкл");
            alliesWindow->show();
            enemiesWindow->show();
            this->isOn = true;
            hotVisible = true;
        }
        else
        {
            this->active->setStyleSheet(baseStyle);
            this->active->setText("Выкл");
            alliesWindow->hide();
            enemiesWindow->hide();
            this->isOn = false;
            hotVisible = false;
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

    void hotHide()
    {
        if (hotVisible)
        {
            alliesWindow->hide();
            enemiesWindow->hide();
        }
        else
        {
            alliesWindow->show();
            enemiesWindow->show();
        }
        hotVisible = !hotVisible;
    }
};
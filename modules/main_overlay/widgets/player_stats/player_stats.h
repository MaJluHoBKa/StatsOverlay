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
#include <QtConcurrent/QtConcurrent>
#include <main_overlay/controller/ApiController.h>

#include <unordered_map>
#include <sstream>
#include <cstdint>
#include <iomanip>
#include <string>
#include <iostream>
#include <thread>

struct PlayerRows
{
    QHBoxLayout *row = nullptr;
    QLabel *name = nullptr;
    QLabel *tank_name = nullptr;
    QLabel *battles = nullptr;
    QLabel *wins = nullptr;
    QLabel *damage = nullptr;
};

class PlayerStats : public QWidget
{
    Q_OBJECT

private:
    ApiController *m_apiController = nullptr;

    QPushButton *buttonAllies = nullptr;
    QPushButton *buttonEnemies = nullptr;

    QVBoxLayout *data_allies = nullptr;
    QVBoxLayout *data_enemies = nullptr;

    QStackedWidget *pages = nullptr;

    std::vector<int> sizesRow = {105, 105, 50, 50, 50};

    std::vector<PlayerRows *> allies_rows;
    std::vector<PlayerRows *> enemies_rows;

    std::vector<Player> allies;
    std::vector<Player> enemies;
    bool isPlayers = false;

    QPushButton *active = nullptr;
    bool isOn = false;
    bool busy = false;

    int m_currentIndexPage = 0;

public:
    explicit PlayerStats(ApiController *apiController, QWidget *parent = nullptr);

    void setIsOn()
    {
        const QString baseStyle = R"(
                QPushButton {
                    background-color: rgb(50, 50, 50);
                    color: #e2ded3;
                    border: none;
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
                    background-color: #4cd964;
                    color: #383838;
                    border: none;
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
            this->isOn = true;
        }
        else
        {
            this->active->setStyleSheet(baseStyle);
            this->active->setText("Выкл");
            this->isOn = false;
        }
    }

    void setButtonActive(QPushButton *button)
    {
        if (this->active == nullptr)
        {
            this->active = button;
        }
    }

    void setButtonAllies(QPushButton *button)
    {
        if (this->buttonAllies == nullptr)
        {
            this->buttonAllies = button;
        }
    }

    void setButtonEnemies(QPushButton *button)
    {
        if (this->buttonEnemies == nullptr)
        {
            this->buttonEnemies = button;
        }
    }

    void setStackedWidgets(QStackedWidget *pages)
    {
        if (this->pages == nullptr)
        {
            this->pages = pages;
        }
    }

    void switchPlayers(int index)
    {
        const QString baseStyle = R"(
                QPushButton {
                    background-color: rgb(40, 40, 40);
                    font-family: Segoe UI;
                    font-weight: bold;
                    font-size: 14px;
                    color: #e2ded3;
                    border-radius: 3px;
                    border: none;
                }
                QPushButton:hover {
                    background-color: rgb(60, 60, 60);
                }
                QPushButton:pressed {
                    background-color: rgb(90, 90, 90);
                }
                )";
        const QString activeStyle = R"(
                QPushButton {
                    background-color: rgb(50, 50, 50);
                    font-family: Segoe UI;
                    font-weight: bold;
                    font-size: 14px;
                    color: #e2ded3;
                    border-radius: 3px;
                    border: none;
                }
                QPushButton:hover {
                    background-color: rgb(60, 60, 60);
                }
                QPushButton:pressed {
                    background-color: rgb(90, 90, 90);
                }
                )";
        if (index == 0)
        {
            this->buttonAllies->setStyleSheet(activeStyle);
            this->buttonEnemies->setStyleSheet(baseStyle);
        }
        else
        {
            this->buttonAllies->setStyleSheet(baseStyle);
            this->buttonEnemies->setStyleSheet(activeStyle);
        }
        this->m_currentIndexPage = index;
        this->pages->setCurrentIndex(index);
    }

    void setDataAlliesLayout(QVBoxLayout *data)
    {
        if (this->data_allies == nullptr)
        {
            this->data_allies = data;
        }
    }

    void setDataEnemiesLayout(QVBoxLayout *data)
    {
        if (this->data_enemies == nullptr)
        {
            this->data_enemies = data;
        }
    }

    void updatingPlayerStats()
    {
        QMetaObject::invokeMethod(
            this,
            [this]()
            {
                if (!this->isOn)
                {
                    clearData();
                    isPlayers = false;
                    return;
                }

                if (!isAuth())
                    return;

                if (busy)
                    return;

                busy = true;

                QtConcurrent::run([this]()
                                  {
                    if (this->m_apiController->loadReplayPlayers())
                    {
                        if(this->isPlayers)
                        {
                            busy = false;
                            return;
                        }
                        if (this->m_apiController->get_players_stats())
                        {
                            // результат надо передать в GUI-поток
                            QMetaObject::invokeMethod(this, [this]()
                            {
                                this->allies = this->m_apiController->getSortedAllies();
                                this->enemies = this->m_apiController->getSortedEnemies();
                                setDataAllies();
                                setDataEnemies();
                                this->isPlayers = true;
                                busy = false;
                            }, Qt::QueuedConnection);
                        }
                    }
                    else
                    {
                        QMetaObject::invokeMethod(this, [this]()
                        {
                            clearData();
                            isPlayers = false;
                            busy = false;
                        }, Qt::QueuedConnection);
                    } });
            },
            Qt::QueuedConnection);
    }

    void addAlliesRows()
    {
        for (int i = 0; i < 7; i++)
        {
            QHBoxLayout *row = new QHBoxLayout;
            row->setSpacing(1);
            row->setContentsMargins(0, 0, 0, 0);

            const QString baseStyle = R"(
                    font-family: Segoe UI;
                    font-size: 12px;
                    font-weight: bold;
                    color: #e2ded3;
                    padding: 5px 0px;
                    margin: 0px;
                    border-radius: 3px;
                )";

            QLabel *playerName = new QLabel;
            playerName->setStyleSheet(baseStyle +
                                      "text-align: left;");
            playerName->setFixedWidth(this->sizesRow[0]);
            playerName->setFixedHeight(20);

            QLabel *tankName = new QLabel;
            tankName->setStyleSheet(baseStyle +
                                    "text-align: left;");
            tankName->setFixedWidth(this->sizesRow[1]);
            tankName->setFixedHeight(20);

            QLabel *battles = new QLabel;
            battles->setFixedWidth(this->sizesRow[2]);
            battles->setFixedHeight(20);
            battles->setStyleSheet(baseStyle);
            battles->setAlignment(Qt::AlignCenter);

            QLabel *wins = new QLabel;
            wins->setFixedWidth(this->sizesRow[3]);
            wins->setFixedHeight(20);
            wins->setStyleSheet(baseStyle);
            wins->setAlignment(Qt::AlignCenter);

            QLabel *damage = new QLabel;
            damage->setFixedWidth(this->sizesRow[4]);
            damage->setFixedHeight(20);
            damage->setStyleSheet(baseStyle);
            damage->setAlignment(Qt::AlignCenter);

            row->addWidget(playerName);
            row->addWidget(tankName);
            row->addWidget(battles);
            row->addWidget(wins);
            row->addWidget(damage);
            this->data_allies->addLayout(row);

            PlayerRows *info = new PlayerRows();
            info->row = row;
            info->name = playerName;
            info->tank_name = tankName;
            info->battles = battles;
            info->wins = wins;
            info->damage = damage;

            allies_rows.push_back(info);
        }
    }

    void addEnemiesRows()
    {
        for (int i = 0; i < 7; i++)
        {
            QHBoxLayout *row = new QHBoxLayout;
            row->setSpacing(1);
            row->setContentsMargins(0, 0, 0, 0);

            const QString baseStyle = R"(
                    font-family: Segoe UI;
                    font-size: 12px;
                    font-weight: bold;
                    color: #e2ded3;
                    padding: 5px 0px;
                    margin: 0px;
                    border-radius: 3px;
                )";

            QLabel *playerName = new QLabel;
            playerName->setStyleSheet(baseStyle +
                                      "text-align: left;");
            playerName->setFixedWidth(this->sizesRow[0]);
            playerName->setFixedHeight(20);

            QLabel *tankName = new QLabel;
            tankName->setStyleSheet(baseStyle +
                                    "text-align: left;");
            tankName->setFixedWidth(this->sizesRow[1]);
            tankName->setFixedHeight(20);

            QLabel *battles = new QLabel;
            battles->setFixedWidth(this->sizesRow[2]);
            battles->setFixedHeight(20);
            battles->setStyleSheet(baseStyle);
            battles->setAlignment(Qt::AlignCenter);

            QLabel *wins = new QLabel;
            wins->setFixedWidth(this->sizesRow[3]);
            wins->setFixedHeight(20);
            wins->setStyleSheet(baseStyle);
            wins->setAlignment(Qt::AlignCenter);

            QLabel *damage = new QLabel;
            damage->setFixedWidth(this->sizesRow[4]);
            damage->setFixedHeight(20);
            damage->setStyleSheet(baseStyle);
            damage->setAlignment(Qt::AlignCenter);

            row->addWidget(playerName);
            row->addWidget(tankName);
            row->addWidget(battles);
            row->addWidget(wins);
            row->addWidget(damage);
            this->data_enemies->addLayout(row);

            PlayerRows *info = new PlayerRows();
            info->row = row;
            info->name = playerName;
            info->tank_name = tankName;
            info->battles = battles;
            info->wins = wins;
            info->damage = damage;

            enemies_rows.push_back(info);
        }
    }

    void setDataAllies()
    {
        const QString baseStyle = R"(
                    font-family: Segoe UI;
                    font-size: 12px;
                    font-weight: bold;
                    color: #e2ded3;
                    padding: 5px 0px;
                    margin: 0px;
                    border-radius: 3px;
                )";

        for (int i = 0; i < this->allies_rows.size(); i++)
        {
            QString name = QString::fromStdString(this->allies[i].nickname);
            if (name.length() > 10)
            {
                name.resize(10);
                name += "...";
            }
            this->allies_rows[i]->name->setText(name);
            this->allies_rows[i]->name->setStyleSheet(baseStyle);
            QString tank_name = QString::fromStdString(this->allies[i].tank_name);
            if (tank_name.length() > 10)
            {
                tank_name.resize(10);
                tank_name += "...";
            }
            this->allies_rows[i]->tank_name->setText(tank_name);
            this->allies_rows[i]->tank_name->setStyleSheet(baseStyle);

            QString color;

            int64_t battles = this->allies[i].battles;
            if (battles < 5000)
            {
                color = "#d13b49";
            }
            else if (battles >= 5000 && battles < 15000)
            {
                color = "#80d41d";
            }
            else if (battles >= 15000 && battles < 30000)
            {
                color = "#5cded6";
            }
            else
            {
                color = "#a08bea";
            }
            this->allies_rows[i]->battles->setText(QString::fromStdString(std::to_string(battles)));
            this->allies_rows[i]->battles->setStyleSheet(baseStyle + QString("color: %1;").arg(color));

            double wins = (static_cast<double>(this->allies[i].wins) / this->allies[i].battles) * 100.0;
            if (wins >= 70.00)
            {
                color = "#a08bea";
            }
            else if (wins >= 60.00 && wins < 70.00)
            {
                color = "#5cded6";
            }
            else if (wins >= 50.00 && wins < 60.00)
            {
                color = "#80d41d";
            }
            else
            {
                color = "#d13b49";
            }
            this->allies_rows[i]->wins->setText(QString::fromStdString(formatFloat(wins) + "%"));
            this->allies_rows[i]->wins->setStyleSheet(baseStyle + QString("color: %1;").arg(color));

            int64_t damage = this->allies[i].damage / this->allies[i].battles;
            if (damage < 1000)
            {
                color = "#d13b49";
            }
            else if (damage >= 1000 && damage < 1500)
            {
                color = "#80d41d";
            }
            else if (damage >= 1500 && damage < 2500)
            {
                color = "#5cded6";
            }
            else
            {
                color = "#a08bea";
            }
            this->allies_rows[i]->damage->setText(QString::fromStdString(std::to_string(damage)));
            this->allies_rows[i]->damage->setStyleSheet(baseStyle + QString("color: %1;").arg(color));
        }
    }

    void setDataEnemies()
    {
        const QString baseStyle = R"(
                    font-family: Segoe UI;
                    font-size: 12px;
                    font-weight: bold;
                    color: #e2ded3;
                    padding: 5px 0px;
                    margin: 0px;
                    border-radius: 3px;
                )";

        for (int i = 0; i < this->enemies.size(); i++)
        {
            QString name = QString::fromStdString(this->enemies[i].nickname);
            if (name.length() > 10)
            {
                name.resize(10);
                name += "...";
            }
            this->enemies_rows[i]->name->setText(name);
            this->enemies_rows[i]->name->setStyleSheet(baseStyle);
            QString tank_name = QString::fromStdString(this->enemies[i].tank_name);
            if (tank_name.length() > 10)
            {
                tank_name.resize(10);
                tank_name += "...";
            }
            this->enemies_rows[i]->tank_name->setText(tank_name);
            this->enemies_rows[i]->tank_name->setStyleSheet(baseStyle);

            QString color;

            int64_t battles = this->enemies[i].battles;
            if (battles < 5000)
            {
                color = "#d13b49";
            }
            else if (battles >= 5000 && battles < 15000)
            {
                color = "#80d41d";
            }
            else if (battles >= 15000 && battles < 30000)
            {
                color = "#5cded6";
            }
            else
            {
                color = "#a08bea";
            }
            this->enemies_rows[i]->battles->setText(QString::fromStdString(std::to_string(battles)));
            this->enemies_rows[i]->battles->setStyleSheet(baseStyle + QString("color: %1;").arg(color));

            double wins = (static_cast<double>(this->enemies[i].wins) / this->enemies[i].battles) * 100.0;
            if (wins >= 70.00)
            {
                color = "#a08bea";
            }
            else if (wins >= 60.00 && wins < 70.00)
            {
                color = "#5cded6";
            }
            else if (wins >= 50.00 && wins < 60.00)
            {
                color = "#80d41d";
            }
            else
            {
                color = "#d13b49";
            }
            this->enemies_rows[i]->wins->setText(QString::fromStdString(formatFloat(wins) + "%"));
            this->enemies_rows[i]->wins->setStyleSheet(baseStyle + QString("color: %1;").arg(color));

            int64_t damage = this->enemies[i].damage / this->enemies[i].battles;
            if (damage < 1000)
            {
                color = "#d13b49";
            }
            else if (damage >= 1000 && damage < 1500)
            {
                color = "#80d41d";
            }
            else if (damage >= 1500 && damage < 2500)
            {
                color = "#5cded6";
            }
            else
            {
                color = "#a08bea";
            }
            this->enemies_rows[i]->damage->setText(QString::fromStdString(std::to_string(damage)));
            this->enemies_rows[i]->damage->setStyleSheet(baseStyle + QString("color: %1;").arg(color));
        }
    }

    void clearData()
    {
        const QString baseStyle = R"(
                    font-family: Segoe UI;
                    font-size: 12px;
                    font-weight: bold;
                    color: #e2ded3;
                    padding: 5px 0px;
                    margin: 0px;
                    border-radius: 3px;
                )";

        for (int i = 0; i < this->allies_rows.size(); i++)
        {
            allies_rows[i]->name->setText("");
            allies_rows[i]->name->setStyleSheet(baseStyle);
            allies_rows[i]->tank_name->setText("");
            allies_rows[i]->tank_name->setStyleSheet(baseStyle);
            allies_rows[i]->battles->setText("");
            allies_rows[i]->battles->setStyleSheet(baseStyle);
            allies_rows[i]->wins->setText("");
            allies_rows[i]->wins->setStyleSheet(baseStyle);
            allies_rows[i]->damage->setText("");
            allies_rows[i]->damage->setStyleSheet(baseStyle);
        }
        for (int i = 0; i < this->enemies_rows.size(); i++)
        {
            enemies_rows[i]->name->setText("");
            enemies_rows[i]->name->setStyleSheet(baseStyle);
            enemies_rows[i]->tank_name->setText("");
            enemies_rows[i]->tank_name->setStyleSheet(baseStyle);
            enemies_rows[i]->battles->setText("");
            enemies_rows[i]->battles->setStyleSheet(baseStyle);
            enemies_rows[i]->wins->setText("");
            enemies_rows[i]->wins->setStyleSheet(baseStyle);
            enemies_rows[i]->damage->setText("");
            enemies_rows[i]->damage->setStyleSheet(baseStyle);
        }
    }

    bool isAuth() const
    {
        return this->m_apiController->is_auth();
    }

    std::string formatFloat(float value)
    {
        int64_t integerPart = static_cast<int64_t>(value);
        float fractional = value - integerPart;

        std::string formatted = formatInt(integerPart);

        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << fractional;
        std::string fracStr = oss.str().substr(1); // убираем "0" перед точкой

        formatted += fracStr;
        return formatted;
    }

    std::string formatInt(int64_t value)
    {
        std::string str = std::to_string(value);
        int insertPosition = str.length() - 3;
        while (insertPosition > 0)
        {
            str.insert(insertPosition, " ");
            insertPosition -= 3;
        }
        return str;
    }

    void nextHotPage()
    {
        if (m_currentIndexPage < pages->count() - 1)
            m_currentIndexPage++;
        switchHotPage();
    }

    void prevHotPage()
    {
        if (m_currentIndexPage > 0)
            m_currentIndexPage--;
        switchHotPage();
    }

    void switchHotPage()
    {
        if (m_currentIndexPage < 0 || m_currentIndexPage >= pages->count())
            return;

        const QString baseStyle = R"(
                QPushButton {
                    background-color: rgb(40, 40, 40);
                    font-family: Segoe UI;
                    font-weight: bold;
                    font-size: 14px;
                    color: #e2ded3;
                    border-radius: 3px;
                    border: none;
                }
                QPushButton:hover {
                    background-color: rgb(60, 60, 60);
                }
                QPushButton:pressed {
                    background-color: rgb(90, 90, 90);
                }
                )";
        const QString activeStyle = R"(
                QPushButton {
                    background-color: rgb(50, 50, 50);
                    font-family: Segoe UI;
                    font-weight: bold;
                    font-size: 14px;
                    color: #e2ded3;
                    border-radius: 3px;
                    border: none;
                }
                QPushButton:hover {
                    background-color: rgb(60, 60, 60);
                }
                QPushButton:pressed {
                    background-color: rgb(90, 90, 90);
                }
                )";
        if (m_currentIndexPage == 0)
        {
            this->buttonAllies->setStyleSheet(activeStyle);
            this->buttonEnemies->setStyleSheet(baseStyle);
        }
        else
        {
            this->buttonAllies->setStyleSheet(baseStyle);
            this->buttonEnemies->setStyleSheet(activeStyle);
        }
        pages->setCurrentIndex(m_currentIndexPage);
    }
};
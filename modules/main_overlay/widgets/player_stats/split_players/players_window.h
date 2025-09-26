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

class PlayerStatsWindow : public QWidget
{
    Q_OBJECT

private:
    QVBoxLayout *data_players = nullptr;

    std::vector<int> sizesRow = {105, 105, 50, 50, 50};

    std::vector<PlayerRows *> rows_players;

    PlayerRows *avg;

    std::vector<Player> players;

public:
    explicit PlayerStatsWindow(const QString &title, QWidget *parent = nullptr);

    void addRows()
    {
        for (int i = 0; i < 8; i++)
        {
            QHBoxLayout *row = new QHBoxLayout;
            row->setSpacing(1);
            row->setContentsMargins(0, 0, 0, 0);

            const QString baseStyle = R"(
                    font-family: Roboto;
                    font-size: 12px;
                    font-weight: bold;
                    color: #e2ded3;
                    padding: 5px 0px;
                    margin: 0px;
                    border-radius: 3px;
                )";

            QLabel *playerName = new QLabel;
            playerName->setStyleSheet(baseStyle);
            playerName->setFixedWidth(this->sizesRow[0]);
            playerName->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
            playerName->setFixedHeight(20);

            QLabel *tankName = new QLabel;
            tankName->setStyleSheet(baseStyle);
            tankName->setFixedWidth(this->sizesRow[1]);
            tankName->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
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

            PlayerRows *info = new PlayerRows();
            info->row = row;
            info->name = playerName;
            info->tank_name = tankName;
            info->battles = battles;
            info->wins = wins;
            info->damage = damage;

            if (i == 7)
            {
                QFrame *line = new QFrame;
                line->setMinimumWidth(0);
                line->setFrameShape(QFrame::HLine);
                line->setFrameShadow(QFrame::Plain); // вместо Sunken/Styled
                line->setStyleSheet("background-color: #e2ded3;");
                line->setFixedHeight(1);
                line->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
                this->data_players->addWidget(line);
                this->data_players->addLayout(row);
                avg = info;
                return;
            }

            this->data_players->addLayout(row);
            rows_players.push_back(info);
        }
    }

    void setData()
    {
        const QString baseStyle = R"(
                    font-family: Roboto;
                    font-size: 12px;
                    font-weight: bold;
                    color: #e2ded3;
                    padding: 5px 0px;
                    margin: 0px;
                    border-radius: 3px;
                )";

        const QString baseStyleAVG = R"(
                    font-family: Roboto;
                    font-size: 12px;
                    font-weight: bold;
                    color: #ffd166;
                    padding: 5px 0px;
                    margin: 0px;
                    border-radius: 3px;
                )";

        int64_t all_damage = 0;
        int64_t all_wins = 0;
        int64_t all_battles = 0;

        for (int i = 0; i < this->rows_players.size(); i++)
        {
            QString name = QString::fromStdString(this->players[i].nickname);
            if (name.length() > 12)
            {
                name.resize(12);
                name += "...";
            }
            this->rows_players[i]->name->setText(name);
            this->rows_players[i]->name->setStyleSheet(baseStyle);
            QString tank_name = QString::fromStdString(this->players[i].tank_name);
            if (tank_name.length() > 12)
            {
                tank_name.resize(12);
                tank_name += "...";
            }
            this->rows_players[i]->tank_name->setText(tank_name);
            this->rows_players[i]->tank_name->setStyleSheet(baseStyle);

            QString color;

            int64_t battles = this->players[i].battles;
            all_battles += battles;
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
            this->rows_players[i]->battles->setText(QString::fromStdString(std::to_string(battles)));
            this->rows_players[i]->battles->setStyleSheet(baseStyle + QString("color: %1;").arg(color));

            double wins = (static_cast<double>(this->players[i].wins) / this->players[i].battles) * 100.0;
            all_wins += this->players[i].wins;
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
            this->rows_players[i]->wins->setText(QString::fromStdString(formatFloat(wins) + "%"));
            this->rows_players[i]->wins->setStyleSheet(baseStyle + QString("color: %1;").arg(color));

            int64_t damage = this->players[i].damage / this->players[i].battles;
            all_damage += this->players[i].damage;
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
            this->rows_players[i]->damage->setText(QString::fromStdString(std::to_string(damage)));
            this->rows_players[i]->damage->setStyleSheet(baseStyle + QString("color: %1;").arg(color));
        }

        this->avg->name->setText("AVG");
        this->avg->name->setStyleSheet(baseStyleAVG);
        this->avg->tank_name->setText("");
        this->avg->tank_name->setStyleSheet(baseStyleAVG);

        this->avg->battles->setText(QString::fromStdString(std::to_string(all_battles / 7)));
        this->avg->battles->setStyleSheet(baseStyleAVG);

        double wins = (static_cast<double>(all_wins) / all_battles) * 100.0;
        this->avg->wins->setText(QString::fromStdString(formatFloat(wins) + "%"));
        this->avg->wins->setStyleSheet(baseStyleAVG);

        int64_t damage = all_damage / all_battles;
        this->avg->damage->setText(QString::fromStdString(std::to_string(damage)));
        this->avg->damage->setStyleSheet(baseStyleAVG);
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

        for (int i = 0; i < this->rows_players.size(); i++)
        {
            rows_players[i]->name->setText("");
            rows_players[i]->name->setStyleSheet(baseStyle);
            rows_players[i]->tank_name->setText("");
            rows_players[i]->tank_name->setStyleSheet(baseStyle);
            rows_players[i]->battles->setText("");
            rows_players[i]->battles->setStyleSheet(baseStyle);
            rows_players[i]->wins->setText("");
            rows_players[i]->wins->setStyleSheet(baseStyle);
            rows_players[i]->damage->setText("");
            rows_players[i]->damage->setStyleSheet(baseStyle);
        }
        avg->name->setText("");
        avg->battles->setText("");
        avg->wins->setText("");
        avg->damage->setText("");
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
};
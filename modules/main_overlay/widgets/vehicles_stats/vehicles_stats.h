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

struct InfoRows
{
    QHBoxLayout *row = nullptr;
    QLabel *name = nullptr;
    QLabel *battles = nullptr;
    QLabel *wins = nullptr;
    QLabel *damage = nullptr;
};

class VehicleStats : public QWidget
{
    Q_OBJECT

private:
    ApiController *m_apiController = nullptr;

    QVBoxLayout *data = nullptr;
    std::vector<int> sizesRow = {105, 50, 50, 50};

    QHash<QString, InfoRows *> info_rows;

public:
    explicit VehicleStats(ApiController *apiController, QWidget *parent = nullptr);

    void setDataLayout(QVBoxLayout *data)
    {
        if (this->data == nullptr)
        {
            this->data = data;
        }
    }

    void updatingVehicleStats()
    {
        if (!isAuth())
            return;

        QtConcurrent::run([this]()
                          {
            if (this->m_apiController->update_vehicles_stats())
            {
                const VehicleData *vehicleData = this->m_apiController->get_updated_vehicles();
                if (vehicleData != nullptr)
                {
                    std::string name = this->m_apiController->getVehicleName(vehicleData->id);
                    double winRate = (static_cast<double>(vehicleData->wins) / vehicleData->battles) * 100.0;
                    int64_t damage = vehicleData->totalDamage / vehicleData->battles;

                    QMetaObject::invokeMethod(this, [this, name, vehicleData, winRate, damage]() {
                        updateTankRow(QString::fromStdString(name),
                                      vehicleData->battles,
                                      std::round(winRate * 100.0) / 100.0,
                                      damage);
                    }, Qt::QueuedConnection);
                }
                else
                {
                    qDebug() << "No updated tanks";
                }
            }
            else
            {
                qDebug() << "Not updated stats VEH";
            } });
    }

    void addTankRow(QString name, int64_t battles, float wins, int64_t damage)
    {
        QHBoxLayout *row = new QHBoxLayout;
        row->setSpacing(1);
        row->setContentsMargins(0, 0, 0, 0);

        int id = QFontDatabase::addApplicationFont(":/main_stats/resources/fonts/JetBrainsMono-Bold.ttf");
        QString family = QFontDatabase::applicationFontFamilies(id).at(0);

        const QString baseStyle = R"(
                    font-family: "%1";
                    font-size: 11px;
                    font-weight: bold;
                    color: #e2ded3;
                    background-color: #383838;
                    padding: 5px 0px;
                    margin: 0px;
                    border-radius: 3px;
                )";
        QString styled = baseStyle.arg(family);

        QLabel *tankName = new QLabel;
        QString copy_name = name;
        if (copy_name.length() > 11)
        {
            copy_name.resize(11);
            copy_name += "...";
        }
        tankName->setText(copy_name);
        tankName->setStyleSheet(styled +
                                "text-align: left;");
        tankName->setFixedWidth(this->sizesRow[0]);
        tankName->setFixedHeight(25);

        QLabel *tankBattles = new QLabel;
        tankBattles->setText(QString::fromStdString(formatInt(battles)));
        tankBattles->setStyleSheet(styled);
        tankBattles->setFixedWidth(this->sizesRow[1]);
        tankBattles->setFixedHeight(25);
        tankBattles->setAlignment(Qt::AlignCenter);

        QLabel *tankWins = new QLabel;
        tankWins->setText(QString::fromStdString(formatFloat(wins) + "%"));
        QString color;
        if (wins >= 70.0)
        {
            color = "#9989e6";
        }
        else if (wins >= 60.0)
        {
            color = "#72d1ff";
        }
        else if (wins >= 50.0)
        {
            color = "#a8e689";
        }
        else
        {
            color = "#ffffff";
        }
        tankWins->setStyleSheet(styled + QString("color: %1;").arg(color));
        tankWins->setFixedWidth(this->sizesRow[2]);
        tankWins->setFixedHeight(25);
        tankWins->setAlignment(Qt::AlignCenter);

        QLabel *tankDamage = new QLabel;
        tankDamage->setText(QString::fromStdString(std::to_string(damage)));
        tankDamage->setStyleSheet(styled);
        tankDamage->setFixedWidth(this->sizesRow[3]);
        tankDamage->setFixedHeight(25);
        tankDamage->setAlignment(Qt::AlignCenter);

        row->addWidget(tankName);
        row->addWidget(tankBattles);
        row->addWidget(tankWins);
        row->addWidget(tankDamage);
        this->data->addLayout(row);

        InfoRows *info = new InfoRows();
        info->row = row;
        info->name = tankName;
        info->battles = tankBattles;
        info->wins = tankWins;
        info->damage = tankDamage;

        info_rows[name] = info;
    }

    void updateTankRow(QString name, int64_t battles, float wins, int64_t damage)
    {
        int id = QFontDatabase::addApplicationFont(":/main_stats/resources/fonts/JetBrainsMono-Bold.ttf");
        QString family = QFontDatabase::applicationFontFamilies(id).at(0);

        const QString baseStyle = R"(
                    font-family: "%1";
                    font-size: 11px;
                    font-weight: bold;
                    color: #e2ded3;
                    background-color: #383838;
                    padding: 5px 0px;
                    margin: 0px;
                    border-radius: 3px;
                )";
        QString styled = baseStyle.arg(family);

        if (this->info_rows.find(name) != this->info_rows.end())
        {
            this->info_rows[name]->battles->setText(QString::fromStdString(std::to_string(battles)));
            this->info_rows[name]->wins->setText(QString::fromStdString(formatFloat(wins) + "%"));
            this->info_rows[name]->damage->setText(QString::fromStdString(std::to_string(damage)));

            QString color;
            if (wins >= 70.0)
            {
                color = "#9989e6";
            }
            else if (wins >= 60.0)
            {
                color = "#72d1ff";
            }
            else if (wins >= 50.0)
            {
                color = "#a8e689";
            }
            else
            {
                color = "#ffffff";
            }
            this->info_rows[name]->wins->setStyleSheet(styled + QString("color: %1;").arg(color));
        }
        else
        {
            addTankRow(name, battles, wins, damage);
        }
    }

    void resetValue()
    {
        if (data)
        {
            QLayoutItem *item;
            while ((item = data->takeAt(0)) != nullptr)
            {
                if (QWidget *w = item->widget())
                    w->deleteLater();
                else if (QLayout *layout = item->layout())
                {
                    QLayoutItem *subItem;
                    while ((subItem = layout->takeAt(0)) != nullptr)
                    {
                        if (QWidget *sw = subItem->widget())
                            sw->deleteLater();
                        delete subItem;
                    }
                    delete layout;
                }
                delete item;
            }
        }

        for (auto row : info_rows)
        {
            delete row;
        }
        info_rows.clear();
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
};
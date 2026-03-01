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
#include <QPainterPath>
#include <QtConcurrent/QtConcurrent>
#include <main_overlay/controller/ApiController.h>
#include <main_overlay/widgets/gun_marks/gun_marks.h>

#include <unordered_map>
#include <sstream>
#include <cstdint>
#include <iomanip>
#include <string>
#include <iostream>
#include <thread>

class TankRowWidget : public QWidget
{
public:
    explicit TankRowWidget(QString nation, QWidget *parent = nullptr)
        : QWidget(parent), m_nation(nation) {}

protected:
    void paintEvent(QPaintEvent *event) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);

        // Сначала клип для скруглённых углов
        QPainterPath path;
        path.addRoundedRect(rect(), 3, 3);
        painter.setClipPath(path);

        // Серый фон на весь виджет
        painter.fillRect(rect(), QColor(35, 35, 35));

        // Картинка слева
        QPixmap tankImg(QString(":vehicle_stats/resources/icons/%1.png").arg(m_nation));
        if (!tankImg.isNull())
        {
            QRect imgRect(0, 0, 120, height());
            painter.drawPixmap(imgRect, tankImg);

            // Градиент: картинка слева → растворяется вправо в серый
            QLinearGradient grad(imgRect.left(), 0, imgRect.right(), 0);
            grad.setColorAt(0.0, QColor(35, 35, 35, 245)); // почти непрозрачный слева
            grad.setColorAt(0.5, QColor(35, 35, 35, 220)); // полупрозрачный в центре
            grad.setColorAt(1.0, QColor(35, 35, 35, 255)); // непрозрачный серый
            painter.fillRect(imgRect, grad);

            // Затемнение сверху
            int fadeSize = 9; // высота fade в пикселях
            QLinearGradient gradTop(0, 0, 0, fadeSize);
            gradTop.setColorAt(0.0, QColor(35, 35, 35, 255)); // непрозрачный
            gradTop.setColorAt(1.0, QColor(35, 35, 35, 0));   // прозрачный
            painter.fillRect(QRect(0, 0, 120, fadeSize), gradTop);

            // Затемнение снизу
            QLinearGradient gradBottom(0, height() - fadeSize, 0, height());
            gradBottom.setColorAt(0.0, QColor(35, 35, 35, 0));   // прозрачный
            gradBottom.setColorAt(1.0, QColor(35, 35, 35, 255)); // непрозрачный
            painter.fillRect(QRect(0, height() - fadeSize, 120, fadeSize), gradBottom);
        }

        QWidget::paintEvent(event);
    }

private:
    QString m_nation;
};

struct InfoRows
{
    QHBoxLayout *row = nullptr;
    QLabel *tier = nullptr;
    QLabel *type = nullptr;
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
    GunMarks *m_gunMark = nullptr;

    QVBoxLayout *data = nullptr;
    std::vector<int> sizesRow = {30, 30, 105, 60, 60, 60};

    QHash<QString, InfoRows *> info_rows;
    QHash<QString, QWidget *> tankRowWidgets;

    bool isMark = false;
    int64_t tankForMarkID = 0;

public:
    explicit VehicleStats(ApiController *apiController, GunMarks *gunMark, QWidget *parent = nullptr);

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
                    int64_t tank_id = vehicleData->id;
                    std::string name = this->m_apiController->getVehicleName(vehicleData->id);
                    int64_t tier = this->m_apiController->getVehicleTier(vehicleData->id);
                    std::string type = this->m_apiController->getVehicleType(vehicleData->id);
                    std::string nation = this->m_apiController->getVehicleNation(vehicleData->id);
                    std::string status = this->m_apiController->getVehicleStatus(vehicleData->id);
                    double winRate = (static_cast<double>(vehicleData->wins) / vehicleData->battles) * 100.0;
                    int64_t damage = vehicleData->totalDamage / vehicleData->battles;


                    if(m_apiController->isMark() && m_apiController->isLock() && this->tankForMarkID == vehicleData->id)
                    {
                        this->m_gunMark->updateInfo(name, vehicleData->totalDamage);
                    }

                    if(m_apiController->isMark() && !m_apiController->isLock())
                    {
                        this->tankForMarkID = vehicleData->id;
                        this->m_gunMark->updateInfo(name, vehicleData->totalDamage);
                        m_apiController->setTankLock(true);
                    }

                    QMetaObject::invokeMethod(this, [this, tank_id, name, tier, type, nation, status, vehicleData, winRate, damage]() {
                        updateTankRow(vehicleData->id,
                                      QString::fromStdString(name),
                                      tier,
                                      QString::fromStdString(type),
                                      QString::fromStdString(nation),
                                      QString::fromStdString(status),
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

    void addTankRow(int64_t tank_id, QString name, int64_t tier, QString type, QString nation, QString status, int64_t battles, float wins, int64_t damage)
    {
        QHBoxLayout *row = new QHBoxLayout;
        row->setSpacing(0);
        row->setContentsMargins(0, 0, 0, 0);

        int id = QFontDatabase::addApplicationFont(":/main_stats/resources/fonts/JetBrainsMono-Bold.ttf");
        QString family = QFontDatabase::applicationFontFamilies(id).at(0);

        const QString baseStyle = R"(
                    font-family: "%1";
                    font-size: 12px;
                    font-weight: bold;
                    color: #e2ded3;
                    background-color: transparent;
                    padding: 5px 0px;
                    margin: 0px;
                    border-radius: 3px;
                )";
        QString styled = baseStyle.arg(family);

        QLabel *tankTier = new QLabel;
        switch (tier)
        {
        case 1:
            tankTier->setText("I");
            break;
        case 2:
            tankTier->setText("II");
            break;
        case 3:
            tankTier->setText("III");
            break;
        case 4:
            tankTier->setText("IV");
            break;
        case 5:
            tankTier->setText("V");
            break;
        case 6:
            tankTier->setText("VI");
            break;
        case 7:
            tankTier->setText("VII");
            break;
        case 8:
            tankTier->setText("VIII");
            break;
        case 9:
            tankTier->setText("IX");
            break;
        case 10:
            tankTier->setText("X");
            break;
        default:
            break;
        }
        tankTier->setMaximumWidth(this->sizesRow[0]);
        tankTier->setFixedHeight(30);
        tankTier->setAlignment(Qt::AlignCenter);

        QLabel *tankType = new QLabel;
        QString typeIconPath = ":vehicle_stats/resources/icons/" + type + "_" + status + ".png";
        QPixmap typePixmap(typeIconPath);
        tankType->setPixmap(typePixmap.scaled(16, 16, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        tankType->setMaximumWidth(this->sizesRow[1]);
        tankType->setFixedHeight(30);
        tankType->setAlignment(Qt::AlignCenter);

        QLabel *tankName = new QLabel;
        QString copy_name = name;
        if (copy_name.length() > 11)
        {
            copy_name.resize(11);
            copy_name += "...";
        }
        tankName->setText(copy_name);

        if (status == "collect")
        {
            tankName->setStyleSheet(styled +
                                    "color: #00bedb;" +
                                    "text-align: left;");
            tankTier->setStyleSheet(styled +
                                    "color: #00bedb;" +
                                    "text-align: center;");
        }
        else if (status == "premium")
        {
            tankName->setStyleSheet(styled +
                                    "color: #ffb900;" +
                                    "text-align: left;");
            tankTier->setStyleSheet(styled +
                                    "color: #ffb900;" +
                                    "text-align: center;");
        }
        else
        {
            tankName->setStyleSheet(styled +
                                    "text-align: left;");
            tankTier->setStyleSheet(styled +
                                    "text-align: center;");
        }
        tankName->setFixedWidth(this->sizesRow[2]);
        tankName->setFixedHeight(30);

        QLabel *tankBattles = new QLabel;
        tankBattles->setText(QString::fromStdString(formatInt(battles)));
        tankBattles->setStyleSheet(styled);
        tankBattles->setFixedWidth(this->sizesRow[3]);
        tankBattles->setFixedHeight(30);
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
        tankWins->setFixedWidth(this->sizesRow[4]);
        tankWins->setFixedHeight(30);
        tankWins->setAlignment(Qt::AlignCenter);

        QLabel *tankDamage = new QLabel;
        tankDamage->setText(QString::fromStdString(std::to_string(damage)));
        tankDamage->setStyleSheet(styled);
        tankDamage->setFixedWidth(this->sizesRow[5]);
        tankDamage->setFixedHeight(30);
        tankDamage->setAlignment(Qt::AlignCenter);

        auto addSeparator = [&]()
        {
            QFrame *sep = new QFrame;
            sep->setFrameShape(QFrame::VLine);
            sep->setFixedWidth(1);
            sep->setFixedHeight(15);
            sep->setStyleSheet("background-color: rgba(255,255,255,0.08);");
            row->addWidget(sep);
        };

        row->addWidget(tankTier);
        addSeparator();
        row->addWidget(tankType);
        addSeparator();
        row->addWidget(tankName);
        addSeparator();
        row->addWidget(tankBattles);
        addSeparator();
        row->addWidget(tankWins);
        addSeparator();
        row->addWidget(tankDamage);

        QWidget *rowWidget = new QWidget;
        rowWidget->setFixedHeight(40);
        rowWidget->setLayout(row);

        this->data->insertWidget(0, rowWidget);

        InfoRows *info = new InfoRows();
        info->row = row;
        info->tier = tankTier;
        info->type = tankType;
        info->name = tankName;
        info->battles = tankBattles;
        info->wins = tankWins;
        info->damage = tankDamage;

        info_rows[name] = info;
        tankRowWidgets[name] = rowWidget;
    }

    void updateTankRow(int64_t tank_id, QString name, int64_t tier, QString type, QString nation, QString status, int64_t battles, float wins, int64_t damage)
    {
        int id = QFontDatabase::addApplicationFont(":/main_stats/resources/fonts/JetBrainsMono-Bold.ttf");
        QString family = QFontDatabase::applicationFontFamilies(id).at(0);

        const QString baseStyle = R"(
                    font-family: "%1";
                    font-size: 11px;
                    font-weight: bold;
                    color: #e2ded3;
                    background-color: transparent;
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
            addTankRow(tank_id, name, tier, type, nation, status, battles, wins, damage);
        }
    }

    // void resetValue()
    // {
    //     if (data)
    //     {
    //         QLayoutItem *item;
    //         while ((item = data->takeAt(0)) != nullptr)
    //         {
    //             if (QWidget *w = item->widget())
    //                 w->deleteLater();
    //             else if (QLayout *layout = item->layout())
    //             {
    //                 QLayoutItem *subItem;
    //                 while ((subItem = layout->takeAt(0)) != nullptr)
    //                 {
    //                     if (QWidget *sw = subItem->widget())
    //                         sw->deleteLater();
    //                     delete subItem;
    //                 }
    //                 delete layout;
    //             }
    //             delete item;
    //         }
    //     }

    //     for (auto row : info_rows)
    //     {
    //         delete row;
    //     }
    //     info_rows.clear();
    // }

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
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
#include <QTimer>
#include <QHash>
#include <QtConcurrent/QtConcurrent>
#include <main_overlay/controller/ApiController.h>

#include <unordered_map>
#include <sstream>
#include <cstdint>
#include <iomanip>
#include <string>
#include <iostream>
#include <thread>

struct InfoOtherLabel
{
    QLabel *value = nullptr;
    QLabel *arrow = nullptr;
    QFrame *line = nullptr;
};

class OtherStats : public QWidget
{
    Q_OBJECT

private:
    std::unordered_map<std::string, InfoOtherLabel *> info;
    OtherStatsData prev_stats;

    QHash<QString, QLabel *> data_master_values;

    ApiController *m_apiController = nullptr;

public:
    explicit OtherStats(ApiController *apiController, QWidget *parent = nullptr);
    void addContent(QVBoxLayout *mainLayout, QPixmap icon_source, QString title);
    void addContentMedal(QHBoxLayout *medalLayout, QPixmap icon_source);
    void addContentMedalValue(QHBoxLayout *valueLayout, QString key);

    void appendInfoLabel(std::string key, QLabel *value, QLabel *arrow, QFrame *line)
    {
        InfoOtherLabel *infoLabel = new InfoOtherLabel;
        infoLabel->value = value;
        infoLabel->arrow = arrow;
        infoLabel->line = line;
        this->info[key] = infoLabel;
    }

    void updatingOtherStats()
    {
        if (!isAuth())
            return;

        QtConcurrent::run([this]()
                          {
            if (this->m_apiController->update_mastery_stats())
            {
                MasteryStatsData masteryStatsData = this->m_apiController->getMasteryStats();

                // Обновляем UI только в GUI-потоке
                QMetaObject::invokeMethod(this, [this, masteryStatsData]() {
                    MasteryData masteryData = masteryStatsData.getCurrentData();
                    setValueMaster("Mastery_1", masteryData.mastery);
                    setValueMaster("Mastery_2", masteryData.mastery_1);
                    setValueMaster("Mastery_3", masteryData.mastery_2);
                    setValueMaster("Mastery_4", masteryData.mastery_3);
                }, Qt::QueuedConnection);
            }
            else
            {
                qDebug() << "Not updated stats MASTERY";
            }
            
            if (this->m_apiController->update_other_stats())
            {
                OtherStatsData otherStatsData = this->m_apiController->getOtherStats();

                // Обновляем UI только в GUI-потоке
                QMetaObject::invokeMethod(this, [this, otherStatsData]() {
                    if (otherStatsData.getShots() > 0)
                    {
                        setValue("Процент попаданий", otherStatsData.getPercentHits());
                    }
                    if (otherStatsData.getBattles() > 0)
                    {
                        setValue("Процент выживания", otherStatsData.getPercentSurvived());
                        setValueK("Коэфф. уничтожения", otherStatsData.getFragsK());
                        setValue("Ср. время выживания", otherStatsData.getLifeTime());
                    }
                    if (otherStatsData.getReceiverDamage() > 0)
                    {
                        setValueK("Коэфф. урона", otherStatsData.getDamageK());
                    }
                    updateColor(otherStatsData);
                    prev_stats = otherStatsData;
                }, Qt::QueuedConnection);
            }
            else
            {
                qDebug() << "Not updated stats MAIN";
            } });
    }

    void setValueMaster(std::string key, int64_t value)
    {
        std::string formatted = formatInt(value);
        QString new_str = QString::fromStdString(formatted);
        this->data_master_values[QString::fromStdString(key)]->setText(new_str);
    }

    void setValue(std::string key, int64_t value)
    {
        int minute = static_cast<int>(value / 60);
        int second = static_cast<int>(value % 60);
        QString new_str = QString("%1:%2").arg(minute).arg(second, 2, 10, QChar('0'));
        this->info[key]->value->setText(new_str);
    }

    void setValue(std::string key, float value)
    {
        std::ostringstream oss;
        oss << formatFloat(value) << "%";
        std::string formatted = oss.str();
        QString new_str = QString::fromStdString(formatted);
        this->info[key]->value->setText(new_str);
    }

    void setValueK(std::string key, float value)
    {
        std::ostringstream oss;
        oss << formatFloat(value);
        std::string formatted = oss.str();
        QString new_str = QString::fromStdString(formatted);
        this->info[key]->value->setText(new_str);
    }

    void updateColor(const OtherStatsData &otherStatsData)
    {
        OtherStatsData prevCopy = this->prev_stats;
        if (otherStatsData.getPercentHits() > prevCopy.getPercentHits())
        {
            this->info["Процент попаданий"]->arrow->setText("▲");
            this->info["Процент попаданий"]->arrow->setStyleSheet("color: #66ff66;");
            this->info["Процент попаданий"]->line->setStyleSheet("background-color: #66ff66;");
        }
        else if (otherStatsData.getPercentHits() < prevCopy.getPercentHits())
        {
            this->info["Процент попаданий"]->arrow->setText("▼");
            this->info["Процент попаданий"]->arrow->setStyleSheet("color: #ff6666;");
            this->info["Процент попаданий"]->line->setStyleSheet("background-color: #ff6666;");
        }

        if (otherStatsData.getPercentSurvived() > prevCopy.getPercentSurvived())
        {
            this->info["Процент выживания"]->arrow->setText("▲");
            this->info["Процент выживания"]->arrow->setStyleSheet("color: #66ff66;");
            this->info["Процент выживания"]->line->setStyleSheet("background-color: #66ff66;");
        }
        else if (otherStatsData.getPercentSurvived() < prevCopy.getPercentSurvived())
        {
            this->info["Процент выживания"]->arrow->setText("▼");
            this->info["Процент выживания"]->arrow->setStyleSheet("color: #ff6666;");
            this->info["Процент выживания"]->line->setStyleSheet("background-color: #ff6666;");
        }

        if (otherStatsData.getDamageK() > prevCopy.getDamageK())
        {
            this->info["Коэфф. урона"]->arrow->setText("▲");
            this->info["Коэфф. урона"]->arrow->setStyleSheet("color: #66ff66;");
            this->info["Коэфф. урона"]->line->setStyleSheet("background-color: #66ff66;");
        }
        else if (otherStatsData.getDamageK() < prevCopy.getDamageK())
        {
            this->info["Коэфф. урона"]->arrow->setText("▼");
            this->info["Коэфф. урона"]->arrow->setStyleSheet("color: #ff6666;");
            this->info["Коэфф. урона"]->line->setStyleSheet("background-color: #ff6666;");
        }

        if (otherStatsData.getFragsK() > prevCopy.getFragsK())
        {
            this->info["Коэфф. уничтожения"]->arrow->setText("▲");
            this->info["Коэфф. уничтожения"]->arrow->setStyleSheet("color: #66ff66;");
            this->info["Коэфф. уничтожения"]->line->setStyleSheet("background-color: #66ff66;");
        }
        else if (otherStatsData.getFragsK() < prevCopy.getFragsK())
        {
            this->info["Коэфф. уничтожения"]->arrow->setText("▼");
            this->info["Коэфф. уничтожения"]->arrow->setStyleSheet("color: #ff6666;");
            this->info["Коэфф. уничтожения"]->line->setStyleSheet("background-color: #ff6666;");
        }
    }

    void resetValue()
    {
        OtherStatsData otherStatsData = this->m_apiController->getOtherStats();
        this->prev_stats = otherStatsData;

        this->data_master_values["Mastery_1"]->setText("-");
        this->data_master_values["Mastery_2"]->setText("-");
        this->data_master_values["Mastery_3"]->setText("-");
        this->data_master_values["Mastery_4"]->setText("-");

        this->info["Процент попаданий"]->value->setText("-");
        this->info["Процент попаданий"]->arrow->setText("");
        this->info["Процент попаданий"]->arrow->setStyleSheet("color: #e2ded3;");
        this->info["Процент попаданий"]->line->setStyleSheet("background-color: #e2ded3;");

        this->info["Процент выживания"]->value->setText("-");
        this->info["Процент выживания"]->arrow->setText("");
        this->info["Процент выживания"]->arrow->setStyleSheet("color: #e2ded3;");
        this->info["Процент выживания"]->line->setStyleSheet("background-color: #e2ded3;");

        this->info["Коэфф. урона"]->value->setText("-");
        this->info["Коэфф. урона"]->arrow->setText("");
        this->info["Коэфф. урона"]->arrow->setStyleSheet("color: #e2ded3;");
        this->info["Коэфф. урона"]->line->setStyleSheet("background-color: #e2ded3;");

        this->info["Коэфф. уничтожения"]->value->setText("-");
        this->info["Коэфф. уничтожения"]->arrow->setText("");
        this->info["Коэфф. уничтожения"]->arrow->setStyleSheet("color: #e2ded3;");
        this->info["Коэфф. уничтожения"]->line->setStyleSheet("background-color: #e2ded3;");

        this->info["Ср. время выживания"]->value->setText("-");
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
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
#include <QtConcurrent/QtConcurrent>
#include <main_overlay/controller/ApiController.h>

#include <unordered_map>
#include <sstream>
#include <cstdint>
#include <iomanip>
#include <string>
#include <iostream>
#include <thread>

struct InfoRatingLabel
{
    QLabel *value = nullptr;
    QLabel *arrow = nullptr;
    QFrame *line = nullptr;
};

class RatingStats : public QWidget
{
    Q_OBJECT

private:
    std::unordered_map<std::string, InfoRatingLabel *> info;
    RatingStatsData prev_stats;

    ApiController *m_apiController = nullptr;

public:
    explicit RatingStats(ApiController *apiController, QWidget *parent = nullptr);
    void addContent(QVBoxLayout *mainLayout, QPixmap icon_source, QString title, bool is_top, bool is_bottom);

    void appendInfoLabel(std::string key, QLabel *value, QLabel *arrow, QFrame *line)
    {
        InfoRatingLabel *infoLabel = new InfoRatingLabel;
        infoLabel->value = value;
        infoLabel->arrow = arrow;
        infoLabel->line = line;
        this->info[key] = infoLabel;
    }

    void updatingRatingStats()
    {
        if (!isAuth())
            return;

        QtConcurrent::run([this]()
                          {
            if (this->m_apiController->update_rating_stats())
            {
                RatingStatsData ratingStatsData = this->m_apiController->getRatingStats();

                // Обновляем UI только в GUI-потоке
                QMetaObject::invokeMethod(this, [this, ratingStatsData]() {
                    setValue("Текущий рейтинг", ratingStatsData.getRating());
                    setValue("Прогресс рейтинга", ratingStatsData.getDiffRating());
                    setValue("Калибровочные бои", ratingStatsData.getCalibBattles());
                    setValue("Проведено боев",
                             ratingStatsData.getBattles(),
                             ratingStatsData.getWins(),
                             ratingStatsData.getLosses());

                    if (ratingStatsData.getBattles() > 0)
                    {
                        setValue("Победы", ratingStatsData.getPercentWins());
                        setValue("Урон", ratingStatsData.getAvgDamage());
                        setValue("Опыт", ratingStatsData.getAvgExp());
                    }

                    updateColor(ratingStatsData);
                    prev_stats = ratingStatsData;
                }, Qt::QueuedConnection);
            }
            else
            {
                qDebug() << "Not updated stats RATING";
            } });
    }

    void setValue(std::string key, int64_t value)
    {
        std::string formatted = formatInt(value);
        QString new_str = QString::fromStdString(formatted);

        this->info[key]->value->setText(new_str);
    }

    void
    setValue(std::string key, int64_t value, int64_t wins, int64_t losses)
    {
        std::string formatted = formatInt(value) + " [" + formatInt(wins) + " - " + formatInt(losses) + "]";
        QString new_str = QString::fromStdString(formatted);
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

    void updateColor(const RatingStatsData &ratingStatsData)
    {
        RatingStatsData prevCopy = this->prev_stats;

        if (ratingStatsData.getRating() > prevCopy.getRating())
        {
            this->info["Текущий рейтинг"]->arrow->setText("▲");
            this->info["Текущий рейтинг"]->arrow->setStyleSheet("color: #66ff66;");
            this->info["Текущий рейтинг"]->line->setStyleSheet("background-color: #66ff66;");
        }
        else if (ratingStatsData.getRating() < prevCopy.getRating())
        {
            this->info["Текущий рейтинг"]->arrow->setText("▼");
            this->info["Текущий рейтинг"]->arrow->setStyleSheet("color: #ff6666;");
            this->info["Текущий рейтинг"]->line->setStyleSheet("background-color: #ff6666;");
        }

        if (ratingStatsData.getDiffRating() > prevCopy.getDiffRating())
        {
            this->info["Прогресс рейтинга"]->arrow->setText("▲");
            this->info["Прогресс рейтинга"]->arrow->setStyleSheet("color: #66ff66;");
            this->info["Прогресс рейтинга"]->line->setStyleSheet("background-color: #66ff66;");
        }
        else if (ratingStatsData.getDiffRating() < prevCopy.getDiffRating())
        {
            this->info["Прогресс рейтинга"]->arrow->setText("▼");
            this->info["Прогресс рейтинга"]->arrow->setStyleSheet("color: #ff6666;");
            this->info["Прогресс рейтинга"]->line->setStyleSheet("background-color: #ff6666;");
        }

        if (ratingStatsData.getCalibBattles() > prevCopy.getCalibBattles())
        {
            this->info["Калибровочные бои"]->arrow->setText("▲");
            this->info["Калибровочные бои"]->arrow->setStyleSheet("color: #66ff66;");
            this->info["Калибровочные бои"]->line->setStyleSheet("background-color: #66ff66;");
        }
        else if (ratingStatsData.getCalibBattles() < prevCopy.getCalibBattles())
        {
            this->info["Калибровочные бои"]->arrow->setText("▼");
            this->info["Калибровочные бои"]->arrow->setStyleSheet("color: #ff6666;");
            this->info["Калибровочные бои"]->line->setStyleSheet("background-color: #ff6666;");
        }

        if (ratingStatsData.getBattles() > prevCopy.getBattles())
        {
            this->info["Проведено боев"]->arrow->setText("▲");
            this->info["Проведено боев"]->arrow->setStyleSheet("color: #66ff66;");
            this->info["Проведено боев"]->line->setStyleSheet("background-color: #66ff66;");
        }
        else if (ratingStatsData.getBattles() < prevCopy.getBattles())
        {
            this->info["Проведено боев"]->arrow->setText("▼");
            this->info["Проведено боев"]->arrow->setStyleSheet("color: #ff6666;");
            this->info["Проведено боев"]->line->setStyleSheet("background-color: #ff6666;");
        }

        if (ratingStatsData.getPercentWins() > prevCopy.getPercentWins())
        {
            this->info["Победы"]->arrow->setText("▲");
            this->info["Победы"]->arrow->setStyleSheet("color: #66ff66;");
            this->info["Победы"]->line->setStyleSheet("background-color: #66ff66;");
        }
        else if (ratingStatsData.getPercentWins() < prevCopy.getPercentWins())
        {
            this->info["Победы"]->arrow->setText("▼");
            this->info["Победы"]->arrow->setStyleSheet("color: #ff6666;");
            this->info["Победы"]->line->setStyleSheet("background-color: #ff6666;");
        }

        if (ratingStatsData.getAvgDamage() > prevCopy.getAvgDamage())
        {
            this->info["Урон"]->arrow->setText("▲");
            this->info["Урон"]->arrow->setStyleSheet("color: #66ff66;");
            this->info["Урон"]->line->setStyleSheet("background-color: #66ff66;");
        }
        else if (ratingStatsData.getAvgDamage() < prevCopy.getAvgDamage())
        {
            this->info["Урон"]->arrow->setText("▼");
            this->info["Урон"]->arrow->setStyleSheet("color: #ff6666;");
            this->info["Урон"]->line->setStyleSheet("background-color: #ff6666;");
        }

        if (ratingStatsData.getAvgExp() > prevCopy.getAvgExp())
        {
            this->info["Опыт"]->arrow->setText("▲");
            this->info["Опыт"]->arrow->setStyleSheet("color: #66ff66;");
            this->info["Опыт"]->line->setStyleSheet("background-color: #66ff66;");
        }
        else if (ratingStatsData.getAvgExp() < prevCopy.getAvgExp())
        {
            this->info["Опыт"]->arrow->setText("▼");
            this->info["Опыт"]->arrow->setStyleSheet("color: #ff6666;");
            this->info["Опыт"]->line->setStyleSheet("background-color: #ff6666;");
        }
    }

    void resetValue()
    {
        RatingStatsData ratingStatsData = this->m_apiController->getRatingStats();
        this->prev_stats = ratingStatsData;
        this->info["Текущий рейтинг"]->value->setText("-");
        this->info["Текущий рейтинг"]->arrow->setText("");
        this->info["Текущий рейтинг"]->arrow->setStyleSheet("color: #e2ded3;");
        this->info["Текущий рейтинг"]->line->setStyleSheet("background-color: #e2ded3;");

        this->info["Прогресс рейтинга"]->value->setText("-");
        this->info["Прогресс рейтинга"]->arrow->setText("");
        this->info["Прогресс рейтинга"]->arrow->setStyleSheet("color: #e2ded3;");
        this->info["Прогресс рейтинга"]->line->setStyleSheet("background-color: #e2ded3;");

        this->info["Калибровочные бои"]->value->setText("-");
        this->info["Калибровочные бои"]->arrow->setText("");
        this->info["Калибровочные бои"]->arrow->setStyleSheet("color: #e2ded3;");
        this->info["Калибровочные бои"]->line->setStyleSheet("background-color: #e2ded3;");

        this->info["Проведено боев"]->value->setText("-");
        this->info["Проведено боев"]->arrow->setText("");
        this->info["Проведено боев"]->arrow->setStyleSheet("color: #e2ded3;");
        this->info["Проведено боев"]->line->setStyleSheet("background-color: #e2ded3;");

        this->info["Победы"]->value->setText("-");
        this->info["Победы"]->arrow->setText("");
        this->info["Победы"]->arrow->setStyleSheet("color: #e2ded3;");
        this->info["Победы"]->line->setStyleSheet("background-color: #e2ded3;");

        this->info["Урон"]->value->setText("-");
        this->info["Урон"]->arrow->setText("");
        this->info["Урон"]->arrow->setStyleSheet("color: #e2ded3;");
        this->info["Урон"]->line->setStyleSheet("background-color: #e2ded3;");

        this->info["Опыт"]->value->setText("-");
        this->info["Опыт"]->arrow->setText("");
        this->info["Опыт"]->arrow->setStyleSheet("color: #e2ded3;");
        this->info["Опыт"]->line->setStyleSheet("background-color: #e2ded3;");
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
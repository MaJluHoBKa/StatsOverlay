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
#include <main_overlay/controller/ApiController.h>

#include <unordered_map>
#include <sstream>
#include <cstdint>
#include <iomanip>
#include <string>
#include <iostream>
#include <thread>

struct InfoLabel
{
    QLabel *value = nullptr;
    QLabel *arrow = nullptr;
    QFrame *line = nullptr;
};

class MainStats : public QWidget
{
    Q_OBJECT

private:
    std::unordered_map<std::string, InfoLabel *> info;
    MainStatsData prev_stats;

    ApiController *m_apiController = nullptr;

public:
    explicit MainStats(ApiController *apiController, QWidget *parent = nullptr);
    void addContent(QVBoxLayout *mainLayout, QPixmap icon_source, QString title, bool is_top, bool is_bottom);

    void appendInfoLabel(std::string key, QLabel *value, QLabel *arrow, QFrame *line)
    {
        InfoLabel *infoLabel = new InfoLabel;
        infoLabel->value = value;
        infoLabel->arrow = arrow;
        infoLabel->line = line;
        this->info[key] = infoLabel;
    }

    void updatingMainStats()
    {
        if (isAuth())
        {
            if (this->m_apiController->update_main_stats())
            {
                MainStatsData mainStatsData = this->m_apiController->getMainStats();
                setValue("Золото", mainStatsData.getGold());
                setValue("Кредиты", mainStatsData.getCredits());
                setValue("Боевой опыт", mainStatsData.getExpBattle());
                setValue("Свободный опыт", mainStatsData.getExpFree());
                setValue("Проведено боев", mainStatsData.getBattles(), mainStatsData.getWins(), mainStatsData.getLosses());
                if (mainStatsData.getBattles() > 0)
                {
                    setValue("Победы", mainStatsData.getPercentWins());
                    setValue("Урон", mainStatsData.getAvgDamage());
                    setValue("Опыт", mainStatsData.getAvgExp());
                }
                updateColor(mainStatsData);
                prev_stats = mainStatsData;
            }
        }
    }

    void setValue(std::string key, int64_t value)
    {
        std::string formatted = formatInt(value);
        QString new_str = QString::fromStdString(formatted);
        QMetaObject::invokeMethod(
            this,
            [this, key, new_str]()
            {
                this->info[key]->value->setText(new_str);
            },
            Qt::QueuedConnection);
    }

    void setValue(std::string key, int64_t value, int64_t wins, int64_t losses)
    {
        std::string formatted = formatInt(value) + " [" + formatInt(wins) + " - " + formatInt(losses) + "]";
        QString new_str = QString::fromStdString(formatted);
        QMetaObject::invokeMethod(
            this,
            [this, key, new_str]()
            {
                this->info[key]->value->setText(new_str);
            },
            Qt::QueuedConnection);
    }

    void setValue(std::string key, float value)
    {
        std::ostringstream oss;
        oss << formatFloat(value) << "%";
        std::string formatted = oss.str();
        QString new_str = QString::fromStdString(formatted);
        QMetaObject::invokeMethod(
            this,
            [this, key, new_str]()
            {
                this->info[key]->value->setText(new_str);
            },
            Qt::QueuedConnection);
    }

    void updateColor(const MainStatsData &mainStatsData)
    {
        MainStatsData prevCopy = this->prev_stats;
        QMetaObject::invokeMethod(
            this,
            [this, prevCopy, mainStatsData]()
            {
                if (mainStatsData.getGold() > prevCopy.getGold())
                {
                    this->info["Золото"]->arrow->setText("▲");
                    this->info["Золото"]->arrow->setStyleSheet("color: #00ff00;");
                    this->info["Золото"]->line->setStyleSheet("background-color: #66ff66;");
                }
                else if (mainStatsData.getGold() < prevCopy.getGold())
                {
                    this->info["Золото"]->arrow->setText("▼");
                    this->info["Золото"]->arrow->setStyleSheet("color: #ff0000;");
                    this->info["Золото"]->line->setStyleSheet("background-color: #ff6666;");
                }

                if (mainStatsData.getCredits() > prevCopy.getCredits())
                {
                    this->info["Кредиты"]->arrow->setText("▲");
                    this->info["Кредиты"]->arrow->setStyleSheet("color: #00ff00;");
                    this->info["Кредиты"]->line->setStyleSheet("background-color: #66ff66;");
                }
                else if (mainStatsData.getCredits() < prevCopy.getCredits())
                {
                    this->info["Кредиты"]->arrow->setText("▼");
                    this->info["Кредиты"]->arrow->setStyleSheet("color: #ff0000;");
                    this->info["Кредиты"]->line->setStyleSheet("background-color: #ff6666;");
                }

                if (mainStatsData.getExpBattle() > prevCopy.getExpBattle())
                {
                    this->info["Боевой опыт"]->arrow->setText("▲");
                    this->info["Боевой опыт"]->arrow->setStyleSheet("color: #00ff00;");
                    this->info["Боевой опыт"]->line->setStyleSheet("background-color: #66ff66;");
                }
                else if (mainStatsData.getExpBattle() < prevCopy.getExpBattle())
                {
                    this->info["Боевой опыт"]->arrow->setText("▼");
                    this->info["Боевой опыт"]->arrow->setStyleSheet("color: #ff0000;");
                    this->info["Боевой опыт"]->line->setStyleSheet("background-color: #ff6666;");
                }

                if (mainStatsData.getExpFree() > prevCopy.getExpFree())
                {
                    this->info["Свободный опыт"]->arrow->setText("▲");
                    this->info["Свободный опыт"]->arrow->setStyleSheet("color: #00ff00;");
                    this->info["Свободный опыт"]->line->setStyleSheet("background-color: #66ff66;");
                }
                else if (mainStatsData.getExpFree() < prevCopy.getExpFree())
                {
                    this->info["Свободный опыт"]->arrow->setText("▼");
                    this->info["Свободный опыт"]->arrow->setStyleSheet("color: #ff0000;");
                    this->info["Свободный опыт"]->line->setStyleSheet("background-color: #ff6666;");
                }
                if (mainStatsData.getBattles() > prevCopy.getBattles())
                {
                    this->info["Проведено боев"]->arrow->setText("▲");
                    this->info["Проведено боев"]->arrow->setStyleSheet("color: #00ff00;");
                    this->info["Проведено боев"]->line->setStyleSheet("background-color: #66ff66;");
                }
                else if (mainStatsData.getBattles() < prevCopy.getBattles())
                {
                    this->info["Проведено боев"]->arrow->setText("▼");
                    this->info["Проведено боев"]->arrow->setStyleSheet("color: #ff0000;");
                    this->info["Проведено боев"]->line->setStyleSheet("background-color: #ff6666;");
                }

                if (mainStatsData.getPercentWins() > prevCopy.getPercentWins())
                {
                    this->info["Победы"]->arrow->setText("▲");
                    this->info["Победы"]->arrow->setStyleSheet("color: #00ff00;");
                    this->info["Победы"]->line->setStyleSheet("background-color: #66ff66;");
                }
                else if (mainStatsData.getPercentWins() < prevCopy.getPercentWins())
                {
                    this->info["Победы"]->arrow->setText("▼");
                    this->info["Победы"]->arrow->setStyleSheet("color: #ff0000;");
                    this->info["Победы"]->line->setStyleSheet("background-color: #ff6666;");
                }

                if (mainStatsData.getAvgDamage() > prevCopy.getAvgDamage())
                {
                    this->info["Урон"]->arrow->setText("▲");
                    this->info["Урон"]->arrow->setStyleSheet("color: #00ff00;");
                    this->info["Урон"]->line->setStyleSheet("background-color: #66ff66;");
                }
                else if (mainStatsData.getAvgDamage() < prevCopy.getAvgDamage())
                {
                    this->info["Урон"]->arrow->setText("▼");
                    this->info["Урон"]->arrow->setStyleSheet("color: #ff0000;");
                    this->info["Урон"]->line->setStyleSheet("background-color: #ff6666;");
                }

                if (mainStatsData.getAvgExp() > prevCopy.getAvgExp())
                {
                    this->info["Опыт"]->arrow->setText("▲");
                    this->info["Опыт"]->arrow->setStyleSheet("color: #00ff00;");
                    this->info["Опыт"]->line->setStyleSheet("background-color: #66ff66;");
                }
                else if (mainStatsData.getAvgExp() < prevCopy.getAvgExp())
                {
                    this->info["Опыт"]->arrow->setText("▼");
                    this->info["Опыт"]->arrow->setStyleSheet("color: #ff0000;");
                    this->info["Опыт"]->line->setStyleSheet("background-color: #ff6666;");
                }
            },
            Qt::QueuedConnection);
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
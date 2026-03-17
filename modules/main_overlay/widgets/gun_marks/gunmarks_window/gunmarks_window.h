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
#include <QPainter>
#include <QMouseEvent>
#include <QProgressBar>
#include <QtConcurrent/QtConcurrent>
#include <main_overlay/controller/ApiController.h>

#include <unordered_map>
#include <sstream>
#include <cstdint>
#include <iomanip>
#include <string>
#include <iostream>
#include <thread>

class GunMarksWindow : public QWidget
{
    Q_OBJECT

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    bool m_dragActive = false;
    QPoint m_dragStartPos;
    double m_backgroundOpacity;

    QLabel *percent = nullptr;
    QLabel *inBattle = nullptr;
    QLabel *next05 = nullptr;
    QLabel *next05_text = nullptr;
    QLabel *next1 = nullptr;
    QLabel *next1_text = nullptr;
    QLabel *next2 = nullptr;
    QLabel *next2_text = nullptr;
    QLabel *shots = nullptr;
    QLabel *currentSummaryDamage = nullptr;
    QLabel *reqAvgDamage = nullptr;

    QProgressBar *progress = nullptr;

public:
    explicit GunMarksWindow(QWidget *parent = nullptr);

    void init(int64_t mark)
    {
        this->currentSummaryDamage->setText("0/" + QString::fromStdString(formatInt(mark)));
        this->reqAvgDamage->setText(QString::fromStdString(formatInt(mark)));
    }

    void setData(int64_t damage, int64_t mark, int64_t firstDamage)
    {
        int64_t totalPlannedDamage = mark * 100;
        float sumDamage = static_cast<float>(damage) / static_cast<float>(100);
        float curPercent = std::round((static_cast<float>(damage) / static_cast<float>(totalPlannedDamage)) * 10000.0f) / 100.0f;

        float lostPercent = 0.0;
        int64_t totalPostDamage = damage - firstDamage;
        float postSumDamage = totalPostDamage / 100;
        float postPercent = std::round(static_cast<float>(((postSumDamage * 100.0f) / mark)) * 100.0f) / 100.0f;
        lostPercent = postPercent - curPercent;

        int64_t damageAfter = damage - firstDamage;
        int64_t reqDamage = (mark * 100 - damageAfter) / 100 + sumDamage;
        if (reqDamage > 0)
        {
            this->reqAvgDamage->setText(QString::fromStdString(formatInt(reqDamage)));
        }
        else
        {
            this->reqAvgDamage->setText(QString::fromStdString(formatInt(0)));
        }

        if (lostPercent >= 0.0)
        {
            this->inBattle->setText("+" + QString::fromStdString(formatFloat(lostPercent) + "%"));
        }
        else
        {
            this->inBattle->setText(QString::fromStdString(formatFloat(lostPercent) + "%"));
        }

        this->currentSummaryDamage->setText(QString::fromStdString(formatInt(static_cast<int64_t>(sumDamage))) + "/" + QString::fromStdString(formatInt(mark)));

        int64_t per05 = (mark * 100) / 200 + static_cast<int64_t>(firstDamage);
        this->next05->setText(QString::fromStdString(formatInt(per05)));

        int64_t per1 = (mark * 100) / 100 + static_cast<int64_t>(firstDamage);
        this->next1->setText(QString::fromStdString(formatInt(per1)));

        int64_t per2 = (mark * 100) / 50 + static_cast<int64_t>(firstDamage);
        this->next2->setText(QString::fromStdString(formatInt(per2)));

        if (curPercent >= 100.0)
        {
            this->percent->setText("100.0%");
            this->progress->setValue(100);
            this->progress->setStyleSheet(R"(
                QProgressBar {
                    background-color: rgba(50, 50, 50, 0);
                    border: 1px solid #646464;
                    border-radius: 1px;
                    height: 6px;
                }

                QProgressBar::chunk {
                    background-color: #4cd964;
                    border-radius: 1px;
                }  
                )");
        }
        else
        {
            this->percent->setText(QString::fromStdString(formatFloat(curPercent)) + "%");
            this->progress->setValue(std::round(curPercent));
            this->progress->setStyleSheet(R"(
                QProgressBar {
                    background-color: rgba(50, 50, 50, 0);
                    border: 1px solid #646464;
                    border-radius: 1px;
                    height: 6px;
                }

                QProgressBar::chunk {
                    background-color: #ffffff;
                    border-radius: 1px;
                }
            )");
        }
    }

    void clearData()
    {
        this->percent->setText("0.0%");
        this->inBattle->setText("0.0%");
        this->currentSummaryDamage->setText("0");
        this->reqAvgDamage->setText("0");
        this->next05->setText("0");
        this->next1->setText("0");
        this->next2->setText("0");
        this->progress->setValue(0);
    }

    std::string formatFloat(float value)
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << value;

        // Если хотите убрать лишние нули (опционально)
        std::string result = oss.str();
        if (result.find('.') != std::string::npos)
        {
            // Удаляем незначащие нули в конце
            result.erase(result.find_last_not_of('0') + 1, std::string::npos);
            // Если осталась только точка, удаляем и ее
            if (result.back() == '.')
            {
                result.pop_back();
            }
        }
        return result;
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

    void toggleNext(bool visible)
    {
        this->next05->setVisible(visible);
        this->next05_text->setVisible(visible);
        this->next1->setVisible(visible);
        this->next1_text->setVisible(visible);
        this->next2->setVisible(visible);
        this->next2_text->setVisible(visible);
        updateGeometry();
        QTimer::singleShot(0, this, [this]()
                           { adjustSize(); });
    }

    void toggleShots(bool visible)
    {
        // for (int i = 0; i < this->rows_players.size(); i++)
        // {
        //     this->rows_players[i]->tank_name->setVisible(visible);
        // }
        // this->avg->tank_name->setVisible(visible);
        // this->tankIcon->setVisible(visible);
        // if (visible)
        // {
        //     defaultWidth += 105;
        // }
        // else
        // {
        //     defaultWidth -= 105;
        // }
        // updateWindowSize();
    }

    // void updateWindowSize()
    // {
    //     if (defaultWidth < 150)
    //     {
    //         setFixedWidth(150);
    //     }
    //     else
    //     {
    //         setFixedWidth(defaultWidth);
    //     }
    // }

    void setOpacity(double opacity)
    {
        this->m_backgroundOpacity = opacity;
    }
};
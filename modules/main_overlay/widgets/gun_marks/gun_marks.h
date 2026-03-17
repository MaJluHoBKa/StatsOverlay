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
#include <QLineEdit>
#include <QIntValidator>
#include <QtConcurrent/QtConcurrent>
#include <main_overlay/controller/ApiController.h>
#include <main_overlay/widgets/gun_marks/gunmarks_window/gunmarks_window.h>

#include <unordered_map>
#include <sstream>
#include <cstdint>
#include <iomanip>
#include <string>
#include <iostream>
#include <thread>

class GunMarks : public QWidget
{
    Q_OBJECT

private slots:
    void updatingMarkStats();
    void toggleNext();
    void toggleShots();

private:
    ApiController *m_apiController = nullptr;

    GunMarksWindow *gunMarksWindow = new GunMarksWindow();

    bool isMarks = false;

    QPushButton *active = nullptr;
    bool isOn = false;
    bool isFirst = true;

    QPushButton *nextPercent = nullptr;
    QPushButton *countShots = nullptr;

    QLabel *nextCheck = nullptr;
    QLabel *shotsCheck = nullptr;

    QLineEdit *tf = nullptr;
    QLineEdit *btf = nullptr;
    QLabel *tl = nullptr;
    QLabel *tl_label = nullptr;

    bool is_next = true;
    bool is_shots = true;

    std::string tankName = "";
    int64_t clearedDamage = 0;
    int64_t damageForBattle = 0;
    int64_t totalDamage = 0;
    int64_t firstDamage = 0;
    std::vector<int64_t> damages;
    int64_t currentDamage = 0;
    int64_t battles = 0;
    int64_t mark = 0;

    double m_backgroundOpacity;

    double hotVisible = false;

public:
    explicit GunMarks(ApiController *apiController, QWidget *parent = nullptr);

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
            if (!tf->text().isEmpty() && !btf->text().isEmpty())
            {
                this->mark = tf->text().toInt();
                this->battles = 100;
                damages.clear();
                for (int i = 0; i < 100; i++)
                {
                    damages.push_back(btf->text().toInt());
                }
                this->currentDamage = btf->text().toInt() * 100;
                if (this->mark > 0)
                {
                    this->active->setStyleSheet(activeStyle);
                    this->active->setText("Закончить");
                    gunMarksWindow->show();
                    gunMarksWindow->init(mark);
                    this->isOn = true;
                    this->isMarks = true;
                    hotVisible = true;
                    m_apiController->setMark(true);
                }
            }
        }
        else
        {
            tl_label->setPixmap(QIcon(":gun_marks/resources/icons/unlock_icon.svg").pixmap(16, 16));
            tl->setText("");
            this->damageForBattle = 0;
            this->damages.resize(0);
            this->battles = 100;
            this->mark = 0;
            this->active->setStyleSheet(baseStyle);
            this->active->setText("Начать");
            gunMarksWindow->hide();
            gunMarksWindow->clearData();
            this->isOn = false;
            this->isMarks = false;
            this->isFirst = true;
            hotVisible = false;
            m_apiController->setMark(false);
            m_apiController->setTankLock(false);
        }
    }

    void updateInfo(std::string tankName, int64_t totalDamage)
    {
        if (isFirst)
        {
            this->tankName = tankName;
            tl_label->setPixmap(QIcon(":gun_marks/resources/icons/lock_icon.svg").pixmap(16, 16));
            tl->setText(QString::fromStdString(tankName));
            isFirst = !isFirst;
        }
        damageForBattle = totalDamage - this->totalDamage;
        this->totalDamage = totalDamage;

        clearedDamage = damages[0];

        damages.push_back(damageForBattle);
        damages.erase(damages.begin());
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

    // void hotHide()
    // {
    //     if (hotVisible)
    //     {
    //         //alliesWindow->hide();
    //        // enemiesWindow->hide();
    //     }
    //     else
    //     {
    //         //alliesWindow->show();
    //         //enemiesWindow->show();
    //     }
    //     hotVisible = !hotVisible;
    // }
};
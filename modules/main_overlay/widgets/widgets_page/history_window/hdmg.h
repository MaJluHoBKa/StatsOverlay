#pragma once

#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QScreen>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSizePolicy>
#include <QPainter>
#include <QPixmap>
#include <QVector>

#include <main_overlay/controller/ReplayTracker.h>
#include <main_overlay/controller/ApiController.h>

class HistoryWidget : public QWidget
{
    Q_OBJECT

protected:
    void paintEvent(QPaintEvent *) override {}

private:
    ApiController *m_apiController = nullptr;

    struct Row
    {
        QWidget *widget = nullptr;
        QLabel *dmg = nullptr;
        QLabel *name = nullptr;
    };

    static constexpr int MAX_ROWS = 10;
    static constexpr int ROW_H = 25;

    QVBoxLayout *m_layout = nullptr;
    QVector<Row> m_rows;

    static QString labelStyle()
    {
        return "background-color: transparent;"
               "font-family: Segoe UI;"
               "font-size: 14px;"
               "font-weight: bold;"
               "padding-left: 30px;"
               "color: #e2ded3;";
    }

    static QString redLabelStyle()
    {
        return "background-color: transparent;"
               "font-family: Segoe UI;"
               "font-size: 14px;"
               "font-weight: bold;"
               "padding-left: 30px;"
               "color: #f09e9e;";
    }

    static QString greenLabelStyle()
    {
        return "background-color: transparent;"
               "font-family: Segoe UI;"
               "font-size: 14px;"
               "font-weight: bold;"
               "padding-left: 30px;"
               "color: #a3f09b;";
    }

    static QString blockLabelStyle()
    {
        return "background-color: transparent;"
               "font-family: Segoe UI;"
               "font-size: 14px;"
               "font-weight: bold;"
               "padding-left: 30px;"
               "color: #dbdbdb;";
    }

public:
    explicit HistoryWidget(ApiController *apiController = nullptr, QWidget *parent = nullptr)
        : m_apiController(apiController), QWidget(parent)
    {
        setFixedWidth(175);
        setContentsMargins(0, 0, 0, 0);
        setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::WindowTransparentForInput | Qt::Tool);
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_TransparentForMouseEvents);

        QRect screenGeometry = QApplication::primaryScreen()->availableGeometry();
        int screenWidth = screenGeometry.width();
        int screenHeight = screenGeometry.height();

        int x = 0;
        int y = screenHeight - height();
        move(x, y);

        m_layout = new QVBoxLayout(this);
        m_layout->setSpacing(0);
        m_layout->setContentsMargins(0, 0, 0, 0);
        setLayout(m_layout);

        for (int i = 0; i < MAX_ROWS; ++i)
        {
            Row r;

            r.widget = new QWidget(this);
            r.widget->setFixedHeight(ROW_H);
            r.widget->setAttribute(Qt::WA_TransparentForMouseEvents);

            QHBoxLayout *lay = new QHBoxLayout(r.widget);
            lay->setContentsMargins(0, 0, 0, 0);
            lay->setSpacing(0);

            r.dmg = new QLabel(r.widget);
            r.dmg->setFixedWidth(175);
            r.dmg->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
            r.dmg->setStyleSheet(labelStyle());
            r.dmg->setAttribute(Qt::WA_TransparentForMouseEvents);

            lay->addWidget(r.dmg);
            r.widget->setLayout(lay);

            r.widget->hide();
            m_rows.append(r);
            m_layout->addWidget(r.widget);
        }
    }

    void setHistory(const QVector<BattleEvent> &history)
    {
        // Только Damage и Incoming
        QVector<const BattleEvent *> filtered;
        for (const BattleEvent &ev : history)
            if (ev.type == BattleEvent::Damage || ev.type == BattleEvent::Incoming || ev.type == BattleEvent::Blocked)
                filtered.append(&ev);

        int total = filtered.size();
        int start = qMax(0, total - MAX_ROWS);
        int count = total - start;

        for (int i = 0; i < MAX_ROWS; ++i)
        {
            if (i >= count)
            {
                m_rows[i].widget->hide();
                continue;
            }

            const BattleEvent &ev = *filtered[total - 1 - i];

            if (ev.type == BattleEvent::Damage)
            {
                QString tank = (QString::fromStdString(m_apiController->getVehicleName(ev.vehicle_cd)) != "0")
                                   ? QString::fromStdString(m_apiController->getVehicleName(ev.vehicle_cd))
                                   : "???";
                m_rows[i].dmg->setText(formatInt(ev.damage) + " - " + tank);
                //  + " - " + QString::fromStdString(m_apiController->getVehicleName(ev.vehicle_cd))
                m_rows[i].dmg->setStyleSheet(greenLabelStyle());

                QString bg = "QWidget { background-image: url(:hdmg/resources/icons/damage_green.png); background-repeat: no-repeat; background-position: left center; }"
                             "QLabel { background: transparent; }";
                m_rows[i].widget->setStyleSheet(bg);
            }
            else if (ev.type == BattleEvent::Incoming)
            {
                QString tank = (QString::fromStdString(m_apiController->getVehicleName(ev.vehicle_cd)) != "0")
                                   ? QString::fromStdString(m_apiController->getVehicleName(ev.vehicle_cd))
                                   : "???";
                m_rows[i].dmg->setText(formatInt(ev.damage) + " - " + tank);
                //  + " - " + QString::fromStdString(m_apiController->getVehicleName(ev.vehicle_cd))
                m_rows[i].dmg->setStyleSheet(redLabelStyle());

                QString bg = "QWidget { background-image: url(:hdmg/resources/icons/damage_red.png); background-repeat: no-repeat; background-position: left center; }"
                             "QLabel { background: transparent; }";
                m_rows[i].widget->setStyleSheet(bg);
            }

            else if (ev.type == BattleEvent::Blocked)
            {
                QString tank = (QString::fromStdString(m_apiController->getVehicleName(ev.vehicle_cd)) != "0")
                                   ? QString::fromStdString(m_apiController->getVehicleName(ev.vehicle_cd))
                                   : "???";
                m_rows[i].dmg->setText(formatInt(ev.damage) + " - " + tank);
                // + " - " + QString::fromStdString(m_apiController->getVehicleName(ev.vehicle_cd))
                m_rows[i].dmg->setStyleSheet(blockLabelStyle());

                QString bg = "QWidget { background-image: url(:hdmg/resources/icons/damage_block.png); background-repeat: no-repeat; background-position: left center; }"
                             "QLabel { background: transparent; }";
                m_rows[i].widget->setStyleSheet(bg);
            }

            m_rows[i].widget->show();
            m_rows[i].widget->show();
        }

        setFixedHeight(count * ROW_H);
    }

    void clearData()
    {
        setHistory({});
    }

    QString formatInt(int64_t value)
    {
        QString str = QString::number(value);
        int insertPosition = str.length() - 3;
        while (insertPosition > 0)
        {
            str.insert(insertPosition, " ");
            insertPosition -= 3;
        }
        return str;
    }
};
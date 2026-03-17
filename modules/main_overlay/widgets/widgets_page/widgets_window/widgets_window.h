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
#include <main_overlay/widgets/widgets_page/widgets_window/FlowLayout.h>

#include <unordered_map>
#include <sstream>
#include <cstdint>
#include <iomanip>
#include <string>
#include <iostream>
#include <thread>

class WidgetsWindow : public QWidget
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
    double m_backgroundOpacity = 0.85;

    FlowLayout *flowLayout = nullptr;
    QMap<QString, QWidget *> tiles;
    QMap<QString, QLabel *> valueLabels;

    QWidget *createTile(const QString &label, const QString &value)
    {
        QWidget *tile = new QWidget;
        tile->setFixedHeight(50);
        tile->setFixedWidth(80);
        tile->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        QVBoxLayout *l = new QVBoxLayout(tile);
        l->setContentsMargins(0, 5, 0, 5);
        l->setAlignment(Qt::AlignCenter);

        QLabel *val = new QLabel(value);
        val->setStyleSheet("color: #e2ded3; font-family: Segoe UI; font-size: 16px; font-weight: bold; background-color: transparent;");
        val->setAlignment(Qt::AlignCenter);
        valueLabels[label] = val;

        QLabel *lbl = new QLabel(label);
        lbl->setStyleSheet("color: #555555; font-family: Segoe UI; font-size: 10px; background-color: transparent;");
        lbl->setAlignment(Qt::AlignCenter);

        l->addWidget(val);
        l->addWidget(lbl);
        return tile;
    }

public:
    explicit WidgetsWindow(QWidget *parent = nullptr);

    void addTile(const QString &label, const QString &value = "—")
    {
        if (!tiles.contains(label))
        {
            QWidget *tile = createTile(label, value);
            tiles[label] = tile;
            flowLayout->addWidget(tile);
        }
    }

    void setTileVisible(const QString &label, bool visible)
    {
        if (tiles.contains(label))
        {
            tiles[label]->setVisible(visible);
            flowLayout->invalidate();
            adjustSize();
        }
    }

    void updateTile(const QString &label, const QString &value)
    {
        if (valueLabels.contains(label))
            valueLabels[label]->setText(value);
    }

    void updateWins(const float &value)
    {
        if (!valueLabels.contains("Победы"))
            return;

        QString color;
        if (value >= 70.0f)
            color = "#9989e6";
        else if (value >= 60.0f)
            color = "#72d1ff";
        else if (value >= 50.0f)
            color = "#a8e689";
        else
            color = "#ffffff";

        QLabel *label = valueLabels["Победы"];
        label->setText(QString::fromStdString(formatFloat(value)) + "%");
        label->setStyleSheet(QString("color: %1; font-family: Segoe UI; font-size: 16px; font-weight: bold; background-color: transparent;").arg(color));
    }

    void toggleBattles(bool visible) { setTileVisible("Бои", visible); }
    void toggleWins(bool visible) { setTileVisible("Победы", visible); }
    void toggleDamage(bool visible) { setTileVisible("Урон", visible); }
    void toggleXp(bool visible) { setTileVisible("Опыт", visible); }
    void toggleAlive(bool visible) { setTileVisible("Выжил", visible); }
    void toggleShots(bool visible) { setTileVisible("Точность", visible); }
    void toggleRating(bool visible) { setTileVisible("Рейтинг", visible); }
    void toggleMaster(bool visible) { setTileVisible("Мастер", visible); }

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

    void setOpacity(double opacity)
    {
        m_backgroundOpacity = opacity;

        int alpha = int(opacity * 255);
        QString style = QString("background-color: rgba(35,35,35,%1); border-radius: 5px;").arg(alpha);

        for (auto &tile : tiles)
            tile->setStyleSheet(style);

        update();
    }
};
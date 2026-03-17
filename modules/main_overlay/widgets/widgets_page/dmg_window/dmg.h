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
#include <QtConcurrent/QtConcurrent>
#include <main_overlay/controller/ApiController.h>

#include <sstream>
#include <cstdint>
#include <iomanip>
#include <string>
#include <iostream>
#include <thread>

struct BattleData
{
    int64_t damage;
    int64_t block;
    int64_t assist;
};

class DamageWidget : public QWidget
{
    Q_OBJECT

protected:
    void paintEvent(QPaintEvent *) override;

private:
    QLabel *damage;
    QLabel *block;
    QLabel *assist;

public:
    explicit DamageWidget(QWidget *parent = nullptr);

    void setData(const BattleData &data)
    {
        this->damage->setText(formatInt(data.damage));
        this->block->setText(formatInt(data.block));
        this->assist->setText(formatInt(data.assist));
    }

    void clearData()
    {
        this->damage->setText("0");
        this->block->setText("0");
        this->assist->setText("0");
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
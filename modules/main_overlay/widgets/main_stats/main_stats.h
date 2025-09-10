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

class MainStats : public QWidget
{
    Q_OBJECT

private:
public:
    explicit MainStats(QWidget *parent = nullptr);
    void addContent(QVBoxLayout *mainLayout, QPixmap icon_source, QString title, bool is_top, bool is_bottom);
};
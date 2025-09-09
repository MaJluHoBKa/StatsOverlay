#pragma once

#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QScreen>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSizePolicy>

class MainOverlay : public QWidget
{
    Q_OBJECT

private:
public:
    explicit MainOverlay(QWidget *parent = nullptr);
};
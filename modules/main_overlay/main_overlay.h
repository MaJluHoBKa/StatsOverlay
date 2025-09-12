#pragma once

#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QScreen>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSizePolicy>
#include <QPainter>
#include <QStackedWidget>
#include <main_overlay/widgets/main_stats/main_stats.h>
#include <main_overlay/widgets/rating_stats/rating_stats.h>
#include <main_overlay/widgets/vehicles_stats/vehicles_stats.h>
#include <main_overlay/widgets/info_page/info_page.h>
#include <main_overlay/controller/ApiController.h>
#include <../modules/sub_overlay/sub_overlay.h>

class MainOverlay : public QWidget
{
    Q_OBJECT

protected:
    void paintEvent(QPaintEvent *) override;

private:
    QStackedWidget *stacked_widget = nullptr;
    ApiController *m_apiController = nullptr;

private slots:
    void switchPage(int index)
    {
        this->stacked_widget->setCurrentIndex(index);
    }

public:
    explicit MainOverlay(ApiController *apiController, QWidget *parent = nullptr);

    void setStackedWidget(QStackedWidget *stacked_widget)
    {
        if (this->stacked_widget == nullptr)
        {
            this->stacked_widget = stacked_widget;
        }
    }

    QStackedWidget *getStackedWidget() const
    {
        return this->stacked_widget;
    }
};
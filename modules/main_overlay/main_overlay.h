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
#include <QMouseEvent>
#include <QMenu>
#include <QSlider>
#include <QAction>
#include <QCheckBox>
#include <QWidgetAction>
#include <QContextMenuEvent>
#include <main_overlay/widgets/main_stats/main_stats.h>
#include <main_overlay/widgets/rating_stats/rating_stats.h>
#include <main_overlay/widgets/vehicles_stats/vehicles_stats.h>
#include <main_overlay/widgets/player_stats/player_stats.h>
#include <main_overlay/widgets/info_page/info_page.h>
#include <main_overlay/controller/ApiController.h>
#include <../modules/sub_overlay/sub_overlay.h>
#include <windows.h>

class MainOverlay : public QWidget
{
    Q_OBJECT

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    QStackedWidget *stacked_widget = nullptr;
    PlayerStats *players = nullptr;
    ApiController *m_apiController = nullptr;

    QWidget *configPanel;
    double m_backgroundOpacity;

    bool m_dragActive = false;
    QPoint m_dragStartPos;
    int m_currentIndexPage = 4;

private slots:
    void switchPage(int index)
    {
        if (index == 1)
        {
            this->players->setMaximumWidth(390);
        }
        else
        {
            this->players->setMaximumWidth(280);
        }
        this->m_currentIndexPage = index;
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

    void setPlayerPage(PlayerStats *players)
    {
        if (this->players == nullptr)
        {
            this->players = players;
        }
    }

    void logout()
    {
        if (this->m_apiController->is_auth())
        {
            if (this->m_apiController->logout())
            {
                QApplication::quit();
            }
        }
        QApplication::quit();
    }

    void nextHotPage()
    {
        if (m_currentIndexPage > 0)
            m_currentIndexPage--;
        switchHotPage();
    }

    void prevHotPage()
    {
        if (m_currentIndexPage < stacked_widget->count() - 1)
            m_currentIndexPage++;
        switchHotPage();
    }

    void switchHotPage()
    {
        if (m_currentIndexPage < 0 || m_currentIndexPage >= stacked_widget->count())
            return;

        // регулируем ширину "players" если нужно
        if (m_currentIndexPage == 1)
            this->players->setMaximumWidth(390);
        else
            this->players->setMaximumWidth(280);

        // размеры окна под страницу
        int w = 300, h = 200;
        switch (m_currentIndexPage)
        {
        case 0:
            w = 300;
            h = 400;
            break; // информационная панель
        case 1:
            w = 400;
            h = 200;
            break; // игроки
        case 2:
            w = 300;
            h = 200;
            break; // танки
        case 3:
            w = 300;
            h = 200;
            break; // рейтинговая статистика
        case 4:
            w = 300;
            h = 200;
            break; // основная статистика
        }

        // сначала выставляем текущую страницу, потом resize
        stacked_widget->setCurrentIndex(m_currentIndexPage);
        this->resize(w, h);
    }
};
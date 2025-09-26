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
#include <QPropertyAnimation>
#include <main_overlay/widgets/main_stats/main_stats.h>
#include <main_overlay/widgets/rating_stats/rating_stats.h>
#include <main_overlay/widgets/other_stats/other_stats.h>
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
    void enterEvent(QEnterEvent *event) override
    {
        QWidget::enterEvent(event);
        showConfigPanel(true);
    }

    void leaveEvent(QEvent *event) override
    {
        QWidget::leaveEvent(event);
        if (!this->underMouse())
        {
            showConfigPanel(false);
        }
    }

private:
    QStackedWidget *stacked_widget = nullptr;
    PlayerStats *players = nullptr;
    VehicleStats *vehicle = nullptr;
    InfoPage *info = nullptr;
    ApiController *m_apiController = nullptr;

    QWidget *configPanel;
    double m_backgroundOpacity;

    bool m_dragActive = false;
    QPoint m_dragStartPos;
    int m_currentIndexPage = 5;
    int configPanelNormalWidth = 30;

    int defaultWidth = 280;

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

    void setVehiclePage(VehicleStats *vehicle)
    {
        if (this->vehicle == nullptr)
        {
            this->vehicle = vehicle;
        }
    }

    void setInfoPage(InfoPage *info)
    {
        if (this->info == nullptr)
        {
            this->info = info;
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

        this->players->setMaximumWidth(260);
        this->vehicle->setMaximumWidth(260);
        this->info->setMaximumWidth(260);

        if (m_currentIndexPage == 0)
            this->info->setMaximumWidth(280);
        else if (m_currentIndexPage == 1)
            this->players->setMaximumWidth(390);
        else if (m_currentIndexPage == 3)
            this->vehicle->setMaximumWidth(310);

        int w = defaultWidth, h = 200;
        switch (m_currentIndexPage)
        {
        case 0:
            w = defaultWidth;
            h = 400;
            break; // информационная панель
        case 1:
            w = defaultWidth + 120;
            h = 200;
            break; // игроки
        case 2:
            w = defaultWidth;
            h = 200;
            break; // коэффициенты
        case 3:
            w = defaultWidth + 50;
            h = 200;
            break; // танки
        case 4:
            w = defaultWidth;
            h = 200;
            break; // рейтинговая статистика
        case 5:
            w = defaultWidth;
            h = 200;
            break; // основная статистика
        }

        // сначала выставляем текущую страницу, потом resize
        stacked_widget->setCurrentIndex(m_currentIndexPage);
        this->resize(w, h);
    }

    void switchPage(int index)
    {
        m_currentIndexPage = index;
        switchHotPage();
    }

    void showConfigPanel(bool show)
    {
        // Анимация панели
        QPropertyAnimation *animPanel = new QPropertyAnimation(configPanel, "maximumWidth");
        animPanel->setDuration(200);
        animPanel->setEasingCurve(QEasingCurve::InOutQuad);

        // Анимация окна
        QPropertyAnimation *animWindow = new QPropertyAnimation(this, "geometry");
        animWindow->setDuration(200);
        animWindow->setEasingCurve(QEasingCurve::InOutQuad);

        QRect startGeom = geometry();
        QRect endGeom = startGeom;

        if (show)
        {
            configPanel->setVisible(true);

            animPanel->setStartValue(0);
            animPanel->setEndValue(configPanelNormalWidth);
            defaultWidth = 280;
            endGeom.setWidth(defaultWidth); // увеличиваем на 30 справа
        }
        else
        {
            animPanel->setStartValue(configPanel->maximumWidth());
            animPanel->setEndValue(0);
            defaultWidth = 250;
            endGeom.setWidth(defaultWidth); // возвращаем дефолтную ширину

            connect(animPanel, &QPropertyAnimation::finished, configPanel, [this]()
                    { configPanel->setVisible(false); });
        }

        animWindow->setStartValue(startGeom);
        animWindow->setEndValue(endGeom);

        animPanel->start(QAbstractAnimation::DeleteWhenStopped);
        animWindow->start(QAbstractAnimation::DeleteWhenStopped);
    }
};

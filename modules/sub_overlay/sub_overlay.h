#pragma once

#include <QApplication>
#include <QWidget>
#include <QScreen>
#include <QVBoxLayout>
#include <QLabel>
#include <QMouseEvent>

class MainOverlay;

class SubOverlay : public QWidget
{
    Q_OBJECT

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    MainOverlay *mainOverlay = nullptr;
    bool drag_active = false;
    bool is_visible = true;
    QPoint global_pos;

public:
    explicit SubOverlay(QWidget *parent = nullptr);

    void setMainWidget(MainOverlay *mainOverlay);

    void setDragActive(bool is_drag)
    {
        this->drag_active = is_drag;
    }

    bool isDragActive() const
    {
        return this->drag_active;
    }

    void setMainVisible(bool is_visible)
    {
        this->is_visible = is_visible;
    }

    bool isMainVisible() const
    {
        return this->is_visible;
    }

    void setGlobalPos(QPoint global_pos)
    {
        this->global_pos = global_pos;
    }

    QPoint getGlobalPos() const
    {
        return this->global_pos;
    }

    void mainHide();

    void snapToClosestScreenEdge();
};

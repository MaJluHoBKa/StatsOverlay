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

#include <unordered_map>

class MainStats : public QWidget
{
    Q_OBJECT

private:
    std::unordered_map<std::string, QLabel *> value_labels;
    std::unordered_map<std::string, QLabel *> arrow_labels;

public:
    explicit MainStats(QWidget *parent = nullptr);
    void addContent(QVBoxLayout *mainLayout, QPixmap icon_source, QString title, bool is_top, bool is_bottom);

    QLabel *getValueLabel(std::string key)
    {
        return this->value_labels[key];
    }

    void appendValueLabel(std::string key, QLabel *label)
    {
        this->value_labels[key] = label;
    }
};
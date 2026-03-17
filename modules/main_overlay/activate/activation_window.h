#pragma once
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QApplication>
#include <QClipboard>
#include <QTimer>
#include <QDate>
#include <QMessageBox>
#include <QCloseEvent>
#include <main_overlay/controller/HWID.h>

class ActivationWindow : public QDialog
{
    Q_OBJECT
public:
    explicit ActivationWindow(QWidget *parent = nullptr)
        : QDialog(parent)
    {
        setWindowTitle("Активация");
        setFixedSize(600, 250);
        setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint | Qt::WindowCloseButtonHint);

        setStyleSheet("background-color: rgba(30,30,30,153);");

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setContentsMargins(20, 20, 20, 20);
        layout->setSpacing(15);

        QLabel *title = new QLabel("Активация программы");
        title->setStyleSheet("font-family: Segoe UI; font-size: 18px; font-weight: bold; color: #e2ded3;");
        title->setAlignment(Qt::AlignCenter);
        layout->addWidget(title);

        QString hwid = HWIDActivator::getHWID();
        hwidLabel = new QLabel("Ваш ID:\n" + hwid);
        hwidLabel->setStyleSheet("font-family: Consolas; font-size: 12px; color: #a8e689; "
                                 "background-color: rgba(42,42,42,180); padding: 10px; border-radius: 5px;");
        hwidLabel->setAlignment(Qt::AlignCenter);
        hwidLabel->setWordWrap(true);
        hwidLabel->setFixedHeight(50);
        layout->addWidget(hwidLabel);

        QPushButton *copyBtn = new QPushButton("Копировать ID");
        copyBtn->setStyleSheet("QPushButton { background-color: rgba(56,56,56,180); color: #e2ded3; "
                               "border: 1px solid #555; border-radius: 5px; padding: 5px; } "
                               "QPushButton:hover { background-color: rgba(72,72,72,180); }");
        layout->addWidget(copyBtn);
        connect(copyBtn, &QPushButton::clicked, this, &ActivationWindow::copyHWID);

        keyInput = new QLineEdit();
        keyInput->setPlaceholderText("Введите ключ активации...");
        keyInput->setStyleSheet("QLineEdit { background-color: rgba(42,42,42,180); color: #e2ded3; "
                                "border: 1px solid #555; border-radius: 5px; padding: 8px; }");
        layout->addWidget(keyInput);

        QPushButton *activateBtn = new QPushButton("Активировать");
        activateBtn->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; border: none; "
                                   "border-radius: 5px; padding: 8px; font-weight: bold; } "
                                   "QPushButton:hover { background-color: #45a049; }");
        layout->addWidget(activateBtn);
        connect(activateBtn, &QPushButton::clicked, this, &ActivationWindow::tryActivate);
    }

protected:
    void closeEvent(QCloseEvent *event) override
    {
        reject(); // сигнал "не активировано"
        event->accept();
    }

private slots:
    void copyHWID()
    {
        QApplication::clipboard()->setText(HWIDActivator::getHWID());
        QPushButton *btn = qobject_cast<QPushButton *>(sender());
        if (btn)
        {
            QString old = btn->text();
            btn->setText("Скопировано!");
            QTimer::singleShot(2000, [btn, old]()
                               { btn->setText(old); });
        }
    }

    void tryActivate()
    {
        QString key = keyInput->text().trimmed();
        if (key.isEmpty())
        {
            keyInput->clear();
            keyInput->setPlaceholderText("Пожалуйста, введите ключ...");
            keyInput->setStyleSheet("QLineEdit { background-color: rgba(42,42,42,180); color: #ffaaaa; "
                                    "border: 1px solid #555; border-radius: 5px; padding: 8px; }");
            return;
        }

        if (HWIDActivator::activate(key))
        {
            QDate now = QDate::currentDate();
            QDate expiry(2026, 3, 31);
            if (now > expiry)
            {
                keyInput->clear();
                keyInput->setPlaceholderText("Версия устарела. Обновите приложение.");
                return;
            }
            accept(); // успех → закрыть с результатом Accepted
        }
        else
        {
            keyInput->clear();
            keyInput->setPlaceholderText("Неверный ключ активации");
            keyInput->setStyleSheet("QLineEdit { background-color: rgba(42,42,42,180); color: #ffaaaa; "
                                    "border: 1px solid #555; border-radius: 5px; padding: 8px; }");
        }
    }

private:
    QLabel *hwidLabel;
    QLineEdit *keyInput;
};

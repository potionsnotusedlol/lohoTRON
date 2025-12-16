#include "QuitGameWindow.h"

// check comments in SettingsWindow.cpp
QuitGameWindow::QuitGameWindow(QWidget* parent) : QDialog(parent) {
    fade_animation = new QPropertyAnimation(this, "windowOpacity", this);
    setWindowOpacity(0.0);
    setWindowTitle("Quit game confirmation");
    resize(400, 300);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setModal(true);
    setStyleSheet(
        "background-color: rgba(0, 0, 0, 230);"
        "color: cyan;"
        "border: 2px solid cyan;"
        "border-radius: 10px;"
        "font-family: \"Bolgarus Beta\";"
    );

    auto *layout = new QVBoxLayout(this);
    auto *quit_label = new QLabel("QUIT?");
    auto *buttons_layout = new QHBoxLayout(this);
    auto *confirm_quit_button = new QPushButton("CONFIRM");
    auto *cancel_quit_button = new QPushButton("CANCEL");

    layout->addWidget(quit_label, 0, Qt::AlignCenter);
    layout->addStretch();
    layout->addLayout(buttons_layout);
    buttons_layout->addWidget(confirm_quit_button, 0, Qt::AlignCenter);
    buttons_layout->addWidget(cancel_quit_button, 0, Qt::AlignCenter);
    quit_label->setStyleSheet(
        "font-size: 96px;"
        "border: none;"
        "background: transparent;"
    );
    confirm_quit_button->setStyleSheet(
        "font-size: 60pt;"
        "font-family: \"FREE FAT FONT\";"
        "color: rgb(192, 50, 33);"
        "border: none;"
        "margin-right: 20px;"
        "background: transparent;"
    );
    cancel_quit_button->setStyleSheet(
        "font-size: 60pt;"
        "font-family: \"FREE FAT FONT\";"
        "border: none;"
        "margin-left: 2px;"
        "background: transparent;"
    );

    auto *quit_label_glow = new QGraphicsDropShadowEffect(quit_label);  
    quit_label_glow->setBlurRadius(24);
    quit_label_glow->setColor(qRgb(0, 255, 255));
    quit_label_glow->setOffset(0, 0);
    quit_label->setGraphicsEffect(quit_label_glow);

    auto *confirm_glow = new QGraphicsDropShadowEffect(confirm_quit_button);
    confirm_glow->setBlurRadius(20);
    confirm_glow->setColor(qRgb(192, 50, 33));
    confirm_glow->setOffset(0, 0);
    confirm_quit_button->setGraphicsEffect(confirm_glow);

    auto *cancel_glow = new QGraphicsDropShadowEffect(cancel_quit_button);
    cancel_glow->setBlurRadius(20);
    cancel_glow->setColor(qRgb(0, 255, 255));
    cancel_glow->setOffset(0, 0);
    cancel_quit_button->setGraphicsEffect(cancel_glow);

    connect(confirm_quit_button, &QPushButton::clicked, this, [this]() { if (auto *mw = parentWidget()->parentWidget()->parentWidget()) mw->close(); }); // parent widget (stack) -> parent widget (main menu) -> parent widget (window)
    connect(cancel_quit_button, &QPushButton::clicked, this, 
        [this]() {
            if (!closing) {
                closing = true;

                auto *fade_out = new QPropertyAnimation(this, "windowOpacity");
                fade_out->setDuration(300);
                fade_out->setStartValue(windowOpacity());
                fade_out->setEndValue(0.0);

                connect(fade_out, &QPropertyAnimation::finished, this, [this]() { QDialog::close(); });
                fade_out->start(QAbstractAnimation::DeleteWhenStopped);
            }
        }
    );
}

void QuitGameWindow::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);
    QWidget* win = window();

    if (win) {
        QRect g = win->geometry();

        move(g.center() - rect().center());
    }

    closing = false;
    fade_animation->stop();
    fade_animation->setTargetObject(this);
    fade_animation->setPropertyName("windowOpacity");
    fade_animation->setDuration(300);
    fade_animation->setStartValue(0.0);
    fade_animation->setEndValue(1.0);
    fade_animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void QuitGameWindow::closeEvent(QCloseEvent* event) {
    if (!closing) {
        event->ignore();
        closing = true;

        auto *fade_out = new QPropertyAnimation(this, "windowOpacity");

        fade_out->setDuration(300);
        fade_out->setStartValue(windowOpacity());
        fade_out->setEndValue(0.0);

        connect(fade_out, &QPropertyAnimation::finished, this, [this]() { QDialog::close(); });
        fade_out->start(QAbstractAnimation::DeleteWhenStopped);
    }
    else QDialog::closeEvent(event);
}
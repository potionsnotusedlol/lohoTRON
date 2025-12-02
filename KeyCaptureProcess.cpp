#include "KeyCaptureProcess.h"

KeyCaptureProcess::KeyCaptureProcess(QWidget* parent) : QDialog(parent) {
    fade_in_animation = new QPropertyAnimation(this, "windowOpacity", this);
    setWindowOpacity(0.0);
    setWindowTitle("Rebind Keys");
    resize(100, 75);
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
    auto *hint = new QLabel("PRESS A BUTON TO ASSIGN");
    auto *cancel_rebinding_button = new QPushButton("CANCEL");

    layout->addWidget(hint, 0, Qt::AlignCenter);
    layout->addStretch();
    layout->addWidget(cancel_rebinding_button, 0, Qt::AlignCenter);
    hint->setStyleSheet(
        "font: 20px;"
        "font-family: \"TraktorMoodFont\";"
        "color: rgb(242, 208, 164);"
        "border: none;"
        "background: transparent;"
    );
    cancel_rebinding_button->setStyleSheet(
        "font-size: 20px;"
        "border: none;"
        "background: transparent;"
    );
    connect(cancel_rebinding_button, &QPushButton::clicked, this,
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

void KeyCaptureProcess::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);
    QWidget* win = window();

    if (win) {
        QRect g = win->geometry();

        move(g.center() - rect().center());
    }

    closing = false;
    fade_in_animation->stop();
    fade_in_animation->setTargetObject(this);
    fade_in_animation->setPropertyName("windowOpacity");
    fade_in_animation->setDuration(300);
    fade_in_animation->setStartValue(0.0);
    fade_in_animation->setEndValue(1.0);
    fade_in_animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void KeyCaptureProcess::closeEvent(QCloseEvent* event) {
    if (!closing) {
        event->ignore();
        closing = true;

        auto *fade_out = new QPropertyAnimation(this, "windowOpacity");
        fade_out->setDuration(300);
        fade_out->setStartValue(windowOpacity());
        fade_out->setEndValue(0.0);
        
        connect(fade_out, &QPropertyAnimation::finished, this, [this]() { QDialog::close(); });
        fade_out->start(QAbstractAnimation::DeleteWhenStopped);

    } else QDialog::closeEvent(event);
}

void KeyCaptureProcess::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) return;

    key_selected = event->key();

    if (key_selected == Qt::Key_Shift || key_selected == Qt::Key_Control || key_selected == Qt::Key_Alt || key_selected == Qt::Key_Meta || key_selected == Qt::Key_AltGr) return;

    accept();
}

int KeyCaptureProcess::getKey() const { return key_selected; }
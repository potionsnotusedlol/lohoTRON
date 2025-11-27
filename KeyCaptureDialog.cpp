#include "KeyCaptureDialog.h"

KeyCaptureDialog::KeyCaptureDialog(QWidget* parent) : QDialog(parent) {
    fade_in_animation = new QPropertyAnimation(this, "windowOpacity", this);
    setWindowOpacity(0.0);
    setWindowTitle("Rebind Keys");
    resize(200, 150);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setModal(true);
    setStyleSheet(
        "background-color: rgba(0, 0, 0, 230);"
        "color: cyan;"
        "border: 2px solid cyan;"
        "border-radius: 10px;"
        "font-family: \"Bolgarus Beta\";"
    );

    QJsonObject root = loadConfigRoot();
    QJsonObject player = root.value("player").toObject();
    QJsonArray keys = player.value("key_bindings").toArray();
    auto *layout = new QVBoxLayout(this);
    auto *forward_layout = new QHBoxLayout(this);
    auto *forward_label = new QLabel("MOVE FORWARD");
    auto key_assigned_forward = keys[0];
    auto *forward_button = new QPushButton();
    auto *cancel_rebinding_button = new QPushButton("CANCEL");

    layout->addWidget(cancel_rebinding_button);
    layout->addStretch();
    layout->addLayout(forward_layout);
    forward_layout->addWidget(forward_label, 0, Qt::AlignLeft);
    forward_layout->addWidget(forward_button, 0, Qt::AlignRight);
    cancel_rebinding_button->setStyleSheet(
        "font-size: 40px;"
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

void KeyCaptureDialog::showEvent(QShowEvent* event) {
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

void KeyCaptureDialog::closeEvent(QCloseEvent* event) {
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
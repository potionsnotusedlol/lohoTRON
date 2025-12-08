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
    QString key_assigned_forward, key_assigned_backward, key_assigned_left, key_assigned_right;
    auto *forward_button = new QPushButton();
    auto *backward_layout = new QHBoxLayout(this);
    auto *backward_label = new QLabel("MOVE BACKWARDS");
    auto *backward_button = new QPushButton();
    auto *left_layout = new QHBoxLayout(this);
    auto *left_label = new QLabel("TURN LEFT");
    auto *left_button = new QPushButton();
    auto *right_layout = new QHBoxLayout(this);
    auto *right_label = new QLabel("TURN RIGHT");
    auto *right_button = new QPushButton();
    auto *cancel_rebinding_button = new QPushButton("CANCEL");
    int rebind_window_width = this->width();

    // neither the root nor the keys array can be empty (made sure in main.cpp)
    if (!keys.isEmpty()) {
        key_assigned_forward = keys[0].toString();
        key_assigned_backward = keys[1].toString();
        key_assigned_left = keys[2].toString();
        key_assigned_right = keys[3].toString();
    }

    layout->addLayout(forward_layout);
    forward_layout->addWidget(forward_label, 0, Qt::AlignLeft);
    forward_layout->addWidget(forward_button, 0, Qt::AlignRight);
    layout->addLayout(backward_layout);
    backward_layout->addWidget(backward_label, 0, Qt::AlignLeft);
    backward_layout->addWidget(backward_button, 0, Qt::AlignRight);
    layout->addLayout(left_layout);
    left_layout->addWidget(left_label, 0, Qt::AlignLeft);
    left_layout->addWidget(left_button, 0, Qt::AlignRight);
    layout->addLayout(right_layout);
    right_layout->addWidget(right_label, 0, Qt::AlignLeft);
    right_layout->addWidget(right_button, 0, Qt::AlignRight);
    layout->addStretch();
    layout->addWidget(cancel_rebinding_button);
    cancel_rebinding_button->setStyleSheet(
        "font-size: 40px;"
        "border: none;"
        "background: transparent;"
    );
    forward_label->setStyleSheet(
        "font-size: 20px;"
        "font-family: \"TraktorMoodFont\";"
        "color: rgb(242, 208, 164);"
        "border: none;"
        "background: transparent;"
    );
    forward_button->setStyleSheet(
        "font-size: 20px;"
        "font-family: \"Wattauchimma\";"
        "color: rgb(192, 50, 113);"
        "border: 3px solid rgb(242, 208, 164);"
        "background: transparent; "       
    );
    forward_button->setFixedWidth(rebind_window_width * 0.25);
    forward_button->setText(key_assigned_forward);
    backward_label->setStyleSheet(
        "font-size: 20px;"
        "font-family: \"TraktorMoodFont\";"
        "color: rgb(242, 208, 164);"
        "border: none;"
        "background: transparent;"
    );
    backward_button->setStyleSheet(
        "font-size: 20px;"
        "font-family: \"Wattauchimma\";"
        "color: rgb(192, 50, 113);"
        "border: 3px solid rgb(242, 208, 164);"
        "background: transparent; "       
    );
    backward_button->setFixedWidth(rebind_window_width * 0.25);
    backward_button->setText(key_assigned_backward);
    left_label->setStyleSheet(
        "font-size: 20px;"
        "font-family: \"TraktorMoodFont\";"
        "color: rgb(242, 208, 164);"
        "border: none;"
        "background: transparent;"
    );
    left_button->setStyleSheet(
        "font-size: 20px;"
        "font-family: \"Wattauchimma\";"
        "color: rgb(192, 50, 113);"
        "border: 3px solid rgb(242, 208, 164);"
        "background: transparent; "       
    );
    left_button->setFixedWidth(rebind_window_width * 0.25);
    left_button->setText(key_assigned_left);
    right_label->setStyleSheet(
        "font-size: 20px;"
        "font-family: \"TraktorMoodFont\";"
        "color: rgb(242, 208, 164);"
        "border: none;"
        "background: transparent;"
    );
    right_button->setStyleSheet(
        "font-size: 20px;"
        "font-family: \"Wattauchimma\";"
        "color: rgb(192, 50, 113);"
        "border: 3px solid rgb(242, 208, 164);"
        "background: transparent; "       
    );
    right_button->setFixedWidth(rebind_window_width * 0.25);
    right_button->setText(key_assigned_right);
    connect(forward_button, &QPushButton::clicked, this,
        [this, forward_button]() {
            KeyCaptureProcess dlg(this, 0);

            if (dlg.exec() == QDialog::Accepted) {
                QKeySequence key = dlg.getKeySelected();

                forward_button->setText(key.toString());
            }


        }    
    );
    connect(backward_button, &QPushButton::clicked, this,
        [this, backward_button]() {
            KeyCaptureProcess dlg(this, 1);

            if (dlg.exec() == QDialog::Accepted) {
                QKeySequence key = dlg.getKeySelected();

                backward_button->setText(key.toString());
            }
        }      
    );
    connect(left_button, &QPushButton::clicked, this,
        [this, left_button]() {
            KeyCaptureProcess dlg(this, 2);

            if (dlg.exec() == QDialog::Accepted) {
                QKeySequence key = dlg.getKeySelected();

                left_button->setText(key.toString());
            }
        }     
    );
    connect(right_button, &QPushButton::clicked, this,
        [this, right_button]() {
            KeyCaptureProcess dlg(this, 3);

            if (dlg.exec() == QDialog::Accepted) {
                QKeySequence key = dlg.getKeySelected();

                right_button->setText(key.toString());
            }
        }      
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
#include "GameStartWindow.h"

GameStartWindow::GameStartWindow(QWidget* parent) : QDialog(parent), opacity_effect(new QGraphicsOpacityEffect(this)), fade_in_animation(new QPropertyAnimation(opacity_effect, "opacity", this)) {
    setWindowTitle("Start New Game");
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

    setGraphicsEffect(opacity_effect);
    opacity_effect->setOpacity(0.0);

    auto *layout = new QVBoxLayout(this);
    auto *game_set_label = new QLabel("NEW GAME SETTINGS");
    auto *setting_layout = new QHBoxLayout(this);
    auto *more_rounds_button = new QPushButton("+");
    auto *rounds_count = new QLineEdit(this);
    auto *less_rounds_button = new QPushButton("-");
    auto *more_bots_button = new QPushButton("HARDER");
    auto *bots_count = new QLineEdit(this);
    auto *less_bots_button = new QPushButton("SIMPLER");
    auto *start_game_button = new QPushButton("READY");
    auto *cancel_game_button = new QPushButton("MENU");

    layout->addWidget(game_set_label, 0, Qt::AlignCenter);
    layout->addStretch();
    layout->addLayout(setting_layout);
    setting_layout->addWidget(more_rounds_button, 0, Qt::AlignCenter);
    setting_layout->addWidget(rounds_count, 0, Qt::AlignCenter);
    setting_layout->addWidget(less_rounds_button, 0, Qt::AlignCenter);
    setting_layout->addWidget(more_bots_button, 0, Qt::AlignCenter);
    setting_layout->addWidget(bots_count, 0, Qt::AlignCenter);
    setting_layout->addWidget(less_bots_button, 0, Qt::AlignCenter);
    layout->addWidget(start_game_button, 0, Qt::AlignCenter);
    layout->addWidget(cancel_game_button, 0, Qt::AlignCenter);

    connect(start_game_button, &QPushButton::clicked, this, [this]() {});
    connect(cancel_game_button, &QPushButton::clicked, this,
        [this]() {
            closing = true;

            fade_in_animation->stop();
            fade_in_animation->setDuration(300);
            fade_in_animation->setStartValue(opacity_effect->opacity());
            fade_in_animation->setEndValue(0.0);
            fade_in_animation->start();
            connect(fade_in_animation, &QPropertyAnimation::finished, this, [this]() { if (closing) close(); });
        }
    );
}

void GameStartWindow::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);

    if (parentWidget()) move(parentWidget()->geometry().center() - rect().center());

    closing = false;

    fade_in_animation->stop();
    fade_in_animation->setDuration(300);
    fade_in_animation->setStartValue(0.0);
    fade_in_animation->setEndValue(1.0);
    fade_in_animation->start();
}

void GameStartWindow::closeEvent(QCloseEvent* event) {
    if (!closing) {
        event->ignore();

        closing = true;

        fade_in_animation->stop();
        fade_in_animation->setDuration(300);
        fade_in_animation->setStartValue(1.0);
        fade_in_animation->setEndValue(0.0);
        fade_in_animation->start();

        connect(fade_in_animation, &QPropertyAnimation::finished, this, [this]() { QDialog::close(); });
    }
    else QDialog::closeEvent(event);
}
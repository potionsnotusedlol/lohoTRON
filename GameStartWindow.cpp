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
    map_size_edit = new QLineEdit(this);
    auto *less_rounds_button = new QPushButton("-");
    auto *more_bots_button = new QPushButton("HARDER");
    bot_count_edit = new QLineEdit(this);
    auto *less_bots_button = new QPushButton("SIMPLER");
    auto *start_game_button = new QPushButton("READY");
    auto *cancel_game_button = new QPushButton("MENU");

    layout->addWidget(game_set_label, 0, Qt::AlignCenter);
    layout->addStretch();
    layout->addLayout(setting_layout);
    setting_layout->addWidget(more_rounds_button, 0, Qt::AlignCenter);
    setting_layout->addWidget(map_size_edit, 0, Qt::AlignCenter);
    setting_layout->addWidget(less_rounds_button, 0, Qt::AlignCenter);
    setting_layout->addWidget(more_bots_button, 0, Qt::AlignCenter);
    setting_layout->addWidget(bot_count_edit, 0, Qt::AlignCenter);
    setting_layout->addWidget(less_bots_button, 0, Qt::AlignCenter);
    layout->addWidget(start_game_button, 0, Qt::AlignCenter);
    layout->addWidget(cancel_game_button, 0, Qt::AlignCenter);

    map_size_edit->setReadOnly(true);
    map_size_edit->setAlignment(Qt::AlignCenter);
    map_size_edit->setStyleSheet("background-color: rgba(255,255,255,30); color: cyan; border: 1px solid cyan;");
    map_size_edit->setFixedWidth(50);

    bot_count_edit->setReadOnly(true);
    bot_count_edit->setAlignment(Qt::AlignCenter);
    bot_count_edit->setStyleSheet("background-color: rgba(255,255,255,30); color: cyan; border: 1px solid cyan;");
    bot_count_edit->setFixedWidth(80);
    
    loadSettings();

    updateBotCountDisplay();

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

    connect(more_rounds_button, &QPushButton::clicked, this, [this]() {
        if (map_size < 20) {
            map_size++;
            map_size_edit->setText(QString::number(map_size));
        }
    });
    
    connect(less_rounds_button, &QPushButton::clicked, this, [this]() {
        if (map_size > 3) { 
            map_size--;
            map_size_edit->setText(QString::number(map_size));
        }
    });
    
    connect(more_bots_button, &QPushButton::clicked, this, [this]() {
        if (bot_count < 10) {
            bot_count++;
            updateBotCountDisplay();
        }
    });
    
    connect(less_bots_button, &QPushButton::clicked, this, [this]() {
        if (bot_count > 0) {
            bot_count--;
            updateBotCountDisplay();
        }
    });

    connect(start_game_button, &QPushButton::clicked, this, [this]() {
        saveSettings();
        accept();
    });
}

void GameStartWindow::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);
    QWidget* win = window();

    if (win) {
        QRect g = win->geometry();

        move(g.center() - rect().center());
    }

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

void GameStartWindow::saveSettings() {
    QFile file("game_config.json");
    if (!file.open(QIODevice::ReadWrite)) {
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();

    // Сохраняем новые параметры
    obj["map_size"] = map_size;
    obj["bot_count"] = bot_count;

    file.resize(0);
    file.write(QJsonDocument(obj).toJson());
    file.close();
}

void GameStartWindow::loadSettings() {
    QFile file("game_config.json");
    if (!file.open(QIODevice::ReadOnly)) {
        // Значения по умолчанию
        map_size = 3;
        bot_count = 1;
        map_size_edit->setText(QString::number(map_size));
        updateBotCountDisplay();
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        map_size = 3;
        bot_count = 1;
        map_size_edit->setText(QString::number(map_size));
        updateBotCountDisplay();
        return;
    }

    QJsonObject obj = doc.object();

    if (obj.contains("map_size")) {
        map_size = obj["map_size"].toInt();
        if (map_size < 3) map_size = 3;
    } else {
        map_size = 3;
    }
    map_size_edit->setText(QString::number(map_size));

    if (obj.contains("bot_count")) {
        bot_count = obj["bot_count"].toInt();
        if (bot_count < 0) bot_count = 0;
    } else {
        bot_count = 1;
    }
    updateBotCountDisplay();
}

void GameStartWindow::updateBotCountDisplay() {
    if (bot_count == 0) {
        bot_count_edit->setText("NONE");
    } else if (bot_count == 1) {
        bot_count_edit->setText("EASY");
    } else if (bot_count == 2) {
        bot_count_edit->setText("MEDIUM");
    } else if (bot_count == 3) {
        bot_count_edit->setText("HARD");
    } else {
        bot_count_edit->setText("EXTREME");
    }
}
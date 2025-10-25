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
    rounds_count = new QLineEdit(this);
    auto *less_rounds_button = new QPushButton("-");
    auto *more_bots_button = new QPushButton("HARDER");
    bots_count = new QLineEdit(this);
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

    rounds_count->setReadOnly(true);
    rounds_count->setAlignment(Qt::AlignCenter);
    rounds_count->setStyleSheet("background-color: rgba(255,255,255,30); color: cyan; border: 1px solid cyan;");
    rounds_count->setFixedWidth(50);
    //rounds_count->setText("1");

    bots_count->setReadOnly(true);
    bots_count->setAlignment(Qt::AlignCenter);
    bots_count->setStyleSheet("background-color: rgba(255,255,255,30); color: cyan; border: 1px solid cyan;");

    bots_count->setFixedWidth(80);
    //bots_count->setText("MEDIUM");
    loadSettings();

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
        int current = rounds_count->text().toInt();
        if (current < 10) {
            rounds_count->setText(QString::number(current + 1));
    }
    });

    connect(less_rounds_button, &QPushButton::clicked, this, [this]() {
        int current = rounds_count->text().toInt();
        if (current > 1) {
            rounds_count->setText(QString::number(current - 1));
        }
    });

    connect(more_bots_button, &QPushButton::clicked, this, [this]() {
        QString current = bots_count->text();
        if (current == "EASY") bots_count->setText("MEDIUM");
        else if (current == "MEDIUM") bots_count->setText("HARD");
        else if (current == "HARD") bots_count->setText("EXTREME");
    });

    connect(less_bots_button, &QPushButton::clicked, this, [this]() {
        QString current = bots_count->text();
        if (current == "EXTREME") bots_count->setText("HARD");
        else if (current == "HARD") bots_count->setText("MEDIUM");
        else if (current == "MEDIUM") bots_count->setText("EASY");
    });

    connect(start_game_button, &QPushButton::clicked, this, [this]() {
        saveSettings();
        accept();
    });
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


void GameStartWindow::saveSettings() {
    QFile file("game_config.json");
    if (!file.open(QIODevice::ReadWrite)) {
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();

    obj["rounds_count"] = rounds_count->text().toInt();
    obj["difficulty"] = bots_count->text();

    file.resize(0);
    file.write(QJsonDocument(obj).toJson());
    file.close();
}


void GameStartWindow::loadSettings() {
    QFile file("game_config.json");
    if (!file.open(QIODevice::ReadOnly)) {
        rounds_count->setText("1");
        bots_count->setText("MEDIUM");
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        rounds_count->setText("1");
        bots_count->setText("MEDIUM");
        return;
    }

    QJsonObject obj = doc.object();

    if (obj.contains("rounds_count")) {
        rounds_count->setText(QString::number(obj["rounds_count"].toInt()));
    } else {
        rounds_count->setText("1");
    }

    if (obj.contains("difficulty")) {
        QString difficulty = obj["difficulty"].toString();
        if (difficulty == "EASY" || difficulty == "MEDIUM" || 
            difficulty == "HARD" || difficulty == "EXTREME") {
            bots_count->setText(difficulty);
        } else {
            bots_count->setText("MEDIUM");
        }
    } else {
        bots_count->setText("MEDIUM");
    }
}
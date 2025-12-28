#include "GameOverWindow.h"

GameOverWindow::GameOverWindow(QWidget* parent)
    : QDialog(parent) {

    fade_in = new QPropertyAnimation(this, "windowOpacity", this);
    setupUI();
    applyStyles();
    setupConnections();
}

void GameOverWindow::setupUI() {
    setWindowOpacity(0.0);
    setModal(true);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    resize(420, 420);

    title_label = new QLabel(this);
    stats_label = new QLabel(this);
    restart_button = new QPushButton("RESTART", this);
    exit_button = new QPushButton("MAIN MENU", this);

    title_label->setAlignment(Qt::AlignCenter);
    stats_label->setAlignment(Qt::AlignCenter);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(title_label);
    layout->addSpacing(20);
    layout->addWidget(stats_label);
    layout->addStretch();
    layout->addWidget(restart_button);
    layout->addWidget(exit_button);

    restart_button->setFixedHeight(80);
    exit_button->setFixedHeight(80);
}

void GameOverWindow::applyStyles() {
    setStyleSheet(
        "background-color: rgba(0,0,0,230);"
        "border: 2px solid cyan;"
        "border-radius: 10px;"
        "color: cyan;"
    );

    title_label->setStyleSheet("font-size: 72pt; font-family: \"Bolgarus Beta\";");
    stats_label->setStyleSheet("font-size: 32pt; font-family: \"Wattauchimma\";");

    restart_button->setStyleSheet(R"(
    QPushButton {
        font-size: 36pt;
        background-color: rgb(127,176,105);
        border-radius: 10px;
    }
        QPushButton:hover {
        background-color: #3daee9;
        border-color: #6fcfff;
        color: black;
    }
    )");

    exit_button->setStyleSheet(R"(
    QPushButton {
        font-size: 36pt;
        background-color: rgb(61,19,8); 
        border-radius: 10px;
    }
    QPushButton:hover {
        background-color: #e05d5d;
        border-color: #ff8a8a;
        color: black;
    }
    )");

    auto glow = new QGraphicsDropShadowEffect(title_label);
    glow->setBlurRadius(30);
    glow->setColor(Qt::cyan);
    glow->setOffset(0,0);
    title_label->setGraphicsEffect(glow);
}

void GameOverWindow::setupConnections() {
    connect(restart_button, &QPushButton::clicked, this, [this]() {
        accept();          
        emit restartGame();
    });

    connect(exit_button, &QPushButton::clicked, this, [this]() {
        accept();
        emit exitToMenu();
    });
}

void GameOverWindow::setMatchResult(bool win, int killedBots, int wonRounds) {
    title_label->setText(win ? "YOU WIN" : "YOU LOSE");

    stats_label->setText(
        QString("KILLED ENEMIES: %1\nWON ROUNDS: %2")
        .arg(killedBots)
        .arg(wonRounds)
    );
}

void GameOverWindow::showEvent(QShowEvent* e) {
    QDialog::showEvent(e);

    fade_in->setDuration(300);
    fade_in->setStartValue(0.0);
    fade_in->setEndValue(1.0);
    fade_in->start();
}

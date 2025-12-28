#include "GamePauseWindow.h"

namespace {

struct PauseSettings {
    int window_width = 400, window_height = 450, animation_duration = 300;
    
};

PauseSettings loadPauseSettings() {
    PauseSettings settings;
    
    return settings;
}

PauseSettings& pauseSettings() {
    static PauseSettings s = loadPauseSettings();
    
    return s;
}

}

GamePauseWindow::GamePauseWindow(QWidget* parent) : QDialog(parent) {
    fade_in_animation = new QPropertyAnimation(this, "windowOpacity", this);
    setupUI();
    applyStyles();
    setupConnections();
}

void GamePauseWindow::setupUI() {
    const PauseSettings &settings = pauseSettings();
    
    setWindowOpacity(0.0); 
    setWindowTitle("Game Paused");
    resize(settings.window_width, settings.window_height);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setModal(true);
    pause_label = new QLabel("PAUSED", this);
    resume_button = new QPushButton("RESUME", this);
    restart_button = new QPushButton("RESTART", this);
    exit_button = new QPushButton("MAIN MENU", this);
    
    auto *main_layout = new QVBoxLayout(this);
    main_layout->addWidget(pause_label, 0, Qt::AlignCenter);
    main_layout->addStretch();
    main_layout->addWidget(resume_button, 0, Qt::AlignCenter);
    main_layout->addWidget(restart_button, 0, Qt::AlignCenter);
    main_layout->addWidget(exit_button, 0, Qt::AlignCenter);
    main_layout->addStretch();
    
    int button_width = settings.window_width * 0.95, button_height = settings.window_height * 0.2;
    
    resume_button->setFixedSize(button_width, button_height);
    restart_button->setFixedSize(button_width, button_height);
    exit_button->setFixedSize(button_width, button_height);
}

void GamePauseWindow::applyStyles() {
    const PauseSettings &settings = pauseSettings();
    
    setStyleSheet(
        "background-color: rgba(0, 0, 0, 230);"
        "color: cyan;"
        "border: 2px solid cyan;"
        "border-radius: 10px;"
        "font-family: \"Bolgarus Beta\";"
    );
    pause_label->setStyleSheet(
        "font-size: 96pt;"
        "border: none;"
        "background: transparent;"
        "margin-bottom: 30px;"
    );
    
    QString button_style = QString(
        "QPushButton {"
            "font-size: 48pt;"
            "font-family: \"Wattauchimma\";"
            "font-weight: bold;"
            "color: black;"
            "border: none;"
            "border-radius: 10px;"
            "padding: 10px;"
            "background-color: rgb(127, 176, 105);"
        "}"
        "QPushButton:hover { background-color: rgb(221, 255, 247); }"
    );
    
    resume_button->setStyleSheet(button_style);
    restart_button->setStyleSheet(button_style);
    exit_button->setStyleSheet(
         "QPushButton {"
            "font-size: 48pt;"
            "font-family: \"Wattauchimma\";"
            "font-weight: bold;"
            "color: black;"
            "border: none;"
            "border-radius: 10px;"
            "padding: 10px;"
            "background-color: rgb(61, 19, 8);"
        "}"
        "QPushButton:hover { background-color: rgb(255, 175, 240); }"
    );
    
    auto pause_label_glow = new QGraphicsDropShadowEffect(pause_label);
    pause_label_glow->setBlurRadius(30);
    pause_label_glow->setColor(qRgb(127, 176, 105));
    pause_label_glow->setOffset(0, 0);
    pause_label->setGraphicsEffect(pause_label_glow);
    
    auto resume_glow = new QGraphicsDropShadowEffect(resume_button);
    resume_glow->setBlurRadius(20);
    resume_glow->setColor(qRgb(127, 176, 105));
    resume_glow->setOffset(0, 0);
    resume_button->setGraphicsEffect(resume_glow);
    
    auto restart_glow = new QGraphicsDropShadowEffect(restart_button);
    restart_glow->setBlurRadius(20);
    restart_glow->setColor(qRgb(127, 176, 105));
    restart_glow->setOffset(0, 0);
    restart_button->setGraphicsEffect(restart_glow);
    
    auto exit_glow = new QGraphicsDropShadowEffect(exit_button);
    exit_glow->setBlurRadius(20);
    exit_glow->setColor(qRgb(61, 19, 8));
    exit_glow->setOffset(0, 0);
    exit_button->setGraphicsEffect(exit_glow);
}

void GamePauseWindow::closeByEsc() {
    if (!closing) {
        closing = true;

        auto *fade_out = new QPropertyAnimation(this, "windowOpacity");
        fade_out->setDuration(pauseSettings().animation_duration);
        fade_out->setStartValue(windowOpacity());
        fade_out->setEndValue(0.0);

        connect(fade_out, &QPropertyAnimation::finished, this,
            [this]() {
                emit cancelPause();  
                QDialog::close();
            }
        );
        fade_out->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void GamePauseWindow::reject() {
    if (!closing) {
        closing = true;

        auto *fade_out = new QPropertyAnimation(this, "windowOpacity");
        fade_out->setDuration(pauseSettings().animation_duration);
        fade_out->setStartValue(windowOpacity());
        fade_out->setEndValue(0.0);

        connect(fade_out, &QPropertyAnimation::finished, this,
            [this]() {
                emit resumeGame();
                QDialog::reject();
            }
        );
        fade_out->start(QAbstractAnimation::DeleteWhenStopped);
    }
}


void GamePauseWindow::setupConnections() {
    connect(resume_button, &QPushButton::clicked, this, &GamePauseWindow::onResumeClicked);
    connect(restart_button, &QPushButton::clicked, this, &GamePauseWindow::onRestartClicked);
    connect(exit_button, &QPushButton::clicked, this, &GamePauseWindow::onExitClicked);
}

void GamePauseWindow::onResumeClicked() {
    if (!closing) {
        closing = true;
        
        // Анимация исчезания
        auto *fade_out = new QPropertyAnimation(this, "windowOpacity");
        fade_out->setDuration(pauseSettings().animation_duration);
        fade_out->setStartValue(windowOpacity());
        fade_out->setEndValue(0.0);

        connect(fade_out, &QPropertyAnimation::finished, this,
            [this]() {
                emit resumeGame();
                QDialog::reject();
            }
        );
        fade_out->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void GamePauseWindow::onRestartClicked() {
    if (!closing) {
        closing = true;
        
        auto *fade_out = new QPropertyAnimation(this, "windowOpacity");
        fade_out->setDuration(pauseSettings().animation_duration);
        fade_out->setStartValue(windowOpacity());
        fade_out->setEndValue(0.0);

        connect(fade_out, &QPropertyAnimation::finished, this,
            [this]() {
                emit restartGame();
                QDialog::reject();
            }
        );
        fade_out->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void GamePauseWindow::onExitClicked() {
    if (!closing) {
        closing = true;
        
        auto *fade_out = new QPropertyAnimation(this, "windowOpacity");
        fade_out->setDuration(pauseSettings().animation_duration);
        fade_out->setStartValue(windowOpacity());
        fade_out->setEndValue(0.0);

        connect(fade_out, &QPropertyAnimation::finished, this,
            [this]() {
                emit exitToMenu();
                QDialog::reject();
            }
        );
        fade_out->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void GamePauseWindow::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);
    
    QWidget* win = window();

    if (win) {
        QRect g = win->geometry();

        move(g.center() - rect().center());
    }
    
    closing = false;
    fade_in_animation->stop();
    fade_in_animation->setDuration(pauseSettings().animation_duration);
    fade_in_animation->setStartValue(0.0);
    fade_in_animation->setEndValue(1.0);
    fade_in_animation->start();
}

void GamePauseWindow::closeEvent(QCloseEvent* event) {
    if (!closing) {
        event->ignore(); 
        closing = true;
        
        auto *fade_out = new QPropertyAnimation(this, "windowOpacity");
        fade_out->setDuration(pauseSettings().animation_duration);
        fade_out->setStartValue(windowOpacity());
        fade_out->setEndValue(0.0);

        connect(fade_out, &QPropertyAnimation::finished, this, [this]() { QDialog::close(); });
        fade_out->start(QAbstractAnimation::DeleteWhenStopped);
    } else QDialog::closeEvent(event);
}
#include "GameStartWindow.h"
#include "mainwindow.h"

namespace {

struct GameDefaults {
    int fieldSizeDefault = 10;
    int fieldSizeMin     = 5;
    int fieldSizeMax     = 20;

    int botsDefault      = 1;
    int botsMin          = 0;
    int botsMax          = 4;
};

QString configFilePath()
{
    return QCoreApplication::applicationDirPath() + "/game_config.json";
}

void writeDefaultConfig(const GameDefaults& cfg)
{
    QJsonObject fieldObj;
    fieldObj["size_default"] = cfg.fieldSizeDefault;
    fieldObj["size_min"]     = cfg.fieldSizeMin;
    fieldObj["size_max"]     = cfg.fieldSizeMax;

    QJsonObject botsObj;
    botsObj["count_default"] = cfg.botsDefault;
    botsObj["count_min"]     = cfg.botsMin;
    botsObj["count_max"]     = cfg.botsMax;

    QJsonObject gameObj;
    gameObj["field"] = fieldObj;
    gameObj["bots"]  = botsObj;

    QJsonObject root;
    root["game"] = gameObj;

    QJsonDocument doc(root);

    QFile file(configFilePath());
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    }
}

GameDefaults loadGameDefaults()
{
    GameDefaults cfg;
    const QString path = configFilePath();

    QFile file(path);

    if (!file.exists()) {
        writeDefaultConfig(cfg);
        return cfg;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return cfg;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    file.close();

    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        writeDefaultConfig(cfg);
        return cfg;
    }

    const QJsonObject root  = doc.object();
    const QJsonObject game  = root.value("game").toObject();
    const QJsonObject field = game.value("field").toObject();
    const QJsonObject bots  = game.value("bots").toObject();

    if (!field.isEmpty()) {
        cfg.fieldSizeDefault = field.value("size_default").toInt(cfg.fieldSizeDefault);
        cfg.fieldSizeMin     = field.value("size_min").toInt(cfg.fieldSizeMin);
        cfg.fieldSizeMax     = field.value("size_max").toInt(cfg.fieldSizeMax);
    }

    if (!bots.isEmpty()) {
        cfg.botsDefault = bots.value("count_default").toInt(cfg.botsDefault);
        cfg.botsMin     = bots.value("count_min").toInt(cfg.botsMin);
        cfg.botsMax     = bots.value("count_max").toInt(cfg.botsMax);
    }

    if (cfg.fieldSizeMin > cfg.fieldSizeMax)
        cfg.fieldSizeMin = cfg.fieldSizeMax;
    if (cfg.fieldSizeDefault < cfg.fieldSizeMin)
        cfg.fieldSizeDefault = cfg.fieldSizeMin;
    else if (cfg.fieldSizeDefault > cfg.fieldSizeMax)
        cfg.fieldSizeDefault = cfg.fieldSizeMax;

    if (cfg.botsMin > cfg.botsMax)
        cfg.botsMin = cfg.botsMax;
    if (cfg.botsDefault < cfg.botsMin)
        cfg.botsDefault = cfg.botsMin;
    else if (cfg.botsDefault > cfg.botsMax)
        cfg.botsDefault = cfg.botsMax;

    return cfg;
}

GameDefaults& defaults()
{
    static GameDefaults d = loadGameDefaults();
    return d;
}

int g_currentFieldSize = -1;
int g_currentBotsCount = -1;

}


GameStartWindow::GameStartWindow(QWidget* parent) : QDialog(parent) {
    fade_in_animation = new QPropertyAnimation(this, "windowOpacity", this);

    const GameDefaults &defs = defaults();
    const int fieldSizeMin = defs.fieldSizeMin;
    const int fieldSizeMax = defs.fieldSizeMax;
    const int botsMin      = defs.botsMin;
    const int botsMax      = defs.botsMax;

    if (g_currentFieldSize < 0)
        g_currentFieldSize = defs.fieldSizeDefault;
    if (g_currentBotsCount < 0)
        g_currentBotsCount = defs.botsDefault;

    setWindowOpacity(0.0);
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

    auto *layout = new QVBoxLayout(this);
    auto *game_set_label = new QLabel("NEW GAME");
    auto *setting_layout = new QHBoxLayout(this);
    auto *set_rounds_layout = new QVBoxLayout(this);
    auto *rounds_hint = new QLabel("ROUNDS:");
    auto *change_rounds_layout = new QHBoxLayout(this);
    auto *more_rounds_button = new QPushButton("+");
    auto *rounds_count = new QLineEdit(this);
    auto *less_rounds_button = new QPushButton("-");
    auto *set_bots_layout = new QVBoxLayout(this);
    auto *bots_hint = new QLabel("BOTS:");
    auto *bots_change_layout = new QHBoxLayout(this);
    auto *more_bots_button = new QPushButton("HARDER");
    auto *bots_count = new QLineEdit(this);
    auto *less_bots_button = new QPushButton("SIMPLER");
    auto *start_game_button = new QPushButton("READY");
    auto *cancel_game_button = new QPushButton("← MENU");

    layout->addWidget(game_set_label, 0, Qt::AlignCenter);
    layout->addStretch();
    layout->addLayout(setting_layout);
    setting_layout->addLayout(set_rounds_layout);
    set_rounds_layout->addWidget(rounds_hint, 0, Qt::AlignCenter);
    set_rounds_layout->addLayout(change_rounds_layout);
    change_rounds_layout->addWidget(more_rounds_button, 0, Qt::AlignCenter);
    change_rounds_layout->addWidget(rounds_count, 0, Qt::AlignCenter);
    change_rounds_layout->addWidget(less_rounds_button, 0, Qt::AlignCenter);
    setting_layout->addLayout(set_bots_layout);
    set_bots_layout->addWidget(bots_hint, 0, Qt::AlignCenter);
    set_bots_layout->addLayout(bots_change_layout);
    bots_change_layout->addWidget(more_bots_button, 0, Qt::AlignCenter);
    bots_change_layout->addWidget(bots_count, 0, Qt::AlignCenter);
    bots_change_layout->addWidget(less_bots_button, 0, Qt::AlignCenter);
    layout->addWidget(start_game_button, 0, Qt::AlignCenter);
    layout->addWidget(cancel_game_button, 0, Qt::AlignLeft);

    game_set_label->setStyleSheet(
        "font-size: 84pt;"
        "border: none;"
        "background: transparent;"
    );
    rounds_hint->setStyleSheet(
        "font-size: 48pt;"
        "font-family: \"Wattauchimma\";"
        "color: cyan;"
        "border: none;"
        "background: transparent;"
    );

    int start_window_width = this->width(), start_window_height = this->height();

    more_rounds_button->setFixedWidth(start_window_width * 0.2);
    more_rounds_button->setFixedHeight(start_window_height * 0.2);
    more_rounds_button->setStyleSheet(
        "font-size: 72pt;"
        "font-family: \"Wattauchimma\";"
        "font-weight: bold;"
        "color: black;"
        "border: none;"
        "padding-bottom: 5px;"
        "background: rgb(192, 50, 113);"
    );
    rounds_count->setFixedWidth(start_window_height * 0.3);
    rounds_count->setFixedHeight(start_window_width * 0.2);
    rounds_count->setStyleSheet(
        "font-size: 36pt;"
        "font-family: \"TraktorMoodFont\";"
        "color: rgb(192, 50, 33);"
        "border: 3px solid rgb(242, 208, 164);"
        "background: black;"
    );
    less_rounds_button->setFixedWidth(start_window_width * 0.2);
    less_rounds_button->setFixedHeight(start_window_height * 0.2);
    less_rounds_button->setStyleSheet(
        "font-size: 72pt;"
        "font-family: \"Wattauchimma\";"
        "font-weight: bold;"
        "color: black;"
        "border: none;"
        "padding-bottom: 5px;"
        "background: rgb(192, 50, 113);"
    );
    bots_hint->setStyleSheet(
        "font-size: 48pt;"
        "font-family: \"Wattauchimma\";"
        "color: cyan;"
        "border: none;"
        "background: transparent;"
    );
    more_bots_button->setFixedWidth(start_window_width * 0.4);
    more_bots_button->setFixedHeight(start_window_height * 0.2);
    more_bots_button->setStyleSheet(
        "font-size: 36pt;"
        "font-family: \"Wattauchimma\";"
        "font-weight: bold;"
        "color: black;"
        "border: none;"
        "padding-bottom: 3px;"
        "background: rgb(242, 208, 164);"
    );
    bots_count->setFixedWidth(start_window_height * 0.3);
    bots_count->setFixedHeight(start_window_width * 0.2);
    bots_count->setStyleSheet(
        "font-size: 36pt;"
        "font-family: \"TraktorMoodFont\";"
        "color: rgb(242, 208, 164);"
        "border: 3px solid rgb(192, 50, 113);"
        "background: black;"
    );
    less_bots_button->setFixedWidth(start_window_width * 0.4);
    less_bots_button->setFixedHeight(start_window_height * 0.2);
    less_bots_button->setStyleSheet(
        "font-size: 36pt;"
        "font-family: \"Wattauchimma\";"
        "font-weight: bold;"
        "color: black;"
        "border: none;"
        "padding-bottom: 3px;"
        "background: rgb(242, 208, 164);"
    );
    start_game_button->setStyleSheet(
        "QPushButton {"
            "font-size: 72pt;"
            "font-family: \"FREE FAT FONT\";"
            "color: black;"
            "border: none;"
            "padding-bottom: 10px;"
            "padding-left: 10px;"
            "padding-right: 10px;"
            "margin-top: 2xPx0px;"
            "background-color: cyan;"
        "}"

        "QPushButton:hover { background-color: rgb(73, 159, 104); }"
    );
    cancel_game_button->setStyleSheet(
        "font-size: 60pt;"
        "font-family: \"FREE FAT FONT\";"
        "border: none;"
        "background: transparent;"
    );

    rounds_count->setAlignment(Qt::AlignCenter);
    bots_count->setAlignment(Qt::AlignCenter);

    rounds_count->setValidator(new QIntValidator(fieldSizeMin, fieldSizeMax, rounds_count));
    bots_count->setValidator(new QIntValidator(botsMin, botsMax, bots_count));

    rounds_count->setText(QString::number(g_currentFieldSize));
    bots_count->setText(QString::number(g_currentBotsCount));

    auto top_label_glow = new QGraphicsDropShadowEffect(game_set_label);

    top_label_glow->setBlurRadius(24);
    top_label_glow->setColor(qRgb(0, 255, 255));
    top_label_glow->setOffset(0, 0);

    auto rounds_hint_glow = new QGraphicsDropShadowEffect(rounds_hint);

    rounds_hint_glow->setBlurRadius(20);
    rounds_hint_glow->setColor(qRgb(0, 255, 255));
    rounds_hint_glow->setOffset(0, 0);

    auto more_rounds_glow = new QGraphicsDropShadowEffect(more_rounds_button);

    more_rounds_glow->setBlurRadius(24);
    more_rounds_glow->setColor(qRgb(192, 50, 113));
    more_rounds_glow->setOffset(0, 0);

    auto less_rounds_glow = new QGraphicsDropShadowEffect(less_rounds_button);

    less_rounds_glow->setBlurRadius(24);
    less_rounds_glow->setColor(qRgb(192, 50, 113));
    less_rounds_glow->setOffset(0, 0);

    auto bots_hint_glow = new QGraphicsDropShadowEffect(bots_hint);

    bots_hint_glow->setBlurRadius(20);
    bots_hint_glow->setColor(qRgb(0, 255, 255));
    bots_hint_glow->setOffset(0, 0);

    auto more_bots_glow = new QGraphicsDropShadowEffect(more_bots_button);

    more_bots_glow->setBlurRadius(24);
    more_bots_glow->setColor(qRgb(242, 208, 164));
    more_bots_glow->setOffset(0, 0);

    auto less_bots_glow = new QGraphicsDropShadowEffect(less_bots_button);

    less_bots_glow->setBlurRadius(24);
    less_bots_glow->setColor(qRgb(242, 208, 164));
    less_bots_glow->setOffset(0, 0);

    auto start_game_glow = new QGraphicsDropShadowEffect(start_game_button);

    start_game_glow->setBlurRadius(36);
    start_game_glow->setColor(qRgb(0, 255, 255));
    start_game_glow->setOffset(0, 0);

    game_set_label->setGraphicsEffect(top_label_glow);
    rounds_hint->setGraphicsEffect(rounds_hint_glow);
    more_rounds_button->setGraphicsEffect(more_rounds_glow);
    less_rounds_button->setGraphicsEffect(less_rounds_glow);
    bots_hint->setGraphicsEffect(bots_hint_glow);
    more_bots_button->setGraphicsEffect(more_bots_glow);
    less_bots_button->setGraphicsEffect(less_bots_glow);
    start_game_button->setGraphicsEffect(start_game_glow);

    connect(start_game_button, &QPushButton::clicked, this,
        [this, rounds_count, bots_count]() {
            bool ok1 = false, ok2 = false;
            int rounds = rounds_count->text().toInt(&ok1);
            int bots   = bots_count->text().toInt(&ok2);

            if (!ok1) rounds = g_currentFieldSize;
            if (!ok2) bots   = g_currentBotsCount;

            g_currentFieldSize = rounds;
            g_currentBotsCount = bots;

            QWidget* w = parentWidget();
            while (w && qobject_cast<mainwindow*>(w) == nullptr) {
                w = w->parentWidget();
            }
            if (auto* mw = qobject_cast<mainwindow*>(w)) {
                mw->startGame(rounds, bots);
            }

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

    connect(more_rounds_button, &QPushButton::clicked, this,
            [rounds_count, fieldSizeMin, fieldSizeMax]() {
        bool ok = false;
        int value = rounds_count->text().toInt(&ok);
        if (!ok) {
            value = fieldSizeMin;
        }

        if (value < fieldSizeMax) {
            ++value;
            g_currentFieldSize = value;
            rounds_count->setText(QString::number(value));
        }
    });

    connect(less_rounds_button, &QPushButton::clicked, this,
            [rounds_count, fieldSizeMin, fieldSizeMax]() {
        bool ok = false;
        int value = rounds_count->text().toInt(&ok);
        if (!ok) {
            value = fieldSizeMin;
        }

        if (value > fieldSizeMin) {
            --value;
            g_currentFieldSize = value;
            rounds_count->setText(QString::number(value));
        }
    });

    connect(more_bots_button, &QPushButton::clicked, this,
            [bots_count, botsMin, botsMax]() {
        bool ok = false;
        int bots = bots_count->text().toInt(&ok);
        if (!ok) {
            bots = botsMin;
        }

        if (bots < botsMax) {
            ++bots;
            g_currentBotsCount = bots;
            bots_count->setText(QString::number(bots));
        }
    });

    connect(less_bots_button, &QPushButton::clicked, this,
            [bots_count, botsMin, botsMax]() {
        bool ok = false;
        int bots = bots_count->text().toInt(&ok);
        if (!ok) {
            bots = botsMin;
        }

        if (bots > botsMin) {
            --bots;
            g_currentBotsCount = bots;
            bots_count->setText(QString::number(bots));
        }
    });

    connect(cancel_game_button, &QPushButton::clicked, this,
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

void GameStartWindow::showEvent(QShowEvent* event) {
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

void GameStartWindow::closeEvent(QCloseEvent* event) {
    if (!closing) {
        event->ignore();
        closing = true;

        auto *fade_out = new QPropertyAnimation(this, "windowOpacity");
        
        fade_out->setDuration(300);
        fade_out->setStartValue(windowOpacity());
        fade_out->setEndValue(0.0);

        connect(fade_out, &QPropertyAnimation::finished, this, [this]() { QDialog::close(); });
        
        fade_in_animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
    else QDialog::closeEvent(event);
}

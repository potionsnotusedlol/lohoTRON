#include "MainMenuWidget.h"
#include "ui_MainMenuWidget.h"

#include <QGraphicsDropShadowEffect>
#include <QResizeEvent>
#include <QPalette>
#include <QPixmap>

#include "SettingsWindow.h"
#include "GameStartWindow.h"
#include "CreatorsWindow.h"
#include "QuitGameWindow.h"

MainMenuWidget::MainMenuWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::MainMenuWidget)
{
    ui->setupUi(this);

    connect(ui->start_button, &QPushButton::clicked,
            this, &MainMenuWidget::onStartButtonClicked);

    connect(ui->settings_button, &QPushButton::clicked, this,
            [this]() {
                SettingsWindow settings_dlg(this);
                settings_dlg.exec();
            });

    connect(ui->creators_info_button, &QPushButton::clicked, this,
            [this]() {
                CreatorsWindow creators_dlg(this);
                creators_dlg.exec();
            });

    connect(ui->quit_button, &QPushButton::clicked, this,
            [this]() {
                QuitGameWindow quit_dlg(this);
                quit_dlg.exec();
            });

    auto glow_logo_label = new QGraphicsDropShadowEffect(ui->logo_label);
    glow_logo_label->setBlurRadius(20);
    glow_logo_label->setColor(qRgb(0, 191, 255));
    glow_logo_label->setOffset(0);

    auto glow_start_menubutton = new QGraphicsDropShadowEffect(ui->start_button);
    glow_start_menubutton->setBlurRadius(10);
    glow_start_menubutton->setColor(qRgb(0, 191, 255));
    glow_start_menubutton->setOffset(0);

    auto glow_settings_menubutton = new QGraphicsDropShadowEffect(ui->settings_button);
    glow_settings_menubutton->setBlurRadius(10);
    glow_settings_menubutton->setColor(qRgb(0, 191, 255));
    glow_settings_menubutton->setOffset(0);

    auto glow_creators_info_menubutton = new QGraphicsDropShadowEffect(ui->creators_info_button);
    glow_creators_info_menubutton->setBlurRadius(10);
    glow_creators_info_menubutton->setColor(qRgb(0, 191, 255));
    glow_creators_info_menubutton->setOffset(0);

    auto glow_quit_game_menubutton = new QGraphicsDropShadowEffect(ui->quit_button);
    glow_quit_game_menubutton->setBlurRadius(10);
    glow_quit_game_menubutton->setColor(qRgb(0, 191, 255));
    glow_quit_game_menubutton->setOffset(0);

    ui->logo_label->setGraphicsEffect(glow_logo_label);
    ui->start_button->setGraphicsEffect(glow_start_menubutton);
    ui->settings_button->setGraphicsEffect(glow_settings_menubutton);
    ui->creators_info_button->setGraphicsEffect(glow_creators_info_menubutton);
    ui->quit_button->setGraphicsEffect(glow_quit_game_menubutton);

    ui->logo_label->setStyleSheet(
        "background: transparent;"
        "font-family: \"Bolgarus Beta\";"
        "font-size: 200pt;"
        "color: rgb(0, 191, 255);"
    );

    ui->start_button->setStyleSheet(
        "QPushButton {"
            "background: transparent;"
            "font-family: \"FREE FAT FONT\";"
            "font-size: 96pt;"
            "color: rgb(255, 255, 255);"
        "}"
        "QPushButton:hover {"
            "background: transparent;"
            "font-family: \"FREE FAT FONT\";"
            "font-size: 96pt;"
            "color: rgb(0, 191, 255);"
        "}"
    );

    ui->settings_button->setStyleSheet(
        "QPushButton {"
            "background: transparent;"
            "font-family: \"FREE FAT FONT\";"
            "font-size: 96pt;"
            "color: rgb(255, 255, 255);"
        "}"
        "QPushButton:hover {"
            "background: transparent;"
            "font-family: \"FREE FAT FONT\";"
            "font-size: 96pt;"
            "color: rgb(0, 191, 255);"
        "}"
    );

    ui->creators_info_button->setStyleSheet(
        "QPushButton {"
            "background: transparent;"
            "font-family: \"FREE FAT FONT\";"
            "font-size: 96pt;"
            "color: rgb(255, 255, 255);"
        "}"
        "QPushButton:hover {"
            "background: transparent;"
            "font-family: \"FREE FAT FONT\";"
            "font-size: 96pt;"
            "color: rgb(0, 191, 255);"
        "}"
    );

    ui->quit_button->setStyleSheet(
        "QPushButton {"
            "background: transparent;"
            "font-family: \"FREE FAT FONT\";"
            "font-size: 96pt;"
            "color: rgb(255, 255, 255);"
        "}"
        "QPushButton:hover {"
            "background: transparent;"
            "font-family: \"FREE FAT FONT\";"
            "font-size: 96pt;"
            "color: rgb(0, 191, 255);"
        "}"
    );

    ui->main_screen_layout->setStretch(0, 3);
    ui->main_screen_layout->setStretch(1, 7);
}

MainMenuWidget::~MainMenuWidget()
{
    delete ui;
}

void MainMenuWidget::onStartButtonClicked()
{
    GameStartWindow game_dlg(this);

    connect(&game_dlg, &GameStartWindow::gameStarted,
            this, [this](int rounds, int bots) {
                emit startGameRequested(rounds, bots);
            });

    game_dlg.exec();
}

void MainMenuWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateSpacings();

    QPalette palette;
    QPixmap bg(":/images/bg_menu.png");

    palette.setBrush(QPalette::Window,
                     QBrush(bg.scaled(size(),
                                      Qt::KeepAspectRatioByExpanding,
                                      Qt::SmoothTransformation)));
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Window);
    this->setPalette(palette);
}

void MainMenuWidget::updateSpacings()
{
    int widget_height = this->height();
    int widget_width  = this->width();

    int target_button_width  = widget_width * 0.56;
    int target_button_height = widget_height * 0.11;

    QList<QPushButton*> main_menu_buttons = {
        ui->start_button,
        ui->settings_button,
        ui->creators_info_button,
        ui->quit_button,
    };

    for (auto btn : main_menu_buttons) {
        btn->setFixedHeight(target_button_height);
        btn->setFixedWidth(target_button_width);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }
}

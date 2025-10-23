#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QGraphicsDropShadowEffect>
#include "SettingsWindow.h"
#include "CreatorsWindow.h"

#include "GameStartWindow.h"

tron_menu::tron_menu(QWidget* parent) : QMainWindow(parent), ui(new Ui::tron_menu) {
    ui->setupUi(this);
    // applying the buttons their function
    connect(ui->start_button, &QPushButton::clicked, this,
        [this]() {
            GameStartWindow game_dlg(this);
            game_dlg.exec();
        }
    );
    connect(ui->settings_button, &QPushButton::clicked, this,
        [this]() {
            SettingsWindow dlg(this);
            dlg.exec();
        }
    );
    connect(ui->creators_info_button, &QPushButton::clicked, this,
        [this]() {
            CreatorsWindow creators_dlg(this);
            creators_dlg.exec();
        }
    );
    connect(ui->quit_button, SIGNAL(clicked()), this, SLOT(killGameProcess()));

    // creating the blue glowiing effect (same for each button, different for logo_label)
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

    // applying the effect for ecah button and label
    ui->logo_label->setGraphicsEffect(glow_logo_label);
    ui->start_button->setGraphicsEffect(glow_start_menubutton);
    ui->settings_button->setGraphicsEffect(glow_settings_menubutton);
    ui->creators_info_button->setGraphicsEffect(glow_creators_info_menubutton);
    ui->quit_button->setGraphicsEffect(glow_quit_game_menubutton);
    // setting styles for all the visible elements of the main menu (including ignoring the background image setting)
    ui->logo_label->setStyleSheet("background: transparent;"
    "font-family: \"Bolgarus Beta\"; font-size: 200pt; color: rgb(0, 191, 255);"); // also added a font style

    ui->start_button->setStyleSheet(
        "QPushButton {"
            "background: transparent;"
            "font-family: \"FREE FAT FONT\";"
            "font-size: 96pt;"
            "color: rgb(255, 255, 255);"
            "border: none;"  
            "outline: none;" 
        "}"

        "QPushButton:hover {"
            "background: transparent;" 
            "font-family: \"FREE FAT FONT\";"
            "font-size: 96pt;"
            "color: rgb(0, 191, 255);"  
            "border: none;"  
            "outline: none;" 
        "}"
    );

    ui->settings_button->setStyleSheet(
        "QPushButton {"
            "background: transparent;"
            "font-family: \"FREE FAT FONT\";"
            "font-size: 96pt;"
            "color: rgb(255, 255, 255);"
            "border: none;"
            "outline: none;"
        "}"

        "QPushButton:hover {"
            "background: transparent;"
            "font-family: \"FREE FAT FONT\";"
            "font-size: 96pt;"
            "color: rgb(0, 191, 255);"
            "border: none;"
            "outline: none;"
        "}"
    );

    ui->creators_info_button->setStyleSheet(
        "QPushButton {"
            "background: transparent;"
            "font-family: \"FREE FAT FONT\";"
            "font-size: 96pt;"
            "color: rgb(255, 255, 255);"
            "border: none;"
            "outline: none;"
        "}"

        "QPushButton:hover {"
            "background: transparent;"
            "font-family: \"FREE FAT FONT\";"
            "font-size: 96pt;"
            "color: rgb(0, 191, 255);"
            "border: none;"
            "outline: none;"
        "}"
    );

    ui->quit_button->setStyleSheet(
        "QPushButton {"
            "background: transparent;"
            "font-family: \"FREE FAT FONT\";"
            "font-size: 96pt;"
            "color: rgb(255, 255, 255);"
            "border: none;"
            "outline: none;"
        "}"

        "QPushButton:hover {"
            "background: transparent;"
            "font-family: \"FREE FAT FONT\";"
            "font-size: 96pt;"
            "color: rgb(0, 191, 255);"
            "border: none;"
            "outline: none;"
        "}"
    );

    // relatively reposition the logo and the buttons (30%-70%)
    ui->main_screen_layout->setStretch(0, 3);
    ui->main_screen_layout->setStretch(1, 7);
}

tron_menu::~tron_menu() { delete ui; }

void tron_menu::resizeEvent(QResizeEvent* event) {
    //QMainWindow::resizeEvent(event);
    updateSpacings();

    QPalette palette;
    QPixmap bg(":/images/bg_menu.png");
    palette.setBrush(QPalette::Window, QBrush(bg.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation)));

    this->setPalette(palette);
}

void tron_menu::updateSpacings() {
    int window_height = ui->centralwidget->height(), window_width = ui->centralwidget->width();
    int target_button_width = window_width * 0.56, target_button_height = window_height * 0.11;
    QList<QPushButton*> main_menu_buttons = {
        ui->start_button,
        ui->settings_button,
        ui->creators_info_button,
        ui->quit_button
    };
    
    for (auto btn : main_menu_buttons) {
        btn->setFixedHeight(target_button_height);
        btn->setFixedWidth(target_button_width);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }
}

// void tron_menu::showCreatorsInfo() {}

// void tron_menu::killGameProcess() {}
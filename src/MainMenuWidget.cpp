#include "MainMenuWidget.h"
#include "ui_MainMenuWidget.h"
#include <QGraphicsDropShadowEffect>
#include "SettingsWindow.h"
#include "GameStartWindow.h"
#include "CreatorsWindow.h"
#include "QuitGameWindow.h"

MainMenuWidget::MainMenuWidget(QWidget* parent) : QWidget(parent), ui(new Ui::MainMenuWidget) {
    ui->setupUi(this);

    // applying functions to buttons
    connect(ui->start_button, &QPushButton::clicked, this,
        [this]() {
            GameStartWindow game_dlg(this);
            game_dlg.exec();
        }
    );
    connect(ui->settings_button, &QPushButton::clicked, this,
        [this]() {
            SettingsWindow settings_dlg(this);
            settings_dlg.exec();
        }
    );
    connect(ui->creators_info_button, &QPushButton::clicked, this,
        [this]() {
            CreatorsWindow creators_dlg(this);
            creators_dlg.exec();
        }
    );
    connect(ui->quit_button, &QPushButton::clicked, this,
        [this]() {
            QuitGameWindow quit_dlg(this);
            quit_dlg.exec();
        }
    );
    //creating a media player for da music to blast thru da hood
    music_player = new QMediaPlayer;
    music_output = new QAudioOutput;
    music_player->setAudioOutput(music_output);
    music_output->setVolume(1);

    QStringList music_playlist = {
        "qrc:/music/Rock N Roll.mp3",
        "qrc:/music/Gangsta-Rap_-_Nigga-Nigga-Nigga_(muzichka.cc).mp3",
        "qrc:/music/videoplayback (1).mp3",
        "qrc:/music/Pi'erre Bourne - Be Mine.mp3",
        "qrc:/music/Tory Lanez - Sorry 4 What_  LV BELT.mp3",
        "qrc:/music/Pi'erre Bourne - Gotta Blast.mp3",
        "qrc:/music/SCOPIN.mp3",
        "qrc:/music/TAY-K - The Race 🏁 (Prod_ S.Diesel) [@DJPHATTT Exclusive] _VIDEO IN DESCRIPTION_.mp3",
        "qrc:/music/Gangsta-Rap_-_Nigga-Nigga-Nigga_(muzichka.cc).mp3",
        "qrc:/music/We're Finally Landing.mp3"
    };

    srand(time(NULL));

    unsigned short track_num = rand() % 10 + 1;

    music_player->setSource(QUrl(music_playlist[track_num - 1]));
    music_player->play();
    connect(this->music_player, &QMediaPlayer::mediaStatusChanged, this,
        [this, music_playlist, &track_num]() {
            if (music_player->mediaStatus() == QMediaPlayer::EndOfMedia || music_player->mediaStatus() == QMediaPlayer::NoMedia) {
                track_num = rand() % 10 + 1;
                music_player->setSource(QUrl(music_playlist[track_num - 1]));
                music_player->play();
            }
        }
    );

    // creating the blue glowing effect (same for each button, different for logo_label)
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

    // applying the effect for each button and label
    ui->logo_label->setGraphicsEffect(glow_logo_label);
    ui->start_button->setGraphicsEffect(glow_start_menubutton);
    ui->settings_button->setGraphicsEffect(glow_settings_menubutton);
    ui->creators_info_button->setGraphicsEffect(glow_creators_info_menubutton);
    ui->quit_button->setGraphicsEffect(glow_quit_game_menubutton);
    // setting styles for all the visible elements of the main menu (including ignoring the background image setting)
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
    // relatively reposition the logo and the buttons (30%-70%)
    ui->main_screen_layout->setStretch(0, 3);
    ui->main_screen_layout->setStretch(1, 7);
}

MainMenuWidget::~MainMenuWidget() { delete ui; }

// resize event listener
void MainMenuWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    updateSpacings();

    QPalette palette;
    QPixmap bg(":/images/bg_menu.png");

    palette.setBrush(QPalette::Window, QBrush(bg.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation)));
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Window);
    this->setPalette(palette);
}

// rebuilding the interface according to new dimensions
void MainMenuWidget::updateSpacings() {
    int widget_height = this->height(), widget_width = this->width();
    int target_button_width = widget_width * 0.56, target_button_height = widget_height * 0.11;
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

QMediaPlayer* MainMenuWidget::music() const { return music_player; }
#include <QApplication>
#include "mainwindow.h"
#include <QFontDatabase>
#include "SettingsWindow.h"

// function to define default settings for first game startup
void createDefaultRoot() {
    QJsonObject root = loadConfigRoot();

    if (!root.isEmpty()) return;

    QJsonObject player = root.value("player").toObject();
    player["name"] = "noname";
    player["color"] = "blue";

    QJsonArray keys = {"W", "S", "A", "D"};

    player["key_bindings"] = keys;
    root["player"] = player;

    QJsonObject envSet;
    envSet["bots_count_default"] = 3;
    envSet["bots_count_max"] = 10;
    envSet["bots_count_min"] = 1;
    envSet["field_size"] = 300;
    root["environment"] = envSet;
    saveConfigRoot(root);
}

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    QStringList fonts =  {
        ":/fonts/Bolgarus Beta.ttf",
        ":/fonts/FREEFATFONT-Regular.otf",
        ":/fonts/Wattauchimma.ttf",
        ":/fonts/TraktorMoodFont-Regular.otf"
    };

    // loading fonts into program DB
    for (const QString& font_path : fonts) int id = QFontDatabase::addApplicationFont(font_path);

    createDefaultRoot();

    mainwindow w;
    w.showFullScreen();

    return a.exec();
}
#include <QApplication>
#include "mainwindow.h"
#include <QFontDatabase>
#include "SettingsWindow.h"

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    QStringList fonts =  {
        ":/fonts/Bolgarus Beta.ttf",
        ":/fonts/FREEFATFONT-Regular.otf",
        ":/fonts/Wattauchimma.ttf"
    };

    for (const QString& font_path : fonts) int id = QFontDatabase::addApplicationFont(font_path);

    tron_menu w;
    w.showFullScreen();

    return a.exec();
}
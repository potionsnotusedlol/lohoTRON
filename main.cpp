#include <QApplication>
#include "mainwindow.h"
#include <QFontDatabase>

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

    tron_menu w;
    w.show();

    return a.exec();
}
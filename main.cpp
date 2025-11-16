#include <QApplication>
#include <QFontDatabase>
#include "mainwindow.h"

int main(int argc, char* argv[]) {
#ifdef Q_OS_UNIX
    qputenv("QT_QPA_PLATFORM", QByteArray("xcb"));
#endif

    QApplication a(argc, argv);

    QStringList fonts =  {
        ":/fonts/Bolgarus Beta.ttf",
        ":/fonts/FREEFATFONT-Regular.otf",
        ":/fonts/Wattauchimma.ttf",
        ":/fonts/TraktorMoodFont-Regular.otf"
    };

    for (const QString& font_path : fonts)
        QFontDatabase::addApplicationFont(font_path);

    mainwindow w;
    w.showFullScreen();

    return a.exec();
}

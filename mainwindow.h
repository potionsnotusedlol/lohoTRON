#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class tron_menu; }
QT_END_NAMESPACE

class tron_menu : public QMainWindow {
    Q_OBJECT
public:
    mainwindow(QWidget * parent = nullptr);
    // ~mainwindow();
};

#endif // MAINWINDOW_H
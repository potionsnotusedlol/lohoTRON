#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "MainMenuWidget.h"
#include "GameProcess.h"
#include <QStackedWidget>

class mainwindow : public QMainWindow {
    Q_OBJECT
public:
    mainwindow(QWidget * parent = nullptr);
    // ~mainwindow();
};

#endif // MAINWINDOW_H
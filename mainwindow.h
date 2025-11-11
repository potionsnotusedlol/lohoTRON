#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "MainMenuWidget.h"
#include "GameProcess.h"
#include <QStackedWidget>

class mainwindow : public QMainWindow {
    Q_OBJECT
public:
    tron_menu(QWidget * parent = nullptr);
    ~tron_menu();
protected:
    void resizeEvent(QResizeEvent* event) override;
// private slots:
//     void showCreatorsInfo();
//     void killGameProcess();
private:
    Ui::tron_menu *ui;

    void updateSpacings();
};

#endif // MAINWINDOW_H
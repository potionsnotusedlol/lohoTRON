#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class tron_menu; }
QT_END_NAMESPACE

class tron_menu : public QMainWindow {
    Q_OBJECT
public:
    tron_menu(QWidget * parent = nullptr);
    ~tron_menu();
protected:
    void resizeEvent(QResizeEvent* event) override;
private:
    Ui::tron_menu *ui;

    void updateSpacings();
};

#endif // MAINWINDOW_H
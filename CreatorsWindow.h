#ifndef CREATORSWINDOW_H
#define CREATORSWINDOW_H

#include <QDialog>
#include <QWidget>
#include <QCloseEvent>
#include <QPropertyAnimation>
#include <QVBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QAbstractAnimation>
#include <QFile>
#include <QTextStream>
#include <QString>

class CreatorsWindow : public QDialog {
    Q_OBJECT
public:
    explicit CreatorsWindow(QWidget* parent = nullptr);
protected:
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
private:
    QPropertyAnimation *fade_in_animation;
    bool closing = false;
};

#endif // CREATORSWINDOW_H
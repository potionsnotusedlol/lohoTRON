#ifndef CREATORSWINDOW_H
#define CREATORSWINDOW_H

#include <QDialog>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QCloseEvent>
#include <QShowEvent>

class CreatorsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit CreatorsWindow(QWidget *parent = nullptr);
    ~CreatorsWindow() = default; 

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onCloseClicked();

private:
    bool closing = false;
    QGraphicsOpacityEffect *opacity_effect;
    QPropertyAnimation *fade_in_animation;
    
    void setupUI();
};

#endif
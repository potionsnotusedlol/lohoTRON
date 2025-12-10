#ifndef GAMESTARTWINDOW_H
#define GAMESTARTWINDOW_H

#include <QDialog>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QVBoxLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QCloseEvent>
#include <QAbstractAnimation>
#include <QWidget>
#include <QRect>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonParseError>
#include <QIntValidator>
#include <QCoreApplication>

class GameStartWindow : public QDialog {
    Q_OBJECT
public:
    explicit GameStartWindow(QWidget* parent = nullptr);
protected:
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
private:
    QPropertyAnimation *fade_in_animation;
    bool closing = false;
};

#endif // GAMESTARTWINDOW_H
#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QDialog>
#include <QPropertyAnimation>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QCloseEvent>
#include <QLineEdit>
#include <QComboBox>
#include <QGraphicsDropShadowEffect>
#include <QWidget>
#include <QAbstractAnimation>
#include <QWidget>
#include <QRect>

class SettingsWindow : public QDialog {
    Q_OBJECT
public:
    explicit SettingsWindow(QWidget* parent = nullptr);
protected:
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
private:
    QPropertyAnimation *fade_in_animation;
    bool closing = false;
};

#endif // SETTINGSWINDOW_H
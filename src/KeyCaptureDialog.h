#ifndef KEYCAPTUREDIALOG_H
#define KEYCAPTUREDIALOG_H

#include <QDialog>
#include <QWidget>
#include <QCloseEvent>
#include <QPropertyAnimation>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QAbstractAnimation>
#include <QFile>
#include <QRect>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include "SettingsWindow.h"
#include "KeyCaptureProcess.h"

class KeyCaptureDialog : public QDialog {
    Q_OBJECT
public:
    explicit KeyCaptureDialog(QWidget* parent = nullptr);
protected:
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
private:
    QPropertyAnimation *fade_in_animation;
    bool closing = false;
};

#endif // KEYCAPTUREDIALOG_H
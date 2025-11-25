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
#include <QRect>
#include <QString>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QCoreApplication>
#include <QKeyEvent>
#include <QKeySequence>

class QKeyEvent;
class SettingsWindow : public QDialog {
    Q_OBJECT
public:
    explicit SettingsWindow(QWidget* parent = nullptr);
protected:
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
private:
    QPropertyAnimation *fade_in_animation;
    bool closing = false;

    QLineEdit*   player_name_input       = nullptr;
    QComboBox*   color_picker_dropdown   = nullptr;
    QPushButton* key_rebinding_button    = nullptr;

    bool    capturingKey   = false;
    QString keyBindingText;

    void loadSettingsFromConfig();
    void saveSettingsToConfig();

    void startKeyCapture();
    void stopKeyCapture();
};

#endif // SETTINGSWINDOW_H
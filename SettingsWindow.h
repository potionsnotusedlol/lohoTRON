#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QDialog>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QCloseEvent>
#include <QLineEdit>
#include <QComboBox>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>

class SettingsWindow : public QDialog {
    Q_OBJECT
public:
    explicit SettingsWindow(QWidget* parent = nullptr);
protected:
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private slots:  
    void onSaveClicked();
    void onCloseClicked();

private:
    QPropertyAnimation *fade_in_animation;
    bool closing = false;
    
    QLineEdit *player_name_input;      
    QComboBox *color_picker_dropdown;
};

#endif // SETTINGSWINDOW_H
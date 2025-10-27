#ifndef GAMESTARTWINDOW_H
#define GAMESTARTWINDOW_H

#include <QDialog>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QVBoxLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QCloseEvent>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
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
    
    QLineEdit *map_size_edit;      
    QLineEdit *bot_count_edit;     
    int map_size = 3;              
    int bot_count = 1;             
    
    void updateBotCountDisplay();
    void saveSettings();
    void loadSettings();
};

#endif // GAMESTARTWINDOW_H
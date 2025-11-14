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

class GameStartWindow : public QDialog {
    Q_OBJECT

public:
    explicit GameStartWindow(QWidget* parent = nullptr);
    
    int getRoundsCount() const { return roundsCount; }
    int getBotsCount() const { return botsCount; }

signals:
    void gameStarted(int rounds, int bots);

protected:
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private slots:
    void increaseRounds();
    void decreaseRounds();
    void increaseBots();
    void decreaseBots();
    void startGame();

private:
    QPropertyAnimation *fade_in_animation;
    bool closing = false;
    
    QLineEdit *rounds_count;
    QLineEdit *bots_count;
    int roundsCount = 3;
    int botsCount = 3;
    
    void updateRoundsDisplay();
    void updateBotsDisplay();
};

#endif // GAMESTARTWINDOW_H
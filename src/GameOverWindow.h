#ifndef GAMEOVERWINDOW_H
#define GAMEOVERWINDOW_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>

class GameOverWindow : public QDialog {
    Q_OBJECT
public:
    explicit GameOverWindow(QWidget* parent = nullptr);

    void setMatchResult(bool win, int killedBots, int wonRounds);

signals:
    void restartGame();
    void exitToMenu();

protected:
    void showEvent(QShowEvent* event) override;

private:
    void setupUI();
    void applyStyles();
    void setupConnections();

    QLabel* title_label;
    QLabel* stats_label;
    QPushButton* restart_button;
    QPushButton* exit_button;

    QPropertyAnimation* fade_in;
};

#endif // GAMEOVERWINDOW_H
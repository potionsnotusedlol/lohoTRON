#ifndef KEYBINDINGSWINDOW_H
#define KEYBINDINGSWINDOW_H

#include <QDialog>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QCloseEvent>
#include <QShowEvent>
#include <QKeyEvent>
#include <QJsonObject>

class KeyBindingsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit KeyBindingsWindow(QWidget *parent = nullptr);

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onCloseClicked();
    void onResetToDefaultClicked();
    void startKeyCapture(int actionIndex);

private:
    void setupUI();
    void updateKeyLabels();
    void resetToDefaultBindings();
    void saveKeyBindings();

    QGraphicsOpacityEffect *opacity_effect;
    QPropertyAnimation *fade_in_animation;
    bool closing = false;

    // Структура для хранения информации о действиях
    struct KeyAction {
        QString actionName;
        QString displayName;
        QString currentKey;
        QLabel *keyLabel;
        QPushButton *changeButton;
    };

    QList<KeyAction> keyActions;
    int capturingActionIndex = -1; // -1 означает, что не происходит захват

    // Названия действий и их отображаемые имена
    QMap<QString, QString> actionDisplayNames = {
        {"up", "Move Up"},
        {"down", "Move Down"}, 
        {"left", "Move Left"},
        {"right", "Move Right"},
        {"action", "Action"}
    };

    // Значения по умолчанию
    QJsonObject defaultBindings = {
        {"up", "W"},
        {"down", "S"},
        {"left", "A"},
        {"right", "D"},
        {"action", "Space"}
    };
};

#endif // KEYBINDINGSWINDOW_H
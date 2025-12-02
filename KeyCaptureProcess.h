#ifndef KEYCAPTUREPROCESS_H
#define KEYCAPTUREPROCESS_H

#include <QDialog>
#include <QWidget>
#include <QCloseEvent>
#include <QPropertyAnimation>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QAbstractAnimation>
#include <QRect>

class KeyCaptureProcess : public QDialog {
    Q_OBJECT
public:
    explicit KeyCaptureProcess(QWidget* parent = nullptr);
    int getKey() const;
protected:
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
private:
    QPropertyAnimation *fade_in_animation;
    bool closing = false;
    long key_selected;
};

#endif // KEYCAPTUREPROCESS_H
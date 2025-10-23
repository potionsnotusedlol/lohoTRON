#include "CreatorsWindow.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include <QCloseEvent>
#include <QShowEvent>

CreatorsWindow::CreatorsWindow(QWidget *parent) : QDialog(parent),
    opacity_effect(new QGraphicsOpacityEffect(this)),
    fade_in_animation(new QPropertyAnimation(opacity_effect, "opacity", this))
{
    setWindowTitle("About Creators");
    setFixedSize(500, 400);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setModal(true);
    setStyleSheet(
        "background-color: rgba(0, 0, 0, 230);"
        "color: cyan;"
        "border: 2px solid cyan;"
        "border-radius: 15px;"
        "font-family: Arial;"
    );

    setGraphicsEffect(opacity_effect);
    opacity_effect->setOpacity(0.0);
    
    setupUI();
}

void CreatorsWindow::setupUI()
{
    auto *layout = new QVBoxLayout(this);
    
    // Заголовок
    auto *title_label = new QLabel("CREATORS");
    auto *title_glow = new QGraphicsDropShadowEffect(title_label);
    title_glow->setBlurRadius(15);
    title_glow->setColor(QColor(0, 255, 255));
    title_glow->setOffset(0, 0);
    title_label->setGraphicsEffect(title_glow);
    title_label->setStyleSheet(
        "font-size: 48px;"
        "font-weight: bold;"
        "border: none;"
        "background: transparent;"
        "color: cyan;"
        "padding: 20px;"
    );
    
    // Информация о создателях
    auto *creators_info = new QLabel(
        "LOHO TRON GAME\n\n"
        "Developed by:\n"
        "• Your Name\n"
        "• Your Team\n\n"
        "Version: 1.0\n"
        "Release: 2024\n\n"
        "Special thanks to:\n"
        "Qt Framework\n"
        "Open Source Community"
    );
    
    creators_info->setStyleSheet(
        "font-size: 16px;"
        "font-weight: bold;"
        "border: none;"
        "background: transparent;"
        "color: rgb(0, 191, 255);"
        "padding: 20px;"
        "line-height: 1.5;"
    );
    creators_info->setAlignment(Qt::AlignCenter);
    
    // Кнопка закрытия
    auto *close_button = new QPushButton("CLOSE");
    close_button->setFixedSize(150, 50);
    close_button->setStyleSheet(
        "QPushButton {"
            "font-size: 18px;"
            "font-weight: bold;"
            "color: rgb(192, 50, 33);"
            "border: 2px solid rgb(192, 50, 33);"
            "border-radius: 8px;"
            "background: transparent;"
            "padding: 5px;"
        "}"
        "QPushButton:hover {"
            "background: rgba(192, 50, 33, 50);"
        "}"
    );
    
    layout->addWidget(title_label, 0, Qt::AlignCenter);
    layout->addWidget(creators_info, 1, Qt::AlignCenter);
    layout->addWidget(close_button, 0, Qt::AlignCenter);
    
    connect(close_button, &QPushButton::clicked, this, &CreatorsWindow::onCloseClicked);
}

void CreatorsWindow::onCloseClicked()
{
    closing = true;
    fade_in_animation->stop();
    fade_in_animation->setDuration(300);
    fade_in_animation->setStartValue(opacity_effect->opacity());
    fade_in_animation->setEndValue(0.0);
    fade_in_animation->start();
    connect(fade_in_animation, &QPropertyAnimation::finished, this, [this]() { 
        if (closing) close(); 
    });
}

void CreatorsWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);

    if (parentWidget()) {
        move(parentWidget()->geometry().center() - rect().center());
    }

    closing = false;
    fade_in_animation->stop();
    fade_in_animation->setDuration(300);
    fade_in_animation->setStartValue(0.0);
    fade_in_animation->setEndValue(1.0);
    fade_in_animation->start();
}

void CreatorsWindow::closeEvent(QCloseEvent *event)
{
    if (!closing) {
        event->ignore();
        closing = true;
        fade_in_animation->stop();
        fade_in_animation->setDuration(300);
        fade_in_animation->setStartValue(1.0);
        fade_in_animation->setEndValue(0.0);
        fade_in_animation->start();
        connect(fade_in_animation, &QPropertyAnimation::finished, this, [this]() { 
            QDialog::close(); 
        });
    } else {
        QDialog::closeEvent(event);
    }
}
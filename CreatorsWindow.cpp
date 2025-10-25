#include "CreatorsWindow.h"

CreatorsWindow::CreatorsWindow(QWidget* parent) : QDialog(parent) {
    fade_in_animation = new QPropertyAnimation(this, "windowOpacity", this);

    setWindowOpacity(0.0);
    setWindowTitle("Creators");
    resize(400, 300);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setModal(true);
    setStyleSheet(
        "background-color: rgba(0, 0, 0, 230);"
        "color: cyan;"
        "border: 2px solid cyan;"
        "border-radius: 10px;"
        "font-family: \"Bolgarus Beta\";"
    );

    auto *layout = new QVBoxLayout(this);
    auto *creators_label = new QLabel("CREATORS");
    auto *info_bar = new QTextEdit(this);
    auto *close_creators_button = new QPushButton("CLOSE");

    layout->addWidget(creators_label, 0, Qt::AlignCenter);
    layout->addStretch();
    layout->addWidget(info_bar, 0, Qt::AlignCenter);
    layout->addWidget(close_creators_button);

    creators_label->setStyleSheet(
        "font-size: 96px;"
        "border: none;"
        "background: transparent;"
    );

    int window_width = this->parentWidget()->width(), window_height = this->parentWidget()->height();

    info_bar->setFixedWidth(window_width * 0.8);
    info_bar->setFixedHeight(window_height * 0.5);
    info_bar->setReadOnly(true);

    QFile creators_file(":/creators_info.md");
    
    if (creators_file.open(QIODevice::ReadOnly | QIODevice::Text)){
        QTextStream ifs(&creators_file);
        QString creators_content = ifs.readAll();

        creators_file.close();

        info_bar->setStyleSheet(
        "font-size: 36pt;"
        "font-family: \"Wattauchimma\";"
        "color: rgb(235, 179, 169);"
        "border: none;"
        "background: transparent;"
    );

    info_bar->setMarkdown(creators_content);
    }
    else {
        info_bar->setStyleSheet("color: rgb(192, 50, 33);");
        info_bar->setText("FAILED TO LOAD CREATORS INFO!!");
    }
    
    close_creators_button->setStyleSheet(
        "font-size: 60pt;"
        "font-family: \"FREE FAT FONT\";"
        "border: none;"
        "background: transparent;"
    );

    connect(close_creators_button, &QPushButton::clicked, this,
        [this]() {
            if (!closing) {
                closing = true;

                auto *fade_out = new QPropertyAnimation(this, "windowOpacity");
                
                fade_out->setDuration(300);
                fade_out->setStartValue(windowOpacity());
                fade_out->setEndValue(0.0);

                connect(fade_out, &QPropertyAnimation::finished, this, [this]() { QDialog::close(); });

                fade_out->start(QAbstractAnimation::DeleteWhenStopped);
            }
        }
    );
}

void CreatorsWindow::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);

    if (parentWidget()) move(parentWidget()->geometry().center() - rect().center());

    closing = false;
    fade_in_animation->stop();
    fade_in_animation->setTargetObject(this);
    fade_in_animation->setPropertyName("windowOpacity");
    fade_in_animation->setDuration(300);
    fade_in_animation->setStartValue(0.0);
    fade_in_animation->setEndValue(1.0);
    fade_in_animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void CreatorsWindow::closeEvent(QCloseEvent* event) {
    if (!closing) {
        event->ignore();
        closing = true;

        auto *fade_out = new QPropertyAnimation(this, "windowOpacity");

        fade_out->setDuration(300);
        fade_out->setStartValue(windowOpacity());
        fade_out->setEndValue(0.0);

        connect(fade_out, &QPropertyAnimation::finished, this, [this]() { QDialog::close() ;});

        fade_out->start(QAbstractAnimation::DeleteWhenStopped);
    }
    else QDialog::closeEvent(event);
}
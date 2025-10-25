#include "SettingsWindow.h"

SettingsWindow::SettingsWindow(QWidget* parent) : QDialog(parent) {
    fade_in_animation = new QPropertyAnimation(this, "windowOpacity", this);

    setWindowOpacity(0.0);
    setWindowTitle("Settings");
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

    // initializing the interface elements
    auto *layout = new QVBoxLayout(this);
    auto *settings_label = new QLabel("SETTINGS");
    auto *player_name_hint = new QLabel("SET A NEW NICKNAME:");
    auto *player_name_input = new QLineEdit(this);
    auto *bottom_layout = new QHBoxLayout(this);
    auto *recolor_and_save_layout = new QVBoxLayout(this);
    auto *color_picker_hint = new QLabel("PICK A COLOR:");
    auto *color_picker_dropdown = new QComboBox(this);
    auto *save_button = new QPushButton("SAVE");
    auto *rebind_and_close_layout = new QVBoxLayout(this);
    auto *key_rebinding_button = new QPushButton("KEY\nBINDINGS");
    auto *close_settings_button = new QPushButton("CLOSE");

    // initializing the window layout
    layout->addWidget(settings_label, 0, Qt::AlignCenter);
    layout->addStretch();
    layout->addWidget(player_name_hint, 0, Qt::AlignCenter);
    layout->addWidget(player_name_input, 0, Qt::AlignCenter);
    layout->addLayout(bottom_layout);
    bottom_layout->addLayout(recolor_and_save_layout);
    bottom_layout->addLayout(rebind_and_close_layout);
    recolor_and_save_layout->addWidget(color_picker_hint, 0, Qt::AlignCenter);
    recolor_and_save_layout->addWidget(color_picker_dropdown, 0, Qt::AlignCenter);
    recolor_and_save_layout->addWidget(save_button, 0, Qt::AlignRight);
    rebind_and_close_layout->addWidget(key_rebinding_button, 0, Qt::AlignCenter);
    rebind_and_close_layout->addWidget(close_settings_button, 0, Qt::AlignLeft);
    
    // SETTINGS LABEL
    auto *text_glow = new QGraphicsDropShadowEffect(settings_label);

    text_glow->setBlurRadius(12);
    text_glow->setColor(qRgb(0, 255, 255));
    text_glow->setOffset(0, 0);
    settings_label->setGraphicsEffect(text_glow);
    settings_label->setStyleSheet(
        "font-size: 96px;"
        "border: none;"
        "background: transparent;"
    );

    // RENAME PLAYER LINE EDIT
    auto *rename_hint_glow = new QGraphicsDropShadowEffect(player_name_hint);

    rename_hint_glow->setBlurRadius(12);
    rename_hint_glow->setColor(qRgb(192, 50, 33));
    rename_hint_glow->setOffset(0, 0);
    player_name_hint->setGraphicsEffect(rename_hint_glow);
    player_name_hint->setStyleSheet(
        "font-size: 36pt;"
        "font-family: \"Wattauchimma\";"
        "color: rgb(192, 50, 33);"
        "border: none;"
        "padding-top: 20px;"
        "background: transparent;"
    );

    int settings_window_width = this->width(), settings_window_height = this->height();

    player_name_input->setMaxLength(50);
    player_name_input->setFixedWidth(settings_window_width * 1.4);
    player_name_input->setFixedHeight(settings_window_height * 0.2);
    player_name_input->setStyleSheet(
        "font-size: 36pt;"
        "font-family: \"TraktorMoodFont\";"
        "color: rgb(192, 50, 33);"
        "border: 3px solid rgb(242, 208, 164);"
        "background: black;"
        "placeholder-text-color: rgba(124, 122, 122, 1);"
    );
    player_name_input->setPlaceholderText("Type New Nickname");

    // REBIND KEYBOARD BUTTON
    auto *rebind_button_glow = new QGraphicsDropShadowEffect(key_rebinding_button);

    rebind_button_glow->setBlurRadius(12);
    rebind_button_glow->setColor(qRgb(192, 50, 33)); // asdas
    rebind_button_glow->setOffset(0, 0);
    key_rebinding_button->setGraphicsEffect(rebind_button_glow);
    key_rebinding_button->setFixedWidth(settings_window_width * 0.5);
    key_rebinding_button->setFixedHeight(settings_window_height * 0.35);
    key_rebinding_button->setStyleSheet(
        "font-size: 36pt;"
        "font-family: \"Wattauchimma\";"
        "color: black;"
        "border: none;"
        "margin-top: 20px;"
        "padding: 10px 10px 0 10px;"
        "background: rgb(192, 50, 33);"
    );

    // CHANGE PLAYER COLOR
    auto color_picker_hint_glow = new QGraphicsDropShadowEffect(color_picker_hint);

    color_picker_hint_glow->setBlurRadius(12);
    color_picker_hint_glow->setColor(qRgb(192, 50, 33));
    color_picker_hint_glow->setOffset(0, 0);
    color_picker_hint->setGraphicsEffect(color_picker_hint_glow);
    color_picker_hint->setStyleSheet(
        "font-size: 36pt;"
        "font-family: \"Wattauchimma\";"
        "color: rgb(192, 50, 33);"
        "border: none;"
        "padding-top: 20px;"
        "background: transparent;"
    );
    color_picker_dropdown->setFixedWidth(settings_window_width * 0.5);
    color_picker_dropdown->setFixedHeight(settings_window_height * 0.15);
    color_picker_dropdown->setEditable(true);
    color_picker_dropdown->lineEdit()->setReadOnly(true);
    color_picker_dropdown->lineEdit()->setAlignment(Qt::AlignCenter);
    color_picker_dropdown->addItem("blue");
    color_picker_dropdown->addItem("cyan");
    color_picker_dropdown->addItem("red");
    
    
    for (int i = 0; i < color_picker_dropdown->count(); ++i) color_picker_dropdown->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole);

    color_picker_dropdown->setStyleSheet(
        "font-size: 36pt;"
        "font-family: \"Wattauchimma\";"
        "color: black;"
        "border: none;"
        "background: cyan;"
    );
    // SAVE SETTINGS BUTTON
    save_button->setStyleSheet(
        "font-size: 60pt;"
        "font-family: \"FREE FAT FONT\";"
        "border: none;"
        "margin-right: 20px;"
        "background: transparent;"
    );
    // CLOSE BUTTON
    close_settings_button->setStyleSheet(
        "font-size: 60pt;"
        "font-family: \"FREE FAT FONT\";"
        "border: none;"
        "margin-left: 2px;"
        "background: transparent;"
    );

    // assigning bottom buttons their functionality
    connect(save_button, &QPushButton::clicked, this, [this]() {});
    connect(close_settings_button, &QPushButton::clicked, this,
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

// opening fade in animation handler
void SettingsWindow::showEvent(QShowEvent* event) {
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

// closing fade out animation handler
void SettingsWindow::closeEvent(QCloseEvent* event) {
    if (!closing) {
        event->ignore();
        closing = true;

        auto *fade_out = new QPropertyAnimation(this, "windowOpacity");

        fade_out->setDuration(300);
        fade_out->setStartValue(windowOpacity());
        fade_out->setEndValue(0.0);

        connect(fade_out, &QPropertyAnimation::finished, this, [this]() { QDialog::close(); });

        fade_out->start(QAbstractAnimation::DeleteWhenStopped);
    }
    else QDialog::closeEvent(event);
}
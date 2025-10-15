#include "SettingsWindow.h"

SettingsWindow::SettingsWindow(QWidget* parent) : QDialog(parent), opacity_effect(new QGraphicsOpacityEffect(this)), fade_in_animation(new QPropertyAnimation(opacity_effect, "opacity", this)) {
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

    setGraphicsEffect(opacity_effect);
    opacity_effect->setOpacity(0.0);

    auto *layout = new QVBoxLayout(this);
    auto *settings_label = new QLabel("SETTINGS");
    auto *player_name_hint = new QLabel("SET A NEW NICKNAME:");
    auto *player_name_input = new QLineEdit(this);
    auto *key_rebinding_button = new QPushButton("KEY BINDINGS");
    auto *color_picker_hint = new QLabel("PICK A COLOR:");
    auto *color_picker_dropdown = new QComboBox(this);
    auto *close_or_save_layout = new QHBoxLayout(this);
    auto *save_button = new QPushButton("SAVE");
    auto *close_settings_button = new QPushButton("CLOSE");

    layout->addWidget(settings_label, 0, Qt::AlignCenter);
    layout->addStretch();
    layout->addWidget(player_name_hint, 0, Qt::AlignCenter);
    layout->addWidget(player_name_input, 0, Qt::AlignCenter);
    layout->addWidget(key_rebinding_button, 0, Qt::AlignCenter);
    layout->addWidget(color_picker_hint, 0, Qt::AlignCenter);
    layout->addWidget(color_picker_dropdown, 0, Qt::AlignCenter);
    layout->addLayout(close_or_save_layout);
    close_or_save_layout->addWidget(save_button);
    close_or_save_layout->addWidget(close_settings_button, 0, Qt::AlignCenter);
    
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
        "color: black;"
        "border: none;"
        "background: rgb(242, 208, 164);"
    );
    player_name_input->setPlaceholderText("Type New Nickname");

    // REBIND KEYBOARD BUTTON
    auto *rebind_button_glow = new QGraphicsDropShadowEffect(key_rebinding_button);
    rebind_button_glow->setBlurRadius(12);
    rebind_button_glow->setColor(qRgb(243, 233, 0));
    rebind_button_glow->setOffset(0, 0);

    key_rebinding_button->setGraphicsEffect(rebind_button_glow);
    key_rebinding_button->setFixedWidth(settings_window_width * 1.25);
    key_rebinding_button->setFixedHeight(settings_window_height * 0.35);
    key_rebinding_button->setStyleSheet(
        "font-size: 60pt;"
        "font-family: \"Wattauchimma\";"
        "color: black;"
        "border: none;"
        "margin-top: 20px;"
        "padding: 10px 10px 0 10px;"
        "background: rgb(243, 233, 0);"
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
        "background: transparent;"
    );
    // CLOSE BUTTON
    close_settings_button->setStyleSheet(
        "font-size: 60pt;"
        "font-family: \"FREE FAT FONT\";"
        "border: none;"
        "background: transparent;"
    );

    connect(save_button, &QPushButton::clicked, this, [this]() {});
    connect(close_settings_button, &QPushButton::clicked, this,
        [this]() {
            closing = true;

            fade_in_animation->stop();
            fade_in_animation->setDuration(300);
            fade_in_animation->setStartValue(opacity_effect->opacity());
            fade_in_animation->setEndValue(0.0);
            fade_in_animation->start();
            connect(fade_in_animation, &QPropertyAnimation::finished, this, [this]() { if (closing) close(); });
        }
    );
}

void SettingsWindow::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);

    if (parentWidget()) move(parentWidget()->geometry().center() - rect().center());

    closing = false;

    fade_in_animation->stop();
    fade_in_animation->setDuration(300);
    fade_in_animation->setStartValue(0.0);
    fade_in_animation->setEndValue(1.0);
    fade_in_animation->start();
}

void SettingsWindow::closeEvent(QCloseEvent* event) {
    if (!closing) {
        event->ignore();
        
        closing = true;

        fade_in_animation->stop();
        fade_in_animation->setDuration(300);
        fade_in_animation->setStartValue(1.0);
        fade_in_animation->setEndValue(0.0);
        fade_in_animation->start();

        connect(fade_in_animation, &QPropertyAnimation::finished, this, [this]() { QDialog::close(); });
    }
    else QDialog::closeEvent(event);
}
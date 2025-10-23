#include "KeyBindingsWindow.h"
#include "ConfigManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include <QKeyEvent>
#include <QDebug>
#include <QMessageBox>

KeyBindingsWindow::KeyBindingsWindow(QWidget *parent) : QDialog(parent),
    opacity_effect(new QGraphicsOpacityEffect(this)),
    fade_in_animation(new QPropertyAnimation(opacity_effect, "opacity", this))
{
    setWindowTitle("Key Bindings");
    setFixedSize(500, 500);
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

void KeyBindingsWindow::setupUI()
{
    auto *layout = new QVBoxLayout(this);

    // Заголовок
    auto *title_label = new QLabel("KEY BINDINGS");
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

    layout->addWidget(title_label, 0, Qt::AlignCenter);

    // Инструкция
    auto *instruction_label = new QLabel("Click 'Change' and press any key to set new binding");
    instruction_label->setStyleSheet(
        "font-size: 14px;"
        "color: rgb(192, 50, 33);"
        "background: transparent;"
        "padding: 10px;"
    );
    instruction_label->setAlignment(Qt::AlignCenter);
    layout->addWidget(instruction_label);

    // Загружаем текущие привязки клавиш
    auto& config = ConfigManager::instance();
    QJsonObject currentBindings = config.getKeyBindings();

    // Инициализируем keyActions
    for (auto it = actionDisplayNames.begin(); it != actionDisplayNames.end(); ++it) {
        QString action = it.key();
        QString displayName = it.value();
        QString key = currentBindings[action].toString(defaultBindings[action].toString());

        KeyAction keyAction;
        keyAction.actionName = action;
        keyAction.displayName = displayName;
        keyAction.currentKey = key;

        // Создаем горизонтальный layout для каждого действия
        auto *actionLayout = new QHBoxLayout();

        // Название действия
        auto *actionLabel = new QLabel(displayName + ":");
        actionLabel->setStyleSheet(
            "font-size: 18px;"
            "font-weight: bold;"
            "color: cyan;"
            "background: transparent;"
            "padding: 10px;"
            "min-width: 150px;"
        );
        actionLayout->addWidget(actionLabel);

        // Отображение текущей клавиши
        keyAction.keyLabel = new QLabel(key);
        keyAction.keyLabel->setStyleSheet(
            "font-size: 18px;"
            "font-weight: bold;"
            "color: rgb(192, 50, 33);"
            "background: rgba(255, 255, 255, 30);"
            "padding: 10px;"
            "border: 1px solid cyan;"
            "border-radius: 5px;"
            "min-width: 80px;"
        );
        keyAction.keyLabel->setAlignment(Qt::AlignCenter);
        actionLayout->addWidget(keyAction.keyLabel);

        // Кнопка для изменения
        keyAction.changeButton = new QPushButton("Change");
        keyAction.changeButton->setFixedSize(100, 40);
        keyAction.changeButton->setStyleSheet(
            "QPushButton {"
                "font-size: 16px;"
                "font-weight: bold;"
                "color: black;"
                "border: none;"
                "border-radius: 5px;"
                "background: cyan;"
                "padding: 5px;"
            "}"
            "QPushButton:hover {"
                "background: rgb(0, 191, 255);"
            "}"
        );
        actionLayout->addWidget(keyAction.changeButton);

        layout->addLayout(actionLayout);

        // Добавляем в список
        keyActions.append(keyAction);
    }

    layout->addStretch();

    // Кнопки Reset и Close
    auto *bottomLayout = new QHBoxLayout();

    auto *resetButton = new QPushButton("RESET TO DEFAULT");
    resetButton->setFixedSize(200, 50);
    resetButton->setStyleSheet(
        "QPushButton {"
            "font-size: 16px;"
            "font-weight: bold;"
            "color: black;"
            "border: none;"
            "border-radius: 8px;"
            "background: rgb(192, 50, 33);"
            "padding: 5px;"
        "}"
        "QPushButton:hover {"
            "background: rgb(212, 70, 53);"
        "}"
    );

    auto *closeButton = new QPushButton("CLOSE");
    closeButton->setFixedSize(150, 50);
    closeButton->setStyleSheet(
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

    bottomLayout->addWidget(resetButton);
    bottomLayout->addWidget(closeButton);
    layout->addLayout(bottomLayout);

    // Подключаем кнопки
    connect(closeButton, &QPushButton::clicked, this, &KeyBindingsWindow::onCloseClicked);
    connect(resetButton, &QPushButton::clicked, this, &KeyBindingsWindow::onResetToDefaultClicked);

    // Подключаем кнопки изменения для каждого действия
    for (int i = 0; i < keyActions.size(); ++i) {
        connect(keyActions[i].changeButton, &QPushButton::clicked, this, [this, i]() {
            startKeyCapture(i);
        });
    }
}

void KeyBindingsWindow::startKeyCapture(int actionIndex)
{
    // Сбрасываем предыдущий захват
    if (capturingActionIndex != -1) {
        keyActions[capturingActionIndex].changeButton->setText("Change");
        keyActions[capturingActionIndex].changeButton->setStyleSheet(
            "QPushButton {"
                "font-size: 16px;"
                "font-weight: bold;"
                "color: black;"
                "border: none;"
                "border-radius: 5px;"
                "background: cyan;"
                "padding: 5px;"
            "}"
            "QPushButton:hover {"
                "background: rgb(0, 191, 255);"
            "}"
        );
    }

    capturingActionIndex = actionIndex;
    keyActions[actionIndex].changeButton->setText("Press any key...");
    keyActions[actionIndex].changeButton->setStyleSheet(
        "QPushButton {"
            "font-size: 16px;"
            "font-weight: bold;"
            "color: black;"
            "border: none;"
            "border-radius: 5px;"
            "background: yellow;"
            "padding: 5px;"
        "}"
    );

    // Обновляем инструкцию
    QString instruction = "Now press any key for: " + keyActions[actionIndex].displayName;
    // Здесь можно обновить label с инструкцией, если он есть
}

void KeyBindingsWindow::keyPressEvent(QKeyEvent *event)
{
    if (capturingActionIndex != -1) {
        // Игнорируем специальные клавиши, которые используются для навигации
        if (event->key() == Qt::Key_Escape) {
            // Отмена захвата
            keyActions[capturingActionIndex].changeButton->setText("Change");
            keyActions[capturingActionIndex].changeButton->setStyleSheet(
                "QPushButton {"
                    "font-size: 16px;"
                    "font-weight: bold;"
                    "color: black;"
                    "border: none;"
                    "border-radius: 5px;"
                    "background: cyan;"
                    "padding: 5px;"
                "}"
                "QPushButton:hover {"
                    "background: rgb(0, 191, 255);"
                "}"
            );
            capturingActionIndex = -1;
            return;
        }

        QString keyText = QKeySequence(event->key()).toString();
        
        // Обработка специальных клавиш
        if (event->key() == Qt::Key_Space) {
            keyText = "Space";
        } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            keyText = "Enter";
        } else if (event->key() == Qt::Key_Tab) {
            keyText = "Tab";
        } else if (event->key() == Qt::Key_Backspace) {
            keyText = "Backspace";
        } else if (event->key() == Qt::Key_Delete) {
            keyText = "Delete";
        } else if (event->key() == Qt::Key_Shift) {
            keyText = "Shift";
        } else if (event->key() == Qt::Key_Control) {
            keyText = "Ctrl";
        } else if (event->key() == Qt::Key_Alt) {
            keyText = "Alt";
        } else if (event->key() == Qt::Key_Meta) {
            keyText = "Meta";
        } else if (keyText.isEmpty()) {
            // Если не удалось получить текстовое представление
            keyText = "Key_" + QString::number(event->key());
        }

        // Проверяем, не используется ли эта клавиша для другого действия
        for (int i = 0; i < keyActions.size(); ++i) {
            if (i != capturingActionIndex && keyActions[i].currentKey == keyText) {
                // Клавиша уже используется, спрашиваем подтверждение
                QMessageBox::StandardButton reply;
                reply = QMessageBox::question(this, "Key Already Used", 
                    QString("Key '%1' is already used for '%2'. Overwrite?").arg(keyText).arg(keyActions[i].displayName),
                    QMessageBox::Yes | QMessageBox::No);
                
                if (reply == QMessageBox::No) {
                    return;
                }
                break;
            }
        }

        // Обновляем отображение
        keyActions[capturingActionIndex].currentKey = keyText;
        keyActions[capturingActionIndex].keyLabel->setText(keyText);
        keyActions[capturingActionIndex].changeButton->setText("Change");
        keyActions[capturingActionIndex].changeButton->setStyleSheet(
            "QPushButton {"
                "font-size: 16px;"
                "font-weight: bold;"
                "color: black;"
                "border: none;"
                "border-radius: 5px;"
                "background: cyan;"
                "padding: 5px;"
            "}"
            "QPushButton:hover {"
                "background: rgb(0, 191, 255);"
            "}"
        );

        // Сохраняем в конфиг
        saveKeyBindings();

        qDebug() << "Key binding saved for" << keyActions[capturingActionIndex].displayName << ":" << keyText;

        capturingActionIndex = -1;
    } else {
        QDialog::keyPressEvent(event);
    }
}

void KeyBindingsWindow::saveKeyBindings()
{
    QJsonObject newBindings;
    for (const KeyAction& action : keyActions) {
        newBindings[action.actionName] = action.currentKey;
    }
    
    ConfigManager::instance().setKeyBindings(newBindings);
    bool success = ConfigManager::instance().saveConfig();
    
    if (success) {
        qDebug() << "Key bindings saved successfully to config file";
    } else {
        qDebug() << "Failed to save key bindings to config file";
    }
}

void KeyBindingsWindow::onCloseClicked()
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

void KeyBindingsWindow::onResetToDefaultClicked()
{
    resetToDefaultBindings();
}

void KeyBindingsWindow::resetToDefaultBindings()
{
    for (int i = 0; i < keyActions.size(); ++i) {
        QString action = keyActions[i].actionName;
        keyActions[i].currentKey = defaultBindings[action].toString();
        keyActions[i].keyLabel->setText(keyActions[i].currentKey);
    }

    // Сохраняем в конфиг
    saveKeyBindings();
    
    qDebug() << "Key bindings reset to default";
}

void KeyBindingsWindow::showEvent(QShowEvent *event)
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

void KeyBindingsWindow::closeEvent(QCloseEvent *event)
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
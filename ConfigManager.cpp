#include "ConfigManager.h"
#include <QDir>
#include <QDebug>

ConfigManager::ConfigManager()
{
    m_configPath = QDir::currentPath() + "/game_config.json";
}

ConfigManager& ConfigManager::instance()
{
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::loadConfig()
{
    QFile file(m_configPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Config file not found, creating default...";
        createDefaultConfig();
        return saveConfig();
    }
    
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    
    if (doc.isNull()) {
        qDebug() << "Invalid config file, creating default...";
        createDefaultConfig();
        return saveConfig();
    }
    
    m_config = doc.object();
    return true;
}

bool ConfigManager::saveConfig()
{
    QFile file(m_configPath);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Failed to open config file for writing";
        return false;
    }
    
    QJsonDocument doc(m_config);
    file.write(doc.toJson());
    file.close();
    return true;
}

void ConfigManager::createDefaultConfig()
{
    m_config["player_name"] = "Player";
    m_config["player_color"] = "cyan";
    
    QJsonObject keyBindings;
    keyBindings["up"] = "W";
    keyBindings["down"] = "S";
    keyBindings["left"] = "A";
    keyBindings["right"] = "D";
    keyBindings["action"] = "Space";
    
    m_config["key_bindings"] = keyBindings;
}

QString ConfigManager::getPlayerName() const
{
    return m_config["player_name"].toString("Player");
}

void ConfigManager::setPlayerName(const QString &name)
{
    m_config["player_name"] = name;
}

QString ConfigManager::getPlayerColor() const
{
    return m_config["player_color"].toString("cyan");
}

void ConfigManager::setPlayerColor(const QString &color)
{
    m_config["player_color"] = color;
}

QJsonObject ConfigManager::getKeyBindings() const
{
    return m_config["key_bindings"].toObject();
}

void ConfigManager::setKeyBindings(const QJsonObject &bindings)
{
    m_config["key_bindings"] = bindings;
}
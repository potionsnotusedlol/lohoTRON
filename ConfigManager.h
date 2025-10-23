#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QDebug>

class ConfigManager
{
public:
    ConfigManager();
    
    bool loadConfig();
    bool saveConfig();
    
    QString getPlayerName() const;
    void setPlayerName(const QString &name);
    
    QString getPlayerColor() const;
    void setPlayerColor(const QString &color);
    
    QJsonObject getKeyBindings() const;
    void setKeyBindings(const QJsonObject &bindings);
    
    static ConfigManager& instance();

private:
    QJsonObject m_config;
    QString m_configPath;
    
    void createDefaultConfig();
};

#endif
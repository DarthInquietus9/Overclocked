#pragma once

#include <QObject>
#include <QString>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <unordered_map>

class ResilienceEngine : public QObject {
    Q_OBJECT
public:
    ResilienceEngine(const QString& configPath = "rules.json", QObject *parent = nullptr) : QObject(parent) {
        loadRules(configPath);
    }

    QString resolveViaRules(const QString& naturalLanguagePrompt) {
        QString lowerPrompt = naturalLanguagePrompt.toLower().trimmed();

        // Check if any rule trigger matches the user's prompt
        for (const auto& pair : m_fallbackRules) {
            if (lowerPrompt.contains(pair.first)) {
                return pair.second;
            }
        }

        return "-- [FALLBACK ERROR] Local inference offline and no matching rule found in rules.json.\n"
               "-- Edit rules.json to add custom deterministic query mappings.";
    }

private:
    std::unordered_map<QString, QString> m_fallbackRules;

    void loadRules(const QString& path) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            // Default baseline fallback if rules.json is missing from the build folder
            m_fallbackRules["all users"] = "SELECT * FROM users;";
            return;
        }

        QByteArray data = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isNull() && doc.isObject()) {
            QJsonArray rulesArray = doc.object()["fallback_rules"].toArray();
            for (const QJsonValue& val : rulesArray) {
                QJsonObject obj = val.toObject();
                QString trigger = obj["trigger"].toString().toLower();
                QString sql = obj["sql"].toString();
                if (!trigger.isEmpty() && !sql.isEmpty()) {
                    m_fallbackRules[trigger] = sql;
                }
            }
        }
    }
};
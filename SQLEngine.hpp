#pragma once

#include <QString>
#include <QStringList>
#include <QVector>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QRegularExpression>
#include <functional>
#include <iostream>
#include <pqxx/pqxx>

struct QueryResult {
    bool success = false;
    QString generatedSQL;
    QString explanation;
    QStringList headers;
    QVector<QStringList> rows;
    QString errorMessage;
};

class SQLEngine : public QObject {
    Q_OBJECT
private:
    QNetworkAccessManager* networkManager;
    QString dbConnStr = "dbname=hackathon_db user=postgres password=postgres host=127.0.0.1 port=5432";

public:
    SQLEngine(QObject *parent = nullptr) : QObject(parent) {
        networkManager = new QNetworkAccessManager(this);
    }

    void initDatabase() {
        // Database connection validation point
    }

    void translateAndExecute(const QString& userInput, std::function<void(QueryResult)> callback) {
        // Construct payload for Ollama (phi3:mini)
        QJsonObject promptObj;
        promptObj["model"] = "phi3:mini";

        QString systemPrompt = "You are an advanced Text-to-SQL engine for a PostgreSQL database. "
                               "The database has two tables:\n"
                               "1. students (id SERIAL, name VARCHAR, subject VARCHAR, score INT)\n"
                               "2. server_logs (id SERIAL, service_name VARCHAR, status_level VARCHAR, response_time_ms INT, timestamp TIMESTAMP)\n"
                               "Convert the user's natural language request into a valid PostgreSQL query. "
                               "Return ONLY the SQL query string wrapped in a JSON object with keys 'sql' and 'explanation'.";

        promptObj["prompt"] = systemPrompt + "\n\nUser request: " + userInput;
        promptObj["stream"] = false;

        QNetworkRequest request(QUrl("http://localhost:11434/api/generate"));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QNetworkReply* reply = networkManager->post(request, QJsonDocument(promptObj).toJson());

        // Handle asynchronous network reply with automatic fallback on failure
        connect(reply, &QNetworkReply::finished, this, [=]() {
            reply->deleteLater();
            QueryResult res;

            if (reply->error() == QNetworkReply::NoError) {
                QByteArray responseData = reply->readAll();
                QJsonDocument doc = QJsonDocument::fromJson(responseData);
                QString llmOutput = doc.object()["response"].toString().trimmed();

                // Clean potential markdown blocks from model output
                llmOutput.remove("```sql");
                llmOutput.remove("```");
                llmOutput = llmOutput.trimmed();

                if (!llmOutput.isEmpty()) {
                    if (llmOutput.startsWith("{")) {
                        QJsonDocument jsonOut = QJsonDocument::fromJson(llmOutput.toUtf8());
                        res.generatedSQL = jsonOut.object()["sql"].toString();
                        res.explanation = "Ollama (phi3:mini): " + jsonOut.object()["explanation"].toString();
                    } else {
                        res.generatedSQL = llmOutput;
                        res.explanation = "Ollama (phi3:mini): Generated dynamic SQL translation.";
                    }

                    if (executeAndFill(res)) {
                        callback(res);
                        return;
                    }
                }
            }

            // Fallback safety net: If Ollama fails or times out, use rules.json
            res = processOfflineFallback(userInput);
            callback(res);
        });
    }

private:
    QueryResult processOfflineFallback(const QString& userInput) {
        QueryResult res;
        QString input = userInput.toLower();

        QFile file("rules.json");
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            QJsonArray rules = doc.object()["rules"].toArray();

            for (const auto& val : rules) {
                QJsonObject rule = val.toObject();
                QJsonArray keywords = rule["keywords"].toArray();

                bool matched = false;
                for (const auto& kw : keywords) {
                    if (input.contains(kw.toString())) {
                        matched = true;
                        break;
                    }
                }

                if (matched) {
                    QString sql = rule["sql"].toString();
                    QString explanation = rule["explanation"].toString();

                    if (sql.contains("{subject}")) {
                        QString subj = "math";
                        if (input.contains("science")) subj = "science";
                        sql.replace("{subject}", subj);
                        explanation.replace("{subject}", subj);
                    } else if (sql.contains("{param}")) {
                        QRegularExpression re(rule["regex"].toString());
                        QRegularExpressionMatch match = re.match(input);
                        QString param = rule["default_param"].toString();
                        if (match.hasMatch() && !match.captured(1).isEmpty()) {
                            param = match.captured(1);
                        }
                        sql.replace("{param}", param);
                        explanation.replace("{param}", param);
                    }

                    res.generatedSQL = sql;
                    res.explanation = "External Rule Engine: " + explanation;
                    executeAndFill(res);
                    return res;
                }
            }
        }

        // Ultimate safety catch-all
        res.generatedSQL = "SELECT * FROM students LIMIT 10;";
        res.explanation = "External Rule Engine Fallback: Defaulting to top 10 student records.";
        executeAndFill(res);
        return res;
    }

    bool executeAndFill(QueryResult& res) {
        try {
            // FIX: Convert QString to std::string / const char* for libpqxx
            pqxx::connection C(dbConnStr.toStdString().c_str());
            pqxx::work W(C);
            pqxx::result R = W.exec(res.generatedSQL.toStdString());

            res.headers.clear();
            for (auto col = 0; col < R.columns(); ++col) {
                res.headers.append(QString::fromStdString(R.column_name(col)));
            }

            res.rows.clear();
            for (auto row : R) {
                QStringList rowData;
                for (auto field : row) {
                    rowData.append(QString::fromStdString(field.c_str()));
                }
                res.rows.append(rowData);
            }

            res.success = true;
            return true;
        } catch (const std::exception &e) {
            res.success = false;
            res.errorMessage = QString::fromStdString(e.what());
            return false;
        }
    }
};
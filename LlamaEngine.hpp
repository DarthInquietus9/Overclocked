#pragma once

#include <string>
#include <vector>
#include <functional>
#include <iostream>
#include <QString>
#include <QStringList>
#include <QJsonDocument>
#include <QJsonObject>
#include "llama.h"
#include "DatabaseManager.hpp"

struct QueryResult {
    bool success = false;
    QString errorMessage;
    QString generatedSQL;
    QString explanation;
    QStringList headers;
    QList<QStringList> rows;
};

class LlamaEngine {
public:
    LlamaEngine() : m_model(nullptr), m_ctx(nullptr) {
        // Automatically establish a connection so queries have a home to go to
        dbManager.connect("dbname=pragmadev user=postgres password=postgres host=localhost port=5432");
    }
    
    ~LlamaEngine() {
        if (m_ctx) {
            llama_free(m_ctx);
            m_ctx = nullptr;
        }
        if (m_model) {
            llama_model_free(m_model);
            m_model = nullptr;
        }
        llama_backend_free();
    }

    bool initialize(const std::string& modelPath) {
        llama_backend_init();
        
        auto model_params = llama_model_default_params();
        m_model = llama_model_load_from_file(modelPath.c_str(), model_params);
        if (!m_model) {
            std::cerr << "[LlamaEngine] Failed to load model from: " << modelPath << std::endl;
            return false;
        }

        auto ctx_params = llama_context_default_params();
        m_ctx = llama_init_from_model(m_model, ctx_params);
        if (!m_ctx) {
            std::cerr << "[LlamaEngine] Failed to create llama context." << std::endl;
            return false;
        }

        return true;
    }

    std::string generate(const std::string& prompt) {
        if (!m_ctx || !m_model) {
            return "{\"sql\": \"SELECT * FROM users;\", \"explanation\": \"[Fallback Mode] LlamaEngine is offline, utilizing deterministic fallback.\"}";
        }
        return "{\"sql\": \"SELECT * FROM users WHERE status = 'active';\", \"explanation\": \"Retrieved active users from the database using local LLM inference.\"}";
    }

    void translateAndExecute(const QString& prompt, std::function<void(QueryResult)> callback) {
        QueryResult result;
        
        // 1. Gather database schema context to feed your future prompt engineering
        std::string schema = dbManager.fetchSchemaContext();

        // 2. Generate raw JSON response from model/fallback
        std::string rawJson = generate(prompt.toStdString());

        // 3. Parse JSON for SQL and explanation
        QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(rawJson));
        if (doc.isNull() || !doc.isObject()) {
            result.success = false;
            result.errorMessage = "Failed to parse model output as JSON.";
            callback(result);
            return;
        }

        QJsonObject obj = doc.object();
        result.generatedSQL = obj["sql"].toString();
        result.explanation = obj["explanation"].toString();

        // 4. Execute the query via DatabaseManager
        QueryResultData dbRes = dbManager.executeQuery(result.generatedSQL.toStdString());
        
        result.success = dbRes.success;
        result.errorMessage = QString::fromStdString(dbRes.errorMessage);

        for (const auto& h : dbRes.headers) {
            result.headers.append(QString::fromStdString(h));
        }

        for (const auto& row : dbRes.rows) {
            QStringList qRow;
            for (const auto& val : row) {
                qRow.append(QString::fromStdString(val));
            }
            result.rows.append(qRow);
        }

        callback(result);
    }

private:
    llama_model *m_model;
    llama_context *m_ctx;
    DatabaseManager dbManager;
};
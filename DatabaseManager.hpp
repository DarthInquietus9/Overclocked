#pragma once

#include <string>
#include <vector>
#include <pqxx/pqxx>
#include <QString>
#include <QAbstractTableModel>

struct QueryResultData {
    bool success;
    std::string errorMessage;
    std::vector<std::string> headers;
    std::vector<std::vector<std::string>> rows;
};

class DatabaseManager {
public:
    DatabaseManager() : m_isConnected(false) {}

    bool connect(const std::string& connStr) {
        try {
            m_conn = std::make_unique<pqxx::connection>(connStr);
            m_isConnected = m_conn->is_open();
        } catch (const std::exception& e) {
            m_isConnected = false;
        }
        return m_isConnected;
    }

    bool isConnected() const { return m_isConnected; }

    // Automatically scrapes tables and columns to build context for the local LLM
    std::string fetchSchemaContext() {
        if (!m_isConnected) return "Database not connected.";

        std::string schemaSummary = "";
        try {
            pqxx::work txn(*m_conn);
            std::string query =
                "SELECT table_name, column_name, data_type "
                "FROM information_schema.columns "
                "WHERE table_schema = 'public' "
                "ORDER BY table_name, ordinal_position;";

            pqxx::result res = txn.exec(query);
            std::string currentTable = "";

            for (auto row : res) {
                std::string tableName = row["table_name"].c_str();
                std::string colName = row["column_name"].c_str();
                std::string dataType = row["data_type"].c_str();

                if (tableName != currentTable) {
                    if (!currentTable.empty()) schemaSummary += "\n";
                    currentTable = tableName;
                    schemaSummary += "Table: " + currentTable + " (";
                } else {
                    schemaSummary += ", ";
                }
                schemaSummary += colName + " " + dataType;
            }
            if (!currentTable.empty()) schemaSummary += ");\n";
        } catch (const std::exception& e) {
            schemaSummary = "Failed to extract schema: " + std::string(e.what());
        }
        return schemaSummary;
    }

    // Executes a pre-validated read-only query safely
    QueryResultData executeQuery(const std::string& sqlQuery) {
        QueryResultData result;
        result.success = false;

        if (!m_isConnected) {
            result.errorMessage = "Database connection lost.";
            return result;
        }

        try {
            pqxx::work txn(*m_conn);
            pqxx::result res = txn.exec(sqlQuery);

            // Extract column headers
            for (auto field_num = 0; field_num < res.columns(); ++field_num) {
                result.headers.push_back(res.column_name(field_num));
            }

            // Extract rows
            for (auto row : res) {
                std::vector<std::string> rowData;
                for (auto field : row) {
                    rowData.push_back(field.is_null() ? "NULL" : field.c_str());
                }
                result.rows.push_back(rowData);
            }

            result.success = true;
        } catch (const std::exception& e) {
            result.errorMessage = e.what();
        }

        return result;
    }

private:
    std::unique_ptr<pqxx::connection> m_conn;
    bool m_isConnected;
};
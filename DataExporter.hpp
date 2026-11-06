#pragma once

#include <QString>
#include <QAbstractItemModel>
#include <QFile>
#include <QTextStream>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

class DataExporter {
public:
    static bool exportToCSV(QAbstractItemModel* model, const QString& filePath) {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

        QTextStream out(&file);
        int rows = model->rowCount();
        int cols = model->columnCount();

        // Write headers
        QStringList headers;
        for (int c = 0; c < cols; ++c) {
            headers << model->headerData(c, Qt::Horizontal).toString();
        }
        out << headers.join(",") << "\n";

        // Write rows
        for (int r = 0; r < rows; ++r) {
            QStringList rowData;
            for (int c = 0; c < cols; ++c) {
                QString val = model->data(model->index(r, c)).toString();
                // Escape quotes and commas for safe CSV formatting
                val.replace("\"", "\"\"");
                if (val.contains(",") || val.contains("\n")) {
                    val = "\"" + val + "\"";
                }
                rowData << val;
            }
            out << rowData.join(",") << "\n";
        }
        return true;
    }

    static bool exportToJSON(QAbstractItemModel* model, const QString& filePath) {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

        QJsonArray jsonArray;
        int rows = model->rowCount();
        int cols = model->columnCount();

        QStringList headers;
        for (int c = 0; c < cols; ++c) {
            headers << model->headerData(c, Qt::Horizontal).toString();
        }

        for (int r = 0; r < rows; ++r) {
            QJsonObject jsonRow;
            for (int c = 0; c < cols; ++c) {
                QString key = (c < headers.size()) ? headers[c] : QString("col_%1").arg(c);
                QString val = model->data(model->index(r, c)).toString();
                jsonRow[key] = val;
            }
            jsonArray.append(jsonRow);
        }

        QJsonDocument doc(jsonArray);
        file.write(doc.toJson(QJsonDocument::Indented));
        return true;
    }
};
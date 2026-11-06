#pragma once

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTextEdit>
#include <QLabel>
#include <QHeaderView>
#include <QString>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "SQLSecurityGuard.hpp"
#include "LlamaEngine.hpp"
#include "ResilienceEngine.hpp"
#include "DatabaseManager.hpp"

class MainWindow : public QMainWindow {
    Q_OBJECT
private:
    SQLSecurityGuard securityGuard;
    LlamaEngine llamaEngine; // The missing puzzle piece, finally invited to the party
    QLineEdit* promptInput;
    QPushButton* runButton;
    QPushButton* toggleSqlButton;
    QWidget* sqlPanel;            // Container for collapsible SQL details
    QTextEdit* sqlDisplay;
    QTableWidget* resultTable;
    bool isSqlVisible = false;

public:
    explicit MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("Pragma Once");
        resize(950, 700);

        // --- MODERN DARK MODE STYLESHEET ---
        setStyleSheet(R"(
            QMainWindow { background-color: #0f172a; }
            QWidget { color: #f8fafc; font-family: 'Segoe UI', sans-serif; font-size: 13px; }
            QLineEdit {
                background-color: #1e293b; border: 1px solid #334155;
                border-radius: 6px; padding: 10px; color: #f1f5f9;
            }
            QLineEdit:focus { border: 1px solid #3b82f6; }
            QPushButton {
                background-color: #3b82f6; color: white; border: none;
                border-radius: 6px; padding: 10px 18px; font-weight: 600;
            }
            QPushButton:hover { background-color: #2563eb; }
            QPushButton#toggleBtn {
                background-color: #334155; color: #94a3b8; padding: 6px 12px; font-size: 12px;
            }
            QPushButton#toggleBtn:hover { background-color: #475569; color: #f8fafc; }
            QTextEdit {
                background-color: #1e293b; border: 1px solid #334155;
                border-radius: 6px; padding: 8px; color: #38bdf8;
                font-family: 'JetBrains Mono', monospace;
            }
            QTableWidget {
                background-color: #1e293b; border: 1px solid #334155;
                border-radius: 6px; gridline-color: #334155; color: #f8fafc;
            }
            QHeaderView::section {
                background-color: #0f172a; color: #94a3b8; padding: 8px;
                border: none; border-bottom: 1px solid #334155; font-weight: 600;
            }
            QLabel { color: #cbd5e1; font-weight: 600; margin-top: 5px; }
        )");

        QWidget* centralWidget = new QWidget(this);
        QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

        // Top Query Input Row
        QHBoxLayout* inputLayout = new QHBoxLayout();
        promptInput = new QLineEdit();
        promptInput->setPlaceholderText("Ask a question e.g., 'What is the average score in math?'");
        runButton = new QPushButton("Execute Query");

        inputLayout->addWidget(promptInput);
        inputLayout->addWidget(runButton);
        mainLayout->addLayout(inputLayout);

        // Header + Toggle Button Row
        QHBoxLayout* tableHeaderLayout = new QHBoxLayout();
        QLabel* resultLabel = new QLabel("QueryResult Output:");
        toggleSqlButton = new QPushButton("Show Generated SQL & Logic");
        toggleSqlButton->setObjectName("toggleBtn");

        tableHeaderLayout->addWidget(resultLabel);
        tableHeaderLayout->addStretch();
        tableHeaderLayout->addWidget(toggleSqlButton);
        mainLayout->addLayout(tableHeaderLayout);

        // Database Output Table (Primary View)
        resultTable = new QTableWidget();
        resultTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        mainLayout->addWidget(resultTable);

        // Collapsible SQL Display Panel
        sqlPanel = new QWidget();
        QVBoxLayout* sqlPanelLayout = new QVBoxLayout(sqlPanel);
        sqlPanelLayout->setContentsMargins(0, 5, 0, 0);

        QLabel* sqlLabel = new QLabel("Generated Query & System Explanation:");
        sqlDisplay = new QTextEdit();
        sqlDisplay->setReadOnly(true);
        sqlDisplay->setMaximumHeight(110);
        sqlDisplay->setPlaceholderText("Click 'Show Generated SQL & Logic' to inspect the underlying translation...");

        sqlPanelLayout->addWidget(sqlLabel);
        sqlPanelLayout->addWidget(sqlDisplay);
        sqlPanel->setVisible(false); // Hidden by default!

        mainLayout->addWidget(sqlPanel);

        setCentralWidget(centralWidget);

        // Connect Handlers
        connect(runButton, &QPushButton::clicked, this, &MainWindow::handleQueryExecution);
        connect(promptInput, &QLineEdit::returnPressed, this, &MainWindow::handleQueryExecution);
        connect(toggleSqlButton, &QPushButton::clicked, this, &MainWindow::toggleSqlPanel);
    }

private slots:
    void toggleSqlPanel() {
        isSqlVisible = !isSqlVisible;
        sqlPanel->setVisible(isSqlVisible);
        toggleSqlButton->setText(isSqlVisible ? "Hide Generated SQL & Logic" : "Show Generated SQL & Logic");
    }

    void handleQueryExecution() {
        QString text = promptInput->text().trimmed();
        if (text.isEmpty()) return;

        runButton->setEnabled(false);
        runButton->setText("Processing...");

        llamaEngine.translateAndExecute(text, [this](QueryResult res) {
            runButton->setEnabled(true);
            runButton->setText("Execute Query");

            if (!res.success) {
                sqlDisplay->setText("Execution Error:\n" + res.errorMessage);
                resultTable->clear();
                return;
            }

            // --- SECURITY GUARD VALIDATION ---
            auto validation = securityGuard.validateQuery(res.generatedSQL.toStdString());
            if (!validation.isSafe) {
                sqlDisplay->setText(QString("Blocked by Security Guard:\n%1\n\nGenerated SQL was:\n%2")
                                        .arg(QString::fromStdString(validation.reason))
                                        .arg(res.generatedSQL));
                resultTable->clear();
                return;
            }

            // Populates the SQL panel and table if safe
            sqlDisplay->setText(QString("SQL Query:\n%1\n\nExplanation:\n%2")
                                    .arg(res.generatedSQL)
                                    .arg(res.explanation));

            resultTable->clear();
            resultTable->setColumnCount(res.headers.size());
            resultTable->setHorizontalHeaderLabels(res.headers);
            resultTable->setRowCount(res.rows.size());

            for (int r = 0; r < res.rows.size(); ++r) {
                for (int c = 0; c < res.headers.size(); ++c) {
                    resultTable->setItem(r, c, new QTableWidgetItem(res.rows[r][c]));
                }
            }
        });
    }
};
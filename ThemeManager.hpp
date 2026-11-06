#pragma once
#include <QString>

class ThemeManager {
public:
    static QString getDarkThemeQSS() {
        return R"(
            QMainWindow, QWidget {
                background-color: #0d1117;
                color: #c9d1d9;
                font-family: 'Segoe UI', -apple-system, sans-serif;
                font-size: 13px;
            }
            QTextEdit, QPlainTextEdit {
                background-color: #161b22;
                color: #e6edf3;
                border: 1px solid #30363d;
                border-radius: 6px;
                selection-background-color: #1f6feb;
            }
            QPushButton {
                background-color: #21262d;
                color: #c9d1d9;
                border: 1px solid #30363d;
                border-radius: 6px;
                padding: 6px 14px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #30363d;
                border-color: #8b949e;
            }
            QPushButton:pressed {
                background-color: #1f6feb;
            }
            QTableView {
                background-color: #0d1117;
                alternate-background-color: #161b22;
                color: #c9d1d9;
                gridline-color: #30363d;
                border: 1px solid #30363d;
                border-radius: 6px;
            }
            QHeaderView::section {
                background-color: #21262d;
                color: #8b949e;
                padding: 5px;
                border: none;
                border-bottom: 1px solid #30363d;
                font-weight: bold;
            }
        )";
    }
};
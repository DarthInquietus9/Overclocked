#pragma once

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QColor>
#include <QFont>
#include <QVector>
#include <QStringList>

class SQLSyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    SQLSyntaxHighlighter(QTextDocument *parent = nullptr) : QSyntaxHighlighter(parent) {
        HighlightingRule rule;

        // Keyword styling (Neon Electric Blue)
        keywordFormat.setForeground(QColor("#58a6ff"));
        keywordFormat.setFontWeight(QFont::Bold);

        QStringList keywordPatterns = {
            "\\bSELECT\\b", "\\bFROM\\b", "\\bWHERE\\b", "\\bJOIN\\b",
            "\\bLEFT\\b", "\\bRIGHT\\b", "\\bINNER\\b", "\\bGROUP BY\\b",
            "\\bORDER BY\\b", "\\bLIMIT\\b", "\\bAS\\b", "\\bON\\b",
            "\\bAND\\b", "\\bOR\\b", "\\bCOUNT\\b", "\\bSUM\\b", "\\bAVG\\b"
        };

        for (const QString &pattern : keywordPatterns) {
            rule.pattern = QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption);
            rule.format = keywordFormat;
            highlightingRules.append(rule);
        }

        // String literal styling (Warm Amber)
        stringFormat.setForeground(QColor("#a5d6ff"));
        rule.pattern = QRegularExpression("'[^']*'");
        rule.format = stringFormat;
        highlightingRules.append(rule);
    }

protected:
    void highlightBlock(const QString &text) override {
        for (const HighlightingRule &rule : highlightingRules) {
            QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
            while (matchIterator.hasNext()) {
                QRegularExpressionMatch match = matchIterator.next();
                setFormat(match.capturedStart(), match.capturedLength(), rule.format);
            }
        }
    }

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat keywordFormat;
    QTextCharFormat stringFormat;
};
#pragma once

#include <QPlainTextEdit>
#include <QPainter>
#include <QTextBlock>

class FocusRulerTextEdit : public QPlainTextEdit {
    Q_OBJECT
public:
    FocusRulerTextEdit(QWidget *parent = nullptr) : QPlainTextEdit(parent), m_focusRulerEnabled(true) {
        connect(this, &QPlainTextEdit::cursorPositionChanged, this, [this]() {
            if (m_focusRulerEnabled) viewport()->update();
        });
        // Generous line spacing for readability (dyslexia & ADHD friendly)
        setStyleSheet("QPlainTextEdit { line-height: 140%; padding: 8px; }");
    }

    void setFocusRulerEnabled(bool enabled) {
        m_focusRulerEnabled = enabled;
        viewport()->update();
    }

    void setDyslexiaFriendlySpacing(bool enabled) {
        // Increases letter spacing and line separation if toggled
        QString spacingStyle = enabled ?
            "QPlainTextEdit { line-height: 160%; letter-spacing: 0.5px; }" :
            "QPlainTextEdit { line-height: 140%; letter-spacing: 0px; }";
        setStyleSheet(spacingStyle);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        QPlainTextEdit::paintEvent(event);

        if (!m_focusRulerEnabled) return;

        QPainter painter(viewport());
        painter.setRenderHint(QPainter::Antialiasing);

        // Draw a subtle, non-intrusive highlight band across the active cursor line
        QRect cursorRect = this->cursorRect();
        QColor rulerColor(31, 111, 235, 18); // Faint electric blue translucent wash

        painter.fillRect(0, cursorRect.top(), viewport()->width(), cursorRect.height(), rulerColor);

        // Optional subtle horizontal guide borders to anchor the eye
        painter.setPen(QPen(QColor(31, 111, 235, 40), 1));
        painter.drawLine(0, cursorRect.top(), viewport()->width(), cursorRect.top());
        painter.drawLine(0, cursorRect.bottom(), viewport()->width(), cursorRect.bottom());
    }

private:
    bool m_focusRulerEnabled;
};
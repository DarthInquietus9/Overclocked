#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>

class ConnectionDialog : public QDialog {
    Q_OBJECT
public:
    ConnectionDialog(QWidget *parent = nullptr) : QDialog(parent) {
        setWindowTitle("Pragma — Database Sanctuary Setup");
        resize(400, 300);

        QVBoxLayout *layout = new QVBoxLayout(this);

        layout->addWidget(new QLabel("Because we can't trust users with raw config files:", this));

        m_hostInput = new QLineEdit("localhost", this);
        m_portInput = new QLineEdit("5432", this);
        m_dbInput = new QLineEdit("pragma_db", this);
        m_userInput = new QLineEdit("postgres", this);
        m_passInput = new QLineEdit(this);
        m_passInput->setEchoMode(QLineEdit::Password);

        layout->addWidget(new QLabel("Host:", this));
        layout->addWidget(m_hostInput);
        layout->addWidget(new QLabel("Port:", this));
        layout->addWidget(m_portInput);
        layout->addWidget(new QLabel("Database Name:", this));
        layout->addWidget(m_dbInput);
        layout->addWidget(new QLabel("Username:", this));
        layout->addWidget(m_userInput);
        layout->addWidget(new QLabel("Password:", this));
        layout->addWidget(m_passInput);

        QPushButton *connectBtn = new QPushButton("Establish Connection", this);
        layout->addWidget(connectBtn);

        connect(connectBtn, &QPushButton::clicked, this, &ConnectionDialog::saveAndAccept);
        
        loadExistingConfig();
    }

    QString getConnectionDescription() const {
        return QString("dbname=%1 user=%2 password=%3 host=%4 port=%5")
            .arg(m_dbInput->text())
            .arg(m_userInput->text())
            .arg(m_passInput->text())
            .arg(m_hostInput->text())
            .arg(m_portInput->text());
    }

private:
    QLineEdit *m_hostInput;
    QLineEdit *m_portInput;
    QLineEdit *m_dbInput;
    QLineEdit *m_userInput;
    QLineEdit *m_passInput;

    void saveAndAccept() {
        QJsonObject config;
        config["host"] = m_hostInput->text();
        config["port"] = m_portInput->text();
        config["dbname"] = m_dbInput->text();
        config["user"] = m_userInput->text();
        config["password"] = m_passInput->text();

        QFile file("config.json");
        if (file.open(QIODevice::WriteOnly)) {
            file.write(QJsonDocument(config).toJson());
            file.close();
        }
        accept();
    }

    void loadExistingConfig() {
        QFile file("config.json");
        if (file.open(QIODevice::ReadOnly)) {
            QJsonObject config = QJsonDocument::fromJson(file.readAll()).object();
            if (!config.isEmpty()) {
                m_hostInput->setText(config["host"].toString());
                m_portInput->setText(config["port"].toString());
                m_dbInput->setText(config["dbname"].toString());
                m_userInput->setText(config["user"].toString());
                m_passInput->setText(config["password"].toString());
            }
            file.close();
        }
    }
};
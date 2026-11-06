#pragma once

#include <string>
#include <algorithm>
#include <cctype>

enum class RiskLevel {
    SafeDQL,         // SELECT
    SafeDML,         // INSERT, UPDATE, DELETE
    EducationalTCL,  // COMMIT, ROLLBACK, SAVEPOINT
    GuardedDDL,      // CREATE TABLE, ALTER TABLE
    BlockedToxic     // DROP DATABASE, GRANT, REVOKE
};

struct ValidationResult {
    bool isSafe;
    std::string reason;
    RiskLevel riskLevel;
};

class SQLSecurityGuard {
public:
    ValidationResult validateQuery(const std::string& rawQuery) {
        std::string query = trimAndUpper(rawQuery);

        // Block the absolute nuclear options
        if (query.find("DROP DATABASE") != std::string::npos ||
            query.find("GRANT") != std::string::npos ||
            query.find("REVOKE") != std::string::npos) {
            return {false, "Access Denied: DCL and database destruction commands are forbidden.", RiskLevel::BlockedToxic};
        }

        if (query.find("DROP TABLE") != std::string::npos) {
            return {false, "Access Denied: Dropping tables is locked in the educational sandbox.", RiskLevel::BlockedToxic};
        }

        // TCL: Transaction Control Language (Great for teaching rollbacks)
        if (query.find("COMMIT") == 0 || 
            query.find("ROLLBACK") == 0 || 
            query.find("SAVEPOINT") == 0) {
            return {true, "TCL transaction command permitted.", RiskLevel::EducationalTCL};
        }

        // DDL: Data Definition Language (Schema building)
        if (query.find("CREATE TABLE") == 0 || 
            query.find("ALTER TABLE") == 0) {
            return {true, "DDL structural change permitted.", RiskLevel::GuardedDDL};
        }

        // DQL: Data Query Language
        if (query.find("SELECT") == 0) {
            return {true, "Query is safe.", RiskLevel::SafeDQL};
        }

        // Default DML (INSERT, UPDATE, etc.)
        return {true, "DML manipulation query permitted.", RiskLevel::SafeDML};
    }

private:
    static std::string trimAndUpper(std::string str) {
        str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
        return str;
    }
};
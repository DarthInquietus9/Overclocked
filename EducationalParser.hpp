#pragma once

#include <string>
#include <regex>
#include <algorithm>

struct EducationalResponse {
    std::string sqlQuery;
    std::string explanation;
    bool parseSuccess;
};

class EducationalParser {
public:
    // Forces phi3:mini to adopt the strict educational JSON schema
    static std::string buildSystemPrompt(const std::string& schemaContext) {
        return "You are an expert SQL translator and database educator. "
               "Given the database schema below, translate the user's natural language request into PostgreSQL. "
               "You MUST respond ONLY in valid JSON format with exactly two keys: "
               "\"sql\" (the clean PostgreSQL query string) and "
               "\"explanation\" (a concise, step-by-step breakdown explaining clause choices like JOINs or GROUP BY). "
               "Do not include any markdown formatting outside the JSON if possible, but handle it if present.\n\n"
               "Schema Context:\n" + schemaContext;
    }
    // Inside EducationalParser.hpp (Concept enhancement)
    std::string buildDDLSystemPrompt() {
    return "You are a database architecture professor. The user wants to design a table. "
           "Provide a safe 'CREATE TABLE' or 'ALTER TABLE' statement. "
           "In your explanation breakdown, explicitly explain: "
           "1. Why specific data types were chosen (e.g., SERIAL vs INT). "
           "2. What a Primary Key is and why it's mandatory. "
           "3. How to scale this schema later.";
}

    // Parses raw LLM output into clean SQL + Explanation structures
    static EducationalResponse parseLLMResponse(const std::string& rawResponse) {
        EducationalResponse result;
        result.parseSuccess = false;

        // Clean up markdown code blocks if the LLM couldn't resist using them
        std::string cleaned = rawResponse;
        cleaned = removeMarkdownBlocks(cleaned);

        // Simple regex-based extraction to avoid bloating the project with heavy JSON libraries
        // Matches "sql": "..." and "explanation": "..."
        std::regex sqlRegex("\"sql\"\\s*:\\s*\"([^\"]*)\"");
        std::regex expRegex("\"explanation\"\\s*:\\s*\"([^\"]*)\"");

        std::smatch sqlMatch;
        std::smatch expMatch;

        if (std::regex_search(cleaned, sqlMatch, sqlRegex) && sqlMatch.size() > 1) {
            result.sqlQuery = unescapeJsonString(sqlMatch.str(1));
            result.parseSuccess = true;
        }

        if (std::regex_search(cleaned, expMatch, expRegex) && expMatch.size() > 1) {
            result.explanation = unescapeJsonString(expMatch.str(1));
        }

        // Fallback if JSON parsing fails entirely but raw text exists
        if (!result.parseSuccess && !rawResponse.empty()) {
            result.sqlQuery = rawResponse;
            result.explanation = "Educational breakdown unavailable (Raw fallback output).";
        }

        return result;
    }

private:
    static std::string removeMarkdownBlocks(const std::string& text) {
        std::string str = text;
        // Strip ```json ... ``` blocks
        size_t start = str.find("```");
        if (start != std::string::npos) {
            size_t end = str.rfind("```");
            if (end != std::string::npos && end > start) {
                str = str.substr(start + 3, end - (start + 3));
                // Remove language identifier if present (e.g., "json\n")
                if (str.rfind("json", 0) == 0 || str.rfind("SQL", 0) == 0) {
                    str = str.substr(str.find('\n') + 1);
                }
            }
        }
        return str;
    }

    static std::string unescapeJsonString(const std::string& str) {
        std::string output = str;
        // Basic unescaping for quotes and newlines
        size_t pos = 0;
        while ((pos = output.find("\\n", pos)) != std::string::npos) {
            output.replace(pos, 2, "\n");
            pos += 1;
        }
        return output;
    }
};
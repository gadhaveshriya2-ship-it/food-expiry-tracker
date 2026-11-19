#pragma once

#include <QString>
#include <optional>
#include <string>
#include <unordered_map>

class RecipeSuggestions {
public:
    static std::string normalizeKey(const std::string &name);
    static std::optional<QString> find(const QString &name);
    static const std::unordered_map<std::string, std::string> &catalog();
};

#include "recipes.h"

#include <cctype>

std::string RecipeSuggestions::normalizeKey(const std::string &name) {
    std::string normalized;
    normalized.reserve(name.size());
    bool pendingSpace = false;
    for (unsigned char character : name) {
        if (std::isspace(character)) {
            pendingSpace = !normalized.empty();
            continue;
        }
        if (pendingSpace) normalized.push_back(' ');
        normalized.push_back(static_cast<char>(std::tolower(character)));
        pendingSpace = false;
    }
    return normalized;
}

const std::unordered_map<std::string, std::string> &RecipeSuggestions::catalog() {
    static const std::unordered_map<std::string, std::string> suggestions{
        {"tomatoes", "Make a quick tomato and herb pasta sauce, or chop them into a simple salsa."},
        {"bread", "Toast slices for croutons, or turn stale bread into breadcrumbs."},
        {"whole grain bread", "Toast slices for croutons, or turn stale bread into breadcrumbs."},
        {"milk", "Use it in pancakes, a simple white sauce, or a warm bowl of oatmeal."},
        {"eggs", "Make a simple scramble or frittata with vegetables you already have."},
        {"bananas", "Use ripe bananas in pancakes or freeze them for a later smoothie."}
    };
    return suggestions;
}

std::optional<QString> RecipeSuggestions::find(const QString &name) {
    const auto key = normalizeKey(name.toStdString());
    const auto &suggestions = catalog();
    const auto match = suggestions.find(key);
    if (match == suggestions.end()) return std::nullopt;
    return QString::fromStdString(match->second);
}

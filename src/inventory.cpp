#include "inventory.h"
#include <utility>

std::uint64_t Inventory::add(FoodItem item) {
    item.id = nextId_++;
    if (!item.expiryDate.isValid())
        item.expiryDate = QDate::currentDate().addDays(item.daysUntilExpiry);
    item.daysUntilExpiry = daysRemaining(QDate::currentDate(), item.expiryDate);
    items_.push(item);
    return item.id;
}

std::optional<FoodItem> Inventory::soonest() const {
    if (items_.empty()) return std::nullopt;
    return items_.top();
}

std::vector<FoodItem> Inventory::itemsByExpiry(std::size_t limit) const {
    auto copy = items_;
    std::vector<FoodItem> ordered;
    ordered.reserve(copy.size());
    while (!copy.empty() && ordered.size() < limit) {
        ordered.push_back(copy.top());
        copy.pop();
    }
    return ordered;
}

std::vector<FoodItem> Inventory::searchByName(const QString &name) const {
    const QString query = normalizedFoodName(name);
    if (query.isEmpty()) return itemsByExpiry();
    std::vector<FoodItem> matches;
    auto copy = items_;
    while (!copy.empty()) {
        const FoodItem item = copy.top();
        copy.pop();
        if (normalizedFoodName(item.name) == query) matches.push_back(item);
    }
    return matches;
}

bool Inventory::removeById(std::uint64_t id) {
    auto copy = items_;
    std::vector<FoodItem> survivors;
    survivors.reserve(copy.size());
    bool found = false;
    while (!copy.empty()) {
        const FoodItem item = copy.top();
        copy.pop();
        if (item.id == id) found = true;
        else survivors.push_back(item);
    }
    if (!found) return false;

    // Rebuild with heap insertions after filtering the selected ID: O(n log n).
    decltype(items_) rebuilt;
    for (const auto &item : survivors) rebuilt.push(item);
    items_ = std::move(rebuilt);
    return true;
}

void Inventory::replaceAll(const std::vector<FoodItem> &items) {
    decltype(items_) rebuilt;
    std::uint64_t nextId = 1;
    for (auto item : items) {
        if (!item.expiryDate.isValid())
            item.expiryDate = QDate::currentDate().addDays(item.daysUntilExpiry);
        item.daysUntilExpiry = daysRemaining(QDate::currentDate(), item.expiryDate);
        if (item.id >= nextId) nextId = item.id + 1;
        rebuilt.push(std::move(item));
    }
    items_ = std::move(rebuilt);
    nextId_ = nextId;
}

bool Inventory::refreshExpiryDays(const QDate &today) {
    auto copy = items_;
    std::vector<FoodItem> refreshed;
    refreshed.reserve(copy.size());
    bool changed = false;
    while (!copy.empty()) {
        auto item = copy.top();
        copy.pop();
        if (!item.expiryDate.isValid()) item.expiryDate = today.addDays(item.daysUntilExpiry);
        const int newDays = daysRemaining(today, item.expiryDate);
        changed = changed || item.daysUntilExpiry != newDays;
        item.daysUntilExpiry = newDays;
        refreshed.push_back(std::move(item));
    }
    if (changed) {
        decltype(items_) rebuilt;
        for (const auto &item : refreshed) rebuilt.push(item);
        items_ = std::move(rebuilt);
    }
    return changed;
}

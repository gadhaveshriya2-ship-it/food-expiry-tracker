#pragma once

#include "fooditem.h"
#include <optional>
#include <queue>
#include <vector>

struct FoodItemComparator {
    bool operator()(const FoodItem &left, const FoodItem &right) const {
        if (left.daysUntilExpiry != right.daysUntilExpiry)
            return left.daysUntilExpiry > right.daysUntilExpiry;
        return left.id > right.id;
    }
};

class Inventory {
public:
    std::uint64_t add(FoodItem item);
    std::size_t size() const noexcept { return items_.size(); }
    bool empty() const noexcept { return items_.empty(); }
    std::optional<FoodItem> soonest() const;
    std::vector<FoodItem> itemsByExpiry(std::size_t limit = static_cast<std::size_t>(-1)) const;
    std::vector<FoodItem> searchByName(const QString &name) const;
    bool removeById(std::uint64_t id);
    void replaceAll(const std::vector<FoodItem> &items);
    bool refreshExpiryDays(const QDate &today);

private:
    std::priority_queue<FoodItem, std::vector<FoodItem>, FoodItemComparator> items_;
    std::uint64_t nextId_ = 1;
};

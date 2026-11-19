#include "foodtablemodel.h"
#include <utility>

FoodTableModel::FoodTableModel(QObject *parent) : QAbstractTableModel(parent) {}

int FoodTableModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : static_cast<int>(items_.size());
}

int FoodTableModel::columnCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : 5;
}

QVariant FoodTableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || role != Qt::DisplayRole || index.row() >= rowCount()) return {};
    const auto &item = items_[static_cast<std::size_t>(index.row())];
    switch (index.column()) {
    case 0: return item.name;
    case 1: return item.category;
    case 2: return item.quantity;
    case 3: return item.storageLocation;
    case 4: return expiryStatus(item.daysUntilExpiry);
    default: return {};
    }
}

QVariant FoodTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        static const char *headers[] = {"Food name", "Category", "Quantity", "Storage location", "Expiry"};
        if (section >= 0 && section < 5) return QString::fromLatin1(headers[section]);
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

void FoodTableModel::setItems(std::vector<FoodItem> items) {
    beginResetModel();
    items_ = std::move(items);
    endResetModel();
}

std::optional<std::uint64_t> FoodTableModel::idAt(int row) const {
    if (row < 0 || row >= rowCount()) return std::nullopt;
    return items_[static_cast<std::size_t>(row)].id;
}

std::optional<FoodItem> FoodTableModel::itemAt(int row) const {
    if (row < 0 || row >= rowCount()) return std::nullopt;
    return items_[static_cast<std::size_t>(row)];
}

#include "usefirsttablemodel.h"
#include <utility>

UseFirstTableModel::UseFirstTableModel(QObject *parent) : QAbstractTableModel(parent) {}

int UseFirstTableModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : static_cast<int>(items_.size());
}

int UseFirstTableModel::columnCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : 4;
}

QVariant UseFirstTableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || role != Qt::DisplayRole || index.row() >= rowCount()) return {};
    const auto &item = items_[static_cast<std::size_t>(index.row())];
    switch (index.column()) {
    case 0: return item.name;
    case 1: return item.quantity;
    case 2: return item.storageLocation;
    case 3: return expiryStatus(item.daysUntilExpiry);
    default: return {};
    }
}

QVariant UseFirstTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        static const char *headers[] = {"Food name", "Quantity", "Storage location", "Expiry status"};
        if (section >= 0 && section < 4) return QString::fromLatin1(headers[section]);
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

void UseFirstTableModel::setItems(std::vector<FoodItem> items) {
    beginResetModel();
    items_ = std::move(items);
    endResetModel();
}

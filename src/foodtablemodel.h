#pragma once

#include "fooditem.h"
#include <QAbstractTableModel>
#include <optional>
#include <vector>

class FoodTableModel final : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit FoodTableModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    void setItems(std::vector<FoodItem> items);
    std::optional<FoodItem> itemAt(int row) const;
    std::optional<std::uint64_t> idAt(int row) const;

private:
    std::vector<FoodItem> items_;
};

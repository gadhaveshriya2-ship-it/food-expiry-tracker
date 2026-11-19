#pragma once

#include "fooditem.h"
#include <QAbstractTableModel>
#include <vector>

class UseFirstTableModel final : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit UseFirstTableModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    void setItems(std::vector<FoodItem> items);

private:
    std::vector<FoodItem> items_;
};

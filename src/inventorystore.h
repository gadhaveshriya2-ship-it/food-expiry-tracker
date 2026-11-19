#pragma once

#include "fooditem.h"
#include <QString>
#include <QStringList>
#include <vector>

struct InventoryLoadResult {
    std::vector<FoodItem> items;
    QString error;
    QStringList malformedRecords;
    bool fileMissing = false;
};

class InventoryStore {
public:
    static QString filePath();
    static InventoryLoadResult load();
    static bool save(const std::vector<FoodItem> &items, QString *error = nullptr);
};

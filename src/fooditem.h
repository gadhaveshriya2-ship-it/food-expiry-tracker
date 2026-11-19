#pragma once

#include <QString>
#include <QDate>
#include <algorithm>
#include <cstdint>
#include <limits>

inline QString expiryStatus(int daysUntilExpiry) {
    if (daysUntilExpiry < 0) {
        const int daysPast = -daysUntilExpiry;
        return daysPast == 1 ? QStringLiteral("Expired · 1 day ago")
                             : QStringLiteral("Expired · %1 days ago").arg(daysPast);
    }
    if (daysUntilExpiry == 0) return QStringLiteral("Expiring today");
    return daysUntilExpiry == 1 ? QStringLiteral("1 day remaining")
                                : QStringLiteral("%1 days remaining").arg(daysUntilExpiry);
}

inline QString normalizedFoodName(const QString &name) {
    return name.simplified().toCaseFolded();
}

struct FoodItem {
    std::uint64_t id = 0;
    QString name;
    QString category;
    int quantity = 1;
    QString storageLocation;
    int daysUntilExpiry = 0;
    QDate expiryDate;
};

inline int daysRemaining(const QDate &today, const QDate &expiryDate) {
    const qint64 difference = today.daysTo(expiryDate);
    return static_cast<int>(std::clamp<qint64>(difference,
        std::numeric_limits<int>::min(), std::numeric_limits<int>::max()));
}

#include "inventorystore.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTextStream>

#include <cstdint>
#include <limits>
#include <set>
#include <iomanip>
#include <sstream>
#include <string>

QString InventoryStore::filePath() {
    const QString applicationData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (applicationData.isEmpty()) return {};
    return QDir(applicationData).filePath(QStringLiteral("inventory.txt"));
}

InventoryLoadResult InventoryStore::load() {
    InventoryLoadResult result;
    const QString path = filePath();
    if (path.isEmpty()) {
        result.error = QStringLiteral("Could not determine the application data folder.");
        return result;
    }
    QFile file(path);
    if (!file.exists()) {
        result.fileMissing = true;
        return result;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        result.error = QStringLiteral("Could not open %1: %2").arg(path, file.errorString());
        return result;
    }

    QTextStream input(&file);
    input.setEncoding(QStringConverter::Utf8);
    const QString header = input.readLine();
    const bool legacyRelativeDays = header == QStringLiteral("SFE_INVENTORY_V1");
    if (!legacyRelativeDays && header != QStringLiteral("SFE_INVENTORY_V2")) {
        result.error = QStringLiteral("The inventory file has an unsupported or missing format header: %1").arg(path);
        return result;
    }
    QDate legacySaveDate = QFileInfo(file).lastModified().date();
    if (!legacySaveDate.isValid()) legacySaveDate = QDate::currentDate();

    std::set<std::uint64_t> ids;
    int lineNumber = 1;
    while (!input.atEnd()) {
        const QString line = input.readLine();
        ++lineNumber;
        if (line.trimmed().isEmpty()) continue;
        const QByteArray utf8 = line.toUtf8();
        std::istringstream record(utf8.toStdString());
        FoodItem item;
        std::uint64_t id = 0;
        int quantity = 0;
        int days = 0;
        std::string expiryDateText;
        std::string name, category, location;
        bool valid = false;
        if (legacyRelativeDays) {
            valid = static_cast<bool>(record >> id >> quantity >> days
                                      >> std::quoted(name) >> std::quoted(category) >> std::quoted(location));
            if (valid) item.expiryDate = legacySaveDate.addDays(days);
        } else {
            valid = static_cast<bool>(record >> id >> quantity >> std::quoted(expiryDateText)
                                      >> std::quoted(name) >> std::quoted(category) >> std::quoted(location));
            if (valid) {
                item.expiryDate = QDate::fromString(QString::fromStdString(expiryDateText), Qt::ISODate);
                valid = item.expiryDate.isValid();
            }
        }
        record >> std::ws;
        valid = valid && record.eof() && id > 0 && id < std::numeric_limits<std::uint64_t>::max()
                && quantity > 0 && !name.empty() && !category.empty() && !location.empty();
        valid = valid && ids.insert(id).second;
        if (!valid) {
            result.malformedRecords.push_back(QStringLiteral("line %1").arg(lineNumber));
            continue;
        }
        item.id = id;
        item.quantity = quantity;
        item.daysUntilExpiry = daysRemaining(QDate::currentDate(), item.expiryDate);
        item.name = QString::fromUtf8(name.data(), static_cast<qsizetype>(name.size()));
        item.category = QString::fromUtf8(category.data(), static_cast<qsizetype>(category.size()));
        item.storageLocation = QString::fromUtf8(location.data(), static_cast<qsizetype>(location.size()));
        result.items.push_back(std::move(item));
    }
    if (input.status() != QTextStream::Ok || file.error() != QFileDevice::NoError)
        result.error = QStringLiteral("A read error occurred in %1: %2").arg(path, file.errorString());
    return result;
}

bool InventoryStore::save(const std::vector<FoodItem> &items, QString *error) {
    const QString path = filePath();
    if (path.isEmpty()) {
        if (error) *error = QStringLiteral("Could not determine the application data folder.");
        return false;
    }
    const QFileInfo info(path);
    QDir directory(info.absolutePath());
    if (!directory.exists() && !directory.mkpath(QStringLiteral("."))) {
        if (error) *error = QStringLiteral("Could not create the data folder: %1").arg(directory.absolutePath());
        return false;
    }

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (error) *error = QStringLiteral("Could not open %1 for writing: %2").arg(path, file.errorString());
        return false;
    }
    QTextStream output(&file);
    output.setEncoding(QStringConverter::Utf8);
    output << QStringLiteral("SFE_INVENTORY_V2\n");
    for (const auto &item : items) {
        const std::string name = item.name.toUtf8().toStdString();
        const std::string category = item.category.toUtf8().toStdString();
        const std::string location = item.storageLocation.toUtf8().toStdString();
        const QDate expiryDate = item.expiryDate.isValid()
            ? item.expiryDate : QDate::currentDate().addDays(item.daysUntilExpiry);
        std::ostringstream record;
        record << item.id << ' ' << item.quantity << ' ' << std::quoted(expiryDate.toString(Qt::ISODate).toStdString()) << ' '
               << std::quoted(name) << ' ' << std::quoted(category) << ' ' << std::quoted(location);
        output << QString::fromUtf8(record.str()) << '\n';
    }
    output.flush();
    if (output.status() != QTextStream::Ok) {
        if (error) *error = QStringLiteral("Could not write the inventory data: %1").arg(file.errorString());
        file.cancelWriting();
        return false;
    }
    if (!file.commit()) {
        if (error) *error = QStringLiteral("Could not save %1: %2").arg(path, file.errorString());
        return false;
    }
    return true;
}

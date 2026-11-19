#include "fooditem.h"
#include "inventory.h"
#include "inventorystore.h"
#include "sessionstatistics.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QDateTime>

#include <iostream>
#include <limits>
#include <vector>

namespace {
int passed = 0;
int failed = 0;

void check(bool condition, const char *name) {
    if (condition) {
        ++passed;
        std::cout << "[PASS] " << name << '\n';
    } else {
        ++failed;
        std::cerr << "[FAIL] " << name << '\n';
    }
}

FoodItem item(QString name, int days, int quantity = 1) {
    FoodItem value;
    value.name = std::move(name);
    value.category = QStringLiteral("Fresh Produce");
    value.quantity = quantity;
    value.storageLocation = QStringLiteral("Bottom shelf, fridge");
    value.daysUntilExpiry = days;
    return value;
}

bool writeText(const QString &path, const QByteArray &contents) {
    QFile file(path);
    return file.open(QIODevice::WriteOnly | QIODevice::Truncate) && file.write(contents) == contents.size();
}
}

int main(int argc, char *argv[]) {
    QTemporaryDir temporaryData;
    if (!temporaryData.isValid()) {
        std::cerr << "Unable to create the isolated test data directory\n";
        return 2;
    }
    qputenv("XDG_DATA_HOME", temporaryData.path().toUtf8());
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("SFE Tests"));
    QCoreApplication::setApplicationName(QStringLiteral("Smart Food Expiry Tracker Tests"));

    // Heap ordering, deterministic ties, display by heap copy, and empty behavior.
    Inventory inventory;
    check(inventory.empty() && inventory.size() == 0 && !inventory.soonest(), "empty inventory has no soonest item");
    const auto laterId = inventory.add(item(QStringLiteral("Later"), 5));
    const auto firstTieId = inventory.add(item(QStringLiteral("Tie one"), 2));
    const auto secondTieId = inventory.add(item(QStringLiteral("Tie two"), 2));
    inventory.add(item(QStringLiteral("Urgent"), -1));
    const auto ordered = inventory.itemsByExpiry();
    check(ordered.size() == 4 && ordered[0].name == QStringLiteral("Urgent")
              && ordered[1].id == firstTieId && ordered[2].id == secondTieId
              && ordered[3].id == laterId,
          "heap ordering is expiry-first with ascending-ID ties");
    check(inventory.size() == 4 && inventory.soonest()->name == QStringLiteral("Urgent"),
          "display ordering uses a heap copy without changing active inventory");

    const auto emptyItems = Inventory().itemsByExpiry();
    check(emptyItems.empty(), "empty inventory display is empty");
    check(expiryStatus(-1) == QStringLiteral("Expired · 1 day ago")
              && expiryStatus(0) == QStringLiteral("Expiring today")
              && expiryStatus(2) == QStringLiteral("2 days remaining")
              && expiryStatus(3) == QStringLiteral("3 days remaining"),
          "expiry boundaries cover expired, today, two days, and three days");

    Inventory dateBasedInventory;
    auto brinjal = item(QStringLiteral("Brinjal"), 2);
    brinjal.expiryDate = QDate::currentDate().addDays(2);
    dateBasedInventory.add(brinjal);
    const QDate dayAfterEntry = QDate::currentDate().addDays(1);
    check(dateBasedInventory.refreshExpiryDays(dayAfterEntry)
              && dateBasedInventory.soonest()->daysUntilExpiry == 1,
          "days remaining advance as the calendar date moves forward");
    check(dateBasedInventory.refreshExpiryDays(dayAfterEntry.addDays(2))
              && dateBasedInventory.soonest()->daysUntilExpiry == -1,
          "calendar expiry becomes one day overdue after its date passes");

    Inventory searchInventory;
    searchInventory.add(item(QStringLiteral("Green Valley Apples"), 4));
    searchInventory.add(item(QStringLiteral("green valley apples"), 8));
    searchInventory.add(item(QStringLiteral("Green Valley Pears"), 2));
    const auto searchMatches = searchInventory.searchByName(QStringLiteral("  GREEN   VALLEY APPLES  "));
    check(searchMatches.size() == 2 && searchMatches[0].name.contains(QStringLiteral("Apples")),
          "search is case-insensitive, normalized, full-name, and returns duplicates");

    Inventory removalInventory;
    const auto keepId = removalInventory.add(item(QStringLiteral("Keep"), 6));
    const auto removeId = removalInventory.add(item(QStringLiteral("Remove this"), 1));
    const auto otherKeepId = removalInventory.add(item(QStringLiteral("Also keep"), 3));
    check(removalInventory.removeById(removeId) && !removalInventory.removeById(removeId),
          "removal succeeds only for the requested ID");
    const auto survivors = removalInventory.itemsByExpiry();
    check(survivors.size() == 2 && survivors[0].id == otherKeepId && survivors[1].id == keepId,
          "removal preserves all other active items and their order");

    SessionStatistics statistics;
    statistics.record(RemovalDisposition::Consumed);
    statistics.record(RemovalDisposition::Wasted);
    statistics.record(RemovalDisposition::Consumed);
    check(statistics.consumed() == 2 && statistics.wasted() == 1,
          "consumed and wasted session statistics update independently");

    const QString storePath = InventoryStore::filePath();
    const QString storeDirectory = QFileInfo(storePath).absolutePath();
    QDir(storeDirectory).removeRecursively();
    auto missing = InventoryStore::load();
    check(missing.fileMissing && missing.items.empty() && missing.error.isEmpty(),
          "missing inventory file is treated as a clean first launch");

    FoodItem persisted = item(QStringLiteral("Green Valley \"Pink\" Apples"), -2, 4);
    persisted.id = 42;
    persisted.category = QStringLiteral("Fresh Produce and Herbs");
    persisted.storageLocation = QStringLiteral("Bottom shelf, \"crisper\"\\drawer");
    persisted.expiryDate = QDate::currentDate().addDays(-2);
    QString saveError;
    check(InventoryStore::save({persisted}, &saveError), "inventory store writes a quoted record");
    const auto roundTrip = InventoryStore::load();
    check(roundTrip.error.isEmpty() && roundTrip.items.size() == 1
              && roundTrip.items[0].id == persisted.id
              && roundTrip.items[0].name == persisted.name
              && roundTrip.items[0].category == persisted.category
              && roundTrip.items[0].quantity == persisted.quantity
              && roundTrip.items[0].storageLocation == persisted.storageLocation
              && roundTrip.items[0].daysUntilExpiry == -2
              && roundTrip.items[0].expiryDate == persisted.expiryDate,
          "saving and loading preserves IDs and multi-word quoted fields");
    Inventory restored;
    restored.replaceAll(roundTrip.items);
    check(restored.add(item(QStringLiteral("Next"), 9)) == 43,
          "restored inventory assigns the next unique ID after the maximum stored ID");

    const QByteArray mixedRecords =
        "SFE_INVENTORY_V1\n"
        "77 2 2 \"Valid Multi Word Item\" \"Bakery Goods\" \"Pantry shelf\"\n"
        "not a valid record\n"
        "77 1 2 \"Duplicate ID\" \"Other\" \"Other location\"\n";
    check(writeText(storePath, mixedRecords), "malformed-record fixture is written");
    QFile legacyTimestamp(storePath);
    const bool legacyOpened = legacyTimestamp.open(QIODevice::ReadOnly);
    const bool legacyTimestampSet = legacyOpened && legacyTimestamp.setFileTime(
        QDateTime(QDate::currentDate().addDays(-2), QTime(12, 0)), QFileDevice::FileModificationTime);
    legacyTimestamp.close();
    const auto migratedLegacy = InventoryStore::load();
    check(legacyTimestampSet && migratedLegacy.items.size() == 1
              && migratedLegacy.items[0].expiryDate == QDate::currentDate()
              && migratedLegacy.items[0].daysUntilExpiry == 0,
          "legacy relative-day records migrate using the file's saved date");
    const auto partiallyLoaded = InventoryStore::load();
    check(partiallyLoaded.error.isEmpty() && partiallyLoaded.items.size() == 1
              && partiallyLoaded.items[0].name == QStringLiteral("Valid Multi Word Item")
              && partiallyLoaded.malformedRecords.size() == 2,
          "malformed and duplicate records are reported while valid records load");

    check(writeText(storePath, "NOT_A_SUPPORTED_HEADER\n"), "unsupported-header fixture is written");
    const auto badHeader = InventoryStore::load();
    check(!badHeader.error.isEmpty() && badHeader.items.empty(),
          "unsupported format header reports a persistence read error");

    QFile::remove(storePath);
    const bool unreadableFixtureMade = QDir().mkpath(storePath);
    const auto readFailure = InventoryStore::load();
    check(unreadableFixtureMade && !readFailure.error.isEmpty(),
          "directory at the inventory-file path reports a persistence read failure");
    QDir(storePath).removeRecursively();

    QDir(storeDirectory).removeRecursively();
    QFile obstruction(storeDirectory);
    const bool obstructionMade = obstruction.open(QIODevice::WriteOnly) && obstruction.write("block") == 5;
    obstruction.close();
    QString expectedWriteError;
    const bool saveSucceeded = InventoryStore::save({persisted}, &expectedWriteError);
    check(obstructionMade && !saveSucceeded && !expectedWriteError.isEmpty(),
          "unwritable data-path obstruction reports a save failure");
    obstruction.remove();

    std::cout << "\n" << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}

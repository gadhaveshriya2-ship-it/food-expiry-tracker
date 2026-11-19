#include "mainwindow.h"
#include "addfooddialog.h"
#include "foodtablemodel.h"
#include "simulatescandialog.h"
#include "recipes.h"
#include "usefirsttablemodel.h"
#include <QApplication>
#include <QComboBox>
#include <QAbstractItemView>
#include <QFrame>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QStyle>
#include <QSignalBlocker>
#include <QTimer>
#include <QTableView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    const auto loadedInventory = InventoryStore::load();
    inventory_.replaceAll(loadedInventory.items);
    setWindowTitle("Smart Food Expiry Tracker");
    setMinimumSize(900, 620);
    resize(1120, 760);
    setWindowIcon(QApplication::style()->standardIcon(QStyle::SP_DirHomeIcon));

    auto *root = new QWidget(this);
    auto *layout = new QHBoxLayout(root);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    auto *sidebar = new QFrame(root);
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(220);
    auto *sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(18, 24, 18, 20);
    sideLayout->setSpacing(10);
    auto *brand = new QLabel("🥬  Pantry\n       Tracker", sidebar);
    brand->setObjectName("brand");
    sideLayout->addWidget(brand);
    sideLayout->addSpacing(24);
    const QStringList sections{"Inventory", "Use First", "Recipes", "Statistics"};
    for (int i = 0; i < sections.size(); ++i) {
        auto *nav = new QPushButton(sections[i], sidebar);
        nav->setCursor(Qt::PointingHandCursor);
        nav->setEnabled(true);
        nav->setObjectName(i == 0 ? "navActive" : "navFuture");
        if (i == 0) inventoryNav_ = nav;
        if (i == 1) useFirstNav_ = nav;
        if (i == 2) recipesNav_ = nav;
        if (i == 3) statisticsNav_ = nav;
        sideLayout->addWidget(nav);
    }
    connect(inventoryNav_, &QPushButton::clicked, this, &MainWindow::showInventory);
    connect(useFirstNav_, &QPushButton::clicked, this, &MainWindow::showUseFirst);
    connect(recipesNav_, &QPushButton::clicked, this, &MainWindow::showRecipes);
    connect(statisticsNav_, &QPushButton::clicked, this, &MainWindow::showStatistics);
    sideLayout->addStretch();
    auto *milestone = new QLabel("MILESTONE 7\nSaved local inventory", sidebar);
    milestone->setObjectName("milestone");
    sideLayout->addWidget(milestone);

    auto *content = new QWidget(root);
    auto *mainLayout = new QVBoxLayout(content);
    mainLayout->setContentsMargins(36, 30, 36, 30);
    mainLayout->setSpacing(20);
    auto *top = new QHBoxLayout;
    auto *titles = new QVBoxLayout;
    auto *title = new QLabel("Smart Food Expiry Tracker", content);
    title->setObjectName("pageTitle");
    auto *subtitle = new QLabel("A clear view of what is in your pantry and when it expires.", content);
    subtitle->setObjectName("subtitle");
    titles->addWidget(title);
    titles->addWidget(subtitle);
    top->addLayout(titles);
    top->addStretch();
    auto *add = new QPushButton("＋  Add Food", content);
    add->setObjectName("addButton");
    add->setCursor(Qt::PointingHandCursor);
    connect(add, &QPushButton::clicked, this, &MainWindow::addFood);
    top->addWidget(add, 0, Qt::AlignVCenter);
    auto *simulate = new QPushButton("Simulate Scan", content);
    simulate->setObjectName("secondaryButton");
    simulate->setToolTip("Choose and review a built-in sample; no camera or scanner is used");
    simulate->setCursor(Qt::PointingHandCursor);
    connect(simulate, &QPushButton::clicked, this, &MainWindow::simulateScan);
    top->addWidget(simulate, 0, Qt::AlignVCenter);
    mainLayout->addLayout(top);

    auto *cards = new QHBoxLayout;
    const QStringList labels{"TOTAL ITEMS", "SOONEST EXPIRY", "CONSUMED", "WASTED"};
    for (int i = 0; i < labels.size(); ++i) {
        const auto &text = labels[i];
        auto *card = new QFrame(content);
        card->setObjectName("card");
        auto *cl = new QVBoxLayout(card);
        cl->setContentsMargins(18, 14, 18, 14);
        auto *caption = new QLabel(text, card);
        caption->setObjectName("cardCaption");
        auto *value = new QLabel(i == 0 || i >= 2 ? "0" : "—", card);
        value->setObjectName("cardValue");
        if (i == 0) itemCountValue_ = value;
        if (i == 1) soonestValue_ = value;
        if (i == 2) consumedValue_ = value;
        if (i == 3) wastedValue_ = value;
        cl->addWidget(caption);
        cl->addWidget(value);
        cards->addWidget(card);
    }
    mainLayout->addLayout(cards);
    auto *sessionNote = new QLabel("Expiry countdown updates daily. Inventory saves when you exit; consumed and wasted counts reset when the app closes.", content);
    sessionNote->setObjectName("subtitle");
    mainLayout->addWidget(sessionNote);

    persistenceStatus_ = new QLabel(content);
    persistenceStatus_->setObjectName("persistenceStatus");
    persistenceStatus_->setWordWrap(true);
    const QString dataPath = InventoryStore::filePath();
    if (!loadedInventory.error.isEmpty()) {
        persistenceStatus_->setText(QStringLiteral("Inventory load error: %1  ·  Data file: %2")
                                        .arg(loadedInventory.error, dataPath));
        persistenceStatus_->setProperty("warning", true);
    } else if (!loadedInventory.malformedRecords.isEmpty()) {
        persistenceStatus_->setText(QStringLiteral("Loaded %1 valid item(s); skipped %2 malformed record(s) (%3). Data file: %4")
                                        .arg(loadedInventory.items.size())
                                        .arg(loadedInventory.malformedRecords.size())
                                        .arg(loadedInventory.malformedRecords.join(", "), dataPath));
        persistenceStatus_->setProperty("warning", true);
    } else if (loadedInventory.fileMissing) {
        persistenceStatus_->setText(QStringLiteral("First run: no saved inventory yet. Items will save on exit to %1")
                                        .arg(dataPath));
    } else {
        persistenceStatus_->setText(QStringLiteral("Loaded %1 item(s). Inventory saves on exit to %2")
                                        .arg(loadedInventory.items.size()).arg(dataPath));
    }
    mainLayout->addWidget(persistenceStatus_);

    alertBanner_ = new QLabel(content);
    alertBanner_->setObjectName("alertBanner");
    alertBanner_->setWordWrap(true);
    alertBanner_->hide();
    mainLayout->addWidget(alertBanner_);

    pageStack_ = new QStackedWidget(content);
    auto *inventoryPage = new QWidget(pageStack_);
    auto *inventoryLayout = new QVBoxLayout(inventoryPage);
    inventoryLayout->setContentsMargins(0, 0, 0, 0);
    inventoryLayout->setSpacing(14);
    auto *tableTitle = new QLabel("Inventory", inventoryPage);
    tableTitle->setObjectName("sectionTitle");
    inventoryLayout->addWidget(tableTitle);
    auto *searchRow = new QHBoxLayout;
    searchEdit_ = new QLineEdit(inventoryPage);
    searchEdit_->setObjectName("searchField");
    searchEdit_->setPlaceholderText("Search exact food name (case-insensitive)");
    auto *clearSearchButton = new QPushButton("Clear", inventoryPage);
    clearSearchButton->setObjectName("secondaryButton");
    removeButton_ = new QPushButton("Remove Selected", inventoryPage);
    removeButton_->setObjectName("removeButton");
    removeButton_->setEnabled(false);
    searchRow->addWidget(searchEdit_, 1);
    searchRow->addWidget(clearSearchButton);
    searchRow->addWidget(removeButton_);
    inventoryLayout->addLayout(searchRow);
    connect(searchEdit_, &QLineEdit::textChanged, this, [this] { refreshInventory(); });
    connect(clearSearchButton, &QPushButton::clicked, searchEdit_, &QLineEdit::clear);
    connect(removeButton_, &QPushButton::clicked, this, &MainWindow::removeSelected);
    auto *inventoryStack = new QStackedWidget(inventoryPage);
    table_ = new QTableView(inventoryStack);
    tableModel_ = new FoodTableModel(this);
    table_->setModel(tableModel_);
    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table_->verticalHeader()->hide();
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setFocusPolicy(Qt::StrongFocus);
    table_->setShowGrid(false);
    table_->setAlternatingRowColors(true);
    table_->setObjectName("inventoryTable");
    connect(table_->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this] {
        removeButton_->setEnabled(!table_->selectionModel()->selectedRows(0).isEmpty());
    });
    auto *emptyPanel = new QWidget(inventoryStack);
    auto *emptyLayout = new QVBoxLayout(emptyPanel);
    emptyLayout->setContentsMargins(0, 0, 0, 0);
    emptyState_ = new QLabel("Your inventory is empty\n\nAdd your first food item to start keeping track.", emptyPanel);
    emptyState_->setObjectName("emptyState");
    emptyState_->setAlignment(Qt::AlignCenter);
    emptyLayout->addWidget(emptyState_);
    inventoryStack->addWidget(emptyPanel);
    inventoryStack->addWidget(table_);
    inventoryStack->setCurrentIndex(0);
    inventoryStack->setMinimumHeight(270);
    inventoryLayout->addWidget(inventoryStack, 1);

    auto *useFirstPage = new QWidget(pageStack_);
    auto *useFirstLayout = new QVBoxLayout(useFirstPage);
    useFirstLayout->setContentsMargins(0, 0, 0, 0);
    useFirstLayout->setSpacing(10);
    auto *useFirstTitle = new QLabel("Use First", useFirstPage);
    useFirstTitle->setObjectName("sectionTitle");
    auto *useFirstDescription = new QLabel("Your five most urgent items, ordered by expiry.", useFirstPage);
    useFirstDescription->setObjectName("subtitle");
    useFirstLayout->addWidget(useFirstTitle);
    useFirstLayout->addWidget(useFirstDescription);
    useFirstStack_ = new QStackedWidget(useFirstPage);
    useFirstTable_ = new QTableView(useFirstStack_);
    useFirstModel_ = new UseFirstTableModel(this);
    useFirstTable_->setModel(useFirstModel_);
    useFirstTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    useFirstTable_->verticalHeader()->hide();
    useFirstTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    useFirstTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    useFirstTable_->setFocusPolicy(Qt::NoFocus);
    useFirstTable_->setShowGrid(false);
    useFirstTable_->setAlternatingRowColors(true);
    useFirstTable_->setObjectName("useFirstTable");
    auto *useEmptyPanel = new QWidget(useFirstStack_);
    auto *useEmptyLayout = new QVBoxLayout(useEmptyPanel);
    auto *useEmpty = new QLabel("Nothing to use first yet\n\nAdd food to see your most urgent items here.", useEmptyPanel);
    useEmpty->setObjectName("emptyState");
    useEmpty->setAlignment(Qt::AlignCenter);
    useEmptyLayout->addWidget(useEmpty);
    useFirstStack_->addWidget(useEmptyPanel);
    useFirstStack_->addWidget(useFirstTable_);
    useFirstStack_->setCurrentIndex(0);
    useFirstStack_->setMinimumHeight(270);
    useFirstLayout->addWidget(useFirstStack_, 1);

    auto *statisticsPage = new QWidget(pageStack_);
    auto *statisticsLayout = new QVBoxLayout(statisticsPage);
    statisticsLayout->setContentsMargins(0, 0, 0, 0);
    statisticsLayout->setSpacing(14);
    auto *statisticsTitle = new QLabel("Session statistics", statisticsPage);
    statisticsTitle->setObjectName("sectionTitle");
    auto *statisticsNote = new QLabel("These counts are for this session only and reset when the app closes.", statisticsPage);
    statisticsNote->setObjectName("subtitle");
    statisticsLayout->addWidget(statisticsTitle);
    statisticsLayout->addWidget(statisticsNote);
    auto *statisticsCards = new QHBoxLayout;
    auto *consumedCard = new QFrame(statisticsPage);
    consumedCard->setObjectName("card");
    auto *consumedLayout = new QVBoxLayout(consumedCard);
    consumedLayout->setContentsMargins(20, 18, 20, 18);
    auto *consumedCaption = new QLabel("CONSUMED", consumedCard);
    consumedCaption->setObjectName("cardCaption");
    statisticsConsumedValue_ = new QLabel("0", consumedCard);
    statisticsConsumedValue_->setObjectName("cardValue");
    consumedLayout->addWidget(consumedCaption);
    consumedLayout->addWidget(statisticsConsumedValue_);
    auto *wastedCard = new QFrame(statisticsPage);
    wastedCard->setObjectName("card");
    auto *wastedLayout = new QVBoxLayout(wastedCard);
    wastedLayout->setContentsMargins(20, 18, 20, 18);
    auto *wastedCaption = new QLabel("WASTED", wastedCard);
    wastedCaption->setObjectName("cardCaption");
    statisticsWastedValue_ = new QLabel("0", wastedCard);
    statisticsWastedValue_->setObjectName("cardValue");
    wastedLayout->addWidget(wastedCaption);
    wastedLayout->addWidget(statisticsWastedValue_);
    statisticsCards->addWidget(consumedCard);
    statisticsCards->addWidget(wastedCard);
    statisticsLayout->addLayout(statisticsCards);
    statisticsLayout->addStretch();

    auto *recipesPage = new QWidget(pageStack_);
    auto *recipesLayout = new QVBoxLayout(recipesPage);
    recipesLayout->setContentsMargins(0, 0, 0, 0);
    recipesLayout->setSpacing(14);
    auto *recipesTitle = new QLabel("Recipe ideas", recipesPage);
    recipesTitle->setObjectName("sectionTitle");
    auto *recipesDescription = new QLabel("Simple suggestions from a built-in local list. These are not nutritional advice.", recipesPage);
    recipesDescription->setObjectName("subtitle");
    recipesLayout->addWidget(recipesTitle);
    recipesLayout->addWidget(recipesDescription);
    auto *pickerCaption = new QLabel("Choose an inventory item (most urgent first)", recipesPage);
    pickerCaption->setObjectName("cardCaption");
    recipeItemPicker_ = new QComboBox(recipesPage);
    recipeItemPicker_->setObjectName("recipeItemPicker");
    recipeItemPicker_->setMinimumHeight(40);
    connect(recipeItemPicker_, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::updateRecipeSuggestion);
    recipesLayout->addWidget(pickerCaption);
    recipesLayout->addWidget(recipeItemPicker_);
    auto *recipeCard = new QFrame(recipesPage);
    recipeCard->setObjectName("card");
    auto *recipeCardLayout = new QVBoxLayout(recipeCard);
    recipeCardLayout->setContentsMargins(20, 18, 20, 18);
    recipeCardLayout->setSpacing(10);
    recipeItemStatus_ = new QLabel(recipeCard);
    recipeItemStatus_->setObjectName("sectionTitle");
    recipeItemStatus_->setWordWrap(true);
    recipeSuggestion_ = new QLabel(recipeCard);
    recipeSuggestion_->setWordWrap(true);
    recipeSuggestion_->setMinimumHeight(70);
    recipeTip_ = new QLabel("Low-waste tip: check what you already have and use the most perishable ingredients first.", recipeCard);
    recipeTip_->setObjectName("subtitle");
    recipeTip_->setWordWrap(true);
    recipeCardLayout->addWidget(recipeItemStatus_);
    recipeCardLayout->addWidget(recipeSuggestion_);
    recipeCardLayout->addWidget(recipeTip_);
    recipesLayout->addWidget(recipeCard);
    recipesLayout->addStretch();

    pageStack_->addWidget(inventoryPage);
    pageStack_->addWidget(useFirstPage);
    pageStack_->addWidget(statisticsPage);
    pageStack_->addWidget(recipesPage);
    pageStack_->setCurrentIndex(0);
    mainLayout->addWidget(pageStack_, 1);
    layout->addWidget(sidebar);
    layout->addWidget(content, 1);
    setCentralWidget(root);

    lastExpiryRefreshDate_ = QDate::currentDate();
    auto *expiryDateTimer = new QTimer(this);
    expiryDateTimer->setInterval(60 * 1000);
    connect(expiryDateTimer, &QTimer::timeout, this, [this] {
        const QDate today = QDate::currentDate();
        if (today != lastExpiryRefreshDate_) refreshInventory();
    });
    expiryDateTimer->start();

    setStyleSheet(R"(
        QMainWindow, QWidget { background: #f5f7f5; color: #1f3027; font-family: 'Segoe UI', sans-serif; font-size: 14px; }
        QLabel { background: transparent; }
        #sidebar { background: #173b2b; }
        #sidebar QLabel { color: #f4f8f4; background: transparent; }
        #brand { font-size: 21px; font-weight: 700; }
        #navActive, #navFuture { text-align: left; padding: 12px 14px; border: 0; border-radius: 8px; font-size: 14px; }
        #navActive { background: #2b5941; color: white; font-weight: 600; }
        #navFuture { color: #c1d0c7; background: transparent; }
        #milestone { color: #b2c8ba; font-size: 12px; line-height: 1.5; padding: 12px 4px; }
        #pageTitle { font-size: 27px; font-weight: 700; color: #172b20; }
        #subtitle { color: #64756b; }
        #persistenceStatus { color: #526458; background: #e9f0eb; border-radius: 6px; padding: 7px 10px; font-size: 12px; }
        #persistenceStatus[warning="true"] { color: #704800; background: #fff0d1; }
        #addButton { background: #2f7951; color: white; border: 0; border-radius: 8px; padding: 12px 18px; font-weight: 600; }
        #searchField { background: white; border: 1px solid #cbd7ce; border-radius: 7px; padding: 9px 11px; }
        #secondaryButton { background: #e8eee9; color: #294d38; border: 1px solid #d2ddd5; border-radius: 7px; padding: 9px 14px; }
        #removeButton { background: #8b3d35; color: white; border: 0; border-radius: 7px; padding: 9px 14px; font-weight: 600; }
        #removeButton:disabled { background: #b8c1bb; color: #f4f7f4; }
        #card { background: white; border: 1px solid #e3e9e4; border-radius: 10px; }
        #cardCaption { color: #75847a; font-size: 11px; font-weight: 700; letter-spacing: 1px; }
        #cardValue { font-size: 26px; font-weight: 700; color: #244c35; }
        #sectionTitle { font-size: 19px; font-weight: 650; }
        #recipeItemPicker { background: white; border: 1px solid #cbd7ce; border-radius: 7px; padding: 8px 10px; }
        #inventoryTable, #useFirstTable { background: white; alternate-background-color: #f8faf8; border: 1px solid #e3e9e4; border-radius: 8px; }
        QHeaderView::section { background: #eef3ef; color: #526458; padding: 11px 8px; border: 0; font-weight: 600; }
        #emptyState { background: white; border: 1px dashed #cbd7ce; border-radius: 10px; color: #6a7a70; font-size: 15px; }
        #alertBanner { background: #fff0d1; color: #704800; border: 1px solid #e4c173; border-left: 5px solid #c18416; border-radius: 7px; padding: 12px 15px; font-weight: 650; }
        #formError { color: #9b332b; background: #fff0ed; border: 1px solid #e9c0b8; border-radius: 6px; padding: 8px; }
        #dialogAddButton { background: #2f7951; color: white; border: 0; border-radius: 6px; padding: 8px 14px; font-weight: 600; }
    )");
    refreshInventory();
}

void MainWindow::addFood() {
    AddFoodDialog dialog(this);
    addFoodFromDialog(dialog);
}

void MainWindow::simulateScan() {
    SimulateScanDialog sampleDialog(this);
    if (sampleDialog.exec() != QDialog::Accepted || !sampleDialog.selectedItem()) return;
    AddFoodDialog reviewDialog(this, sampleDialog.selectedItem(), true);
    addFoodFromDialog(reviewDialog);
}

void MainWindow::addFoodFromDialog(AddFoodDialog &dialog) {
    if (dialog.exec() != QDialog::Accepted || !dialog.foodItem()) return;
    inventory_.add(*dialog.foodItem());
    refreshInventory();
}

void MainWindow::refreshInventory() {
    lastExpiryRefreshDate_ = QDate::currentDate();
    inventory_.refreshExpiryDays(lastExpiryRefreshDate_);
    const QString query = normalizedFoodName(searchEdit_->text());
    const auto visibleItems = query.isEmpty() ? inventory_.itemsByExpiry() : inventory_.searchByName(searchEdit_->text());
    tableModel_->setItems(visibleItems);
    useFirstModel_->setItems(inventory_.itemsByExpiry(5));
    refreshRecipes();
    itemCountValue_->setText(QString::number(inventory_.size()));
    consumedValue_->setText(QString::number(statistics_.consumed()));
    wastedValue_->setText(QString::number(statistics_.wasted()));
    statisticsConsumedValue_->setText(QString::number(statistics_.consumed()));
    statisticsWastedValue_->setText(QString::number(statistics_.wasted()));
    const auto soonest = inventory_.soonest();
    if (soonest) {
        soonestValue_->setText(expiryStatus(soonest->daysUntilExpiry));
        if (soonest->daysUntilExpiry <= 2) {
            alertBanner_->setText(QStringLiteral("Heads up — %1: %2.")
                                      .arg(soonest->name, expiryStatus(soonest->daysUntilExpiry)));
            alertBanner_->show();
        } else {
            alertBanner_->hide();
        }
    } else {
        soonestValue_->setText("—");
        alertBanner_->clear();
        alertBanner_->hide();
    }
    auto *stack = qobject_cast<QStackedWidget *>(table_->parentWidget());
    if (visibleItems.empty()) {
        if (inventory_.empty())
            emptyState_->setText("Your inventory is empty\n\nAdd your first food item to start keeping track.");
        else
            emptyState_->setText(QStringLiteral("No exact match for “%1”.\n\nClear search to show the full inventory.")
                                     .arg(searchEdit_->text().trimmed()));
        stack->setCurrentWidget(emptyState_->parentWidget());
    } else {
        stack->setCurrentWidget(table_);
    }
    useFirstStack_->setCurrentIndex(inventory_.empty() ? 0 : 1);
    removeButton_->setEnabled(false);
}

void MainWindow::showInventory() {
    refreshInventory();
    pageStack_->setCurrentIndex(0);
    setActiveNavigation(inventoryNav_);
}

void MainWindow::showUseFirst() {
    refreshInventory();
    pageStack_->setCurrentIndex(1);
    setActiveNavigation(useFirstNav_);
}

void MainWindow::showStatistics() {
    refreshInventory();
    pageStack_->setCurrentIndex(2);
    setActiveNavigation(statisticsNav_);
}

void MainWindow::showRecipes() {
    refreshInventory();
    pageStack_->setCurrentIndex(3);
    setActiveNavigation(recipesNav_);
}

void MainWindow::refreshRecipes() {
    const auto items = inventory_.itemsByExpiry();
    const auto oldId = recipeItemPicker_->currentData().toULongLong();
    const QSignalBlocker blocker(recipeItemPicker_);
    recipeItemPicker_->clear();
    int selectedIndex = -1;
    for (const auto &item : items) {
        recipeItemPicker_->addItem(item.name, QVariant::fromValue<qulonglong>(item.id));
        if (item.id == oldId) selectedIndex = recipeItemPicker_->count() - 1;
    }
    if (selectedIndex < 0 && !items.empty()) selectedIndex = 0;
    recipeItemPicker_->setCurrentIndex(selectedIndex);
    recipeItemPicker_->setEnabled(!items.empty());
    updateRecipeSuggestion();
}

void MainWindow::updateRecipeSuggestion() {
    const auto selectedId = recipeItemPicker_->currentData().toULongLong();
    if (recipeItemPicker_->currentIndex() < 0 || inventory_.empty()) {
        recipeItemStatus_->setText("No inventory items yet");
        recipeSuggestion_->setText("Add food to see a local recipe idea for an item in your pantry.");
        return;
    }
    const auto items = inventory_.itemsByExpiry();
    for (const auto &item : items) {
        if (item.id != selectedId) continue;
        recipeItemStatus_->setText(QStringLiteral("%1 · %2").arg(item.name, expiryStatus(item.daysUntilExpiry)));
        const auto suggestion = RecipeSuggestions::find(item.name);
        if (suggestion) {
            recipeSuggestion_->setText(*suggestion);
        } else {
            recipeSuggestion_->setText(QStringLiteral("No recipe idea is built in for “%1” yet. Try a familiar recipe that uses it.").arg(item.name));
        }
        return;
    }
    recipeItemStatus_->setText("No inventory items yet");
    recipeSuggestion_->setText("Add food to see a local recipe idea for an item in your pantry.");
}

void MainWindow::removeSelected() {
    const auto selectedRows = table_->selectionModel()->selectedRows(0);
    if (selectedRows.isEmpty()) return;
    const auto item = tableModel_->itemAt(selectedRows.front().row());
    if (!item) {
        QMessageBox::warning(this, "Selection changed", "That inventory selection is no longer available. The table has been refreshed.");
        refreshInventory();
        return;
    }

    QMessageBox choice(QMessageBox::Question, "Remove food", QStringLiteral("How should “%1” be recorded?").arg(item->name),
                       QMessageBox::NoButton, this);
    choice.setInformativeText("Choose whether this item was consumed or wasted. Cancel leaves the inventory unchanged.");
    auto *consumed = choice.addButton("Consumed", QMessageBox::AcceptRole);
    auto *wasted = choice.addButton("Wasted", QMessageBox::DestructiveRole);
    auto *cancel = choice.addButton(QMessageBox::Cancel);
    choice.setEscapeButton(cancel);
    choice.exec();
    const auto clicked = choice.clickedButton();
    if (clicked != consumed && clicked != wasted) return;

    if (!inventory_.removeById(item->id)) {
        QMessageBox::warning(this, "Item no longer available", "This item is no longer in the inventory. The table has been refreshed.");
        refreshInventory();
        return;
    }
    statistics_.record(clicked == consumed ? RemovalDisposition::Consumed : RemovalDisposition::Wasted);
    refreshInventory();
}

void MainWindow::setActiveNavigation(QPushButton *active) {
    for (auto *button : {inventoryNav_, useFirstNav_, recipesNav_, statisticsNav_}) {
        button->setObjectName(button == active ? "navActive" : "navFuture");
        button->style()->unpolish(button);
        button->style()->polish(button);
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    while (true) {
        QString error;
        if (InventoryStore::save(inventory_.itemsByExpiry(), &error)) {
            persistenceStatus_->setText(QStringLiteral("Inventory saved to %1").arg(InventoryStore::filePath()));
            event->accept();
            return;
        }

        QMessageBox choice(QMessageBox::Critical, "Could not save inventory",
                           "Your inventory was not saved.", QMessageBox::NoButton, this);
        choice.setInformativeText(QStringLiteral("%1\n\nChoose Retry to try again, or Exit without saving to close and lose the current inventory changes.")
                                      .arg(error));
        auto *retry = choice.addButton("Retry", QMessageBox::AcceptRole);
        auto *exitWithoutSaving = choice.addButton("Exit without saving", QMessageBox::DestructiveRole);
        auto *cancel = choice.addButton(QMessageBox::Cancel);
        choice.setEscapeButton(cancel);
        choice.exec();
        const auto clicked = choice.clickedButton();
        if (clicked == retry) continue;
        if (clicked == exitWithoutSaving) {
            event->accept();
            return;
        }
        event->ignore();
        return;
    }
}

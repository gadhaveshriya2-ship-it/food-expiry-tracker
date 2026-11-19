#pragma once
#include <QMainWindow>
#include <QDate>
#include "inventory.h"
#include "sessionstatistics.h"
#include "inventorystore.h"
#include <QCloseEvent>

class FoodTableModel;
class UseFirstTableModel;
class AddFoodDialog;
class QLabel;
class QPushButton;
class QStackedWidget;
class QTableView;
class QLineEdit;
class QComboBox;

class MainWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void addFood();
    void simulateScan();
    void showInventory();
    void showUseFirst();
    void showStatistics();
    void showRecipes();
    void removeSelected();

private:
    void refreshInventory();
    void setActiveNavigation(QPushButton *active);
    void addFoodFromDialog(AddFoodDialog &dialog);
    void refreshRecipes();
    void updateRecipeSuggestion();
    Inventory inventory_;
    SessionStatistics statistics_;
    FoodTableModel *tableModel_ = nullptr;
    QTableView *table_ = nullptr;
    QLabel *emptyState_ = nullptr;
    QLabel *itemCountValue_ = nullptr;
    QLabel *soonestValue_ = nullptr;
    QLabel *consumedValue_ = nullptr;
    QLabel *wastedValue_ = nullptr;
    QLabel *statisticsConsumedValue_ = nullptr;
    QLabel *statisticsWastedValue_ = nullptr;
    QLabel *alertBanner_ = nullptr;
    QLabel *persistenceStatus_ = nullptr;
    QPushButton *inventoryNav_ = nullptr;
    QPushButton *useFirstNav_ = nullptr;
    QPushButton *statisticsNav_ = nullptr;
    QPushButton *recipesNav_ = nullptr;
    QPushButton *removeButton_ = nullptr;
    QLineEdit *searchEdit_ = nullptr;
    QStackedWidget *pageStack_ = nullptr;
    QStackedWidget *useFirstStack_ = nullptr;
    QComboBox *recipeItemPicker_ = nullptr;
    QLabel *recipeItemStatus_ = nullptr;
    QLabel *recipeSuggestion_ = nullptr;
    QLabel *recipeTip_ = nullptr;
    QDate lastExpiryRefreshDate_;
    UseFirstTableModel *useFirstModel_ = nullptr;
    QTableView *useFirstTable_ = nullptr;
};

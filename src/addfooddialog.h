#pragma once

#include "fooditem.h"
#include <QDialog>
#include <optional>

class QLineEdit;
class QSpinBox;
class QLabel;

class AddFoodDialog final : public QDialog {
    Q_OBJECT
public:
    explicit AddFoodDialog(QWidget *parent = nullptr,
                           std::optional<FoodItem> initialItem = std::nullopt,
                           bool simulatedScan = false);
    std::optional<FoodItem> foodItem() const { return foodItem_; }

private slots:
    void submit();

private:
    QLineEdit *nameEdit_ = nullptr;
    QLineEdit *categoryEdit_ = nullptr;
    QSpinBox *quantitySpin_ = nullptr;
    QLineEdit *locationEdit_ = nullptr;
    QSpinBox *daysSpin_ = nullptr;
    QLabel *errorLabel_ = nullptr;
    std::optional<FoodItem> foodItem_;
};

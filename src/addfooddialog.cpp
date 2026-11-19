#include "addfooddialog.h"
#include <QDialogButtonBox>
#include <QDate>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

AddFoodDialog::AddFoodDialog(QWidget *parent, std::optional<FoodItem> initialItem, bool simulatedScan) : QDialog(parent) {
    setWindowTitle(simulatedScan ? "Review simulated scan" : "Add food to inventory");
    setMinimumWidth(420);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 22, 24, 20);
    layout->setSpacing(14);
    auto *intro = new QLabel(simulatedScan
        ? "Review or edit these sample values before adding. This demonstration does not use a camera, OCR, or barcode reader."
        : "Enter the food details. Inventory changes are saved when the app exits.", this);
    intro->setWordWrap(true);
    layout->addWidget(intro);

    auto *form = new QFormLayout;
    form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    nameEdit_ = new QLineEdit(this);
    nameEdit_->setPlaceholderText("e.g. Cherry tomatoes");
    categoryEdit_ = new QLineEdit(this);
    categoryEdit_->setPlaceholderText("e.g. Fresh produce");
    quantitySpin_ = new QSpinBox(this);
    quantitySpin_->setRange(1, 1000000);
    quantitySpin_->setValue(1);
    quantitySpin_->setSuffix(" units");
    locationEdit_ = new QLineEdit(this);
    locationEdit_->setPlaceholderText("e.g. Top shelf of fridge");
    daysSpin_ = new QSpinBox(this);
    daysSpin_->setRange(-36500, 36500);
    daysSpin_->setSuffix(" days");
    daysSpin_->setToolTip("Countdown is based on today's date and advances automatically each calendar day.");
    if (initialItem) {
        nameEdit_->setText(initialItem->name);
        categoryEdit_->setText(initialItem->category);
        quantitySpin_->setValue(initialItem->quantity);
        locationEdit_->setText(initialItem->storageLocation);
        daysSpin_->setValue(initialItem->daysUntilExpiry);
    }
    form->addRow("Food name *", nameEdit_);
    form->addRow("Category *", categoryEdit_);
    form->addRow("Quantity *", quantitySpin_);
    form->addRow("Storage location *", locationEdit_);
    form->addRow("Days until expiry *", daysSpin_);
    layout->addLayout(form);

    errorLabel_ = new QLabel(this);
    errorLabel_->setObjectName("formError");
    errorLabel_->setWordWrap(true);
    errorLabel_->hide();
    layout->addWidget(errorLabel_);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
    auto *addButton = buttons->addButton(simulatedScan ? "Add simulated item" : "Add to inventory", QDialogButtonBox::AcceptRole);
    addButton->setObjectName("dialogAddButton");
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(addButton, &QPushButton::clicked, this, &AddFoodDialog::submit);
    layout->addWidget(buttons);
}

void AddFoodDialog::submit() {
    const QString name = nameEdit_->text().trimmed();
    const QString category = categoryEdit_->text().trimmed();
    const QString location = locationEdit_->text().trimmed();
    if (name.isEmpty() || category.isEmpty() || location.isEmpty()) {
        errorLabel_->setText("Enter a food name, category, and storage location to continue.");
        errorLabel_->show();
        if (name.isEmpty()) nameEdit_->setFocus();
        else if (category.isEmpty()) categoryEdit_->setFocus();
        else locationEdit_->setFocus();
        return;
    }
    if (quantitySpin_->value() < 1) {
        errorLabel_->setText("Quantity must be at least 1.");
        errorLabel_->show();
        quantitySpin_->setFocus();
        return;
    }
    FoodItem item{0, name, category, quantitySpin_->value(), location, daysSpin_->value()};
    item.expiryDate = QDate::currentDate().addDays(daysSpin_->value());
    foodItem_ = item;
    accept();
}

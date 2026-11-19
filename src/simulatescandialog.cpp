#include "simulatescandialog.h"
#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

SimulateScanDialog::SimulateScanDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Simulate Scan");
    setMinimumWidth(440);
    samples_ = {
        {0, "Tomatoes", "Fresh produce", 3, "Fridge drawer", 3},
        {0, "Whole grain bread", "Bakery", 1, "Kitchen counter", 2},
        {0, "Milk", "Dairy", 1, "Fridge door", 5},
        {0, "Eggs", "Dairy", 6, "Fridge shelf", 10},
        {0, "Bananas", "Fresh produce", 4, "Fruit bowl", 4}
    };

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 22, 24, 20);
    layout->setSpacing(14);
    auto *notice = new QLabel(
        "Demonstration only: this does not access a camera or use OCR, barcode reading, or external services."
        " Choose a built-in sample, then review and edit its details before adding it.", this);
    notice->setWordWrap(true);
    layout->addWidget(notice);

    samplePicker_ = new QComboBox(this);
    samplePicker_->setObjectName("samplePicker");
    for (const auto &sample : samples_) samplePicker_->addItem(sample.name);
    layout->addWidget(new QLabel("Built-in sample food", this));
    layout->addWidget(samplePicker_);
    preview_ = new QLabel(this);
    preview_->setObjectName("samplePreview");
    preview_->setWordWrap(true);
    layout->addWidget(preview_);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
    auto *reviewButton = buttons->addButton("Review sample", QDialogButtonBox::AcceptRole);
    reviewButton->setObjectName("dialogAddButton");
    connect(samplePicker_, qOverload<int>(&QComboBox::currentIndexChanged), this, &SimulateScanDialog::updatePreview);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(reviewButton, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(buttons);
    updatePreview();
}

std::optional<FoodItem> SimulateScanDialog::selectedItem() const {
    const int index = samplePicker_->currentIndex();
    if (index < 0 || index >= static_cast<int>(samples_.size())) return std::nullopt;
    return samples_[static_cast<std::size_t>(index)];
}

void SimulateScanDialog::updatePreview() {
    const auto sample = selectedItem();
    if (!sample) {
        preview_->clear();
        return;
    }
    preview_->setText(QStringLiteral("Category: %1\nQuantity: %2 units\nLocation: %3\nExpiry: %4 days")
                          .arg(sample->category)
                          .arg(sample->quantity)
                          .arg(sample->storageLocation)
                          .arg(sample->daysUntilExpiry));
}

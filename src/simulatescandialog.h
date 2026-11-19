#pragma once

#include "fooditem.h"
#include <QDialog>
#include <optional>
#include <vector>

class QComboBox;
class QLabel;

class SimulateScanDialog final : public QDialog {
    Q_OBJECT
public:
    explicit SimulateScanDialog(QWidget *parent = nullptr);
    std::optional<FoodItem> selectedItem() const;

private slots:
    void updatePreview();

private:
    QComboBox *samplePicker_ = nullptr;
    QLabel *preview_ = nullptr;
    std::vector<FoodItem> samples_;
};
